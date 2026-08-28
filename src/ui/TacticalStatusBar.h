/**
 * @file TacticalStatusBar.h
 * @brief High-precision tactical status bar displaying spatial system metrics and 3D coordinates.
 *
 * @details Implements Single Responsibility Principle (SRP). Displays WGS84 spatial reference, 
 * scale, and 3D geographic telemetry (Latitude, Longitude, Altitude/Height).
 */

#ifndef TACTICALSTATUSBAR_H
#define TACTICALSTATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include "core/models/GeoCoordinate.h"

namespace GISApp::UI {

/**
 * @class TacticalStatusBar
 * @brief Custom status bar footer displaying 3D geographic telemetry.
 */
class TacticalStatusBar : public QStatusBar {
    Q_OBJECT

public:
    /**
     * @brief Constructor for TacticalStatusBar.
     * @param parent Qt parent widget.
     */
    explicit TacticalStatusBar(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~TacticalStatusBar() override = default;

public slots:
    /**
     * @brief Updates 3D geographic coordinates (Latitude, Longitude, Altitude/Height).
     * @param coord GeoCoordinate Value Object.
     */
    void updateCoordinates(const GISApp::Core::Models::GeoCoordinate &coord);

    /**
     * @brief Updates displayed map scale denominator.
     * @param scaleDenominator Scale value (e.g. 15000000).
     */
    void updateScale(double scaleDenominator);

    /**
     * @brief Updates displayed zoom level and map scale denominator.
     * @param zoomLevel Current camera zoom level (e.g. 10.5).
     * @param scaleDenominator Scale value (e.g. 507210).
     */
    void updateZoomAndScale(double zoomLevel, double scaleDenominator);

private:
    QLabel *m_infoLabel;  ///< Left metadata label (Spatial Reference & Scale)
    QLabel *m_coordLabel; ///< Right glowing 3D coordinate telemetry label
};

} // namespace GISApp::UI

#endif // TACTICALSTATUSBAR_H
