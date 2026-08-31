/**
 * @file UdpSensorMessageHandler.h
 * @brief Handler strategy for Message ID 902 (Sensor telemetry).
 */

#ifndef UDPSENSORMESSAGEHANDLER_H
#define UDPSENSORMESSAGEHANDLER_H

#include "IUdpMessageHandler.h"
#include "core/Udp/UdpMessages/SensorMessage.h"
#include <memory>

namespace GISApp::Core::Udp::Handlers {

class UdpSensorMessageHandler : public IUdpMessageHandler {
public:
    UdpSensorMessageHandler();
    ~UdpSensorMessageHandler() override = default;

    MESSAGE_ID messageId() const override;
    bool processPayload(const QByteArray &payload) override;

    Services::SensorMessage* sensorMessage() const { return m_sensorMsg.get(); }
    Services::CUSTOM_MESSAGE* customMessage() override { return m_sensorMsg.get(); }

private:
    std::unique_ptr<Services::SensorMessage> m_sensorMsg;
};

} // namespace GISApp::Core::Udp::Handlers

#endif // UDPSENSORMESSAGEHANDLER_H
