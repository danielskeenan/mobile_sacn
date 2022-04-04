/**
 * @file HttpServer.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/HttpServer.h"
#include <boost/dll/runtime_symbol_info.hpp>
#include "mobilesacn_config.h"
#include "libmobilesacn/rpc/ChanCheck.h"
#include <crow/app.h>
#include <crow/middlewares/cors.h>

namespace mobilesacn {

#define APP_ROUTE_WS(server, endpoint, Handler_T, ...) \
  CROW_ROUTE(server, endpoint).websocket() \
      .onaccept([this](const crow::request &req) { \
        auto &handler = \
            handlers_.try_emplace(endpoint, Handler(std::make_unique<Handler_T>(__VA_ARGS__))) \
                .first->second; \
        return true; \
      }).onopen([this](crow::websocket::connection &conn) { \
        handlers_.at(endpoint).GetHandler()->HandleWsOpen(conn); \
      }).onmessage([this](crow::websocket::connection &conn, const std::string &message, bool is_binary) { \
        handlers_.at(endpoint).GetHandler()->HandleWsMessage(conn, message, is_binary); \
      }).onerror([this](crow::websocket::connection &conn) { \
        handlers_.at(endpoint).GetHandler()->HandleWsError(conn); \
      }).onclose([this](crow::websocket::connection &conn, const std::string &reason) { \
        handlers_.at(endpoint).GetHandler()->HandleWsClose(conn, reason); \
      })

void HttpServer::Run() {
  crow::logger::setHandler(&crow_log_handler_);
  crow::Crow<crow::CORSHandler> server;
  server
      .server_name(config::kProjectName)
      .bindaddr(options_.backend_address)
      .port(options_.backend_port)
      .multithreaded();
  auto &cors = server.get_middleware<crow::CORSHandler>();
  cors.global()
      .methods(crow::HTTPMethod::Get);

  // API Hooks
  APP_ROUTE_WS(server, "/ws/chan_check", rpc::ChanCheck, options_.sacn_address);

  // Serve Web UI files.
  CROW_ROUTE(server, "/").methods(crow::HTTPMethod::Get)(&RedirectToIndex);
  CROW_ROUTE(server, "/<path>").methods(crow::HTTPMethod::Get)(
      [](const crow::request &req, const std::string &file_path_partial) {
        crow::response resp;
        std::filesystem::path static_file_path;
        if (std::filesystem::path(file_path_partial).has_extension()) {
          // Try a real file.
          static_file_path = GetWebUiRoot() / file_path_partial;
        } else {
          // Serve index.html to enable client-side routing.
          // See https://create-react-app.dev/docs/deployment#serving-apps-with-client-side-routing
          static_file_path = GetWebUiRoot() / "index.html";
        }

        if (!FilePathInWebroot(static_file_path)) {
          // File is not in the webroot, so not allowed.
          resp.clear();
          resp.code = crow::status::NOT_FOUND;
        } else {
          resp.set_static_file_info_unsafe(static_file_path.string());
        }

        return resp;
      });

  server.run();
}

std::filesystem::path HttpServer::GetWebUiRoot() {
  static std::optional<std::filesystem::path> web_ui_root;
  if (!web_ui_root.has_value()) {
    const auto program_location = boost::dll::program_location().parent_path();
    web_ui_root = (program_location.parent_path() / config::kWebPath).string();
    web_ui_root = std::filesystem::canonical(*web_ui_root);
  }

  return *web_ui_root;
}

crow::response HttpServer::RedirectToIndex() {
  crow::response resp;
  resp.redirect("/index.html");

  return resp;
}

bool HttpServer::FilePathInWebroot(const std::filesystem::path &path) {
  const auto abs_path = std::filesystem::weakly_canonical(path);
  const std::filesystem::path webroot = GetWebUiRoot();

  const auto mismatch = std::mismatch(webroot.begin(), webroot.end(), abs_path.begin(), abs_path.end());
  return mismatch.first == webroot.end();
}

} // mobilesacn
