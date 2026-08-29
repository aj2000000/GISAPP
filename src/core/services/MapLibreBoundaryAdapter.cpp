#include "MapLibreBoundaryAdapter.h"
#include "SystemConfigManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QVariantMap>
#include <QDebug>

namespace GISApp::Core::Services {

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

QVector<Models::BoundaryRecord> MapLibreBoundaryAdapter::loadSavedBoundaries()
{
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
    qDebug() << "[MapLibreBoundaryAdapter] Loaded" << result.size() << "saved boundary records from disk.";
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

    // 1. Add / Update Source
    if (m_map->sourceExists("boundary-geojson-source")) {
        m_map->setProperty("boundary-geojson-source", "data", QString::fromUtf8(dataToUse));
    } else {
        QVariantMap sourceParams;
        sourceParams["type"] = "geojson";
        sourceParams["data"] = dataToUse;
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
