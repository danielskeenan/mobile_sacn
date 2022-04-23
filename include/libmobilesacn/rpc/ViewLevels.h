/**
 * @file ViewLevels.h
 *
 * @author Dan Keenan
 * @date 4/23/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_VIEWLEVELS_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_VIEWLEVELS_H_

#include "RpcHandler.h"
#include "libmobilesacn/sacn.h"
#include <sacn/cpp/receiver.h>
#include <crow/websocket.h>
#include "proto/view_levels.pb.h"
#include "libmobilesacn/SacnMerger.h"

namespace mobilesacn::rpc {

/**
 * View Levels handler
 */
class ViewLevels : public RpcHandler, public sacn::Receiver::NotifyHandler {
 public:
  explicit ViewLevels(const etcpal::IpAddr &sacn_address);

  void HandleWsOpen(crow::websocket::connection &conn) override;
  void HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) override;
  void HandleWsClose(crow::websocket::connection &conn, const std::string &reason) override;

  void HandleUniverseData(sacn::Receiver::Handle receiver_handle,
                          const etcpal::SockAddr &source_addr,
                          const SacnRemoteSource &source_info,
                          const SacnRecvUniverseData &universe_data) override;
  void HandleSourcesLost(sacn::Receiver::Handle handle,
                         uint16_t universe,
                         const std::vector<SacnLostSource> &lost_sources) override;

 private:
  std::unique_ptr<sacn::Receiver, SacnReceiverDeleter> sacn_receiver_;
  etcpal::IpAddr sacn_address_;
  SacnMerger sacn_merger_;
  std::unordered_map<EtcPalUuid, std::string> source_uuid_strings_;
  std::optional<std::reference_wrapper<crow::websocket::connection>> conn_;
  ViewLevelsRes res_;

  void SendCurrentState(crow::websocket::connection &conn) const;
};

} // mobilesacn::rpc

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_VIEWLEVELS_H_
