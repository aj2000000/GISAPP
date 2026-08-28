/**
 * @file FlashNotificationStrategy.cpp
 * @brief Modeless Flash toast notification presentation strategy anchored to QMainWindow Top-Right overlay.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "ui/notifications/FlashNotificationStrategy.h"
#include <QWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QList>
#include <QPointer>
#include <QDebug>

namespace GISApp::UI::Notifications {

// Global list tracking active toast widgets for dynamic stacking & reflow
static QList<QPointer<QWidget>> s_activeToasts;

/**
 * @brief Helper to resolve the true main application window (QMainWindow), bypassing dialogs.
 */
static QWidget* findMainWindow(QWidget *parentWidget = nullptr) {
    const auto topLevels = QApplication::topLevelWidgets();
    QWidget *candidate = nullptr;

    // 1. Look specifically for a QMainWindow in topLevelWidgets
    for (QWidget *w : topLevels) {
        if (!w || !w->isVisible() || w->property("isToast").toBool()) continue;

        if (qobject_cast<QMainWindow*>(w)) {
            return w;
        }

        if (!w->inherits("QDialog") && !w->inherits("QMenu") && !w->inherits("QToolTip")) {
            candidate = w;
        }
    }

    if (candidate) return candidate;

    // 2. If parentWidget exists, climb up to highest non-dialog parent
    if (parentWidget) {
        QWidget *w = parentWidget->window();
        while (w && (w->inherits("QDialog") || w->inherits("QMenu"))) {
            w = w->parentWidget() ? w->parentWidget()->window() : nullptr;
        }
        if (w) return w;
    }

    // 3. Fallback to active window if it's not a dialog
    QWidget *active = QApplication::activeWindow();
    if (active && !active->inherits("QDialog") && !active->inherits("QMenu")) {
        return active;
    }

    return nullptr;
}

/**
 * @brief Calculates top-right target position relative to top-level main window client geometry.
 */
static QPoint calculateTopRightPos(QWidget *topWin, int toastWidth, int yOffset) {
    if (topWin && topWin->isVisible()) {
        // Return local coordinates relative to topWin parent widget
        int x = topWin->rect().width() - toastWidth - 24;
        int y = yOffset;
        return QPoint(x, y);
    }

    // Fallback screen coordinates if main window not available
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen ? screen->geometry() : QRect(0, 0, 1920, 1080);
    return QPoint(screenRect.right() - toastWidth - 24, screenRect.top() + yOffset);
}

/**
 * @brief Helper function to dynamically recalculate positions for all active toasts
 * to stack top-right of main window and smoothly reflow upwards when a toast is closed.
 */
static void repositionActiveToasts(QWidget *parentWidget) {
    s_activeToasts.removeAll(nullptr);
    QWidget *topWin = findMainWindow(parentWidget);

    int yOffset = 65; // Top margin offset below main window header bar
    for (int i = 0; i < s_activeToasts.size(); ++i) {
        QWidget *t = s_activeToasts[i];
        if (!t) continue;

        QPoint targetPos = calculateTopRightPos(topWin, t->width(), yOffset);

        if (t->pos() != targetPos && t->isVisible()) {
            QPropertyAnimation *reposAnim = new QPropertyAnimation(t, "pos", t);
            reposAnim->setDuration(250);
            reposAnim->setStartValue(t->pos());
            reposAnim->setEndValue(targetPos);
            reposAnim->setEasingCurve(QEasingCurve::OutCubic);
            reposAnim->start(QAbstractAnimation::DeleteWhenStopped);
        }

        t->raise(); // Keep toasts on top of the z-stack
        yOffset += t->height() + 12; // 12px vertical gap between toasts
    }
}

