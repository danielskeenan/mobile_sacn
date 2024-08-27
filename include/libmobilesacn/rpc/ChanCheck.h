/**
 * @file ChanCheckHandler.h
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#ifndef CHANCHECKHANDLER_H
#define CHANCHECKHANDLER_H

#include "RpcHandler.h"
#include <QObject>

namespace mobilesacn::rpc {

/**
 * Handler for Channel Check.
 */
class ChanCheck : public RpcHandler
{
    Q_OBJECT

public:
    using RpcHandler::RpcHandler;
    static constexpr auto kProtocol = "ChanCheck";
    [[nodiscard]] const char* getProtocol() override { return kProtocol; }

public Q_SLOTS:
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;

private:
    bool transmitting_ = false;
};

} // mobilesacn::rpc

#endif //CHANCHECKHANDLER_H
