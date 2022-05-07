/**
 * @file Effect.cpp
 *
 * @author Dan Keenan
 * @date 5/7/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/fx/Effect.h"

namespace mobilesacn::fx {

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
