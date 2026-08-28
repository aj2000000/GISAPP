/**
 * @file GisEntityRegistry.cpp
 * @brief Implementation of GisEntityRegistry.
 */

#include "GisEntityRegistry.h"
#include <QDebug>

namespace GISApp::Core::Models {

GisEntityRegistry& GisEntityRegistry::instance()
{
    static GisEntityRegistry registry;
    return registry;
}

GisEntityRegistry::GisEntityRegistry()
{
    registerBuiltInTypes();
}

void GisEntityRegistry::registerBuiltInTypes()
{
    // 1. Tactical Track Entity Type
    EntityTypeDescriptor trackDesc;
    trackDesc.typeId = "track";
    trackDesc.displayName = "Tactical Track";
    trackDesc.category = EntityCategory::Track;
    trackDesc.painterStrategyId = "circle";
    trackDesc.defaultStyle.strokeColor = QColor("#0284c7");
    trackDesc.defaultStyle.fillColor = QColor("#38bdf8");
    trackDesc.defaultStyle.strokeWidth = 2.0;
    trackDesc.propertySchemas = {
        {"speed", "Speed (kts)", QMetaType::Double, 0.0, false, {}},
        {"course", "Course (°)", QMetaType::Double, 0.0, false, {}},
        {"altitude", "Altitude (m)", QMetaType::Double, 0.0, false, {}},
        {"affinity", "Affinity", QMetaType::QString, "FRIENDLY", false, {"FRIENDLY", "HOSTILE", "NEUTRAL", "UNKNOWN"}}
    };
    registerEntityType(trackDesc);

    // 2. Area of View Entity Type
    EntityTypeDescriptor aovDesc;
    aovDesc.typeId = "area_of_view";
    aovDesc.displayName = "Area of View Sector";
    aovDesc.category = EntityCategory::AirZone;
    aovDesc.painterStrategyId = "polygon";
    aovDesc.defaultStyle.strokeColor = QColor("#10b981");
    aovDesc.defaultStyle.fillColor = QColor(16, 185, 129, 60);
    aovDesc.defaultStyle.strokeWidth = 2.0;
    aovDesc.propertySchemas = {
        {"start_angle", "Start Angle (°)", QMetaType::Double, 0.0, false, {}},
        {"end_angle", "End Angle (°)", QMetaType::Double, 90.0, false, {}},
        {"max_range_km", "Max Range (km)", QMetaType::Double, 50.0, false, {}},
        {"sensor_type", "Sensor Type", QMetaType::QString, "RADAR", false, {"RADAR", "EO/IR", "SONAR", "OPTICAL"}}
    };
    registerEntityType(aovDesc);

    // 3. Custom Tactical Marking Type
    EntityTypeDescriptor markingDesc;
    markingDesc.typeId = "tactical_marking";
    markingDesc.displayName = "Tactical Marking / Waypoint";
    markingDesc.category = EntityCategory::Waypoint;
    markingDesc.painterStrategyId = "icon";
    markingDesc.defaultStyle.strokeColor = QColor("#f59e0b");
    markingDesc.defaultStyle.fillColor = QColor(245, 158, 11, 80);
    markingDesc.defaultStyle.strokeWidth = 2.0;
    markingDesc.propertySchemas = {
        {"description", "Description", QMetaType::QString, "", false, {}},
        {"priority", "Priority", QMetaType::QString, "MEDIUM", false, {"LOW", "MEDIUM", "HIGH", "CRITICAL"}}
    };
    registerEntityType(markingDesc);
}

bool GisEntityRegistry::registerEntityType(const EntityTypeDescriptor &desc)
{
    if (desc.typeId.isEmpty()) {
        qWarning() << "[GisEntityRegistry] Cannot register entity type with empty typeId.";
        return false;
    }
    m_descriptors.insert(desc.typeId, desc);
    qDebug() << "[GisEntityRegistry] Registered Entity Type:" << desc.typeId << "(" << desc.displayName << ")";
    return true;
}

bool GisEntityRegistry::unregisterEntityType(const QString &typeId)
{
    return m_descriptors.remove(typeId) > 0;
}

bool GisEntityRegistry::isRegistered(const QString &typeId) const
{
    return m_descriptors.contains(typeId);
}

EntityTypeDescriptor GisEntityRegistry::descriptor(const QString &typeId) const
{
    return m_descriptors.value(typeId, EntityTypeDescriptor());
}

QList<EntityTypeDescriptor> GisEntityRegistry::registeredTypes() const
{
    return m_descriptors.values();
}

std::shared_ptr<GenericGisEntity> GisEntityRegistry::createEntity(
    const QString &typeId,
    const QString &name,
    const QString &id) const
{
    auto desc = descriptor(typeId);
    auto entity = std::make_shared<GenericGisEntity>(id, name.isEmpty() ? desc.displayName : name, typeId);
    entity->setCategory(desc.category);
    entity->setRenderStyle(desc.defaultStyle);

    // Apply default property schema values
    for (const auto &schema : desc.propertySchemas) {
        entity->setProperty(schema.key, schema.defaultValue);
    }

    return entity;
}

} // namespace GISApp::Core::Models
