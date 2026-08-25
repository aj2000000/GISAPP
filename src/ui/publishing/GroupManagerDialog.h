/**
 * @file GroupManagerDialog.h
 * @brief Modal dialog to manage Layer Groups (Add, Rename, Delete).
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef GROUPMANAGERDIALOG_H
#define GROUPMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include "layers/LayerManager.h"

namespace GISApp::UI::Publishing {

/**
 * @class GroupManagerDialog
 * @brief GUI Dialog for managing Layer Group nodes in LayerTreeModel.
 */
class GroupManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit GroupManagerDialog(GISApp::Layers::LayerManager *layerManager, QWidget *parent = nullptr);
    ~GroupManagerDialog() override = default;

private slots:
    void onAddGroup();
    void onRenameGroup();
    void onDeleteGroup();

private:
    void setupUI();
    void populateGroupList();

    GISApp::Layers::LayerManager *m_layerManager;
    QListWidget *m_listGroups;
    QPushButton *m_btnAdd;
    QPushButton *m_btnRename;
    QPushButton *m_btnDelete;
    QPushButton *m_btnClose;
};

} // namespace GISApp::UI::Publishing

#endif // GROUPMANAGERDIALOG_H
