/**
 * @file MapLibreTrackAdapter.h
 * @brief Concrete MapLibre Native renderer adapter implementing IMapRendererAdapter.
 * Translates domain GIS entities into MapLibre GeoJSON sources and style layers.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef MAPLIBRETRACKADAPTER_H
#define MAPLIBRETRACKADAPTER_H

#include "../renderers/IMapRendererAdapter.h"
#include "../repositories/ITrackRepository.h"
#include <QObject>
#include <QMapLibre/Map>

namespace GISApp::Layers {
    class LayerManager;
}

namespace GISApp::Core::Services {

class MapLibreTrackAdapter : public QObject, public Renderers::IMapRendererAdapter {
    Q_OBJECT
public:
    explicit MapLibreTrackAdapter(QMapLibre::Map *map, Repositories::ITrackRepository *repository, QObject *parent = nullptr);
    ~MapLibreTrackAdapter() override = default;

    void setMap(QMapLibre::Map *map);
    void setLayerManager(Layers::LayerManager *layerManager);

    // IMapRendererAdapter Interface Implementation
    void renderEntity(std::shared_ptr<Models::IGisEntity> entity) override;
    void renderEntities(const QVector<std::shared_ptr<Models::IGisEntity>> &entities) override;
    void removeEntity(const QString &entityId) override;
    void clearEntities() override;

public slots:
    void refreshFromRepository();

private:
    void ensureLayersCreated();

    QMapLibre::Map *m_map{nullptr};
    Repositories::ITrackRepository *m_repository{nullptr};
    Layers::LayerManager *m_layerManager{nullptr};
    bool m_layersCreated{false};
};

} // namespace GISApp::Core::Services

#endif // MAPLIBRETRACKADAPTER_H
