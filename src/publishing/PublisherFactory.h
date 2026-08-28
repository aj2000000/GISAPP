/**
 * @file PublisherFactory.h
 * @brief Factory for instantiating IPublisherStrategy instances based on LayerType.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef PUBLISHERFACTORY_H
#define PUBLISHERFACTORY_H

#include "publishing/IPublisherStrategy.h"
#include <memory>

namespace GISApp::Publishing {

/**
 * @class PublisherFactory
 * @brief Factory Pattern implementation producing IPublisherStrategy instances.
 * Adheres to Single Responsibility Principle (SRP) and Open/Closed Principle (OCP).
 */
class PublisherFactory {
public:
    /**
     * @brief Creates a unique instance of IPublisherStrategy corresponding to the requested LayerType.
     * @param type Raster or Vector layer type.
     * @return Unique pointer to the constructed strategy.
     */
    static std::unique_ptr<IPublisherStrategy> createPublisher(LayerType type);
};

} // namespace GISApp::Publishing

#endif // PUBLISHERFACTORY_H
