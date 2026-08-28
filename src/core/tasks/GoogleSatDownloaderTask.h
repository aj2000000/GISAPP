/**
 * @file GoogleSatDownloaderTask.h
 * @brief Asynchronous Google Earth Satellite Tile Downloader and GeoTIFF Stitcher.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef GOOGLESATDOWNLOADERTASK_H
#define GOOGLESATDOWNLOADERTASK_H

#include "core/tasks/IBackgroundTask.h"
#include <QString>
#include <QMutex>
#include <atomic>

namespace GISApp::Core::Tasks {

struct DownloaderParams {
    double minLat{0.0};
    double maxLat{0.0};
    double minLon{0.0};
    double maxLon{0.0};
    int minZoom{1};
    int maxZoom{15};
    QString outputPath;
};

/**
 * @class GoogleSatDownloaderTask
 * @brief Concrete Command implementing IBackgroundTask for downloading satellite imagery and stitching GeoTIFFs.
 */
class GoogleSatDownloaderTask : public IBackgroundTask {
public:
    explicit GoogleSatDownloaderTask(const DownloaderParams &params);
    ~GoogleSatDownloaderTask() override = default;

    bool execute(ProgressCallback progressCb) override;
    void cancel() override;

    QString id() const override;
    QString name() const override;
    TaskState state() const override;
    int progress() const override;
    QString statusText() const override;
    QDateTime startTime() const override;
    QDateTime endTime() const override;
    void notifyCompletion(bool success) override;

    /**
     * @brief Convenience static method launching background download task.
     * @param params Downloader parameters (bounding box, zoom level, output path).
     * @return Unique background task ID.
     */
    static QString startDownload(const DownloaderParams &params);

    /**
     * @brief Calculates total tile count and estimated file size in megabytes.
     */
    static void calculateTileEstimate(const DownloaderParams &params, int &totalTiles, double &estimatedSizeMb);

private:
    DownloaderParams m_params;
    QString m_id;
    QString m_name;
    TaskState m_state{TaskState::Queued};
    int m_progress{0};
    QString m_statusText;
    QDateTime m_startTime;
    QDateTime m_endTime;
    std::atomic<bool> m_cancelRequested{false};
    mutable QMutex m_mutex;
};

} // namespace GISApp::Core::Tasks

#endif // GOOGLESATDOWNLOADERTASK_H
