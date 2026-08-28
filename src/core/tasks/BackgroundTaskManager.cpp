/**
 * @file BackgroundTaskManager.cpp
 * @brief Implementation of thread-safe BackgroundTaskManager.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "core/tasks/BackgroundTaskManager.h"
#include "core/tasks/FunctionalTask.h"
#include <QMutexLocker>
#include <QUuid>
#include <QtConcurrent>
#include <QDebug>
#include <cmath>

namespace GISApp::Core::Tasks {

BackgroundTaskManager& BackgroundTaskManager::instance()
{
    static BackgroundTaskManager _instance;
    return _instance;
}

BackgroundTaskManager::BackgroundTaskManager(QObject *parent)
    : QObject(parent)
{
}

QString BackgroundTaskManager::submitTaskCommand(TaskPtr task)
{
    if (!task) return QString();

    QMutexLocker locker(&m_mutex);
    QString taskId = task->id();

    TaskInfo info;
    info.id = taskId;
    info.name = task->name();
    info.progress = task->progress();
    info.statusText = task->statusText().isEmpty() ? "Task queued in background..." : task->statusText();
    info.state = TaskState::Running;
    info.startTime = task->startTime().isValid() ? task->startTime() : QDateTime::currentDateTime();
    info.commandTask = task;

    m_tasks.append(info);

    emit taskStarted(taskId, task->name());

    QtConcurrent::run([this, task]() {
        bool success = task->execute([this, task](int percent, const QString &status) {
            updateProgress(task->id(), percent, status);
        });

        if (task->state() == TaskState::Cancelled) {
            cancelTask(task->id());
        } else if (success) {
            markCompleted(task->id(), task->statusText().isEmpty() ? "Task finished successfully." : task->statusText());
        } else {
            markFailed(task->id(), task->statusText().isEmpty() ? "Task failed during execution." : task->statusText());
        }
    });

    return taskId;
}

QString BackgroundTaskManager::submitTask(const QString &taskName, std::function<void(QString taskId)> workerTask)
{
    QString taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto funcTask = std::make_shared<FunctionalTask>(taskId, taskName, workerTask);
    return submitTaskCommand(funcTask);
}

void BackgroundTaskManager::registerProcess(const QString &taskId, QProcess *proc)
{
    QMutexLocker locker(&m_mutex);
    for (auto &task : m_tasks) {
        if (task.id == taskId) {
            task.process = proc;
            break;
        }
    }
}

void BackgroundTaskManager::updateProgress(const QString &taskId, int percent, const QString &status)
{
    QMutexLocker locker(&m_mutex);
    for (auto &task : m_tasks) {
        if (task.id == taskId) {
            task.progress = std::clamp(percent, 0, 100);
            task.statusText = status;
            break;
        }
    }
    emit taskProgressUpdated(taskId, percent, status);
}

void BackgroundTaskManager::markCompleted(const QString &taskId, const QString &completionMsg)
{
    QMutexLocker locker(&m_mutex);
    for (auto &task : m_tasks) {
        if (task.id == taskId) {
            task.progress = 100;
            task.statusText = completionMsg;
            task.state = TaskState::Completed;
            task.endTime = QDateTime::currentDateTime();
            task.process = nullptr;
            break;
        }
    }
    emit taskCompleted(taskId, completionMsg);
}

void BackgroundTaskManager::markFailed(const QString &taskId, const QString &errorMsg)
{
    QMutexLocker locker(&m_mutex);
    for (auto &task : m_tasks) {
        if (task.id == taskId) {
            task.statusText = errorMsg;
            task.state = TaskState::Failed;
            task.endTime = QDateTime::currentDateTime();
            task.process = nullptr;
            break;
        }
    }
    emit taskFailed(taskId, errorMsg);
}

void BackgroundTaskManager::cancelTask(const QString &taskId)
{
    QMutexLocker locker(&m_mutex);
    for (auto &task : m_tasks) {
        if (task.id == taskId) {
            if (task.commandTask) {
                task.commandTask->cancel();
            }
            if (task.process && task.process->state() != QProcess::NotRunning) {
                task.process->kill();
            }
            task.state = TaskState::Cancelled;
            task.statusText = "Task cancelled by user.";
            task.endTime = QDateTime::currentDateTime();
            task.process = nullptr;
            break;
        }
    }
    emit taskCancelled(taskId);
}

QVector<TaskInfo> BackgroundTaskManager::allTasks() const
{
    QMutexLocker locker(&m_mutex);
    return m_tasks;
}

int BackgroundTaskManager::activeTaskCount() const
{
    QMutexLocker locker(&m_mutex);
    int count = 0;
    for (const auto &task : m_tasks) {
        if (task.state == TaskState::Running || task.state == TaskState::Queued) {
            count++;
        }
    }
    return count;
}

QString BackgroundTaskManager::formatTimeRemaining(const QDateTime &startTime, int currentPercent)
{
    if (!startTime.isValid() || currentPercent <= 0) {
        return "Calculating ETC...";
    }
    if (currentPercent >= 100) {
        return "Done (0s)";
    }

    qint64 elapsedSecs = startTime.secsTo(QDateTime::currentDateTime());
    if (elapsedSecs <= 0) {
        return "Calculating ETC...";
    }

    double totalEstimatedSecs = (static_cast<double>(elapsedSecs) / currentPercent) * 100.0;
    qint64 remainingSecs = std::max<qint64>(0, std::round(totalEstimatedSecs - elapsedSecs));

    if (remainingSecs < 60) {
        return QString("%1s remaining").arg(remainingSecs);
    } else if (remainingSecs < 3600) {
        qint64 mins = remainingSecs / 60;
        qint64 secs = remainingSecs % 60;
        return QString("%1m %2s remaining").arg(mins).arg(secs);
    } else {
        qint64 hrs = remainingSecs / 3600;
        qint64 mins = (remainingSecs % 3600) / 60;
        return QString("%1h %2m remaining").arg(hrs).arg(mins);
    }
}

QString BackgroundTaskManager::formatElapsedTime(const QDateTime &startTime, const QDateTime &endTime)
{
    if (!startTime.isValid()) return "00:00";
    QDateTime targetTime = endTime.isValid() ? endTime : QDateTime::currentDateTime();
    qint64 elapsedSecs = startTime.secsTo(targetTime);
    if (elapsedSecs < 0) elapsedSecs = 0;
    qint64 mins = elapsedSecs / 60;
    qint64 secs = elapsedSecs % 60;
    return QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

} // namespace GISApp::Core::Tasks
