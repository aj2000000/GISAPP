/**
 * @file CriticalNotificationStrategy.cpp
 * @brief Implementation of Critical notification modal dialog presentation strategy.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "ui/notifications/CriticalNotificationStrategy.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

namespace GISApp::UI::Notifications {

void CriticalNotificationStrategy::display(const GISApp::Core::Notifications::Notification &notification,
                                           QWidget *parent,
                                           DismissCallback onDismissed)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->setMinimumWidth(420);

    // Styling: Dark tactical theme with critical red header accent
    dialog->setStyleSheet(R"(
        QDialog {
            background-color: #1e293b;
            border: 2px solid #ef4444;
            border-radius: 8px;
        }
        QLabel#TitleLabel {
            color: #ef4444;
            font-size: 16px;
            font-weight: bold;
        }
        QLabel#MessageLabel {
            color: #f8fafc;
            font-size: 13px;
            line-height: 1.4;
        }
        QLabel#TimeLabel {
            color: #94a3b8;
            font-size: 11px;
        }
        QPushButton#AckButton {
            background-color: #dc2626;
            color: #ffffff;
            font-weight: bold;
            font-size: 13px;
            border: none;
            border-radius: 4px;
            padding: 8px 20px;
        }
        QPushButton#AckButton:hover {
            background-color: #b91c1c;
        }
        QPushButton#AckButton:pressed {
            background-color: #991b1b;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    // Header with Icon & Title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel("🚨", dialog);
    iconLabel->setStyleSheet("font-size: 24px;");

    QLabel *titleLabel = new QLabel(notification.title(), dialog);
    titleLabel->setObjectName("TitleLabel");

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel, 1);
    mainLayout->addLayout(headerLayout);

    // Separator line
    QFrame *line = new QFrame(dialog);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #334155;");
    mainLayout->addWidget(line);

    // Message Content
    QLabel *messageLabel = new QLabel(notification.message(), dialog);
    messageLabel->setObjectName("MessageLabel");
    messageLabel->setWordWrap(true);
    mainLayout->addWidget(messageLabel);

    // Timestamp
    QLabel *timeLabel = new QLabel(QString("Timestamp: %1").arg(notification.timestamp().toString("yyyy-MM-dd hh:mm:ss")), dialog);
    timeLabel->setObjectName("TimeLabel");
    mainLayout->addWidget(timeLabel);

    // Footer with Acknowledge Button
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->addStretch();

    QPushButton *ackBtn = new QPushButton("✓ ACKNOWLEDGE", dialog);
    ackBtn->setObjectName("AckButton");
    ackBtn->setCursor(Qt::PointingHandCursor);

    QString notifId = notification.id();
    connect(ackBtn, &QPushButton::clicked, dialog, [dialog, notifId, onDismissed]() {
        if (onDismissed) {
            onDismissed(notifId);
        }
        dialog->accept();
    });

    footerLayout->addWidget(ackBtn);
    mainLayout->addLayout(footerLayout);

    // Drop shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialog);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(239, 68, 68, 80));
    shadow->setOffset(0, 4);
    dialog->setGraphicsEffect(shadow);

    dialog->exec();
}

} // namespace GISApp::UI::Notifications
