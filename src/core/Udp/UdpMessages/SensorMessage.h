/**
 * @file SensorMessage.h
 * @brief Message parser and SQLite batch saver for Sensor telemetry (Msg ID: 902).
 */

#ifndef SENSOR_MESSAGE_H
#define SENSOR_MESSAGE_H

#include "core/interfaces/CUSTOM_MESSAGE.h"
#include "core/models/SensorEntity.h"
#include "MessageId.h"
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

#pragma pack(push, 1)
struct SensorMessageHeader {
    int32_t numberOfEntity = 0;
};
#pragma pack(pop)

namespace GISApp::Core::Services {

class SensorMessage : public CUSTOM_MESSAGE {
    Q_OBJECT

public:
    SensorMessageHeader msgHeader;
    QVector<SensorStruct> entityList;
    GISApp::Core::Models::SensorEntity entityHelper;

    explicit SensorMessage(const QString &tableName = "sensor_telemetry",
                           const QString &layerName = "sensor",
                           const QString &layerGroup = "SensorGroup",
                           QObject *parent = nullptr)
        : CUSTOM_MESSAGE(parent), entityHelper(tableName, layerName, layerGroup) {}

    uint32_t messageId() const override { return SENSOR_MSG_ID; }

    bool parseAndSaveToDb(const QByteArray &payload) override {
        if (payload.size() < static_cast<int>(sizeof(SensorMessageHeader))) return false;

        std::memcpy(&msgHeader, payload.constData(), sizeof(SensorMessageHeader));
        entityList.clear();

        int offset = sizeof(SensorMessageHeader);
        int expectedSize = offset + (msgHeader.numberOfEntity * sizeof(SensorStruct));

        qDebug() << "[SensorMessage] 🔍 recv numberOfEntity:" << msgHeader.numberOfEntity
                 << "| sizeof(SensorStruct):" << sizeof(SensorStruct)
                 << "| payload size:" << payload.size()
                 << "| expectedSize:" << expectedSize;

        if (payload.size() < expectedSize) {
            qWarning() << "[SensorMessage] Truncated payload size:" << payload.size() << "expected:" << expectedSize;
            return false;
        }

        auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
        if (!db.isOpen()) return false;

        entityHelper.ensureTableCreated();
        db.transaction();

        QSqlQuery query(db);
        query.prepare(QString(R"(
            INSERT INTO %1 (id, name, sensor_type, sensor_height, sensor_coverage, sensor_axis, sensor_bearing, source, latitude, longitude, altitude)
            VALUES (:id, :name, :stype, :sheight, :scov, :saxis, :sbearing, :source, :lat, :lon, :alt)
            ON CONFLICT(id) DO UPDATE SET
                name=excluded.name, sensor_type=excluded.sensor_type, sensor_height=excluded.sensor_height,
                sensor_coverage=excluded.sensor_coverage, sensor_axis=excluded.sensor_axis, sensor_bearing=excluded.sensor_bearing,
                source=excluded.source, latitude=excluded.latitude, longitude=excluded.longitude, altitude=excluded.altitude;
        )").arg(entityHelper.tableName()));

        for (int i = 0; i < msgHeader.numberOfEntity; ++i) {
            QByteArray chunk = payload.mid(offset, sizeof(SensorStruct));
            if (entityHelper.extractPayload(chunk)) {
                entityList.append(entityHelper.entityData);

                QVariantMap map = entityHelper.constructMap();
                query.bindValue(":id", map["id"]);
                query.bindValue(":name", map["name"]);
                query.bindValue(":stype", map["sensorType"]);
                query.bindValue(":sheight", map["sensorHeight"]);
                query.bindValue(":scov", map["sensorCoverage"]);
                query.bindValue(":saxis", map["sensorAxis"]);
                query.bindValue(":sbearing", map["sensorBearing"]);
                query.bindValue(":source", map["source"]);
                query.bindValue(":lat", map["latitude"]);
                query.bindValue(":lon", map["longitude"]);
                query.bindValue(":alt", map["altitude"]);
                if (!query.exec()) {
                    qWarning() << "[SensorMessage] SQL Insert failed for entity ID" << map["id"] << ":" << query.lastError().text();
                }
            }
            offset += sizeof(SensorStruct);
        }

        db.commit();

        updateTable();
        updateLayer();

        emit messageProcessed(msgHeader.numberOfEntity);
        return true;
    }

    bool updateLayer() override {
        QJsonArray features;
        for (const auto &item : entityList) {
            entityHelper.entityData = item;
            features.append(entityHelper.constructGeoJsonFeature());
        }

        QJsonObject featureCollection = entityHelper.constructGeoJsonFeatureCollection(features);
        QString geoJsonPath = entityHelper.getGeoJsonFilePath();

        QFile file(geoJsonPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(featureCollection).toJson());
            file.close();
            emit layerUpdated(entityHelper.layerName(), geoJsonPath);
            return true;
        }
        return false;
    }

    bool updateTable() override {
        emit tableDataUpdated(entityHelper.tableName());
        return true;
    }
};

} // namespace GISApp::Core::Services

#endif // SENSOR_MESSAGE_H
