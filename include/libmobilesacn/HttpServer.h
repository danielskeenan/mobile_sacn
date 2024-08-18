/**
 * @file HttpServer.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_

#include <QObject>
#include <string>
#include <QHttpServer>

namespace mobilesacn {
/**
 * HTTP web server.
 *
 * Handles serving the Web UI and RPC requests.
 */
class HttpServer : public QObject {
  public:
    struct Options {
      std::string backend_address;
      std::string sacn_address;
    };

    explicit HttpServer(Options options, QObject* parent = nullptr);

    void run();
    void stop();
    [[nodiscard]] std::string getUrl() const;

  private:
    /** Start with this port number when finding a port to bind to. */
    static const uint16_t kHttpPortStart = 5050;
    Options options_;
    QHttpServer* server_ = nullptr;
};
} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
