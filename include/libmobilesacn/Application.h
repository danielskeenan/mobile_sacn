/**
 * @file Application.h
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_APPLICATION_H
#define MOBILE_SACN_APPLICATION_H

#define CROW_DISABLE_STATIC_DIR

#include "EtcPalLogHandler.h"
#include <memory>
#include <string>
#include <filesystem>
#include <string>
#include <QObject>

namespace mobilesacn {
class HttpServer;

/**
 * Application instance.
 */
class Application : public QObject {
  Q_OBJECT
  public:
    struct Options {
      std::string backend_address;
      std::string sacn_address;
    };

    explicit Application(QObject* parent = nullptr);
    ~Application() override;

    void run(const Options &options);
    void stop();
    [[nodiscard]] std::string getWebUrl() const;

  private:
    etcpal::Logger etcPalLogger_;
    EtcPalLogHandler etcPalLogHandler_;
    HttpServer* httpServer_ = nullptr;
};
} // mobilesacn

#endif //MOBILE_SACN_APPLICATION_H
