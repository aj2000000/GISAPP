/**
 * @file LayerRegistryManager.cpp
 * @brief Implementation of LayerRegistryManager JSON persistence.
 */

#include "publishing/LayerRegistryManager.h"
#include "publishing/LayerPublishingService.h"
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
    QString configDir = QDir::currentPath() + "/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return configDir + "/published_layers.json";
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
        obj["tilePath"] = meta.tilePath.isEmpty() ? QString("/home/crl/aman/MAPDATA/%1").arg(QString(meta.name).toLower().replace(' ', '_')) : meta.tilePath;
        obj["groupName"] = meta.groupName;
        obj["isTiled"] = meta.isTiled;
        obj["isVisible"] = meta.isVisible;
        obj["opacity"] = meta.opacity;
        obj["orderIndex"] = meta.orderIndex;
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
    if (!layerManager || !layerManager->model()) return;

    int orderIdx = 0;
    std::function<void(GISApp::Layers::LayerTreeNode*)> traverse = [&](GISApp::Layers::LayerTreeNode *node) {
        if (!node) return;
        if (node->nodeType() == GISApp::Layers::NodeType::Layer) {
            for (auto &meta : m_registry) {
                if (meta.name == node->name()) {
                    meta.isVisible = (node->checkState() == Qt::Checked);
                    meta.opacity = node->opacity();
                    meta.orderIndex = orderIdx++;
                    break;
                }
            }
        } else if (node->nodeType() == GISApp::Layers::NodeType::Group) {
            GISApp::Layers::LayerGroupNode *gNode = static_cast<GISApp::Layers::LayerGroupNode*>(node);
            for (int i = 0; i < gNode->childCount(); ++i) {
                traverse(gNode->child(i));
            }
        }
    };

    auto root = layerManager->model()->rootNode();
    for (int i = 0; i < root->childCount(); ++i) {
        traverse(root->child(i));
    }

    saveToDisk();
}

void LayerRegistryManager::registerGroup(const QString &groupName) {
    if (!groupName.isEmpty() && !m_customGroups.contains(groupName)) {
        m_customGroups.append(groupName);
        saveToDisk();
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
                                                  const QString &groupName)
{
    registerGroup(groupName);

    // Avoid duplicate entries for same layer name
    for (auto &meta : m_registry) {
        if (meta.name == layerName) {
            meta.type = type;
            meta.folderPath = folderPath;
            meta.groupName = groupName;
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

    m_registry.append(meta);
    saveToDisk();
}

void LayerRegistryManager::restoreSavedLayers(GISApp::Layers::LayerManager *layerManager,
                                              QMapLibre::Map *map,
                                              LayerPublishingService *publishingService)
{
    if (!layerManager || !map || !publishingService) return;

    loadFromDisk();

    // 1. First restore custom groups into LayerManager tree
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

    if (m_registry.isEmpty()) return;

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

        qWarning() << "[LayerRegistry] Restoring saved layer:" << meta.name << "from" << meta.folderPath << "| Opacity:" << meta.opacity << "| Visible:" << meta.isVisible;
        publishingService->publishLayer(meta.type, meta.folderPath, meta.name, targetGroup, layerManager, map);

        // Apply saved opacity & visibility to restored node
        if (layerManager->model()) {
            auto root = layerManager->model()->rootNode();
            std::function<void(GISApp::Layers::LayerTreeNode*)> applyMeta = [&](GISApp::Layers::LayerTreeNode *node) {
                if (!node) return;
                if (node->nodeType() == GISApp::Layers::NodeType::Layer && node->name() == meta.name) {
                    layerManager->setOpacity(node, meta.opacity);
                    layerManager->setVisibility(node, meta.isVisible);
                } else if (node->nodeType() == GISApp::Layers::NodeType::Group) {
                    GISApp::Layers::LayerGroupNode *gNode = static_cast<GISApp::Layers::LayerGroupNode*>(node);
                    for (int i = 0; i < gNode->childCount(); ++i) {
                        applyMeta(gNode->child(i));
                    }
                }
            };
            for (int i = 0; i < root->childCount(); ++i) {
                applyMeta(root->child(i));
            }
        }
    }
    layerManager->syncRenderOrder();
}

QList<PublishedLayerMeta> LayerRegistryManager::getSavedLayers() const {
    return m_registry;
}

QStringList LayerRegistryManager::getSavedGroups() const {
    return m_customGroups;
}

} // namespace GISApp::Publishing
