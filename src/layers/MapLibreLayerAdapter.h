/**
 * @file MapLibreLayerAdapter.h
 * @brief Concrete MapLibre Native binding for ILayerAdapter.
 */

#ifndef MAPLIBRELAYERADAPTER_H
#define MAPLIBRELAYERADAPTER_H

#include "layers/ILayerAdapter.h"
#include <QMapLibre/Map>

namespace GISApp::Layers {

/**
 * @class MapLibreLayerAdapter
 * @brief Adapts QMapLibre Native C++ layer calls to ILayerAdapter interface.
 */
class MapLibreLayerAdapter : public ILayerAdapter {
public:
    /**
     * @brief Construct MapLibreLayerAdapter.
     * @param layerId Unique layer ID registered in MapLibre style JSON.
     * @param mapPointer Pointer to active QMapLibre::Map instance.
     * @param defaultExtent Spatial extent for pan to bounds.
     */
    MapLibreLayerAdapter(const QString &layerId, 
                         QMapLibre::Map *mapPointer, 
                         const LayerExtent &defaultExtent = LayerExtent(),
                         const QVariantMap &layerParams = {},
                         const QVariantMap &strokeParams = {},
                         const QVariantMap &sourceParams = {});

    QString layerId() const override;
    void setVisibility(bool visible) override;
    bool isVisible() const override;
    void setOpacity(float opacity) override;
    float opacity() const override;
    LayerExtent getExtent() const override;
    void removeLayer() override;
    void reinsertLayer(const QString &beforeLayerId) override;

private:
    QString m_layerId;
    QMapLibre::Map *m_map;
    bool m_visible{true};
    float m_opacity{1.0f};
    LayerExtent m_extent;
    QVariantMap m_layerParams;
    QVariantMap m_strokeParams;
    QVariantMap m_sourceParams;
};

} // namespace GISApp::Layers

#endif // MAPLIBRELAYERADAPTER_H
