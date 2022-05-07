/**
 * @file Effect.cpp
 *
 * @author Dan Keenan
 * @date 5/7/22
 * @copyright GNU GPLv3
 */

#include <spdlog/spdlog.h>
#include "libmobilesacn/fx/Effect.h"

namespace mobilesacn::fx {

void Effect::UpdateFromProtobufMessage(const EffectSettings &effect_settings) {
  const decltype(Effect::addresses_) new_addresses(effect_settings.addresses().cbegin(),
                                                   effect_settings.addresses().cend());
  if (new_addresses != addresses_) {
    // Addresses changed.
    addresses_ = new_addresses;
    AddressesChanged();
  }
  duration_ = std::chrono::milliseconds(effect_settings.duration_ms());

  DoUpdateFromProtobufMessage(effect_settings);
}

void Effect::Start() {
  if (runner_.joinable()) {
    Stop();
  }
  spdlog::trace("Starting effect");
  stop_ = false;
  runner_ = std::thread(&Effect::Run, this);
}

void Effect::Stop() {
  spdlog::trace("Stopping effect");
  stop_ = true;
  // Wait for completion.
  if (runner_.joinable()) {
    runner_.join();
  }
  spdlog::trace("Stopped");
}

void Effect::Run() noexcept {
  while (!stop_) {
    Tick();
    HtpMerge();
    ProcessBuf(output_buf_);
    std::this_thread::sleep_for(duration_);
  }
  // Restore old levels
  ProcessBuf(regular_buf_);
}

void Effect::ProcessBuf(const DmxBuffer &buf) const {
  if (update_pap_) {
    if (per_address_priority_) {
      sacn_transmitter_->UpdateLevelsAndPap(
          univ_, buf.data(), buf.size(),
          priorities_.data(), priorities_.size()
      );
    } else {
      sacn_transmitter_->UpdateLevelsAndPap(
          univ_, buf.data(), buf.size(),
          nullptr, 0
      );
    }
    update_pap_ = false;
  } else {
    sacn_transmitter_->UpdateLevels(univ_, buf.data(), buf.size());
  }
}

} // mobilesacn::fx
