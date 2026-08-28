#include "MapLibreTrackAdapter.h"
#include "SystemConfigManager.h"
#include "../../layers/LayerManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <cmath>

namespace GISApp::Core::Services {

MapLibreTrackAdapter::MapLibreTrackAdapter(QMapLibre::Map *map, Repositories::ITrackRepository *repository, QObject *parent)
    : QObject(parent), m_map(map), m_repository(repository)
{
    if (m_repository) {
        connect(m_repository, &Repositories::ITrackRepository::tracksUpdated,
                this, &MapLibreTrackAdapter::refreshFromRepository);
    }
    refreshFromRepository();
}

void MapLibreTrackAdapter::setMap(QMapLibre::Map *map)
{
    m_map = map;
    m_layersCreated = false;
    refreshFromRepository();
}

void MapLibreTrackAdapter::setLayerManager(Layers::LayerManager *layerManager)
{
    m_layerManager = layerManager;
}

void MapLibreTrackAdapter::ensureLayersCreated()
{
    if (!m_map) {
        return;
    }

    if (m_map->sourceExists("tracks-geojson-source") && m_map->layerExists("tracks-circle-layer")) {
        m_layersCreated = true;
        return;
    }

    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/tactical_tracks.geojson";

    if (!QFile::exists(geojsonPath)) {
        QFile file(geojsonPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(R"({"type":"FeatureCollection","features":[]})");
            file.close();
        }
    }

    if (!m_map->sourceExists("tracks-geojson-source")) {
        QVariantMap sourceParams;
        sourceParams["type"] = "geojson";
        sourceParams["data"] = QString("file://%1").arg(geojsonPath);
        m_map->addSource("tracks-geojson-source", sourceParams);
    }

    if (!m_map->layerExists("tracks-circle-layer")) {
        QVariantMap layerParams;
        layerParams["id"] = "tracks-circle-layer";
        layerParams["type"] = "circle";
        layerParams["source"] = "tracks-geojson-source";

        QVariantMap paintMap;
        paintMap["circle-color"] = "rgba(0, 255, 255, 0.95)";
        paintMap["circle-radius"] = 9.0;
        paintMap["circle-stroke-color"] = "#ffffff";
        paintMap["circle-stroke-width"] = 2.5;
        layerParams["paint"] = paintMap;

        m_map->addLayer("tracks-circle-layer", layerParams);
    }

    m_layersCreated = true;
}



void MapLibreTrackAdapter::refreshFromRepository()
{
    if (!m_map || !m_repository) {
        return;
    }

    ensureLayersCreated();

    QVector<Models::TrackRecord> tracks = m_repository->getAllTracks();

    if (!tracks.isEmpty()) {
        double minLat = 90.0, maxLat = -90.0;
        double minLon = 180.0, maxLon = -180.0;
        for (const auto &track : tracks) {
            if (track.trackLat < minLat) minLat = track.trackLat;
            if (track.trackLat > maxLat) maxLat = track.trackLat;
            if (track.trackLong < minLon) minLon = track.trackLong;
            if (track.trackLong > maxLon) maxLon = track.trackLong;
        }

        if (std::abs(maxLat - minLat) < 0.0001) {
            minLat -= 0.005;
            maxLat += 0.005;
        }
        if (std::abs(maxLon - minLon) < 0.0001) {
            minLon -= 0.005;
            maxLon += 0.005;
        }

        Layers::LayerExtent trackExtent{
            Core::Models::GeoCoordinate(minLat, minLon),
            Core::Models::GeoCoordinate(maxLat, maxLon)
        };

        if (m_layerManager) {
            auto trackNode = m_layerManager->findLayerByLayerId("tracks-circle-layer");
            if (trackNode && trackNode->adapter()) {
                trackNode->adapter()->setExtent(trackExtent);
            }
        }
    }

    QJsonArray features;
    for (const auto &track : tracks) {
        features.append(track.toGeoJsonFeature());
    }

    QJsonObject featureCollection;
    featureCollection["type"] = "FeatureCollection";
    featureCollection["features"] = features;

    QJsonDocument doc(featureCollection);
    QString rawGeoJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    // Save GeoJSON to file on disk for persistent offline backup
    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/tactical_tracks.geojson";

    QFile file(geojsonPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(rawGeoJson.toUtf8());
        file.close();
    }

    if (m_map) {
        m_layersCreated = false;
        if (m_map->layerExists("tracks-circle-layer")) {
            m_map->removeLayer("tracks-circle-layer");
        }
        if (m_map->sourceExists("tracks-geojson-source")) {
            m_map->removeSource("tracks-geojson-source");
        }
        ensureLayersCreated();
        m_map->triggerRepaint();
        qDebug() << "[MapLibreTrackAdapter] Real-time updated MapLibre GeoJSON layer with" << tracks.size() << "tracks.";
    }
}


void MapLibreTrackAdapter::renderEntity(std::shared_ptr<Models::IGisEntity> entity)
{
    Q_UNUSED(entity);
    refreshFromRepository();
}

void MapLibreTrackAdapter::renderEntities(const QVector<std::shared_ptr<Models::IGisEntity>> &entities)
{
    Q_UNUSED(entities);
    refreshFromRepository();
}

void MapLibreTrackAdapter::removeEntity(const QString &entityId)
{
    Q_UNUSED(entityId);
    refreshFromRepository();
}

void MapLibreTrackAdapter::clearEntities()
{
    refreshFromRepository();
}

} // namespace GISApp::Core::Services
