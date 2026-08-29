/**
 * @file mediatorclass.h
 * @brief Mediator component bridging UDP network threads and the UdpMessageDispatcher.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef MEDIATORCLASS_H
#define MEDIATORCLASS_H

#include <QObject>
#include <QTimer>
#include "handlers/UdpMessageDispatcher.h"

class UDPCommunication;
class DataStore;
class UDPSendData;

/**
 * @class MediatorClass
 * @brief Network coordinator managing socket lifecycle and forwarding packets to UdpMessageDispatcher.
 */
class MediatorClass : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs MediatorClass, starts UDP listening thread, and initializes dispatcher.
     * @param parent Optional Qt parent object.
     */
    explicit MediatorClass(QObject *parent = nullptr);

    /**
     * @brief Returns a pointer to the internal UdpMessageDispatcher instance.
     * @return Pointer to UdpMessageDispatcher.
     */
    GISApp::Core::Udp::Handlers::UdpMessageDispatcher* dispatcher() const { return m_dispatcher; }

signals:
    /**
     * @brief Optional generic signal emitted whenever any raw UDP payload arrives.
     * @param message Raw QByteArray datagram buffer.
     */
    void rawUdpPayloadReceived(const QByteArray &message);

private slots:
    /**
     * @brief Internal slot connected to ProcessUDPMessageThread.
     * @param message Raw UDP packet payload.
     */
    void receivedMessageFromUDP(QByteArray message);

private:
    UDPCommunication *udpCommObj{nullptr};
    UDPSendData *udpSendObj{nullptr};
    GISApp::Core::Udp::Handlers::UdpMessageDispatcher *m_dispatcher{nullptr};
};

#endif // MEDIATORCLASS_H
