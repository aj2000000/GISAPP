/**
 * @file Table.h
 * @brief Storage and data management layer for ExpEntity objects (ExpTable).
 */

#ifndef EXP_TABLE_H
#define EXP_TABLE_H

#include "Entity.h"
#include "core/database/DatabaseManager.h"
#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QDebug>
#include <functional>

namespace GISApp::Experiment {

class ExpTable : public QObject {
    Q_OBJECT

private:
    QString m_tableName;
    QMap<int32_t, ExpEntity> m_entities;

public:
    explicit ExpTable(const QString &tableName = "exp_telemetry", QObject *parent = nullptr)
        : QObject(parent), m_tableName(tableName)
    {
        ensureTableCreated();
    }

    virtual ~ExpTable() = default;

    QString tableName() const { return m_tableName; }

    QString createTableSql() const {
        return QString(R"(
            CREATE TABLE IF NOT EXISTS %1 (
                id INTEGER PRIMARY KEY,
                name TEXT,
                latitude REAL,
                longitude REAL,
                altitude REAL,
                category TEXT,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )").arg(m_tableName);
    }

    bool ensureTableCreated() {
        auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
        if (!db.isOpen()) {
            qWarning() << "[ExpTable] Database not open for table:" << m_tableName;
            return false;
        }
        QSqlQuery query(db);
        if (!query.exec(createTableSql())) {
            qWarning() << "[ExpTable] Failed to create table:" << m_tableName << query.lastError().text();
            return false;
        }
        return true;
    }

    bool addOrUpdateEntity(const ExpEntity &entity) {
        m_entities[entity.id] = entity;

        auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare(QString(R"(
                INSERT INTO %1 (id, name, latitude, longitude, altitude, category)
                VALUES (:id, :name, :lat, :lon, :alt, :cat)
                ON CONFLICT(id) DO UPDATE SET
                    name=excluded.name,
                    latitude=excluded.latitude,
                    longitude=excluded.longitude,
                    altitude=excluded.altitude,
                    category=excluded.category,
                    updated_at=CURRENT_TIMESTAMP;
            )").arg(m_tableName));

            query.bindValue(":id", entity.id);
            query.bindValue(":name", entity.name);
            query.bindValue(":lat", entity.latitude);
            query.bindValue(":lon", entity.longitude);
            query.bindValue(":alt", entity.altitude);
            query.bindValue(":cat", entity.category);

            if (!query.exec()) {
                qWarning() << "[ExpTable] DB insert/update error:" << query.lastError().text();
            }
        }

        emit entityUpdated(entity.id);
        emit tableDataUpdated(m_tableName);
        return true;
    }

    bool addOrUpdateEntities(const QVector<ExpEntity> &entities) {
        auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
        bool useDb = db.isOpen();
        if (useDb) {
            db.transaction();
        }

        QSqlQuery query(db);
        if (useDb) {
            query.prepare(QString(R"(
                INSERT INTO %1 (id, name, latitude, longitude, altitude, category)
                VALUES (:id, :name, :lat, :lon, :alt, :cat)
                ON CONFLICT(id) DO UPDATE SET
                    name=excluded.name,
                    latitude=excluded.latitude,
                    longitude=excluded.longitude,
                    altitude=excluded.altitude,
                    category=excluded.category,
                    updated_at=CURRENT_TIMESTAMP;
            )").arg(m_tableName));
        }

        for (const auto &entity : entities) {
            m_entities[entity.id] = entity;
            if (useDb) {
                query.bindValue(":id", entity.id);
                query.bindValue(":name", entity.name);
                query.bindValue(":lat", entity.latitude);
                query.bindValue(":lon", entity.longitude);
                query.bindValue(":alt", entity.altitude);
                query.bindValue(":cat", entity.category);
                query.exec();
            }
        }

        if (useDb) {
            db.commit();
        }

        emit tableDataUpdated(m_tableName);
        return true;
    }

    ExpEntity getEntity(int32_t id) const {
        return m_entities.value(id, ExpEntity());
    }

    bool hasEntity(int32_t id) const {
        return m_entities.contains(id);
    }

    QVector<ExpEntity> getAllEntities() const {
        QVector<ExpEntity> result;
        result.reserve(m_entities.size());
        for (auto it = m_entities.constBegin(); it != m_entities.constEnd(); ++it) {
            result.append(it.value());
        }
        return result;
    }

    QVector<ExpEntity> filterEntities(std::function<bool(const ExpEntity&)> predicate) const {
        QVector<ExpEntity> filtered;
        for (auto it = m_entities.constBegin(); it != m_entities.constEnd(); ++it) {
            if (predicate(it.value())) {
                filtered.append(it.value());
            }
        }
        return filtered;
    }

    QVector<ExpEntity> filterByCategory(const QString &category) const {
        if (category.isEmpty() || category == "ALL" || category == "*") {
            return getAllEntities();
        }
        return filterEntities([category](const ExpEntity &e) {
            return e.category.compare(category, Qt::CaseInsensitive) == 0;
        });
    }

    QJsonObject constructGeoJsonFeatureCollection(const QVector<ExpEntity> &entitiesList) const {
        QJsonArray features;
        for (const auto &entity : entitiesList) {
            features.append(entity.toGeoJsonFeature());
        }
        QJsonObject collection;
        collection["type"] = "FeatureCollection";
        collection["features"] = features;
        return collection;
    }

    QJsonObject constructGeoJsonFeatureCollection() const {
        return constructGeoJsonFeatureCollection(getAllEntities());
    }

    void clear() {
        m_entities.clear();
        emit tableDataUpdated(m_tableName);
    }

signals:
    void entityUpdated(int32_t entityId);
    void tableDataUpdated(const QString &tableName);
};

} // namespace GISApp::Experiment

using ExpTable = GISApp::Experiment::ExpTable;

#endif // EXP_TABLE_H
