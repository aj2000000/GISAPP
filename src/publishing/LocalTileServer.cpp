/**
 * @file LocalTileServer.cpp
 * @brief Dynamic XYZ Tile Streaming Server using Web Mercator tile cropping.
 */

#include "publishing/LocalTileServer.h"
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QDebug>
#include <cmath>

namespace GISApp::Publishing {

LocalTileServer& LocalTileServer::instance() {
    static LocalTileServer server;
    return server;
}

LocalTileServer::LocalTileServer(QObject *parent) : QTcpServer(parent) {}

bool LocalTileServer::startServer(quint16 port) {
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
    m_tileCache[layerId] = bytes;
}

QString LocalTileServer::getLayerUrl(const QString &layerId) const {
    return QString("http://127.0.0.1:%1/raster/%2.png").arg(m_port).arg(layerId);
}

void LocalTileServer::registerLayerFolder(const QString &layerId, const QString &folderPath) {
    m_layerFolders[layerId] = folderPath;

    // Automatically build VRT virtual mosaic catalog for all GeoTIFF tiles in folder
    QString vrtPath = QString("/tmp/mosaic_%1.vrt").arg(layerId);
    QDir dir(folderPath);
    QFileInfoList tifs = dir.entryInfoList(QStringList() << "*.tif" << "*.tiff", QDir::Files);

    if (!tifs.isEmpty()) {
        QStringList args;
        args << "-addalpha" << vrtPath;
        for (const auto &tif : tifs) {
            args << tif.absoluteFilePath();
        }
        int exitCode = QProcess::execute("gdalbuildvrt", args);
        if (exitCode == 0) {
            m_vrtPaths[layerId] = vrtPath;
            qWarning() << "[LocalTileServer] Successfully created VRT mosaic catalog:" << vrtPath << "for" << tifs.size() << "GeoTIFF tiles.";
        } else {
            qWarning() << "[LocalTileServer] Warning: gdalbuildvrt failed for layer" << layerId;
        }
    }
}

void LocalTileServer::registerLayerTilePath(const QString &layerId, const QString &tilePath) {
    m_tilePaths[layerId] = tilePath;
    qWarning() << "[LocalTileServer] Registered pre-tiled static disk store:" << tilePath << "for layer:" << layerId;
}

QString LocalTileServer::getTileUrlTemplate(const QString &layerId) const {
    return QString("http://127.0.0.1:%1/tiles/%2/{z}/{x}/{y}.png").arg(m_port).arg(layerId);
}

void LocalTileServer::registerVectorMbtiles(const QString &layerId, const QString &mbtilesPath) {
    m_vectorMbtilesPaths[layerId] = mbtilesPath;
    qWarning() << "[LocalTileServer] 📦 Registered Vector MBTiles store:" << mbtilesPath << "for layer:" << layerId;
}

QString LocalTileServer::getVectorUrlTemplate(const QString &layerId) const {
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

                QString mbtilesPath = m_vectorMbtilesPaths.value(layerId);
                if (mbtilesPath.isEmpty()) {
                    mbtilesPath = QString("/home/crl/aman/MAPDATA/mbtiles/%1.mbtiles").arg(layerId);
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

                qWarning().noquote() << QString("[LocalTileServer] 🌐 Incoming HTTP XYZ Tile Request -> Layer: %1 | Zoom z: %2 | Tile x: %3 | Tile y: %4")
                                        .arg(layerId).arg(z).arg(x).arg(y);

                QString cacheKey = QString("%1_%2_%3_%4").arg(layerId).arg(z).arg(x).arg(y);

                QByteArray body;
                if (m_tileCache.contains(cacheKey)) {
                    body = m_tileCache.value(cacheKey);
                } else {
                    // 1. Check static pre-tiled PNG store (/home/crl/aman/MAPDATA/{layerName}/{z}/{x}/{y}.png)
                    QString tileDir = m_tilePaths.value(layerId);
                    if (tileDir.isEmpty()) {
                        tileDir = QString("/home/crl/aman/MAPDATA/%1").arg(layerId);
                    }

                    QString staticPngFile = QString("%1/%2/%3/%4.png").arg(tileDir).arg(z).arg(x).arg(y);
                    if (QFile::exists(staticPngFile)) {
                        QFile file(staticPngFile);
                        if (file.open(QIODevice::ReadOnly)) {
                            body = file.readAll();
                            file.close();
                            m_tileCache[cacheKey] = body;
                        }
                    } else if (z > 14 && (m_vrtPaths.contains(layerId) || QFile::exists(QString("/tmp/mosaic_%1.vrt").arg(layerId)))) {
                        // Deep zoom (> 14): Dynamic crop from VRT catalog for high resolution details!
                        double west, north, east, south;
                        tileToLatLonBounds(z, x, y, west, north, east, south);

                        QString vrtPath = m_vrtPaths.value(layerId);
                        if (vrtPath.isEmpty() || !QFile::exists(vrtPath)) {
                            vrtPath = QString("/tmp/mosaic_%1.vrt").arg(layerId);
                        }

                        if (QFile::exists(vrtPath)) {
                            QString tempPng = QString("/tmp/tile_%1.png").arg(cacheKey);

                            QStringList args;
                            args << "-projwin" << QString::number(west, 'f', 6) << QString::number(north, 'f', 6)
                                 << QString::number(east, 'f', 6) << QString::number(south, 'f', 6)
                                 << "-outsize" << "256" << "256" << "-of" << "PNG"
                                 << vrtPath << tempPng;

                            QProcess::execute("gdal_translate", args);

                            QFile pngFile(tempPng);
                            if (pngFile.open(QIODevice::ReadOnly)) {
                                body = pngFile.readAll();
                                pngFile.close();
                                m_tileCache[cacheKey] = body;
                            }
                        }
                    } else if (m_tilePaths.contains(layerId) || QDir(tileDir).exists()) {
                        // Pre-tiled store exists: missing tile means it is outside raster bounds -> Instant transparent PNG!
                        body = getTransparentTilePNG();
                        m_tileCache[cacheKey] = body;
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

        QByteArray body = m_tileCache.value(layerId);
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
