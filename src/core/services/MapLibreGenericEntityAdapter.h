/**
 * @file MapLibreGenericEntityAdapter.h
 * @brief Universal MapLibre Adapter for rendering all polymorphic GIS entities.
 * Automatically synchronizes GeoJSON sources and style layers with IGisEntityRepository.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef MAPLIBREGENERICENTITYADAPTER_H
#define MAPLIBREGENERICENTITYADAPTER_H

#include <QObject>
#include <QMapLibre/Map>
#include "../repositories/IGisEntityRepository.h"
#include "../renderers/IMapRendererAdapter.h"

namespace GISApp::Layers {
    class LayerManager;
}

namespace GISApp::Core::Services {

using GISApp::Core::Repositories::IGisEntityRepository;
using GISApp::Core::Models::GenericGisEntity;

/**
 * @class MapLibreGenericEntityAdapter
 * @brief Observer adapter bridging IGisEntityRepository with QMapLibre::Map engine.
 */
class MapLibreGenericEntityAdapter : public QObject, public Renderers::IMapRendererAdapter {
    Q_OBJECT

public:
    explicit MapLibreGenericEntityAdapter(
        QMapLibre::Map *map,
        IGisEntityRepository *repository,
        QObject *parent = nullptr
    );
    virtual ~MapLibreGenericEntityAdapter() override = default;

    void setMap(QMapLibre::Map *map);
    void setLayerManager(Layers::LayerManager *layerManager);
    void setSourceId(const QString &sourceId) { m_sourceId = sourceId; }

    // IMapRendererAdapter Interface Implementation
    void renderEntity(std::shared_ptr<Models::IGisEntity> entity) override;
    void renderEntities(const QVector<std::shared_ptr<Models::IGisEntity>> &entities) override;
    void removeEntity(const QString &entityId) override;
    void clearEntities() override;

public slots:
    void refreshFromRepository();
    void onEntityAdded(std::shared_ptr<GenericGisEntity> entity);
    void onEntityUpdated(std::shared_ptr<GenericGisEntity> entity);
    void onEntityRemoved(const QString &entityId);

private:
    void ensureMapLayersCreated();

    QMapLibre::Map *m_map{nullptr};
    IGisEntityRepository *m_repository{nullptr};
    Layers::LayerManager *m_layerManager{nullptr};

    QString m_sourceId{"generic-entities-src"};
    QString m_circleLayerId{"generic-entities-circles"};
    QString m_lineLayerId{"generic-entities-lines"};
    QString m_fillLayerId{"generic-entities-fills"};

    bool m_layersCreated{false};
};

} // namespace GISApp::Core::Services

#endif // MAPLIBREGENERICENTITYADAPTER_H
