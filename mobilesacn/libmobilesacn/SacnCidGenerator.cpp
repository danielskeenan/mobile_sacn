/**
 * @file SacnCidGenerator.cpp
 *
 * @author Dan Keenan
 * @date 8/27/24
 * @copyright GNU GPLv3
 */

#include <span>
#include "SacnCidGenerator.h"

namespace mobilesacn {

SacnCidGenerator& SacnCidGenerator::get()
{
    static SacnCidGenerator instance;

    return instance;
}

etcpal::Uuid SacnCidGenerator::cidForProtocolAndClient(const char* protocol,
                                                       const std::string& clientIpAddr)
{
    std::lock_guard macAddrLock(macAddressLock_);
    // Use the client's ip address to ensure a unique CID is generated per-client.
    const uint32_t uuidNum = [&clientIpAddr]()-> uint32_t {
        const auto ipAddr = etcpal::IpAddr::FromString(clientIpAddr.data());
        if (ipAddr.IsV4()) {
            return ipAddr.v4_data();
        }
        if (ipAddr.IsV6()) {
            const std::span<const uint8_t, 16> ipv6Bytes(ipAddr.v6_data(), 16);
            return etcpal_unpack_u32b(ipv6Bytes.last(4).data());
        }

        return 0;
    }();

    return etcpal::Uuid::Device(protocol, macAddress_.data(), uuidNum);
}

void SacnCidGenerator::setMacAddress(etcpal::MacAddr macAddress)
{
    std::scoped_lock lock(macAddressLock_);
    macAddress_ = macAddress;
}

} // mobilesacn
