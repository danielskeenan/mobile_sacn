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

void ChanCheck::HandleWsOpen(crow::websocket::connection &conn) {
  if (!sacn_transmitter_) {
    sacn_transmitter_ = GetSacnTransmitter(sacn_address_, "Chan Check", conn.get_remote_ip());
  }

  SendCurrentState(conn);
}

void ChanCheck::HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) {
  ChanCheckReq req;
  if (!req.ParseFromString(message)) {
    return;
  }

  // Sanity check request.
  if (req.universe() < kSacnMinUniv || req.universe() > kSacnMaxUniv
      || req.address() < kDmxMinAddr || req.address() > kDmxMaxAddr
      || req.priority() < kSacnMinPriority || req.priority() > kSacnMaxPriority
      || req.level() < kLevelMin || req.level() > kLevelMax) {
    // Do nothing.
    SendCurrentState(conn);
    return;
  }

  const bool stop_transmitting = (!req.transmit() && transmitting_);
  const bool start_transmitting = req.transmit() && !transmitting_;
  const bool change_univ = (req.universe() != univ_ || start_transmitting) && req.transmit();
  const bool change_addr = (req.address() != addr_ || change_univ || start_transmitting) && req.transmit();
  const bool change_priority = (req.priority() != priority_ || req.per_address_priority() != per_address_priority_
      || (req.per_address_priority() && change_addr) || start_transmitting) && req.transmit();
  const bool change_level = (req.level() != level_ || start_transmitting) && req.transmit();
  if (!sacn_transmitter_) {
    sacn_transmitter_ = GetSacnTransmitter(sacn_address_, "Chan Check", conn.get_remote_ip());
  }

  if (stop_transmitting) {
    sacn_transmitter_->RemoveUniverse(univ_);
  }

  if (change_univ) {
    sacn_transmitter_->RemoveUniverse(univ_);
    sacn::Source::UniverseSettings univ_config(req.universe());
    univ_config.priority = req.priority();
    auto sacn_interfaces = GetMulticastInterfacesForAddress(sacn_address_);
    sacn_transmitter_->AddUniverse(univ_config, sacn_interfaces);
    buf_.fill(0);
  }

  if (change_priority) {
    sacn_transmitter_->ChangePriority(req.universe(), req.priority());
    // Per-address-priority is updated below, with the level change.
  }

  if (change_addr) {
    buf_[addr_ - 1] = 0;
    priorities_[addr_ - 1] = 0;
    // The new level is set below.
  }

  if (change_level || change_addr || change_priority) {
    buf_[req.address() - 1] = req.level();
    priorities_[req.address() - 1] = req.priority();
    // This is optimized a bit to only update the per-address-priority when absolutely necessary as that causes an
    // increase in network traffic.
    if (!change_priority) {
      // Priority hasn't changed OR per-address-priority not in use.
      sacn_transmitter_->UpdateLevels(req.universe(), buf_.data(), buf_.size());
    } else {
      if (req.per_address_priority()) {
        // Priority or address has changed necessitating a change in per-address-priority.
        sacn_transmitter_->UpdateLevelsAndPap(req.universe(),
                                              buf_.data(), buf_.size(),
                                              priorities_.data(), priorities_.size());
      } else {
        // Stop using per-address-priority.
        sacn_transmitter_->UpdateLevelsAndPap(req.universe(),
                                              buf_.data(), buf_.size(),
                                              nullptr, 0);
      }
    }
  }

  transmitting_ = req.transmit();
  priority_ = req.priority();
  per_address_priority_ = req.per_address_priority();
  univ_ = req.universe();
  addr_ = req.address();
  level_ = static_cast<uint8_t>(req.level());
  SendCurrentState(conn);
}

void ChanCheck::SendCurrentState(crow::websocket::connection &conn) const {
  ChanCheckRes msg;
  msg.set_transmitting(transmitting_);
  msg.set_priority(priority_);
  msg.set_universe(univ_);
  msg.set_address(addr_);
  msg.set_level(level_);
  msg.set_per_address_priority(per_address_priority_);

  conn.send_binary(msg.SerializeAsString());
}

} // mobilesacn::rpc
