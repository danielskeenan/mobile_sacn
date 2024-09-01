/**
 * @file TransmitLevels.h
 *
 * @author Dan Keenan
 * @date 8/28/24
 * @copyright GNU GPLv3
 */

#ifndef TRANSMITLEVELS_H
#define TRANSMITLEVELS_H

#include "RpcHandler.h"
#include "TransmitHandler.h"

namespace mobilesacn::rpc {
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

  public Q_SLOTS:
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;

  private:
    void onChangeLevels(const uint8_t *levelsData);
};
} // mobilesacn::rpc

#endif //TRANSMITLEVELS_H
