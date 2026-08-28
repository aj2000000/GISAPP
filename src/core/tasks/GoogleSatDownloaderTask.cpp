/**
 * @file GoogleSatDownloaderTask.cpp
 * @brief Asynchronous Google Earth Satellite Tile Downloader and GeoTIFF Stitcher.
 */

#include "core/tasks/GoogleSatDownloaderTask.h"
#include "core/tasks/BackgroundTaskManager.h"
#include "core/notifications/NotificationManager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QDebug>
#include <cmath>
#include <vector>
#include <algorithm>

#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <cpl_string.h>

namespace GISApp::Core::Tasks {

static int lonToTileX(double lon, int zoom) {
    double n = std::pow(2.0, zoom);
    int x = static_cast<int>(std::floor((lon + 180.0) / 360.0 * n));
    return std::clamp(x, 0, static_cast<int>(n) - 1);
}

static int latToTileY(double lat, int zoom) {
    double latRad = lat * M_PI / 180.0;
    double n = std::pow(2.0, zoom);
    int y = static_cast<int>(std::floor((1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * n));
    return std::clamp(y, 0, static_cast<int>(n) - 1);
}

static double tileXToLon(int x, int zoom) {
    return x / std::pow(2.0, zoom) * 360.0 - 180.0;
}

static double tileYToLat(int y, int zoom) {
    double n = M_PI - 2.0 * M_PI * y / std::pow(2.0, zoom);
    return 180.0 / M_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));
}

GoogleSatDownloaderTask::GoogleSatDownloaderTask(const DownloaderParams &params)
    : m_params(params),
      m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_name(QString("Download Google Sat (%1 to %2)").arg(params.minZoom).arg(params.maxZoom)),
      m_startTime(QDateTime::currentDateTime())
{
}

void GoogleSatDownloaderTask::cancel() {
    m_cancelRequested = true;
    QMutexLocker locker(&m_mutex);
    if (m_state == TaskState::Queued || m_state == TaskState::Running) {
        m_state = TaskState::Cancelled;
        m_endTime = QDateTime::currentDateTime();
    }
}

QString GoogleSatDownloaderTask::id() const {
    QMutexLocker locker(&m_mutex);
    return m_id;
}

QString GoogleSatDownloaderTask::name() const {
    QMutexLocker locker(&m_mutex);
    return m_name;
}

TaskState GoogleSatDownloaderTask::state() const {
    QMutexLocker locker(&m_mutex);
    return m_state;
}

int GoogleSatDownloaderTask::progress() const {
    QMutexLocker locker(&m_mutex);
    return m_progress;
}

QString GoogleSatDownloaderTask::statusText() const {
    QMutexLocker locker(&m_mutex);
    return m_statusText;
}

QDateTime GoogleSatDownloaderTask::startTime() const {
    QMutexLocker locker(&m_mutex);
    return m_startTime;
}

QDateTime GoogleSatDownloaderTask::endTime() const {
    QMutexLocker locker(&m_mutex);
    return m_endTime;
}

void GoogleSatDownloaderTask::calculateTileEstimate(const DownloaderParams &params, int &totalTiles, double &estimatedSizeMb)
{
    totalTiles = 0;
    for (int z = params.minZoom; z <= params.maxZoom; ++z) {
        int minX = lonToTileX(params.minLon, z);
        int maxX = lonToTileX(params.maxLon, z);
        int minY = latToTileY(params.maxLat, z);
        int maxY = latToTileY(params.minLat, z);

        if (minX > maxX) std::swap(minX, maxX);
        if (minY > maxY) std::swap(minY, maxY);

        int tilesInZoom = (maxX - minX + 1) * (maxY - minY + 1);
        totalTiles += tilesInZoom;
    }

    // Average JPEG tile size ~25 KB
    estimatedSizeMb = (totalTiles * 25.0) / 1024.0;
}

QString GoogleSatDownloaderTask::startDownload(const DownloaderParams &params)
{
    auto task = std::make_shared<GoogleSatDownloaderTask>(params);
    return BackgroundTaskManager::instance().submitTaskCommand(task);
}

struct TileWorkItem {
    int x;
    int y;
    int destX;
    int destY;
    int retries{0};
};

