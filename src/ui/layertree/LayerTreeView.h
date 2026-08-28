/**
 * @file LayerTreeView.h
 * @brief Interactive QTreeView Widget for Layer Management with right-aligned action icons.
 */

#ifndef LAYERTREEVIEW_H
#define LAYERTREEVIEW_H

#include <QTreeView>
#include <QMenu>
#include "layers/LayerManager.h"
#include "ui/layertree/LayerItemDelegate.h"

namespace GISApp::UI {

/**
 * @class LayerTreeView
 * @brief Custom QTreeView widget providing tactical C2 layer controls.
 */
class LayerTreeView : public QTreeView {
    Q_OBJECT

public:
    explicit LayerTreeView(QWidget *parent = nullptr);
    ~LayerTreeView() override = default;

    /**
     * @brief Bind LayerManager facade to this Tree View.
     * @param manager Pointer to active LayerManager.
     */
    void setLayerManager(GISApp::Layers::LayerManager *manager);

signals:
    /**
     * @brief Signal emitted when user triggers "Zoom to Extent".
     * @param extent Target spatial bounding box.
     */
    void zoomToExtentRequested(const GISApp::Layers::LayerExtent &extent);

    /**
     * @brief Signal emitted when user requests Area of View XML ingestion.
     */
    void ingestAreaOfViewRequested();

    /**
     * @brief Signal emitted when user requests Tactical Tracks CSV ingestion.
     */
    void ingestTracksRequested();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    GISApp::Layers::LayerManager *m_layerManager{nullptr};
    LayerItemDelegate *m_itemDelegate{nullptr};
};

} // namespace GISApp::UI

#endif // LAYERTREEVIEW_H
