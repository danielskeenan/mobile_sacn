/**
 * @file EtcPalLogHandlerTest.cpp
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
#include "../common/SpdLogNotifySink.h"
#include "etcpal/log.h"
#include "libmobilesacn/EtcPalLogHandler.h"
#include <spdlog/spdlog.h>

using namespace mobilesacn;

TEST(EtcPalLogHandlerTest, Log) {
  // Setup the log observer.
  static const std::string log_msg = "Test Log Message.";
  bool logged = false;
  mobilesacn::testing::NotifySinkSt::OnLogCb log_cb = [&logged](const spdlog::details::log_msg &msg) {
    EXPECT_EQ(std::string_view(msg.payload.data(), msg.payload.size()), log_msg);
    logged = true;
  };
  auto test_sink = std::make_shared<mobilesacn::testing::NotifySinkSt>(log_cb);
  spdlog::default_logger()->sinks() = {test_sink};

  // Setup etcpal.
  etcpal_init(ETCPAL_FEATURE_LOGGING);
  EtcPalLogHandler etc_pal_log_handler;
  etcpal::Logger logger;
  logger.Startup(etc_pal_log_handler);

  logger.Log(ETCPAL_LOG_WARNING, log_msg.c_str());

  logger.Shutdown();
  etcpal_deinit(ETCPAL_FEATURE_LOGGING);

  EXPECT_TRUE(logged);
}
