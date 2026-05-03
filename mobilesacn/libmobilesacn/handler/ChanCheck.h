/**
 * @file ChanCheck.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_CHANCHECK_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_CHANCHECK_H

#include "TransmitHandler.h"
#include <QTimer>

namespace mobilesacn::handler {

/**
 * Handler for Channel Check.
 */
class ChanCheck final : public TransmitHandler
{
    Q_OBJECT

public:
    explicit ChanCheck(QWebSocket *ws, QObject *parent);
    static constexpr auto kProtocol = "ChanCheck";
    [[nodiscard]] const char *getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Chan Check"); }

protected:
    void onChangePriority(uint8_t priority) override;
    void startTransmitting() override;

protected Q_SLOTS:
    void onBinaryMessage(const QByteArray &message) override;

private:
    uint16_t address_ = 1;
    uint8_t level_ = 0;
    QTimer* blinker_;

    void onChangeAddress(uint16_t useAddress);
    void onChangeLevel(uint8_t useLevel);
    void onChangeBlink(bool blink);
    void updateLevelBuf();
    void updatePapBuf();

private Q_SLOTS:
    void blink();
};

} // namespace mobilesacn::handler

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_CHANCHECK_H
