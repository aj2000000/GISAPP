/**
 * @file BackgroundTaskDialog.h
 * @brief Qt GUI Dialog for viewing, tracking, and cancelling background GIS tasks.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef BACKGROUNDTASKDIALOG_H
#define BACKGROUNDTASKDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "core/tasks/BackgroundTaskManager.h"

namespace GISApp::UI::Tasks {

/**
 * @class BackgroundTaskDialog
 * @brief Tactical dialog displaying active background publishing pipelines and progress bars.
 */
class BackgroundTaskDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackgroundTaskDialog(QWidget *parent = nullptr);
    ~BackgroundTaskDialog() override = default;

public slots:
    void refreshTaskTable();

private slots:
    void onCancelTaskClicked();

private:
    void setupUI();

    QTableWidget *m_taskTable;
    QPushButton *m_btnClose;
    QTimer *m_refreshTimer;
};

} // namespace GISApp::UI::Tasks

#endif // BACKGROUNDTASKDIALOG_H
