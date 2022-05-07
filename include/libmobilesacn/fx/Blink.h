/**
 * @file Blink.h
 *
 * @author Dan Keenan
 * @date 5/4/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_BLINK_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_BLINK_H_

#include "Effect.h"

namespace mobilesacn::fx {

/**
 * Blink on/off
 */
class Blink : public Effect {
 public:
  using Effect::Effect;

  [[nodiscard]] uint8_t GetLevel() const {
    return level_;
  }

  void SetLevel(uint8_t level) {
    level_ = level;
  }

 protected:
  void Tick() noexcept override;

  void AddressesChanged() override {
    effect_buf_.fill(0);
  }

 private:
  uint8_t level_ = 255;
};

} // mobilesacn::fx

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_BLINK_H_
