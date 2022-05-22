/**
 * @file updater_win.cpp
 *
 * Updater functions on Windows using winsparkle (https://github.com/vslavik/winsparkle).
 *
 * @author Dan Keenan
 * @date 5/21/2022
 * @copyright GNU GPLv3
 */

#include <fmt/format.h>
#include "updater.h"
#include "mobilesacn_config.h"
#include <winsparkle.h>
#include <Windows.h>
#include <QApplication>
#include <QResource>

namespace mobilesacn {

void throw_last_error() {
  const auto err = GetLastError();
  PSTR msg;
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, err, 0, (LPSTR) &msg, 100,
                nullptr);
  throw std::runtime_error(msg);
}

std::wstring to_wstring(const char *str) {
  std::wstring wstr;
  int ret = MultiByteToWideChar(CP_UTF8, 0, str, (int) strlen(str), nullptr, 0);
  if (ret <= 0) {
    throw_last_error();
  }
  wstr.resize(ret + 10);
  ret = MultiByteToWideChar(CP_UTF8, 0, str, (int) strlen(str), wstr.data(), static_cast<int>(wstr.size()));
  if (ret <= 0) {
    throw_last_error();
  }

  return wstr;
}

void init_updater() {
  win_sparkle_set_appcast_url(fmt::format("{}/appcast.xml", config::kProjectHomepageUrl).c_str());
  win_sparkle_set_app_details(to_wstring(config::kProjectAuthor).c_str(),
                              to_wstring(config::kProjectDisplayName).c_str(),
                              to_wstring(config::kProjectVersion).c_str());
  win_sparkle_set_registry_path(fmt::format(R"(Software\{}\{}\WinSparkle)",
                                            config::kProjectOrganizationName, config::kProjectName).c_str());
  const QResource dsa_pub_key(":dsa_pub.pem");
  win_sparkle_set_dsa_pub_pem(dsa_pub_key.uncompressedData().data());
  win_sparkle_set_shutdown_request_callback([]() {
    qApp->quit();
  });
  win_sparkle_init();
}

void cleanup_updater() {
  win_sparkle_cleanup();
}

} // mobilesacn
