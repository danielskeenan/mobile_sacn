/**
 * @file RpcHandler.h
 *
 * @author Dan Keenan
 * @date 4/2/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_RPCHANDLER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_RPCHANDLER_H_

#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/websocket.h>

namespace mobilesacn::rpc {

/**
 * Base class for RPC method handlers.
 */
class RpcHandler {
 public:
  virtual void HandleWsOpen(crow::websocket::connection &conn) {}

  virtual void HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) {}

  virtual void HandleWsError(crow::websocket::connection &conn) {}

  virtual void HandleWsClose(crow::websocket::connection &conn, const std::string &reason) {}
};

} // mobilesacn::rpc

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_RPCHANDLER_H_
