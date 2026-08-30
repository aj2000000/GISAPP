/**
 * @file MapLibreLayerAdapter.cpp
 * @brief Implementation of MapLibre Native Layer Adapter with custom layer filtering.
 */

#include "layers/MapLibreLayerAdapter.h"
#include "publishing/UdlRepositoryManager.h"
#include "SystemConfigManager.h"
#include <QVariantMap>
#include <QFile>
#include <QDebug>
#include <algorithm>


namespace GISApp::Layers {

MapLibreLayerAdapter::MapLibreLayerAdapter(const QString &layerId, 
                                           QMapLibre::Map *mapPointer, 
                                           const LayerExtent &defaultExtent,
                                           const QVariantMap &layerParams,
                                           const QVariantMap &strokeParams,
                                           const QVariantMap &sourceParams,
                                           const QString &rawUdlLayerId)
    : m_layerId(layerId), 
      m_map(mapPointer), 
      m_extent(defaultExtent),
      m_layerParams(layerParams),
      m_strokeParams(strokeParams),
      m_sourceParams(sourceParams),
      m_rawUdlLayerId(rawUdlLayerId)
{
}

QString MapLibreLayerAdapter::layerId() const {
    return m_layerId;
}

void MapLibreLayerAdapter::setVisibility(bool visible) {
    m_visible = visible;
    if (!m_map) return;

    // Demand-driven / Lazy initialization for remote online layers (e.g. Google Earth Satellite)
    if (visible && !m_sourceParams.isEmpty()) {
        QString sourceId = m_layerParams.value("source").toString();
        if (sourceId.isEmpty()) sourceId = m_layerId + "-src";

        if (!m_map->sourceExists(sourceId)) {
            qWarning() << "[LayerAdapter] 🌐 Lazy-initializing remote source & layer on-demand for:" << m_layerId;
            m_map->addSource(sourceId, m_sourceParams);
            if (!m_layerParams.isEmpty() && !m_map->layerExists(m_layerId)) {
                m_map->addLayer(m_layerId, m_layerParams);
            }
        }
    }

    QString visString = visible ? QString("visible") : QString("none");

    if (m_layerId.startsWith("udl-")) {
        qWarning() << "[LayerAdapter] Setting visibility for UDL layer:" << m_layerId << "->" << visString;
        QString fillId = m_layerId + "-fill";
        QString lineId = m_layerId + "-line";
        QString circleId = m_layerId + "-circle";
        QString symbolId = m_layerId + "-symbol";
        if (m_map->layerExists(fillId)) m_map->setLayoutProperty(fillId, "visibility", visString);
        if (m_map->layerExists(lineId)) m_map->setLayoutProperty(lineId, "visibility", visString);
        if (m_map->layerExists(circleId)) m_map->setLayoutProperty(circleId, "visibility", visString);
        if (m_map->layerExists(symbolId)) m_map->setLayoutProperty(symbolId, "visibility", visString);
    } else if (m_layerId == "dark-matter-base") {
        const auto layers = m_map->layerIds();
        for (const QString &id : layers) {
            // Protect user-published custom layers from base map bulk toggling
            if (id != "air-zones-layer" && id != "radar-coverage" && 
                !id.startsWith("raster-") && !id.startsWith("vector-") && id != "google-satellite-layer") {
                m_map->setLayoutProperty(id, "visibility", visString);
            }
        }
    } else {
        qWarning() << "[LayerAdapter] Setting visibility for single layer:" << m_layerId << "->" << visString;
        if (m_map->layerExists(m_layerId)) {
            m_map->setLayoutProperty(m_layerId, "visibility", visString);
        }
        QString strokeId = m_layerId + "-stroke";
        if (m_map->layerExists(strokeId)) {
            m_map->setLayoutProperty(strokeId, "visibility", visString);
        }
    }
}

bool MapLibreLayerAdapter::isVisible() const {
    return m_visible;
}

void MapLibreLayerAdapter::setOpacity(float opacity) {
    m_opacity = std::clamp(opacity, 0.0f, 1.0f);
    if (!m_map) return;

    double dOpacity = static_cast<double>(m_opacity);

    if (m_layerId.startsWith("udl-")) {
        qWarning() << "[LayerAdapter] Setting opacity for UDL layer:" << m_layerId << "->" << dOpacity;
        QString fillId = m_layerId + "-fill";
        QString lineId = m_layerId + "-line";
        QString circleId = m_layerId + "-circle";
        QString symbolId = m_layerId + "-symbol";
        if (m_map->layerExists(fillId)) {
            if (qFuzzyCompare(dOpacity, 1.0)) {
                m_map->setPaintProperty(fillId, "fill-opacity", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, 0.35}});
            } else {
                m_map->setPaintProperty(fillId, "fill-opacity", QVariantList{"*", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, 0.35}}, dOpacity});
            }
        }
        if (m_map->layerExists(lineId)) {
            if (qFuzzyCompare(dOpacity, 1.0)) {
                m_map->setPaintProperty(lineId, "line-opacity", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "strokeOpacity"}, QVariantList{"get", "lineOpacity"}, 1.0}});
            } else {
                m_map->setPaintProperty(lineId, "line-opacity", QVariantList{"*", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "strokeOpacity"}, QVariantList{"get", "lineOpacity"}, 1.0}}, dOpacity});
            }
        }
        if (m_map->layerExists(circleId)) {
            if (qFuzzyCompare(dOpacity, 1.0)) {
                m_map->setPaintProperty(circleId, "circle-opacity", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, QVariantList{"get", "strokeOpacity"}, 1.0}});
            } else {
                m_map->setPaintProperty(circleId, "circle-opacity", QVariantList{"*", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, QVariantList{"get", "strokeOpacity"}, 1.0}}, dOpacity});
            }
        }
        if (m_map->layerExists(symbolId)) {
            if (qFuzzyCompare(dOpacity, 1.0)) {
                m_map->setPaintProperty(symbolId, "text-opacity", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "textOpacity"}, 1.0}});
            } else {
                m_map->setPaintProperty(symbolId, "text-opacity", QVariantList{"*", QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "textOpacity"}, 1.0}}, dOpacity});
            }
        }
    } else if (m_layerId == "dark-matter-base") {
        const auto layers = m_map->layerIds();
        for (const QString &id : layers) {
            if (id != "air-zones-layer" && id != "radar-coverage" && 
                !id.startsWith("raster-") && !id.startsWith("vector-")) {
                m_map->setPaintProperty(id, "raster-opacity", dOpacity);
                m_map->setPaintProperty(id, "fill-opacity", dOpacity);
            }
        }
    } else {
        qWarning() << "[LayerAdapter] Setting opacity for single layer:" << m_layerId << "->" << dOpacity;
        if (m_layerId.startsWith("raster-")) {
            m_map->setPaintProperty(m_layerId, "raster-opacity", dOpacity);
        } else {
            QString mainType = m_layerParams.value("type").toString();
            if (mainType == "fill") {
                m_map->setPaintProperty(m_layerId, "fill-opacity", dOpacity);
            } else if (mainType == "line") {
                m_map->setPaintProperty(m_layerId, "line-opacity", dOpacity);
            } else if (mainType == "symbol") {
                m_map->setPaintProperty(m_layerId, "icon-opacity", dOpacity);
            } else if (mainType == "circle") {
                m_map->setPaintProperty(m_layerId, "circle-opacity", dOpacity);
            }

            QString strokeType = m_strokeParams.value("type").toString();
            if (strokeType == "line") {
                m_map->setPaintProperty(m_layerId + "-stroke", "line-opacity", dOpacity);
            } else if (strokeType == "circle") {
                m_map->setPaintProperty(m_layerId + "-stroke", "circle-opacity", dOpacity);
            }
        }
    }
}

