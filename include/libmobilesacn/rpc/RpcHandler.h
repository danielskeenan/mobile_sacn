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
#include <optional>

namespace mobilesacn::rpc {

/**
 * Base class for RPC method handlers.
 */
class RpcHandler {
 public:
  /**
   * The connection @p conn has been opened.
   * @param conn
   */
  virtual void HandleWsOpen(crow::websocket::connection &conn) {}

  /**
   * A message @p message has been received.
   * @param conn
   * @param message
   * @param is_binary
   */
  virtual void HandleWsMessage(crow::websocket::connection &conn, const std::string &message, bool is_binary) {}

  /**
   * An error @p error has occurred in the connection.
   *
   * @param conn
   */
  virtual void HandleWsError(crow::websocket::connection &conn) {}

  /**
   * The connection @p conn was closed.
   *
   * @param conn May be `nullptr` if the connection was closed by the server after the client has gone away.
   * @param reason
   */
  virtual void HandleWsClose(crow::websocket::connection *conn, const std::string &reason) {}
};

} // mobilesacn::rpc

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_RPC_RPCHANDLER_H_
