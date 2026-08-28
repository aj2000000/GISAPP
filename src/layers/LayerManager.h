/**
 * @file LayerManager.h
 * @brief Facade Manager for GIS Layer Operations (Developer API).
 *
 * Provides a clean API for adding layer groups, inserting layers, controlling
 * opacity, visibility, and zooming to spatial extents.
 *
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QObject>
#include "layers/LayerTreeModel.h"

namespace GISApp::Layers {

/**
 * @class LayerManager
 * @brief Single Responsibility Controller for application-wide Layer Tree operations.
 */
class LayerManager : public QObject {
    Q_OBJECT

public:
    explicit LayerManager(QObject *parent = nullptr);
    ~LayerManager() override = default;

    LayerTreeModel* model() const;

    /**
     * @brief Create a new Layer Group in the tree.
     * @param groupName Display name of the group.
     * @param parentGroup Parent group (nullptr for root).
     * @return LayerGroupNode* Pointer to created group node.
     */
    LayerGroupNode* addGroup(const QString &groupName, LayerGroupNode *parentGroup = nullptr);

    /**
     * @brief Add an individual layer node under a group or root.
     * @param layerName Display name of the layer.
     * @param adapter Shared ILayerAdapter implementation.
     * @param parentGroup Target parent group.
     * @return LayerNode* Pointer to created layer node.
     */
    LayerNode* addLayer(const QString &layerName, std::shared_ptr<ILayerAdapter> adapter, LayerGroupNode *parentGroup = nullptr);

    /**
     * @brief Set layer or group visibility.
     * @param node Target node.
     * @param visible True for visible, false for hidden.
     */
    void setVisibility(LayerTreeNode *node, bool visible);

    /**
     * @brief Set layer or group opacity.
     * @param node Target node.
     * @param opacity Float between 0.0 and 1.0.
     */
    void setOpacity(LayerTreeNode *node, float opacity);

    /**
     * @brief Remove a node from the tree.
     * @param node Target node.
     */
    void removeNode(LayerTreeNode *node);

    bool moveUp(LayerTreeNode *node);
    bool moveDown(LayerTreeNode *node);

    /**
     * @brief Synchronize MapLibre engine Z-stack with the UI tree list order.
     */
    void syncRenderOrder();

    /**
     * @brief Request panning/zooming to the geographic extent of a node.
     * @param node Target node.
     */
    void panToExtent(LayerTreeNode *node);

    /**
     * @brief Find a LayerNode by its underlying layer ID string.
     * @param layerId Unique layer ID in MapLibre style stack.
     * @return LayerNode* Pointer to matching layer node or nullptr if not present.
     */
    LayerNode* findLayerByLayerId(const QString &layerId) const;

    /**
     * @brief Find a LayerGroupNode by group name.
     * @param groupName Name of the group to search for.
     * @return LayerGroupNode* Pointer to matching group node or nullptr.
     */
    LayerGroupNode* findGroupByName(const QString &groupName) const;

signals:
    /**
     * @brief Emitted when a pan to extent action is triggered.
     * @param extent Spatial bounding box coordinates.
     */
    void panToExtentRequested(const LayerExtent &extent);

private:
    LayerTreeModel *m_model;
};

} // namespace GISApp::Layers

#endif // LAYERMANAGER_H
