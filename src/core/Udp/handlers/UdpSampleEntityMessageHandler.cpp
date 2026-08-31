/**
 * @file UdpSampleEntityMessageHandler.cpp
 * @brief Strategy implementation for processing ID 901 UDP datagrams into SampleEntity structures.
 */

#include "UdpSampleEntityMessageHandler.h"
#include "UdpMessages/MessageId.h"
#include "UdpMessages/Structures.h"
#include <QDebug>

namespace GISApp::Core::Udp::Handlers {

UdpSampleEntityMessageHandler::UdpSampleEntityMessageHandler()
    : m_sampleMsg(std::make_unique<Services::SampleEntityMessage>())
{
}

MESSAGE_ID UdpSampleEntityMessageHandler::messageId() const
{
    return SAMPLE_ENTITY_MSG_ID; // 901
}

bool UdpSampleEntityMessageHandler::processPayload(const QByteArray &payload)
{
    if (static_cast<size_t>(payload.size()) < sizeof(STRUCT_MESSAGE_HEADER)) {
        qWarning() << "[UdpSampleEntityMessageHandler] Payload smaller than STRUCT_MESSAGE_HEADER!";
        return false;
    }

    // Skip the 20-byte outer STRUCT_MESSAGE_HEADER and extract the SampleEntity batch payload
    QByteArray bodyPayload = payload.mid(sizeof(STRUCT_MESSAGE_HEADER));

    qDebug() << "[UdpSampleEntityMessageHandler] 📥 Ingesting ID 901 datagram, body size:" << bodyPayload.size();

    return m_sampleMsg->parseAndSaveToDb(bodyPayload);
}

} // namespace GISApp::Core::Udp::Handlers
