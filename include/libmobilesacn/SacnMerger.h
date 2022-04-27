/**
 * @file SacnMerger.h
 *
 * @author Dan Keenan
 * @date 4/23/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACNMERGER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACNMERGER_H_

#include "sacn.h"
#include <sacn/cpp/dmx_merger.h>
#include <array>
#include <boost/bimap.hpp>

namespace mobilesacn {

/**
 * Wrapper around sacn::DmxMerger to keep the level buffers together for the lifetime of the merger.
 */
class SacnMerger {
 public:
  explicit SacnMerger();

  void Reset();

  inline sacn_dmx_merger_source_t AddSource(EtcPalUuid source_cid) {
    const auto handle_it = cid_handle_map_.left.find(source_cid);
    if (handle_it != cid_handle_map_.left.end()) {
      // Already exists.
      return handle_it->second;
    }
    const auto handle = merger_->AddSource().value();
    cid_handle_map_.insert({source_cid, handle});
    return handle;
  }

  inline void RemoveSource(EtcPalUuid source_cid) {
    const auto handle = cid_handle_map_.left.find(source_cid);
    if (handle != cid_handle_map_.left.end()) {
      merger_->RemoveSource(handle->second);
      cid_handle_map_.left.erase(handle);
    }
  }

  void HandleReceivedSacn(const SacnRemoteSource &source_info, const SacnRecvUniverseData &universe_data);

  inline void UpdateLevels(EtcPalUuid source_cid, const uint8_t *new_levels, size_t new_levels_count) {
    const auto handle = cid_handle_map_.left.find(source_cid);
    if (handle == cid_handle_map_.left.end()) {
      // We don't know about this source.
      return;
    }
    merger_->UpdateLevels(handle->second, new_levels, new_levels_count);
  }

  inline void UpdateUniversePriority(EtcPalUuid source_cid, uint8_t universe_priority) {
    const auto handle = cid_handle_map_.left.find(source_cid);
    if (handle == cid_handle_map_.left.end()) {
      // We don't know about this source.
      return;
    }
    merger_->UpdateUniversePriority(handle->second, universe_priority);
  }

  [[nodiscard]] const std::array<uint8_t, DMX_ADDRESS_COUNT> &GetBuf() const {
    return buf_;
  }

  /**
   * Get a list of winning CIDs for each address.
   * @return
   */
  [[nodiscard]] std::array<std::string, DMX_ADDRESS_COUNT> GetOwnerCids() const;

 private:
  std::array<uint8_t, DMX_ADDRESS_COUNT> buf_{0};
  std::array<sacn_dmx_merger_source_t, DMX_ADDRESS_COUNT> owners_{0};
  std::array<uint8_t, DMX_ADDRESS_COUNT> per_address_priorities_{0};
  boost::bimap<EtcPalUuid, sacn_dmx_merger_source_t> cid_handle_map_;
  std::unique_ptr<sacn::DmxMerger, ShutdownDeleter<sacn::DmxMerger>> merger_;
};

} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACNMERGER_H_
