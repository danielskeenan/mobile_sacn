/**
 * @file ChanCheck.cpp
 *
 * @author Dan Keenan
 * @date 4/2/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/rpc/ChanCheck.h"
#include "proto/chan_check.pb.h"
#include "libmobilesacn/fx/EffectFactory.h"
#include <spdlog/spdlog.h>
#include <etcpal/uuid.h>
#include <google/protobuf/util/message_differencer.h>

using google::protobuf::util::MessageDifferencer;

namespace mobilesacn::rpc {

void ChanCheck::HandleWsOpen(crow::websocket::connection &conn) {
  if (!sacn_transmitter_) {
    sacn_transmitter_ = GetSacnTransmitter(sacn_address_, kIdentifier, conn.get_remote_ip());
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

  // Use the chan check address for any effects.
  req.mutable_effect()->clear_addresses();
  req.mutable_effect()->add_addresses(req.address());
  const bool stop_transmitting = !req.transmit() && transmitting_;
  const bool start_transmitting = req.transmit() && !transmitting_;
  const bool change_univ = (req.universe() != univ_ || start_transmitting) && req.transmit();
  const bool change_addr = (req.address() != addr_ || change_univ || start_transmitting) && req.transmit();
  const bool change_priority = (req.priority() != priority_ || req.per_address_priority() != per_address_priority_
      || (req.per_address_priority() && change_addr) || start_transmitting) && req.transmit();
  const bool change_level = (req.level() != level_ || start_transmitting) && req.transmit();
  const bool change_effect = (!MessageDifferencer::Equals(req.effect(), effect_settings_)
      || change_univ || start_transmitting) && req.transmit();
  // If there is an effect, it will manage sending level data.
  const bool send_levels = req.effect().type() == EffectType::NONE;
  if (!sacn_transmitter_) {
    sacn_transmitter_ = GetSacnTransmitter(sacn_address_, kIdentifier, conn.get_remote_ip());
  }

  if (stop_transmitting) {
    spdlog::debug("{} stopped transmitting on univ {}", kIdentifier, univ_);
    sacn_transmitter_->RemoveUniverse(univ_);
  }

  if (change_univ) {
    spdlog::debug("{} started transmitting on univ {} with priority {}", kIdentifier, univ_, req.priority());
    sacn_transmitter_->RemoveUniverse(univ_);
    sacn::Source::UniverseSettings univ_config(req.universe());
    univ_config.priority = req.priority();
    auto sacn_interfaces = GetMulticastInterfacesForAddress(sacn_address_);
    sacn_transmitter_->AddUniverse(univ_config, sacn_interfaces);
    buf_.fill(0);
  }

  if (change_priority) {
    spdlog::debug("{} changed univ {} priority to {}", kIdentifier, univ_, req.priority());
    sacn_transmitter_->ChangePriority(req.universe(), req.priority());
    // Per-address-priority is updated below, with the level change.
  }

  if (change_addr) {
    buf_[addr_ - 1] = 0;
    priorities_[addr_ - 1] = 0;
    // The new level is set below.
  }

  if (change_level || change_addr || change_priority || change_effect) {
    spdlog::debug("{} {}/{} @ {} (pri {})",
                  kIdentifier,
                  univ_,
                  req.address(),
                  req.level(),
                  req.per_address_priority() ? fmt::to_string(req.priority()) : "X");
    buf_[req.address() - 1] = req.level();
    priorities_[req.address() - 1] = req.priority();
    // This is optimized a bit to only update the per-address-priority when absolutely necessary as that causes an
    // increase in network traffic.
    if (!change_priority) {
      // Priority hasn't changed OR per-address-priority not in use.
      if (send_levels) {
        spdlog::trace("{} setting univ {}:\n{}", kIdentifier, req.universe(), fmt::join(buf_, ", "));
        sacn_transmitter_->UpdateLevels(req.universe(), buf_.data(), buf_.size());
      }
    } else {
      if (req.per_address_priority()) {
        // Priority or address has changed necessitating a change in per-address-priority.
        if (send_levels) {
          spdlog::trace("{} setting univ {}:\n{}\npri:\n{}",
                        kIdentifier,
                        req.universe(),
                        fmt::join(buf_, ", "),
                        fmt::join(priorities_, ", "));
          sacn_transmitter_->UpdateLevelsAndPap(req.universe(),
                                                buf_.data(), buf_.size(),
                                                priorities_.data(), priorities_.size());
        }
        if (effect_) {
          effect_->SetPerAddressPriority(true);
        }
      } else {
        // Stop using per-address-priority.
        if (send_levels) {
          spdlog::trace("{} setting univ {}:\n{}\npri: none",
                        kIdentifier,
                        req.universe(),
                        fmt::join(buf_, ", "));
          sacn_transmitter_->UpdateLevelsAndPap(req.universe(),
                                                buf_.data(), buf_.size(),
                                                nullptr, 0);
        }
        if (effect_) {
          effect_->SetPerAddressPriority(false);
        }
      }
    }
  }

  if (change_effect) {
    const bool change_effect_type = req.effect().type() != effect_settings_.type();
    if (change_effect_type || change_univ) {
      if (effect_) {
        effect_->Stop();
      }
      effect_ = fx::CreateEffect(req.effect(), sacn_transmitter_.get(), univ_, buf_, priorities_);
    }
    if (effect_) {
      effect_->UpdateFromProtobufMessage(req.effect());
      effect_->SetPerAddressPriority(req.per_address_priority());
    }
  }
  if (effect_) {
    if (!effect_->IsRunning() && req.transmit()) {
      effect_->Start();
    }
    if (effect_->IsRunning() && !req.transmit()) {
      effect_->Stop();
    }
  }

  transmitting_ = req.transmit();
  priority_ = req.priority();
  per_address_priority_ = req.per_address_priority();
  univ_ = req.universe();
  addr_ = req.address();
  level_ = static_cast<uint8_t>(req.level());
  effect_settings_ = req.effect();
  SendCurrentState(conn);
}

void ChanCheck::HandleWsClose(crow::websocket::connection *conn, const std::string &reason) {
  effect_.reset();
  sacn_transmitter_.reset();
}

void ChanCheck::SendCurrentState(crow::websocket::connection &conn) const {
  ChanCheckRes msg;
  msg.set_transmitting(transmitting_);
  msg.set_priority(priority_);
  msg.set_universe(univ_);
  msg.set_address(addr_);
  msg.set_level(level_);
  msg.set_per_address_priority(per_address_priority_);
  msg.mutable_effect()->CopyFrom(effect_settings_);
  spdlog::trace("{} current state: {}", kIdentifier, msg.ShortDebugString());

  conn.send_binary(msg.SerializeAsString());
}

} // mobilesacn::rpc
