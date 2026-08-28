/**
 * @file GisEntityRegistry.h
 * @brief Singleton Registry for registering and creating GIS entity types.
 * Follows Abstract Factory and Registry patterns for SOLID Open-Closed Principle compliance.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef GISENTITYREGISTRY_H
#define GISENTITYREGISTRY_H

#include "GenericGisEntity.h"
#include <QMap>
#include <QList>
#include <QString>

namespace GISApp::Core::Models {

/**
 * @struct EntityPropertySchema
 * @brief Attribute specification for dynamic entity editor auto-generation.
 */
struct EntityPropertySchema {
    QString key;
    QString label;
    QMetaType::Type type{QMetaType::QString};
    QVariant defaultValue;
    bool isRequired{false};
    QStringList enumOptions; // For dropdown / combo choices
};

/**
 * @struct EntityTypeDescriptor
 * @brief Metadata definition for registering a new GIS entity type.
 */
struct EntityTypeDescriptor {
    QString typeId;                             // Unique identifier e.g. "track", "area_of_view"
    QString displayName;                        // Human readable title e.g. "Tactical Track"
    EntityCategory category{EntityCategory::Custom};
    EntityRenderStyle defaultStyle;
    QList<EntityPropertySchema> propertySchemas;
    QString painterStrategyId{"default"};
};

/**
 * @class GisEntityRegistry
 * @brief Central Registry for managing GIS Entity metadata and instantiation factories.
 */
class GisEntityRegistry {
public:
    static GisEntityRegistry& instance();

    bool registerEntityType(const EntityTypeDescriptor &desc);
    bool unregisterEntityType(const QString &typeId);
    bool isRegistered(const QString &typeId) const;

    EntityTypeDescriptor descriptor(const QString &typeId) const;
    QList<EntityTypeDescriptor> registeredTypes() const;

    std::shared_ptr<GenericGisEntity> createEntity(
        const QString &typeId,
        const QString &name = QString(),
        const QString &id = QString()
    ) const;

private:
    GisEntityRegistry();
    ~GisEntityRegistry() = default;

    GisEntityRegistry(const GisEntityRegistry&) = delete;
    GisEntityRegistry& operator=(const GisEntityRegistry&) = delete;

    void registerBuiltInTypes();

    QMap<QString, EntityTypeDescriptor> m_descriptors;
};

} // namespace GISApp::Core::Models

#endif // GISENTITYREGISTRY_H
