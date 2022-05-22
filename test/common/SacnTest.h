/**
 * @file SacnTest.h
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_TEST_COMMON_SACNTEST_H_
#define MOBILE_SACN_TEST_COMMON_SACNTEST_H_

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <etcpal/netint.h>
#include <sacn/cpp/common.h>
#include <sacn/cpp/receiver.h>
#include <utility>
#include <array>
#include <atomic>
#include <fmt/ostream.h>
#include "libmobilesacn/sacn.h"

namespace mobilesacn::testing {

using DmxBuffer = std::array<uint8_t, DMX_ADDRESS_COUNT>;

class SacnTest : public ::testing::Test {
 protected:
  void SetUp() override {
    etcpal_init(kEtcPalFeatures);
    sacn::Init();
    spdlog::default_logger()->sinks() = {};
  }

  void TearDown() override {
    sacn::Deinit();
    etcpal_deinit(kEtcPalFeatures);
  }

 private:
  static const auto kEtcPalFeatures = ETCPAL_FEATURE_NETINTS;
};

/**
 * sACN notify handler that calls @p on_data whenever new data is received.
 *
 * The callbacks will only be called when ready_for_test is true.  This allows
 * the transmitter to be configured without causing test failures with intermediate
 * data.
 */
class TestSacnNotifyHandler : public sacn::Receiver::NotifyHandler {
 public:
  using DataCb = std::function<void(unsigned int univ,
                                    unsigned int priority,
                                    DmxBuffer buf,
                                    unsigned int test_line_num)>;
  using PriorityCb = std::function<void(unsigned int univ, DmxBuffer buf, unsigned int test_line_num)>;

  explicit TestSacnNotifyHandler(DataCb on_data, PriorityCb on_pap)
      : on_data_(std::move(on_data)), on_pap_(std::move(on_pap)) {}

  /**
   * Call when incoming data is ready for verification.
   * @param line_no The line number to print when expectations fail.
   */
  void ReadyForTest(unsigned int line_no) {
    test_line_num_ = line_no;
  }

  /**
   * Call to stop verifying incoming data.
   */
  void NotReadyForTest() {
    test_line_num_ = 0;
  }

  /** @internal */
  void HandleUniverseData(sacn::Receiver::Handle receiver_handle,
                          const etcpal::SockAddr &source_addr,
                          const SacnRemoteSource &source_info,
                          const SacnRecvUniverseData &universe_data) override {
    if (test_line_num_ == 0) {
      return;
    }

    DmxBuffer buf{0};
    const auto start_address = universe_data.slot_range.start_address;
    for (unsigned int addr = start_address - 1, ix = 0; ix < universe_data.slot_range.address_count; ++addr, ++ix) {
      buf[addr] = universe_data.values[ix];
    }
    if (universe_data.start_code == SACN_STARTCODE_DMX && on_data_) {
      on_data_(universe_data.universe_id, universe_data.priority, buf, test_line_num_);
    } else if (universe_data.start_code == SACN_STARTCODE_PRIORITY && on_pap_) {
      on_pap_(universe_data.universe_id, buf, test_line_num_);
    }
  }

  /** @internal */
  void HandleSourcesLost(sacn::Receiver::Handle handle,
                         uint16_t universe,
                         const std::vector<SacnLostSource> &lost_sources) override {
    // Do nothing.
  }

 private:
  DataCb on_data_;
  PriorityCb on_pap_;
  std::atomic<unsigned int> test_line_num_ = 0;
};

} // mobilesacn::testing

namespace std {
/**
 * Print the DMX buffer as a grid of values.
 * @param buf
 * @param os
 */
inline void PrintTo(const std::array<uint8_t, DMX_ADDRESS_COUNT> &buf, std::ostream *os) {
  for (size_t ix = 0; ix < buf.size(); ++ix) {
    if (ix % 16 == 0) {
      *os << fmt::format("\n{:03}: ", ix + 1);
    }
    *os << fmt::format("{:02X} ", buf[ix]);
  }
}
}

#endif //MOBILE_SACN_TEST_COMMON_SACNTEST_H_
