/**
 * @file sacn.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/sacn.h"
#include "etcpal_netint/NetIntInfo.h"

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

} // mobilesacn
