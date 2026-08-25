#include "publishing/VectorLayerPublisher.h"
#include "publishing/LocalTileServer.h"
#include "layers/MapLibreLayerAdapter.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QVariantMap>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QCoreApplication>
#include <QSvgRenderer>
#include <QPainter>
#include <QImage>
#include <QDebug>

namespace GISApp::Publishing {

bool VectorLayerPublisher::publish(const QString &folderPath,
                                   const QString &layerName,
                                   GISApp::Layers::LayerGroupNode *targetGroup,
                                   GISApp::Layers::LayerManager *layerManager,
                                   QMapLibre::Map *map,
                                   ProgressCallback progressCb)
{
    if (!map || !layerManager) {
        m_statusMessage = "Engine map or layer manager is invalid.";
        return false;
    }

    QFileInfo inputInfo(folderPath);
    QString targetFilePath;
    QDir datasetDir = inputInfo.isDir() ? QDir(folderPath) : inputInfo.dir();

    if (inputInfo.isFile()) {
        targetFilePath = inputInfo.absoluteFilePath();
    } else if (inputInfo.isDir()) {
        QFileInfoList vectorFiles = datasetDir.entryInfoList({"*.geojson", "*.json", "*.shp"}, QDir::Files);
        if (!vectorFiles.isEmpty()) {
            targetFilePath = vectorFiles.first().absoluteFilePath();
        }
    }

    if (targetFilePath.isEmpty()) {
        m_statusMessage = "No valid vector source file (.geojson, .json, or .shp) found.";
        return false;
    }

    // --- Check for SVG, SLD, and QML Companion Files ---
    QFileInfoList svgFiles = datasetDir.entryInfoList({"*.svg", "*.SVG"}, QDir::Files);
    QFileInfoList sldFiles = datasetDir.entryInfoList({"*.sld", "*.SLD"}, QDir::Files);
    QFileInfoList qmlFiles = datasetDir.entryInfoList({"*.qml", "*.QML"}, QDir::Files);

    bool hasCustomSymbolStyle = (!svgFiles.isEmpty() && !sldFiles.isEmpty() && !qmlFiles.isEmpty());
    qWarning() << "[VectorPublisher] Dataset directory:" << datasetDir.absolutePath() 
               << "| SVG count:" << svgFiles.size() 
               << "| SLD count:" << sldFiles.size() 
               << "| QML count:" << qmlFiles.size() 
               << "| Custom SVG/SLD/QML Styling Active:" << hasCustomSymbolStyle;

    QString sanitizedId = "vector-" + QString::number(qHash(layerName)) + "-" + QString::number(QDateTime::currentMSecsSinceEpoch());

    QDir mapDataDir("/home/crl/aman/MAPDATA/mbtiles");
    if (!mapDataDir.exists()) {
        mapDataDir.mkpath(".");
    }

    QString mbtilesPath = QString("/home/crl/aman/MAPDATA/mbtiles/%1.mbtiles").arg(sanitizedId);

    // Smart max zoom based on file size: global datasets use maxzoom=8 to prevent infinite tile generation
    qint64 fileSize = QFileInfo(targetFilePath).size();
    int maxZoom = (fileSize > 500000) ? 8 : 10;

    // ⚡ Fast Restoration Check: Skip conversion if MBTiles already pre-built!
    if (QFile::exists(mbtilesPath)) {
        qWarning() << "[VectorPublisher] ⚡ Fast Restoration: Pre-existing Vector MBTiles store found at" << mbtilesPath << ". Skipping conversion completely!";
    } else {
        if (progressCb) progressCb(10, "Converting vector features to MBTiles vector tile store...");

        QStringList ogrArgs;
        ogrArgs << "-f" << "MBTILES" << mbtilesPath << targetFilePath
                << "-dsco" << "MINZOOM=0" << "-dsco" << QString("MAXZOOM=%1").arg(maxZoom);

        QProcess process;
        process.start("ogr2ogr", ogrArgs);

        int pct = 15;
        while (!process.waitForFinished(100)) {
            if (pct < 65) {
                pct += 1;
                if (progressCb) progressCb(pct, "Generating optimized MBTiles vector pyramids...");
            }
            QCoreApplication::processEvents();
        }

        if (process.exitCode() != 0 || !QFile::exists(mbtilesPath)) {
            qWarning() << "[VectorPublisher] Warning: ogr2ogr MBTiles conversion failed. Exit code:" << process.exitCode();
            m_statusMessage = "Failed to convert vector file to MBTiles format.";
            return false;
        }
        qWarning() << "[VectorPublisher] Successfully converted vector file to MBTiles at:" << mbtilesPath;
    }

    // Register MBTiles store with LocalTileServer
    LocalTileServer::instance().registerVectorMbtiles(sanitizedId, mbtilesPath);

    // Extract Metadata & Bounds from SQLite MBTiles database
    QString vectorLayerName = QFileInfo(targetFilePath).completeBaseName();
    GISApp::Layers::LayerExtent vectorExtent{
        GISApp::Core::Models::GeoCoordinate(8.4, 68.7),
        GISApp::Core::Models::GeoCoordinate(37.6, 97.25)
    };

    QString connName = QString("meta_conn_%1").arg(qHash(mbtilesPath));
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(mbtilesPath);
        if (db.open()) {
            QSqlQuery q(db);
            q.exec("SELECT name, value FROM metadata");
            while (q.next()) {
                QString key = q.value(0).toString();
                QString val = q.value(1).toString();
                if (key == "json" && !val.isEmpty()) {
                    // Extract source-layer id from vector_layers JSON array
                    int idx = val.indexOf("\"id\":\"");
                    if (idx != -1) {
                        int endIdx = val.indexOf("\"", idx + 6);
                        if (endIdx != -1) {
                            vectorLayerName = val.mid(idx + 6, endIdx - (idx + 6));
                        }
                    }
                } else if (key == "bounds") {
                    QStringList b = val.split(',');
                    if (b.size() == 4) {
                        double minLon = b[0].toDouble();
                        double minLat = b[1].toDouble();
                        double maxLon = b[2].toDouble();
                        double maxLat = b[3].toDouble();
                        vectorExtent = GISApp::Layers::LayerExtent{
                            GISApp::Core::Models::GeoCoordinate(minLat, minLon),
                            GISApp::Core::Models::GeoCoordinate(maxLat, maxLon)
                        };
                    }
                }
            }
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(connName);
    qWarning() << "[VectorPublisher] Target vector source-layer name:" << vectorLayerName;

    if (progressCb) progressCb(70, "Registering Vector Tile service with MapLibre graphics engine...");

    // Register Vector Source in MapLibre Engine
    QVariantMap sourceParams;
    sourceParams["type"] = "vector";
    sourceParams["tiles"] = QVariantList{ LocalTileServer::instance().getVectorUrlTemplate(sanitizedId) };
    sourceParams["minzoom"] = 0;
    sourceParams["maxzoom"] = maxZoom;

    if (!map->sourceExists(sanitizedId + "-src")) {
        map->addSource(sanitizedId + "-src", sourceParams);
    }

    QVariantMap layerParams;
    QVariantMap strokeParams;

    if (hasCustomSymbolStyle) {
        // --- 🌿 CUSTOM SVG + SLD + QML SYMBOL STYLING (e.g. Bamboo) ---
        QString svgPath = svgFiles.first().absoluteFilePath();
        QString iconId = "icon-" + QString::number(qHash(svgPath));

        QImage iconImage(32, 32, QImage::Format_ARGB32);
        iconImage.fill(Qt::transparent);

        QSvgRenderer svgRenderer(svgPath);
        if (svgRenderer.isValid()) {
            QPainter painter(&iconImage);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            svgRenderer.render(&painter);
            qWarning() << "[VectorPublisher] Successfully rendered custom SVG symbol icon from:" << svgPath;
        } else {
            qWarning() << "[VectorPublisher] Warning: QSvgRenderer failed to parse SVG file:" << svgPath;
        }

        map->addImage(iconId, iconImage);

        // Render Symbol Layer with SVG Icon ONLY (no extra point/circle plot)
        layerParams["id"] = sanitizedId;
        layerParams["type"] = "symbol";
        layerParams["source"] = sanitizedId + "-src";
        layerParams["source-layer"] = vectorLayerName;

        QVariantMap layoutMap;
        layoutMap["icon-image"] = iconId;
        layoutMap["icon-size"] = 0.5;
        layoutMap["icon-allow-overlap"] = true;
        layoutMap["icon-ignore-placement"] = true;
        layerParams["layout"] = layoutMap;

        QVariantMap paintMap;
        paintMap["icon-opacity"] = 1.0;
        layerParams["paint"] = paintMap;

        if (!map->layerExists(sanitizedId)) {
            map->addLayer(sanitizedId, layerParams);
        }
    } else {
        // --- 📍 STANDARD VECTOR / FALLBACK SIMPLE POINT / FILL PLOT ---
        layerParams["id"] = sanitizedId;
        layerParams["type"] = "fill";
        layerParams["source"] = sanitizedId + "-src";
        layerParams["source-layer"] = vectorLayerName;

        QVariantMap paintMap;
        paintMap["fill-color"] = "#10b981";
        paintMap["fill-opacity"] = 0.55;
        paintMap["fill-outline-color"] = "#047857";
        layerParams["paint"] = paintMap;

        if (!map->layerExists(sanitizedId)) {
            map->addLayer(sanitizedId, layerParams);
        }

        // Add outline stroke sub-layer for crisp boundary rendering
        QString strokeLayerId = sanitizedId + "-stroke";
        strokeParams["id"] = strokeLayerId;
        strokeParams["type"] = "line";
        strokeParams["source"] = sanitizedId + "-src";
        strokeParams["source-layer"] = vectorLayerName;

        QVariantMap linePaint;
        linePaint["line-color"] = "#059669";
        linePaint["line-width"] = 2.0;
        strokeParams["paint"] = linePaint;

        if (!map->layerExists(strokeLayerId)) {
            map->addLayer(strokeLayerId, strokeParams);
        }
    }

    auto adapter = std::make_shared<GISApp::Layers::MapLibreLayerAdapter>(sanitizedId, map, vectorExtent, layerParams, strokeParams);
    layerManager->addLayer(layerName, adapter, targetGroup);

    if (progressCb) progressCb(100, "Vector MBTiles layer successfully published!");
    m_statusMessage = QString("Successfully published vector MBTiles dataset: %1.").arg(layerName);
    return true;
}

} // namespace GISApp::Publishing
