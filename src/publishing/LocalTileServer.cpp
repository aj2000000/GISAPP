/**
 * @file LocalTileServer.cpp
 * @brief Dynamic XYZ Tile Streaming Server using Native C++ GDAL Dataset In-Memory Sampling & Bounded LRU Cache.
 */

#include "publishing/LocalTileServer.h"
#include "core/SystemConfigManager.h"
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QMutexLocker>
#include <QDebug>
#include <cmath>

namespace GISApp::Publishing {

LocalTileServer& LocalTileServer::instance() {
    static LocalTileServer server;
    return server;
}

LocalTileServer::LocalTileServer(QObject *parent) : QTcpServer(parent) {
    GDALAllRegister();
}

LocalTileServer::~LocalTileServer() {
    clearCache();
}

void LocalTileServer::clearCache() {
    QMutexLocker locker(&m_mutex);
    for (auto dsHandle : m_gdalDatasets) {
        if (dsHandle) {
            GDALClose(static_cast<GDALDataset*>(dsHandle));
        }
    }
    m_gdalDatasets.clear();
    m_tileCache.clear();
    m_cacheKeyOrder.clear();
}

void LocalTileServer::unregisterLayer(const QString &layerId) {
    QMutexLocker locker(&m_mutex);
    if (m_vrtPaths.contains(layerId)) {
        QString vrtPath = m_vrtPaths.take(layerId);
        if (m_gdalDatasets.contains(vrtPath)) {
            GDALDatasetH dsHandle = m_gdalDatasets.take(vrtPath);
            if (dsHandle) {
                GDALClose(static_cast<GDALDataset*>(dsHandle));
            }
        }
    }
    m_layerFolders.remove(layerId);
    m_tilePaths.remove(layerId);
    m_vectorMbtilesPaths.remove(layerId);

    // Evict cache entries starting with layerId
    QList<QString> keysToRemove;
    for (auto it = m_tileCache.keyBegin(); it != m_tileCache.keyEnd(); ++it) {
        if (it->startsWith(layerId)) {
            keysToRemove.append(*it);
        }
    }
    for (const auto &k : keysToRemove) {
        m_tileCache.remove(k);
        m_cacheKeyOrder.removeAll(k);
    }
}

void LocalTileServer::putTileCache(const QString &key, const QByteArray &data) {
    QMutexLocker locker(&m_mutex);
    if (m_tileCache.contains(key)) {
        m_tileCache[key] = data;
        return;
    }
    while (m_cacheKeyOrder.size() >= MAX_CACHE_ENTRIES) {
        QString oldestKey = m_cacheKeyOrder.dequeue();
        m_tileCache.remove(oldestKey);
    }
    m_tileCache[key] = data;
    m_cacheKeyOrder.enqueue(key);
}

QByteArray LocalTileServer::getTileCache(const QString &key) {
    QMutexLocker locker(&m_mutex);
    return m_tileCache.value(key);
}

bool LocalTileServer::startServer(quint16 port) {
    QMutexLocker locker(&m_mutex);
    if (isListening()) return true;
    m_port = port;
    bool ok = listen(QHostAddress::LocalHost, m_port);
    if (ok) {
        qWarning() << "[LocalTileServer] Embedded Raster XYZ Service running at http://127.0.0.1:" << m_port;
    }
    return ok;
}

void LocalTileServer::registerLayerTexture(const QString &layerId, const QImage &image) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    putTileCache(layerId, bytes);
}

QString LocalTileServer::getLayerUrl(const QString &layerId) const {
    QMutexLocker locker(&m_mutex);
    return QString("http://127.0.0.1:%1/raster/%2.png").arg(m_port).arg(layerId);
}

void LocalTileServer::registerLayerVrtPath(const QString &layerId, const QString &vrtPath) {
    QMutexLocker locker(&m_mutex);
    if (QFile::exists(vrtPath)) {
        m_vrtPaths[layerId] = vrtPath;
        qWarning() << "[LocalTileServer] 🗺️ Registered VRT catalog path:" << vrtPath << "for layer:" << layerId;
    }
}

