/**
 * @file NotificationFactory.h
 * @brief Factory class for creating notification strategies.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef NOTIFICATIONFACTORY_H
#define NOTIFICATIONFACTORY_H

#include "core/notifications/INotificationStrategy.h"
#include <memory>

namespace GISApp::Core::Notifications {

/**
 * @class NotificationFactory
 * @brief Factory Design Pattern implementation for instantiating notification strategies.
 * 
 * Demonstrates:
 * - Factory Design Pattern
 * - SOLID: Single Responsibility Principle (Factory exclusively manages strategy creation)
 */
class NotificationFactory {
public:
    /**
     * @brief Creates a concrete strategy matching the requested NotificationType.
     */
    static std::unique_ptr<INotificationStrategy> createStrategy(NotificationType type);
};

} // namespace GISApp::Core::Notifications

#endif // NOTIFICATIONFACTORY_H
