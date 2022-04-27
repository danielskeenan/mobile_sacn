/**
 * @file ChanCheckTest.cpp
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "../../common/CrowConnMock.h"
#include "../../common/SacnTest.h"
#include "libmobilesacn/rpc/ChanCheck.h"
#include "proto/chan_check.pb.h"
#include "../../common/SpdLogNotifySink.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Eq;

class ChanCheckTest : public mobilesacn::testing::SacnTest {
};

TEST_F(ChanCheckTest, ChanCheck) {
  // Setup the log observer.
  auto log_msg = "New chan_check connection from 127.0.0.1";
  unsigned int log_count = 0;
  mobilesacn::testing::NotifySinkSt::OnLogCb log_cb = [&log_msg, &log_count](const spdlog::details::log_msg &msg) {
    EXPECT_LT(msg.level, spdlog::level::warn);
    EXPECT_EQ(msg.payload, log_msg);
    ++log_count;
  };
  auto test_sink = std::make_shared<mobilesacn::testing::NotifySinkSt>(log_cb);
  spdlog::default_logger()->sinks() = {test_sink};

  // Setup.
  auto test_univ = 1;
  auto test_priority = 100;
  auto test_transmitting = false;
  auto test_address = 1;
  auto test_level = 255;
  mobilesacn::testing::DmxBuffer test_buf{0};
  // Used to ensure sACN packets have actually been processed.
  std::atomic<unsigned long> sacn_count = 0;
  mobilesacn::testing::TestSacnNotifyHandler sacn_handler(
      [&test_univ, &test_priority, &test_buf, &sacn_count](unsigned int univ,
                                                           unsigned int priority,
                                                           std::array<uint8_t, DMX_ADDRESS_COUNT> buf) {
        EXPECT_EQ(univ, test_univ);
        EXPECT_EQ(priority, test_priority);
        EXPECT_EQ(buf, test_buf);
        ++sacn_count;
      });
  sacn::Receiver::Settings sacn_settings(test_univ);
  sacn::Receiver sacn_receiver;
  sacn_receiver.Startup(sacn_settings, sacn_handler);
  mobilesacn::rpc::ChanCheckReq req;
  mobilesacn::rpc::ChanCheckRes res;

  // Initial state.
  req.set_universe(test_univ);
  res.set_universe(test_univ);
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  req.set_address(test_address);
  res.set_address(test_address);
  req.set_level(test_level);
  res.set_level(test_level);

  // Open connection.
  sacn_handler.ready_for_test = true;
  std::optional<NiceMock<mobilesacn::testing::CrowConnMock>> conn_mock;
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  mobilesacn::rpc::ChanCheck chan_check_handler(etcpal::IpAddr::FromString("127.0.0.1"));
  chan_check_handler.HandleWsOpen(*conn_mock);
  EXPECT_EQ(log_count, 1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // No transmitting has occurred.
  EXPECT_EQ(sacn_count, 0);

  // Start transmitting.
  sacn_handler.ready_for_test = false;
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf[test_address - 1] = test_level;
  auto before_sacn_count = sacn_count.load();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_GT(sacn_count, before_sacn_count);

  // Change address.
  sacn_handler.ready_for_test = false;
  test_address = 2;
  req.set_address(test_address);
  res.set_address(test_address);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_count = sacn_count.load();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_GT(sacn_count, before_sacn_count);

  // Change priority.
  sacn_handler.ready_for_test = false;
  test_priority = 200;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_count = sacn_count.load();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_GT(sacn_count, before_sacn_count);

  // Change universe and address.
  sacn_handler.ready_for_test = false;
  test_univ = 2;
  test_address = 3;
  req.set_universe(test_univ);
  res.set_universe(test_univ);
  req.set_address(test_address);
  res.set_address(test_address);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  // Pay attention to a different universe.
  sacn_receiver.ChangeUniverse(test_univ);
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_count = sacn_count.load();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_GT(sacn_count, before_sacn_count);

  sacn_receiver.Shutdown();
}
