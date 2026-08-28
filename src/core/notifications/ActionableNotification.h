/**
 * @file ActionableNotification.h
 * @brief Concrete subclass for interactive notifications featuring a custom action button and handler.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef ACTIONABLENOTIFICATION_H
#define ACTIONABLENOTIFICATION_H

#include "core/notifications/AbstractNotification.h"
#include <functional>

namespace GISApp::Core::Notifications {

/**
 * @class ActionableNotification
 * @brief Subclass for interactive notifications with action buttons.
 */
class ActionableNotification : public AbstractNotification {
public:
    using ActionHandler = std::function<void()>;

    ActionableNotification(const QString &title,
                           const QString &message,
                           const QString &actionButtonText,
                           ActionHandler handler,
                           NotificationType type = NotificationType::Info);

    ~ActionableNotification() override = default;

    QString actionButtonText() const;
    void executeAction() const;

private:
    QString m_actionButtonText;
    ActionHandler m_actionHandler;
};

} // namespace GISApp::Core::Notifications

#endif // ACTIONABLENOTIFICATION_H
