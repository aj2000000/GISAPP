/**
 * @file IMapRendererAdapter.h
 * @brief Engine-agnostic abstract interface for GIS map renderers.
 * Decouples domain entities from concrete rendering engines (MapLibre, Cesium, ArcGIS, QGIS).
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IMAPRENDERERADAPTER_H
#define IMAPRENDERERADAPTER_H

#include "../models/IGisEntity.h"
#include <QVector>
#include <memory>

namespace GISApp::Core::Renderers {

class IMapRendererAdapter {
public:
    virtual ~IMapRendererAdapter() = default;

    /**
     * @brief Render or update a single GIS entity on the map view.
     */
    virtual void renderEntity(std::shared_ptr<Models::IGisEntity> entity) = 0;

    /**
     * @brief Render or update a collection of GIS entities on the map view.
     */
    virtual void renderEntities(const QVector<std::shared_ptr<Models::IGisEntity>> &entities) = 0;

    /**
     * @brief Remove an entity from the map view by ID.
     */
    virtual void removeEntity(const QString &entityId) = 0;

    /**
     * @brief Clear all rendered entities from the layer.
     */
    virtual void clearEntities() = 0;
};

} // namespace GISApp::Core::Renderers

#endif // IMAPRENDERERADAPTER_H