static void writeTileToGdal(GDALDataset *dataset, int destX, int destY, const QImage &tileImg) {
    if (!dataset || tileImg.isNull()) return;

    int tw = tileImg.width();
    int th = tileImg.height();

    QByteArray rBuf(tw * th, 0);
    QByteArray gBuf(tw * th, 0);
    QByteArray bBuf(tw * th, 0);

    char *rPtr = rBuf.data();
    char *gPtr = gBuf.data();
    char *bPtr = bBuf.data();

    for (int py = 0; py < th; ++py) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(tileImg.constScanLine(py));
        int offset = py * tw;
        for (int px = 0; px < tw; ++px) {
            QRgb pixel = scanline[px];
            rPtr[offset + px] = static_cast<char>(qRed(pixel));
            gPtr[offset + px] = static_cast<char>(qGreen(pixel));
            bPtr[offset + px] = static_cast<char>(qBlue(pixel));
        }
    }

    dataset->GetRasterBand(1)->RasterIO(GF_Write, destX, destY, tw, th, rBuf.data(), tw, th, GDT_Byte, 0, 0);
    dataset->GetRasterBand(2)->RasterIO(GF_Write, destX, destY, tw, th, gBuf.data(), tw, th, GDT_Byte, 0, 0);
    dataset->GetRasterBand(3)->RasterIO(GF_Write, destX, destY, tw, th, bBuf.data(), tw, th, GDT_Byte, 0, 0);
}

