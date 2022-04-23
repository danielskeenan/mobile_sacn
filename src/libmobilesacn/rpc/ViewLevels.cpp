/**
 * @file ViewLevels.cpp
 *
 * @author Dan Keenan
 * @date 4/23/22
 * @copyright GNU GPLv3
 */

#include <spdlog/spdlog.h>
#include "libmobilesacn/rpc/ViewLevels.h"

namespace mobilesacn::rpc {

ViewLevels::ViewLevels(const etcpal::IpAddr &sacn_address) : sacn_address_(sacn_address) {
  res_.set_universe(kSacnMinUniv);
}

void ViewLevels::HandleWsOpen(crow::websocket::connection &conn) {
  spdlog::info("New view_levels connection from {}", conn.get_remote_ip());
  conn_ = conn;
  if (!sacn_receiver_) {
    sacn_receiver_ = GetSacnReceiver(sacn_address_, res_.universe(), *this);
  }
  SendCurrentState(conn);
}

void ViewLevels::HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) {
  ViewLevelsReq req;
  if (!req.ParseFromString(message)) {
    spdlog::warn("Bad request in view_levels");
    return;
  }
  spdlog::debug("Received {}", req.ShortDebugString());

  // Sanity check request.
  if (req.universe() < kSacnMinUniv || req.universe() > kSacnMaxUniv) {
    spdlog::warn("Out-of-bounds request in view_levels");
    // Do nothing.
    SendCurrentState(conn);
    return;
  }

  const bool change_univ = req.universe() != res_.universe() || !sacn_receiver_;

  if (change_univ) {
    // Reset first to avoid race conditions.
    sacn_receiver_.reset();
    sacn_merger_.Reset();
    res_.Clear();
    sacn_receiver_ = GetSacnReceiver(sacn_address_, req.universe(), *this);
  }

  res_.set_universe(req.universe());
  SendCurrentState(conn);
}

void ViewLevels::HandleWsClose(crow::websocket::connection &conn, const std::string &reason) {
  sacn_receiver_.reset();
  conn_.reset();
}

void ViewLevels::SendCurrentState(crow::websocket::connection &conn) const {
  conn.send_binary(res_.SerializeAsString());
}

void ViewLevels::HandleUniverseData(sacn::Receiver::Handle receiver_handle,
                                    const etcpal::SockAddr &source_addr,
                                    const SacnRemoteSource &source_info,
                                    const SacnRecvUniverseData &universe_data) {
  if (universe_data.universe_id != res_.universe()) {
    // Don't care.
    return;
  }

  auto uuid_string_it = source_uuid_strings_.find(source_info.cid);
  if (uuid_string_it == source_uuid_strings_.end()) {
    // New source.
    const auto inserted = source_uuid_strings_.insert({source_info.cid, etcpal::Uuid(source_info.cid).ToString()});
    uuid_string_it = inserted.first;
    (*res_.mutable_sources())[uuid_string_it->second] = source_info.name;
  }
  const auto &uuid_string = uuid_string_it->second;

  // Update level data.
  sacn_merger_.HandleReceivedSacn(source_info, universe_data);
  // Do a copy here to avoid race conditions.
  const auto levels = sacn_merger_.GetBuf();
  res_.clear_levels();
  res_.mutable_levels()->Reserve(levels.size());
  for (const auto level : levels) {
    res_.add_levels(level);
  }
  // Do a copy here to avoid race conditions.
  const auto owners = sacn_merger_.GetOwnerCids();
  res_.clear_winning_sources();
  res_.mutable_winning_sources()->Reserve(owners.size());
  for (const auto &owner_cid : owners) {
    res_.add_winning_sources(owner_cid);
  }

  if (conn_.has_value()) {
    SendCurrentState(*conn_);
  }
}

void ViewLevels::HandleSourcesLost(sacn::Receiver::Handle handle,
                                   uint16_t universe,
                                   const std::vector<SacnLostSource> &lost_sources) {
  if (universe != res_.universe()) {
    // Don't care.
    return;
  }

  bool source_removed = false;
  for (const auto &lost_source : lost_sources) {
    const auto uuid_string_it = source_uuid_strings_.find(lost_source.cid);
    if (uuid_string_it != source_uuid_strings_.end()) {
      // Remove the source.
      const std::string uuid_string = uuid_string_it->second;
      source_uuid_strings_.erase(uuid_string_it);
      res_.mutable_sources()->erase(uuid_string);
      sacn_merger_.RemoveSource(lost_source.cid);
      source_removed = true;
    }
  }

  if (source_removed && conn_.has_value()) {
    SendCurrentState(*conn_);
  }
}

} // mobilesacn::rpc
