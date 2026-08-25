/**
 * @file RasterLayerPublisher.h
 * @brief Concrete strategy for processing and stitching GeoTIFF raster maps.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef RASTERLAYERPUBLISHER_H
#define RASTERLAYERPUBLISHER_H

#include "publishing/IPublisherStrategy.h"

namespace GISApp::Publishing {

/**
 * @class RasterLayerPublisher
 * @brief Ingests folders with multiple GeoTIFF (.tif) files, computes bounding mosaic extents,
 * and registers optimized raster tile sources in MapLibre Native.
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
                 ProgressCallback progressCb = nullptr) override;

    QString publisherType() const override { return "Raster (GeoTIFF Mosaicing)"; }
    QString statusMessage() const override { return m_statusMessage; }

private:
    QString m_statusMessage;
};

} // namespace GISApp::Publishing

#endif // RASTERLAYERPUBLISHER_H
