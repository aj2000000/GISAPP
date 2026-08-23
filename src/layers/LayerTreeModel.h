/**
 * @file LayerTreeModel.h
 * @brief Qt QAbstractItemModel for Layer Tree hierarchy.
 *
 * Exposes LayerTreeNode composite hierarchy to QTreeView.
 *
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef LAYERTREEMODEL_H
#define LAYERTREEMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include "layers/LayerTreeNode.h"

namespace GISApp::Layers {

/**
 * @class LayerTreeModel
 * @brief Bridge model connecting Composite LayerTreeNodes to Qt QTreeView.
 */
class LayerTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum CustomRoles {
        OpacityRole = Qt::UserRole + 1,
        ExtentRole = Qt::UserRole + 2,
        NodeTypeRole = Qt::UserRole + 3
    };

    explicit LayerTreeModel(QObject *parent = nullptr);
    ~LayerTreeModel() override = default;

    LayerGroupNode* rootNode() const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    LayerTreeNode* nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(LayerTreeNode *node) const;

    void beginNodeInsert(const QModelIndex &parent, int first, int last) {
        beginInsertRows(parent, first, last);
    }
    void endNodeInsert() {
        endInsertRows();
    }

private:
    std::unique_ptr<LayerGroupNode> m_rootNode;
};

} // namespace GISApp::Layers

#endif // LAYERTREEMODEL_H
