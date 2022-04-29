/**
 * @file CrowLogHandlerTest.cpp
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
#include "../common/SpdLogNotifySink.h"
#include "libmobilesacn/CrowLogHandler.h"
#include "crow/app.h"
#include <spdlog/spdlog.h>

using namespace mobilesacn;

TEST(CrowLogHandlerTest, Log) {
  // Setup the log observer.
  static const std::string log_msg = "Test Log Message.";
  bool logged = false;
  mobilesacn::testing::NotifySinkSt::OnLogCb log_cb = [&logged](const spdlog::details::log_msg &msg) {
    EXPECT_EQ(std::string_view(msg.payload.data(), msg.payload.size()), log_msg);
    logged = true;
  };
  auto test_sink = std::make_shared<mobilesacn::testing::NotifySinkSt>(log_cb);
  spdlog::default_logger()->sinks() = {test_sink};

  // Setup crow.
  CrowLogHandler crow_log_handler;
  crow::logger::setHandler(&crow_log_handler);

  CROW_LOG_WARNING << log_msg;

  EXPECT_TRUE(logged);
}
