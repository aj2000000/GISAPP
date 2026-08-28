/**
 * @file IEntityPainter.h
 * @brief Strategy pattern interface for custom map painting and feature rendering.
 * Allows developers to define custom visual rendering behavior per entity type.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IENTITYPAINTER_H
#define IENTITYPAINTER_H

#include <QJsonObject>
#include <QString>
#include <memory>
#include "GenericGisEntity.h"

namespace GISApp::Core::Renderers {

using GISApp::Core::Models::GenericGisEntity;

/**
 * @class IEntityPainter
 * @brief Strategy interface for customizing entity map representation and GeoJSON styling.
 */
class IEntityPainter {
public:
    virtual ~IEntityPainter() = default;

    virtual QString painterId() const = 0;

    /**
     * @brief Enrich a standard GeoJSON Feature with custom painter properties (e.g. SVG path, custom icons).
     */
    virtual QJsonObject customizeGeoJsonFeature(
        const GenericGisEntity &entity,
        const QJsonObject &baseFeature
    ) const = 0;
};

/**
 * @class DefaultEntityPainter
 * @brief Default fallback painter applying stroke, fill, icon, and label properties directly.
 */
class DefaultEntityPainter : public IEntityPainter {
public:
    QString painterId() const override { return "default"; }

    QJsonObject customizeGeoJsonFeature(
        const GenericGisEntity &entity,
        const QJsonObject &baseFeature
    ) const override {
        Q_UNUSED(entity);
        return baseFeature;
    }
};

} // namespace GISApp::Core::Renderers

#endif // IENTITYPAINTER_H
