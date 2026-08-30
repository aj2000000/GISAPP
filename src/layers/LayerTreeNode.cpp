/**
 * @file LayerTreeNode.cpp
 * @brief Implementation of LayerTreeNode, LayerGroupNode, and LayerNode.
 */

#include "layers/LayerTreeNode.h"
#include <algorithm>

namespace GISApp::Layers {

// --- LayerTreeNode Base Implementation ---
LayerTreeNode::LayerTreeNode(const QString &name, NodeType type, LayerTreeNode *parent)
    : m_name(name), m_type(type), m_parent(parent)
{
}

QString LayerTreeNode::name() const { return m_name; }
void LayerTreeNode::setName(const QString &name) { m_name = name; }
NodeType LayerTreeNode::nodeType() const { return m_type; }

LayerTreeNode* LayerTreeNode::parentNode() const { return m_parent; }
void LayerTreeNode::setParentNode(LayerTreeNode *parent) { m_parent = parent; }

Qt::CheckState LayerTreeNode::checkState() const { return m_checkState; }
void LayerTreeNode::setCheckState(Qt::CheckState state) { m_checkState = state; }

float LayerTreeNode::opacity() const { return m_opacity; }
void LayerTreeNode::setOpacity(float opacity) { m_opacity = std::clamp(opacity, 0.0f, 1.0f); }

int LayerTreeNode::childCount() const { return 0; }
LayerTreeNode* LayerTreeNode::child(int) const { return nullptr; }

int LayerTreeNode::row() const {
    if (m_parent) {
        for (int i = 0; i < m_parent->childCount(); ++i) {
            if (m_parent->child(i) == this) return i;
        }
    }
    return 0;
}

LayerExtent LayerTreeNode::getExtent() const { return LayerExtent(); }

// --- LayerGroupNode Implementation ---
LayerGroupNode::LayerGroupNode(const QString &name, LayerTreeNode *parent)
    : LayerTreeNode(name, NodeType::Group, parent)
{
}

void LayerGroupNode::addChild(std::unique_ptr<LayerTreeNode> child) {
    if (child) {
        child->setParentNode(this);
        m_children.push_back(std::move(child));
        updateGroupCheckState();
    }
}

std::unique_ptr<LayerTreeNode> LayerGroupNode::removeChild(int row) {
    if (row >= 0 && row < static_cast<int>(m_children.size())) {
        auto node = std::move(m_children[row]);
        m_children.erase(m_children.begin() + row);
        updateGroupCheckState();
        return node;
    }
    return nullptr;
}

void LayerGroupNode::swapChildren(int row1, int row2) {
    if (row1 >= 0 && row1 < static_cast<int>(m_children.size()) &&
        row2 >= 0 && row2 < static_cast<int>(m_children.size()) && row1 != row2) {
        std::swap(m_children[row1], m_children[row2]);
    }
}

int LayerGroupNode::childCount() const {
    return static_cast<int>(m_children.size());
}

LayerTreeNode* LayerGroupNode::child(int row) const {
    if (row >= 0 && row < static_cast<int>(m_children.size())) {
        return m_children[row].get();
    }
    return nullptr;
}

int LayerGroupNode::row() const {
    return LayerTreeNode::row();
}

void LayerGroupNode::setCheckState(Qt::CheckState state) {
    LayerTreeNode::setCheckState(state);
    for (auto &childNode : m_children) {
        childNode->setCheckState(state);
    }
}

void LayerGroupNode::setOpacity(float opacity) {
    LayerTreeNode::setOpacity(opacity);
    for (auto &childNode : m_children) {
        childNode->setOpacity(opacity);
    }
}

static void collectGroupLeafExtents(const LayerTreeNode *node, std::vector<LayerExtent> &extents) {
    if (!node) return;
    if (node->nodeType() == NodeType::Layer) {
        LayerExtent ext = node->getExtent();
        if (ext.isValid()) {
            extents.push_back(ext);
        }
    } else if (node->nodeType() == NodeType::Group) {
        auto groupNode = static_cast<const LayerGroupNode*>(node);
        for (int i = 0; i < groupNode->childCount(); ++i) {
            collectGroupLeafExtents(groupNode->child(i), extents);
        }
    }
}

LayerExtent LayerGroupNode::getExtent() const {
    std::vector<LayerExtent> leafExtents;
    collectGroupLeafExtents(this, leafExtents);

    if (leafExtents.empty()) {
        return LayerExtent();
    }

    double minLat = 90.0;
    double minLon = 180.0;
    double maxLat = -90.0;
    double maxLon = -180.0;
    bool foundAny = false;

    for (const auto &ext : leafExtents) {
        if (!ext.isValid()) continue;
        minLat = std::min(minLat, ext.southWest.latitude());
        minLon = std::min(minLon, ext.southWest.longitude());
        maxLat = std::max(maxLat, ext.northEast.latitude());
        maxLon = std::max(maxLon, ext.northEast.longitude());
        foundAny = true;
    }

    if (!foundAny) return LayerExtent();

    minLat = std::clamp(minLat, -89.9, 89.9);
    maxLat = std::clamp(maxLat, -89.9, 89.9);
    minLon = std::clamp(minLon, -180.0, 180.0);
    maxLon = std::clamp(maxLon, -180.0, 180.0);

    LayerExtent combinedExtent;
    combinedExtent.southWest = GISApp::Core::Models::GeoCoordinate(minLat, minLon);
    combinedExtent.northEast = GISApp::Core::Models::GeoCoordinate(maxLat, maxLon);
    return combinedExtent;
}

void LayerGroupNode::updateGroupCheckState() {
    if (m_children.empty()) return;

    int checkedCount = 0;
    int partiallyCheckedCount = 0;

    for (const auto &childNode : m_children) {
        if (childNode->checkState() == Qt::Checked) checkedCount++;
        else if (childNode->checkState() == Qt::PartiallyChecked) partiallyCheckedCount++;
    }

    if (checkedCount == static_cast<int>(m_children.size())) {
        LayerTreeNode::setCheckState(Qt::Checked);
    } else if (checkedCount == 0 && partiallyCheckedCount == 0) {
        LayerTreeNode::setCheckState(Qt::Unchecked);
    } else {
        LayerTreeNode::setCheckState(Qt::PartiallyChecked);
    }
}

// --- LayerNode Implementation ---
LayerNode::LayerNode(const QString &name, std::shared_ptr<ILayerAdapter> adapter, LayerTreeNode *parent)
    : LayerTreeNode(name, NodeType::Layer, parent), m_adapter(adapter)
{
}

std::shared_ptr<ILayerAdapter> LayerNode::adapter() const {
    return m_adapter;
}

void LayerNode::setAdapter(std::shared_ptr<ILayerAdapter> adapter) {
    m_adapter = adapter;
    if (m_adapter) {
        m_adapter->setVisibility(checkState() == Qt::Checked);
        m_adapter->setOpacity(opacity());
    }
}

void LayerNode::setCheckState(Qt::CheckState state) {
    LayerTreeNode::setCheckState(state);
    if (m_adapter) {
        m_adapter->setVisibility(state == Qt::Checked);
    }
    if (parentNode()) {
        static_cast<LayerGroupNode*>(parentNode())->updateGroupCheckState();
    }
}

void LayerNode::setOpacity(float opacity) {
    LayerTreeNode::setOpacity(opacity);
    if (m_adapter) {
        m_adapter->setOpacity(opacity);
    }
}

LayerExtent LayerNode::getExtent() const {
    return m_adapter ? m_adapter->getExtent() : LayerExtent();
}

} // namespace GISApp::Layers
