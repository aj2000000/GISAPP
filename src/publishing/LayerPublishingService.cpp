/**
 * @file LayerPublishingService.cpp
 * @brief Implementation of LayerPublishingService delegation and background task integration.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "publishing/LayerPublishingService.h"
#include "publishing/LayerRegistryManager.h"
#include "core/tasks/BackgroundTaskManager.h"
#include <QMetaObject>
#include <QCoreApplication>
#include <QDebug>

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
                                          ProgressCallback progressCb,
                                          int minZoom,
                                          int maxZoom,
                                          bool runInBackground)
{
    if (runInBackground) {
        // Dispatch to Core BackgroundTaskManager Subsystem
        m_lastStatus = QString("Task '%1' dispatched to Background Task Manager.").arg(layerName);

        GISApp::Core::Tasks::BackgroundTaskManager::instance().submitTask(
            QString("Publish Layer: %1").arg(layerName),
            [type, folderPath, layerName, targetGroup, layerManager, map, minZoom, maxZoom](QString taskId) {
                
                auto bgProgressCb = [taskId](int percent, const QString &statusText) {
                    GISApp::Core::Tasks::BackgroundTaskManager::instance().updateProgress(taskId, percent, statusText);
                };

                // STEP 1: Execute heavy file parsing, VRT building, & GDAL pyramid generation in THIS worker thread!
                std::shared_ptr<IPublisherStrategy> publisher = PublisherFactory::createPublisher(type);
                bool prepOk = publisher ? publisher->prepareInBackground(folderPath, layerName, bgProgressCb) : false;

                if (!prepOk) {
                    GISApp::Core::Tasks::BackgroundTaskManager::instance().markFailed(taskId, QString("Failed to prepare spatial catalog for '%1'.").arg(layerName));
                    return;
                }

                // STEP 2: Only invoke light map rendering and layer addition on the main GUI thread safely!
                QMetaObject::invokeMethod(QCoreApplication::instance(), [publisher, type, folderPath, layerName, targetGroup, layerManager, map, minZoom, maxZoom, taskId, bgProgressCb]() {
                    bool ok = publisher ? publisher->publish(folderPath, layerName, targetGroup, layerManager, map, bgProgressCb, minZoom, maxZoom) : false;

                    if (ok) {
                        QString groupName = targetGroup ? targetGroup->name() : "";
                        GISApp::Publishing::LayerRegistryManager::instance().registerPublishedLayer(type, folderPath, layerName, groupName, minZoom, maxZoom);
                        GISApp::Core::Tasks::BackgroundTaskManager::instance().markCompleted(taskId, QString("Layer '%1' published successfully!").arg(layerName));
                    } else {
                        GISApp::Core::Tasks::BackgroundTaskManager::instance().markFailed(taskId, QString("Failed to publish '%1'.").arg(layerName));
                    }
                }, Qt::QueuedConnection);
            }
        );
        return true;
    }

    // Direct synchronous execution via Factory strategy
    auto publisher = PublisherFactory::createPublisher(type);
    if (!publisher) {
        m_lastStatus = "Error: Failed to instantiate publisher strategy.";
        return false;
    }

    bool success = publisher->publish(folderPath, layerName, targetGroup, layerManager, map, progressCb, minZoom, maxZoom);
    m_lastStatus = publisher->statusMessage();
    return success;
}

QString LayerPublishingService::lastStatusMessage() const {
    return m_lastStatus;
}

} // namespace GISApp::Publishing
