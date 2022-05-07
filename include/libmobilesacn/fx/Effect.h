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
#include "proto/effect.pb.h"

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
  explicit Effect(sacn::Source *sacn_transmitter,
                  uint8_t univ,
                  const std::vector<unsigned int> &addresses,
                  const DmxBuffer &buf,
                  const DmxBuffer &priorities) :
      sacn_transmitter_(sacn_transmitter),
      univ_(univ),
      addresses_(addresses),
      regular_buf_(buf),
      priorities_(priorities) {}

  /**
   * Create a new effect runner.
   *
   * @param sacn_transmitter
   * @param univ
   * @param effect_settings
   * @param buf
   */
  explicit Effect(sacn::Source *sacn_transmitter,
                  uint8_t univ,
                  const EffectSettings &effect_settings,
                  const DmxBuffer &buf,
                  const DmxBuffer &priorities) :
      sacn_transmitter_(sacn_transmitter),
      univ_(univ),
      addresses_(effect_settings.addresses().cbegin(), effect_settings.addresses().cend()),
      regular_buf_(buf),
      priorities_(priorities) {}

  virtual ~Effect() {
    Stop();
  }

  void SetDuration(const std::chrono::milliseconds &duration) {
    duration_ = duration;
  }

  [[nodiscard]] const std::vector<unsigned int> &GetAddresses() const {
    return addresses_;
  }

  void SetAddresses(const std::vector<unsigned int> &addresses) {
    addresses_ = addresses;
    std::sort(addresses_.begin(), addresses_.end());
    AddressesChanged();
  }

  void SetPerAddressPriority(bool per_address_priority) {
    per_address_priority_ = per_address_priority;
    update_pap_ = true;
  }

  void UpdatePap() {
    update_pap_ = true;
  }

  [[nodiscard]] bool IsRunning() const {
    return !stop_;
  }

  void UpdateFromProtobufMessage(const EffectSettings &effect_settings);

  void Start();

  void Stop();

 protected:
  DmxBuffer effect_buf_{0};

  /**
   * Called every interval to update the effect DMX buffer (effect_buf_).
   */
  virtual void Tick() noexcept = 0;

  /**
   * Called when the address list changes.
   */
  virtual void AddressesChanged() {}

  /**
   * Called when a new protobuf message is passed in.
   * @param effect_settings
   */
  virtual void DoUpdateFromProtobufMessage(const EffectSettings &effect_settings) {}

 private:
  sacn::Source *sacn_transmitter_;
  uint8_t univ_;
  std::chrono::milliseconds duration_{1000};
  std::vector<unsigned int> addresses_;
  std::thread runner_;
  // Effects are created stopped.
  bool stop_ = true;
  /** Holds non-effect data that must be merged with effect data. */
  const DmxBuffer &regular_buf_;
  DmxBuffer output_buf_{0};
  bool per_address_priority_ = false;
  // Flag is set when priorities change.
  mutable bool update_pap_ = false;
  const DmxBuffer &priorities_;

  void Run() noexcept;

  constexpr void HtpMerge() noexcept {
    for (size_t i = 0; i < output_buf_.size(); ++i) {
      output_buf_[i] = std::max(regular_buf_[i], effect_buf_[i]);
    }
  }

  void ProcessBuf(const DmxBuffer &buf) const;
};

} // mobilesacn::fx

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECT_H_
