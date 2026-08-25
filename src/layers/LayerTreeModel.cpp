/**
 * @file LayerTreeModel.cpp
 * @brief Implementation of Qt LayerTreeModel.
 */

#include "layers/LayerTreeModel.h"

namespace GISApp::Layers {

LayerTreeModel::LayerTreeModel(QObject *parent)
    : QAbstractItemModel(parent), m_rootNode(std::make_unique<LayerGroupNode>("Root"))
{
}

LayerGroupNode* LayerTreeModel::rootNode() const {
    return m_rootNode.get();
}

QModelIndex LayerTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();

    LayerTreeNode *parentNode = nodeFromIndex(parent);
    LayerTreeNode *childNode = parentNode->child(row);
    if (childNode) {
        return createIndex(row, column, childNode);
    }
    return QModelIndex();
}

QModelIndex LayerTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) return QModelIndex();

    LayerTreeNode *childNode = static_cast<LayerTreeNode*>(child.internalPointer());
    LayerTreeNode *parentNode = childNode->parentNode();

    if (parentNode == m_rootNode.get() || !parentNode) {
        return QModelIndex();
    }

    return createIndex(parentNode->row(), 0, parentNode);
}

int LayerTreeModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0) return 0;

    LayerTreeNode *parentNode = nodeFromIndex(parent);
    return parentNode ? parentNode->childCount() : 0;
}

int LayerTreeModel::columnCount(const QModelIndex &) const {
    return 1; // Single column tree with checkbox and layer title
}

QVariant LayerTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    LayerTreeNode *node = static_cast<LayerTreeNode*>(index.internalPointer());

    switch (role) {
        case Qt::DisplayRole:
            return node->name();
        case Qt::CheckStateRole:
            return node->checkState();
        case OpacityRole:
            return node->opacity();
        case NodeTypeRole:
            return static_cast<int>(node->nodeType());
        default:
            break;
    }
    return QVariant();
}

bool LayerTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) return false;

    LayerTreeNode *node = static_cast<LayerTreeNode*>(index.internalPointer());

    if (role == Qt::CheckStateRole) {
        node->setCheckState(static_cast<Qt::CheckState>(value.toInt()));
        emit layoutChanged();
        return true;
    } else if (role == OpacityRole) {
        node->setOpacity(value.toFloat());
        emit dataChanged(index, index, {OpacityRole});
        return true;
    }

    return false;
}

Qt::ItemFlags LayerTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

LayerTreeNode* LayerTreeModel::nodeFromIndex(const QModelIndex &index) const {
    if (index.isValid()) {
        return static_cast<LayerTreeNode*>(index.internalPointer());
    }
    return m_rootNode.get();
}

QModelIndex LayerTreeModel::indexFromNode(LayerTreeNode *node) const {
    if (!node || node == m_rootNode.get()) return QModelIndex();
    return createIndex(node->row(), 0, node);
}

bool LayerTreeModel::moveNodeUp(const QModelIndex &index) {
    if (!index.isValid()) return false;
    LayerTreeNode *node = nodeFromIndex(index);
    if (!node || !node->parentNode()) return false;
    LayerGroupNode *parentGroup = static_cast<LayerGroupNode*>(node->parentNode());
    int curRow = node->row();
    if (curRow <= 0) return false;

    beginMoveRows(parent(index), curRow, curRow, parent(index), curRow - 1);
    parentGroup->swapChildren(curRow, curRow - 1);
    endMoveRows();

    emit orderChanged();
    emit layoutChanged();
    return true;
}

bool LayerTreeModel::moveNodeDown(const QModelIndex &index) {
    if (!index.isValid()) return false;
    LayerTreeNode *node = nodeFromIndex(index);
    if (!node || !node->parentNode()) return false;
    LayerGroupNode *parentGroup = static_cast<LayerGroupNode*>(node->parentNode());
    int curRow = node->row();
    if (curRow >= parentGroup->childCount() - 1) return false;

    beginMoveRows(parent(index), curRow + 1, curRow + 1, parent(index), curRow);
    parentGroup->swapChildren(curRow, curRow + 1);
    endMoveRows();

    emit orderChanged();
    emit layoutChanged();
    return true;
}

bool LayerTreeModel::removeNode(LayerTreeNode *node) {
    if (!node || node == m_rootNode.get()) return false;

    LayerTreeNode *parent = node->parentNode();
    if (!parent || parent->nodeType() != NodeType::Group) return false;

    LayerGroupNode *parentGroup = static_cast<LayerGroupNode*>(parent);
    int r = node->row();
    QModelIndex parentIdx = indexFromNode(parentGroup);

    beginRemoveRows(parentIdx, r, r);
    parentGroup->removeChild(r);
    endRemoveRows();

    emit orderChanged();
    emit layoutChanged();
    return true;
}

} // namespace GISApp::Layers
