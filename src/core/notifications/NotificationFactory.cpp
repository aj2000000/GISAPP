/**
 * @file NotificationFactory.cpp
 * @brief Factory implementation for binding NotificationTypes to concrete presentation strategies.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "core/notifications/NotificationFactory.h"
#include "ui/notifications/CriticalNotificationStrategy.h"
#include "ui/notifications/FlashNotificationStrategy.h"

namespace GISApp::Core::Notifications {

std::unique_ptr<INotificationStrategy> NotificationFactory::createStrategy(NotificationType type) {
    switch (type) {
    case NotificationType::Critical:
        return std::make_unique<GISApp::UI::Notifications::CriticalNotificationStrategy>();
    case NotificationType::Flash:
    case NotificationType::Info:
    case NotificationType::Warning:
    default:
        return std::make_unique<GISApp::UI::Notifications::FlashNotificationStrategy>();
    }
}

} // namespace GISApp::Core::Notifications
