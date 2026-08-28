/**
 * @file FunctionalTask.h
 * @brief Concrete Command wrapper adapting functional lambdas into IBackgroundTask objects.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef FUNCTIONALTASK_H
#define FUNCTIONALTASK_H

#include "core/tasks/IBackgroundTask.h"
#include <QMutex>

namespace GISApp::Core::Tasks {

/**
 * @class FunctionalTask
 * @brief Concrete Command wrapping std::function worker lambdas.
 * Enables backwards compatibility and functional execution within the IBackgroundTask command framework.
 */
class FunctionalTask : public IBackgroundTask {
public:
    using WorkerFunction = std::function<void(QString taskId)>;

    FunctionalTask(const QString &id, const QString &name, WorkerFunction worker);
    ~FunctionalTask() override = default;

    bool execute(ProgressCallback progressCb) override;
    void cancel() override;

    QString id() const override;
    QString name() const override;
    TaskState state() const override;
    int progress() const override;
    QString statusText() const override;
    QDateTime startTime() const override;
    QDateTime endTime() const override;

    void updateProgress(int percent, const QString &status);
    void setState(TaskState state, const QString &finalStatus = QString());

private:
    QString m_id;
    QString m_name;
    WorkerFunction m_worker;
    TaskState m_state{TaskState::Queued};
    int m_progress{0};
    QString m_statusText;
    QDateTime m_startTime;
    QDateTime m_endTime;
    mutable QMutex m_mutex;
};

} // namespace GISApp::Core::Tasks

#endif // FUNCTIONALTASK_H
