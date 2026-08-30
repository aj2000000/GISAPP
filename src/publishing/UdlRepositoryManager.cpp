/**
 * @file UdlRepositoryManager.cpp
 * @brief Implementation of UdlRepositoryManager.
 */

#include "src/publishing/UdlRepositoryManager.h"
#include "core/database/DatabaseManager.h"
#include "core/SystemConfigManager.h"
#include <QDateTime>

namespace GISApp::Publishing {

UdlRepositoryManager& UdlRepositoryManager::instance() {
    static UdlRepositoryManager inst;
    return inst;
}

UdlRepositoryManager::UdlRepositoryManager(QObject *parent)
    : QObject(parent)
{
    QDir dir(storageDirectoryPath());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString UdlRepositoryManager::storageDirectoryPath() const {
    return "/home/aman/MAPDATA/udl_layers";
}

QString UdlRepositoryManager::getUdlGeoJsonPath(const QString &layerId) {
    return QString("%1/%2.geojson").arg(storageDirectoryPath(), layerId);
}

bool UdlRepositoryManager::getEntity(const QString &entityId, UdlEntityItem &outItem) {
    auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("SELECT ENTITY_ID, LAYER_ID, ENTITY_NAME, ENTITY_TYPE, GEOMETRY_JSON, STYLE_JSON, CREATED_AT FROM UDL_ENTITIES WHERE ENTITY_ID = :id;");
    query.bindValue(":id", entityId);

    if (query.exec() && query.next()) {
        outItem.entityId = query.value(0).toString();
        outItem.layerId = query.value(1).toString();
        outItem.entityName = query.value(2).toString();
        outItem.entityType = query.value(3).toString();
        outItem.geometryJson = QJsonDocument::fromJson(query.value(4).toString().toUtf8()).object();
        outItem.styleJson = QJsonDocument::fromJson(query.value(5).toString().toUtf8()).object();
        outItem.createdAt = query.value(6).toString();
        return true;
    }
    return false;
}

bool UdlRepositoryManager::saveEntity(const UdlEntityItem &item, bool recordUndo) {
    UdlEntityItem existingItem;
    bool exists = getEntity(item.entityId, existingItem);

    auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) {
        qCritical() << "[UdlRepositoryManager] Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO UDL_ENTITIES (ENTITY_ID, LAYER_ID, ENTITY_NAME, ENTITY_TYPE, GEOMETRY_JSON, STYLE_JSON, CREATED_AT)
        VALUES (:id, :lid, :name, :type, :geom, :style, :time)
        ON CONFLICT(ENTITY_ID) DO UPDATE SET
            ENTITY_NAME = excluded.ENTITY_NAME,
            ENTITY_TYPE = excluded.ENTITY_TYPE,
            GEOMETRY_JSON = excluded.GEOMETRY_JSON,
            STYLE_JSON = excluded.STYLE_JSON;
    )");

    query.bindValue(":id", item.entityId);
    query.bindValue(":lid", item.layerId);
    query.bindValue(":name", item.entityName);
    query.bindValue(":type", item.entityType);
    query.bindValue(":geom", QString(QJsonDocument(item.geometryJson).toJson(QJsonDocument::Compact)));
    query.bindValue(":style", QString(QJsonDocument(item.styleJson).toJson(QJsonDocument::Compact)));
    query.bindValue(":time", item.createdAt.isEmpty() ? QDateTime::currentDateTime().toString(Qt::ISODate) : item.createdAt);

    if (!query.exec()) {
        qCritical() << "[UdlRepositoryManager] Save entity query failed:" << query.lastError().text();
        return false;
    }

    if (recordUndo) {
        if (exists) {
            m_undoStack.append({UdlUndoType::Update, item, existingItem});
        } else {
            m_undoStack.append({UdlUndoType::Create, item, UdlEntityItem()});
        }
        emit undoStateChanged(!m_undoStack.isEmpty());
    }

    syncGeoJsonFile(item.layerId);
    return true;
}

bool UdlRepositoryManager::deleteEntity(const QString &entityId, const QString &layerId, bool recordUndo) {
    UdlEntityItem existingItem;
    bool found = getEntity(entityId, existingItem);

    auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("DELETE FROM UDL_ENTITIES WHERE ENTITY_ID = :id;");
    query.bindValue(":id", entityId);
    if (!query.exec()) {
        qCritical() << "[UdlRepositoryManager] Delete entity query failed:" << query.lastError().text();
        return false;
    }

    if (recordUndo && found) {
        m_undoStack.append({UdlUndoType::Delete, existingItem, UdlEntityItem()});
        emit undoStateChanged(!m_undoStack.isEmpty());
    }

    syncGeoJsonFile(layerId);
    return true;
}

bool UdlRepositoryManager::deleteLayer(const QString &layerId) {
    if (layerId.isEmpty()) return false;

    qWarning() << "[UdlRepositoryManager] Deleting UDL layer and all associated entities for layerId:" << layerId;

    // 1. Delete all entities belonging to this layerId from SQLite table UDL_ENTITIES
    auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("DELETE FROM UDL_ENTITIES WHERE LAYER_ID = :lid;");
        query.bindValue(":lid", layerId);
        if (!query.exec()) {
            qCritical() << "[UdlRepositoryManager] Delete layer entities query failed:" << query.lastError().text();
        } else {
            qDebug() << "[UdlRepositoryManager] Successfully deleted entities from SQLite for layer:" << layerId;
        }
    }

