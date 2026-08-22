/**
 * @file MapLibreWidget.cpp
 * @brief Implementation of MapLibreWidget adapter methods and event filtering.
 */

#include "map/MapLibreWidget.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include <QMapLibre/Map>


namespace GISApp::Map {

MapLibreWidget::MapLibreWidget(const QMapLibre::Settings &settings, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_mapWidget = new QMapLibre::MapWidget(settings);
    m_mapWidget->setMouseTracking(true);
    m_mapWidget->installEventFilter(this);

    layout->addWidget(m_mapWidget);
    setMouseTracking(true);

    // Create transparent overlay sitting on top of m_mapWidget
    m_overlayWidget = new OverlayWidget(m_mapWidget, m_mapWidget);
    m_overlayWidget->raise();
    m_mapWidget->installEventFilter(this);
}

QMapLibre::Map* MapLibreWidget::map()
{
    return m_mapWidget ? m_mapWidget->map() : nullptr;
}

void MapLibreWidget::setCenter(const GISApp::Core::Models::GeoCoordinate &coordinate, double zoomLevel)
{
    if (!coordinate.isValid() || !map()) {
        return;
    }

    QMapLibre::Coordinate maplibreCoord(coordinate.latitude(), coordinate.longitude());
    map()->setCoordinateZoom(maplibreCoord, zoomLevel);
}

void MapLibreWidget::setMapStyle(const char *styleUrl)
{
    if (styleUrl && map()) {
        map()->setStyleUrl(QString::fromUtf8(styleUrl));
    }
}

void MapLibreWidget::updateMap()
{
    if (m_mapWidget) {
        m_mapWidget->update();
    }
}

double MapLibreWidget::zoom() const
{
    return (m_mapWidget && m_mapWidget->map()) ? m_mapWidget->map()->zoom() : 0.0;
}

void MapLibreWidget::setZoom(double zoomLevel)
{
    if (m_mapWidget && m_mapWidget->map()) {
        m_mapWidget->map()->setZoom(zoomLevel);
    }
}


bool MapLibreWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_mapWidget && map()) {
        if (m_overlayWidget) {
            m_overlayWidget->update();
        }
        
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QMapLibre::Coordinate coord = map()->coordinateForPixel(mouseEvent->position());
            GISApp::Core::Models::GeoCoordinate geoCoord(coord.first, coord.second);
            emit mouseCoordinateChanged(geoCoord);
            emit mouseMoved(mouseEvent, geoCoord);
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QMapLibre::Coordinate coord = map()->coordinateForPixel(mouseEvent->position());
            GISApp::Core::Models::GeoCoordinate geoCoord(coord.first, coord.second);
            emit mousePressed(mouseEvent, geoCoord);
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QMapLibre::Coordinate coord = map()->coordinateForPixel(mouseEvent->position());
            GISApp::Core::Models::GeoCoordinate geoCoord(coord.first, coord.second);
            emit mouseReleased(mouseEvent, geoCoord);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MapLibreWidget::setWaypoints(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints)
{
    if (m_overlayWidget) {
        m_overlayWidget->setWaypoints(waypoints);
    }
}


void MapLibreWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_overlayWidget && m_mapWidget) {
        m_overlayWidget->resize(m_mapWidget->size());
    }
}
} // namespace GISApp::Map
