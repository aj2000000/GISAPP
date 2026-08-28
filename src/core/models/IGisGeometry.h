/**
 * @file IGisGeometry.h
 * @brief Top-level abstract interface and concrete classes for spatial geometries.
 * Supports Point, Polyline, Polygon, MultiPoint, TextAnnotation, ImageOverlay, and Custom geometries.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IGISGEOMETRY_H
#define IGISGEOMETRY_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

namespace GISApp::Core::Models {

enum class GeometryType {
    Point,
    Polyline,
    Polygon,
    MultiPoint,
    TextAnnotation,
    ImageOverlay,
    Custom
};

struct Coordinate3D {
    double latitude{0.0};
    double longitude{0.0};
    double altitude{0.0};

    Coordinate3D() = default;
    Coordinate3D(double lat, double lon, double alt = 0.0)
        : latitude(lat), longitude(lon), altitude(alt) {}
};

/**
 * @class IGisGeometry
 * @brief Framework-agnostic top-level abstract spatial geometry interface.
 */
class IGisGeometry {
public:
    virtual ~IGisGeometry() = default;

    virtual GeometryType type() const = 0;
    virtual QVector<Coordinate3D> coordinates() const = 0;
    virtual QJsonObject toGeoJsonGeometry() const = 0;
};

/**
 * @class PointGeometry
 * @brief Single 3D coordinate point representation.
 */
class PointGeometry : public IGisGeometry {
public:
    PointGeometry() = default;
    PointGeometry(double lat, double lon, double alt = 0.0) : m_coord(lat, lon, alt) {}
    explicit PointGeometry(const Coordinate3D &coord) : m_coord(coord) {}

    GeometryType type() const override { return GeometryType::Point; }

    QVector<Coordinate3D> coordinates() const override {
        return { m_coord };
    }

    Coordinate3D coordinate() const { return m_coord; }

    QJsonObject toGeoJsonGeometry() const override {
        QJsonObject geomObj;
        geomObj["type"] = "Point";
        QJsonArray coords;
        coords.append(m_coord.longitude);
        coords.append(m_coord.latitude);
        if (m_coord.altitude != 0.0) {
            coords.append(m_coord.altitude);
        }
        geomObj["coordinates"] = coords;
        return geomObj;
    }

private:
    Coordinate3D m_coord;
};

/**
 * @class PolylineGeometry
 * @brief Ordered list of 3D coordinates representing lines, tracks, and paths.
 */
class PolylineGeometry : public IGisGeometry {
public:
    PolylineGeometry() = default;
    explicit PolylineGeometry(const QVector<Coordinate3D> &coords) : m_coords(coords) {}

    GeometryType type() const override { return GeometryType::Polyline; }

    QVector<Coordinate3D> coordinates() const override { return m_coords; }

    QJsonObject toGeoJsonGeometry() const override {
        QJsonObject geomObj;
        geomObj["type"] = "LineString";
        QJsonArray lineCoords;
        for (const auto &c : m_coords) {
            QJsonArray pt;
            pt.append(c.longitude);
            pt.append(c.latitude);
            lineCoords.append(pt);
        }
        geomObj["coordinates"] = lineCoords;
        return geomObj;
    }

private:
    QVector<Coordinate3D> m_coords;
};

/**
 * @class PolygonGeometry
 * @brief Ring of 3D coordinates representing areas, zones, and boundaries.
 */
class PolygonGeometry : public IGisGeometry {
public:
    PolygonGeometry() = default;
    explicit PolygonGeometry(const QVector<Coordinate3D> &ring) : m_ring(ring) {}

    GeometryType type() const override { return GeometryType::Polygon; }

    QVector<Coordinate3D> coordinates() const override { return m_ring; }

    QJsonObject toGeoJsonGeometry() const override {
        QJsonObject geomObj;
        geomObj["type"] = "Polygon";
        QJsonArray ringArray;
        for (const auto &c : m_ring) {
            QJsonArray pt;
            pt.append(c.longitude);
            pt.append(c.latitude);
            ringArray.append(pt);
        }
        QJsonArray polygonCoords;
        polygonCoords.append(ringArray);
        geomObj["coordinates"] = polygonCoords;
        return geomObj;
    }

private:
    QVector<Coordinate3D> m_ring;
};

} // namespace GISApp::Core::Models

#endif // IGISGEOMETRY_H
