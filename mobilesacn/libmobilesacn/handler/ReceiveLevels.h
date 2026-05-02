/**
 * @file ReceiveLevels.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_RECEIVELEVELS_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_RECEIVELEVELS_H

#include "BaseHandler.h"
#include "MergeReceiver.h"
#include "SourceDetector.h"
#include "sacn/common.h"

namespace mobilesacn::handler {

/**
 * Handler for Receive Levels.
 */
class ReceiveLevels : public BaseHandler
{
    Q_OBJECT
public:
    explicit ReceiveLevels(QWebSocket *ws, QObject *parent);

    static constexpr auto kProtocol = "ReceiveLevels";
    [[nodiscard]] const char *getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Receive Levels"); }

private:
    struct LastSeen
    {
        std::array<uint8_t, DMX_ADDRESS_COUNT> levels{};
        std::array<uint8_t, DMX_ADDRESS_COUNT> priorities{};
        std::array<std::string, DMX_ADDRESS_COUNT> owners{};
    };
    static constexpr auto kMessageInterval = std::chrono::milliseconds(100);
    std::mutex lastSeenMutex_;
    LastSeen lastSeen_;
    MergeReceiver::Ptr receiver_;
    bool flickerFinder_ = false;
    std::mutex flickerFinderReferenceBufferMutex_;
    std::array<uint8_t, DMX_ADDRESS_COUNT> flickerFinderReferenceBuffer_{};

    void onChangeUniverse(uint16_t universe);
    void onChangeFlickerFinder(bool flickerFinder);

private Q_SLOTS:
    void onBinaryMessage(const QByteArray &data);
    void onSourceUpdated(const SourceDetectorSource &source) const;
    void onSourceUpdated(const sacn::MergeReceiver::Source &source) const;
    void onSourceExpired(const std::string &cid) const;
    void onMergedData(
        const SacnRecvMergedData &mergedData,
        const std::array<std::string, DMX_ADDRESS_COUNT> &ownerCids);
    void onSourceLost(const std::string &cid) const;
};

} // namespace mobilesacn::handler

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_RECEIVELEVELS_H
