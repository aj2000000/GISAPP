/**
 * @file MapController.cpp
 * @brief Implementation of MapController logic querying live map zoom state.
 */

#include "controllers/MapController.h"

namespace GISApp::Controllers {

MapController::MapController(GISApp::Core::Interfaces::IMapView *mapView, QObject *parent)
    : QObject(parent), m_mapView(mapView)
{
}

void MapController::centerOn(const GISApp::Core::Models::GeoCoordinate &coordinate, double zoomLevel)
{
    if (m_mapView && coordinate.isValid()) {
        m_mapView->setCenter(coordinate, zoomLevel);
    }
}

void MapController::setStyle(const QString &styleUrl)
{
    if (m_mapView && !styleUrl.isEmpty()) {
        m_mapView->setMapStyle(styleUrl.toUtf8().constData());
    }
}

void MapController::zoomIn()
{
    if (m_mapView) {
        double currentZoom = m_mapView->zoom();
        m_mapView->setZoom(currentZoom + 1.0);
    }
}

void MapController::zoomOut()
{
    if (m_mapView) {
        double currentZoom = m_mapView->zoom();
        if (currentZoom > 1.0) {
            m_mapView->setZoom(currentZoom - 1.0);
        }
    }
}

} // namespace GISApp::Controllers
