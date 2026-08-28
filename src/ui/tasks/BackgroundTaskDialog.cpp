/**
 * @file BackgroundTaskDialog.cpp
 * @brief Implementation of BackgroundTaskDialog table layout and live refresh loop with widget recycling.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "ui/tasks/BackgroundTaskDialog.h"
#include "core/tasks/BackgroundTaskManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QProgressBar>
#include <QDateTime>

namespace GISApp::UI::Tasks {

BackgroundTaskDialog::BackgroundTaskDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("⚙️ Background Spatial Tasks Monitor"));
    resize(820, 420);
    setupUI();

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &BackgroundTaskDialog::refreshTaskTable);
    m_refreshTimer->start(800); // Auto refresh every 800ms for smooth progress & ETC updates

    refreshTaskTable();
}

void BackgroundTaskDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    QLabel *titleLabel = new QLabel(tr("Active & Completed Spatial Processing Pipelines:"), this);
    titleLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #38bdf8;");
    mainLayout->addWidget(titleLabel);

    m_taskTable = new QTableWidget(this);
    m_taskTable->setColumnCount(6);
    m_taskTable->setHorizontalHeaderLabels({
        tr("Task Name"),
        tr("Progress"),
        tr("Elapsed / ETC"),
        tr("Status / Pipeline Log"),
        tr("State"),
        tr("Action")
    });

    m_taskTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_taskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_taskTable->setColumnWidth(1, 120);
    m_taskTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_taskTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_taskTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_taskTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_taskTable->verticalHeader()->setVisible(false);
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(m_taskTable);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_btnClose = new QPushButton(tr("Close"), this);
    btnLayout->addWidget(m_btnClose);
    mainLayout->addLayout(btnLayout);

    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(R"(
        QDialog {
            background-color: #0f172a;
            color: #f8fafc;
            font-family: 'Segoe UI', Inter, sans-serif;
        }
        QTableWidget {
            background-color: #1e293b;
            alternate-background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 8px;
            color: #f8fafc;
            gridline-color: #334155;
        }
        QTableWidget::item {
            color: #f8fafc;
            background-color: #1e293b;
        }
        QTableWidget::item:alternate {
            background-color: #0f172a;
            color: #f8fafc;
        }
        QTableWidget::item:selected {
            background-color: #0284c7;
            color: #ffffff;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #38bdf8;
            padding: 6px;
            font-weight: bold;
            border: 1px solid #334155;
        }
        QProgressBar {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 4px;
            text-align: center;
            color: #ffffff;
            font-weight: bold;
            font-size: 11px;
        }
        QProgressBar::chunk {
            background-color: #10b981;
            border-radius: 3px;
        }
        QPushButton {
            background-color: #334155;
            color: #f8fafc;
            border: 1px solid #475569;
            border-radius: 6px;
            padding: 6px 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #475569;
        }
    )");
}

void BackgroundTaskDialog::refreshTaskTable()
{
    auto tasks = GISApp::Core::Tasks::BackgroundTaskManager::instance().allTasks();
    if (m_taskTable->rowCount() != tasks.size()) {
        m_taskTable->setRowCount(tasks.size());
    }

    for (int i = 0; i < tasks.size(); ++i) {
        const auto &task = tasks.at(i);

        // Col 0: Task Name
        QTableWidgetItem *nameItem = m_taskTable->item(i, 0);
        if (!nameItem) {
            nameItem = new QTableWidgetItem();
            m_taskTable->setItem(i, 0, nameItem);
        }
        if (nameItem->text() != task.name) {
            nameItem->setText(task.name);
        }

        // Col 1: Progress Bar with Explicit Percentage (%p%) - REUSE WIDGET
        QProgressBar *pBar = qobject_cast<QProgressBar*>(m_taskTable->cellWidget(i, 1));
        if (!pBar) {
            pBar = new QProgressBar();
            pBar->setRange(0, 100);
            pBar->setFormat("%p%");
            m_taskTable->setCellWidget(i, 1, pBar);
        }
        pBar->setValue(task.progress);

        // Col 2: Elapsed / ETC
        QString timeInfoStr;
        QString elapsedStr = GISApp::Core::Tasks::BackgroundTaskManager::formatElapsedTime(task.startTime, task.endTime);

        if (task.state == GISApp::Core::Tasks::TaskState::Running) {
            QString remainingStr = GISApp::Core::Tasks::BackgroundTaskManager::formatTimeRemaining(task.startTime, task.progress);
            timeInfoStr = QString("⏱️ %1 | ⏳ %2").arg(elapsedStr).arg(remainingStr);
        } else if (task.state == GISApp::Core::Tasks::TaskState::Completed) {
            timeInfoStr = QString("⏱️ %1 (Finished)").arg(elapsedStr);
        } else {
            timeInfoStr = QString("⏱️ %1 (Stopped)").arg(elapsedStr);
        }
        
        QTableWidgetItem *timeItem = m_taskTable->item(i, 2);
        if (!timeItem) {
            timeItem = new QTableWidgetItem();
            m_taskTable->setItem(i, 2, timeItem);
        }
        if (timeItem->text() != timeInfoStr) {
            timeItem->setText(timeInfoStr);
        }

        // Col 3: Status / Pipeline Log
        QTableWidgetItem *statusItem = m_taskTable->item(i, 3);
        if (!statusItem) {
            statusItem = new QTableWidgetItem();
            m_taskTable->setItem(i, 3, statusItem);
        }
        if (statusItem->text() != task.statusText) {
            statusItem->setText(task.statusText);
        }

        // Col 4: State
        QString stateStr;
        switch (task.state) {
            case GISApp::Core::Tasks::TaskState::Queued: stateStr = "⏳ Queued"; break;
            case GISApp::Core::Tasks::TaskState::Running: stateStr = "⚙️ Running"; break;
            case GISApp::Core::Tasks::TaskState::Completed: stateStr = "✅ Done"; break;
            case GISApp::Core::Tasks::TaskState::Failed: stateStr = "❌ Failed"; break;
            case GISApp::Core::Tasks::TaskState::Cancelled: stateStr = "🛑 Cancelled"; break;
        }

        QTableWidgetItem *stateItem = m_taskTable->item(i, 4);
        if (!stateItem) {
            stateItem = new QTableWidgetItem();
            m_taskTable->setItem(i, 4, stateItem);
        }
        if (stateItem->text() != stateStr) {
            stateItem->setText(stateStr);
        }

        // Col 5: Action (Cancel button) - REUSE OR REMOVE WIDGET
        if (task.state == GISApp::Core::Tasks::TaskState::Running) {
            QPushButton *btnCancel = qobject_cast<QPushButton*>(m_taskTable->cellWidget(i, 5));
            if (!btnCancel) {
                btnCancel = new QPushButton(tr("Cancel"));
                btnCancel->setStyleSheet("background-color: #ef4444; color: white; padding: 2px 8px; border-radius: 4px;");
                connect(btnCancel, &QPushButton::clicked, this, &BackgroundTaskDialog::onCancelTaskClicked);
                m_taskTable->setCellWidget(i, 5, btnCancel);
            }
            btnCancel->setProperty("taskId", task.id);
        } else {
            if (m_taskTable->cellWidget(i, 5)) {
                m_taskTable->removeCellWidget(i, 5);
            }
        }
    }
}

void BackgroundTaskDialog::onCancelTaskClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        QString taskId = btn->property("taskId").toString();
        GISApp::Core::Tasks::BackgroundTaskManager::instance().cancelTask(taskId);
        refreshTaskTable();
    }
}

} // namespace GISApp::UI::Tasks
