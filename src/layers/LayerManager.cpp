/**
 * @file LayerManager.cpp
 * @brief Implementation of LayerManager facade controller.
 */

#include "layers/LayerManager.h"
#include "publishing/LayerRegistryManager.h"
#include "publishing/LocalTileServer.h"

namespace GISApp::Layers {

LayerManager::LayerManager(QObject *parent)
    : QObject(parent), m_model(new LayerTreeModel(this))
{
}

LayerTreeModel* LayerManager::model() const {
    return m_model;
}

LayerGroupNode* LayerManager::addGroup(const QString &groupName, LayerGroupNode *parentGroup) {
    LayerGroupNode *targetParent = parentGroup ? parentGroup : m_model->rootNode();
    
    int row = targetParent->childCount();
    m_model->beginNodeInsert(m_model->indexFromNode(targetParent), row, row);
    
    auto newGroup = std::make_unique<LayerGroupNode>(groupName);
    LayerGroupNode *rawPtr = newGroup.get();
    targetParent->addChild(std::move(newGroup));
    
    m_model->endNodeInsert();

    GISApp::Publishing::LayerRegistryManager::instance().registerGroup(groupName);
    GISApp::Publishing::LayerRegistryManager::instance().syncTreeState(this);

    return rawPtr;
}

LayerNode* LayerManager::addLayer(const QString &layerName, std::shared_ptr<ILayerAdapter> adapter, LayerGroupNode *parentGroup) {
    LayerGroupNode *targetParent = parentGroup ? parentGroup : m_model->rootNode();
    
    int row = targetParent->childCount();
    m_model->beginNodeInsert(m_model->indexFromNode(targetParent), row, row);
    
    auto newLayer = std::make_unique<LayerNode>(layerName, adapter);
    LayerNode *rawPtr = newLayer.get();
    targetParent->addChild(std::move(newLayer));
    
    m_model->endNodeInsert();
    syncRenderOrder();
    GISApp::Publishing::LayerRegistryManager::instance().syncTreeState(this);
    return rawPtr;
}

void LayerManager::setVisibility(LayerTreeNode *node, bool visible) {
    if (node) {
        node->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
        emit m_model->layoutChanged();
        GISApp::Publishing::LayerRegistryManager::instance().syncTreeState(this);
    }
}

void LayerManager::setOpacity(LayerTreeNode *node, float opacity) {
    if (node) {
        node->setOpacity(opacity);
        emit m_model->layoutChanged();
        GISApp::Publishing::LayerRegistryManager::instance().syncTreeState(this);
    }
}

static void collectLayerAdapters(LayerTreeNode *node, std::vector<std::shared_ptr<ILayerAdapter>> &adapters) {
    if (!node) return;
    if (node->nodeType() == NodeType::Layer) {
        auto layerNode = static_cast<LayerNode*>(node);
        if (layerNode->adapter()) {
            adapters.push_back(layerNode->adapter());
        }
    } else if (node->nodeType() == NodeType::Group) {
        auto groupNode = static_cast<LayerGroupNode*>(node);
        for (int i = 0; i < groupNode->childCount(); ++i) {
            collectLayerAdapters(groupNode->child(i), adapters);
        }
    }
}

void LayerManager::syncRenderOrder() {
    if (!m_model || !m_model->rootNode()) return;
    std::vector<std::shared_ptr<ILayerAdapter>> adapters;
    collectLayerAdapters(m_model->rootNode(), adapters);

    // Re-stack MapLibre graphics layers in REVERSE order (from bottom-most UI row to top-most UI row).
    // The top-most item in the UI tree list will be moved to top LAST, placing it on top of all lower layers on the map screen!
    for (auto it = adapters.rbegin(); it != adapters.rend(); ++it) {
        if (*it) {
            (*it)->reinsertLayer("");
        }
    }
}

bool LayerManager::moveUp(LayerTreeNode *node) {
    if (!node || !m_model) return false;
    QModelIndex idx = m_model->indexFromNode(node);
    bool res = m_model->moveNodeUp(idx);
    if (res) {
        syncRenderOrder();
        GISApp::Publishing::LayerRegistryManager::instance().syncTreeState(this);
    }
    return res;
}

bool LayerManager::moveDown(LayerTreeNode *node) {
    if (!node || !m_model) return false;
    QModelIndex idx = m_model->indexFromNode(node);
    bool res = m_model->moveNodeDown(idx);
    if (res) {
        syncRenderOrder();
        GISApp::Publishing::LayerRegistryManager::instance().syncTreeState(this);
    }
    return res;
}

void LayerManager::panToExtent(LayerTreeNode *node) {
    if (node) {
        LayerExtent extent = node->getExtent();
        if (extent.isValid()) {
            emit panToExtentRequested(extent);
        }
    }
}

void LayerManager::removeNode(LayerTreeNode *node) {
    if (!node || !m_model) return;

    if (node->nodeType() == NodeType::Layer) {
        LayerNode *layerNode = static_cast<LayerNode*>(node);
        if (layerNode->adapter()) {
            layerNode->adapter()->removeLayer();
        }
        QString layerName = node->name();
        m_model->removeNode(node);
        GISApp::Publishing::LayerRegistryManager::instance().unregisterLayer(layerName);
        QString sanitizedId = QString("raster-%1").arg(qHash(layerName));
        GISApp::Publishing::LocalTileServer::instance().unregisterLayer(sanitizedId);
    } else if (node->nodeType() == NodeType::Group) {
        LayerGroupNode *groupNode = static_cast<LayerGroupNode*>(node);
        QString groupName = node->name();

        std::vector<LayerTreeNode*> children;
        for (int i = 0; i < groupNode->childCount(); ++i) {
            children.push_back(groupNode->child(i));
        }
        for (LayerTreeNode *child : children) {
            removeNode(child);
        }

        m_model->removeNode(groupNode);
        GISApp::Publishing::LayerRegistryManager::instance().unregisterGroup(groupName);
    }
}

} // namespace GISApp::Layers
