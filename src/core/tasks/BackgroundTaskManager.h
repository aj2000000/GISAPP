/**
 * @file BackgroundTaskManager.h
 * @brief Thread-safe Singleton Manager for Asynchronous GIS Spatial Tasks.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef BACKGROUNDTASKMANAGER_H
#define BACKGROUNDTASKMANAGER_H

#include "core/tasks/IBackgroundTask.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>
#include <QDateTime>
#include <QProcess>
#include <functional>

namespace GISApp::Core::Tasks {

/**
 * @struct TaskInfo
 * @brief Metadata tracking execution status, progress, and logs of a task.
 */
struct TaskInfo {
    QString id;                  ///< Unique UUID/hash task identifier
    QString name;                ///< Human-readable layer or operation name
    int progress{0};             ///< Progress percentage (0 - 100)
    QString statusText;          ///< Current log message or pipeline stage
    TaskState state{TaskState::Queued}; ///< Current state of execution
    QDateTime startTime;         ///< Timestamp when task was initiated
    QDateTime endTime;           ///< Timestamp when task was completed/failed/cancelled
    QProcess *process{nullptr};  ///< Active QProcess instance (if external tool execution)
    TaskPtr commandTask;         ///< Underlying polymorphic Command object
};

/**
 * @class BackgroundTaskManager
 * @brief Core subsystem managing background threads, process lifecycle, and progress signals.
 * Implements Singleton, Command Invoker, and Observer patterns.
 */
class BackgroundTaskManager : public QObject {
    Q_OBJECT

public:
    static BackgroundTaskManager& instance();

    /**
     * @brief Submits a polymorphic IBackgroundTask Command for asynchronous execution.
     * @param task Command instance inheriting from IBackgroundTask.
     * @return Task ID string.
     */
    QString submitTaskCommand(TaskPtr task);

    /**
     * @brief Legacy helper submitting a std::function worker lambda by wrapping it in a FunctionalTask Command.
     */
    QString submitTask(const QString &taskName, std::function<void(QString taskId)> workerTask);

    void registerProcess(const QString &taskId, QProcess *proc);
    void updateProgress(const QString &taskId, int percent, const QString &status);
    void markCompleted(const QString &taskId, const QString &completionMsg);
    void markFailed(const QString &taskId, const QString &errorMsg);
    void cancelTask(const QString &taskId);
    QVector<TaskInfo> allTasks() const;
    int activeTaskCount() const;

    /**
     * @brief Computes dynamic Estimated Time to Complete (ETC / Remaining Time) string.
     */
    static QString formatTimeRemaining(const QDateTime &startTime, int currentPercent);

    /**
     * @brief Formats elapsed duration into MM:SS format.
     */
    static QString formatElapsedTime(const QDateTime &startTime, const QDateTime &endTime = QDateTime());

signals:
    void taskStarted(const QString &taskId, const QString &taskName);
    void taskProgressUpdated(const QString &taskId, int percent, const QString &status);
    void taskCompleted(const QString &taskId, const QString &completionMsg);
    void taskFailed(const QString &taskId, const QString &errorMsg);
    void taskCancelled(const QString &taskId);

private:
    explicit BackgroundTaskManager(QObject *parent = nullptr);
    ~BackgroundTaskManager() override = default;

    BackgroundTaskManager(const BackgroundTaskManager&) = delete;
    BackgroundTaskManager& operator=(const BackgroundTaskManager&) = delete;

    mutable QMutex m_mutex;
    QVector<TaskInfo> m_tasks;
};

} // namespace GISApp::Core::Tasks

#endif // BACKGROUNDTASKMANAGER_H
