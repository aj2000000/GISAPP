/**
 * @file UdlDrawingTool.cpp
 * @brief Implementation of UdlDrawingTool.
 */

#include "src/tools/UdlDrawingTool.h"
#include "src/publishing/UdlRepositoryManager.h"
#include "src/ui/udl/UdlEntityStyleDialog.h"
#include <QMouseEvent>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QInputDialog>
#include <cmath>

namespace GISApp::Tools {

static double calculateDistanceMeters(const GISApp::Core::Models::GeoCoordinate &c1,
                                     const GISApp::Core::Models::GeoCoordinate &c2)
{
    constexpr double earthRadiusMeters = 6371000.0;
    double lat1Rad = c1.latitude() * M_PI / 180.0;
    double lat2Rad = c2.latitude() * M_PI / 180.0;
    double dLatRad = (c2.latitude() - c1.latitude()) * M_PI / 180.0;
    double dLonRad = (c2.longitude() - c1.longitude()) * M_PI / 180.0;

    double a = std::sin(dLatRad / 2.0) * std::sin(dLatRad / 2.0) +
               std::cos(lat1Rad) * std::cos(lat2Rad) *
               std::sin(dLonRad / 2.0) * std::sin(dLonRad / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return earthRadiusMeters * c;
}

static std::vector<GISApp::Core::Models::GeoCoordinate> generateCirclePolygon(
    const GISApp::Core::Models::GeoCoordinate &center, double radiusMeters, int numPoints = 64)
{
    std::vector<GISApp::Core::Models::GeoCoordinate> circlePoints;
    circlePoints.reserve(numPoints + 1);

    constexpr double earthRadiusMeters = 6378137.0;
    double lat1Rad = center.latitude() * (M_PI / 180.0);
    double lon1Rad = center.longitude() * (M_PI / 180.0);
    double dRad = radiusMeters / earthRadiusMeters;

    for (int i = 0; i <= numPoints; ++i) {
        double bearing = (i * 360.0 / numPoints) * (M_PI / 180.0);
        double lat2Rad = std::asin(std::sin(lat1Rad) * std::cos(dRad) +
                                  std::cos(lat1Rad) * std::sin(dRad) * std::cos(bearing));
        double lon2Rad = lon1Rad + std::atan2(std::sin(bearing) * std::sin(dRad) * std::cos(lat1Rad),
                                              std::cos(dRad) - std::sin(lat1Rad) * std::sin(lat2Rad));
        
        circlePoints.emplace_back(lat2Rad * (180.0 / M_PI), lon2Rad * (180.0 / M_PI));
    }
    return circlePoints;
}

UdlDrawingTool::UdlDrawingTool(QObject *parent)
    : QObject(parent)
{
}

void UdlDrawingTool::activate() {
    m_active = true;
    m_waypoints.clear();
    m_isLeftPressed = false;
    m_isDragging = false;
    emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, true, m_strokeColor, m_fillColor);
    qDebug() << "[UdlDrawingTool] Activated for Layer:" << m_layerId << "GeomType:" << static_cast<int>(m_geomType);
}

void UdlDrawingTool::deactivate() {
    m_active = false;
    m_waypoints.clear();
    m_isLeftPressed = false;
    m_isDragging = false;
    emit waypointsUpdated(m_waypoints);
    emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, false, m_strokeColor, m_fillColor);
    qDebug() << "[UdlDrawingTool] Deactivated.";
}

void UdlDrawingTool::undoLastPoint() {
    if (!m_waypoints.empty()) {
        m_waypoints.pop_back();
        emit waypointsUpdated(m_waypoints);
        emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, m_active, m_strokeColor, m_fillColor);
        qDebug() << "[UdlDrawingTool] Undid last waypoint. Remaining points:" << m_waypoints.size();
    }
}

void UdlDrawingTool::clearWaypoints() {
    m_waypoints.clear();
    emit waypointsUpdated(m_waypoints);
    emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, m_active, m_strokeColor, m_fillColor);
    qDebug() << "[UdlDrawingTool] Cleared waypoints.";
}

void UdlDrawingTool::onMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) {
    if (!m_active || m_layerId.isEmpty()) return;

    m_currentMouseCoord = coordinate;

    if (event->button() == Qt::RightButton) {
        // Right click finishes polyline or polygon, or pops/clears points
        if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Polyline && m_waypoints.size() >= 2) {
            finishShape();
        } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Polygon && m_waypoints.size() >= 3) {
            finishShape();
        } else if (!m_waypoints.empty()) {
            undoLastPoint();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_isLeftPressed = true;
        m_isDragging = false;
        m_dragStartPos = event->pos();
        m_dragStartCoord = coordinate;
    }
}

