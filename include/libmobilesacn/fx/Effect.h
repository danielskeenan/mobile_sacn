/**
 * @file Effect.h
 *
 * @author Dan Keenan
 * @date 5/4/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECT_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECT_H_

#include <libmobilesacn/sacn.h>
#include <optional>
#include <thread>
#include <chrono>

namespace mobilesacn::fx {

/**
 * Base class for effect.
 */
class Effect {
 public:
  /**
   * Create a new effect runner.
   *
   * @param sacn_transmitter
   * @param univ
   * @param buf The effect's values will be HTP merged with this buffer and then transmitted.
   */
  explicit Effect(sacn::Source *sacn_transmitter, uint8_t univ, const DmxBuffer &buf) :
      sacn_transmitter_(sacn_transmitter), univ_(univ), regular_buf_(buf) {}

  virtual ~Effect() {
    Stop();
  }

  [[nodiscard]] const std::chrono::milliseconds &GetInterval() const {
    return interval_;
  }

  void SetInterval(const std::chrono::milliseconds &interval) {
    interval_ = interval;
  }

  void Start() {
    if (runner_.joinable()) {
      Stop();
    }
    stop_ = false;
    runner_ = std::thread(&Effect::Run, this);
  }

  void Stop() {
    stop_ = true;
    // Wait for completion.
    if (runner_.joinable()) {
      runner_.join();
    }
  }

  /**
   * Called every interval to update the effect DMX buffer (effect_buf_).
   */
  virtual void Tick() noexcept = 0;

 protected:
  DmxBuffer effect_buf_{0};

 private:
  sacn::Source *sacn_transmitter_;
  uint8_t univ_;
  std::chrono::milliseconds interval_{1000};
  std::thread runner_;
  bool stop_ = false;
  /** Holds non-effect data that must be merged with effect data. */
  const DmxBuffer &regular_buf_;
  DmxBuffer output_buf_{0};

  void Run() noexcept;

  constexpr void HtpMerge() noexcept {
    for (size_t i = 0; i < output_buf_.size(); ++i) {
      output_buf_[i] = std::max(regular_buf_[i], effect_buf_[i]);
    }
  }
};

} // mobilesacn::fx

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECT_H_
