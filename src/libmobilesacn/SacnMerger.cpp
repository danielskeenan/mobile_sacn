/**
 * @file SacnMerger.cpp
 *
 * @author Dan Keenan
 * @date 4/23/22
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/SacnMerger.h>

namespace mobilesacn {

SacnMerger::SacnMerger() : merger_(new sacn::DmxMerger) {
  sacn::DmxMerger::Settings settings(buf_.data());
  settings.owners = owners_.data();
  settings.per_address_priorities = per_address_priorities_.data();
  merger_->Startup(settings);
}

void SacnMerger::HandleReceivedSacn(const SacnRemoteSource &source_info, const SacnRecvUniverseData &universe_data) {
  // Add source checks for existence, so we don't need to check twice.
  const auto handle = AddSource(source_info.cid);
  merger_->UpdateLevels(handle, universe_data.values, universe_data.slot_range.address_count);
  merger_->UpdateUniversePriority(handle, universe_data.priority);
}

std::array<std::string, DMX_ADDRESS_COUNT> SacnMerger::GetOwnerCids() const {
  std::array<std::string, DMX_ADDRESS_COUNT> owner_cids{};
  for (unsigned int ix = 0; ix < DMX_ADDRESS_COUNT; ++ix) {
    const auto handle = owners_[ix];
    if (per_address_priorities_[ix] == 0) {
      // No owner.
      continue;
    }

    const auto cid = cid_handle_map_.right.find(handle);
    if (cid == cid_handle_map_.right.end()) {
      // No owner.
      continue;
    }

    owner_cids[ix] = etcpal::Uuid(cid->get_left()).ToString();
  }

  return owner_cids;
}

} // mobilesacn
