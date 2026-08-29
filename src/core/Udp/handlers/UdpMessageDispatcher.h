/**
 * @file UdpMessageDispatcher.h
 * @brief Central dispatcher routing incoming UDP datagrams to registered message handlers.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef UDPMESSAGEDISPATCHER_H
#define UDPMESSAGEDISPATCHER_H

#include <QObject>
#include <QMap>
#include <memory>
#include "IUdpMessageHandler.h"

namespace GISApp::Core::Udp::Handlers {

/**
 * @class UdpMessageDispatcher
 * @brief Dispatcher maintaining a registry of IUdpMessageHandler strategies.
 */
class UdpMessageDispatcher : public QObject {
    Q_OBJECT

public:
    explicit UdpMessageDispatcher(QObject *parent = nullptr);
    ~UdpMessageDispatcher() override = default;

    /**
     * @brief Registers a message handler strategy for a specific Message ID.
     * @param handler Shared pointer to the concrete message handler.
     */
    void registerHandler(std::shared_ptr<IUdpMessageHandler> handler);

public slots:
    /**
     * @brief Central slot connected to MediatorClass signal emitting incoming raw UDP payloads.
     * @param payload Raw binary datagram buffer.
     */
    void dispatchMessage(const QByteArray &payload);

private:
    QMap<MESSAGE_ID, std::shared_ptr<IUdpMessageHandler>> m_handlers;
};

} // namespace GISApp::Core::Udp::Handlers

#endif // UDPMESSAGEDISPATCHER_H
