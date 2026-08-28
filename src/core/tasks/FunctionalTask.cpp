/**
 * @file FunctionalTask.cpp
 * @brief Implementation of FunctionalTask command wrapper.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "core/tasks/FunctionalTask.h"

namespace GISApp::Core::Tasks {

FunctionalTask::FunctionalTask(const QString &id, const QString &name, WorkerFunction worker)
    : m_id(id), m_name(name), m_worker(worker), m_startTime(QDateTime::currentDateTime())
{
}

bool FunctionalTask::execute(ProgressCallback /*progressCb*/) {
    {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Running;
    }
    if (m_worker) {
        m_worker(m_id);
    }
    return (m_state == TaskState::Completed);
}

void FunctionalTask::cancel() {
    QMutexLocker locker(&m_mutex);
    if (m_state == TaskState::Queued || m_state == TaskState::Running) {
        m_state = TaskState::Cancelled;
        m_endTime = QDateTime::currentDateTime();
    }
}

QString FunctionalTask::id() const {
    QMutexLocker locker(&m_mutex);
    return m_id;
}

QString FunctionalTask::name() const {
    QMutexLocker locker(&m_mutex);
    return m_name;
}

TaskState FunctionalTask::state() const {
    QMutexLocker locker(&m_mutex);
    return m_state;
}

int FunctionalTask::progress() const {
    QMutexLocker locker(&m_mutex);
    return m_progress;
}

QString FunctionalTask::statusText() const {
    QMutexLocker locker(&m_mutex);
    return m_statusText;
}

QDateTime FunctionalTask::startTime() const {
    QMutexLocker locker(&m_mutex);
    return m_startTime;
}

QDateTime FunctionalTask::endTime() const {
    QMutexLocker locker(&m_mutex);
    return m_endTime;
}

void FunctionalTask::updateProgress(int percent, const QString &status) {
    QMutexLocker locker(&m_mutex);
    m_progress = percent;
    m_statusText = status;
}

void FunctionalTask::setState(TaskState state, const QString &finalStatus) {
    QMutexLocker locker(&m_mutex);
    m_state = state;
    if (!finalStatus.isEmpty()) {
        m_statusText = finalStatus;
    }
    if (state == TaskState::Completed || state == TaskState::Failed || state == TaskState::Cancelled) {
        m_endTime = QDateTime::currentDateTime();
    }
}

} // namespace GISApp::Core::Tasks
