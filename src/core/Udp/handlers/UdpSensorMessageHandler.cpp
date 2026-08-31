/**
 * @file UdpSensorMessageHandler.cpp
 * @brief Processing implementation for Message ID 902.
 */

#include "UdpSensorMessageHandler.h"
#include "UdpMessages/MessageId.h"
#include "UdpMessages/Structures.h"
#include <QDebug>

namespace GISApp::Core::Udp::Handlers {

UdpSensorMessageHandler::UdpSensorMessageHandler()
    : m_sensorMsg(std::make_unique<Services::SensorMessage>())
{
}

MESSAGE_ID UdpSensorMessageHandler::messageId() const
{
    return SENSOR_MSG_ID; // 902
}

bool UdpSensorMessageHandler::processPayload(const QByteArray &payload)
{
    if (static_cast<size_t>(payload.size()) < sizeof(STRUCT_MESSAGE_HEADER)) {
        qWarning() << "[UdpSensorMessageHandler] Payload smaller than STRUCT_MESSAGE_HEADER!";
        return false;
    }

    // Skip the 20-byte outer header and pass batch payload
    QByteArray bodyPayload = payload.mid(sizeof(STRUCT_MESSAGE_HEADER));
    return m_sensorMsg->parseAndSaveToDb(bodyPayload);
}

} // namespace GISApp::Core::Udp::Handlers
