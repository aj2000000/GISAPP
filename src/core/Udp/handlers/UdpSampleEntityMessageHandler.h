/**
 * @file UdpSampleEntityMessageHandler.h
 * @brief Strategy handler for deserializing SAMPLE_ENTITY_MSG (ID: 901) UDP datagrams.
 */

#ifndef UDPSAMPLEENTITYMESSAGEHANDLER_H
#define UDPSAMPLEENTITYMESSAGEHANDLER_H

#include "IUdpMessageHandler.h"
#include "core/Udp/UdpMessages/SampleEntityMessage.h"
#include <memory>

namespace GISApp::Core::Udp::Handlers {

class UdpSampleEntityMessageHandler : public IUdpMessageHandler {
public:
    UdpSampleEntityMessageHandler();
    ~UdpSampleEntityMessageHandler() override = default;

    MESSAGE_ID messageId() const override;
    bool processPayload(const QByteArray &payload) override;

    Services::SampleEntityMessage* sampleMessage() const { return m_sampleMsg.get(); }
    Services::CUSTOM_MESSAGE* customMessage() override { return m_sampleMsg.get(); }

private:
    std::unique_ptr<Services::SampleEntityMessage> m_sampleMsg;
};

} // namespace GISApp::Core::Udp::Handlers

#endif // UDPSAMPLEENTITYMESSAGEHANDLER_H
