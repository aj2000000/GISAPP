/**
 * @file CUSTOM_ENTITY.h
 * @brief Abstract base class for telemetry and custom GIS entities.
 */

#ifndef CUSTOM_ENTITY_H
#define CUSTOM_ENTITY_H

#include <QString>
#include <QVariantMap>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "core/database/DatabaseManager.h"

namespace GISApp::Core::Models {

/**
 * @class CUSTOM_ENTITY
 * @brief Abstract interface defining telemetry entity mapping, binary payload extraction, and dynamic table creation.
 */
class CUSTOM_ENTITY {
public:
    virtual ~CUSTOM_ENTITY() = default;

    // Pure Virtual Properties
    virtual QString tableName() const = 0;
    virtual QString layerName() const = 0;
    virtual QString layerGroup() const = 0;

    // Binary byte-level extraction into memory structure
    virtual bool extractPayload(const QByteArray &payload) = 0;

    // Key-Value map for database insertion and GeoJSON feature properties
    virtual QVariantMap constructMap() const = 0;

    // Construct single GeoJSON Feature object for MapLibre plotting
    virtual QJsonObject constructGeoJsonFeature() const = 0;

    // Construct full GeoJSON FeatureCollection object
    virtual QJsonObject constructGeoJsonFeatureCollection(const QJsonArray &features) const {
        QJsonObject collection;
        collection["type"] = "FeatureCollection";
        collection["features"] = features;
        return collection;
    }

    // Default GeoJSON backing file path on disk
    virtual QString getGeoJsonFilePath() const {
        return QString("/home/aman/MAPDATA/udl_layers/%1.geojson").arg(tableName());
    }

    // SQL CREATE TABLE statement specific to entity schema
    virtual QString createTableSql() const = 0;

    // Template Method: Auto-creates database table in SQLite if not existing
    virtual bool ensureTableCreated() {
        auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
        if (!db.isOpen()) {
            qCritical() << "[CUSTOM_ENTITY] Database connection is closed!";
            return false;
        }

        QSqlQuery query(db);
        if (!query.exec(createTableSql())) {
            qCritical() << "[CUSTOM_ENTITY] Failed to auto-create table" << tableName() << ":" << query.lastError().text();
            return false;
        }
        return true;
    }
};

} // namespace GISApp::Core::Models

#endif // CUSTOM_ENTITY_H