void FlashNotificationStrategy::display(const GISApp::Core::Notifications::Notification &notification,
                                        QWidget *parent,
                                        DismissCallback onDismissed)
{
    // Clean null pointers from active stack
    s_activeToasts.removeAll(nullptr);

    // Resolve main application window (specifically ignoring dialogs)
    QWidget *topWin = findMainWindow(parent);

    // Create child overlay widget parented to topWin (MainWindow)
    QWidget *toast = nullptr;
    if (topWin) {
        toast = new QWidget(topWin);
        toast->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    } else {
        toast = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    }

    toast->setAttribute(Qt::WA_DeleteOnClose);
    toast->setProperty("isToast", true);
    toast->setFixedSize(380, 92);

    // Determine Theme & Accent Colors based on Notification Type
    QString accentColor = "#38bdf8"; // Neon Sky Blue default
    QString iconStr = "⚡";

    if (notification.type() == GISApp::Core::Notifications::NotificationType::Warning) {
        accentColor = "#f59e0b"; // Amber Warning
        iconStr = "⚠️";
    } else if (notification.type() == GISApp::Core::Notifications::NotificationType::Info) {
        accentColor = "#10b981"; // Emerald Info
        iconStr = "ℹ️";
    }

    // Modern Dark Glassmorphism Styling
    toast->setStyleSheet(QString(R"(
        QWidget#ToastContainer {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0f172a, stop:1 #1e293b);
            border: 1px solid %1;
            border-radius: 10px;
        }
        QLabel#IconBadge {
            background-color: rgba(15, 23, 42, 0.6);
            border: 1px solid %1;
            border-radius: 18px;
            font-size: 16px;
        }
        QLabel#FlashTitle {
            color: #f8fafc;
            font-size: 13px;
            font-weight: 700;
            letter-spacing: 0.5px;
        }
        QLabel#FlashMessage {
            color: #94a3b8;
            font-size: 11px;
            line-height: 1.3;
        }
        QPushButton#CloseBtn {
            background: rgba(255, 255, 255, 0.05);
            color: #94a3b8;
            font-size: 12px;
            font-weight: bold;
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 10px;
            min-width: 20px;
            max-width: 20px;
            min-height: 20px;
            max-height: 20px;
        }
        QPushButton#CloseBtn:hover {
            background: #ef4444;
            color: #ffffff;
            border: none;
        }
        QProgressBar#LifespanBar {
            background-color: rgba(255, 255, 255, 0.08);
            border: none;
            border-radius: 1px;
            max-height: 3px;
        }
        QProgressBar#LifespanBar::chunk {
            background-color: %1;
            border-radius: 1px;
        }
    )").arg(accentColor));

    toast->setObjectName("ToastContainer");

    QVBoxLayout *outerLayout = new QVBoxLayout(toast);
    outerLayout->setContentsMargins(14, 12, 14, 8);
    outerLayout->setSpacing(8);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    // Icon Badge Widget
    QLabel *iconBadge = new QLabel(iconStr, toast);
    iconBadge->setObjectName("IconBadge");
    iconBadge->setFixedSize(36, 36);
    iconBadge->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(iconBadge);

    // Text Container (Title + Body Message)
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    QLabel *titleLabel = new QLabel(notification.title(), toast);
    titleLabel->setObjectName("FlashTitle");

    QLabel *messageLabel = new QLabel(notification.message(), toast);
    messageLabel->setObjectName("FlashMessage");
    messageLabel->setWordWrap(true);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(messageLabel);
    contentLayout->addLayout(textLayout, 1);

    // Close Button
    QPushButton *closeBtn = new QPushButton("✕", toast);
    closeBtn->setObjectName("CloseBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    contentLayout->addWidget(closeBtn, 0, Qt::AlignTop);

    outerLayout->addLayout(contentLayout, 1);

    // Animated Countdown Progress Bar
    int durationMs = notification.durationMs() > 0 ? notification.durationMs() : 5000;
    QProgressBar *progressBar = new QProgressBar(toast);
    progressBar->setObjectName("LifespanBar");
    progressBar->setRange(0, durationMs);
    progressBar->setValue(durationMs);
    progressBar->setTextVisible(false);
    outerLayout->addWidget(progressBar);

    // Calculate Y Offset for Stacked Position in Top-Right
    int yOffset = 65;
    for (const auto &activeToast : s_activeToasts) {
        if (activeToast) {
            yOffset += activeToast->height() + 12;
        }
    }
    s_activeToasts.append(toast);

    // Calculate exact Top-Right Target Position inside topWin client area
    QPoint targetPos = calculateTopRightPos(topWin, toast->width(), yOffset);
    QPoint startPos(targetPos.x() + 50, targetPos.y()); // Slide-in off-screen right by 50px
    toast->move(startPos);

    qDebug() << "[FlashNotificationStrategy] 📢 Displaying Flash Notification:" << notification.title()
             << "Target Local Pos:" << targetPos << "topWin:" << topWin;

    // Drop Shadow Effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(toast);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(0, 6);
    toast->setGraphicsEffect(shadow);

    // Opacity Effect for Smooth Fade
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(toast);
    opacityEffect->setOpacity(0.0);
    toast->setGraphicsEffect(opacityEffect);

    // 1. Entrance Animation (Slide In from Right + Fade In)
    QParallelAnimationGroup *entranceGroup = new QParallelAnimationGroup(toast);

    QPropertyAnimation *slideIn = new QPropertyAnimation(toast, "pos", entranceGroup);
    slideIn->setDuration(350);
    slideIn->setStartValue(startPos);
    slideIn->setEndValue(targetPos);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation *fadeIn = new QPropertyAnimation(opacityEffect, "opacity", entranceGroup);
    fadeIn->setDuration(350);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    entranceGroup->addAnimation(slideIn);
    entranceGroup->addAnimation(fadeIn);
    entranceGroup->start(QAbstractAnimation::DeleteWhenStopped);

    // 2. Countdown Progress Bar Animation
    QPropertyAnimation *progressAnim = new QPropertyAnimation(progressBar, "value", toast);
    progressAnim->setDuration(durationMs);
    progressAnim->setStartValue(durationMs);
    progressAnim->setEndValue(0);
    progressAnim->start(QAbstractAnimation::DeleteWhenStopped);

    // Exit Callback & Animation
    QString notifId = notification.id();
    auto startExitAnimation = [toast, opacityEffect, notifId, onDismissed, parent]() {
        QParallelAnimationGroup *exitGroup = new QParallelAnimationGroup(toast);

        QPoint exitPos = toast->pos() + QPoint(50, 0); // Slide right out
        QPropertyAnimation *slideOut = new QPropertyAnimation(toast, "pos", exitGroup);
        slideOut->setDuration(250);
        slideOut->setStartValue(toast->pos());
        slideOut->setEndValue(exitPos);
        slideOut->setEasingCurve(QEasingCurve::InCubic);

        QPropertyAnimation *fadeOut = new QPropertyAnimation(opacityEffect, "opacity", exitGroup);
        fadeOut->setDuration(250);
        fadeOut->setStartValue(opacityEffect->opacity());
        fadeOut->setEndValue(0.0);

        exitGroup->addAnimation(slideOut);
        exitGroup->addAnimation(fadeOut);

        QObject::connect(exitGroup, &QParallelAnimationGroup::finished, toast, [toast, notifId, onDismissed, parent]() {
            s_activeToasts.removeAll(toast);
            if (onDismissed) {
                onDismissed(notifId);
            }
            toast->close();
            // Smoothly reflow remaining active toasts upwards to close the gap
            repositionActiveToasts(parent);
        });

        exitGroup->start(QAbstractAnimation::DeleteWhenStopped);
    };

    // Connect close button
    QObject::connect(closeBtn, &QPushButton::clicked, toast, startExitAnimation);

    // Display modeless toast widget on top of main window stack
    toast->show();
    toast->raise();

    // Single-shot timer to trigger exit animation after duration
    QTimer::singleShot(durationMs, toast, startExitAnimation);
}

} // namespace GISApp::UI::Notifications
