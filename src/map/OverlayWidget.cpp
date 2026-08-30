/**
 * @file OverlayWidget.cpp
 * @brief Implementation of transparent measurement lines, UDL previews, and marker painter.
 */

#include "map/OverlayWidget.h"
#include "src/publishing/UdlRepositoryManager.h"
#include <QPainter>
#include <QPen>
#include <QFontMetrics>
#include <QJsonArray>
#include <cmath>

namespace GISApp::Map {

static double calculateHaversineDistanceMeters(
    const GISApp::Core::Models::GeoCoordinate &c1,
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

static QString formatDistanceStr(double meters)
{
    if (meters >= 1000.0) {
        return QString("%1 km").arg(meters / 1000.0, 0, 'f', 2);
    }
    return QString("%1 m").arg(qRound(meters));
}

static std::vector<GISApp::Core::Models::GeoCoordinate> createGeodesicCircle(
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

OverlayWidget::OverlayWidget(QMapLibre::MapWidget *mapWidget, QWidget *parent)
    : QWidget(parent ? parent : mapWidget), m_mapWidget(mapWidget)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    if (mapWidget) {
        resize(mapWidget->size());
    }
}

void OverlayWidget::setWaypoints(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints)
{
    m_waypoints = waypoints;
    update();
}

void OverlayWidget::setUdlPreview(const std::vector<GISApp::Core::Models::GeoCoordinate> &waypoints,
                                 const GISApp::Core::Models::GeoCoordinate &mouseCoord,
                                 GISApp::UI::UDL::UdlGeometryType geomType,
                                 bool active,
                                 const QColor &strokeColor,
                                 const QColor &fillColor)
{
    m_udlWaypoints = waypoints;
    m_udlMouseCoord = mouseCoord;
    m_udlGeomType = geomType;
    m_udlPreviewActive = active;
    m_udlStrokeColor = strokeColor;
    m_udlFillColor = fillColor;
    update();
}

void OverlayWidget::paintEvent(QPaintEvent*)
{
    if (!m_mapWidget || !m_mapWidget->map()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Red polyline pen for measure distance line
    if (!m_waypoints.empty()) {
        QPen linePen(QColor(255, 40, 40), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(linePen);

        QPolygonF polyline;
        for (const auto &coord : m_waypoints) {
            QPointF pixel = m_mapWidget->map()->pixelForCoordinate({coord.latitude(), coord.longitude()});
            polyline.append(pixel);
        }

        if (polyline.size() > 1) {
            painter.drawPolyline(polyline);
        }

        painter.setBrush(QColor(255, 255, 255));
        QPen borderPen(QColor(255, 40, 40), 2);
        painter.setPen(borderPen);

        for (const auto &pt : polyline) {
            painter.drawEllipse(pt, 6, 6);
        }
    }

    // 2. Real-time UDL preview rendering
    if (m_udlPreviewActive) {
        QColor stroke = m_udlStrokeColor.isValid() ? m_udlStrokeColor : QColor("#f59e0b");
        QColor fill = m_udlFillColor.isValid() ? m_udlFillColor : QColor("#ff9933");
        fill.setAlpha(90);

        QPen solidPen(stroke, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen dashPen(stroke, 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);

        QPointF mousePx;
        bool hasMouse = m_udlMouseCoord.isValid();
        if (hasMouse) {
            mousePx = m_mapWidget->map()->pixelForCoordinate({m_udlMouseCoord.latitude(), m_udlMouseCoord.longitude()});
        }

        QPolygonF waypointsPx;
        for (const auto &wpt : m_udlWaypoints) {
            waypointsPx.append(m_mapWidget->map()->pixelForCoordinate({wpt.latitude(), wpt.longitude()}));
        }

        if (m_udlGeomType == GISApp::UI::UDL::UdlGeometryType::Point) {
            if (hasMouse) {
                painter.setPen(QPen(stroke, 1.5, Qt::DashLine));
                painter.drawLine(mousePx.x() - 15, mousePx.y(), mousePx.x() + 15, mousePx.y());
                painter.drawLine(mousePx.x(), mousePx.y() - 15, mousePx.x(), mousePx.y() + 15);

                painter.setPen(QPen(Qt::white, 2));
                painter.setBrush(stroke);
                painter.drawEllipse(mousePx, 7, 7);
            }
        }
        else if (m_udlGeomType == GISApp::UI::UDL::UdlGeometryType::Text) {
            if (hasMouse) {
                painter.setPen(QPen(stroke, 1.5, Qt::DashLine));
                painter.drawEllipse(mousePx, 5, 5);

                QString badgeText = tr("🔤 Place Label Here");
                QFont font("sans-serif", 10, QFont::Bold);
                painter.setFont(font);
                QFontMetrics fm(font);
                int tw = fm.horizontalAdvance(badgeText) + 12;
                int th = fm.height() + 6;

                QRectF bgRect(mousePx.x() + 12, mousePx.y() - th / 2.0, tw, th);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(30, 30, 46, 220));
                painter.drawRoundedRect(bgRect, 4, 4);

                painter.setPen(stroke);
                painter.drawText(bgRect, Qt::AlignCenter, badgeText);
            }
        }
        else if (m_udlGeomType == GISApp::UI::UDL::UdlGeometryType::Polyline) {
            if (waypointsPx.size() > 1) {
                painter.setPen(solidPen);
                painter.drawPolyline(waypointsPx);
            }

            if (!waypointsPx.isEmpty() && hasMouse) {
                painter.setPen(dashPen);
                painter.drawLine(waypointsPx.last(), mousePx);
            }

            painter.setPen(QPen(stroke, 2));
            painter.setBrush(Qt::white);
            for (const auto &pt : waypointsPx) {
                painter.drawEllipse(pt, 5, 5);
            }

            if (hasMouse) {
                painter.setPen(QPen(Qt::white, 2));
                painter.setBrush(stroke);
                painter.drawEllipse(mousePx, 6, 6);

                double totalMeters = 0.0;
                for (size_t i = 1; i < m_udlWaypoints.size(); ++i) {
                    totalMeters += calculateHaversineDistanceMeters(m_udlWaypoints[i - 1], m_udlWaypoints[i]);
                }
                if (!m_udlWaypoints.empty()) {
                    totalMeters += calculateHaversineDistanceMeters(m_udlWaypoints.back(), m_udlMouseCoord);
                }

                QString infoText = QString("Length: %1 (Click to add, Right-click to finish)").arg(formatDistanceStr(totalMeters));
                QFont font("sans-serif", 9, QFont::Bold);
                painter.setFont(font);
                QFontMetrics fm(font);
                int tw = fm.horizontalAdvance(infoText) + 14;
                int th = fm.height() + 6;

                QRectF bgRect(mousePx.x() + 14, mousePx.y() - th / 2.0, tw, th);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(24, 24, 37, 220));
                painter.drawRoundedRect(bgRect, 4, 4);

                painter.setPen(Qt::white);
                painter.drawText(bgRect, Qt::AlignCenter, infoText);
            }
        }
        else if (m_udlGeomType == GISApp::UI::UDL::UdlGeometryType::Polygon) {
            QPolygonF previewPoly = waypointsPx;
            if (hasMouse) {
                previewPoly.append(mousePx);
            }

            if (previewPoly.size() >= 3) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(fill);
                painter.drawPolygon(previewPoly);
            }

            if (waypointsPx.size() > 1) {
                painter.setPen(solidPen);
                painter.drawPolyline(waypointsPx);
            }

            if (!waypointsPx.isEmpty() && hasMouse) {
                painter.setPen(dashPen);
                painter.drawLine(waypointsPx.last(), mousePx);
                if (waypointsPx.size() >= 2) {
                    painter.drawLine(mousePx, waypointsPx.first());
                }
            }

            painter.setPen(QPen(stroke, 2));
            painter.setBrush(Qt::white);
            for (const auto &pt : waypointsPx) {
                painter.drawEllipse(pt, 5, 5);
            }

            if (hasMouse) {
                painter.setPen(QPen(Qt::white, 2));
                painter.setBrush(stroke);
                painter.drawEllipse(mousePx, 6, 6);

                QString prompt;
                if (m_udlWaypoints.size() < 2) {
                    prompt = tr("Left-click to place polygon vertex %1").arg(m_udlWaypoints.size() + 1);
                } else {
                    prompt = tr("Left-click for vertex %1 | Right-click to finish Polygon").arg(m_udlWaypoints.size() + 1);
                }

                QFont font("sans-serif", 9, QFont::Bold);
                painter.setFont(font);
                QFontMetrics fm(font);
                int tw = fm.horizontalAdvance(prompt) + 14;
                int th = fm.height() + 6;

                QRectF bgRect(mousePx.x() + 14, mousePx.y() - th / 2.0, tw, th);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(24, 24, 37, 220));
                painter.drawRoundedRect(bgRect, 4, 4);

                painter.setPen(Qt::white);
                painter.drawText(bgRect, Qt::AlignCenter, prompt);
            }
        }
        else if (m_udlGeomType == GISApp::UI::UDL::UdlGeometryType::Circle) {
            if (m_udlWaypoints.empty()) {
                if (hasMouse) {
                    painter.setPen(QPen(stroke, 1.5, Qt::DashLine));
                    painter.drawEllipse(mousePx, 6, 6);

                    QString prompt = tr("Left-click to set Circle Center");
                    QFont font("sans-serif", 9, QFont::Bold);
                    painter.setFont(font);
                    QFontMetrics fm(font);
                    int tw = fm.horizontalAdvance(prompt) + 14;
                    int th = fm.height() + 6;

                    QRectF bgRect(mousePx.x() + 14, mousePx.y() - th / 2.0, tw, th);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(24, 24, 37, 220));
                    painter.drawRoundedRect(bgRect, 4, 4);

                    painter.setPen(Qt::white);
                    painter.drawText(bgRect, Qt::AlignCenter, prompt);
                }
            } else {
                GISApp::Core::Models::GeoCoordinate center = m_udlWaypoints[0];
                QPointF centerPx = m_mapWidget->map()->pixelForCoordinate({center.latitude(), center.longitude()});
                
                GISApp::Core::Models::GeoCoordinate edgeCoord = hasMouse ? m_udlMouseCoord : center;
                double radiusMeters = calculateHaversineDistanceMeters(center, edgeCoord);

                auto circleGeo = createGeodesicCircle(center, radiusMeters, 64);
                QPolygonF circlePx;
                for (const auto &pt : circleGeo) {
                    circlePx.append(m_mapWidget->map()->pixelForCoordinate({pt.latitude(), pt.longitude()}));
                }

                painter.setPen(Qt::NoPen);
                painter.setBrush(fill);
                painter.drawPolygon(circlePx);

                painter.setPen(dashPen);
                painter.drawPolyline(circlePx);

                painter.setPen(QPen(Qt::white, 2));
                painter.setBrush(stroke);
                painter.drawEllipse(centerPx, 6, 6);

                if (hasMouse) {
                    painter.setPen(QPen(stroke, 2, Qt::DotLine));
                    painter.drawLine(centerPx, mousePx);

                    painter.setPen(QPen(Qt::white, 2));
                    painter.setBrush(stroke);
                    painter.drawEllipse(mousePx, 6, 6);

                    QString prompt = QString("Radius: %1 (Left-click to finish Circle)").arg(formatDistanceStr(radiusMeters));
                    QFont font("sans-serif", 9, QFont::Bold);
                    painter.setFont(font);
                    QFontMetrics fm(font);
                    int tw = fm.horizontalAdvance(prompt) + 14;
                    int th = fm.height() + 6;

                    QRectF bgRect(mousePx.x() + 14, mousePx.y() - th / 2.0, tw, th);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(24, 24, 37, 220));
                    painter.drawRoundedRect(bgRect, 4, 4);

                    painter.setPen(Qt::white);
                    painter.drawText(bgRect, Qt::AlignCenter, prompt);
                }
            }
        }
    }

    // 3. Render persistent UDL Text entities
    const auto allEntities = GISApp::Publishing::UdlRepositoryManager::instance().getAllEntities();
    for (const auto &item : allEntities) {
        if (item.entityType != "Text") continue;
        if (!item.geometryJson.contains("coordinates")) continue;

        QJsonArray coords = item.geometryJson["coordinates"].toArray();
        if (coords.size() < 2) continue;

        double lon = coords[0].toDouble();
        double lat = coords[1].toDouble();
        if (lat < -89.9 || lat > 89.9 || lon < -180.0 || lon > 180.0) continue;

        QPointF pt = m_mapWidget->map()->pixelForCoordinate({lat, lon});
        if (!QRectF(rect()).adjusted(-100, -100, 100, 100).contains(pt)) continue;

        QString labelText = item.styleJson.value("textContent").toString();
        if (labelText.isEmpty()) labelText = item.entityName;
        if (labelText.isEmpty()) continue;

        // Font Family & Size
        QString fontFamilyStr = item.styleJson.value("fontFamily").toString("sans-serif");
        int fontSize = item.styleJson.value("fontSize").toInt(14);
        if (fontSize < 8) fontSize = 14;

        QFont font(fontFamilyStr, fontSize, QFont::Bold);
        painter.setFont(font);
        QFontMetrics fm(font);

        int textWidth = fm.horizontalAdvance(labelText);
        int textHeight = fm.height();
        QRectF textRect(pt.x() - textWidth / 2.0 - 8, pt.y() - textHeight / 2.0 - 4, textWidth + 16, textHeight + 8);

        // Text Color & Opacity
        QString textColStr = item.styleJson.value("textColor").toString(item.styleJson.value("strokeColor").toString("#ffffff"));
        QColor textColor = QColor::isValidColorName(textColStr) ? QColor(textColStr) : QColor("#ffffff");
        double textOpacity = item.styleJson.value("textOpacity").toDouble(1.0);
        textColor.setAlphaF(qBound(0.0, textOpacity, 1.0));

        // Background Color & Opacity
        QString bgColStr = item.styleJson.value("bgColor").toString("#0f172a");
        QColor bgColor = QColor::isValidColorName(bgColStr) ? QColor(bgColStr) : QColor("#0f172a");
        double bgOpacity = item.styleJson.value("bgOpacity").toDouble(0.85);
        bgColor.setAlphaF(qBound(0.0, bgOpacity, 1.0));

        // Border Color, Opacity & Width
        QString borderColStr = item.styleJson.value("borderColor").toString(item.styleJson.value("strokeColor").toString("#f59e0b"));
        QColor borderColor = QColor::isValidColorName(borderColStr) ? QColor(borderColStr) : QColor("#f59e0b");
        double borderOpacity = item.styleJson.value("borderOpacity").toDouble(1.0);
        borderColor.setAlphaF(qBound(0.0, borderOpacity, 1.0));
        int borderWidth = item.styleJson.value("borderWidth").toInt(1);

        // Draw pill background badge with border
        if (borderWidth > 0 && borderColor.alpha() > 0) {
            painter.setPen(QPen(borderColor, borderWidth));
        } else {
            painter.setPen(Qt::NoPen);
        }
        painter.setBrush(bgColor);
        painter.drawRoundedRect(textRect, 5, 5);

        // Draw text label
        painter.setPen(textColor);
        painter.drawText(textRect, Qt::AlignCenter, labelText);
    }
}

} // namespace GISApp::Map
