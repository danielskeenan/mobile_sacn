/**
 * @file ChanCheckTest.cpp
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
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
  // Setup.
  auto test_univ = 1;
  auto test_priority = 100;
  auto test_transmitting = false;
  auto test_address = 1;
  auto test_level = 255;
  mobilesacn::DmxBuffer test_buf{0};
  mobilesacn::DmxBuffer test_priorities{0};
  // Used to ensure sACN packets have actually been processed.
  std::mutex sacn_levels_received_mx;
  std::vector<mobilesacn::DmxBuffer> sacn_levels_received;
  std::mutex sacn_priorities_received_mx;
  std::vector<mobilesacn::DmxBuffer> sacn_priorities_received;
  mobilesacn::testing::TestSacnNotifyHandler sacn_handler(
      [&test_univ, &test_priority, &sacn_levels_received, &sacn_levels_received_mx](unsigned int univ,
                                                                                    unsigned int priority,
                                                                                    mobilesacn::DmxBuffer buf,
                                                                                    unsigned int test_line_num) {
        EXPECT_EQ(univ, test_univ) << "Expected univ " << test_univ << ", got " << univ
                                   << " for line " << test_line_num;
        EXPECT_EQ(priority, test_priority) << "Expected priority " << test_priority << ", got " << priority
                                           << " for line " << test_line_num;
        std::lock_guard sacn_levels_received_lock(sacn_levels_received_mx);
        sacn_levels_received.push_back(buf);
      },
      [&test_univ, &sacn_priorities_received, &sacn_priorities_received_mx](unsigned int univ,
                                                                            mobilesacn::DmxBuffer buf,
                                                                            unsigned int test_line_num) {
        EXPECT_EQ(univ, test_univ) << "Expected univ " << test_univ << ", got " << univ
                                   << " for line " << test_line_num;
        std::lock_guard sacn_priorities_received_lock(sacn_priorities_received_mx);
        sacn_priorities_received.push_back(buf);
      });
  sacn::Receiver::Settings sacn_settings(test_univ);
  sacn::Receiver sacn_receiver;
  sacn_receiver.Startup(sacn_settings, sacn_handler);
  ChanCheckReq req;
  ChanCheckRes res;
  // The expected response will have a default instance of EffectSettings.
  res.mutable_effect()->add_addresses(test_address);

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
  sacn_handler.ReadyForTest(__LINE__);
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
  sacn_handler.NotReadyForTest();
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf[test_address - 1] = test_level;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  auto before_sacn_levels_count = sacn_levels_received.size();
  auto before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change address.
  sacn_handler.NotReadyForTest();
  test_address = 2;
  req.set_address(test_address);
  res.set_address(test_address);
  res.mutable_effect()->set_addresses(0, test_address);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change priority.
  sacn_handler.NotReadyForTest();
  test_priority = 200;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change universe and address.
  sacn_handler.NotReadyForTest();
  test_univ = 2;
  test_address = 3;
  req.set_universe(test_univ);
  res.set_universe(test_univ);
  req.set_address(test_address);
  res.set_address(test_address);
  res.mutable_effect()->set_addresses(0, test_address);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  // Pay attention to a different universe.
  sacn_receiver.ChangeUniverse(test_univ);
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change level.
  sacn_handler.NotReadyForTest();
  test_level = 127;
  req.set_level(test_level);
  res.set_level(test_level);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Enable per-address-priority.
  sacn_handler.NotReadyForTest();
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
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_GT(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change universe and address with PAP enabled.
  sacn_handler.NotReadyForTest();
  test_univ = 1;
  test_address = 1;
  req.set_universe(test_univ);
  res.set_universe(test_univ);
  req.set_address(test_address);
  res.set_address(test_address);
  res.mutable_effect()->set_addresses(0, test_address);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  // Pay attention to a different universe.
  sacn_receiver.ChangeUniverse(test_univ);
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  test_priorities.fill(0);
  test_priorities[test_address - 1] = test_priority;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_GT(sacn_priorities_received.size(), before_sacn_pap_count);

  // Change priority with PAP enabled.
  sacn_handler.NotReadyForTest();
  test_priority = 200;
  req.set_priority(test_priority);
  res.set_priority(test_priority);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  test_priorities.fill(0);
  test_priorities[test_address - 1] = test_priority;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_GT(sacn_priorities_received.size(), before_sacn_pap_count);

  // Disable per-address-priority.
  sacn_handler.NotReadyForTest();
  req.set_per_address_priority(false);
  res.set_per_address_priority(false);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  test_buf.fill(0);
  test_buf[test_address - 1] = test_level;
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Stop transmitting
  sacn_handler.NotReadyForTest();
  test_transmitting = false;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  // Start transmitting, then close client.
  sacn_handler.NotReadyForTest();
  test_transmitting = true;
  req.set_transmit(test_transmitting);
  res.set_transmitting(test_transmitting);
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString()));
  chan_check_handler.HandleWsMessage(*conn_mock, req.SerializeAsString(), true);
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(sacn_levels_received.back(), test_buf);
  EXPECT_GT(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);
  sacn_handler.NotReadyForTest();
  chan_check_handler.HandleWsClose(nullptr, "");
  before_sacn_levels_count = sacn_levels_received.size();
  before_sacn_pap_count = sacn_priorities_received.size();
  sacn_handler.ReadyForTest(__LINE__);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(sacn_levels_received.size(), before_sacn_levels_count);
  EXPECT_EQ(sacn_priorities_received.size(), before_sacn_pap_count);

  sacn_receiver.Shutdown();
}
