/**
 * @file GenericGisEntity.cpp
 * @brief Implementation of GenericGisEntity.
 */

#include "GenericGisEntity.h"
#include <QUuid>

namespace GISApp::Core::Models {

GenericGisEntity::GenericGisEntity()
    : m_entityId(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_entityName("Unnamed Entity")
{
}

GenericGisEntity::GenericGisEntity(const QString &id, const QString &name, const QString &typeId)
    : m_entityId(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id)
    , m_entityName(name)
    , m_entityType(typeId)
{
}

QVariant GenericGisEntity::property(const QString &key, const QVariant &defaultValue) const
{
    return m_properties.value(key, defaultValue);
}

void GenericGisEntity::setProperty(const QString &key, const QVariant &value)
{
    m_properties.insert(key, value);
}

QJsonObject GenericGisEntity::toGeoJsonFeature() const
{
    QJsonObject feature;
    feature["type"] = "Feature";
    feature["id"] = m_entityId;

    if (m_geometry) {
        feature["geometry"] = m_geometry->toGeoJsonGeometry();
    } else {
        feature["geometry"] = QJsonObject();
    }

    QJsonObject propsObj;
    propsObj["id"] = m_entityId;
    propsObj["name"] = m_entityName;
    propsObj["entity_type"] = m_entityType;
    propsObj["category"] = static_cast<int>(m_category);
    propsObj["stroke"] = m_renderStyle.strokeColor.name();
    propsObj["stroke-width"] = m_renderStyle.strokeWidth;
    propsObj["stroke-opacity"] = m_renderStyle.strokeColor.alphaF();
    propsObj["fill"] = m_renderStyle.fillColor.name();
    propsObj["fill-opacity"] = m_renderStyle.fillColor.alphaF();
    propsObj["heading"] = m_renderStyle.rotationHeading;

    // Export dynamic custom properties into properties object
    for (auto it = m_properties.constBegin(); it != m_properties.constEnd(); ++it) {
        propsObj[it.key()] = QJsonValue::fromVariant(it.value());
    }

    feature["properties"] = propsObj;
    return feature;
}

} // namespace GISApp::Core::Models
