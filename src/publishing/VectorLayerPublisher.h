/**
 * @file VectorLayerPublisher.h
 * @brief Concrete strategy for processing Shapefile (.shp set) and GeoJSON vector data.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef VECTORLAYERPUBLISHER_H
#define VECTORLAYERPUBLISHER_H

#include "publishing/IPublisherStrategy.h"

namespace GISApp::Publishing {

/**
 * @class VectorLayerPublisher
 * @brief Scans directories for Shapefiles (.shp, .dbf, .shx, .prj, .sld) or GeoJSON files,
 * converts vector features, applies styling, and creates MapLibre vector layers.
 */
class VectorLayerPublisher : public IPublisherStrategy {
public:
    VectorLayerPublisher() = default;
    ~VectorLayerPublisher() override = default;

    bool publish(const QString &folderPath,
                 const QString &layerName,
                 GISApp::Layers::LayerGroupNode *targetGroup,
                 GISApp::Layers::LayerManager *layerManager,
                 QMapLibre::Map *map,
                 ProgressCallback progressCb = nullptr,
                 int minZoom = 0,
                 int maxZoom = 22) override;

    bool prepareInBackground(const QString &folderPath,
                            const QString &layerName,
                            ProgressCallback progressCb = nullptr) override;

    QString publisherType() const override { return "Vector (Shapefile / GeoJSON)"; }
    QString statusMessage() const override { return m_statusMessage; }

private:
    QString m_statusMessage;
};

} // namespace GISApp::Publishing

#endif // VECTORLAYERPUBLISHER_H
