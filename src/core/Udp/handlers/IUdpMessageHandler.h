/**
 * @file IUdpMessageHandler.h
 * @brief Strategy interface for parsing and processing specific UDP binary message types.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef IUDPMESSAGEHANDLER_H
#define IUDPMESSAGEHANDLER_H

#include <QByteArray>
#include "UdpMessages/IRS.h"

namespace GISApp::Core::Udp::Handlers {

/**
 * @class IUdpMessageHandler
 * @brief Abstract base class for all UDP message deserializers and domain handlers.
 */
class IUdpMessageHandler {
public:
    virtual ~IUdpMessageHandler() = default;

    /**
     * @brief Returns the unique Message ID that this handler processes.
     * @return MESSAGE_ID numeric identifier.
     */
    virtual MESSAGE_ID messageId() const = 0;

    /**
     * @brief Processes and deserializes the incoming raw UDP binary payload.
     * @param payload Complete datagram QByteArray buffer.
     * @return True if parsing and processing succeeded, false otherwise.
     */
    virtual bool processPayload(const QByteArray &payload) = 0;
};

} // namespace GISApp::Core::Udp::Handlers

#endif // IUDPMESSAGEHANDLER_H
