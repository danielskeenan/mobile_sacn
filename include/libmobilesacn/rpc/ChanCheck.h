/**
 * @file ChanCheck.h
 *
 * @author Dan Keenan
 * @date 4/2/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CHANCHECK_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CHANCHECK_H_

#include "RpcHandler.h"
#include "libmobilesacn/sacn.h"
#include "libmobilesacn/fx/Effect.h"
#include "proto/effect.pb.h"
#include <sacn/cpp/source.h>
#include <crow/websocket.h>

namespace mobilesacn::rpc {

/**
 * Channel check handler
 */
class ChanCheck : public RpcHandler {
 public:
  explicit ChanCheck(const etcpal::IpAddr &sacn_address) : sacn_address_(sacn_address) {
    effect_settings_.add_addresses(addr_);
  }

  void HandleWsOpen(crow::websocket::connection &conn) override;
  void HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) override;
  void HandleWsClose(crow::websocket::connection *conn, const std::string &reason) override;

 private:
  constexpr static const auto kIdentifier = "Chan Check";

  std::unique_ptr<sacn::Source, SacnSourceDeleter> sacn_transmitter_;
  etcpal::IpAddr sacn_address_;
  bool transmitting_ = false;
  unsigned int priority_ = 100;
  unsigned int univ_ = 1;
  unsigned int addr_ = 1;
  uint8_t level_ = 255;
  bool per_address_priority_ = false;
  EffectSettings effect_settings_;
  std::unique_ptr<fx::Effect> effect_;
  DmxBuffer buf_{0};
  DmxBuffer priorities_{0};

  void SendCurrentState(crow::websocket::connection &conn) const;
};

} // mobilesacn::rpc

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CHANCHECK_H_
