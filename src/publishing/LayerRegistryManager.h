/**
 * @file LayerRegistryManager.h
 * @brief Persistent JSON Layer Registry for auto-restoring published layers on application startup.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef LAYERREGISTRYMANAGER_H
#define LAYERREGISTRYMANAGER_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "publishing/IPublisherStrategy.h"
#include "layers/LayerManager.h"
#include "layers/LayerTreeNode.h"

namespace QMapLibre { class Map; }

namespace GISApp::Publishing {

class LayerPublishingService;

struct PublishedLayerMeta {
    QString name;
    LayerType type;
    QString folderPath;
    QString tilePath;
    QString groupName;
    bool isTiled{true};
    bool isVisible{true};
    float opacity{1.0f};
    int orderIndex{0};
    int minZoom{0};
    int maxZoom{22};
};

/**
 * @class LayerRegistryManager
 * @brief Singleton manager providing persistent disk storage and auto-restoration of published GIS layers.
 */
class LayerRegistryManager {
public:
    static LayerRegistryManager& instance();

    void registerPublishedLayer(LayerType type,
                                const QString &folderPath,
                                const QString &layerName,
                                const QString &groupName,
                                int minZoom = 0,
                                int maxZoom = 22);

    void registerGroup(const QString &groupName);
    void unregisterLayer(const QString &layerName);
    void unregisterGroup(const QString &groupName);
    void syncTreeState(GISApp::Layers::LayerManager *layerManager);

    void restoreSavedLayers(GISApp::Layers::LayerManager *layerManager,
                            QMapLibre::Map *map,
                            LayerPublishingService *publishingService);

    QList<PublishedLayerMeta> getSavedLayers() const;
    QStringList getSavedGroups() const;

private:
    LayerRegistryManager();
    ~LayerRegistryManager() = default;

    QString registryFilePath() const;
    void loadFromDisk();
    void saveToDisk();

    QList<PublishedLayerMeta> m_registry;
    QStringList m_customGroups;
    bool m_isRestoring{false};
    bool m_restorationComplete{false};
};

} // namespace GISApp::Publishing

#endif // LAYERREGISTRYMANAGER_H
