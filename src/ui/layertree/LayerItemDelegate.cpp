/**
 * @file LayerItemDelegate.cpp
 * @brief Custom Delegate implementation for right-aligned tactical Layer action icons.
 */

#include "ui/layertree/LayerItemDelegate.h"
#include "layers/LayerTreeModel.h"
#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QToolTip>
#include <QApplication>

namespace GISApp::UI {

LayerItemDelegate::LayerItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

LayerItemDelegate::ActionButtons LayerItemDelegate::getActionButtons(const QRect &optionRect) const
{
    ActionButtons btns;
    int btnWidth = 20;
    int btnHeight = 18;
    int spacing = 4;
    int rightMargin = 6;

    int y = optionRect.top() + (optionRect.height() - btnHeight) / 2;

    int xOpacity = optionRect.right() - rightMargin - btnWidth;
    int xPan     = xOpacity - spacing - btnWidth;
    int xVis     = xPan - spacing - btnWidth;

    btns.opacityRect = QRect(xOpacity, y, btnWidth, btnHeight);
    btns.panRect     = QRect(xPan, y, btnWidth, btnHeight);
    btns.visRect     = QRect(xVis, y, btnWidth, btnHeight);

    return btns;
}

QSize LayerItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(qMax(size.height(), 26));
    return size;
}

void LayerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Draw standard QTreeView item (Text, Checkbox, Indentation)
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Reserve 75px space on the right for action icons so text doesn't overlap
    opt.rect.setRight(opt.rect.right() - 75);
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    ActionButtons btns = getActionButtons(option.rect);

    // 1. Fetch Item Properties from LayerTreeModel
    bool isVisible = (index.data(Qt::CheckStateRole).toInt() == Qt::Checked);
    float opacity = index.data(GISApp::Layers::LayerTreeModel::OpacityRole).toFloat();

    // 2. Draw Visibility Toggle Icon (👁)
    QColor visColor = isVisible ? QColor("#10b981") : QColor("#6b7280");
    painter->setPen(visColor);
    painter->setBrush(QColor(visColor.red(), visColor.green(), visColor.blue(), 30));
    painter->drawRoundedRect(btns.visRect, 3, 3);
    painter->drawText(btns.visRect, Qt::AlignCenter, isVisible ? "👁" : "🕶");

    // 3. Draw Pan to Extent Icon (🎯)
    QColor panColor = QColor("#3b82f6");
    painter->setPen(panColor);
    painter->setBrush(QColor(59, 130, 246, 30));
    painter->drawRoundedRect(btns.panRect, 3, 3);
    painter->drawText(btns.panRect, Qt::AlignCenter, "🎯");

    // 4. Draw Opacity Icon (💧) & Percentage
    QColor opacityColor = (opacity > 0.8f) ? QColor("#06b6d4") : ((opacity > 0.3f) ? QColor("#f59e0b") : QColor("#ef4444"));
    painter->setPen(opacityColor);
    painter->setBrush(QColor(opacityColor.red(), opacityColor.green(), opacityColor.blue(), 30));
    painter->drawRoundedRect(btns.opacityRect, 3, 3);
    painter->drawText(btns.opacityRect, Qt::AlignCenter, "💧");

    painter->restore();
}

bool LayerItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!index.isValid()) return false;

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        ActionButtons btns = getActionButtons(option.rect);
        QPoint pos = mouseEvent->pos();

        GISApp::Layers::LayerTreeModel *treeModel = qobject_cast<GISApp::Layers::LayerTreeModel*>(const_cast<QAbstractItemModel*>(model));
        GISApp::Layers::LayerTreeNode *node = treeModel ? treeModel->nodeFromIndex(index) : nullptr;

        if (node) {
            // 1. Visibility Button Clicked
            if (btns.visRect.contains(pos)) {
                if (event->type() == QEvent::MouseButtonRelease) {
                    emit visibilityToggleRequested(index);
                }
                return true;
            }

            // 2. Pan / Zoom to Extent Button Clicked
            if (btns.panRect.contains(pos)) {
                if (event->type() == QEvent::MouseButtonRelease) {
                    GISApp::Layers::LayerExtent extent = node->getExtent();
                    if (extent.isValid()) {
                        emit zoomToExtentRequested(extent);
                    } else {
                        QToolTip::showText(mouseEvent->globalPosition().toPoint(), "No spatial extent available for this item.");
                    }
                }
                return true;
            }

            // 3. Transparency / Opacity Button Clicked
            if (btns.opacityRect.contains(pos)) {
                if (event->type() == QEvent::MouseButtonRelease) {
                    int currentPercent = static_cast<int>(node->opacity() * 100.0f);
                    bool ok = false;
                    QWidget *parentWidget = const_cast<QWidget*>(option.widget);
                    int newPercent = QInputDialog::getInt(parentWidget, "Layer Transparency",
                                                          QString("Set Opacity for '%1' (0-100%):").arg(node->name()),
                                                          currentPercent, 0, 100, 5, &ok);
                    if (ok) {
                        emit opacityChangeRequested(index, static_cast<float>(newPercent) / 100.0f);
                    }
                }
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

} // namespace GISApp::UI
