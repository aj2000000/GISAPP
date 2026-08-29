/**
 * @file mediatorclass.cpp
 * @brief Implementation of MediatorClass network router.
 */

#include "mediatorclass.h"
#include "udpcommunication.h"
#include "datastore.h"
#include "UdpMessages/MessageId.h"
#include "UdpMessages/Structures.h"
#include <QDateTime>
#include <QDebug>

MediatorClass::MediatorClass(QObject *parent) 
    : QObject(parent)
{
    // Read socket configuration (IP & Ports)
    DataStore::getInstance().readConfigurationFiles();

    // Create central UdpMessageDispatcher
    m_dispatcher = new GISApp::Core::Udp::Handlers::UdpMessageDispatcher(this);

    // Start UDP socket thread listening on Port 8540
    quint16 udp_port = DataStore::getInstance().client_side_udp_recv_port;
    udpCommObj = new UDPCommunication(udp_port);
    udpCommObj->start();

    // Connect ProcessUDPMessageThread output to MediatorClass slot
    connect(&ProcessUDPMessageThread::getInstance(), 
            &ProcessUDPMessageThread::sendUdpDataToMediatorSignal,
            this, 
            &MediatorClass::receivedMessageFromUDP);

    // Initialize singleton worker thread
    ProcessUDPMessageThread::getInstance();
}

void MediatorClass::receivedMessageFromUDP(QByteArray message)
{
    if (static_cast<size_t>(message.size()) < sizeof(STRUCT_MESSAGE_HEADER)) {
        return;
    }

    STRUCT_MESSAGE_HEADER msg_header;
    SMEMCPY(&msg_header, message.constData(), sizeof(msg_header));

    qDebug() << "[MediatorClass] 📥 Datagram Recv | Time:" << QDateTime::currentDateTime().toString("hh:mm:ss.z")
             << "| Src:" << msg_header.source_id 
             << "| Dst:" << msg_header.destination_id 
             << "| Msg ID:" << msg_header.message_id 
             << "| PktSeq:" << msg_header.packet_seq_no;

    // Emit generic signal for any logging/monitoring listeners
    emit rawUdpPayloadReceived(message);

    // Forward payload to UdpMessageDispatcher for automatic handler lookup and execution
    if (m_dispatcher) {
        m_dispatcher->dispatchMessage(message);
    }
}
