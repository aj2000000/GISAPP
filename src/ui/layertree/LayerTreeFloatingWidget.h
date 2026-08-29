/**
 * @file LayerTreeFloatingWidget.h
 * @brief Floating, draggable tactical overlay panel for Layer Hierarchy Management.
 */

#ifndef LAYERTREEFLOATINGWIDGET_H
#define LAYERTREEFLOATINGWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "ui/layertree/LayerTreeView.h"
#include "layers/LayerManager.h"

namespace GISApp::UI {

/**
 * @class LayerTreeFloatingWidget
 * @brief Translucent draggable floating panel housing LayerTreeView over map canvas.
 */
class LayerTreeFloatingWidget : public QFrame {
    Q_OBJECT

public:
    explicit LayerTreeFloatingWidget(QWidget *parent = nullptr);
    ~LayerTreeFloatingWidget() override = default;

    /**
     * @brief Set LayerManager facade for tree model data binding.
     */
    void setLayerManager(GISApp::Layers::LayerManager *manager);

    /**
     * @brief Access embedded LayerTreeView widget.
     */
    LayerTreeView* treeView() const { return m_treeView; }

signals:
    void closed();
    void ingestAreaOfViewRequested();
    void ingestTracksRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QWidget *m_headerBar;
    QLabel *m_titleLabel;
    QPushButton *m_collapseButton;
    QPushButton *m_closeButton;
    LayerTreeView *m_treeView;



    QVBoxLayout *m_mainLayout;

    QPoint m_dragPosition;
    bool m_isCollapsed{false};
};

} // namespace GISApp::UI

#endif // LAYERTREEFLOATINGWIDGET_H
