/**
 * @file Control.cpp
 *
 * @author Dan Keenan
 * @date 4/16/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/rpc/Control.h"
#include <spdlog/spdlog.h>
#include "proto/control.pb.h"

namespace mobilesacn::rpc {

void Control::HandleWsOpen(crow::websocket::connection &conn) {
  spdlog::info("New control connection from {}", conn.get_remote_ip());
  if (!sacn_transmitter_) {
    sacn_transmitter_ = GetSacnTransmitter(sacn_address_, "Control", conn.get_remote_ip());
  }

  SendCurrentState(conn);
}

void Control::HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) {
  ControlReq req;
  if (!req.ParseFromString(message)) {
    return;
  }

  // Sanity check request.
  if (req.universe() < kSacnMinUniv || req.universe() > kSacnMaxUniv
      || req.priority() < kSacnMinPriority || req.priority() > kSacnMaxPriority
      || req.levels_size() != DMX_ADDRESS_COUNT) {
    // Do nothing.
    SendCurrentState(conn);
    return;
  }

  const bool stop_transmitting = (!req.transmit() && transmitting_);
  const bool start_transmitting = req.transmit() && !transmitting_;
  const bool change_univ = (req.universe() != univ_ || start_transmitting) && req.transmit();
  const bool change_priority = (req.priority() != priority_ || start_transmitting) && req.transmit();

  if (stop_transmitting) {
    sacn_transmitter_->RemoveUniverse(univ_);
  }

  if (change_univ) {
    sacn_transmitter_->RemoveUniverse(univ_);
    sacn::Source::UniverseSettings univ_config(req.universe());
    univ_config.priority = req.priority();
    auto sacn_interfaces = GetMulticastInterfacesForAddress(sacn_address_);
    sacn_transmitter_->AddUniverse(univ_config, sacn_interfaces);
  }

  if (change_priority) {
    sacn_transmitter_->ChangePriority(req.universe(), req.priority());
  }

  // Update level buffer.
  std::copy(req.levels().cbegin(), req.levels().cend(), buf_.begin());
  sacn_transmitter_->UpdateLevels(req.universe(), buf_.data(), buf_.size());

  transmitting_ = req.transmit();
  priority_ = req.priority();
  univ_ = req.universe();
  SendCurrentState(conn);
}

void Control::SendCurrentState(crow::websocket::connection &conn) const {
  ControlRes msg;
  msg.set_transmitting(transmitting_);
  msg.set_priority(priority_);
  msg.set_universe(univ_);
  msg.mutable_levels()->Add(buf_.cbegin(), buf_.cend());

  conn.send_binary(msg.SerializeAsString());
}

} // mobilesacn::rpc
