/**
 * @file ReceiveLevels.h
 *
 * @author Dan Keenan
 * @date 8/30/24
 * @copyright GNU GPLv3
 */

#ifndef RECEIVELEVELS_H
#define RECEIVELEVELS_H

#include "RpcHandler.h"
#include <sacn/cpp/merge_receiver.h>
#include "SubscribableMergeReceiver.h"

namespace mobilesacn::rpc {

/**
 * Handler for Receive Levels.
 */
class ReceiveLevels final : public RpcHandler
{
    Q_OBJECT

public:
    using RpcHandler::RpcHandler;
    ~ReceiveLevels() override;

    static constexpr auto kProtocol = "ReceiveLevels";
    [[nodiscard]] const char* getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Receive Levels"); }

public Q_SLOTS:
    void handleConnected() override;
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;
    void handleClose() override;

private:
    SubscribableMergeReceiver::Ptr receiver_;

    void onChangeUniverse(uint16_t universe);
};
} // mobilesacn::rpc

#endif //RECEIVELEVELS_H
