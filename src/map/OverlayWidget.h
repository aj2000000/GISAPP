/**
 * @file OverlayWidget.h
 * @brief Transparent overlay layer drawing measurement lines and markers over MapLibre OpenGL canvas.
 */

#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <vector>
#include <qmaplibrewidgets.hpp>
#include "core/models/GeoCoordinate.h"

#include <QColor>
#include "src/ui/udl/UdlEntityStyleDialog.h"

namespace GISApp::Map {

/**
 * @class OverlayWidget
 * @brief Translucent overlay passing mouse events to MapLibre while painting graphics on top.
 */
class OverlayWidget : public QWidget {
    Q_OBJECT

public:
    explicit OverlayWidget(QMapLibre::MapWidget *mapWidget, QWidget *parent = nullptr);
    ~OverlayWidget() override = default;

    void setWaypoints(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints);
    void setUdlPreview(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints,
                       const GISApp::Core::Models::GeoCoordinate &mouseCoord,
                       GISApp::UI::UDL::UdlGeometryType geomType,
                       bool active,
                       const QColor &strokeColor,
                       const QColor &fillColor);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QMapLibre::MapWidget *m_mapWidget;
    std::vector<GISApp::Core::Models::GeoCoordinate> m_waypoints;

    // UDL Real-time preview state
    std::vector<GISApp::Core::Models::GeoCoordinate> m_udlWaypoints;
    GISApp::Core::Models::GeoCoordinate m_udlMouseCoord;
    GISApp::UI::UDL::UdlGeometryType m_udlGeomType{GISApp::UI::UDL::UdlGeometryType::Point};
    bool m_udlPreviewActive{false};
    QColor m_udlStrokeColor{QColor("#f59e0b")};
    QColor m_udlFillColor{QColor("#ff9933")};
};

} // namespace GISApp::Map

#endif // OVERLAYWIDGET_H
