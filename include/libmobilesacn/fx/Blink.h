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

  void Tick() noexcept override;

  [[nodiscard]] unsigned int GetAddr() const {
    return addr_;
  }

  void SetAddr(unsigned int addr) {
    // Turn off old address first.
    effect_buf_[addr_ - 1] = 0;
    addr_ = addr;
  }

  [[nodiscard]] uint8_t GetLevel() const {
    return level_;
  }

  void SetLevel(uint8_t level) {
    level_ = level;
  }

 private:
  unsigned int addr_ = 1;
  uint8_t level_ = 255;
};

} // mobilesacn::fx

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_BLINK_H_
