/**
 * @file LayerItemDelegate.h
 * @brief Custom QStyledItemDelegate rendering tactical action icons (Visibility, Pan to Extent, Opacity).
 */

#ifndef LAYERITEMDELEGATE_H
#define LAYERITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include "layers/ILayerAdapter.h"

namespace GISApp::UI {

/**
 * @class LayerItemDelegate
 * @brief Renders right-aligned interactive buttons on Layer Tree items and captures click events.
 */
class LayerItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit LayerItemDelegate(QObject *parent = nullptr);
    ~LayerItemDelegate() override = default;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void visibilityToggleRequested(const QModelIndex &index);
    void zoomToExtentRequested(const GISApp::Layers::LayerExtent &extent);
    void opacityChangeRequested(const QModelIndex &index, float opacity);

private:
    struct ActionButtons {
        QRect visRect;
        QRect panRect;
        QRect opacityRect;
    };

    ActionButtons getActionButtons(const QRect &optionRect) const;
};

} // namespace GISApp::UI

#endif // LAYERITEMDELEGATE_H
