/**
 * @file RpcHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/RpcHandler.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "libmobilesacn/rpc/ChanCheck.h"

namespace mobilesacn::rpc {

RpcHandler::RpcHandler(crow::websocket::connection& ws, QObject* parent)
    : QObject(parent),
      ws_(ws) {}

} // mobilesacn::rpc
