/**
 * @file OverlayWidget.cpp
 * @brief Implementation of transparent measurement lines and marker painter.
 */

#include "map/OverlayWidget.h"
#include <QPainter>
#include <QPen>

namespace GISApp::Map {

OverlayWidget::OverlayWidget(QMapLibre::MapWidget *mapWidget, QWidget *parent)
    : QWidget(parent ? parent : mapWidget), m_mapWidget(mapWidget)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    if (mapWidget) {
        resize(mapWidget->size());
    }
}

void OverlayWidget::setWaypoints(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints)
{
    m_waypoints = waypoints;
    update();
}

void OverlayWidget::paintEvent(QPaintEvent*)
{
    if (m_waypoints.empty() || !m_mapWidget || !m_mapWidget->map()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Red polyline pen for distance line
    QPen linePen(QColor(255, 40, 40), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(linePen);

    QPolygonF polyline;
    for (const auto &coord : m_waypoints) {
        QPointF pixel = m_mapWidget->map()->pixelForCoordinate({coord.latitude(), coord.longitude()});
        polyline.append(pixel);
    }

    if (polyline.size() > 1) {
        painter.drawPolyline(polyline);
    }

    // 2. Target marker dots at each clicked point
    painter.setBrush(QColor(255, 255, 255));
    QPen borderPen(QColor(255, 40, 40), 2);
    painter.setPen(borderPen);

    for (const auto &pt : polyline) {
        painter.drawEllipse(pt, 6, 6);
    }
}

} // namespace GISApp::Map
