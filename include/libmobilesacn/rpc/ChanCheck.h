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
#include <sacn/cpp/source.h>
#include <crow/websocket.h>

namespace mobilesacn::rpc {

/**
 * Channel check handler
 */
class ChanCheck : public RpcHandler {
 public:
  explicit ChanCheck(const etcpal::IpAddr &sacn_address);

  void HandleWsOpen(crow::websocket::connection &conn) override;
  void HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) override;

 private:
  std::unique_ptr<sacn::Source, SacnSourceDeleter> sacn_transmitter_;
  std::vector<SacnMcastInterface> sacn_interfaces_;
  bool transmitting_ = false;
  unsigned int priority_ = 100;
  unsigned int univ_ = 1;
  unsigned int addr_ = 1;
  std::array<uint8_t, DMX_ADDRESS_COUNT> buf_{0};

  void SendCurrentState(crow::websocket::connection &conn) const;
};

} // mobilesacn::rpc

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_CHANCHECK_H_
