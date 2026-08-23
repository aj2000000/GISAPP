/**
 * @file LayerManager.cpp
 * @brief Implementation of LayerManager facade controller.
 */

#include "layers/LayerManager.h"

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
    return rawPtr;
}

void LayerManager::setVisibility(LayerTreeNode *node, bool visible) {
    if (node) {
        node->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
        emit m_model->layoutChanged();
    }
}

void LayerManager::setOpacity(LayerTreeNode *node, float opacity) {
    if (node) {
        node->setOpacity(opacity);
        emit m_model->layoutChanged();
    }
}

void LayerManager::panToExtent(LayerTreeNode *node) {
    if (node) {
        LayerExtent extent = node->getExtent();
        if (extent.isValid()) {
            emit panToExtentRequested(extent);
        }
    }
}

} // namespace GISApp::Layers