void UdlDrawingTool::onMouseMove(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) {
    if (!m_active) return;
    m_currentMouseCoord = coordinate;

    if (m_isLeftPressed && !m_isDragging) {
        int delta = (event->pos() - m_dragStartPos).manhattanLength();
        if (delta > 6) { // Drag threshold to distinguish map panning from point clicking
            m_isDragging = true;
        }
    }

    emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, true, m_strokeColor, m_fillColor);
}

void UdlDrawingTool::onMouseRelease(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) {
    if (!m_active || m_layerId.isEmpty()) return;

    if (event->button() == Qt::LeftButton && m_isLeftPressed) {
        m_isLeftPressed = false;

        if (m_isDragging) {
            // User was dragging to pan the map; do not treat as waypoint click
            m_isDragging = false;
            return;
        }

        // Clean click -> add point to waypoints
        m_waypoints.push_back(m_dragStartCoord);
        emit waypointsUpdated(m_waypoints);
        emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, true, m_strokeColor, m_fillColor);

        if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Point || m_geomType == GISApp::UI::UDL::UdlGeometryType::Text) {
            finishShape();
        } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Circle && m_waypoints.size() >= 2) {
            finishShape();
        }
    }
}

void UdlDrawingTool::onMouseDoubleClick(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) {
    Q_UNUSED(coordinate);
    if (!m_active || m_layerId.isEmpty()) return;

    if (event->button() == Qt::LeftButton) {
        if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Polyline) {
            // Remove duplicate waypoint if 2nd click of double-click added the same point twice
            if (m_waypoints.size() >= 3 && calculateDistanceMeters(m_waypoints.back(), m_waypoints[m_waypoints.size() - 2]) < 20.0) {
                m_waypoints.pop_back();
            }
            if (m_waypoints.size() >= 2) {
                finishShape();
            }
        } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Polygon) {
            // Remove duplicate waypoint if 2nd click of double-click added the same point twice
            if (m_waypoints.size() >= 4 && calculateDistanceMeters(m_waypoints.back(), m_waypoints[m_waypoints.size() - 2]) < 20.0) {
                m_waypoints.pop_back();
            }
            if (m_waypoints.size() >= 3) {
                finishShape();
            }
        }
    }
}

