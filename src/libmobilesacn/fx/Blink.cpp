/**
 * @file Blink.cpp
 *
 * @author Dan Keenan
 * @date 5/4/22
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/fx/Blink.h>
#include <spdlog/spdlog.h>

namespace mobilesacn::fx {

void Blink::Tick() noexcept {
  for (const auto addr : GetAddresses()) {
    auto &buf_level = effect_buf_[addr - 1];
    if (buf_level == 0) {
      buf_level = level_;
    } else {
      buf_level = 0;
    }
  }
}

void Blink::DoUpdateFromProtobufMessage(const EffectSettings &effect_settings) {
  if (!effect_settings.has_blink_settings()) {
    spdlog::warn("Tried to update blink with wrong settings payload");
    return;
  }
  const auto new_level = effect_settings.blink_settings().level();
  if (new_level >= kLevelMin && new_level <= kLevelMax) {
    level_ = new_level;
  }
}

} // mobilesacn::fx
