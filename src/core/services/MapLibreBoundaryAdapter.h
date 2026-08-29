#ifndef MAPLIBREBOUNDARYADAPTER_H
#define MAPLIBREBOUNDARYADAPTER_H

#include <QObject>
#include <QVector>
#include "QMapLibre/Map"
#include "models/BoundaryRecord.h"

namespace GISApp::Core::Services {

class MapLibreBoundaryAdapter : public QObject {
    Q_OBJECT
public:
    explicit MapLibreBoundaryAdapter(QMapLibre::Map *map, QObject *parent = nullptr);
    ~MapLibreBoundaryAdapter() override = default;

    void setMap(QMapLibre::Map *map);
    void setBoundaries(const QVector<Models::BoundaryRecord> &boundaries);
    QVector<Models::BoundaryRecord> loadSavedBoundaries();
    void ensureLayersCreated(const QByteArray &geoJsonData = QByteArray());

private:
    QMapLibre::Map *m_map{nullptr};
    QVector<Models::BoundaryRecord> m_boundaries;
    bool m_layersCreated{false};
};

} // namespace GISApp::Core::Services

#endif // MAPLIBREBOUNDARYADAPTER_H
