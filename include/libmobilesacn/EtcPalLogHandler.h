/**
 * @file EtcPalLogHandler.h
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_ETCPALLOGHANDLER_H
#define MOBILE_SACN_ETCPALLOGHANDLER_H

#include <etcpal/cpp/log.h>

namespace mobilesacn {

/**
 * Connect EtcPal's logging facility to ours.
 */
class EtcPalLogHandler : public etcpal::LogMessageHandler {
 public:
  void HandleLogMessage(const EtcPalLogStrings &strings) override;
};

} // mobilesacn

#endif //MOBILE_SACN_ETCPALLOGHANDLER_H
