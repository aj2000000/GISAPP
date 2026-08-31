/**
 * @file Entity.h
 * @brief Representation of an individual entity (ExpEntity) for the GIS Experiment module.
 * @note ExpEntity does not derive from CUSTOM_ENTITY.
 */

#ifndef EXP_ENTITY_H
#define EXP_ENTITY_H

#include <QString>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <cstdint>
#include <cstring>

#pragma pack(push, 1)
#ifndef EXP_STRUCT_LOCATION_DEFINED
#define EXP_STRUCT_LOCATION_DEFINED
struct ExpLocationData {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
};
#endif

struct ExpEntityStruct {
    int32_t id = 0;
    char name[100] = {0};
    ExpLocationData loc;
};
#pragma pack(pop)

namespace GISApp::Experiment {

class ExpEntity {
public:
    int32_t id = 0;
    QString name;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    QString category = "General";
    QVariantMap properties;

    ExpEntity() = default;

    ExpEntity(int32_t entityId, const QString &entityName, double lat, double lon, double alt = 0.0, const QString &cat = "General")
        : id(entityId), name(entityName), latitude(lat), longitude(lon), altitude(alt), category(cat) {}

    explicit ExpEntity(const ExpEntityStruct &rawStruct, const QString &cat = "General")
        : id(rawStruct.id),
          name(QString::fromUtf8(rawStruct.name, qstrnlen(rawStruct.name, sizeof(rawStruct.name)))),
          latitude(rawStruct.loc.latitude),
          longitude(rawStruct.loc.longitude),
          altitude(rawStruct.loc.altitude),
          category(cat) {}

    ExpEntityStruct toRawStruct() const {
        ExpEntityStruct st;
        st.id = id;
        std::memset(st.name, 0, sizeof(st.name));
        QByteArray nameBytes = name.toUtf8();
        std::strncpy(st.name, nameBytes.constData(), sizeof(st.name) - 1);
        st.loc.latitude = latitude;
        st.loc.longitude = longitude;
        st.loc.altitude = altitude;
        return st;
    }

    QVariantMap toVariantMap() const {
        QVariantMap map;
        map["id"] = id;
        map["name"] = name;
        map["latitude"] = latitude;
        map["longitude"] = longitude;
        map["altitude"] = altitude;
        map["category"] = category;
        for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
            map[it.key()] = it.value();
        }
        return map;
    }

    QJsonObject toGeoJsonFeature() const {
        QJsonObject feature;
        feature["type"] = "Feature";

        QJsonObject geometry;
        geometry["type"] = "Point";
        geometry["coordinates"] = QJsonArray{longitude, latitude, altitude};

        QJsonObject props;
        props["id"] = id;
        props["name"] = name;
        props["altitude"] = altitude;
        props["category"] = category;
        for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
            props[it.key()] = QJsonValue::fromVariant(it.value());
        }

        feature["geometry"] = geometry;
        feature["properties"] = props;
        return feature;
    }

    static ExpEntity fromGeoJsonFeature(const QJsonObject &feature) {
        ExpEntity entity;
        if (feature.contains("properties") && feature["properties"].isObject()) {
            QJsonObject props = feature["properties"].toObject();
            entity.id = props.value("id").toInt();
            entity.name = props.value("name").toString();
            entity.altitude = props.value("altitude").toDouble();
            entity.category = props.value("category").toString("General");
        }
        if (feature.contains("geometry") && feature["geometry"].isObject()) {
            QJsonObject geom = feature["geometry"].toObject();
            if (geom.value("type").toString() == "Point" && geom.contains("coordinates")) {
                QJsonArray coords = geom["coordinates"].toArray();
                if (coords.size() >= 2) {
                    entity.longitude = coords.at(0).toDouble();
                    entity.latitude = coords.at(1).toDouble();
                }
                if (coords.size() >= 3) {
                    entity.altitude = coords.at(2).toDouble();
                }
            }
        }
        return entity;
    }
};

} // namespace GISApp::Experiment

using ExpEntity = GISApp::Experiment::ExpEntity;

#endif // EXP_ENTITY_H
