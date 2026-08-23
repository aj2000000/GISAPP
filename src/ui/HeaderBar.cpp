/**
 * @file HeaderBar.cpp
 * @brief Implementation of HeaderBar layout and action controls.
 */

#include "ui/HeaderBar.h"

namespace GISApp::UI {

HeaderBar::HeaderBar(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("HeaderBar");
    setFixedHeight(48);
    setupUi();
}

void HeaderBar::setupUi()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 4, 16, 4);

    // --- Left Title & Subtitle Info ---
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText(
        "<span style='color:#10b981; font-weight:bold; font-size:14px;'>GIS-DSS</span> "
        "<span style='color:#4b5563;'>|</span> "
        "<span style='color:#e5e7eb; font-weight:500; font-size:14px;'>Acme Widgets Inc.</span><br/>"
        "<span style='color:#6b7280; font-size:11px;'>v2.4.0 • System Operator</span>"
    );

    layout->addWidget(titleLabel);
    layout->addStretch();

    // --- Right Action Tools ---
    QPushButton *zonesBtn = createActionButton("All Zones Visible ▾", "Toggle Zones");
    QPushButton *undoBtn  = createActionButton("↶", "Undo Action");
    QPushButton *dlBtn    = createActionButton("📥", "Export Data");
    QPushButton *playBtn  = createActionButton("▶", "Play Simulation");
    QPushButton *notifBtn = createActionButton("🔔", "Notifications");

    layout->addWidget(zonesBtn);
    layout->addWidget(undoBtn);
    layout->addWidget(dlBtn);
    layout->addWidget(playBtn);
    layout->addWidget(notifBtn);
}

QPushButton* HeaderBar::createActionButton(const QString &text, const QString &tooltip)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setToolTip(tooltip);
    btn->setObjectName("HeaderActionButton");
    btn->setMinimumHeight(28);
    return btn;
}

} // namespace GISApp::UI
