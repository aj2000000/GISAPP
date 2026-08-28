/**
 * @file LayerRegistryManager.cpp
 * @brief Implementation of LayerRegistryManager JSON persistence.
 */

#include "publishing/LayerRegistryManager.h"
#include "publishing/LayerPublishingService.h"
#include "core/SystemConfigManager.h"
#include <QStandardPaths>

namespace GISApp::Publishing {

LayerRegistryManager& LayerRegistryManager::instance() {
    static LayerRegistryManager instance;
    return instance;
}

LayerRegistryManager::LayerRegistryManager() {
    loadFromDisk();
}

QString LayerRegistryManager::registryFilePath() const {
    return GISApp::Core::SystemConfigManager::instance().getPublishedLayersPath();
}

void LayerRegistryManager::loadFromDisk() {
    m_registry.clear();
    m_customGroups.clear();
    QFile file(registryFilePath());
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray layersArr;

        if (doc.isObject()) {
            QJsonObject rootObj = doc.object();
            QJsonArray groupsArr = rootObj["groups"].toArray();
            for (const QJsonValue &gVal : groupsArr) {
                QString gName = gVal.toString();
                if (!gName.isEmpty() && !m_customGroups.contains(gName)) {
                    m_customGroups.append(gName);
                }
            }
            layersArr = rootObj["layers"].toArray();
        } else if (doc.isArray()) {
            layersArr = doc.array();
        }

        for (const QJsonValue &val : layersArr) {
            QJsonObject obj = val.toObject();
            PublishedLayerMeta meta;
            meta.name = obj["name"].toString();
            meta.type = (obj["type"].toString() == "Vector") ? LayerType::Vector : LayerType::Raster;
            meta.folderPath = obj["folderPath"].toString();
            meta.tilePath = obj["tilePath"].toString();
            meta.groupName = obj["groupName"].toString();
            meta.isTiled = obj.value("isTiled").toBool(true);
            meta.isVisible = obj.value("isVisible").toBool(true);
            meta.opacity = static_cast<float>(obj.value("opacity").toDouble(1.0));
            meta.orderIndex = obj.value("orderIndex").toInt(0);
            meta.minZoom = obj.value("minZoom").toInt(0);
            meta.maxZoom = obj.value("maxZoom").toInt(22);

            if (!meta.name.isEmpty() && (!meta.folderPath.isEmpty() || !meta.tilePath.isEmpty())) {
                m_registry.append(meta);
                if (!meta.groupName.isEmpty() && !m_customGroups.contains(meta.groupName)) {
                    m_customGroups.append(meta.groupName);
                }
            }
        }
    }
}

void LayerRegistryManager::saveToDisk() {
    QJsonObject rootObj;

    QJsonArray groupsArr;
    for (const QString &grp : m_customGroups) {
        groupsArr.append(grp);
    }
    rootObj["groups"] = groupsArr;

    QJsonArray layersArr;
    for (const auto &meta : m_registry) {
        QJsonObject obj;
        obj["name"] = meta.name;
        obj["type"] = (meta.type == LayerType::Vector) ? "Vector" : "Raster";
        obj["folderPath"] = meta.folderPath;
        obj["tilePath"] = meta.tilePath.isEmpty() ? QString("%1/MAPDATA/%2").arg(QDir::homePath()).arg(QString(meta.name).toLower().replace(' ', '_')) : meta.tilePath;
        obj["groupName"] = meta.groupName;
        obj["isTiled"] = meta.isTiled;
        obj["isVisible"] = meta.isVisible;
        obj["opacity"] = meta.opacity;
        obj["orderIndex"] = meta.orderIndex;
        obj["minZoom"] = meta.minZoom;
        obj["maxZoom"] = meta.maxZoom;
        layersArr.append(obj);
    }
    rootObj["layers"] = layersArr;

    QJsonDocument doc(rootObj);
    QFile file(registryFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qWarning() << "[LayerRegistry] Saved" << m_customGroups.size() << "groups &" << m_registry.size() << "published layers to" << registryFilePath();
    }
}

