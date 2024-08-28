/**
 * @file SacnCidGenerator.h
 *
 * @author Dan Keenan
 * @date 8/27/24
 * @copyright GNU GPLv3
 */

#ifndef SACNCIDGENERATOR_H
#define SACNCIDGENERATOR_H

#include <mutex>
#include <unordered_map>
#include <etcpal/cpp/inet.h>
#include <etcpal/cpp/uuid.h>

namespace mobilesacn {

class SacnCidGenerator
{
public:
    static SacnCidGenerator& get();

    etcpal::Uuid cidForProtocolAndClient(const char* protocol, const std::string& clientIpAddr);

    void setMacAddress(etcpal::MacAddr macAddress);

private:
    explicit SacnCidGenerator() {}

    std::mutex macAddressLock_;
    etcpal::MacAddr macAddress_;
};

} // mobilesacn

#endif //SACNCIDGENERATOR_H
