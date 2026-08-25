/**
 * @file IPublisherStrategy.h
 * @brief Strategy interface for publishing GIS layers (Raster and Vector).
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IPUBLISHERSTRATEGY_H
#define IPUBLISHERSTRATEGY_H

#include <QString>
#include <QMapLibre/Map>
#include "layers/LayerManager.h"
#include "layers/LayerTreeNode.h"

#include <functional>

namespace GISApp::Publishing {

using ProgressCallback = std::function<void(int percent, const QString &statusText)>;

/**
 * @enum LayerType
 * @brief Represents supported publishing layer modalities.
 */
enum class LayerType {
    Raster,
    Vector
};

/**
 * @class IPublisherStrategy
 * @brief Abstract Strategy Interface defining spatial data ingestion and MapLibre registration.
 */
class IPublisherStrategy {
public:
    virtual ~IPublisherStrategy() = default;

    /**
     * @brief Executes ingestion pipeline for folder contents with real-time progress callbacks.
     * @param folderPath Target directory path.
     * @param layerName Custom layer name.
     * @param targetGroup Target LayerGroupNode (nullptr for root).
     * @param layerManager Active LayerManager facade.
     * @param map Active QMapLibre::Map rendering instance.
     * @param progressCb Optional callback function for UI progress bar updates.
     * @return True if layer was successfully published and rendered.
     */
    virtual bool publish(const QString &folderPath,
                         const QString &layerName,
                         GISApp::Layers::LayerGroupNode *targetGroup,
                         GISApp::Layers::LayerManager *layerManager,
                         QMapLibre::Map *map,
                         ProgressCallback progressCb = nullptr) = 0;

    /**
     * @brief Gets publisher type description.
     */
    virtual QString publisherType() const = 0;

    /**
     * @brief Gets diagnostic status message from last publishing attempt.
     */
    virtual QString statusMessage() const = 0;
};

} // namespace GISApp::Publishing

#endif // IPUBLISHERSTRATEGY_H
