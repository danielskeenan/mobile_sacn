/**
 * @file TransmitHandler.h
 *
 * @author Dan Keenan
 * @date 8/28/24
 * @copyright GNU GPLv3
 */

#ifndef TRANSMITHANDLER_H
#define TRANSMITHANDLER_H

#include "RpcHandler.h"
#include <sacn/cpp/source.h>

namespace mobilesacn::rpc {
/**
 * Base class for handlers that transmit sACN.
 */
class TransmitHandler : public RpcHandler {
    Q_OBJECT

  public:
    using RpcHandler::RpcHandler;
    ~TransmitHandler() override;

  public Q_SLOTS:
    void handleConnected() override;

  protected:
    bool perAddressPriority_ = false;
    std::array<uint8_t, DMX_ADDRESS_COUNT> levelBuf_;
    std::array<uint8_t, DMX_ADDRESS_COUNT> papBuf_;

    sacn::Source::Settings sacnSettings_;
    sacn::Source::UniverseSettings univSettings_;
    sacn::Source sacn_;

    [[nodiscard]] bool currentlyTransmitting() const {
      return sacn_.handle().IsValid();
    }

    [[nodiscard]] virtual std::string getSourceName() const;

    virtual void onChangeTransmit(bool transmit);
    virtual void onChangePriority(uint8_t priority);
    virtual void onChangePap(bool usePap);
    virtual void onChangeUniverse(uint16_t useUniverse);
    virtual void startTransmitting();
    virtual void stopTransmitting();
    void sendLevelsAndPap();
};
} // mobilesacn::rpc

#endif //TRANSMITHANDLER_H
