/**
 * @file UdlDrawingTool.h
 * @brief Interactive GIS Tool for digitizing Point, Polyline, Polygon, and Text UDL entities.
 */

#ifndef UDLDRAWINGTOOL_H
#define UDLDRAWINGTOOL_H

#include <QObject>
#include <vector>
#include <QColor>
#include "core/interfaces/ITool.h"
#include "core/models/GeoCoordinate.h"
#include "src/ui/udl/UdlEntityStyleDialog.h"

namespace GISApp::Tools {

class UdlDrawingTool : public QObject, public GISApp::Core::Interfaces::ITool {
    Q_OBJECT

public:
    explicit UdlDrawingTool(QObject *parent = nullptr);
    ~UdlDrawingTool() override = default;

    QString toolName() const override { return "UdlDrawingTool"; }

    void activate() override;
    void deactivate() override;

    void setTargetLayerId(const QString &layerId) { m_layerId = layerId; }
    QString targetLayerId() const { return m_layerId; }

    void setGeometryType(GISApp::UI::UDL::UdlGeometryType type) { m_geomType = type; }
    GISApp::UI::UDL::UdlGeometryType geometryType() const { return m_geomType; }

    void setContinuousMode(bool enabled) { m_isContinuous = enabled; }
    bool isContinuousMode() const { return m_isContinuous; }

    void setQuickNameMode(bool enabled) { m_isQuickName = enabled; }
    bool isQuickNameMode() const { return m_isQuickName; }

    void setPresetColors(const QColor &stroke, const QColor &fill) {
        m_strokeColor = stroke;
        m_fillColor = fill;
        if (!m_waypoints.empty()) {
            emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, m_active, m_strokeColor, m_fillColor);
        }
    }

    void setPendingTextLabel(const QString &label) { m_pendingTextLabel = label; }
    QString pendingTextLabel() const { return m_pendingTextLabel; }

    bool hasWaypoints() const { return !m_waypoints.empty(); }
    size_t waypointsCount() const { return m_waypoints.size(); }

    void undoLastPoint();
    void finishShape();
    void clearWaypoints();

    void onMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;
    void onMouseMove(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;
    void onMouseRelease(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;
    void onMouseDoubleClick(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) override;

signals:
    void waypointsUpdated(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints);
    void previewUpdated(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints,
                        const GISApp::Core::Models::GeoCoordinate &mouseCoord,
                        GISApp::UI::UDL::UdlGeometryType geomType,
                        bool active,
                        const QColor &strokeColor,
                        const QColor &fillColor);
    void entityCreated(const QString &entityId, const QString &layerId);

private:
    QString m_layerId;
    QString m_pendingTextLabel;
    GISApp::UI::UDL::UdlGeometryType m_geomType{GISApp::UI::UDL::UdlGeometryType::Point};
    bool m_active{false};
    bool m_isContinuous{true};
    bool m_isQuickName{true};
    QColor m_strokeColor{"#f59e0b"};
    QColor m_fillColor{"#ff9933"};
    std::vector<GISApp::Core::Models::GeoCoordinate> m_waypoints;
    GISApp::Core::Models::GeoCoordinate m_currentMouseCoord;
    int m_entityCounter{1};

    QPoint m_dragStartPos;
    GISApp::Core::Models::GeoCoordinate m_dragStartCoord;
    bool m_isLeftPressed{false};
    bool m_isDragging{false};
};

} // namespace GISApp::Tools

#endif // UDLDRAWINGTOOL_H
