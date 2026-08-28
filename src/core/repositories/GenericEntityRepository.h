/**
 * @file GenericEntityRepository.h
 * @brief Thread-safe SQLite-backed implementation of IGisEntityRepository.
 * Supports any GIS entity type without custom SQL table creation per entity.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef GENERICENTITYREPOSITORY_H
#define GENERICENTITYREPOSITORY_H

#include "IGisEntityRepository.h"
#include <QMap>
#include <QReadWriteLock>

namespace GISApp::Core::Repositories {

/**
 * @class GenericEntityRepository
 * @brief Concrete repository for polymorphic spatial entities with reactive signal dispatching.
 */
class GenericEntityRepository : public IGisEntityRepository {
    Q_OBJECT

public:
    explicit GenericEntityRepository(QObject *parent = nullptr);
    virtual ~GenericEntityRepository() override = default;

    // IGisEntityRepository Implementation
    bool addEntity(std::shared_ptr<GenericGisEntity> entity) override;
    bool updateEntity(std::shared_ptr<GenericGisEntity> entity) override;
    bool removeEntity(const QString &entityId) override;
    bool clearAll() override;

    std::shared_ptr<GenericGisEntity> findById(const QString &entityId) const override;
    QList<std::shared_ptr<GenericGisEntity>> findAll() const override;
    QList<std::shared_ptr<GenericGisEntity>> findByType(const QString &typeId) const override;
    QList<std::shared_ptr<GenericGisEntity>> findByCategory(EntityCategory category) const override;
    int count() const override;

    // Batch Operations
    int addBatch(const QList<std::shared_ptr<GenericGisEntity>> &entities);

private:
    void ensureTableExists();
    bool saveToDb(const std::shared_ptr<GenericGisEntity> &entity);
    bool removeFromDb(const QString &entityId);
    void loadFromDb();

    mutable QReadWriteLock m_lock;
    QMap<QString, std::shared_ptr<GenericGisEntity>> m_cache;
};

} // namespace GISApp::Core::Repositories

#endif // GENERICENTITYREPOSITORY_H
