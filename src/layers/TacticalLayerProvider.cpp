#include "TacticalLayerProvider.h"
#include "MapLibreLayerAdapter.h"
#include <QVariantMap>
#include <cmath>
#include <QDebug>

#include "publishing/LayerRegistryManager.h"
#include "publishing/LayerPublishingService.h"

namespace GISApp {
namespace Layers {

TacticalLayerProvider::TacticalLayerProvider(QObject *parent)
    : QObject(parent)
{
}

void TacticalLayerProvider::setupTacticalLayers(QMapLibre::Map *map)
{
    if (!map) return;

    // 1. Restricted Airspace Zone GeoJSON (Delhi/NCR Region)
    if (!map->sourceExists("air-zones-source")) {
        QVariantMap sourceParams;
        sourceParams["type"] = "geojson";
        sourceParams["data"] = QString(R"({
          "type": "FeatureCollection",
          "features": [
            {
              "type": "Feature",
              "geometry": {
                "type": "Polygon",
                "coordinates": [[
                  [76.85, 28.40],
                  [77.45, 28.40],
                  [77.45, 28.90],
                  [76.85, 28.90],
                  [76.85, 28.40]
                ]]
              },
              "properties": {}
            }
          ]
        })");
        map->addSource("air-zones-source", sourceParams);
    }

    if (!map->layerExists("air-zones-layer")) {
        QVariantMap layerParams;
        layerParams["id"] = "air-zones-layer";
        layerParams["type"] = "fill";
        layerParams["source"] = "air-zones-source";
        
        QVariantMap paintMap;
        paintMap["fill-color"] = "rgba(255, 60, 60, 0.4)";
        paintMap["fill-outline-color"] = "rgba(255, 0, 0, 0.9)";
        layerParams["paint"] = paintMap;

        map->addLayer("air-zones-layer", layerParams);
    }

    // 2. Primary Radar Coverage Circle GeoJSON (Northern India)
    if (!map->sourceExists("radar-coverage-source")) {
        QVariantMap radarSource;
        radarSource["type"] = "geojson";
        
        QString coordsStr;
        double centerLat = 28.6139;
        double centerLon = 77.2090;
        double radius = 4.5;
        for (int i = 0; i <= 36; ++i) {
            double angle = (i * 10.0) * 3.14159265358979323846 / 180.0;
            double lat = centerLat + radius * std::sin(angle);
            double lon = centerLon + radius * std::cos(angle) * 1.15;
            coordsStr += QString("[%1, %2]").arg(lon, 0, 'f', 4).arg(lat, 0, 'f', 4);
            if (i < 36) coordsStr += ", ";
        }

        radarSource["data"] = QString(R"({
          "type": "FeatureCollection",
          "features": [
            {
              "type": "Feature",
              "geometry": {
                "type": "Polygon",
                "coordinates": [[ %1 ]]
              },
              "properties": {}
            }
          ]
        })").arg(coordsStr);
        map->addSource("radar-coverage-source", radarSource);
    }

    if (!map->layerExists("radar-coverage")) {
        QVariantMap radarLayer;
        radarLayer["id"] = "radar-coverage";
        radarLayer["type"] = "fill";
        radarLayer["source"] = "radar-coverage-source";

        QVariantMap radarPaint;
        radarPaint["fill-color"] = "rgba(0, 230, 255, 0.25)";
        radarPaint["fill-outline-color"] = "rgba(0, 200, 255, 0.85)";
        radarLayer["paint"] = radarPaint;

        map->addLayer("radar-coverage", radarLayer);
    }
}

void TacticalLayerProvider::populateLayerTree(LayerManager *layerManager, QMapLibre::Map *map, GISApp::Controllers::MapController *mapController)
{
    if (!layerManager || !map) {
        qWarning() << "[TacticalLayerProvider] populateLayerTree failed: layerManager or map is null";
        return;
    }

    if (layerManager->model() && layerManager->model()->rootNode() && layerManager->model()->rootNode()->childCount() > 0) {
        qWarning() << "[TacticalLayerProvider] Layer tree already populated. Skipping duplicate population.";
        return;
    }
    qWarning() << "[TacticalLayerProvider] populateLayerTree executing...";

    auto rasterGroup = layerManager->addGroup("🗺️ Raster Imagery & DSM");
    auto tacticalGroup = layerManager->addGroup("🛡️ Tactical Operations");
    auto intelligenceGroup = layerManager->addGroup("📡 Signals & Sensors");

    LayerExtent indiaExtent{
        GISApp::Core::Models::GeoCoordinate(8.4, 68.7),
        GISApp::Core::Models::GeoCoordinate(37.6, 97.25)
    };

    LayerExtent ncrExtent{
        GISApp::Core::Models::GeoCoordinate(28.40, 76.85),
        GISApp::Core::Models::GeoCoordinate(28.88, 77.40)
    };

    auto airZoneAdapter = std::make_shared<MapLibreLayerAdapter>(
        "air-zones-layer", map, ncrExtent);
    layerManager->addLayer("Restricted Airspace Zones", airZoneAdapter, tacticalGroup);

    auto radarCoverageAdapter = std::make_shared<MapLibreLayerAdapter>(
        "radar-coverage", map, indiaExtent);
    layerManager->addLayer("Primary Radar Coverage", radarCoverageAdapter, intelligenceGroup);

    // Auto-restore custom published layers from persistent disk JSON registry
    GISApp::Publishing::LayerPublishingService publishingService;
    GISApp::Publishing::LayerRegistryManager::instance().restoreSavedLayers(layerManager, map, &publishingService);
}

} // namespace Layers
} // namespace GISApp
