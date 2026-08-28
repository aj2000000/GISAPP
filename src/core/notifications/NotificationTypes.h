/**
 * @file NotificationTypes.h
 * @brief Enums and payload data structures for the Notification Subsystem.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef NOTIFICATIONTYPES_H
#define NOTIFICATIONTYPES_H

#include <QString>
#include <QDateTime>
#include <QUuid>

namespace GISApp::Core::Notifications {

/**
 * @enum NotificationType
 * @brief Categorizes the behavior and display strategy of notifications.
 */
enum class NotificationType {
    Critical,   ///< Requires explicit user acknowledgement (Modal dialog)
    Flash,      ///< Modeless popup toast that automatically vanishes (e.g. after 5s)
    Info,       ///< Informational message
    Warning     ///< Warning message
};

/**
 * @class Notification
 * @brief Encapsulates notification data, metadata, and type classification.
 * 
 * Demonstrates OOP Encapsulation by exposing clean getters/setters and maintaining data integrity.
 */
class Notification {
public:
    Notification()
        : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
        , m_type(NotificationType::Flash)
        , m_timestamp(QDateTime::currentDateTime())
        , m_durationMs(5000)
    {}

    Notification(const QString &title,
                 const QString &message,
                 NotificationType type = NotificationType::Flash,
                 int durationMs = 5000)
        : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
        , m_title(title)
        , m_message(message)
        , m_type(type)
        , m_timestamp(QDateTime::currentDateTime())
        , m_durationMs(durationMs)
    {}

    // Getters
    QString id() const { return m_id; }
    QString title() const { return m_title; }
    QString message() const { return m_message; }
    NotificationType type() const { return m_type; }
    QDateTime timestamp() const { return m_timestamp; }
    int durationMs() const { return m_durationMs; }

    // Setters
    void setTitle(const QString &title) { m_title = title; }
    void setMessage(const QString &message) { m_message = message; }
    void setType(NotificationType type) { m_type = type; }
    void setDurationMs(int durationMs) { m_durationMs = durationMs; }

private:
    QString m_id;
    QString m_title;
    QString m_message;
    NotificationType m_type;
    QDateTime m_timestamp;
    int m_durationMs;
};

} // namespace GISApp::Core::Notifications

#endif // NOTIFICATIONTYPES_H
