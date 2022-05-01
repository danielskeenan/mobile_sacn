/**
 * @file HttpReasons.h
 *
 * @author Dan Keenan
 * @date 5/1/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPREASONS_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPREASONS_H_

namespace mobilesacn {

namespace DisconnectReason {
static const auto Default = "quit";
static const auto ClientDisconnected = "client_disconnected";
} // DisconnectReason

} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPREASONS_H_