void LocalTileServer::registerLayerFolder(const QString &layerId, const QString &folderPath) {
    {
        QMutexLocker locker(&m_mutex);
        m_layerFolders[layerId] = folderPath;

        if (m_vrtPaths.contains(layerId) && QFile::exists(m_vrtPaths[layerId])) {
            qWarning() << "[LocalTileServer] ⚡ Fast Startup: Using pre-registered VRT catalog at:" << m_vrtPaths[layerId];
            return;
        }

        // Check if VRT catalog already exists in MAPDATA directory or /tmp
        QString vrtPath = QString("%1/MAPDATA/%2.vrt").arg(QDir::homePath()).arg(layerId);
        if (!QFile::exists(vrtPath)) {
            vrtPath = QString("/tmp/mosaic_%1.vrt").arg(layerId);
        }

        if (QFile::exists(vrtPath)) {
            m_vrtPaths[layerId] = vrtPath;
            qWarning() << "[LocalTileServer] ⚡ Fast Startup: Reusing existing VRT catalog at:" << vrtPath;
            return;
        }
    }

    // Automatically build VRT virtual mosaic catalog for all GeoTIFF tiles in folder if missing
    QDir dir(folderPath);
    QFileInfoList tifs = dir.entryInfoList(QStringList() << "*.tif" << "*.tiff" << "*.TIF" << "*.TIFF", QDir::Files);

    if (!tifs.isEmpty()) {
        QString vrtPath = QString("%1/MAPDATA/%2.vrt").arg(QDir::homePath()).arg(layerId);
        QStringList args;
        args << "-addalpha" << vrtPath;
        for (const auto &tif : tifs) {
            args << tif.absoluteFilePath();
        }
        int exitCode = QProcess::execute("gdalbuildvrt", args);
        if (exitCode == 0) {
            QMutexLocker locker(&m_mutex);
            m_vrtPaths[layerId] = vrtPath;

            // Build internal multi-scale resolution overviews for instant sampling
            QStringList addoArgs;
            addoArgs << "-r" << "average" << vrtPath << "2" << "4" << "8" << "16" << "32" << "64";
            QProcess::execute("gdaladdo", addoArgs);

            qWarning() << "[LocalTileServer] Successfully created VRT mosaic catalog & overviews:" << vrtPath << "for" << tifs.size() << "GeoTIFF tiles.";
        } else {
            qWarning() << "[LocalTileServer] Warning: gdalbuildvrt failed for layer" << layerId;
        }
    }
}

void LocalTileServer::registerLayerTilePath(const QString &layerId, const QString &tilePath) {
    QMutexLocker locker(&m_mutex);
    m_tilePaths[layerId] = tilePath;
    qWarning() << "[LocalTileServer] Registered pre-tiled static disk store:" << tilePath << "for layer:" << layerId;
}

QString LocalTileServer::getTileUrlTemplate(const QString &layerId) const {
    QMutexLocker locker(&m_mutex);
    return QString("http://127.0.0.1:%1/tiles/%2/{z}/{x}/{y}.png").arg(m_port).arg(layerId);
}

void LocalTileServer::registerVectorMbtiles(const QString &layerId, const QString &mbtilesPath) {
    QMutexLocker locker(&m_mutex);
    m_vectorMbtilesPaths[layerId] = mbtilesPath;
    qWarning() << "[LocalTileServer] 📦 Registered Vector MBTiles store:" << mbtilesPath << "for layer:" << layerId;
}

QString LocalTileServer::getVectorUrlTemplate(const QString &layerId) const {
    QMutexLocker locker(&m_mutex);
    return QString("http://127.0.0.1:%1/vector/%2/{z}/{x}/{y}.pbf").arg(m_port).arg(layerId);
}

