/**
 * @file FlashNotificationStrategy.h
 * @brief Strategy for rendering Modeless Flash notifications that auto-vanish after 5 seconds.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef FLASHNOTIFICATIONSTRATEGY_H
#define FLASHNOTIFICATIONSTRATEGY_H

#include "core/notifications/INotificationStrategy.h"
#include <QObject>

namespace GISApp::UI::Notifications {

/**
 * @class FlashNotificationStrategy
 * @brief Concrete strategy for Flash Notifications.
 * 
 * Pops up a modeless toast notification widget in the corner of the map/application window,
 * automatically fading out and closing after 5 seconds (configurable duration).
 */
class FlashNotificationStrategy : public QObject, public GISApp::Core::Notifications::INotificationStrategy {
    Q_OBJECT
public:
    FlashNotificationStrategy() = default;
    ~FlashNotificationStrategy() override = default;

    void display(const GISApp::Core::Notifications::Notification &notification,
                 QWidget *parent,
                 DismissCallback onDismissed) override;
};

} // namespace GISApp::UI::Notifications

#endif // FLASHNOTIFICATIONSTRATEGY_H
