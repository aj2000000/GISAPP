/**
 * @file IGisEntity.h
 * @brief Top-level abstract interface for all GIS entities (Tracks, Waypoints, Tactical Markings, Zones).
 * Framework-agnostic definition following OOP and SOLID principles.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IGISENTITY_H
#define IGISENTITY_H

#include "IGisGeometry.h"
#include <QString>
#include <QColor>
#include <QJsonObject>
#include <memory>

namespace GISApp::Core::Models {

enum class EntityCategory {
    Track,
    Waypoint,
    TacticalMarking,
    AirZone,
    Custom
};

enum class SymbolRenderType {
    DefaultCircle,
    SvgIcon,
    PngImage,
    TextLabel,
    MilStd2525Symbol,
    CustomPaintDelegate
};

/**
 * @struct EntityRenderStyle
 * @brief Engine-agnostic visual styling properties for GIS entity rendering.
 */
struct EntityRenderStyle {
    SymbolRenderType renderType{SymbolRenderType::DefaultCircle};
    QString iconPath;               // Path to SVG / PNG / MIL-STD-2525 icon file
    QString labelText;              // Text string for annotations or entity labels
    QColor strokeColor{Qt::cyan};   // Line / border color
    double strokeWidth{2.0};        // Line thickness in pixels
    QColor fillColor{QColor(0, 255, 255, 50)}; // Fill color with alpha
    double rotationHeading{0.0};    // Orientation heading in degrees (0 - 360)
    double scale{1.0};              // Symbol scaling factor
};

/**
 * @class IGisEntity
 * @brief Top-level polymorphic abstract interface for all spatial entities in the GIS application.
 */
class IGisEntity {
public:
    virtual ~IGisEntity() = default;

    virtual QString entityId() const = 0;
    virtual QString entityName() const = 0;
    virtual EntityCategory category() const = 0;

    // Polymorphic Spatial Geometry
    virtual std::shared_ptr<IGisGeometry> geometry() const = 0;

    // Polymorphic Visual Presentation Style
    virtual EntityRenderStyle renderStyle() const = 0;

    // Standard GeoJSON Feature serialization for map renderers
    virtual QJsonObject toGeoJsonFeature() const = 0;
};

} // namespace GISApp::Core::Models

#endif // IGISENTITY_H
