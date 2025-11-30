/**
 * @file RpcHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <etcpal/cpp/netint.h>
#include "RpcHandler.h"

namespace mobilesacn::rpc {

RpcHandler::RpcHandler(crow::websocket::connection& ws)
    : QObject(nullptr),
      ws_(ws) {}

void RpcHandler::sendBinary(const uint8_t* data, const std::size_t size)
{
    auto lock = std::scoped_lock(wsMutex_);
    ws_.send_binary(std::string(reinterpret_cast<const char*>(data), size));
}

} // mobilesacn::rpc
