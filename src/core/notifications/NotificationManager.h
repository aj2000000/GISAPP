/**
 * @file NotificationManager.h
 * @brief Central Manager for system-wide notification dispatching and lifecycle tracking.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include "core/notifications/NotificationTypes.h"
#include "core/notifications/INotificationStrategy.h"
#include "core/notifications/INotificationObserver.h"

#include <QObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QWidget>
#include <memory>

namespace GISApp::Core::Notifications {

/**
 * @class NotificationManager
 * @brief Central Manager & Facade for notification publishing and dispatching.
 * 
 * Demonstrates:
 * - Singleton Pattern (single thread-safe instance)
 * - Observer Pattern (notifies subscribers on publication, acknowledgement, and dismissal)
 * - Strategy Pattern (delegates rendering to strategy derivative)
 * - Factory Pattern (creates strategy dynamically via NotificationFactory)
 * - SOLID principles: High-level application code depends on NotificationManager facade; UI strategies remain decoupled.
 */
class NotificationManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Access the Singleton instance of NotificationManager.
     */
    static NotificationManager* instance();

    ~NotificationManager() override = default;

    /**
     * @brief Publishes a notification using its configured type and strategy.
     */
    void postNotification(const Notification &notification, QWidget *parent = nullptr);

    /**
     * @brief Helper to publish a Critical Notification (requires explicit user acknowledgement).
     */
    void notifyCritical(const QString &title, const QString &message, QWidget *parent = nullptr);

    /**
     * @brief Helper to publish a Modeless Flash Notification (vanishes automatically after 5s).
     */
    void notifyFlash(const QString &title, const QString &message, int durationMs = 5000, QWidget *parent = nullptr);

    /**
     * @brief Registers an observer for notification lifecycle events.
     */
    void addObserver(INotificationObserver *observer);

    /**
     * @brief Unregisters an observer.
     */
    void removeObserver(INotificationObserver *observer);

    /**
     * @brief Retrieves active (undismissed) notifications.
     */
    QList<Notification> activeNotifications() const;

signals:
    void notificationPublished(const GISApp::Core::Notifications::Notification &notification);
    void notificationAcknowledged(const QString &notificationId);
    void notificationDismissed(const QString &notificationId);

private:
    NotificationManager() = default;
    Q_DISABLE_COPY(NotificationManager)

    void handleNotificationDismissed(const QString &notificationId, NotificationType type);

    mutable QMutex m_mutex;
    QList<INotificationObserver*> m_observers;
    QMap<QString, Notification> m_activeNotifications;
};

} // namespace GISApp::Core::Notifications

#endif // NOTIFICATIONMANAGER_H
