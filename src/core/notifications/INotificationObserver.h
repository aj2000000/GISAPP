/**
 * @file INotificationObserver.h
 * @brief Observer pattern interface for listening to notification lifecycle events.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef INOTIFICATIONOBSERVER_H
#define INOTIFICATIONOBSERVER_H

#include "NotificationTypes.h"

namespace GISApp::Core::Notifications {

/**
 * @class INotificationObserver
 * @brief Observer interface for subscribing to notification state transitions.
 * 
 * Demonstrates:
 * - Observer Design Pattern
 * - SOLID: Interface Segregation Principle (ISP - lean callback interface)
 */
class INotificationObserver {
public:
    virtual ~INotificationObserver() = default;

    virtual void onNotificationPublished(const Notification &notification) = 0;
    virtual void onNotificationAcknowledged(const QString &notificationId) = 0;
    virtual void onNotificationDismissed(const QString &notificationId) = 0;
};

} // namespace GISApp::Core::Notifications

#endif // INOTIFICATIONOBSERVER_H
