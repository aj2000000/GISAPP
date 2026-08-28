/**
 * @file GoogleSatDownloaderTask.cpp
 * @brief Asynchronous Google Earth Satellite Tile Downloader and GeoTIFF Stitcher.
 */

#include "core/tasks/GoogleSatDownloaderTask.h"
#include "core/tasks/BackgroundTaskManager.h"
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

    QImage stitchedImage(imgWidth, imgHeight, QImage::Format_RGB32);
    stitchedImage.fill(Qt::black);

    QNetworkAccessManager nam;
    int downloadedCount = 0;
    int validTilesCount = 0;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (m_cancelRequested) {
                QMutexLocker locker(&m_mutex);
                m_state = TaskState::Cancelled;
                m_endTime = QDateTime::currentDateTime();
                return false;
            }

            int serverNum = (x + y) % 4;
            QString tileUrl = QString("https://mt%1.google.com/vt/lyrs=y&x=%2&y=%3&z=%4")
                                  .arg(serverNum).arg(x).arg(y).arg(targetZoom);

            QNetworkRequest request((QUrl(tileUrl)));
            request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36");

            QEventLoop loop;
            QTimer timer;
            timer.setSingleShot(true);
            
            QNetworkReply *reply = nam.get(request);
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
                if (reply->isRunning()) {
                    reply->abort();
                }
                loop.quit();
            });

            timer.start(10000); // 10 second network timeout per tile
            loop.exec();

            if (reply->error() == QNetworkReply::NoError) {
                QImage tileImg;
                if (tileImg.loadFromData(reply->readAll())) {
                    int destX = (x - minX) * 256;
                    int destY = (y - minY) * 256;
                    QPainter painter(&stitchedImage);
                    painter.drawImage(destX, destY, tileImg);
                    validTilesCount++;
                }
            }
            reply->deleteLater();

            downloadedCount++;
            int percent = (downloadedCount * 90) / totalTiles;
            QString statusMsg = QString("Downloaded tile %1 of %2 (Zoom %3)")
                                    .arg(downloadedCount).arg(totalTiles).arg(targetZoom);

            {
                QMutexLocker locker(&m_mutex);
                m_progress = percent;
                m_statusText = statusMsg;
            }
            if (progressCb) {
                progressCb(percent, statusMsg);
            }
        }
    }

    if (validTilesCount == 0) {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Failed;
        m_statusText = "Download failed: No satellite tiles could be fetched. Check internet connection.";
        m_endTime = QDateTime::currentDateTime();
        return false;
    }

    if (progressCb) {
        progressCb(92, "Writing GeoTIFF raster dataset...");
    }

    // Ensure target directory exists
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
    if (!dataset) {
        CSLDestroy(options);
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

    QByteArray rLine(imgWidth, 0);
    QByteArray gLine(imgWidth, 0);
    QByteArray bLine(imgWidth, 0);

    for (int y = 0; y < imgHeight; ++y) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(stitchedImage.constScanLine(y));
        char *rPtr = rLine.data();
        char *gPtr = gLine.data();
        char *bPtr = bLine.data();

        for (int x = 0; x < imgWidth; ++x) {
            QRgb pixel = scanline[x];
            rPtr[x] = static_cast<char>(qRed(pixel));
            gPtr[x] = static_cast<char>(qGreen(pixel));
            bPtr[x] = static_cast<char>(qBlue(pixel));
        }

        dataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y, imgWidth, 1, rLine.data(), imgWidth, 1, GDT_Byte, 0, 0);
        dataset->GetRasterBand(2)->RasterIO(GF_Write, 0, y, imgWidth, 1, gLine.data(), imgWidth, 1, GDT_Byte, 0, 0);
        dataset->GetRasterBand(3)->RasterIO(GF_Write, 0, y, imgWidth, 1, bLine.data(), imgWidth, 1, GDT_Byte, 0, 0);
    }

    GDALClose(dataset);
    CSLDestroy(options);

    {
        QMutexLocker locker(&m_mutex);
        m_state = TaskState::Completed;
        m_progress = 100;
        m_statusText = QString("Successfully saved GeoTIFF to %1").arg(m_params.outputPath);
        m_endTime = QDateTime::currentDateTime();
    }
    return true;
}

} // namespace GISApp::Core::Tasks
