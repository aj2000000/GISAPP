/**
 * @file ILayerAdapter.h
 * @brief Interface abstraction for GIS map layer adapters (Bridge Pattern).
 *
 * Provides single-responsibility abstraction for controlling layer visibility,
 * opacity, spatial extent/bounds, and MapLibre style attributes.
 *
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef ILAYERADAPTER_H
#define ILAYERADAPTER_H

#include <QString>
#include <QPair>
#include "core/models/GeoCoordinate.h"

namespace GISApp::Layers {

/**
 * @brief Represents a bounding box extent (SouthWest, NorthEast).
 */
struct LayerExtent {
    GISApp::Core::Models::GeoCoordinate southWest;
    GISApp::Core::Models::GeoCoordinate northEast;

    bool isValid() const {
        return southWest.isValid() && northEast.isValid();
    }
};

/**
 * @class ILayerAdapter
 * @brief Abstract Base Class for underlying GIS Map rendering layers.
 *
 * Adheres to Interface Segregation Principle (ISP) and Dependency Inversion (DIP).
 */
class ILayerAdapter {
public:
    virtual ~ILayerAdapter() = default;

    /**
     * @brief Get the unique engine layer identifier string.
     * @return QString Unique layer ID in MapLibre style stack.
     */
    virtual QString layerId() const = 0;

    /**
     * @brief Set layer visibility state.
     * @param visible True to enable rendering, false to hide layer.
     */
    virtual void setVisibility(bool visible) = 0;

    /**
     * @brief Check if layer is currently set to visible.
     * @return bool True if visible.
     */
    virtual bool isVisible() const = 0;

    /**
     * @brief Set layer opacity/transparency.
     * @param opacity Floating-point value from 0.0 (fully transparent) to 1.0 (fully opaque).
     */
    virtual void setOpacity(float opacity) = 0;

    /**
     * @brief Get current opacity of the layer.
     * @return float Value in range [0.0, 1.0].
     */
    virtual float opacity() const = 0;

    /**
     * @brief Get geographic spatial bounds of this layer for Panning / Zoom to Extent.
     * @return LayerExtent Struct containing bounding coordinates.
     */
    virtual LayerExtent getExtent() const = 0;

    /**
     * @brief Set geographic spatial bounds of this layer.
     */
    virtual void setExtent(const LayerExtent &extent) { (void)extent; }

    /**
     * @brief Remove layer and associated source from underlying engine.
     */
    virtual void removeLayer() = 0;

    /**
     * @brief Re-insert layer into engine graphics Z-stack before target layer ID.
     */
    virtual void reinsertLayer(const QString &beforeLayerId) { (void)beforeLayerId; }
};

} // namespace GISApp::Layers

#endif // ILAYERADAPTER_H