    // 2. Remove the GeoJSON file on disk if it exists
    QString filePath = getUdlGeoJsonPath(layerId);
    if (QFile::exists(filePath)) {
        if (QFile::remove(filePath)) {
            qDebug() << "[UdlRepositoryManager] Removed GeoJSON file from disk:" << filePath;
        } else {
            qWarning() << "[UdlRepositoryManager] Failed to remove GeoJSON file:" << filePath;
        }
    }

    // 3. Remove entries from m_undoStack related to this layerId
    for (int i = m_undoStack.size() - 1; i >= 0; --i) {
        if (m_undoStack[i].primary.layerId == layerId) {
            m_undoStack.removeAt(i);
        }
    }
    emit undoStateChanged(!m_undoStack.isEmpty());

    // 4. Emit signal so listeners update
    emit udlLayerUpdated(layerId, "");
    return true;
}

bool UdlRepositoryManager::undoLastAction() {
    if (m_undoStack.isEmpty()) return false;

    UdlUndoItem action = m_undoStack.takeLast();
    emit undoStateChanged(!m_undoStack.isEmpty());

    if (action.type == UdlUndoType::Create) {
        qDebug() << "[UdlRepositoryManager] Undoing creation of entity:" << action.primary.entityId;
        return deleteEntity(action.primary.entityId, action.primary.layerId, false);
    } else if (action.type == UdlUndoType::Delete) {
        qDebug() << "[UdlRepositoryManager] Undoing deletion of entity:" << action.primary.entityId;
        return saveEntity(action.primary, false);
    } else if (action.type == UdlUndoType::Update) {
        qDebug() << "[UdlRepositoryManager] Undoing update of entity:" << action.secondary.entityId;
        return saveEntity(action.secondary, false);
    }
    return false;
}

QList<UdlEntityItem> UdlRepositoryManager::getEntitiesForLayer(const QString &layerId) {
    QList<UdlEntityItem> list;
    auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) return list;

    QSqlQuery query(db);
    query.prepare("SELECT ENTITY_ID, LAYER_ID, ENTITY_NAME, ENTITY_TYPE, GEOMETRY_JSON, STYLE_JSON, CREATED_AT FROM UDL_ENTITIES WHERE LAYER_ID = :lid;");
    query.bindValue(":lid", layerId);

    if (query.exec()) {
        while (query.next()) {
            UdlEntityItem item;
            item.entityId = query.value(0).toString();
            item.layerId = query.value(1).toString();
            item.entityName = query.value(2).toString();
            item.entityType = query.value(3).toString();
            item.geometryJson = QJsonDocument::fromJson(query.value(4).toString().toUtf8()).object();
            item.styleJson = QJsonDocument::fromJson(query.value(5).toString().toUtf8()).object();
            item.createdAt = query.value(6).toString();
            list.append(item);
        }
    }
    return list;
}