bool GoogleSatDownloaderTask::execute(ProgressCallback progressCb)
{
    {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Running;
    }

    int targetZoom = m_params.maxZoom;
    int minX = lonToTileX(m_params.minLon, targetZoom);
    int maxX = lonToTileX(m_params.maxLon, targetZoom);
    int minY = latToTileY(m_params.maxLat, targetZoom);
    int maxY = latToTileY(m_params.minLat, targetZoom);

    if (minX > maxX) std::swap(minX, maxX);
    if (minY > maxY) std::swap(minY, maxY);

    int cols = maxX - minX + 1;
    int rows = maxY - minY + 1;
    int totalTiles = cols * rows;

    if (totalTiles <= 0) {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Failed;
        m_statusText = "Invalid bounding box or tile count.";
        m_endTime = QDateTime::currentDateTime();
        return false;
    }

    int imgWidth = cols * 256;
    int imgHeight = rows * 256;

    // Ensure output directory exists
    QFileInfo fileInfo(m_params.outputPath);
    QDir().mkpath(fileInfo.absolutePath());

    GDALAllRegister();
    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver) {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Failed;
        m_statusText = "GDAL GTiff driver unavailable.";
        m_endTime = QDateTime::currentDateTime();
        return false;
    }

    char **options = nullptr;
    options = CSLSetNameValue(options, "COMPRESS", "JPEG");
    options = CSLSetNameValue(options, "TILED", "YES");

    GDALDataset *dataset = driver->Create(m_params.outputPath.toUtf8().constData(),
                                         imgWidth, imgHeight, 3, GDT_Byte, options);
    CSLDestroy(options);

    if (!dataset) {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Failed;
        m_statusText = "Failed to create output GeoTIFF file.";
        m_endTime = QDateTime::currentDateTime();
        return false;
    }

    double westLon = tileXToLon(minX, targetZoom);
    double eastLon = tileXToLon(maxX + 1, targetZoom);
    double northLat = tileYToLat(minY, targetZoom);
    double southLat = tileYToLat(maxY + 1, targetZoom);

    double pixelWidth = (eastLon - westLon) / imgWidth;
    double pixelHeight = (northLat - southLat) / imgHeight;

    double geoTransform[6] = { westLon, pixelWidth, 0.0, northLat, 0.0, -pixelHeight };
    dataset->SetGeoTransform(geoTransform);

    OGRSpatialReference srs;
    srs.importFromEPSG(4326);
    char *wkt = nullptr;
    srs.exportToWkt(&wkt);
    dataset->SetProjection(wkt);
    CPLFree(wkt);

    // Prepare tile work items
    std::vector<TileWorkItem> workQueue;
    workQueue.reserve(totalTiles);
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            TileWorkItem item;
            item.x = x;
            item.y = y;
            item.destX = (x - minX) * 256;
            item.destY = (y - minY) * 256;
            workQueue.push_back(item);
        }
    }

    QNetworkAccessManager nam;
    int downloadedCount = 0;
    int validTilesCount = 0;

    // Batch downloading up to 6 tiles concurrently
    const size_t BATCH_SIZE = 6;
    size_t queueIndex = 0;

    while (queueIndex < workQueue.size()) {
        if (m_cancelRequested) {
            GDALClose(dataset);
            QMutexLocker locker(&m_mutex);
            m_state = TaskState::Cancelled;
            m_endTime = QDateTime::currentDateTime();
            return false;
        }

        size_t currentBatchCount = std::min(BATCH_SIZE, workQueue.size() - queueIndex);
        QEventLoop loop;
        int finishedInBatch = 0;

        for (size_t i = 0; i < currentBatchCount; ++i) {
            TileWorkItem &item = workQueue[queueIndex + i];
            int serverNum = (item.x + item.y) % 4;
            QString tileUrl = QString("https://mt%1.google.com/vt/lyrs=y&x=%2&y=%3&z=%4")
                                  .arg(serverNum).arg(item.x).arg(item.y).arg(targetZoom);

            QNetworkRequest request((QUrl(tileUrl)));
            request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
            request.setRawHeader("Referer", "https://maps.google.com/");

            QNetworkReply *reply = nam.get(request);

            QTimer *timer = new QTimer(reply);
            timer->setSingleShot(true);

            QObject::connect(timer, &QTimer::timeout, reply, [reply]() {
                if (reply->isRunning()) {
                    reply->abort();
                }
            });

            QObject::connect(reply, &QNetworkReply::finished, [reply, timer, &item, dataset, &finishedInBatch, &validTilesCount, &loop, currentBatchCount]() {
                timer->stop();
                if (reply->error() == QNetworkReply::NoError) {
                    QImage tileImg;
                    if (tileImg.loadFromData(reply->readAll())) {
                        writeTileToGdal(dataset, item.destX, item.destY, tileImg);
                        validTilesCount++;
                    }
                } else if (item.retries < 2) {
                    item.retries++;
                    qWarning() << "[GoogleSatDownloader] Retrying tile x:" << item.x << "y:" << item.y << "Attempt:" << item.retries;
                }
                reply->deleteLater();
                finishedInBatch++;
                if (static_cast<size_t>(finishedInBatch) >= currentBatchCount) {
                    loop.quit();
                }
            });

            timer->start(8000); // 8 second timeout per tile
        }

        loop.exec();

        downloadedCount += currentBatchCount;
        queueIndex += currentBatchCount;

        int percent = (downloadedCount * 95) / totalTiles;
        QString statusMsg = QString("Streaming tiles to disk: %1 of %2 (Zoom %3)")
                                .arg(downloadedCount).arg(totalTiles).arg(targetZoom);

        {
            QMutexLocker locker(&m_mutex);
            m_progress = percent;
            m_statusText = statusMsg;
        }
        if (progressCb) {
            progressCb(percent, statusMsg);
        }

        if (downloadedCount % 30 == 0) {
            dataset->FlushCache();
        }
    }

    if (validTilesCount == 0) {
        GDALClose(dataset);
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Failed;
        m_statusText = "Download failed: No satellite tiles could be fetched. Check internet connection.";
        m_endTime = QDateTime::currentDateTime();
        return false;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_progress = 98;
        m_statusText = "Finalizing GeoTIFF dataset & writing metadata...";
    }
    if (progressCb) {
        progressCb(98, "Finalizing GeoTIFF dataset & writing metadata...");
    }

    dataset->FlushCache();
    GDALClose(dataset);

    QString finalMsg = QString("Successfully saved GeoTIFF to %1").arg(m_params.outputPath);
    {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Completed;
        m_progress = 100;
        m_statusText = finalMsg;
        m_endTime = QDateTime::currentDateTime();
    }
    if (progressCb) {
        progressCb(100, finalMsg);
    }
    return true;
}

void GoogleSatDownloaderTask::notifyCompletion(bool success)
{
    if (success) {
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Download Complete",
            QString("Satellite imagery downloaded successfully to '%1'.").arg(m_params.outputPath)
        );
    } else {
        GISApp::Core::Notifications::NotificationManager::instance()->notifyCritical(
            "Download Failed",
            QString("Failed to download satellite imagery: %1").arg(statusText())
        );
    }
}

} // namespace GISApp::Core::Tasks
