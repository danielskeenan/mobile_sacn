/**
 * @file TransmitLevels.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_TRANSMITLEVELS_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_TRANSMITLEVELS_H

#include "TransmitHandler.h"

namespace mobilesacn::handler {

/**
 * Handler for Transmit Levels.
 */
class TransmitLevels final : public TransmitHandler {
    Q_OBJECT

  public:
    using TransmitHandler::TransmitHandler;
    static constexpr auto kProtocol = "TransmitLevels";

    [[nodiscard]] const char *getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Transmit"); }

protected Q_SLOTS:
    void onBinaryMessage(const QByteArray &data) override;

private:
    void onChangeLevels(const uint8_t *levelsData);
};

} // namespace mobilesacn::handler

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_TRANSMITLEVELS_H
