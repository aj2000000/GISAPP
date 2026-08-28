/**
 * @file MapLibreWidget.h
 * @brief Concrete Map Widget adapter using transparent OverlayWidget for graphic overlays.
 */

#ifndef MAPLIBREWIDGET_H
#define MAPLIBREWIDGET_H

#include <QWidget>
#include <qmaplibrewidgets.hpp>
#include "core/interfaces/IMapView.h"
#include "core/models/GeoCoordinate.h"
#include "map/OverlayWidget.h"

namespace GISApp::Map {

class MapLibreWidget : public QWidget, public GISApp::Core::Interfaces::IMapView {
    Q_OBJECT

public:
    explicit MapLibreWidget(const QMapLibre::Settings &settings = QMapLibre::Settings(), QWidget *parent = nullptr);
    ~MapLibreWidget() override = default;

    // --- IMapView Interface Implementation ---
    void setCenter(const GISApp::Core::Models::GeoCoordinate &coordinate, double zoomLevel) override;
    void setMapStyle(const char *styleUrl) override;
    void updateMap() override;
    double zoom() const override;
    void setZoom(double zoomLevel) override;

    void setWaypoints(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints);
    QMapLibre::Map* map();

signals:
    void mouseCoordinateChanged(const GISApp::Core::Models::GeoCoordinate &coordinate);
    void mouseMoved(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate);
    void mousePressed(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate);
    void mouseReleased(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate);
    void cameraChanged(double zoomLevel, double scaleDenominator, const GISApp::Core::Models::GeoCoordinate &center);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void emitCameraChanged();

    QMapLibre::MapWidget *m_mapWidget;
    OverlayWidget *m_overlayWidget;
};

} // namespace GISApp::Map

#endif // MAPLIBREWIDGET_H
