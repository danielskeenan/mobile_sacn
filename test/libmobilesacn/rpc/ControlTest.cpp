/**
 * @file ControlTest.cpp
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
#include "../../common/CrowConnMock.h"
#include "../../common/SacnTest.h"
#include "proto/control.pb.h"
#include "../../common/SpdLogNotifySink.h"
#include "libmobilesacn/rpc/Control.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Eq;

class ControlTest : public mobilesacn::testing::SacnTest {
};

TEST_F(ControlTest, Control) {
  // Setup.
  auto test_transmitting = false;
  auto test_priority = 100;
  auto test_univ = 1;
  mobilesacn::DmxBuffer test_buf{0};
  // Used to ensure sACN packets have actually been processed.
  std::mutex sacn_levels_received_mx;
  std::vector<mobilesacn::DmxBuffer> sacn_levels_received;
  mobilesacn::testing::TestSacnNotifyHandler sacn_handler(
      [&test_univ, &test_priority, &sacn_levels_received, &sacn_levels_received_mx](unsigned int univ,
                                                                                    unsigned int priority,
                                                                                    mobilesacn::DmxBuffer buf) {
        EXPECT_EQ(univ, test_univ);
        EXPECT_EQ(priority, test_priority);
        std::lock_guard sacn_levels_received_lock(sacn_levels_received_mx);
        sacn_levels_received.push_back(buf);
      }, {});
  sacn::Receiver::Settings sacn_settings(test_univ);
  sacn::Receiver sacn_receiver;
  sacn_receiver.Startup(sacn_settings, sacn_handler);
  ControlReq req;
  ControlRes res;

  // Initial state.
  req.set_universe(test_univ);
  res.set_universe(test_univ);
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  req.clear_levels();
  req.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  res.clear_levels();
  res.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());

  // Open connection.
  sacn_handler.ready_for_test = true;
  std::optional<NiceMock<mobilesacn::testing::CrowConnMock>> conn_mock;
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  mobilesacn::rpc::Control control_handler(etcpal::IpAddr::FromString("127.0.0.1"));
  control_handler.HandleWsOpen(*conn_mock);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // No transmitting has occurred.
  EXPECT_EQ(sacn_levels_received.size(), 0);

  // Start transmitting.
  sacn_handler.ready_for_test = false;
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  auto before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_count);

  // Change some levels.
  sacn_handler.ready_for_test = false;
  // Some vague simulation of real levels.
  for (auto addr = 0, level = 0; addr < test_buf.size(); ++addr, ++level) {
    if (level > 255) {
      level = 0;
    }
    test_buf[addr] = level;
  }
  req.clear_levels();
  req.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  res.clear_levels();
  res.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_count);

  // Change priority.
  sacn_handler.ready_for_test = false;
  test_priority = 200;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  // Reverse previous levels.
  for (int addr = test_buf.size() - 1, level = 0; addr >= 0; --addr, ++level) {
    if (level > 255) {
      level = 0;
    }
    test_buf[addr] = level;
  }
  req.clear_levels();
  req.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  res.clear_levels();
  res.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_count);

  // Change universe.
  sacn_handler.ready_for_test = false;
  test_univ = 2;
  req.set_universe(test_univ);
  res.set_universe(test_univ);
  for (auto addr = 0, level = 0; addr < test_buf.size(); ++addr, ++level) {
    if (level > 255) {
      level = 0;
    }
    test_buf[addr] = level;
  }
  req.clear_levels();
  req.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  res.clear_levels();
  res.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  // Pay attention to a different universe.
  sacn_receiver.ChangeUniverse(test_univ);
  before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_count);

  // Stop transmitting
  sacn_handler.ready_for_test = false;
  test_transmitting = false;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_EQ(sacn_levels_received.size(), before_sacn_count);

  // Start transmitting, then close client.
  sacn_handler.ready_for_test = false;
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_count);
  sacn_handler.ready_for_test = false;
  before_sacn_count = sacn_levels_received.size();
  control_handler.HandleWsClose(nullptr, "");
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.size(), before_sacn_count);

  sacn_receiver.Shutdown();
}
