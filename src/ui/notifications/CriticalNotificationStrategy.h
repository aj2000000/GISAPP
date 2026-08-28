/**
 * @file CriticalNotificationStrategy.h
 * @brief Strategy for rendering Critical notifications requiring explicit user acknowledgement.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef CRITICALNOTIFICATIONSTRATEGY_H
#define CRITICALNOTIFICATIONSTRATEGY_H

#include "core/notifications/INotificationStrategy.h"
#include <QObject>

namespace GISApp::UI::Notifications {

/**
 * @class CriticalNotificationStrategy
 * @brief Concrete strategy for Critical Notifications.
 * 
 * Displays a modal dialog with rich tactical visual aesthetics (red accent bar, clear warning icon,
 * explicit Acknowledge button). The dialog stays open until the user clicks Acknowledge.
 */
class CriticalNotificationStrategy : public QObject, public GISApp::Core::Notifications::INotificationStrategy {
    Q_OBJECT
public:
    CriticalNotificationStrategy() = default;
    ~CriticalNotificationStrategy() override = default;

    void display(const GISApp::Core::Notifications::Notification &notification,
                 QWidget *parent,
                 DismissCallback onDismissed) override;
};

} // namespace GISApp::UI::Notifications

#endif // CRITICALNOTIFICATIONSTRATEGY_H
