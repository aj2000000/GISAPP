/**
 * @file MapLibreAreaOfViewAdapter.h
 * @brief MapLibre engine adapter for rendering Area of View Polygons on the map canvas.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef MAPLIBREAREAOFVIEWADAPTER_H
#define MAPLIBREAREAOFVIEWADAPTER_H

#include "../repositories/IAreaOfViewRepository.h"
#include <QMapLibre/Map>
#include <QObject>

namespace GISApp::Core::Services {

class MapLibreAreaOfViewAdapter : public QObject {
    Q_OBJECT
public:
    explicit MapLibreAreaOfViewAdapter(QMapLibre::Map *map, Repositories::IAreaOfViewRepository *repository, QObject *parent = nullptr);
    ~MapLibreAreaOfViewAdapter() override = default;

    void setMap(QMapLibre::Map *map);
    void ensureLayersCreated();

public slots:
    void refreshFromRepository();

private:
    QMapLibre::Map *m_map{nullptr};
    Repositories::IAreaOfViewRepository *m_repository{nullptr};
    bool m_layersCreated{false};
};

} // namespace GISApp::Core::Services

#endif // MAPLIBREAREAOFVIEWADAPTER_H
