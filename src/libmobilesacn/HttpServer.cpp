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
#include <crow/middlewares/cors.h>
#include <boost/container_hash/hash.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace mobilesacn {

HttpServer::HttpServer(HttpServer::Options options)
    : options_(std::move(options)) {
  crow::logger::setHandler(&crow_log_handler_);
  crow_
      .server_name(config::kProjectName)
      .bindaddr(options_.backend_address)
      .port(options_.backend_port)
      .multithreaded()
      .tick(std::chrono::minutes(1), [this]() {
        CleanupUnusedHandlers();
      });
//  auto &cors = server.get_middleware<crow::CORSHandler>();
//  cors.global()
//      .methods(crow::HTTPMethod::Get);

  // API Hooks
  SetWebsocketHandler<rpc::ChanCheck>(CROW_ROUTE(crow_, "/ws/chan_check").websocket());

  // Serve Web UI files.
  CROW_ROUTE(crow_, "/").methods(crow::HTTPMethod::Get)(&RedirectToIndex);
  CROW_ROUTE(crow_, "/<path>").methods(crow::HTTPMethod::Get)(
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
}

template<>
std::pair<const std::string, HttpServer::Handler> &HttpServer::GetHandler<rpc::ChanCheck>(const std::string &ip_addr) {
  const auto instance_id = fmt::format("ChanCheck_{}", ip_addr);
  auto it =
      handlers_.try_emplace(instance_id, Handler(ip_addr, std::make_unique<rpc::ChanCheck>(options_.sacn_address)))
          .first;
  return *it;
}

void HttpServer::Run() {
  crow_handle_ = crow_.run_async();
}

void HttpServer::Stop() {
  crow_.stop();
  crow_handle_.wait();
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

void HttpServer::CleanupUnusedHandlers() {
  const std::chrono::minutes expires_in(1);
  for (auto it = handlers_.begin(); it != handlers_.end();) {
    if (it->second.GetLastUse() + expires_in < std::chrono::steady_clock::now()) {
      spdlog::info("Closed {} due to inactivity.", it->first);
      it = handlers_.erase(it);
    } else {
      ++it;
    }
  }
}

} // mobilesacn
