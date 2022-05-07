/**
 * @file sacn.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACN_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACN_H_

#include <sacn/cpp/source.h>
#include <sacn/cpp/receiver.h>
#include "etcpal_netint/NetIntInfo.h"
#include <array>

namespace mobilesacn {

static const auto kSacnMinUniv = 1;
static const auto kSacnMaxUniv = 63999;
static const auto kSacnMinPriority = 0;
static const auto kSacnMaxPriority = 200;
static const auto kDmxMinAddr = 1;
static const auto kDmxMaxAddr = DMX_ADDRESS_COUNT;
static const auto kLevelMin = 0;
static const auto kLevelMax = 255;

/**
 * Deleter that calls `Shutdown()` on the pointer.
 *
 * @tparam T
 */
template<class T>
struct ShutdownDeleter {
  void operator()(T *ptr) {
    ptr->Shutdown();
    delete ptr;
  }
};

using SacnSourceDeleter = ShutdownDeleter<sacn::Source>;
using SacnReceiverDeleter = ShutdownDeleter<sacn::Receiver>;

using DmxBuffer = std::array<uint8_t, DMX_ADDRESS_COUNT>;

std::vector<SacnMcastInterface> GetMulticastInterfacesForAddress(const etcpal::IpAddr &addr);

std::unique_ptr<sacn::Source, SacnSourceDeleter> GetSacnTransmitter(const etcpal::IpAddr &addr,
                                                                    const std::string &name,
                                                                    const std::string &client_ip_addr);

std::unique_ptr<sacn::Receiver, SacnReceiverDeleter> GetSacnReceiver(const etcpal::IpAddr &addr,
                                                                     uint16_t universe,
                                                                     sacn::Receiver::NotifyHandler &notify_handler);
} // mobilesacn

/**
 * Allows hashing of EtcPal UUIDs.
 */
template<>
struct std::hash<EtcPalUuid> {
  std::size_t operator()(const EtcPalUuid &uuid) const noexcept;
};
template<>
struct std::hash<etcpal::Uuid> {
  std::size_t operator()(const etcpal::Uuid &uuid) const noexcept {
    return std::hash<EtcPalUuid>()(uuid.get());
  }
};

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACN_H_
