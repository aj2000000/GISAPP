/**
 * @file GenericGisEntity.h
 * @brief Concrete implementation of IGisEntity supporting dynamic properties, geometry, and styling.
 * Designed for SOLID Open-Closed Principle and developer extensibility.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef GENERICGISENTITY_H
#define GENERICGISENTITY_H

#include "IGisEntity.h"
#include <QVariantMap>

namespace GISApp::Core::Models {

/**
 * @class GenericGisEntity
 * @brief Universal concrete class representing any GIS spatial entity.
 */
class GenericGisEntity : public IGisEntity {
public:
    GenericGisEntity();
    GenericGisEntity(const QString &id, const QString &name, const QString &typeId = "generic");
    virtual ~GenericGisEntity() override = default;

    // IGisEntity Interface Implementation
    QString entityId() const override { return m_entityId; }
    QString entityName() const override { return m_entityName; }
    EntityCategory category() const override { return m_category; }
    std::shared_ptr<IGisGeometry> geometry() const override { return m_geometry; }
    EntityRenderStyle renderStyle() const override { return m_renderStyle; }
    QJsonObject toGeoJsonFeature() const override;

    // Setters & Attribute Mutators
    void setEntityId(const QString &id) { m_entityId = id; }
    void setEntityName(const QString &name) { m_entityName = name; }
    void setCategory(EntityCategory category) { m_category = category; }
    void setGeometry(std::shared_ptr<IGisGeometry> geometry) { m_geometry = geometry; }
    void setRenderStyle(const EntityRenderStyle &style) { m_renderStyle = style; }

    // Dynamic Property Operations
    QString entityType() const { return m_entityType; }
    void setEntityType(const QString &typeId) { m_entityType = typeId; }

    QVariant property(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setProperty(const QString &key, const QVariant &value);
    QVariantMap properties() const { return m_properties; }
    void setProperties(const QVariantMap &props) { m_properties = props; }

private:
    QString m_entityId;
    QString m_entityName;
    QString m_entityType{"generic"};
    EntityCategory m_category{EntityCategory::Custom};
    std::shared_ptr<IGisGeometry> m_geometry;
    EntityRenderStyle m_renderStyle;
    QVariantMap m_properties;
};

} // namespace GISApp::Core::Models

#endif // GENERICGISENTITY_H
