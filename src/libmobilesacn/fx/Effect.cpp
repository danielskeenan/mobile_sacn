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
  interval_ = std::chrono::milliseconds(effect_settings.duration_ms());

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
    sacn_transmitter_->UpdateLevels(univ_, output_buf_.data(), output_buf_.size());
    std::this_thread::sleep_for(interval_);
  }
  // Restore old levels
  sacn_transmitter_->UpdateLevels(univ_, regular_buf_.data(), regular_buf_.size());
}

} // mobilesacn::fx