QByteArray LocalTileServer::getVectorTileData(const QString &mbtilesPath, int z, int x, int y) {
    if (!QFile::exists(mbtilesPath)) {
        qWarning() << "[LocalTileServer] Vector MBTiles file does not exist at:" << mbtilesPath;
        return QByteArray();
    }

    int tms_y = (1 << z) - 1 - y;
    QString connName = QString("vector_conn_%1_%2").arg(qHash(mbtilesPath)).arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    QByteArray tileData;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(mbtilesPath);
        if (db.open()) {
            QSqlQuery query(db);
            query.prepare("SELECT tile_data FROM tiles WHERE zoom_level = :z AND tile_column = :x AND tile_row = :y");
            query.bindValue(":z", z);
            query.bindValue(":x", x);
            query.bindValue(":y", tms_y);

            if (query.exec() && query.next()) {
                tileData = query.value(0).toByteArray();
            } else {
                qWarning() << "[LocalTileServer] Tile query empty/failed for layer" << mbtilesPath << "| z:" << z << "x:" << x << "y:" << y << "tms_y:" << tms_y << "| Error:" << query.lastError().text();
            }
            db.close();
        } else {
            qWarning() << "[LocalTileServer] Error opening SQLite database" << mbtilesPath << ":" << db.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(connName);
    return tileData;
}

// Convert XYZ Tile index to Geographic WGS84 Lat/Lon Bounding Box
static void tileToLatLonBounds(int z, int x, int y, double &west, double &north, double &east, double &south) {
    double n = std::pow(2.0, z);
    west = x / n * 360.0 - 180.0;
    east = (x + 1) / n * 360.0 - 180.0;

    double latRadNorth = std::atan(std::sinh(M_PI * (1.0 - 2.0 * y / n)));
    north = latRadNorth * 180.0 / M_PI;

    double latRadSouth = std::atan(std::sinh(M_PI * (1.0 - 2.0 * (y + 1) / n)));
    south = latRadSouth * 180.0 / M_PI;
}

static QByteArray getTransparentTilePNG() {
    static QByteArray transparentPngData;
    if (transparentPngData.isEmpty()) {
        QImage emptyImg(256, 256, QImage::Format_RGBA8888);
        emptyImg.fill(Qt::transparent);
        QBuffer buffer(&transparentPngData);
        buffer.open(QIODevice::WriteOnly);
        emptyImg.save(&buffer, "PNG");
    }
    return transparentPngData;
}

QByteArray LocalTileServer::renderNativeGdalTile(const QString &vrtPath, double west, double north, double east, double south) {
    if (!QFile::exists(vrtPath)) return getTransparentTilePNG();

    GDALDataset *ds = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        if (m_gdalDatasets.contains(vrtPath)) {
            ds = static_cast<GDALDataset*>(m_gdalDatasets.value(vrtPath));
        } else {
            ds = static_cast<GDALDataset*>(GDALOpenShared(vrtPath.toUtf8().constData(), GA_ReadOnly));
            if (ds) {
                m_gdalDatasets[vrtPath] = ds;
            }
        }
    }

    if (!ds) return getTransparentTilePNG();

    double gt[6];
    if (ds->GetGeoTransform(gt) != CE_None) {
        return getTransparentTilePNG();
    }

    double invGt[6];
    if (!GDALInvGeoTransform(gt, invGt)) {
        return getTransparentTilePNG();
    }

    int dsW = ds->GetRasterXSize();
    int dsH = ds->GetRasterYSize();

    // Dataset Geographic Bounds (WGS84 or Mercator)
    double dsMinX = gt[0];
    double dsMaxX = gt[0] + gt[1] * dsW;
    double dsMaxY = gt[3];
    double dsMinY = gt[3] + gt[5] * dsH; // gt[5] is negative

    if (dsMinX > dsMaxX) std::swap(dsMinX, dsMaxX);
    if (dsMinY > dsMaxY) std::swap(dsMinY, dsMaxY);

    // Geographic Intersection of Requested Tile [west, east, south, north] and Dataset [dsMinX, dsMaxX, dsMinY, dsMaxY]
    double interMinX = std::max(west, dsMinX);
    double interMaxX = std::min(east, dsMaxX);
    double interMinY = std::max(south, dsMinY);
    double interMaxY = std::min(north, dsMaxY);

    if (interMinX >= interMaxX || interMinY >= interMaxY) {
        // Tile does not intersect dataset at all!
        return getTransparentTilePNG();
    }

    // 1. Compute Dataset Pixel Coordinates for the Intersection Window
    double startXDouble, startYDouble, endXDouble, endYDouble;
    GDALApplyGeoTransform(invGt, interMinX, interMaxY, &startXDouble, &startYDouble);
    GDALApplyGeoTransform(invGt, interMaxX, interMinY, &endXDouble, &endYDouble);

    int srcX = std::clamp(static_cast<int>(std::floor(startXDouble)), 0, dsW - 1);
    int srcY = std::clamp(static_cast<int>(std::floor(startYDouble)), 0, dsH - 1);
    int endX = std::clamp(static_cast<int>(std::ceil(endXDouble)), 1, dsW);
    int endY = std::clamp(static_cast<int>(std::ceil(endYDouble)), 1, dsH);

    int srcW = std::max(1, endX - srcX);
    int srcH = std::max(1, endY - srcY);

    // 2. Compute Output Sub-pixel Rectangle inside the 256x256 Output Tile
    double tileW = east - west;
    double tileH = north - south; // positive

    int dstX = std::clamp(static_cast<int>(std::floor(256.0 * (interMinX - west) / tileW)), 0, 255);
    int dstY = std::clamp(static_cast<int>(std::floor(256.0 * (north - interMaxY) / tileH)), 0, 255);
    int dstXEnd = std::clamp(static_cast<int>(std::ceil(256.0 * (interMaxX - west) / tileW)), dstX + 1, 256);
    int dstYEnd = std::clamp(static_cast<int>(std::ceil(256.0 * (north - interMinY) / tileH)), dstY + 1, 256);

    int dstW = dstXEnd - dstX;
    int dstH = dstYEnd - dstY;

    if (dstW <= 0 || dstH <= 0) {
        return getTransparentTilePNG();
    }

    int bandCount = ds->GetRasterCount();
    if (bandCount < 1) return getTransparentTilePNG();

    // 3. Read dataset sub-window into temp buffer of size dstW x dstH
    QByteArray tempBuffer(dstW * dstH * 4, 0);
    GByte *pTemp = reinterpret_cast<GByte*>(tempBuffer.data());

    int bandMap[4] = {1, 2, 3, 4};
    int readBands = std::min(bandCount, 4);

    CPLErr err = ds->RasterIO(GF_Read, srcX, srcY, srcW, srcH,
                              pTemp, dstW, dstH, GDT_Byte,
                              readBands, bandMap,
                              4, dstW * 4, 1);

    if (err != CE_None) {
        return getTransparentTilePNG();
    }

    // 4. Create transparent 256x256 QImage and place tempBuffer at (dstX, dstY)
    QImage img(256, 256, QImage::Format_RGBA8888);
    img.fill(Qt::transparent);

    for (int subY = 0; subY < dstH; ++subY) {
        int targetY = dstY + subY;
        if (targetY >= 256) break;

        uchar *line = img.scanLine(targetY);
        for (int subX = 0; subX < dstW; ++subX) {
            int targetX = dstX + subX;
            if (targetX >= 256) break;

            int idx = (subY * dstW + subX) * 4;
            uchar r = pTemp[idx];
            uchar g = (readBands >= 2) ? pTemp[idx + 1] : r;
            uchar b = (readBands >= 3) ? pTemp[idx + 2] : r;
            uchar a = (readBands >= 4) ? pTemp[idx + 3] : 255;

            uchar *pixel = line + (targetX * 4);
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = a;
        }
    }

    QByteArray pngBytes;
    QBuffer buf(&pngBytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return pngBytes;
}

void LocalTileServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, [this, socket]() {
        QByteArray request = socket->readAll();
        QString reqStr = QString::fromUtf8(request);

        // Pattern 0: Vector PBF Tile GET /vector/{layerId}/{z}/{x}/{y}.pbf
        if (reqStr.contains("GET /vector/")) {
            int start = reqStr.indexOf("/vector/") + 8;
            int end = reqStr.indexOf(" ", start);
            QString path = reqStr.mid(start, end - start);
            QStringList parts = path.split('/', Qt::SkipEmptyParts);

            if (parts.size() >= 4) {
                QString layerId = parts[0];
                int z = parts[1].toInt();
                int x = parts[2].toInt();
                int y = parts[3].section('.', 0, 0).toInt();

                QString mbtilesPath;
                {
                    QMutexLocker locker(&m_mutex);
                    mbtilesPath = m_vectorMbtilesPaths.value(layerId);
                }
                if (mbtilesPath.isEmpty()) {
                    mbtilesPath = QString("%1/VectorTiles/%2.mbtiles").arg(GISApp::Core::SystemConfigManager::instance().getMapDataDir()).arg(layerId);
                }

                QByteArray pbfData = getVectorTileData(mbtilesPath, z, x, y);
                if (!pbfData.isEmpty()) {
                    QByteArray res;
                    res.append("HTTP/1.1 200 OK\r\nContent-Type: application/x-protobuf\r\n");
                    res.append("Content-Encoding: gzip\r\n");
                    res.append("Content-Length: " + QByteArray::number(pbfData.size()) + "\r\n");
                    res.append("Access-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n");
                    res.append(pbfData);
                    socket->write(res);
                    socket->disconnectFromHost();
                    return;
                } else {
                    QByteArray response;
                    response.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
                    socket->write(response);
                    socket->disconnectFromHost();
                    return;
                }
            }
        }

        // Pattern 1: Dynamic XYZ Tile GET /tiles/{layerId}/{z}/{x}/{y}.png
        if (reqStr.contains("GET /tiles/")) {
            int start = reqStr.indexOf("/tiles/") + 7;
            int end = reqStr.indexOf(" ", start);
            QString path = reqStr.mid(start, end - start);
            QStringList parts = path.split('/', Qt::SkipEmptyParts);

            if (parts.size() >= 4) {
                QString layerId = parts[0];
                int z = parts[1].toInt();
                int x = parts[2].toInt();
                int y = parts[3].section('.', 0, 0).toInt();

                QString cacheKey = QString("%1_%2_%3_%4").arg(layerId).arg(z).arg(x).arg(y);

                QByteArray body = getTileCache(cacheKey);
                if (body.isEmpty()) {
                    // 1. Check static pre-tiled PNG store ($HOME/MAPDATA/{layerName}/{z}/{x}/{y}.png)
                    QString tileDir;
                    {
                        QMutexLocker locker(&m_mutex);
                        tileDir = m_tilePaths.value(layerId);
                    }
                    if (tileDir.isEmpty()) {
                        tileDir = QString("%1/%2").arg(GISApp::Core::SystemConfigManager::instance().getMapDataDir()).arg(layerId);
                    }

                    QString staticPngFile = QString("%1/%2/%3/%4.png").arg(tileDir).arg(z).arg(x).arg(y);
                    if (QFile::exists(staticPngFile)) {
                        QFile file(staticPngFile);
                        if (file.open(QIODevice::ReadOnly)) {
                            body = file.readAll();
                            file.close();
                            putTileCache(cacheKey, body);
                        }
                    } else {
                        // 2. Native C++ GDAL In-Memory Tile Rendering
                        double west, north, east, south;
                        tileToLatLonBounds(z, x, y, west, north, east, south);

                        QString vrtPath;
                        bool hasTilePath = false;
                        {
                            QMutexLocker locker(&m_mutex);
                            vrtPath = m_vrtPaths.value(layerId);
                            hasTilePath = m_tilePaths.contains(layerId);
                        }
                        if (vrtPath.isEmpty() || !QFile::exists(vrtPath)) {
                            vrtPath = QString("%1/%2.vrt").arg(GISApp::Core::SystemConfigManager::instance().getMapDataDir()).arg(layerId);
                        }
                        if (!QFile::exists(vrtPath)) {
                            vrtPath = QString("/tmp/mosaic_%1.vrt").arg(layerId);
                        }

                        if (QFile::exists(vrtPath)) {
                            body = renderNativeGdalTile(vrtPath, west, north, east, south);
                            if (!body.isEmpty()) {
                                putTileCache(cacheKey, body);
                            }
                        } else if (hasTilePath || QDir(tileDir).exists()) {
                            body = getTransparentTilePNG();
                            putTileCache(cacheKey, body);
                        }
                    }
                }

                if (!body.isEmpty()) {
                    QByteArray res;
                    res.append("HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n");
                    res.append("Content-Length: " + QByteArray::number(body.size()) + "\r\n");
                    res.append("Access-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n");
                    res.append(body);
                    socket->write(res);
                    socket->disconnectFromHost();
                    return;
                }
            }
        }

        // Fallback Pattern 2: Single Static Texture GET /raster/{layerId}.png
        QString layerId;
        if (reqStr.contains("GET /raster/")) {
            int start = reqStr.indexOf("/raster/") + 8;
            int end = reqStr.indexOf(".png", start);
            if (end > start) {
                layerId = reqStr.mid(start, end - start);
            }
        }

        QByteArray body = getTileCache(layerId);
        QByteArray response;

        if (!body.isEmpty()) {
            response.append("HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n");
            response.append("Content-Length: " + QByteArray::number(body.size()) + "\r\n");
            response.append("Access-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n");
            response.append(body);
        } else {
            response.append("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        }

        socket->write(response);
        socket->disconnectFromHost();
    });

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

} // namespace GISApp::Publishing
