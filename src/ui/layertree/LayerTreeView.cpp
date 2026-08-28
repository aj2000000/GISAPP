/**
 * @file LayerTreeView.cpp
 * @brief Implementation of LayerTreeView widget with right-aligned action icons and ingestion context options.
 */

#include "ui/layertree/LayerTreeView.h"
#include "layers/LayerTreeModel.h"
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>

namespace GISApp::UI {

LayerTreeView::LayerTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setObjectName("LayerTreeView");
    setHeaderHidden(true);
    setAnimated(true);
    setIndentation(16);
    setSelectionMode(QAbstractItemView::SingleSelection);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Set custom Item Delegate for right-aligned Visibility, Pan, and Opacity icons
    m_itemDelegate = new LayerItemDelegate(this);
    setItemDelegate(m_itemDelegate);

    // Connect Delegate Signals to LayerManager and Parent View Signals
    connect(m_itemDelegate, &LayerItemDelegate::zoomToExtentRequested,
            this, &LayerTreeView::zoomToExtentRequested);

    connect(m_itemDelegate, &LayerItemDelegate::visibilityToggleRequested, [this](const QModelIndex &index) {
        if (!m_layerManager || !m_layerManager->model()) return;
        GISApp::Layers::LayerTreeNode *node = m_layerManager->model()->nodeFromIndex(index);
        if (node) {
            bool isVisible = (node->checkState() == Qt::Checked);
            m_layerManager->setVisibility(node, !isVisible);
        }
    });

    connect(m_itemDelegate, &LayerItemDelegate::opacityChangeRequested, [this](const QModelIndex &index, float opacity) {
        if (!m_layerManager || !m_layerManager->model()) return;
        GISApp::Layers::LayerTreeNode *node = m_layerManager->model()->nodeFromIndex(index);
        if (node) {
            m_layerManager->setOpacity(node, opacity);
        }
    });

    // Apply Tactical Dark QSS
    setStyleSheet(R"(
        QTreeView#LayerTreeView {
            background-color: #0f1317;
            color: #d1d5db;
            border: 1px solid #1f2937;
            border-radius: 6px;
            padding: 4px;
        }
        QTreeView#LayerTreeView::item {
            padding: 4px;
            border-radius: 4px;
        }
        QTreeView#LayerTreeView::item:hover {
            background-color: rgba(255, 255, 255, 0.10);
            color: #ffffff;
        }
        QTreeView#LayerTreeView::item:selected {
            background-color: rgba(16, 185, 129, 0.20);
            color: #10b981;
            border: 1px solid rgba(16, 185, 129, 0.40);
        }
        QTreeView#LayerTreeView::indicator {
            width: 14px;
            height: 14px;
        }
    )");
}

void LayerTreeView::setLayerManager(GISApp::Layers::LayerManager *manager) {
    m_layerManager = manager;
    if (m_layerManager && m_layerManager->model()) {
        setModel(m_layerManager->model());
        expandAll();
    }
}

void LayerTreeView::contextMenuEvent(QContextMenuEvent *event) {
    QMenu contextMenu(this);
    contextMenu.setStyleSheet(R"(
        QMenu {
            background-color: #111827;
            color: #e5e7eb;
            border: 1px solid #374151;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item:selected {
            background-color: #10b981;
            color: #000000;
        }
    )");

    QModelIndex idx = indexAt(event->pos());
    GISApp::Layers::LayerTreeNode *node = (m_layerManager && idx.isValid()) ? m_layerManager->model()->nodeFromIndex(idx) : nullptr;

    if (node) {
        QAction *zoomAction = contextMenu.addAction("🎯 Pan to Extent");
        QAction *opacityAction = contextMenu.addAction("💧 Set Opacity...");
        QAction *moveUpAction = contextMenu.addAction("⬆ Move Up (Render Higher)");
        QAction *moveDownAction = contextMenu.addAction("🔽 Move Down (Render Lower)");
        QAction *toggleAction = contextMenu.addAction(node->checkState() == Qt::Checked ? "👁 Hide Layer" : "👁 Show Layer");
        contextMenu.addSeparator();
        QAction *removeAction = contextMenu.addAction(node->nodeType() == GISApp::Layers::NodeType::Group ? "🗑 Remove Group" : "🗑 Remove Layer");

        connect(zoomAction, &QAction::triggered, [this, node]() {
            GISApp::Layers::LayerExtent extent = node->getExtent();
            if (extent.isValid()) {
                emit zoomToExtentRequested(extent);
            }
        });

        connect(opacityAction, &QAction::triggered, [this, node]() {
            bool ok = false;
            int currentPercent = static_cast<int>(node->opacity() * 100.0f);
            int percent = QInputDialog::getInt(this, "Layer Opacity", "Opacity Percentage (0-100%):", currentPercent, 0, 100, 5, &ok);
            if (ok) {
                m_layerManager->setOpacity(node, static_cast<float>(percent) / 100.0f);
            }
        });

        connect(moveUpAction, &QAction::triggered, [this, node]() {
            if (m_layerManager) {
                m_layerManager->moveUp(node);
            }
        });

        connect(moveDownAction, &QAction::triggered, [this, node]() {
            if (m_layerManager) {
                m_layerManager->moveDown(node);
            }
        });

        connect(toggleAction, &QAction::triggered, [this, node]() {
            bool isVisible = (node->checkState() == Qt::Checked);
            m_layerManager->setVisibility(node, !isVisible);
        });

        connect(removeAction, &QAction::triggered, [this, node]() {
            if (!m_layerManager) return;
            QString itemType = (node->nodeType() == GISApp::Layers::NodeType::Group) ? "group" : "layer";
            if (QMessageBox::question(this, "Confirm Removal",
                                      QString("Are you sure you want to remove the %1 '%2'?").arg(itemType, node->name()),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                m_layerManager->removeNode(node);
            }
        });

        contextMenu.addSeparator();
    }

    // Ingestion Actions available directly from Layer Tree
    QAction *ingestAovAction = contextMenu.addAction("📥 Ingest Area of View (XML)...");
    QAction *ingestTracksAction = contextMenu.addAction("📥 Ingest Tactical Tracks (CSV)...");

    connect(ingestAovAction, &QAction::triggered, this, &LayerTreeView::ingestAreaOfViewRequested);
    connect(ingestTracksAction, &QAction::triggered, this, &LayerTreeView::ingestTracksRequested);

    contextMenu.exec(event->globalPos());
}

} // namespace GISApp::UI