void LayerRegistryManager::syncTreeState(GISApp::Layers::LayerManager *layerManager) {
    if (m_isRestoring || !m_restorationComplete) return;
    if (!layerManager || !layerManager->model()) return;

    auto root = layerManager->model()->rootNode();
    if (!root) return;

    m_customGroups.clear();
    int globalOrderIdx = 0;

    std::function<void(GISApp::Layers::LayerTreeNode*, const QString&)> traverse = 
        [&](GISApp::Layers::LayerTreeNode *node, const QString &currentGroupName) {
        if (!node) return;

        if (node->nodeType() == GISApp::Layers::NodeType::Group) {
            GISApp::Layers::LayerGroupNode *gNode = static_cast<GISApp::Layers::LayerGroupNode*>(node);
            QString gName = gNode->name();
            if (node->parentNode() == root && !gName.isEmpty()) {
                if (!m_customGroups.contains(gName)) {
                    m_customGroups.append(gName);
                }
            }
            for (int i = 0; i < gNode->childCount(); ++i) {
                traverse(gNode->child(i), gName);
            }
        } else if (node->nodeType() == GISApp::Layers::NodeType::Layer) {
            for (auto &meta : m_registry) {
                if (meta.name == node->name()) {
                    meta.isVisible = (node->checkState() == Qt::Checked);
                    meta.opacity = node->opacity();
                    if (!currentGroupName.isEmpty()) {
                        meta.groupName = currentGroupName;
                    }
                    meta.orderIndex = globalOrderIdx;
                    break;
                }
            }
            globalOrderIdx++;
        }
    };

    for (int i = 0; i < root->childCount(); ++i) {
        traverse(root->child(i), QString());
    }

    saveToDisk();
}

void LayerRegistryManager::registerGroup(const QString &groupName) {
    if (!groupName.isEmpty() && !m_customGroups.contains(groupName)) {
        m_customGroups.append(groupName);
        if (m_restorationComplete && !m_isRestoring) {
            saveToDisk();
        }
    }
}

void LayerRegistryManager::unregisterLayer(const QString &layerName) {
    bool changed = false;
    for (int i = 0; i < m_registry.size(); ++i) {
        if (m_registry[i].name == layerName) {
            m_registry.removeAt(i);
            changed = true;
            break;
        }
    }
    if (changed) {
        saveToDisk();
        qWarning() << "[LayerRegistry] Unregistered layer:" << layerName;
    }
}

void LayerRegistryManager::unregisterGroup(const QString &groupName) {
    bool changed = m_customGroups.removeOne(groupName);
    for (int i = m_registry.size() - 1; i >= 0; --i) {
        if (m_registry[i].groupName == groupName) {
            m_registry.removeAt(i);
            changed = true;
        }
    }
    if (changed) {
        saveToDisk();
        qWarning() << "[LayerRegistry] Unregistered group:" << groupName;
    }
}

void LayerRegistryManager::registerPublishedLayer(LayerType type,
                                                  const QString &folderPath,
                                                  const QString &layerName,
                                                  const QString &groupName,
                                                  int minZoom,
                                                  int maxZoom)
{
    registerGroup(groupName);

    // Avoid duplicate entries for same layer name
    for (auto &meta : m_registry) {
        if (meta.name == layerName) {
            meta.type = type;
            meta.folderPath = folderPath;
            meta.groupName = groupName;
            meta.minZoom = minZoom;
            meta.maxZoom = maxZoom;
            saveToDisk();
            return;
        }
    }

    PublishedLayerMeta meta;
    meta.name = layerName;
    meta.type = type;
    meta.folderPath = folderPath;
    meta.groupName = groupName;
    meta.isVisible = true;
    meta.minZoom = minZoom;
    meta.maxZoom = maxZoom;

    m_registry.append(meta);
    saveToDisk();
}

