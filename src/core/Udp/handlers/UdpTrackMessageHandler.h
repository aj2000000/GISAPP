/**
 * @file UdpTrackMessageHandler.h
 * @brief Concrete handler for deserializing MAIN_LITE_TRACK_MSG (ID: 613) into TrackRecords.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef UDPTRACKMESSAGEHANDLER_H
#define UDPTRACKMESSAGEHANDLER_H

#include "IUdpMessageHandler.h"
#include "core/repositories/ITrackRepository.h"

namespace GISApp::Core::Udp::Handlers {

/**
 * @class UdpTrackMessageHandler
 * @brief Strategy deserializing binary track messages into TrackRecord entities for storage.
 */
class UdpTrackMessageHandler : public IUdpMessageHandler {
public:
    /**
     * @brief Constructs track message handler.
     * @param trackRepo Repository where deserialized tracks are saved.
     */
    explicit UdpTrackMessageHandler(Repositories::ITrackRepository *trackRepo);
    ~UdpTrackMessageHandler() override = default;

    MESSAGE_ID messageId() const override;
    bool processPayload(const QByteArray &payload) override;

private:
    Repositories::ITrackRepository *m_trackRepo{nullptr};
};

} // namespace GISApp::Core::Udp::Handlers

#endif // UDPTRACKMESSAGEHANDLER_H
