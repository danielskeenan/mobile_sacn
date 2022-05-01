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
using mobilesacn::testing::RecentPacketIs;

class ChanCheckTest : public mobilesacn::testing::SacnTest {
};

TEST_F(ChanCheckTest, ChanCheck) {
  // Setup.
  auto test_univ = 1;
  auto test_priority = 100;
  auto test_transmitting = false;
  auto test_address = 1;
  auto test_level = 255;
  mobilesacn::testing::DmxBuffer test_buf{0};
  mobilesacn::testing::DmxBuffer test_priorities{0};
  // Used to ensure sACN packets have actually been processed.
  std::mutex sacn_levels_received_mx;
  std::vector<mobilesacn::testing::DmxBuffer> sacn_levels_received;
  std::mutex sacn_priorities_received_mx;
  std::vector<mobilesacn::testing::DmxBuffer> sacn_priorities_received;
  mobilesacn::testing::TestSacnNotifyHandler sacn_handler(
      [&test_univ, &test_priority, &sacn_levels_received, &sacn_levels_received_mx](unsigned int univ,
                                                                                    unsigned int priority,
                                                                                    mobilesacn::testing::DmxBuffer buf) {
        EXPECT_EQ(univ, test_univ);
        EXPECT_EQ(priority, test_priority);
        std::lock_guard sacn_levels_received_lock(sacn_levels_received_mx);
        sacn_levels_received.push_back(buf);
      },
      [&test_univ, &sacn_priorities_received, &sacn_priorities_received_mx](unsigned int univ,
                                                                            mobilesacn::testing::DmxBuffer buf) {
        EXPECT_EQ(univ, test_univ);
        std::lock_guard sacn_priorities_received_lock(sacn_priorities_received_mx);
        sacn_priorities_received.push_back(buf);
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
  req.set_per_address_priority(false);
  res.set_per_address_priority(false);
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
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // No transmitting has occurred.
  EXPECT_EQ(sacn_levels_received.size(), 0);
  EXPECT_EQ(sacn_priorities_received.size(), 0);

  // Start transmitting.
  sacn_handler.ready_for_test = false;
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf[test_address - 1] = test_level;
  auto before_sacn_levels_count = sacn_levels_received.size();
  auto before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change address.
  sacn_handler.ready_for_test = false;
  test_address = 2;
  req.set_address(test_address);
  res.set_address(test_address);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change priority.
  sacn_handler.ready_for_test = false;
  test_priority = 200;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

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
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change level.
  sacn_handler.ready_for_test = false;
  test_level = 127;
  req.set_level(test_level);
  res.set_level(test_level);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Enable per-address-priority.
  sacn_handler.ready_for_test = false;
  req.set_per_address_priority(true);
  res.set_per_address_priority(true);
  test_priority = 150;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  test_priorities.fill(0);
  test_priorities[test_address - 1] = test_priority;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_GT(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change universe and address with PAP enabled.
  sacn_handler.ready_for_test = false;
  test_univ = 1;
  test_address = 1;
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
  test_priorities.fill(0);
  test_priorities[test_address - 1] = test_priority;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_GT(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change priority with PAP enabled.
  sacn_handler.ready_for_test = false;
  test_priority = 200;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  test_priorities.fill(0);
  test_priorities[test_address - 1] = test_priority;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_GT(sacn_priorities_received.size(), before_sacn_pap_count);

  // Disable per-address-priority.
  sacn_handler.ready_for_test = false;
  req.set_per_address_priority(false);
  res.set_per_address_priority(false);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Stop transmitting
  sacn_handler.ready_for_test = false;
  test_transmitting = false;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Start transmitting, then close client.
  sacn_handler.ready_for_test = false;
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_THAT(test_buf, RecentPacketIs(sacn_levels_received));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);
  sacn_handler.ready_for_test = false;
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  chan_check_handler.HandleWsClose(nullptr, "");
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  sacn_receiver.Shutdown();
}