float MapLibreLayerAdapter::opacity() const {
    return m_opacity;
}

LayerExtent MapLibreLayerAdapter::getExtent() const {
    if (m_layerId.startsWith("udl-") || !m_rawUdlLayerId.isEmpty()) {
        QString targetUdlId = !m_rawUdlLayerId.isEmpty() ? m_rawUdlLayerId : m_layerId;
        auto calc = GISApp::Publishing::UdlRepositoryManager::instance().calculateLayerExtent(targetUdlId);
        if (calc.isValid()) {
            return calc;
        }
    }
    return m_extent;
}

void MapLibreLayerAdapter::setExtent(const LayerExtent &extent) {
    m_extent = extent;
}

void MapLibreLayerAdapter::removeLayer() {
    if (!m_map || m_layerId.isEmpty()) return;

    qWarning() << "[MapLibreLayerAdapter] Removing layer and source from MapLibre engine:" << m_layerId;

    if (m_layerId.startsWith("udl-")) {
        QString fillId = m_layerId + "-fill";
        QString lineId = m_layerId + "-line";
        QString circleId = m_layerId + "-circle";
        QString symbolId = m_layerId + "-symbol";
        QString srcId = m_layerId + "-src";
        if (m_map->layerExists(symbolId)) m_map->removeLayer(symbolId);
        if (m_map->layerExists(circleId)) m_map->removeLayer(circleId);
        if (m_map->layerExists(lineId)) m_map->removeLayer(lineId);
        if (m_map->layerExists(fillId)) m_map->removeLayer(fillId);
        if (m_map->sourceExists(srcId)) m_map->removeSource(srcId);
        return;
    }

    if (m_map->layerExists(m_layerId)) {
        m_map->removeLayer(m_layerId);
    }
    QString strokeId = m_layerId + "-stroke";
    if (m_map->layerExists(strokeId)) {
        m_map->removeLayer(strokeId);
    }

    QString sourceId = m_layerId + "-src";
    if (m_map->sourceExists(sourceId)) {
        m_map->removeSource(sourceId);
    }
}

