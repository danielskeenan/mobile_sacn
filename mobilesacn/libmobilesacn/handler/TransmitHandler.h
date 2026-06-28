/**
 * @file TransmitHandler.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_TRANSMITHANDLER_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_TRANSMITHANDLER_H

#include "BaseHandler.h"
#include <sacn/cpp/source.h>

namespace mobilesacn::handler {

class TransmitHandler : public BaseHandler
{
    Q_OBJECT

public:
    explicit TransmitHandler(QWebSocket *ws, QObject *parent);
    ~TransmitHandler() override;

protected:
    bool perAddressPriority_ = false;
    std::array<uint8_t, kSacnDmxAddressCount> levelBuf_{};
    std::array<uint8_t, kSacnDmxAddressCount> papBuf_{};

    sacn::Source::Settings sacnSettings_;
    sacn::Source::UniverseSettings univSettings_;
    sacn::Source sacn_;

    [[nodiscard]] bool currentlyTransmitting() const { return sacn_.handle().IsValid(); }

    [[nodiscard]] virtual std::string getSourceName() const;

    virtual void onChangeTransmit(bool transmit);
    virtual void onChangePriority(uint8_t priority);
    virtual void onChangePap(bool usePap);
    virtual void onChangeUniverse(uint16_t useUniverse);
    virtual void startTransmitting();
    virtual void stopTransmitting();
    void sendLevelsAndPap();

protected Q_SLOTS:
    virtual void onBinaryMessage(const QByteArray& data) = 0;
};

} // namespace mobilesacn::handler

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_TRANSMITHANDLER_H
