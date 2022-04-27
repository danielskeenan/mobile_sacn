/**
 * @file SpdLogNotifySink.h
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_TEST_COMMON_SPDLOGNOTIFYSINK_H_
#define MOBILE_SACN_TEST_COMMON_SPDLOGNOTIFYSINK_H_

#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace mobilesacn::testing {

/**
 * Sink used for testing logging adapters.
 * @tparam Mutex
 */
template<typename Mutex>
class NotifySink : public spdlog::sinks::base_sink<Mutex> {
 public:
  using OnLogCb = std::function<void(const spdlog::details::log_msg &)>;

  explicit NotifySink(OnLogCb log_cb) : log_cb_(std::move(log_cb)) {}

 protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    log_cb_(msg);
  }

  void flush_() override {
    // Do nothing.
  }

 private:
  OnLogCb log_cb_;
};

using NotifySinkMt = NotifySink<std::mutex>;
using NotifySinkSt = NotifySink<spdlog::details::null_mutex>;

} // mobilesacn::testing

#endif //MOBILE_SACN_TEST_COMMON_SPDLOGNOTIFYSINK_H_
