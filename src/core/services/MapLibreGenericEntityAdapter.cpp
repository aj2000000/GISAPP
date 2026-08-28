/**
 * @file MapLibreGenericEntityAdapter.cpp
 * @brief Implementation of MapLibreGenericEntityAdapter.
 */

#include "MapLibreGenericEntityAdapter.h"
#include "../models/GisEntityRegistry.h"
#include "SystemConfigManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QDebug>

namespace GISApp::Core::Services {

using namespace GISApp::Core::Models;

MapLibreGenericEntityAdapter::MapLibreGenericEntityAdapter(
    QMapLibre::Map *map,
    IGisEntityRepository *repository,
    QObject *parent)
    : QObject(parent)
    , m_map(map)
    , m_repository(repository)
{
    if (m_repository) {
        connect(m_repository, &IGisEntityRepository::entityAdded,
                this, &MapLibreGenericEntityAdapter::onEntityAdded);
        connect(m_repository, &IGisEntityRepository::entityUpdated,
                this, &MapLibreGenericEntityAdapter::onEntityUpdated);
        connect(m_repository, &IGisEntityRepository::entityRemoved,
                this, &MapLibreGenericEntityAdapter::onEntityRemoved);
        connect(m_repository, &IGisEntityRepository::repositoryCleared,
                this, &MapLibreGenericEntityAdapter::refreshFromRepository);
    }
}

void MapLibreGenericEntityAdapter::setMap(QMapLibre::Map *map)
{
    m_map = map;
    m_layersCreated = false;
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::setLayerManager(Layers::LayerManager *layerManager)
{
    m_layerManager = layerManager;
}

void MapLibreGenericEntityAdapter::ensureMapLayersCreated()
{
    if (!m_map || m_layersCreated) return;

    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/generic_entities.geojson";

    if (!QFile::exists(geojsonPath)) {
        QFile file(geojsonPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(R"({"type":"FeatureCollection","features":[]})");
            file.close();
        }
    }

    // 1. Add GeoJSON Source
    if (!m_map->sourceExists(m_sourceId)) {
        QVariantMap sourceParams;
        sourceParams["type"] = "geojson";
        sourceParams["data"] = QString("file://%1").arg(geojsonPath);
        m_map->addSource(m_sourceId, sourceParams);
    }

    // 2. Add Polygon Fill Layer
    if (!m_map->layerExists(m_fillLayerId)) {
        QVariantMap fillLayer;
        fillLayer["id"] = m_fillLayerId;
        fillLayer["type"] = "fill";
        fillLayer["source"] = m_sourceId;

        QVariantMap filter;
        filter["0"] = "==";
        filter["1"] = "$type";
        filter["2"] = "Polygon";
        fillLayer["filter"] = filter;

        QVariantMap paint;
        paint["fill-color"] = "rgba(16, 185, 129, 0.4)";
        paint["fill-outline-color"] = "#10b981";
        fillLayer["paint"] = paint;

        m_map->addLayer(m_fillLayerId, fillLayer);
    }

    // 3. Add Polyline Layer
    if (!m_map->layerExists(m_lineLayerId)) {
        QVariantMap lineLayer;
        lineLayer["id"] = m_lineLayerId;
        lineLayer["type"] = "line";
        lineLayer["source"] = m_sourceId;

        QVariantMap filter;
        filter["0"] = "==";
        filter["1"] = "$type";
        filter["2"] = "LineString";
        lineLayer["filter"] = filter;

        QVariantMap paint;
        paint["line-color"] = "#38bdf8";
        paint["line-width"] = 2.5;
        lineLayer["paint"] = paint;

        m_map->addLayer(m_lineLayerId, lineLayer);
    }

    // 4. Add Point/Circle Symbol Layer
    if (!m_map->layerExists(m_circleLayerId)) {
        QVariantMap circleLayer;
        circleLayer["id"] = m_circleLayerId;
        circleLayer["type"] = "circle";
        circleLayer["source"] = m_sourceId;

        QVariantMap filter;
        filter["0"] = "==";
        filter["1"] = "$type";
        filter["2"] = "Point";
        circleLayer["filter"] = filter;

        QVariantMap paint;
        paint["circle-color"] = "rgba(56, 189, 248, 0.95)";
        paint["circle-radius"] = 8.0;
        paint["circle-stroke-color"] = "#ffffff";
        paint["circle-stroke-width"] = 2.0;
        circleLayer["paint"] = paint;

        m_map->addLayer(m_circleLayerId, circleLayer);
    }

    m_layersCreated = true;
}

void MapLibreGenericEntityAdapter::renderEntity(std::shared_ptr<Models::IGisEntity> entity)
{
    Q_UNUSED(entity);
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::renderEntities(const QVector<std::shared_ptr<Models::IGisEntity>> &entities)
{
    Q_UNUSED(entities);
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::removeEntity(const QString &entityId)
{
    Q_UNUSED(entityId);
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::clearEntities()
{
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::onEntityAdded(std::shared_ptr<GenericGisEntity> entity)
{
    Q_UNUSED(entity);
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::onEntityUpdated(std::shared_ptr<GenericGisEntity> entity)
{
    Q_UNUSED(entity);
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::onEntityRemoved(const QString &entityId)
{
    Q_UNUSED(entityId);
    refreshFromRepository();
}

void MapLibreGenericEntityAdapter::refreshFromRepository()
{
    if (!m_map || !m_repository) return;

    auto entities = m_repository->findAll();
    QJsonArray features;

    for (const auto &entity : entities) {
        if (!entity) continue;
        features.append(entity->toGeoJsonFeature());
    }

    QJsonObject featureCollection;
    featureCollection["type"] = "FeatureCollection";
    featureCollection["features"] = features;

    QString rawGeoJson = QString::fromUtf8(QJsonDocument(featureCollection).toJson(QJsonDocument::Compact));

    // Write file on disk
    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QDir().mkpath(mapDataDir);
    QString geojsonPath = mapDataDir + "/generic_entities.geojson";

    QFile file(geojsonPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(rawGeoJson.toUtf8());
        file.close();
    }

    m_layersCreated = false;
    if (m_map->layerExists(m_circleLayerId)) m_map->removeLayer(m_circleLayerId);
    if (m_map->layerExists(m_lineLayerId)) m_map->removeLayer(m_lineLayerId);
    if (m_map->layerExists(m_fillLayerId)) m_map->removeLayer(m_fillLayerId);
    if (m_map->sourceExists(m_sourceId)) m_map->removeSource(m_sourceId);

    ensureMapLayersCreated();
    m_map->triggerRepaint();

    qDebug() << "[MapLibreGenericEntityAdapter] Refreshed generic GeoJSON map source with" << features.size() << "entities.";
}

} // namespace GISApp::Core::Services
