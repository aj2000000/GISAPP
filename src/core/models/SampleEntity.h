/**
 * @file SampleEntity.h
 * @brief Concrete implementation of CUSTOM_ENTITY for sample telemetry payloads.
 */

#ifndef SAMPLEENTITY_H
#define SAMPLEENTITY_H

#include "core/interfaces/CUSTOM_ENTITY.h"
#include "publishing/LayerRegistryManager.h"
#include <cstring>

#pragma pack(push, 1)

#ifndef STRUCT_LOCATION_DEFINED
#define STRUCT_LOCATION_DEFINED
struct STRUCT_LOCATION_DATA {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
};
#endif

struct SampleEntityStruct {
    int32_t id = 0;
    char name[100] = {0};
    STRUCT_LOCATION_DATA loc;
};
#pragma pack(pop)

namespace GISApp::Core::Models {

class SampleEntity : public CUSTOM_ENTITY {
public:
    SampleEntityStruct entityData;
    QString m_tableName;
    QString m_layerName;
    QString m_layerGroup;

    SampleEntity(const QString &tableName = "sample_telemetry",
                 const QString &layerName = "Telemetry Layer",
                 const QString &layerGroup = "Sensors")
        : m_tableName(tableName), m_layerName(layerName), m_layerGroup(layerGroup)
    {
        // Register layer group in Layer Tree hierarchy
        GISApp::Publishing::LayerRegistryManager::instance().registerGroup(m_layerGroup);

        // Auto-create database table if not existing
        ensureTableCreated();
    }

    QString tableName() const override { return m_tableName; }
    QString layerName() const override { return m_layerName; }
    QString layerGroup() const override { return m_layerGroup; }

    bool extractPayload(const QByteArray &payload) override {
        if (payload.size() < static_cast<int>(sizeof(SampleEntityStruct))) {
            qWarning() << "[SampleEntity] Payload smaller than struct size! Recv:" << payload.size() << "Expected:" << sizeof(SampleEntityStruct);
            return false;
        }
        std::memcpy(&entityData, payload.constData(), sizeof(SampleEntityStruct));
        return true;
    }

    QVariantMap constructMap() const override {
        QVariantMap map;
        map["id"] = entityData.id;
        map["name"] = QString::fromUtf8(entityData.name, qstrnlen(entityData.name, 100));
        map["latitude"] = entityData.loc.latitude;
        map["longitude"] = entityData.loc.longitude;
        map["altitude"] = entityData.loc.altitude;
        return map;
    }

    QJsonObject constructGeoJsonFeature() const override {
        QJsonObject feature;
        feature["type"] = "Feature";

        QJsonObject geometry;
        geometry["type"] = "Point";
        geometry["coordinates"] = QJsonArray{entityData.loc.longitude, entityData.loc.latitude};

        QJsonObject properties;
        properties["id"] = entityData.id;
        properties["name"] = QString::fromUtf8(entityData.name, qstrnlen(entityData.name, 100));
        properties["altitude"] = entityData.loc.altitude;

        feature["geometry"] = geometry;
        feature["properties"] = properties;
        return feature;
    }

    QString createTableSql() const override {
        return QString(R"(
            CREATE TABLE IF NOT EXISTS %1 (
                id INTEGER PRIMARY KEY,
                name TEXT,
                latitude REAL,
                longitude REAL,
                altitude REAL,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )").arg(m_tableName);
    }
};

} // namespace GISApp::Core::Models

#endif // SAMPLEENTITY_H
