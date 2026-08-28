/**
 * @file IBackgroundTask.h
 * @brief Abstract Command interface for Asynchronous Background GIS Tasks.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IBACKGROUNDTASK_H
#define IBACKGROUNDTASK_H

#include <QString>
#include <QDateTime>
#include <QProcess>
#include <functional>
#include <memory>

namespace GISApp::Core::Tasks {

/**
 * @enum TaskState
 * @brief Lifecycle states for background task execution.
 */
enum class TaskState {
    Queued,
    Running,
    Completed,
    Failed,
    Cancelled
};

using ProgressCallback = std::function<void(int percent, const QString &statusText)>;

/**
 * @class IBackgroundTask
 * @brief Command Pattern interface defining background task execution, lifecycle, and progress control.
 * Applies Single Responsibility Principle (SRP), Liskov Substitution Principle (LSP), and Interface Segregation (ISP).
 */
class IBackgroundTask {
public:
    virtual ~IBackgroundTask() = default;

    /**
     * @brief Executes the task operation synchronously within a background worker context.
     * @param progressCb Callback to report percentage (0-100) and status string updates.
     * @return True if execution succeeded, false otherwise.
     */
    virtual bool execute(ProgressCallback progressCb) = 0;

    /**
     * @brief Requests cancellation of the background task operation.
     */
    virtual void cancel() = 0;

    /**
     * @brief Gets unique task identifier.
     */
    virtual QString id() const = 0;

    /**
     * @brief Gets human-readable task display name.
     */
    virtual QString name() const = 0;

    /**
     * @brief Gets current lifecycle state.
     */
    virtual TaskState state() const = 0;

    /**
     * @brief Gets progress percentage (0 - 100).
     */
    virtual int progress() const = 0;

    /**
     * @brief Gets current diagnostic status message.
     */
    virtual QString statusText() const = 0;

    /**
     * @brief Gets timestamp when task started.
     */
    virtual QDateTime startTime() const = 0;

    /**
     * @brief Gets timestamp when task finished/failed/cancelled.
     */
    virtual QDateTime endTime() const = 0;

    /**
     * @brief Triggers completion notification. Default implementation in AbstractBackgroundTask displays a Flash Notification.
     */
    virtual void notifyCompletion(bool success) { Q_UNUSED(success); }
};

using TaskPtr = std::shared_ptr<IBackgroundTask>;

} // namespace GISApp::Core::Tasks

#endif // IBACKGROUNDTASK_H
