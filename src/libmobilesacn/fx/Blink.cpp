/**
 * @file Blink.cpp
 *
 * @author Dan Keenan
 * @date 5/4/22
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/fx/Blink.h>

namespace mobilesacn::fx {

void Blink::Tick() noexcept {
  auto &buf_level = effect_buf_[addr_ - 1];
  if (buf_level == 0) {
    buf_level = level_;
  } else {
    buf_level = 0;
  }
}

} // mobilesacn::fx
