/**
 * @file RpcHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <etcpal/cpp/netint.h>
#include <libmobilesacn/rpc/RpcHandler.h>

#include "libmobilesacn/rpc/ChanCheck.h"

namespace mobilesacn::rpc {

RpcHandler::RpcHandler(crow::websocket::connection& ws, QObject* parent)
    : QObject(parent),
      ws_(ws)
{
    // Needed for all sACN transmitters.
    const auto sacnNetInt = getWsUserData()->sacnNetInt;
    sacnMcastInterfaces_.push_back(SacnMcastInterface{
        .iface = {
            .ip_type = sacnNetInt.addr().raw_type(),
            .index = sacnNetInt.index().value(),
        },
        .status = etcpal::netint::IsUp(sacnNetInt.index()) ? kEtcPalErrOk : kEtcPalErrNotConn
    });
}

} // mobilesacn::rpc
