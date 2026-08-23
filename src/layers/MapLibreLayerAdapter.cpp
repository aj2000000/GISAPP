/**
 * @file MapLibreLayerAdapter.cpp
 * @brief Implementation of MapLibre Native Layer Adapter.
 */

#include "layers/MapLibreLayerAdapter.h"
#include <QVariantMap>

namespace GISApp::Layers {

MapLibreLayerAdapter::MapLibreLayerAdapter(const QString &layerId, QMapLibre::Map *mapPointer, const LayerExtent &defaultExtent)
    : m_layerId(layerId), m_map(mapPointer), m_extent(defaultExtent)
{
}

QString MapLibreLayerAdapter::layerId() const {
    return m_layerId;
}

#include <QDebug>

void MapLibreLayerAdapter::setVisibility(bool visible) {
    m_visible = visible;
    if (!m_map) return;

    QString visString = visible ? QString("visible") : QString("none");

    if (m_layerId == "dark-matter-base") {
        const auto layers = m_map->layerIds();
        qWarning() << "[DEBUG - LayerAdapter] Toggling base map to" << visString << "| Layers count:" << layers.size();
        for (const QString &id : layers) {
            // Keep user-added custom tactical overlays intact
            if (id != "air-zones-layer" && id != "radar-coverage") {
                m_map->setLayoutProperty(id, "visibility", visString);
            }
        }
    } else {
        qWarning() << "[DEBUG - LayerAdapter] Toggling single layer:" << m_layerId << "to" << visString;
        m_map->setLayoutProperty(m_layerId, "visibility", visString);
    }
}

bool MapLibreLayerAdapter::isVisible() const {
    return m_visible;
}

void MapLibreLayerAdapter::setOpacity(float opacity) {
    m_opacity = std::clamp(opacity, 0.0f, 1.0f);
    if (!m_map) return;

    if (m_layerId == "dark-matter-base") {
        const auto layers = m_map->layerIds();
        qWarning() << "[DEBUG - LayerAdapter] Setting base map opacity to" << m_opacity << "| Layers count:" << layers.size();
        for (const QString &id : layers) {
            if (id != "air-zones-layer" && id != "radar-coverage") {
                m_map->setPaintProperty(id, "raster-opacity", m_opacity);
                m_map->setPaintProperty(id, "fill-opacity", m_opacity);
                m_map->setPaintProperty(id, "line-opacity", m_opacity);
                m_map->setPaintProperty(id, "circle-opacity", m_opacity);
                m_map->setPaintProperty(id, "symbol-opacity", m_opacity);
                m_map->setPaintProperty(id, "background-opacity", m_opacity);
            }
        }
    } else {
        qWarning() << "[DEBUG - LayerAdapter] Setting single layer opacity:" << m_layerId << "to" << m_opacity;
        m_map->setPaintProperty(m_layerId, "raster-opacity", m_opacity);
        m_map->setPaintProperty(m_layerId, "fill-opacity", m_opacity);
        m_map->setPaintProperty(m_layerId, "line-opacity", m_opacity);
        m_map->setPaintProperty(m_layerId, "circle-opacity", m_opacity);
        m_map->setPaintProperty(m_layerId, "symbol-opacity", m_opacity);
        m_map->setPaintProperty(m_layerId, "background-opacity", m_opacity);
    }
}

float MapLibreLayerAdapter::opacity() const {
    return m_opacity;
}

LayerExtent MapLibreLayerAdapter::getExtent() const {
    return m_extent;
}

} // namespace GISApp::Layers
