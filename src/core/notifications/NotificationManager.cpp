/**
 * @file NotificationManager.cpp
 * @brief Implementation of NotificationManager singleton facade.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "core/notifications/NotificationManager.h"
#include "core/notifications/NotificationFactory.h"
#include <QMutexLocker>
#include <QMetaObject>
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

namespace GISApp::Core::Notifications {

NotificationManager* NotificationManager::instance() {
    static NotificationManager s_instance;
    return &s_instance;
}

void NotificationManager::postNotification(const Notification &notification, QWidget *parent) {
    {
        QMutexLocker locker(&m_mutex);
        m_activeNotifications.insert(notification.id(), notification);
    }

    qDebug() << "[NotificationManager] 📢 Posted Notification:" << notification.id()
             << "|" << notification.title() << "| Type:" << static_cast<int>(notification.type())
             << "| Thread:" << QThread::currentThread();

    // Broadcast to Observers
    {
        QMutexLocker locker(&m_mutex);
        for (auto *obs : m_observers) {
            if (obs) obs->onNotificationPublished(notification);
        }
    }
    emit notificationPublished(notification);

    // Instantiate strategy via Factory
    auto strategy = NotificationFactory::createStrategy(notification.type());
    if (strategy) {
        // Strategy rendering callback
        auto dismissCb = [this, type = notification.type()](const QString &id) {
            this->handleNotificationDismissed(id, type);
        };

        // Ensure strategy widget creation always executes on the Main GUI thread
        if (QThread::currentThread() == QCoreApplication::instance()->thread()) {
            strategy->display(notification, parent, dismissCb);
        } else {
            qDebug() << "[NotificationManager] 🔀 Dispatching notification display from worker thread to Main GUI thread";
            QMetaObject::invokeMethod(QCoreApplication::instance(), [strategy = std::move(strategy), notification, parent, dismissCb]() mutable {
                strategy->display(notification, parent, dismissCb);
            }, Qt::QueuedConnection);
        }
    }
}

void NotificationManager::notifyCritical(const QString &title, const QString &message, QWidget *parent) {
    Notification notif(title, message, NotificationType::Critical);
    postNotification(notif, parent);
}

void NotificationManager::notifyFlash(const QString &title, const QString &message, int durationMs, QWidget *parent) {
    Notification notif(title, message, NotificationType::Flash, durationMs);
    postNotification(notif, parent);
}

void NotificationManager::handleNotificationDismissed(const QString &notificationId, NotificationType type) {
    {
        QMutexLocker locker(&m_mutex);
        m_activeNotifications.remove(notificationId);
    }

    if (type == NotificationType::Critical) {
        qDebug() << "[NotificationManager] ✅ Critical Notification Acknowledged by User:" << notificationId;
        {
            QMutexLocker locker(&m_mutex);
            for (auto *obs : m_observers) {
                if (obs) obs->onNotificationAcknowledged(notificationId);
            }
        }
        emit notificationAcknowledged(notificationId);
    } else {
        qDebug() << "[NotificationManager] ⏱️ Flash Notification Vanished/Dismissed:" << notificationId;
        {
            QMutexLocker locker(&m_mutex);
            for (auto *obs : m_observers) {
                if (obs) obs->onNotificationDismissed(notificationId);
            }
        }
        emit notificationDismissed(notificationId);
    }
}

void NotificationManager::addObserver(INotificationObserver *observer) {
    QMutexLocker locker(&m_mutex);
    if (observer && !m_observers.contains(observer)) {
        m_observers.append(observer);
    }
}

void NotificationManager::removeObserver(INotificationObserver *observer) {
    QMutexLocker locker(&m_mutex);
    m_observers.removeAll(observer);
}

QList<Notification> NotificationManager::activeNotifications() const {
    QMutexLocker locker(&m_mutex);
    return m_activeNotifications.values();
}

} // namespace GISApp::Core::Notifications
