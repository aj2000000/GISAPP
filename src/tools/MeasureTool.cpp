/**
 * @file MeasureTool.cpp
 * @brief Implementation of Haversine geodesic distance tool.
 */

#include "tools/MeasureTool.h"
#include <cmath>

namespace GISApp::Tools {

MeasureTool::MeasureTool(QObject *parent)
    : QObject(parent), m_active(false)
{
}

void MeasureTool::activate()
{
    m_active = true;
    m_waypoints.clear();
    emit distanceUpdated(0.0);
    emit waypointsUpdated(m_waypoints);
}

void MeasureTool::deactivate()
{
    m_active = false;
    m_waypoints.clear();
    emit waypointsUpdated(m_waypoints);
}

void MeasureTool::onMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate)
{
    if (!m_active || !coordinate.isValid() || event->button() != Qt::LeftButton) {
        return;
    }

    m_waypoints.push_back(coordinate);

    double totalKm = 0.0;
    for (size_t i = 1; i < m_waypoints.size(); ++i) {
        totalKm += calculateHaversineDistance(m_waypoints[i - 1], m_waypoints[i]);
    }

    emit distanceUpdated(totalKm);
    emit waypointsUpdated(m_waypoints);
}


void MeasureTool::onMouseMove(QMouseEvent*, const GISApp::Core::Models::GeoCoordinate&) {}
void MeasureTool::onMouseRelease(QMouseEvent*, const GISApp::Core::Models::GeoCoordinate&) {}

double MeasureTool::calculateHaversineDistance(const GISApp::Core::Models::GeoCoordinate &c1,
                                                const GISApp::Core::Models::GeoCoordinate &c2)
{
    static const double EARTH_RADIUS_KM = 6371.0;

    double lat1Rad = c1.latitude() * M_PI / 180.0;
    double lat2Rad = c2.latitude() * M_PI / 180.0;
    double dLatRad = (c2.latitude() - c1.latitude()) * M_PI / 180.0;
    double dLonRad = (c2.longitude() - c1.longitude()) * M_PI / 180.0;

    double a = std::sin(dLatRad / 2.0) * std::sin(dLatRad / 2.0) +
               std::cos(lat1Rad) * std::cos(lat2Rad) *
               std::sin(dLonRad / 2.0) * std::sin(dLonRad / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}

} // namespace GISApp::Tools
