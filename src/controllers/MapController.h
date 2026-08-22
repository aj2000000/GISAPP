/**
 * @file MapController.h
 * @brief Application Controller managing map navigation and view state.
 */

#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include "core/interfaces/IMapView.h"
#include "core/models/GeoCoordinate.h"

namespace GISApp::Controllers {

/**
 * @class MapController
 * @brief Controller coordinating map view state, navigation, and user actions.
 */
class MapController : public QObject {
    Q_OBJECT

public:
    explicit MapController(GISApp::Core::Interfaces::IMapView *mapView, QObject *parent = nullptr);
    ~MapController() override = default;

public slots:
    void centerOn(const GISApp::Core::Models::GeoCoordinate &coordinate, double zoomLevel);
    void setStyle(const QString &styleUrl);
    void zoomIn();
    void zoomOut();

private:
    GISApp::Core::Interfaces::IMapView *m_mapView;
};

} // namespace GISApp::Controllers

#endif // MAPCONTROLLER_H
