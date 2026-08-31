/**
 * @file SensorEntity.h
 * @brief Concrete implementation of CUSTOM_ENTITY for Sensor telemetry.
 */

#ifndef SENSOR_ENTITY_H
#define SENSOR_ENTITY_H

#include "core/interfaces/CUSTOM_ENTITY.h"
#include "publishing/LayerRegistryManager.h"
#include <cstring>
#include <cstdint>

#pragma pack(push, 1)

#ifndef STRUCT_LOCATION_DEFINED
#define STRUCT_LOCATION_DEFINED
struct STRUCT_LOCATION_DATA {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
};
#endif

#ifndef SENSOR_DEPLOYED_TIME_DEFINED
#define SENSOR_DEPLOYED_TIME_DEFINED
struct SensorDeployedTime {
    uint8_t day = 0;
    uint8_t month = 0;
    uint16_t year = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
};
#endif

struct SensorStruct {
    int32_t id = 0;
    char name[100] = {0};
    uint8_t sensorType = 0;
    int32_t sensorHeight = 0;
    int32_t sensorCoverage = 0;
    int32_t sensorAxis = 0;
    int32_t sensorBearing = 0;
    int32_t source = 0;
    STRUCT_LOCATION_DATA loc;
    SensorDeployedTime deployedTime;
};
#pragma pack(pop)

namespace GISApp::Core::Models {

class SensorEntity : public CUSTOM_ENTITY {
public:
    SensorStruct entityData;
    QString m_tableName;
    QString m_layerName;
    QString m_layerGroup;

    SensorEntity(const QString &tableName = "sensor_telemetry",
                 const QString &layerName = "sensor",
                 const QString &layerGroup = "SensorGroup")
        : m_tableName(tableName), m_layerName(layerName), m_layerGroup(layerGroup)
    {
        // Register group in Layer Tree hierarchy
        GISApp::Publishing::LayerRegistryManager::instance().registerGroup(m_layerGroup);

        // Auto-create SQLite table if not existing
        ensureTableCreated();
    }

    QString tableName() const override { return m_tableName; }
    QString layerName() const override { return m_layerName; }
    QString layerGroup() const override { return m_layerGroup; }

    bool extractPayload(const QByteArray &payload) override {
        if (payload.size() < static_cast<int>(sizeof(SensorStruct))) {
            qWarning() << "[SensorEntity] Payload too small! Recv:" << payload.size() << "Expected:" << sizeof(SensorStruct);
            return false;
        }
        std::memcpy(&entityData, payload.constData(), sizeof(SensorStruct));
        return true;
    }

    QVariantMap constructMap() const override {
        QVariantMap map;
        map["id"] = entityData.id;
        map["name"] = QString::fromUtf8(entityData.name, qstrnlen(entityData.name, 100));
        map["sensorType"] = entityData.sensorType;
        map["sensorHeight"] = entityData.sensorHeight;
        map["sensorCoverage"] = entityData.sensorCoverage;
        map["sensorAxis"] = entityData.sensorAxis;
        map["sensorBearing"] = entityData.sensorBearing;
        map["source"] = entityData.source;
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
        properties["sensorType"] = entityData.sensorType;
        properties["sensorHeight"] = entityData.sensorHeight;
        properties["sensorCoverage"] = entityData.sensorCoverage;
        properties["sensorBearing"] = entityData.sensorBearing;
        properties["altitude"] = entityData.loc.altitude;
        properties["fillColor"] = "#FFFF00";
        properties["strokeColor"] = "#FFFF00";
        properties["pointRadius"] = 7.0;

        feature["geometry"] = geometry;
        feature["properties"] = properties;
        return feature;
    }

    QString createTableSql() const override {
        return QString(R"(
            CREATE TABLE IF NOT EXISTS %1 (
                id INTEGER PRIMARY KEY,
                name TEXT,
                sensor_type INTEGER,
                sensor_height INTEGER,
                sensor_coverage INTEGER,
                sensor_axis INTEGER,
                sensor_bearing INTEGER,
                source INTEGER,
                latitude REAL,
                longitude REAL,
                altitude REAL,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )").arg(m_tableName);
    }
};

} // namespace GISApp::Core::Models

#endif // SENSOR_ENTITY_H
