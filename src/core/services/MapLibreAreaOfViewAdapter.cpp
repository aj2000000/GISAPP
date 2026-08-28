#include "MapLibreAreaOfViewAdapter.h"
#include "SystemConfigManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QVariantMap>
#include <QDebug>

namespace GISApp::Core::Services {

MapLibreAreaOfViewAdapter::MapLibreAreaOfViewAdapter(QMapLibre::Map *map, Repositories::IAreaOfViewRepository *repository, QObject *parent)
    : QObject(parent), m_map(map), m_repository(repository)
{
    if (m_repository) {
        connect(m_repository, &Repositories::IAreaOfViewRepository::areaOfViewUpdated,
                this, &MapLibreAreaOfViewAdapter::refreshFromRepository);
    }
    refreshFromRepository();
}

void MapLibreAreaOfViewAdapter::setMap(QMapLibre::Map *map)
{
    m_map = map;
    m_layersCreated = false;
    refreshFromRepository();
}

void MapLibreAreaOfViewAdapter::ensureLayersCreated()
{
    if (!m_map) {
        return;
    }

    if (m_map->sourceExists("area-of-view-geojson-source") &&
        m_map->layerExists("area-of-view-fill-layer") &&
        m_map->layerExists("area-of-view-fill-layer-stroke")) {
        m_layersCreated = true;
        return;
    }

    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/area_of_view.geojson";

    if (!QFile::exists(geojsonPath)) {
        QFile file(geojsonPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(R"({"type":"FeatureCollection","features":[]})");
            file.close();
        }
    }

    // 1. Add Source
    if (!m_map->sourceExists("area-of-view-geojson-source")) {
        QVariantMap sourceParams;
        sourceParams["type"] = "geojson";
        sourceParams["data"] = QString("file://%1").arg(geojsonPath);
        m_map->addSource("area-of-view-geojson-source", sourceParams);
    }

    // 2. Add Polygon Fill Layer (no fill color)
    if (!m_map->layerExists("area-of-view-fill-layer")) {
        QVariantMap fillParams;
        fillParams["id"] = "area-of-view-fill-layer";
        fillParams["type"] = "fill";
        fillParams["source"] = "area-of-view-geojson-source";

        QVariantMap fillPaint;
        fillPaint["fill-color"] = "rgba(0, 0, 0, 0)";
        fillPaint["fill-opacity"] = 0.0; // Completely transparent fill
        fillParams["paint"] = fillPaint;
        m_map->addLayer("area-of-view-fill-layer", fillParams);
    }

    // 3. Add Polygon Line Border Layer (brown color)
    if (!m_map->layerExists("area-of-view-fill-layer-stroke")) {
        QVariantMap lineParams;
        lineParams["id"] = "area-of-view-fill-layer-stroke";
        lineParams["type"] = "line";
        lineParams["source"] = "area-of-view-geojson-source";

        QVariantMap linePaint;
        linePaint["line-color"] = "#8B4513"; // Brown color
        linePaint["line-width"] = 3.5;
        lineParams["paint"] = linePaint;
        m_map->addLayer("area-of-view-fill-layer-stroke", lineParams);
    }

    m_layersCreated = true;
    qDebug() << "[MapLibreAreaOfViewAdapter] Area of View fill & stroke layers successfully registered in MapLibre engine (Brown outline, no fill).";
}

void MapLibreAreaOfViewAdapter::refreshFromRepository()
{
    if (!m_map) {
        return;
    }

    ensureLayersCreated();

    QJsonArray features;
    if (m_repository) {
        auto records = m_repository->getAll();
        for (const auto &rec : records) {
            features.append(rec.toGeoJsonFeature());
        }
    }

    QJsonObject featureCollection;
    featureCollection["type"] = "FeatureCollection";
    featureCollection["features"] = features;

    QJsonDocument doc(featureCollection);
    QString rawGeoJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/area_of_view.geojson";

    QFile file(geojsonPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(rawGeoJson.toUtf8());
        file.close();
    }

    if (m_map) {
        m_layersCreated = false;
        if (m_map->layerExists("area-of-view-fill-layer-stroke")) {
            m_map->removeLayer("area-of-view-fill-layer-stroke");
        }
        if (m_map->layerExists("area-of-view-fill-layer")) {
            m_map->removeLayer("area-of-view-fill-layer");
        }
        if (m_map->sourceExists("area-of-view-geojson-source")) {
            m_map->removeSource("area-of-view-geojson-source");
        }
        ensureLayersCreated();
        m_map->triggerRepaint();
        qDebug() << "[MapLibreAreaOfViewAdapter] Real-time updated MapLibre Area of View source with" << features.size() << "polygons.";
    }
}

} // namespace GISApp::Core::Services
