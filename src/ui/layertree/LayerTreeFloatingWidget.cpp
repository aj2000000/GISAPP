/**
 * @file LayerTreeFloatingWidget.cpp
 * @brief Implementation of floating draggable tactical layer tree widget with ingestion actions.
 */

#include "ui/layertree/LayerTreeFloatingWidget.h"
#include <QGraphicsDropShadowEffect>

namespace GISApp::UI {

LayerTreeFloatingWidget::LayerTreeFloatingWidget(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("LayerTreeFloatingWidget");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(290);

    // Glassmorphic Tactical Styling
    setStyleSheet(
        "QFrame#LayerTreeFloatingWidget {"
        "  background-color: rgba(15, 23, 42, 0.95);"
        "  border: 1px solid #10b981;"
        "  border-radius: 8px;"
        "}"
    );

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 1. Draggable Header Bar
    m_headerBar = new QWidget(this);
    m_headerBar->setStyleSheet(
        "background-color: rgba(30, 41, 59, 0.95);"
        "border-top-left-radius: 7px;"
        "border-top-right-radius: 7px;"
        "border-bottom: 1px solid rgba(255, 255, 255, 0.1);"
    );
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerBar);
    headerLayout->setContentsMargins(10, 6, 8, 6);

    m_titleLabel = new QLabel("🗺️ LAYER MANAGEMENT", m_headerBar);
    m_titleLabel->setStyleSheet("font-weight: bold; color: #10b981; font-size: 11px; letter-spacing: 0.5px;");

    m_collapseButton = new QPushButton("−", m_headerBar);
    m_collapseButton->setFixedSize(20, 20);
    m_collapseButton->setStyleSheet(
        "QPushButton { color: #94a3b8; background: transparent; border: none; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { color: #ffffff; background: rgba(255, 255, 255, 0.1); border-radius: 3px; }"
    );

    m_closeButton = new QPushButton("×", m_headerBar);
    m_closeButton->setFixedSize(20, 20);
    m_closeButton->setStyleSheet(
        "QPushButton { color: #94a3b8; background: transparent; border: none; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { color: #ef4444; background: rgba(239, 68, 68, 0.2); border-radius: 3px; }"
    );

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_collapseButton);
    headerLayout->addWidget(m_closeButton);

    m_mainLayout->addWidget(m_headerBar);

    // 2. Tree View Container
    m_treeView = new LayerTreeView(this);
    m_treeView->setMinimumHeight(240);
    m_mainLayout->addWidget(m_treeView);

    // 3. Bottom Quick Ingestion Button Bar
    m_buttonBar = new QWidget(this);
    m_buttonBar->setStyleSheet(
        "background-color: rgba(15, 23, 42, 0.90);"
        "border-top: 1px solid rgba(255, 255, 255, 0.08);"
        "border-bottom-left-radius: 7px;"
        "border-bottom-right-radius: 7px;"
    );
    QHBoxLayout *btnLayout = new QHBoxLayout(m_buttonBar);
    btnLayout->setContentsMargins(6, 6, 6, 6);
    btnLayout->setSpacing(6);

    m_ingestAovBtn = new QPushButton("📥 Ingest AoV (XML)", m_buttonBar);
    m_ingestAovBtn->setCursor(Qt::PointingHandCursor);
    m_ingestAovBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(0, 136, 255, 0.20);
            color: #00E5FF;
            border: 1px solid #0088FF;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: rgba(0, 136, 255, 0.40);
            color: #ffffff;
        }
    )");

    m_ingestTracksBtn = new QPushButton("📥 Tracks (CSV)", m_buttonBar);
    m_ingestTracksBtn->setCursor(Qt::PointingHandCursor);
    m_ingestTracksBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(16, 185, 129, 0.20);
            color: #10b981;
            border: 1px solid #10b981;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: rgba(16, 185, 129, 0.40);
            color: #ffffff;
        }
    )");

    btnLayout->addWidget(m_ingestAovBtn);
    btnLayout->addWidget(m_ingestTracksBtn);
    m_mainLayout->addWidget(m_buttonBar);

    // 4. Connect Control & Ingestion Signals
    connect(m_collapseButton, &QPushButton::clicked, [this]() {
        m_isCollapsed = !m_isCollapsed;
        m_treeView->setVisible(!m_isCollapsed);
        m_buttonBar->setVisible(!m_isCollapsed);
        m_collapseButton->setText(m_isCollapsed ? "+" : "−");
        adjustSize();
    });

    connect(m_closeButton, &QPushButton::clicked, [this]() {
        hide();
        emit closed();
    });

    connect(m_ingestAovBtn, &QPushButton::clicked, this, &LayerTreeFloatingWidget::ingestAreaOfViewRequested);
    connect(m_ingestTracksBtn, &QPushButton::clicked, this, &LayerTreeFloatingWidget::ingestTracksRequested);

    connect(m_treeView, &LayerTreeView::ingestAreaOfViewRequested, this, &LayerTreeFloatingWidget::ingestAreaOfViewRequested);
    connect(m_treeView, &LayerTreeView::ingestTracksRequested, this, &LayerTreeFloatingWidget::ingestTracksRequested);

    // Drop shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(0, 6);
    setGraphicsEffect(shadow);
}

void LayerTreeFloatingWidget::setLayerManager(GISApp::Layers::LayerManager *manager)
{
    if (m_treeView) {
        m_treeView->setLayerManager(manager);
    }
}

void LayerTreeFloatingWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_headerBar->rect().contains(m_headerBar->mapFrom(this, event->pos()))) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    } else {
        QFrame::mousePressEvent(event);
    }
}

void LayerTreeFloatingWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    } else {
        QFrame::mouseMoveEvent(event);
    }
}

} // namespace GISApp::UI