void LayerRegistryManager::restoreSavedLayers(GISApp::Layers::LayerManager *layerManager,
                                              QMapLibre::Map *map,
                                              LayerPublishingService *publishingService)
{
    if (!layerManager || !map || !publishingService) return;

    m_isRestoring = true;
    loadFromDisk();

    // 1. Ensure custom groups exist in LayerManager tree
    for (const QString &grpName : m_customGroups) {
        if (grpName.isEmpty()) continue;
        GISApp::Layers::LayerGroupNode *existingGrp = nullptr;
        if (layerManager->model()) {
            auto root = layerManager->model()->rootNode();
            for (int i = 0; i < root->childCount(); ++i) {
                auto child = root->child(i);
                if (child->nodeType() == GISApp::Layers::NodeType::Group && child->name() == grpName) {
                    existingGrp = static_cast<GISApp::Layers::LayerGroupNode*>(child);
                    break;
                }
            }
        }
        if (!existingGrp) {
            layerManager->addGroup(grpName);
        }
    }

    // 2. Re-order top-level group nodes under root to match saved m_customGroups order
    if (layerManager->model() && !m_customGroups.isEmpty()) {
        auto root = layerManager->model()->rootNode();
        if (root) {
            for (int targetIdx = 0; targetIdx < m_customGroups.size(); ++targetIdx) {
                QString targetGrpName = m_customGroups[targetIdx];
                int currentIdx = -1;
                for (int i = 0; i < root->childCount(); ++i) {
                    if (root->child(i)->name() == targetGrpName) {
                        currentIdx = i;
                        break;
                    }
                }
                if (currentIdx != -1 && currentIdx > targetIdx) {
                    while (currentIdx > targetIdx) {
                        layerManager->moveUp(root->child(currentIdx));
                        currentIdx--;
                    }
                } else if (currentIdx != -1 && currentIdx < targetIdx) {
                    while (currentIdx < targetIdx) {
                        layerManager->moveDown(root->child(currentIdx));
                        currentIdx++;
                    }
                }
            }
        }
    }

    if (!m_registry.isEmpty()) {
        // Sort by saved Z-order index before restoring
        std::sort(m_registry.begin(), m_registry.end(), [](const PublishedLayerMeta &a, const PublishedLayerMeta &b) {
            return a.orderIndex < b.orderIndex;
        });

        qWarning() << "[LayerRegistry] Auto-restoring" << m_registry.size() << "published layers in saved order from disk...";

        for (const auto &meta : m_registry) {
            if (!QFileInfo(meta.folderPath).exists()) {
                qWarning() << "[LayerRegistry] Skipping missing file/folder:" << meta.folderPath;
                continue;
            }

            // Find or create target group
            GISApp::Layers::LayerGroupNode *targetGroup = nullptr;
            QString searchGroupName = meta.groupName;
            if (searchGroupName.isEmpty() || searchGroupName == "🌍 Base Maps & Terrain") {
                searchGroupName = (meta.type == LayerType::Vector) ? "📍 Custom Vector Layers" : "🗺️ Raster Imagery & DSM";
            }

            if (layerManager->model()) {
                auto root = layerManager->model()->rootNode();
                for (int i = 0; i < root->childCount(); ++i) {
                    auto child = root->child(i);
                    if (child->nodeType() == GISApp::Layers::NodeType::Group && child->name() == searchGroupName) {
                        targetGroup = static_cast<GISApp::Layers::LayerGroupNode*>(child);
                        break;
                    }
                }
                if (!targetGroup) {
                    targetGroup = layerManager->addGroup(searchGroupName);
                }
            }

            qWarning() << "[LayerRegistry] Restoring saved layer in background:" << meta.name << "from" << meta.folderPath << "| Zoom:" << meta.minZoom << "-" << meta.maxZoom << "| Opacity:" << meta.opacity << "| Visible:" << meta.isVisible;
            publishingService->publishLayer(meta.type, meta.folderPath, meta.name, targetGroup, layerManager, map, nullptr, meta.minZoom, meta.maxZoom, true, true, meta.opacity, meta.isVisible);
        }

        // 3. Re-order child nodes inside groups to strictly match saved orderIndex
        if (layerManager->model()) {
            auto root = layerManager->model()->rootNode();
            std::function<void(GISApp::Layers::LayerGroupNode*)> sortGroupChildren = [&](GISApp::Layers::LayerGroupNode *parentGroup) {
                if (!parentGroup) return;
                for (int i = 0; i < parentGroup->childCount(); ++i) {
                    auto childNode = parentGroup->child(i);
                    if (childNode->nodeType() == GISApp::Layers::NodeType::Group) {
                        sortGroupChildren(static_cast<GISApp::Layers::LayerGroupNode*>(childNode));
                    }
                }
                bool swapped = true;
                while (swapped) {
                    swapped = false;
                    for (int i = 0; i < parentGroup->childCount() - 1; ++i) {
                        auto nodeA = parentGroup->child(i);
                        auto nodeB = parentGroup->child(i + 1);
                        int orderA = 999999;
                        int orderB = 999999;
                        for (const auto &meta : m_registry) {
                            if (meta.name == nodeA->name()) orderA = meta.orderIndex;
                            if (meta.name == nodeB->name()) orderB = meta.orderIndex;
                        }
                        if (orderA > orderB) {
                            layerManager->moveDown(nodeA);
                            swapped = true;
                        }
                    }
                }
            };
            for (int i = 0; i < root->childCount(); ++i) {
                if (root->child(i)->nodeType() == GISApp::Layers::NodeType::Group) {
                    sortGroupChildren(static_cast<GISApp::Layers::LayerGroupNode*>(root->child(i)));
                }
            }
        }
    }

    m_isRestoring = false;
    m_restorationComplete = true;

    // Perform final state sync and map rendering z-index sort
    syncTreeState(layerManager);
    layerManager->syncRenderOrder();
}

QList<PublishedLayerMeta> LayerRegistryManager::getSavedLayers() const {
    return m_registry;
}

QStringList LayerRegistryManager::getSavedGroups() const {
    return m_customGroups;
}

} // namespace GISApp::Publishing
