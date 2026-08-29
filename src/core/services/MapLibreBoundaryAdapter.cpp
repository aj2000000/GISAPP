#include "MapLibreBoundaryAdapter.h"
#include "SystemConfigManager.h"
#include "database/DatabaseManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QVariantMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace GISApp::Core::Services {

static QString serializeBoundaryPoints(const QVector<Models::Coordinate3D> &points)
{
    QJsonArray arr;
    for (const auto &pt : points) {
        QJsonObject obj;
        obj["lat"] = pt.latitude;
        obj["lon"] = pt.longitude;
        obj["alt"] = pt.altitude;
        arr.append(obj);
    }
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

static QVector<Models::Coordinate3D> deserializeBoundaryPoints(const QString &jsonStr)
{
    QVector<Models::Coordinate3D> points;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const auto &val : arr) {
            if (val.isObject()) {
                QJsonObject obj = val.toObject();
                double lat = obj.value("lat").toDouble();
                double lon = obj.value("lon").toDouble();
                double alt = obj.value("alt").toDouble();
                points.append(Models::Coordinate3D(lat, lon, alt));
            }
        }
    }
    return points;
}

MapLibreBoundaryAdapter::MapLibreBoundaryAdapter(QMapLibre::Map *map, QObject *parent)
    : QObject(parent), m_map(map)
{
}

void MapLibreBoundaryAdapter::setMap(QMapLibre::Map *map)
{
    m_map = map;
    m_layersCreated = false;
    if (!m_boundaries.isEmpty()) {
        setBoundaries(m_boundaries);
    }
}

void MapLibreBoundaryAdapter::saveBoundariesToDatabase(const QVector<Models::BoundaryRecord> &boundaries)
{
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) return;

    QSqlQuery clearQuery("DELETE FROM BOUNDARIES", db);
    clearQuery.exec();

    if (boundaries.isEmpty()) return;

    if (!db.transaction()) {
        qWarning() << "[MapLibreBoundaryAdapter] Failed to start SQLite transaction for boundaries";
    }

    QSqlQuery insertQuery(db);
    insertQuery.prepare(R"(
        INSERT INTO BOUNDARIES (BOUNDARY_ID, NAME, N_POINTS, POINTS_JSON)
        VALUES (:boundary_id, :name, :n_points, :points_json)
    )");

    for (const auto &rec : boundaries) {
        insertQuery.bindValue(":boundary_id", rec.boundaryId);
        insertQuery.bindValue(":name", rec.name);
        insertQuery.bindValue(":n_points", rec.points.size());
        insertQuery.bindValue(":points_json", serializeBoundaryPoints(rec.points));
        if (!insertQuery.exec()) {
            qCritical() << "[MapLibreBoundaryAdapter] SQLite Insert Error:" << insertQuery.lastError().text();
            db.rollback();
            return;
        }
    }

    if (!db.commit()) {
        qWarning() << "[MapLibreBoundaryAdapter] Failed to commit SQLite transaction for boundaries";
    }
    qDebug() << "[MapLibreBoundaryAdapter] Successfully saved" << boundaries.size() << "boundary records to SQLite database.";
}

QVector<Models::BoundaryRecord> MapLibreBoundaryAdapter::loadBoundariesFromDatabase()
{
    QVector<Models::BoundaryRecord> result;
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) return result;

    QSqlQuery query("SELECT BOUNDARY_ID, NAME, POINTS_JSON FROM BOUNDARIES", db);
    while (query.next()) {
        Models::BoundaryRecord rec;
        rec.boundaryId = query.value("BOUNDARY_ID").toInt();
        rec.name = query.value("NAME").toString();
        if (rec.name.isEmpty()) rec.name = "Boundary";
        rec.points = deserializeBoundaryPoints(query.value("POINTS_JSON").toString());
        if (!rec.points.isEmpty()) {
            result.append(rec);
        }
    }
    qDebug() << "[MapLibreBoundaryAdapter] Loaded" << result.size() << "boundary records from SQLite database.";
    return result;
}

