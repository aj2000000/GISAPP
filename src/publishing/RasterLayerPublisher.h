/**
 * @file RasterLayerPublisher.h
 * @brief Concrete strategy for multi-resolution GeoTIFF raster map rendering.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef RASTERLAYERPUBLISHER_H
#define RASTERLAYERPUBLISHER_H

#include "publishing/IPublisherStrategy.h"

namespace GISApp::Publishing {

/**
 * @class RasterLayerPublisher
 * @brief Strategy handling GeoTIFF ingest, gdal2tiles XYZ generation, and MapLibre raster rendering.
 */
class RasterLayerPublisher : public IPublisherStrategy {
public:
    RasterLayerPublisher() = default;
    ~RasterLayerPublisher() override = default;

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

    QString publisherType() const override { return "Raster (GeoTIFF / VRT Mosaic)"; }
    QString statusMessage() const override { return m_statusMessage; }

private:
    QString m_statusMessage;
};

} // namespace GISApp::Publishing

#endif // RASTERLAYERPUBLISHER_H
