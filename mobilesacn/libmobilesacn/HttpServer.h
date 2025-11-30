/**
 * @file HttpServer.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_

#include <string>
#include <filesystem>
#include <QObject>
#include <crow/app.h>
#include <crow/middlewares/cors.h>
#include <etcpal/cpp/netint.h>
#include "CrowLogHandler.h"

namespace mobilesacn {
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

    using CrowServer = crow::Crow<crow::CORSHandler>;

    /**
     * Create a new HTTP Server. There should only be one of these!
     *
     * The constructor will change the working directory to the webroot.
     *
     * @param options
     * @param parent
     */
    explicit HttpServer(Options options, QObject* parent = nullptr);

    void run();
    void stop();
    [[nodiscard]] std::string getUrl();

    [[nodiscard]] const Options& getOptions() const
    {
        return options_;
    }

private:
    static const uint16_t kHttpPort = 5050;

    Options options_;
    CrowServer server_;
    CrowLogHandler crowLogHandler_;
    std::future<void> serverHandle_;

    static const std::filesystem::path& getWebRoot();
};
} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
