/**
 * @file ChanCheck.cpp
 *
 * @author Dan Keenan
 * @date 4/2/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/rpc/ChanCheck.h"
#include "proto/chan_check.pb.h"
#include <spdlog/spdlog.h>
#include "mobilesacn_config.h"
#include <etcpal/uuid.h>

namespace mobilesacn::rpc {

ChanCheck::ChanCheck(const etcpal::IpAddr &sacn_address)
    : sacn_transmitter_(new sacn::Source), sacn_interfaces_(GetMulticastInterfacesForAddress(sacn_address)) {
  const auto cid = etcpal::Uuid::V5(etcpal::Uuid::FromString(mobilesacn::config::kSAcnCid), "ChanCheck");
  sacn::Source::Settings sacn_config(cid, fmt::format("{} (Chan Check)", config::kProjectDisplayName));
  const auto result = sacn_transmitter_->Startup(sacn_config);
  if (!result.IsOk()) {
    spdlog::critical("Error starting the sACN subsystem: {}", result.ToCString());
  }
}

void ChanCheck::HandleWsOpen(crow::websocket::connection &conn) {
  spdlog::info("New chan_check connection from {}", conn.get_remote_ip());
  SendCurrentState(conn);
}

void ChanCheck::HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) {
  ChanCheckReq req;
  req.ParseFromString(message);

  spdlog::info("Received chan_check {}", req.ShortDebugString());

  // Sanity check request.
  if (req.universe() < kSacnMinUniv || req.universe() > kSacnMaxUniv
      || req.address() < kDmxMinAddr || req.address() > kDmxMaxAddr
      || req.priority() < kSacnMinPriority || req.priority() > kSacnMaxPriority) {
    // Do nothing.
    SendCurrentState(conn);
    return;
  }

  const bool stop_transmitting = (!req.transmit() && transmitting_);
  const bool start_transmitting = req.transmit() && !transmitting_;
  const bool change_univ = ((req.universe() != univ_) || start_transmitting) && req.transmit();
  const bool change_priority = ((req.priority() != priority_) || start_transmitting) && req.transmit();
  const bool change_addr = ((req.address() != addr_) || change_univ || start_transmitting) && req.transmit();

  if (stop_transmitting) {
    sacn_transmitter_->RemoveUniverse(univ_);
  }

  if (change_univ) {
    sacn_transmitter_->RemoveUniverse(univ_);
    sacn::Source::UniverseSettings univ_config(req.universe());
    univ_config.priority = req.priority();
    sacn_transmitter_->AddUniverse(univ_config, sacn_interfaces_);
    buf_.fill(0);
  }

  if (change_priority) {
    sacn_transmitter_->ChangePriority(req.universe(), req.priority());
  }

  if (change_addr) {
    buf_[addr_ - 1] = 0;
    buf_[req.address() - 1] = 255;
    sacn_transmitter_->UpdateLevels(req.universe(), buf_.data(), buf_.size());
  }

  transmitting_ = req.transmit();
  priority_ = req.priority();
  univ_ = req.universe();
  addr_ = req.address();
  SendCurrentState(conn);
}

void ChanCheck::SendCurrentState(crow::websocket::connection &conn) const {
  ChanCheckRes msg;
  msg.set_transmitting(transmitting_);
  msg.set_priority(priority_);
  msg.set_universe(univ_);
  msg.set_address(addr_);

  conn.send_binary(msg.SerializeAsString());
}

} // mobilesacn::rpc
