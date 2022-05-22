/**
 * @file BlinkTest.cpp
 *
 * @author Dan Keenan
 * @date 5/4/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../common/SacnTest.h"
#include "libmobilesacn/fx/Blink.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Eq;

class BlinkTest : public mobilesacn::testing::SacnTest {};

/**
 * Amount of time to offset the wait before checking the results.
 *
 * Needed to ensure the check happens at the correct point in the effect.
 */
static const std::chrono::milliseconds kExtraWait(500);

TEST_F(BlinkTest, Run) {
  // Setup.
  unsigned int test_univ = 1;
  unsigned int test_address = 1;
  uint8_t test_level = 255;
  std::chrono::milliseconds test_interval(1000);
  mobilesacn::DmxBuffer regular_buf{0};
  mobilesacn::DmxBuffer expected_buf{0};
  mobilesacn::DmxBuffer priorities{100};
  // Used to ensure sACN packets have actually been processed.
  std::mutex sacn_levels_received_mx;
  std::vector<mobilesacn::DmxBuffer> sacn_levels_received;
  mobilesacn::testing::TestSacnNotifyHandler sacn_handler(
      [&test_univ, &sacn_levels_received, &sacn_levels_received_mx](unsigned int univ,
                                                                    unsigned int priority,
                                                                    mobilesacn::DmxBuffer buf) {
        EXPECT_EQ(univ, test_univ);
        std::lock_guard sacn_levels_received_lock(sacn_levels_received_mx);
        sacn_levels_received.push_back(buf);
      }, {});
  sacn::Receiver::Settings sacn_settings(test_univ);
  sacn::Receiver sacn_receiver;
  sacn_receiver.Startup(sacn_settings, sacn_handler);
  const auto test_cid = etcpal::Uuid::V4();
  const std::string test_source_name = "test_transmitter";
  const sacn::Source::Settings sacn_transmitter_settings(test_cid, test_source_name);
  sacn::Source sacn_transmitter;
  sacn_transmitter.Startup(sacn_transmitter_settings);
  mobilesacn::fx::Blink blink(&sacn_transmitter, test_univ, {test_address}, regular_buf, priorities);

  // Start effect.
  sacn_transmitter.AddUniverse(test_univ);
  blink.SetDuration(test_interval);
  blink.SetLevel(test_level);
  expected_buf[test_address - 1] = test_level;
  sacn_handler.ready_for_test = true;
  blink.Start();
  std::this_thread::sleep_for(test_interval - kExtraWait);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
  sacn_handler.ready_for_test = false;
  expected_buf[test_address - 1] = 0;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);

  // Change interval.
  sacn_handler.ready_for_test = false;
  test_interval = std::chrono::seconds(2);
  blink.SetDuration(test_interval);
  expected_buf[test_address - 1] = test_level;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
  sacn_handler.ready_for_test = false;
  expected_buf[test_address - 1] = 0;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);

  // Change level.
  sacn_handler.ready_for_test = false;
  test_level = 127;
  blink.SetLevel(test_level);
  expected_buf[test_address - 1] = test_level;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
  sacn_handler.ready_for_test = false;
  expected_buf[test_address - 1] = 0;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);

  // Change address.
  sacn_handler.ready_for_test = false;
  test_address = 2;
  blink.SetAddresses({test_address});
  expected_buf.fill(0);
  expected_buf[test_address - 1] = test_level;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
  sacn_handler.ready_for_test = false;
  expected_buf[test_address - 1] = 0;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);

  // Check merge with other levels.
  sacn_handler.ready_for_test = false;
  regular_buf.fill(31);
  expected_buf = regular_buf;
  expected_buf[test_address - 1] = test_level;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
  sacn_handler.ready_for_test = false;
  expected_buf = regular_buf;
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);

  // Stop effect
  sacn_handler.ready_for_test = false;
  blink.Stop();
  sacn_handler.ready_for_test = true;
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
  std::this_thread::sleep_for(test_interval);
  EXPECT_EQ(sacn_levels_received.back(), expected_buf);
}
