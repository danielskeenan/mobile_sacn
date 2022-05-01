/**
 * @file Control.h
 *
 * @author Dan Keenan
 * @date 4/16/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CONTROL_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CONTROL_H_

#include <crow/websocket.h>
#include "RpcHandler.h"
#include "etcpal/cpp/inet.h"
#include "sacn/cpp/source.h"
#include "libmobilesacn/sacn.h"

namespace mobilesacn::rpc {

/**
 * Live control handler
 */
class Control : public RpcHandler {
 public:
  explicit Control(const etcpal::IpAddr &sacn_address) : sacn_address_(sacn_address) {}

  void HandleWsOpen(crow::websocket::connection &conn) override;
  void HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) override;
  void HandleWsClose(crow::websocket::connection *conn, const std::string &reason) override;

 private:
  std::unique_ptr<sacn::Source, SacnSourceDeleter> sacn_transmitter_;
  etcpal::IpAddr sacn_address_;
  bool transmitting_ = false;
  unsigned int priority_ = 100;
  unsigned int univ_ = 1;
  std::array<uint8_t, DMX_ADDRESS_COUNT> buf_{0};

  void SendCurrentState(crow::websocket::connection &conn) const;
};

} // mobilesacn::rpc

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CONTROL_H_