QList<UdlEntityItem> UdlRepositoryManager::getAllEntities() {
    QList<UdlEntityItem> list;
    auto db = GISApp::Core::Database::DatabaseManager::instance()->database();
    if (!db.isOpen()) return list;

    QSqlQuery query(db);
    if (query.exec("SELECT ENTITY_ID, LAYER_ID, ENTITY_NAME, ENTITY_TYPE, GEOMETRY_JSON, STYLE_JSON, CREATED_AT FROM UDL_ENTITIES;")) {
        while (query.next()) {
            UdlEntityItem item;
            item.entityId = query.value(0).toString();
            item.layerId = query.value(1).toString();
            item.entityName = query.value(2).toString();
            item.entityType = query.value(3).toString();
            item.geometryJson = QJsonDocument::fromJson(query.value(4).toString().toUtf8()).object();
            item.styleJson = QJsonDocument::fromJson(query.value(5).toString().toUtf8()).object();
            item.createdAt = query.value(6).toString();
            list.append(item);
        }
    }
    return list;
}

bool UdlRepositoryManager::syncGeoJsonFile(const QString &layerId) {
    if (layerId.isEmpty()) return false;

    auto entities = getEntitiesForLayer(layerId);
    QJsonArray featuresArr;

    for (const auto &item : entities) {
        QJsonObject feature;
        feature["type"] = "Feature";

        QJsonObject properties = item.styleJson;
        properties["entityId"] = item.entityId;
        properties["layerId"] = item.layerId;
        properties["name"] = item.entityName;
        properties["entityType"] = item.entityType;
        if (item.entityType == "Text" && (!properties.contains("textContent") || properties["textContent"].toString().isEmpty())) {
            properties["textContent"] = item.entityName;
        }

        feature["properties"] = properties;
        feature["geometry"] = item.geometryJson;
        featuresArr.append(feature);
    }

    QJsonObject rootObj;
    rootObj["type"] = "FeatureCollection";
    rootObj["features"] = featuresArr;

    QString filePath = getUdlGeoJsonPath(layerId);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(rootObj).toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "[UdlRepositoryManager] Synced" << featuresArr.size() << "entities for layer" << layerId << "to" << filePath;
        emit udlLayerUpdated(layerId, filePath);
        return true;
    } else {
        qCritical() << "[UdlRepositoryManager] Failed to write GeoJSON file:" << filePath;
        return false;
    }
}

GISApp::Layers::LayerExtent UdlRepositoryManager::calculateLayerExtent(const QString &layerId) {
    auto entities = getEntitiesForLayer(layerId);
    if (entities.isEmpty()) {
        return GISApp::Layers::LayerExtent{
            GISApp::Core::Models::GeoCoordinate(8.0, 68.0),
            GISApp::Core::Models::GeoCoordinate(37.0, 97.0)
        };
    }

    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;
    bool foundPoint = false;

    auto processCoord = [&](double lon, double lat) {
        if (lat < -89.9 || lat > 89.9 || lon < -180.0 || lon > 180.0) return;
        minLat = std::min(minLat, lat);
        maxLat = std::max(maxLat, lat);
        minLon = std::min(minLon, lon);
        maxLon = std::max(maxLon, lon);
        foundPoint = true;
    };

    std::function<void(const QJsonValue&)> parseCoords = [&](const QJsonValue &val) {
        if (val.isArray()) {
            QJsonArray arr = val.toArray();
            if (arr.size() >= 2 && arr[0].isDouble() && arr[1].isDouble()) {
                processCoord(arr[0].toDouble(), arr[1].toDouble());
            } else {
                for (const auto &elem : arr) {
                    parseCoords(elem);
                }
            }
        }
    };

    for (const auto &ent : entities) {
        if (ent.geometryJson.contains("coordinates")) {
            parseCoords(ent.geometryJson["coordinates"]);
        }
    }

    if (!foundPoint) {
        return GISApp::Layers::LayerExtent{
            GISApp::Core::Models::GeoCoordinate(8.0, 68.0),
            GISApp::Core::Models::GeoCoordinate(37.0, 97.0)
        };
    }

    return GISApp::Layers::LayerExtent{
        GISApp::Core::Models::GeoCoordinate(minLat, minLon),
        GISApp::Core::Models::GeoCoordinate(maxLat, maxLon)
    };
}

} // namespace GISApp::Publishing
