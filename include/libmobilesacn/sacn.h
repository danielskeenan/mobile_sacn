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
#include "etcpal_netint/NetIntInfo.h"

namespace mobilesacn {

static const auto kSacnMinUniv = 1;
static const auto kSacnMaxUniv = 63999;
static const auto kSacnMinPriority = 0;
static const auto kSacnMaxPriority = 200;
static const auto kDmxMinAddr = 1;
static const auto kDmxMaxAddr = DMX_ADDRESS_COUNT;

struct SacnSourceDeleter {
  void operator()(sacn::Source *ptr) {
    ptr->Shutdown();
    delete ptr;
  }
};

std::vector<SacnMcastInterface> GetMulticastInterfacesForAddress(const etcpal::IpAddr &addr);

} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_SACN_H_
