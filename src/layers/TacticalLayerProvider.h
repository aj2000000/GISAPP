#ifndef GISAPP_LAYERS_TACTICALLAYERPROVIDER_H
#define GISAPP_LAYERS_TACTICALLAYERPROVIDER_H

#include <QObject>
#include <QMapLibre/Map>
#include "layers/LayerManager.h"
#include "controllers/MapController.h"

namespace GISApp {
namespace Layers {

/**
 * @brief Responsible for generating tactical layers and setting up the Layer Tree.
 */
class TacticalLayerProvider : public QObject {
    Q_OBJECT
public:
    explicit TacticalLayerProvider(QObject *parent = nullptr);

    /**
     * @brief Populates MapLibre with tactical GeoJSON sources and vector paint styling.
     */
    void setupTacticalLayers(QMapLibre::Map *map);

    /**
     * @brief Assembles the Layer Tree architecture with base maps and tactical overlays.
     */
    void populateLayerTree(LayerManager *layerManager, QMapLibre::Map *map, GISApp::Controllers::MapController *mapController);
};

} // namespace Layers
} // namespace GISApp

#endif // GISAPP_LAYERS_TACTICALLAYERPROVIDER_H
