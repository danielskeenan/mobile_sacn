/**
 * @file Application.h
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_APPLICATION_H
#define MOBILE_SACN_APPLICATION_H

#define CROW_DISABLE_STATIC_DIR

#include "EtcPalLogHandler.h"
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <filesystem>
#include <etcpal/cpp/inet.h>

namespace sacn {
class Source;
}

namespace mobilesacn {

class HttpServer;

/**
 * Application instance.
 */
class Application {
 public:
  struct Options {
    std::string backend_address;
    unsigned int backend_port;
    etcpal::IpAddr sacn_address;
  };

  explicit Application(Options options);
  ~Application();

  int Run();

 private:
  Options options_;
  etcpal::Logger etc_pal_logger_;
  EtcPalLogHandler etc_pal_log_handler_;
  std::unique_ptr<HttpServer> http_server_;
};

} // mobilesacn

#endif //MOBILE_SACN_APPLICATION_H