void MapLibreLayerAdapter::reinsertLayer(const QString &beforeLayerId) {
    if (!m_map || m_layerId.isEmpty()) return;

    if (m_layerId.startsWith("udl-")) {
        // UDL sub-layers (-fill, -line, -circle) are managed dynamically via GeoJSON sources
        return;
    }

    if (m_layerId == "tracks-circle-layer") {
        if (!m_map->sourceExists("tracks-geojson-source")) {
            QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
            QString geojsonPath = mapDataDir + "/tactical_tracks.geojson";
            QVariantMap sourceParams;
            sourceParams["type"] = "geojson";
            sourceParams["data"] = QString("file://%1").arg(geojsonPath);
            m_map->addSource("tracks-geojson-source", sourceParams);
        }
        if (m_layerParams.isEmpty()) {
            m_layerParams["id"] = "tracks-circle-layer";
            m_layerParams["type"] = "circle";
            m_layerParams["source"] = "tracks-geojson-source";
            
            QVariantList getColorExpr;
            getColorExpr << "get" << "color";

            QVariantMap paintMap;
            paintMap["circle-color"] = getColorExpr;
            paintMap["circle-radius"] = 9.0;
            paintMap["circle-stroke-color"] = "#FFFFFF";
            paintMap["circle-stroke-width"] = 2.5;
            m_layerParams["paint"] = paintMap;


        }
    }

    if (m_layerId == "area-of-view-fill-layer") {
        if (!m_map->sourceExists("area-of-view-geojson-source")) {
            QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
            QString geojsonPath = mapDataDir + "/area_of_view.geojson";
            if (!QFile::exists(geojsonPath)) {
                QFile file(geojsonPath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(R"({"type":"FeatureCollection","features":[]})");
                    file.close();
                }
            }
            QVariantMap sourceParams;
            sourceParams["type"] = "geojson";
            sourceParams["data"] = QString("file://%1").arg(geojsonPath);
            m_map->addSource("area-of-view-geojson-source", sourceParams);
        }

        if (m_layerParams.isEmpty()) {
            m_layerParams["id"] = "area-of-view-fill-layer";
            m_layerParams["type"] = "fill";
            m_layerParams["source"] = "area-of-view-geojson-source";
            QVariantMap fillPaint;
            fillPaint["fill-color"] = "rgba(0, 0, 0, 0)";
            fillPaint["fill-opacity"] = 0.0;
            m_layerParams["paint"] = fillPaint;
        }

        if (m_strokeParams.isEmpty()) {
            m_strokeParams["id"] = "area-of-view-fill-layer-stroke";
            m_strokeParams["type"] = "line";
            m_strokeParams["source"] = "area-of-view-geojson-source";
            QVariantMap strokePaint;
            strokePaint["line-color"] = "#8B4513";
            strokePaint["line-width"] = 3.5;
            m_strokeParams["paint"] = strokePaint;
        }
    }

    if (m_layerParams.isEmpty()) {
        // Do not remove layer if we cannot re-add it (avoids destroying native engine layers)
        return;
    }


    qWarning() << "[MapLibreLayerAdapter] Re-inserting layer:" << m_layerId << "BEFORE:" << beforeLayerId;

    QString strokeId = m_layerId + "-stroke";
    try {
        if (m_map->layerExists(m_layerId)) {
            m_map->removeLayer(m_layerId);
        }
        if (m_map->layerExists(strokeId)) {
            m_map->removeLayer(strokeId);
        }

        QString targetBefore = beforeLayerId;
        if (targetBefore.isEmpty() && m_layerId != "boundary-outer-line-layer" && m_layerId != "boundary-inner-line-layer" && m_layerId != "tracks-circle-layer") {
            if (m_map->layerExists("boundary-outer-line-layer")) {
                targetBefore = "boundary-outer-line-layer";
            } else if (m_map->layerExists("area-of-view-fill-layer")) {
                targetBefore = "area-of-view-fill-layer";
            } else if (m_map->layerExists("tracks-circle-layer")) {
                targetBefore = "tracks-circle-layer";
            }
        }

        if (!m_map->layerExists(m_layerId)) {
            if (targetBefore.isEmpty()) {
                m_map->addLayer(m_layerId, m_layerParams);
            } else {
                m_map->addLayer(m_layerId, m_layerParams, targetBefore);
            }
        }

        if (!m_strokeParams.isEmpty() && !m_map->layerExists(strokeId)) {
            if (targetBefore.isEmpty()) {
                m_map->addLayer(strokeId, m_strokeParams);
            } else {
                m_map->addLayer(strokeId, m_strokeParams, targetBefore);
            }
        }
    } catch (const std::exception &e) {
        qWarning() << "[MapLibreLayerAdapter] Exception during reinsertLayer:" << e.what();
    } catch (...) {
        qWarning() << "[MapLibreLayerAdapter] Unknown exception during reinsertLayer";
    }

    if (m_map->layerExists(m_layerId)) {
        m_map->setLayoutProperty(m_layerId, "visibility", m_visible ? "visible" : "none");
        QString typeStr = m_layerParams.value("type").toString();
        if (typeStr == "raster") {
            m_map->setPaintProperty(m_layerId, "raster-opacity", static_cast<double>(m_opacity));
        } else if (typeStr == "fill") {
            QVariantMap paint = m_layerParams.value("paint").toMap();
            if (paint.contains("fill-opacity") && paint.value("fill-opacity").toDouble() == 0.0) {
                m_map->setPaintProperty(m_layerId, "fill-opacity", 0.0);
            } else {
                m_map->setPaintProperty(m_layerId, "fill-opacity", static_cast<double>(m_opacity));
            }
        } else if (typeStr == "symbol") {
            m_map->setPaintProperty(m_layerId, "icon-opacity", static_cast<double>(m_opacity));
        } else if (typeStr == "circle") {
            m_map->setPaintProperty(m_layerId, "circle-opacity", static_cast<double>(m_opacity));
        }
    }

    if (!m_strokeParams.isEmpty() && m_map->layerExists(strokeId)) {
        m_map->setLayoutProperty(strokeId, "visibility", m_visible ? "visible" : "none");
        QString strokeTypeStr = m_strokeParams.value("type").toString();
        if (strokeTypeStr == "line") {
            m_map->setPaintProperty(strokeId, "line-opacity", static_cast<double>(m_opacity));
        } else if (strokeTypeStr == "circle") {
            m_map->setPaintProperty(strokeId, "circle-opacity", static_cast<double>(m_opacity));
        }
    }
}

} // namespace GISApp::Layers
