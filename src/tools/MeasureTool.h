/**
 * @file MeasureTool.h
 * @brief Concrete GIS Tool calculating Haversine geodesic distance between points.
 */

#ifndef MEASURETOOL_H
#define MEASURETOOL_H

#include <QObject>
#include <vector>
#include "core/interfaces/ITool.h"
#include "core/models/GeoCoordinate.h"

namespace GISApp::Tools {

/**
 * @class MeasureTool
 * @brief Strategy tool calculating real-world distance across map waypoints.
 */
class MeasureTool : public QObject, public GISApp::Core::Interfaces::ITool {
    Q_OBJECT

public:
    explicit MeasureTool(QObject *parent = nullptr);
    ~MeasureTool() override = default;

    QString toolName() const override { return "MeasureTool"; }

    void activate() override;
    void deactivate() override;

    void onMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;
    void onMouseMove(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;
    void onMouseRelease(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;

    /**
     * @brief Calculates Haversine geodesic distance in kilometers between two coordinates.
     */
    static double calculateHaversineDistance(const GISApp::Core::Models::GeoCoordinate &c1,
                                             const GISApp::Core::Models::GeoCoordinate &c2);

signals:
    /**
     * @brief Emitted when distance measurement updates.
     * @param totalDistanceKm Total accumulated distance in kilometers.
     */
    void distanceUpdated(double totalDistanceKm);
    void waypointsUpdated(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints);

private:
    std::vector<GISApp::Core::Models::GeoCoordinate> m_waypoints;
    bool m_active;
};

} // namespace GISApp::Tools

#endif // MEASURETOOL_H
