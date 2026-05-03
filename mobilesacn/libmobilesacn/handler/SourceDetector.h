/**
 * @file SourceDetector.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_SOURCEDETECTOR_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_SOURCEDETECTOR_H

#include <mutex>
#include <sacn/cpp/common.h>
#include <sacn/cpp/source_detector.h>
#include <QObject>

namespace mobilesacn::handler {

struct SourceDetectorSource
{
    std::string cid;
    std::string name;
    std::vector<uint16_t> universes;
};

/**
 * Wrap an sacn::SourceDetector to handle multiple subscribers.
 */
class SourceDetector : public QObject, public sacn::SourceDetector::NotifyHandler
{
    Q_OBJECT
public:
    static SourceDetector *get();

    SourceDetector(const SourceDetector &) = delete;
    SourceDetector &operator=(const SourceDetector &) = delete;
    ~SourceDetector() override;

    void startup();
    void shutdown();

    void HandleSourceUpdated(
        sacn::RemoteSourceHandle handle,
        const etcpal::Uuid &cid,
        const std::string &name,
        const std::vector<uint16_t> &sourcedUniverses) override;
    void HandleSourceExpired(
        sacn::RemoteSourceHandle handle, const etcpal::Uuid &cid, const std::string &name) override;

    [[nodiscard]] std::unordered_map<etcpal::Uuid, SourceDetectorSource> sources() const;

Q_SIGNALS:
    void sourceUpdated(const SourceDetectorSource &source);
    void sourceExpired(const std::string &cid);

private:
    using QObject::QObject;

    mutable std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, SourceDetectorSource> sources_;
};

} // namespace mobilesacn::handler

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_SOURCEDETECTOR_H
