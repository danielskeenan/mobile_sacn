/**
 * @file ChanCheck.h
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#ifndef CHANCHECK_H
#define CHANCHECK_H

#include "RpcHandler.h"
#include <sacn/cpp/source.h>

#include "TransmitHandler.h"

namespace mobilesacn::rpc {

/**
 * Handler for Channel Check.
 */
class ChanCheck final : public TransmitHandler
{
    Q_OBJECT

public:
    using TransmitHandler::TransmitHandler;
    static constexpr auto kProtocol = "ChanCheck";
    [[nodiscard]] const char* getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Chan Check"); }

public Q_SLOTS:
    void handleConnected() override;
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;

protected:
    void onChangePriority(uint8_t priority) override;
    void startTransmitting() override;

private:
    uint16_t address_ = 1;
    uint8_t level_ = 0;

    void onChangeAddress(uint16_t useAddress);
    void onChangeLevel(uint8_t useLevel);
    void updateLevelBuf();
    void updatePapBuf();
};

} // mobilesacn::rpc

#endif //CHANCHECK_H
