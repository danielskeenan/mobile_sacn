/**
 * @file NetIntInfo.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_ETCPAL_NETINT_NETINTINFO_H_
#define MOBILE_SACN_INCLUDE_ETCPAL_NETINT_NETINTINFO_H_

#include <etcpal/netint.h>
#include <etcpal/cpp/inet.h>
#include <optional>

namespace etcpal_netint {

class NetIntInfo {
 public:
  explicit NetIntInfo(const EtcPalNetintInfo &other) :
      index_{other.index},
      addr_(other.addr),
      mask_(other.mask),
      mac_(other.mac),
      id_(other.id),
      friendly_name_(other.friendly_name),
      is_default_(other.is_default) {}

  [[nodiscard]] unsigned int GetIndex() const {
    return index_;
  }

  [[nodiscard]] const etcpal::IpAddr &GetAddr() const {
    return addr_;
  }

  [[nodiscard]] const etcpal::IpAddr &GetMask() const {
    return mask_;
  }

  [[nodiscard]] const etcpal::MacAddr &GetMax() const {
    return mac_;
  }

  [[nodiscard]] const std::string &GetId() const {
    return id_;
  }

  [[nodiscard]] const std::string &GetFriendlyName() const {
    return friendly_name_;
  }

  [[nodiscard]] bool IsDefault() const {
    return is_default_;
  }

  [[nodiscard]] bool IsUp() const {
#ifdef PLATFORM_WINDOWS
    etcpal_netint_refresh_interfaces(nullptr);
#endif
    return etcpal_netint_is_up(index_);
  }

 private:
  unsigned int index_;
  etcpal::IpAddr addr_;
  etcpal::IpAddr mask_;
  etcpal::MacAddr mac_;
  std::string id_;
  std::string friendly_name_;
  bool is_default_;
};

[[nodiscard]] inline std::vector<NetIntInfo> GetInterfaces() {
  static std::vector<NetIntInfo> ifaces;
  bool if_list_changed = false;
  etcpal_netint_refresh_interfaces(&if_list_changed);
  if (ifaces.empty() || if_list_changed) {
    ifaces.clear();
    const auto if_count = etcpal_netint_get_num_interfaces();
    const auto if_arr = etcpal_netint_get_interfaces();
    ifaces.reserve(if_count);
    for (size_t i = 0; i < if_count; ++i) {
      ifaces.emplace_back(if_arr[i]);
    }
  }

  return ifaces;
}

[[nodiscard]] inline std::optional<NetIntInfo> GetDefaultInterface(etcpal::IpAddrType ip_type) {
  unsigned int ix;
  const auto res = etcpal_netint_get_default_interface(static_cast<etcpal_iptype_t>(ip_type), &ix);
  if (res == kEtcPalErrOk) {
    const auto ifaces = GetInterfaces();
    const auto found = std::find_if(ifaces.cbegin(), ifaces.cend(), [ix, ip_type](const NetIntInfo &net_int_info) {
      return net_int_info.GetIndex() == ix && net_int_info.GetAddr().type() == ip_type;
    });
    if (found != ifaces.cend()) {
      return *found;
    }
  }
  return {};
}

} // etcpal_netint

#endif //MOBILE_SACN_INCLUDE_ETCPAL_NETINT_NETINTINFO_H_
