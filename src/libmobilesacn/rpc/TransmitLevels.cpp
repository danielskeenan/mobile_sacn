/**
 * @file TransmitLevels.cpp
 *
 * @author Dan Keenan
 * @date 8/28/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/TransmitLevels.h>
#include <mobilesacn_messages/TransmitLevels.h>

namespace mobilesacn::rpc {

void TransmitLevels::handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data)
{
    auto msg = message::GetTransmitLevels(data.data());
    if (msg->val_type() == message::TransmitLevelsVal::transmit) {
        onChangeTransmit(msg->val_as_transmit()->transmit());
    } else if (msg->val_type() == message::TransmitLevelsVal::priority) {
        onChangePriority(msg->val_as_priority()->priority());
    } else if (msg->val_type() == message::TransmitLevelsVal::perAddressPriority) {
        onChangePap(msg->val_as_perAddressPriority()->usePap());
    } else if (msg->val_type() == message::TransmitLevelsVal::universe) {
        onChangeUniverse(msg->val_as_universe()->universe());
    } else if (msg->val_type() == message::TransmitLevelsVal::levels) {
        const auto levelsData = msg->val_as_levels()->levels();
        Q_ASSERT(levelsData->size() == levelBuf_.size());
        onChangeLevels(levelsData->data());
    }
}

void TransmitLevels::onChangeLevels(const uint8_t* levelsData)
{
    std::memcpy(levelBuf_.data(), levelsData, levelBuf_.size());

    // Addresses set to 0 also get a PAP priority of 0 (i.e. unused).
    auto levelsIt = levelBuf_.cbegin();
    auto papIt = papBuf_.begin();
    for (; levelsIt != levelBuf_.cend() && papIt != papBuf_.end(); ++levelsIt, ++papIt) {
        *papIt = *levelsIt == 0 ? 0 : univSettings_.priority;
    }

    sendLevelsAndPap();
}

} // mobilesacn::rpc
