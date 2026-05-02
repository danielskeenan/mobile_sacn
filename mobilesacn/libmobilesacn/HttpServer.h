/**
 * @file HttpServer.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_

#include <etcpal/cpp/netint.h>
#include <filesystem>
#include <httplib.h>
#include <string>
#include <QThread>
#include <QThreadPool>
#include <QWebSocketServer>

namespace mobilesacn {

/**
 * httplib::TaskQueue that uses a QThreadPool.
 *
 * This keeps all threads managed by a QThread.
 */
class HttpQtTaskQueue : public QObject, public httplib::TaskQueue
{
    Q_OBJECT

public:
    explicit HttpQtTaskQueue(QObject *parent = nullptr);
    bool enqueue(std::function<void()> fn) override;
    void shutdown() override;

private:
    QThreadPool *threadPool_;
};

/**
 * Run the HTTP server in its own thread.
 */
class HttpServerController : public QThread
{
    Q_OBJECT

public:
    explicit HttpServerController(
        httplib::Server *server, QWebSocketServer *wsServer, QObject *parent = nullptr);

protected:
    void run() override;

private:
    httplib::Server *server_;
    QWebSocketServer *wsServer_;

    // Route handlers.
    static void serveStaticFile(const httplib::Request &req, httplib::Response &res);
};

/**
 * HTTP web server.
 *
 * Handles serving the Web UI and RPC requests.
 */
class HttpServer : public QObject
{
    Q_OBJECT

public:
    struct Options
    {
        std::string backend_address;
        etcpal::NetintInfo sacn_interface;
    };

    /**
     * Create a new HTTP Server. There should only be one of these!
     *
     * The constructor will change the working directory to the webroot.
     *
     * @param options
     * @param parent
     */
    explicit HttpServer(Options options, QObject *parent = nullptr);
    ~HttpServer();

    void run();
    void stop();
    [[nodiscard]] std::string getUrl();

    [[nodiscard]] const Options &getOptions() const { return options_; }

private:
    static constexpr uint16_t kHttpPort = 5050;

    Options options_;
    httplib::Server server_;
    QWebSocketServer wsServer_;

private Q_SLOTS:
    void onWsNewConnection();
    void onWsAcceptError(QAbstractSocket::SocketError socketError);
    void onWsServerError(QWebSocketProtocol::CloseCode closeCode);
};
} // namespace mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
