/**
 * @file CrowLogHandler.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_CROWLOGHANDLER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_CROWLOGHANDLER_H_

#include <crow/logging.h>

namespace mobilesacn {

/**
 * Connect Crow's logging facility to ours.
 */
class CrowLogHandler : public crow::ILogHandler {
 public:
  void log(std::string message, crow::LogLevel level) override;
};

} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_CROWLOGHANDLER_H_
