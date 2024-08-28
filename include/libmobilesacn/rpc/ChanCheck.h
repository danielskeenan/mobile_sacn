/**
 * @file ChanCheck.h
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#ifndef CHANCHECK_H
#define CHANCHECK_H

#include "RpcHandler.h"
#include <QObject>
#include <mobilesacn_messages/Universe.h>
#include <sacn/cpp/source.h>

namespace mobilesacn::rpc {

/**
 * Handler for Channel Check.
 */
class ChanCheck : public RpcHandler
{
    Q_OBJECT

public:
    explicit ChanCheck(crow::websocket::connection& ws, QObject* parent = 0);
    ~ChanCheck() override;
    static constexpr auto kProtocol = "ChanCheck";
    [[nodiscard]] const char* getProtocol() override { return kProtocol; }

public Q_SLOTS:
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;

private:
    bool perAddressPriority_ = false;
    uint16_t address_ = 1;
    uint8_t level_ = 0;
    std::array<uint8_t, DMX_ADDRESS_COUNT> levelBuf_;
    std::array<uint8_t, DMX_ADDRESS_COUNT> papBuf_;

    sacn::Source::Settings sacnSettings_;
    sacn::Source::UniverseSettings univSettings_;
    sacn::Source sacn_;

    [[nodiscard]] bool currentlyTransmitting() const
    {
        return sacn_.handle().IsValid();
    }

    void onChangeTransmit(bool transmit);
    void onChangePriority(uint8_t priority);
    void onChangePap(bool usePap);
    void onChangeUniverse(uint16_t useUniverse);
    void onChangeAddress(uint16_t useAddress);
    void onChangeLevel(uint8_t useLevel);

    void updateLevelBuf();
    void updatePapBuf();
    void sendLevelsAndPap();
};

} // mobilesacn::rpc

#endif //CHANCHECK_H
