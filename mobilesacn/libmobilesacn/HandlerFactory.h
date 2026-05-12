/**
 * @file HandlerFactory.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLERFACTORY_H
#define MOBILESACN_LIBMOBILESACN_HANDLERFACTORY_H

#include "handler/BaseHandler.h"

namespace mobilesacn {
/**
 * Create a handler for the given @p ws.
 *
 * The returned handler will be deleted when @p ws is closed.
 *
 * @param ws Connected websocket.
 * @param parent Parent QObject for created handler.
 * @return A handler object, or nullptr if no handler could be found.
 */
BaseHandler *createWebHandler(QWebSocket *ws, QObject *parent);
} // namespace mobilesacn

#endif //MOBILESACN_LIBMOBILESACN_HANDLERFACTORY_H
