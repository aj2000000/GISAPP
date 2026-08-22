/**
 * @file IMapView.h
 * @brief Abstract interface for Map View implementations.
 */

#ifndef IMAPVIEW_H
#define IMAPVIEW_H

#include "core/models/GeoCoordinate.h"

namespace GISApp::Core::Interfaces {

class IMapView {
public:
    virtual ~IMapView() = default;

    virtual void setCenter(const GISApp::Core::Models::GeoCoordinate &coordinate, double zoomLevel) = 0;
    virtual void setMapStyle(const char *styleUrl) = 0;
    virtual void updateMap() = 0;

    /**
     * @brief Gets current live zoom level from the map engine.
     */
    virtual double zoom() const = 0;

    /**
     * @brief Sets the zoom level on the map engine.
     * @param zoomLevel Target zoom.
     */
    virtual void setZoom(double zoomLevel) = 0;
};

} // namespace GISApp::Core::Interfaces

#endif // IMAPVIEW_H
