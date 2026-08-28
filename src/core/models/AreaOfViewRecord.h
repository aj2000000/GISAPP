/**
 * @file AreaOfViewRecord.h
 * @brief Domain model representing an 'Area of View' Polygon Entity.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef AREAOFVIEWRECORD_H
#define AREAOFVIEWRECORD_H

#include "IGisEntity.h"
#include <QJsonObject>
#include <QVector>
#include <memory>

namespace GISApp::Core::Models {

class AreaOfViewRecord : public IGisEntity {
public:
    AreaOfViewRecord() = default;
    ~AreaOfViewRecord() override = default;

    // IGisEntity Interface Implementation
    QString entityId() const override { return QString::number(id); }
    QString entityName() const override { return name; }
    EntityCategory category() const override { return EntityCategory::AirZone; }

    std::shared_ptr<IGisGeometry> geometry() const override;
    EntityRenderStyle renderStyle() const override;
    QJsonObject toGeoJsonFeature() const override;

    // Domain Schema Fields
    int id{0};
    QString name;
    int nPoints{0};
    QVector<Coordinate3D> points;
};

} // namespace GISApp::Core::Models

#endif // AREAOFVIEWRECORD_H
