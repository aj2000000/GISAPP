/**
 * @file IGisEntityRepository.h
 * @brief Abstract polymorphic repository interface for GIS entities.
 * Follows SOLID Dependency Inversion Principle (DIP) and Repository Pattern.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IGISENTITYREPOSITORY_H
#define IGISENTITYREPOSITORY_H

#include <QObject>
#include <QList>
#include <memory>
#include "GenericGisEntity.h"

namespace GISApp::Core::Repositories {

using GISApp::Core::Models::GenericGisEntity;
using GISApp::Core::Models::EntityCategory;

/**
 * @class IGisEntityRepository
 * @brief Abstract repository interface defining CRUD operations and reactive Qt signals for spatial entities.
 */
class IGisEntityRepository : public QObject {
    Q_OBJECT

public:
    explicit IGisEntityRepository(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IGisEntityRepository() override = default;

    // CRUD Operations
    virtual bool addEntity(std::shared_ptr<GenericGisEntity> entity) = 0;
    virtual bool updateEntity(std::shared_ptr<GenericGisEntity> entity) = 0;
    virtual bool removeEntity(const QString &entityId) = 0;
    virtual bool clearAll() = 0;

    // Query Methods
    virtual std::shared_ptr<GenericGisEntity> findById(const QString &entityId) const = 0;
    virtual QList<std::shared_ptr<GenericGisEntity>> findAll() const = 0;
    virtual QList<std::shared_ptr<GenericGisEntity>> findByType(const QString &typeId) const = 0;
    virtual QList<std::shared_ptr<GenericGisEntity>> findByCategory(EntityCategory category) const = 0;
    virtual int count() const = 0;

signals:
    void entityAdded(std::shared_ptr<GISApp::Core::Models::GenericGisEntity> entity);
    void entityUpdated(std::shared_ptr<GISApp::Core::Models::GenericGisEntity> entity);
    void entityRemoved(const QString &entityId);
    void repositoryCleared();
};

} // namespace GISApp::Core::Repositories

#endif // IGISENTITYREPOSITORY_H
