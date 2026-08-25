/**
 * @file LayerPublishingService.cpp
 * @brief Implementation of LayerPublishingService delegation.
 */

#include "publishing/LayerPublishingService.h"

namespace GISApp::Publishing {

LayerPublishingService::LayerPublishingService(QObject *parent)
    : QObject(parent)
{
}

bool LayerPublishingService::publishLayer(LayerType type,
                                          const QString &folderPath,
                                          const QString &layerName,
                                          GISApp::Layers::LayerGroupNode *targetGroup,
                                          GISApp::Layers::LayerManager *layerManager,
                                          QMapLibre::Map *map,
                                          ProgressCallback progressCb)
{
    bool success = false;
    if (type == LayerType::Raster) {
        success = m_rasterPublisher.publish(folderPath, layerName, targetGroup, layerManager, map, progressCb);
        m_lastStatus = m_rasterPublisher.statusMessage();
    } else {
        success = m_vectorPublisher.publish(folderPath, layerName, targetGroup, layerManager, map, progressCb);
        m_lastStatus = m_vectorPublisher.statusMessage();
    }
    return success;
}

QString LayerPublishingService::lastStatusMessage() const {
    return m_lastStatus;
}

} // namespace GISApp::Publishing
