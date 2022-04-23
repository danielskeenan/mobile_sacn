/**
 * @file sacn.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include <spdlog/spdlog.h>
#include "libmobilesacn/sacn.h"
#include "etcpal_netint/NetIntInfo.h"
#include "mobilesacn_config.h"
#include <boost/container_hash/hash.hpp>

namespace mobilesacn {

std::vector<SacnMcastInterface> GetMulticastInterfacesForAddress(const etcpal::IpAddr &addr) {
  std::vector<SacnMcastInterface> ifaces;
  for (const auto &iface : etcpal_netint::GetInterfaces()) {
    if (iface.GetAddr() == addr) {
      const EtcPalMcastNetintId mcast_netint_id{
          .ip_type = static_cast<etcpal_iptype_t>(iface.GetAddr().type()),
          .index = iface.GetIndex(),
      };
      const SacnMcastInterface mcast_iface{
          .iface = mcast_netint_id,
          .status = iface.IsUp() ? kEtcPalErrOk : kEtcPalErrNotConn,
      };
      ifaces.push_back(mcast_iface);
    }
  }

  return ifaces;
}

std::unique_ptr<sacn::Source, SacnSourceDeleter> GetSacnTransmitter(const etcpal::IpAddr &addr,
                                                                    const std::string &name,
                                                                    const std::string &client_ip_addr) {
  const auto multicast_ifaces = GetMulticastInterfacesForAddress(addr);
  std::unique_ptr<sacn::Source, SacnSourceDeleter> sacn(new sacn::Source);

  // Find the mac address for use in generating the transmitter CID.
  const auto mac_address = [&multicast_ifaces]() -> std::array<uint8_t, 6> {
    for (const auto &sacn_iface : multicast_ifaces) {
      const auto iface = etcpal_netint::GetInterface(sacn_iface.iface.index);
      if (!iface.has_value()) {
        continue;
      }
      return iface->GetMac().ToArray();
    }
    spdlog::warn("Could not find valid MAC Address for sACN interface");
    return {0};
  }();
  const auto ip_addr = etcpal::IpAddr::FromString(client_ip_addr);
  const auto cid = etcpal::Uuid::Device(name, mac_address, ip_addr.v4_data());

  sacn::Source::Settings sacn_config(cid, fmt::format("{} ({})", config::kProjectDisplayName, name));
  const auto result = sacn->Startup(sacn_config);
  if (!result.IsOk()) {
    spdlog::critical("Error starting the sACN transmitter: {}", result.ToCString());
  }

  return sacn;
}

std::unique_ptr<sacn::Receiver, SacnReceiverDeleter> GetSacnReceiver(const etcpal::IpAddr &addr,
                                                                     uint16_t universe,
                                                                     sacn::Receiver::NotifyHandler &notify_handler) {
  auto multicast_ifaces = GetMulticastInterfacesForAddress(addr);

  std::unique_ptr<sacn::Receiver, SacnReceiverDeleter> sacn(new sacn::Receiver);

  sacn::Receiver::Settings sacn_config(universe);
  const auto result = sacn->Startup(sacn_config, notify_handler, multicast_ifaces);

  if (!result.IsOk()) {
    spdlog::critical("Error starting the sACN receiver: {}", result.ToCString());
  }

  return sacn;
}

} // mobilesacn

std::size_t std::hash<EtcPalUuid>::operator()(const EtcPalUuid &uuid) const noexcept {
  return boost::hash_range(std::cbegin(uuid.data), std::cend(uuid.data));
}
