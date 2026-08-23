/**
 * @file LayerTreeNode.h
 * @brief Composite Pattern nodes for Layer Tree hierarchy (Groups & Layers).
 *
 * Implements Composite Pattern allowing uniform management of Layer Groups
 * and individual GIS map Layers.
 *
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef LAYERTREENODE_H
#define LAYERTREENODE_H

#include <QString>
#include <Qt>
#include <vector>
#include <memory>
#include "layers/ILayerAdapter.h"

namespace GISApp::Layers {

/**
 * @enum NodeType
 * @brief Identifies whether a node is a Layer Group or an individual Layer.
 */
enum class NodeType {
    Group,
    Layer
};

/**
 * @class LayerTreeNode
 * @brief Abstract Base Class for tree nodes (Composite Component).
 */
class LayerTreeNode {
public:
    explicit LayerTreeNode(const QString &name, NodeType type, LayerTreeNode *parent = nullptr);
    virtual ~LayerTreeNode() = default;

    QString name() const;
    void setName(const QString &name);

    NodeType nodeType() const;

    LayerTreeNode* parentNode() const;
    void setParentNode(LayerTreeNode *parent);

    virtual Qt::CheckState checkState() const;
    virtual void setCheckState(Qt::CheckState state);

    virtual float opacity() const;
    virtual void setOpacity(float opacity);

    virtual int childCount() const;
    virtual LayerTreeNode* child(int row) const;
    virtual int row() const;

    virtual LayerExtent getExtent() const;

private:
    QString m_name;
    NodeType m_type;
    LayerTreeNode *m_parent{nullptr};
    Qt::CheckState m_checkState{Qt::Checked};
    float m_opacity{1.0f};
};

/**
 * @class LayerGroupNode
 * @brief Composite class containing child LayerTreeNodes (Group Node).
 */
class LayerGroupNode : public LayerTreeNode {
public:
    explicit LayerGroupNode(const QString &name, LayerTreeNode *parent = nullptr);

    void addChild(std::unique_ptr<LayerTreeNode> child);
    std::unique_ptr<LayerTreeNode> removeChild(int row);

    int childCount() const override;
    LayerTreeNode* child(int row) const override;
    int row() const override;

    void setCheckState(Qt::CheckState state) override;
    void setOpacity(float opacity) override;
    LayerExtent getExtent() const override;

    void updateGroupCheckState();

private:
    std::vector<std::unique_ptr<LayerTreeNode>> m_children;
};

/**
 * @class LayerNode
 * @brief Leaf class representing an individual GIS map layer (Leaf Node).
 */
class LayerNode : public LayerTreeNode {
public:
    LayerNode(const QString &name, std::shared_ptr<ILayerAdapter> adapter, LayerTreeNode *parent = nullptr);

    std::shared_ptr<ILayerAdapter> adapter() const;

    void setCheckState(Qt::CheckState state) override;
    void setOpacity(float opacity) override;
    LayerExtent getExtent() const override;

private:
    std::shared_ptr<ILayerAdapter> m_adapter;
};

} // namespace GISApp::Layers

#endif // LAYERTREENODE_H
