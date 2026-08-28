/**
 * @file PublisherFactory.cpp
 * @brief Factory implementation producing spatial publisher strategies.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "publishing/PublisherFactory.h"
#include "publishing/RasterLayerPublisher.h"
#include "publishing/VectorLayerPublisher.h"

namespace GISApp::Publishing {

std::unique_ptr<IPublisherStrategy> PublisherFactory::createPublisher(LayerType type) {
    switch (type) {
        case LayerType::Raster:
            return std::make_unique<RasterLayerPublisher>();
        case LayerType::Vector:
            return std::make_unique<VectorLayerPublisher>();
        default:
            return std::make_unique<RasterLayerPublisher>();
    }
}

} // namespace GISApp::Publishing
