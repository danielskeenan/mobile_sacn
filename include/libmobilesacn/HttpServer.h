/**
 * @file HttpServer.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_

#include <unordered_map>
#include <string>
#include <chrono>
#include <filesystem>
#include <sacn/cpp/source.h>
#include "libmobilesacn/rpc/RpcHandler.h"
#include "CrowLogHandler.h"

namespace mobilesacn {

/**
 * HTTP web server.
 *
 * Handles serving the Web UI and RPC requests.
 */
class HttpServer {
 public:
  struct Options {
    std::string backend_address;
    unsigned int backend_port;
    etcpal::IpAddr sacn_address;
  };

  explicit HttpServer(Options options)
      : options_(std::move(options)) {}

  void Run();

 private:
  class Handler {
   public:
    explicit Handler(std::unique_ptr<rpc::RpcHandler> handler) :
        handler_(std::move(handler)), last_use_(std::chrono::steady_clock::now()) {}

    /** Get the handler and update the last use time. */
    [[nodiscard]] const std::unique_ptr<rpc::RpcHandler> &GetHandler() {
      last_use_ = std::chrono::steady_clock::now();
      return handler_;
    }

    [[nodiscard]] const std::chrono::time_point<std::chrono::steady_clock> &GetLastUse() const {
      return last_use_;
    }

   private:
    std::unique_ptr<rpc::RpcHandler> handler_;
    std::chrono::time_point<std::chrono::steady_clock> last_use_;
  };

  Options options_;
  CrowLogHandler crow_log_handler_;
  std::unordered_map<std::string, Handler> handlers_;

  /**
   * Find the web root for the web UI.
   *
   * This path will never change during the program's execution.
   */
  static std::filesystem::path GetWebUiRoot();
  static crow::response RedirectToIndex();

  /**
   * Verify the static file path is in the webroot
   * @return
   */
  static bool FilePathInWebroot(const std::filesystem::path &path);
};

} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
