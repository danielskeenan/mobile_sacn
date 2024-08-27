/**
 * @file CrowLogHandler.h
 *
 * @author Dan Keenan
 * @date 8/27/24
 * @copyright GNU GPLv3
 */

#ifndef CROWLOGHANDLER_H
#define CROWLOGHANDLER_H
#include <crow/logging.h>

namespace mobilesacn {

class CrowLogHandler : public crow::ILogHandler
{
public:
    void log(std::string message, crow::LogLevel crowLogLevel) override;
};

} // mobilesacn

#endif //CROWLOGHANDLER_H
