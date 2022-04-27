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
#include <etcpal/netint.h>
#include <sacn/cpp/common.h>
#include <sacn/cpp/receiver.h>
#include <utility>
#include <array>
#include <atomic>

namespace mobilesacn::testing {

using DmxBuffer = std::array<uint8_t, DMX_ADDRESS_COUNT>;

class SacnTest : public ::testing::Test {
 protected:
  void SetUp() override {
    etcpal_init(kEtcPalFeatures);
    sacn::Init();
  }

  void TearDown() override {
    sacn::Deinit();
    etcpal_deinit(kEtcPalFeatures);
  }

 private:
  static const auto kEtcPalFeatures = ETCPAL_FEATURE_NETINTS;
};

/**
 * Sacn notify handler that calls @p on_data whenever new data is received.
 */
class TestSacnNotifyHandler : public sacn::Receiver::NotifyHandler {
 public:
  using DataCb = std::function<void(unsigned int univ, unsigned int priority, DmxBuffer buf)>;

  explicit TestSacnNotifyHandler(DataCb on_data) : on_data_(std::move(on_data)) {}

  /**
   * Set this to `true` when the incoming data is ready for verification.
   *
   * The `on_data_ callback will only be called when this is true!
   */
  std::atomic<bool> ready_for_test = false;

  /** @internal */
  void HandleUniverseData(sacn::Receiver::Handle receiver_handle,
                          const etcpal::SockAddr &source_addr,
                          const SacnRemoteSource &source_info,
                          const SacnRecvUniverseData &universe_data) override {
    if (!ready_for_test) {
      return;
    }

    DmxBuffer buf{0};
    const auto start_address = universe_data.slot_range.start_address;
    for (unsigned int addr = start_address - 1, ix = 0; ix < universe_data.slot_range.address_count; ++addr, ++ix) {
      buf[addr] = universe_data.values[ix];
    }
    on_data_(universe_data.universe_id, universe_data.priority, buf);
  }

  /** @internal */
  void HandleSourcesLost(sacn::Receiver::Handle handle,
                         uint16_t universe,
                         const std::vector<SacnLostSource> &lost_sources) override {
    // Do nothing.
  }

 private:
  DataCb on_data_;
};

} // mobilesacn::testing

#endif //MOBILE_SACN_TEST_COMMON_SACNTEST_H_