void UdlDrawingTool::finishShape() {
    if (m_waypoints.empty() || m_layerId.isEmpty()) return;

    QString entityId = QString("udl_ent_%1").arg(QDateTime::currentMSecsSinceEpoch());
    QString entityName;
    QString entityTypeStr;
    QJsonObject geomObj;
    QJsonObject styleObj;

    styleObj["strokeColor"] = m_strokeColor.name();
    styleObj["fillColor"] = m_fillColor.name();
    styleObj["fillOpacity"] = 0.35;
    styleObj["lineWidth"] = 3;

    if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Point) {
        entityTypeStr = "Point";
        entityName = QString("Point #%1").arg(m_entityCounter++);
        geomObj["type"] = "Point";
        QJsonArray coords;
        coords.append(m_waypoints[0].longitude());
        coords.append(m_waypoints[0].latitude());
        geomObj["coordinates"] = coords;
    } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Polyline) {
        entityTypeStr = "Polyline";
        entityName = QString("Line #%1").arg(m_entityCounter++);
        geomObj["type"] = "LineString";
        QJsonArray coordsArr;
        for (const auto &wpt : m_waypoints) {
            QJsonArray pt;
            pt.append(wpt.longitude());
            pt.append(wpt.latitude());
            coordsArr.append(pt);
        }
        geomObj["coordinates"] = coordsArr;
    } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Polygon) {
        entityTypeStr = "Polygon";
        entityName = QString("Polygon #%1").arg(m_entityCounter++);
        geomObj["type"] = "Polygon";
        QJsonArray ringArr;
        for (const auto &wpt : m_waypoints) {
            QJsonArray pt;
            pt.append(wpt.longitude());
            pt.append(wpt.latitude());
            ringArr.append(pt);
        }
        // Close ring if not closed
        if (!m_waypoints.empty()) {
            QJsonArray pt;
            pt.append(m_waypoints[0].longitude());
            pt.append(m_waypoints[0].latitude());
            ringArr.append(pt);
        }
        QJsonArray polygonCoords;
        polygonCoords.append(ringArr);
        geomObj["coordinates"] = polygonCoords;
    } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Circle) {
        if (m_waypoints.size() < 2) return;
        entityTypeStr = "Circle";
        entityName = QString("Circle #%1").arg(m_entityCounter++);

        double radiusMeters = calculateDistanceMeters(m_waypoints[0], m_waypoints[1]);
        auto circleRing = generateCirclePolygon(m_waypoints[0], radiusMeters, 64);

        geomObj["type"] = "Polygon";
        QJsonArray ringArr;
        for (const auto &wpt : circleRing) {
            QJsonArray pt;
            pt.append(wpt.longitude());
            pt.append(wpt.latitude());
            ringArr.append(pt);
        }
        QJsonArray polygonCoords;
        polygonCoords.append(ringArr);
        geomObj["coordinates"] = polygonCoords;

        styleObj["radiusMeters"] = radiusMeters;
        styleObj["centerLon"] = m_waypoints[0].longitude();
        styleObj["centerLat"] = m_waypoints[0].latitude();
    } else if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Text) {
        entityTypeStr = "Text";

        geomObj["type"] = "Point";
        QJsonArray coords;
        coords.append(m_waypoints[0].longitude());
        coords.append(m_waypoints[0].latitude());
        geomObj["coordinates"] = coords;

        QString defaultLabelText = m_pendingTextLabel.trimmed();
        if (defaultLabelText.isEmpty()) {
            defaultLabelText = QString("Text Label #%1").arg(m_entityCounter);
        }

        GISApp::UI::UDL::UdlEntityStyleDialog dlg(m_geomType);
        dlg.setWindowTitle(tr("Add Text Label & Font Properties"));
        dlg.setEntityName(defaultLabelText);
        dlg.setTextContent(defaultLabelText);
        dlg.setTextColor(m_strokeColor);
        dlg.setBorderColor(m_strokeColor);
        dlg.setBgColor(QColor("#0f172a"));
        dlg.setBgOpacity(0.85);

        if (dlg.exec() == QDialog::Accepted) {
            entityName = dlg.entityName().isEmpty() ? defaultLabelText : dlg.entityName();
            styleObj = dlg.styleJsonObject();
            if (styleObj.value("textContent").toString().trimmed().isEmpty()) {
                styleObj["textContent"] = entityName;
            }
            m_entityCounter++;
        } else {
            m_waypoints.clear();
            emit waypointsUpdated(m_waypoints);
            emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, m_active, m_strokeColor, m_fillColor);
            return;
        }
    }

    if (!m_isQuickName && m_geomType != GISApp::UI::UDL::UdlGeometryType::Text) {
        GISApp::UI::UDL::UdlEntityStyleDialog dlg(m_geomType);
        dlg.setEntityName(entityName);
        dlg.setStrokeColor(m_strokeColor);
        dlg.setFillColor(m_fillColor);
        if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Text) {
            dlg.setTextContent(entityName);
            dlg.setTextColor(m_strokeColor);
            dlg.setBorderColor(m_strokeColor);
        }
        dlg.setStyleJsonObject(styleObj);

        if (dlg.exec() == QDialog::Accepted) {
            if (!dlg.entityName().isEmpty()) entityName = dlg.entityName();
            styleObj = dlg.styleJsonObject();
            if (m_geomType == GISApp::UI::UDL::UdlGeometryType::Text && styleObj.value("textContent").toString().isEmpty()) {
                styleObj["textContent"] = entityName;
            }
        } else {
            m_waypoints.clear();
            emit waypointsUpdated(m_waypoints);
            emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, m_active, m_strokeColor, m_fillColor);
            return;
        }
    }

    GISApp::Publishing::UdlEntityItem item;
    item.entityId = entityId;
    item.layerId = m_layerId;
    item.entityName = entityName;
    item.entityType = entityTypeStr;
    item.geometryJson = geomObj;
    item.styleJson = styleObj;
    item.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    GISApp::Publishing::UdlRepositoryManager::instance().saveEntity(item);
    qDebug() << "[UdlDrawingTool] Created entity:" << entityName << "on layer:" << m_layerId;
    emit entityCreated(entityId, m_layerId);

    m_waypoints.clear();
    emit waypointsUpdated(m_waypoints);
    emit previewUpdated(m_waypoints, m_currentMouseCoord, m_geomType, m_active && m_isContinuous, m_strokeColor, m_fillColor);

    if (!m_isContinuous) {
        deactivate();
    }
}

} // namespace GISApp::Tools
