/**
 * @file MapLibreLayerAdapter.cpp
 * @brief Implementation of MapLibre Native Layer Adapter with custom layer filtering.
 */

#include "layers/MapLibreLayerAdapter.h"
#include <QVariantMap>
#include <QDebug>
#include <algorithm>

namespace GISApp::Layers {

MapLibreLayerAdapter::MapLibreLayerAdapter(const QString &layerId, 
                                           QMapLibre::Map *mapPointer, 
                                           const LayerExtent &defaultExtent,
                                           const QVariantMap &layerParams,
                                           const QVariantMap &strokeParams,
                                           const QVariantMap &sourceParams)
    : m_layerId(layerId), 
      m_map(mapPointer), 
      m_extent(defaultExtent),
      m_layerParams(layerParams),
      m_strokeParams(strokeParams),
      m_sourceParams(sourceParams)
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

    if (m_layerId == "dark-matter-base") {
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

    if (m_layerId == "dark-matter-base") {
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
    return m_extent;
}

void MapLibreLayerAdapter::removeLayer() {
    if (!m_map || m_layerId.isEmpty()) return;

    qWarning() << "[MapLibreLayerAdapter] Removing layer and source from MapLibre engine:" << m_layerId;

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

    qWarning() << "[MapLibreLayerAdapter] Re-inserting layer:" << m_layerId << "BEFORE:" << beforeLayerId;

    if (m_map->layerExists(m_layerId)) {
        m_map->removeLayer(m_layerId);
    }
    QString strokeId = m_layerId + "-stroke";
    if (m_map->layerExists(strokeId)) {
        m_map->removeLayer(strokeId);
    }

    if (!m_layerParams.isEmpty()) {
        m_map->addLayer(m_layerId, m_layerParams, beforeLayerId);
        m_map->setLayoutProperty(m_layerId, "visibility", m_visible ? "visible" : "none");
        QString typeStr = m_layerParams.value("type").toString();
        if (typeStr == "raster") {
            m_map->setPaintProperty(m_layerId, "raster-opacity", static_cast<double>(m_opacity));
        } else if (typeStr == "fill") {
            m_map->setPaintProperty(m_layerId, "fill-opacity", static_cast<double>(m_opacity));
        } else if (typeStr == "symbol") {
            m_map->setPaintProperty(m_layerId, "icon-opacity", static_cast<double>(m_opacity));
        } else if (typeStr == "circle") {
            m_map->setPaintProperty(m_layerId, "circle-opacity", static_cast<double>(m_opacity));
        }
    }
    if (!m_strokeParams.isEmpty()) {
        m_map->addLayer(strokeId, m_strokeParams, beforeLayerId);
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
