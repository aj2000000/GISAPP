/**
 * @file INotificationStrategy.h
 * @brief Strategy pattern interface for rendering different notification presentation types.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef INOTIFICATIONSTRATEGY_H
#define INOTIFICATIONSTRATEGY_H

#include "NotificationTypes.h"
#include <QWidget>
#include <functional>

namespace GISApp::Core::Notifications {

/**
 * @class INotificationStrategy
 * @brief Abstract Strategy interface for rendering notifications to the user interface.
 * 
 * Demonstrates:
 * - Strategy Design Pattern
 * - SOLID: Single Responsibility Principle (SRP - rendering decoupled from logic)
 * - SOLID: Open/Closed Principle (OCP - new strategies can be added seamlessly)
 * - SOLID: Liskov Substitution Principle (LSP - strategies are interchangeable)
 */
class INotificationStrategy {
public:
    using DismissCallback = std::function<void(const QString &notificationId)>;

    virtual ~INotificationStrategy() = default;

    /**
     * @brief Renders and presents the notification on screen.
     * @param notification Notification payload data.
     * @param parent Optional parent widget context.
     * @param onDismissed Callback triggered when notification closes/vanishes.
     */
    virtual void display(const Notification &notification,
                         QWidget *parent,
                         DismissCallback onDismissed) = 0;
};

} // namespace GISApp::Core::Notifications

#endif // INOTIFICATIONSTRATEGY_H
