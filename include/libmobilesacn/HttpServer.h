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
#include "rpc/RpcHandler.h"
#include "CrowLogHandler.h"
#include <crow/routing.h>
#include <crow/app.h>
#include <spdlog/spdlog.h>
#include "HttpReasons.h"

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
    etcpal::IpAddr sacn_address;
  };

  explicit HttpServer(Options options);

  void Run();
  void Stop();
  std::string GetUrl() const;

 private:
  class Handler {
   public:
    explicit Handler(const std::string &client_ip_addr, std::unique_ptr<rpc::RpcHandler> handler) :
        client_ip_addr_(client_ip_addr), handler_(std::move(handler)), last_use_(std::chrono::steady_clock::now()) {}

    /** Get the handler and update the last use time. */
    [[nodiscard]] const std::unique_ptr<rpc::RpcHandler> &GetHandler() const {
      last_use_ = std::chrono::steady_clock::now();
      return handler_;
    }

    [[nodiscard]] const std::chrono::time_point<std::chrono::steady_clock> &GetLastUse() const {
      return last_use_;
    }

   private:
    std::string client_ip_addr_;
    std::unique_ptr<rpc::RpcHandler> handler_;
    mutable std::chrono::time_point<std::chrono::steady_clock> last_use_;
  };

  static inline const unsigned int kServerPort = 5050;

  Options options_;
  CrowLogHandler crow_log_handler_;
  crow::SimpleApp crow_;
  std::future<void> crow_handle_;
  std::unordered_map<std::string, Handler> handlers_;

  /**
   * Find the web root for the web UI.
   *
   * This path will never change during the program's execution.
   */
  static std::filesystem::path GetWebUiRoot();
  static crow::response RedirectToIndex();

  /**
   * Verify the static file path is in the webroot.
   * @return
   */
  static bool FilePathInWebroot(const std::filesystem::path &path);

  /**
   * Setup a websocket handler.
   *
   * @tparam Handler_T
   * @param rule A route created using CROW_ROUTE().websocket().
   * @param identifier The identifier to use in logging.
   */
  template<class Handler_T>
  void SetWebsocketHandler(crow::WebSocketRule &rule, const std::string &identifier) {
    // Queries to conn.get_remote_ip() can throw if the socket is dead.
    rule
        .onaccept([this](const crow::request &req) {
          GetHandler<Handler_T>(req.remote_ip_address);
          return true;
        }).onopen([this, identifier](crow::websocket::connection &conn) {
          spdlog::info("New \"{}\" connection from {}", identifier, GetConnRemoteIp(conn));
          try {
            GetHandler<Handler_T>(conn.get_remote_ip()).second.GetHandler()->HandleWsOpen(conn);
          } catch (const boost::system::system_error &) {
            conn.close(DisconnectReason::ClientDisconnected);
          }
        })
        .onmessage([this](crow::websocket::connection &conn, const std::string &message, bool is_binary) {
          try {
            GetHandler<Handler_T>(conn.get_remote_ip()).second.GetHandler()->HandleWsMessage(conn, message, is_binary);
          } catch (const boost::system::system_error &) {
            conn.close(DisconnectReason::ClientDisconnected);
          }
        })
        .onerror([this](crow::websocket::connection &conn) {
          try {
            GetHandler<Handler_T>(conn.get_remote_ip()).second.GetHandler()->HandleWsError(conn);
          } catch (const boost::system::system_error &) {
            conn.close(DisconnectReason::ClientDisconnected);
          }
        })
        .onclose([this, identifier](crow::websocket::connection &conn, const std::string &reason) {
          spdlog::info("Closed \"{}\" connection from {}", identifier, GetConnRemoteIp(conn));
          if (reason == DisconnectReason::ClientDisconnected) {
            spdlog::debug("Websocket connection \"{}\" closed because client went away unexpectedly", identifier);
          }
          try {
            auto &it = GetHandler<Handler_T>(conn.get_remote_ip());
            it.second.GetHandler()->HandleWsClose(&conn, reason);
            handlers_.erase(it.first);
          } catch (const boost::system::system_error &) {
            // Do nothing; the connection is already closed.
          }
        });
  }

  template<class T>
  std::pair<const std::string, Handler> &GetHandler(const std::string &ip_addr);

  void CleanupUnusedHandlers();

  /**
   * Try to get the remote up for a websocket connection.
   *
   * Used for logging, not guaranteed to return a valid IP if the client has disconnected.
   * @param conn
   * @return
   */
  static std::string GetConnRemoteIp(crow::websocket::connection &conn);
};

} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
