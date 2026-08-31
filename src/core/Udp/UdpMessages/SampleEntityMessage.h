/**
 * @file SampleEntityMessage.h
 * @brief Concrete message handler for batch SampleEntity telemetry packets.
 */

#ifndef SAMPLEENTITYMESSAGE_H
#define SAMPLEENTITYMESSAGE_H

#include "core/interfaces/CUSTOM_MESSAGE.h"
#include "core/models/SampleEntity.h"
#include "MessageId.h"
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

#pragma pack(push, 1)
struct SampleEntityMessageHeader {
    int32_t numberOfEntity = 0;
};
#pragma pack(pop)

namespace GISApp::Core::Services {

class SampleEntityMessage : public CUSTOM_MESSAGE {
    Q_OBJECT

public:
    SampleEntityMessageHeader msgHeader;
    QVector<SampleEntityStruct> entityList;
    GISApp::Core::Models::SampleEntity entityHelper;

    explicit SampleEntityMessage(const QString &tableName = "sample_telemetry",
                                 const QString &layerName = "Telemetry Layer",
                                 const QString &layerGroup = "Sensors",
                                 QObject *parent = nullptr)
        : CUSTOM_MESSAGE(parent), entityHelper(tableName, layerName, layerGroup) {}

    uint32_t messageId() const override { return SAMPLE_ENTITY_MSG_ID; }

    bool parseAndSaveToDb(const QByteArray &payload) override {
        if (payload.size() < static_cast<int>(sizeof(SampleEntityMessageHeader))) return false;

        std::memcpy(&msgHeader, payload.constData(), sizeof(SampleEntityMessageHeader));
        qDebug() << "[SampleEntityMessage] 🔍 Ingested numberOfEntity:" << msgHeader.numberOfEntity << "| Body payload size:" << payload.size();
        entityList.clear();

        int offset = sizeof(SampleEntityMessageHeader);
        int expectedSize = offset + (msgHeader.numberOfEntity * sizeof(SampleEntityStruct));

        if (payload.size() < expectedSize) {
            qWarning() << "[SampleEntityMessage] Truncated payload size:" << payload.size() << "expected:" << expectedSize;
            return false;
        }

        qDebug() << "[SampleEntityMessage] 📥 Successfully parsing batch of" << msgHeader.numberOfEntity << "entities.";

        auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
        if (!db.isOpen()) return false;

        entityHelper.ensureTableCreated();
        db.transaction();

        QSqlQuery query(db);
        query.prepare(QString(R"(
            INSERT INTO %1 (id, name, latitude, longitude, altitude)
            VALUES (:id, :name, :lat, :lon, :alt)
            ON CONFLICT(id) DO UPDATE SET
                name=excluded.name, latitude=excluded.latitude,
                longitude=excluded.longitude, altitude=excluded.altitude;
        )").arg(entityHelper.tableName()));

        for (int i = 0; i < msgHeader.numberOfEntity; ++i) {
            if (offset + static_cast<int>(sizeof(SampleEntityStruct)) > payload.size()) {
                qWarning() << "[SampleEntityMessage] ⚠️ Offset bounds overrun at index:" << i;
                break;
            }
            QByteArray chunk = payload.mid(offset, sizeof(SampleEntityStruct));
            if (entityHelper.extractPayload(chunk)) {
                entityList.append(entityHelper.entityData);

                QVariantMap map = entityHelper.constructMap();
                query.bindValue(":id", map["id"]);
                query.bindValue(":name", map["name"]);
                query.bindValue(":lat", map["latitude"]);
                query.bindValue(":lon", map["longitude"]);
                query.bindValue(":alt", map["altitude"]);
                query.exec();
            }
            offset += sizeof(SampleEntityStruct);
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

#endif // SAMPLEENTITYMESSAGE_H
