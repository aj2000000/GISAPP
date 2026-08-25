/**
 * @file GroupManagerDialog.cpp
 * @brief Implementation of GroupManagerDialog.
 */

#include "ui/publishing/GroupManagerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>

namespace GISApp::UI::Publishing {

GroupManagerDialog::GroupManagerDialog(GISApp::Layers::LayerManager *layerManager, QWidget *parent)
    : QDialog(parent), m_layerManager(layerManager)
{
    setWindowTitle(tr("Manage Layer Groups"));
    resize(380, 420);
    setupUI();
    populateGroupList();
}

void GroupManagerDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_listGroups = new QListWidget(this);
    layout->addWidget(m_listGroups);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnAdd = new QPushButton(tr("Add Group"), this);
    m_btnRename = new QPushButton(tr("Rename"), this);
    m_btnDelete = new QPushButton(tr("Delete"), this);
    btnLayout->addWidget(m_btnAdd);
    btnLayout->addWidget(m_btnRename);
    btnLayout->addWidget(m_btnDelete);
    layout->addLayout(btnLayout);

    m_btnClose = new QPushButton(tr("Close"), this);
    layout->addWidget(m_btnClose);

    connect(m_btnAdd, &QPushButton::clicked, this, &GroupManagerDialog::onAddGroup);
    connect(m_btnRename, &QPushButton::clicked, this, &GroupManagerDialog::onRenameGroup);
    connect(m_btnDelete, &QPushButton::clicked, this, &GroupManagerDialog::onDeleteGroup);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(R"(
        QDialog { background-color: #111827; color: #f3f4f6; }
        QListWidget { background-color: #1f2937; color: #ffffff; border: 1px solid #374151; border-radius: 4px; }
        QPushButton { background-color: #374151; color: #ffffff; border-radius: 4px; padding: 6px; }
        QPushButton:hover { background-color: #4b5563; }
    )");
}

void GroupManagerDialog::populateGroupList()
{
    m_listGroups->clear();
    if (!m_layerManager || !m_layerManager->model()) return;

    auto root = m_layerManager->model()->rootNode();
    for (int i = 0; i < root->childCount(); ++i) {
        auto child = root->child(i);
        if (child->nodeType() == GISApp::Layers::NodeType::Group) {
            QListWidgetItem *item = new QListWidgetItem("📁 " + child->name(), m_listGroups);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>(child));
        }
    }
}

void GroupManagerDialog::onAddGroup()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New Group"), tr("Group Name:"), QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty() && m_layerManager) {
        m_layerManager->addGroup(name);
        populateGroupList();
    }
}

void GroupManagerDialog::onRenameGroup()
{
    auto item = m_listGroups->currentItem();
    if (!item) return;

    GISApp::Layers::LayerGroupNode *node = static_cast<GISApp::Layers::LayerGroupNode*>(
        item->data(Qt::UserRole).value<void*>());

    if (node) {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Rename Group"), tr("New Group Name:"), QLineEdit::Normal, node->name(), &ok);
        if (ok && !name.isEmpty()) {
            node->setName(name);
            emit m_layerManager->model()->layoutChanged();
            populateGroupList();
        }
    }
}

void GroupManagerDialog::onDeleteGroup()
{
    auto item = m_listGroups->currentItem();
    if (!item || !m_layerManager) return;

    GISApp::Layers::LayerGroupNode *node = static_cast<GISApp::Layers::LayerGroupNode*>(
        item->data(Qt::UserRole).value<void*>());

    if (node) {
        if (QMessageBox::question(this, tr("Confirm Delete"), tr("Are you sure you want to delete group '%1'?").arg(node->name())) == QMessageBox::Yes) {
            m_layerManager->removeNode(node);
            populateGroupList();
        }
    }
}

} // namespace GISApp::UI::Publishing
