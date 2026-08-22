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

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QMapLibre::MapWidget *m_mapWidget;
    std::vector<GISApp::Core::Models::GeoCoordinate> m_waypoints;
};

} // namespace GISApp::Map

#endif // OVERLAYWIDGET_H