QVector<Models::BoundaryRecord> MapLibreBoundaryAdapter::loadSavedBoundaries()
{
    // First try SQLite database
    QVector<Models::BoundaryRecord> dbBoundaries = loadBoundariesFromDatabase();
    if (!dbBoundaries.isEmpty()) {
        m_boundaries = dbBoundaries;
        return m_boundaries;
    }

    // Fallback to disk GeoJSON file if DB table is empty
    QVector<Models::BoundaryRecord> result;
    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QString geojsonPath = mapDataDir + "/boundary.geojson";

    if (!QFile::exists(geojsonPath)) {
        return result;
    }

    QFile file(geojsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return result;

    QJsonObject rootObj = doc.object();
    QJsonArray features = rootObj.value("features").toArray();

    for (const auto &val : features) {
        if (!val.isObject()) continue;
        QJsonObject feature = val.toObject();
        QJsonObject props = feature.value("properties").toObject();
        QJsonObject geom = feature.value("geometry").toObject();

        Models::BoundaryRecord rec;
        rec.boundaryId = props.value("id").toInt();
        rec.name = props.value("name").toString("Boundary");

        QJsonArray coordsArray = geom.value("coordinates").toArray();
        for (const auto &cVal : coordsArray) {
            QJsonArray ptArray = cVal.toArray();
            if (ptArray.size() >= 2) {
                double lon = ptArray.at(0).toDouble();
                double lat = ptArray.at(1).toDouble();
                double alt = ptArray.size() >= 3 ? ptArray.at(2).toDouble() : 0.0;
                rec.points.append(Models::Coordinate3D(lat, lon, alt));
            }
        }
        if (!rec.points.isEmpty()) {
            result.append(rec);
        }
    }
    m_boundaries = result;
    if (!m_boundaries.isEmpty()) {
        saveBoundariesToDatabase(m_boundaries);
    }
    qDebug() << "[MapLibreBoundaryAdapter] Loaded" << result.size() << "saved boundary records from disk GeoJSON fallback.";
    return result;
}

void MapLibreBoundaryAdapter::ensureLayersCreated(const QByteArray &geoJsonData)
{
    if (!m_map) return;

    QByteArray dataToUse = geoJsonData;
    if (dataToUse.isEmpty()) {
        QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
        QDir().mkpath(mapDataDir);
        QString geojsonPath = mapDataDir + "/boundary.geojson";
        if (QFile::exists(geojsonPath)) {
            QFile file(geojsonPath);
            if (file.open(QIODevice::ReadOnly)) {
                dataToUse = file.readAll();
                file.close();
            }
        }
    }
    if (dataToUse.isEmpty()) {
        dataToUse = R"({"type":"FeatureCollection","features":[]})";
    }

    QVariantMap sourceParams;
    sourceParams["type"] = "geojson";
    sourceParams["data"] = dataToUse;

    // 1. Add / Update Source
    if (m_map->sourceExists("boundary-geojson-source")) {
        m_map->updateSource("boundary-geojson-source", sourceParams);
    } else {
        m_map->addSource("boundary-geojson-source", sourceParams);
    }

    // 2. Add Outer Green Line Layer (Width 6.0px)
    if (!m_map->layerExists("boundary-outer-line-layer")) {
        QVariantMap outerParams;
        outerParams["id"] = "boundary-outer-line-layer";
        outerParams["type"] = "line";
        outerParams["source"] = "boundary-geojson-source";

        QVariantMap outerPaint;
        outerPaint["line-color"] = "#16A34A"; // Vibrant Green Outer Stroke
        outerPaint["line-width"] = 6.0;
        outerPaint["line-opacity"] = 0.95;
        outerParams["paint"] = outerPaint;

        QVariantMap outerLayout;
        outerLayout["line-join"] = "round";
        outerLayout["line-cap"] = "round";
        outerParams["layout"] = outerLayout;

        m_map->addLayer("boundary-outer-line-layer", outerParams);
    }

    // 3. Add Inner Saffron Line Layer (Width 3.0px)
    if (!m_map->layerExists("boundary-inner-line-layer")) {
        QVariantMap innerParams;
        innerParams["id"] = "boundary-inner-line-layer";
        innerParams["type"] = "line";
        innerParams["source"] = "boundary-geojson-source";

        QVariantMap innerPaint;
        innerPaint["line-color"] = "#FF7700"; // Saffron Inner Stroke
        innerPaint["line-width"] = 3.0;
        innerPaint["line-opacity"] = 1.0;
        innerParams["paint"] = innerPaint;

        QVariantMap innerLayout;
        innerLayout["line-join"] = "round";
        innerLayout["line-cap"] = "round";
        innerParams["layout"] = innerLayout;

        m_map->addLayer("boundary-inner-line-layer", innerParams);
    }

    m_layersCreated = true;
    qDebug() << "[MapLibreBoundaryAdapter] Boundary Outer Green & Inner Saffron stroke layers registered successfully.";
}

void MapLibreBoundaryAdapter::setBoundaries(const QVector<Models::BoundaryRecord> &boundaries)
{
    m_boundaries = boundaries;
    saveBoundariesToDatabase(m_boundaries);
    if (!m_map) return;

    QJsonArray features;
    for (const auto &b : m_boundaries) {
        features.append(b.toGeoJsonFeature());
    }

    QJsonObject featureCollection;
    featureCollection["type"] = "FeatureCollection";
    featureCollection["features"] = features;

    QJsonDocument doc(featureCollection);
    QByteArray rawGeoJsonBytes = doc.toJson(QJsonDocument::Compact);

    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/boundary.geojson";

    QFile file(geojsonPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(rawGeoJsonBytes);
        file.close();
    }

    if (m_map) {
        ensureLayersCreated(rawGeoJsonBytes);
        m_map->triggerRepaint();
        qDebug() << "[MapLibreBoundaryAdapter] Real-time updated MapLibre Boundary source with" << m_boundaries.size() << "polylines.";
    }
}

} // namespace GISApp::Core::Services
