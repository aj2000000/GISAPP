#ifndef BOUNDARYRECORD_H
#define BOUNDARYRECORD_H

#include "IGisEntity.h"
#include <QJsonObject>
#include <QVector>
#include <memory>

namespace GISApp::Core::Models {

class BoundaryRecord : public IGisEntity {
public:
    BoundaryRecord() = default;
    ~BoundaryRecord() override = default;

    // IGisEntity Interface Implementation
    QString entityId() const override { return QString::number(boundaryId); }
    QString entityName() const override { return name.isEmpty() ? QString("Boundary #%1").arg(boundaryId) : name; }
    EntityCategory category() const override { return EntityCategory::AirZone; }

    std::shared_ptr<IGisGeometry> geometry() const override { return nullptr; }
    EntityRenderStyle renderStyle() const override { return EntityRenderStyle(); }
    QJsonObject toGeoJsonFeature() const override;

    // Fields
    int boundaryId{0};
    QString name{"Boundary"};
    QVector<Coordinate3D> points;
};

} // namespace GISApp::Core::Models

#endif // BOUNDARYRECORD_H
