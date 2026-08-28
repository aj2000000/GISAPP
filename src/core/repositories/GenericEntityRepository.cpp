/**
 * @file GenericEntityRepository.cpp
 * @brief Implementation of GenericEntityRepository.
 */

#include "GenericEntityRepository.h"
#include "DatabaseManager.h"
#include "IGisGeometry.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace GISApp::Core::Repositories {

using namespace GISApp::Core::Models;
using GISApp::Core::Database::DatabaseManager;

GenericEntityRepository::GenericEntityRepository(QObject *parent)
    : IGisEntityRepository(parent)
{
    ensureTableExists();
    loadFromDb();
}

void GenericEntityRepository::ensureTableExists()
{
    auto db = DatabaseManager::instance()->database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS generic_entities (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            type_id TEXT NOT NULL,
            category INTEGER NOT NULL,
            geometry_json TEXT,
            style_json TEXT,
            properties_json TEXT,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    if (!q.exec(sql)) {
        qWarning() << "[GenericEntityRepository] Failed to create generic_entities table:" << q.lastError().text();
    }
}

bool GenericEntityRepository::addEntity(std::shared_ptr<GenericGisEntity> entity)
{
    if (!entity || entity->entityId().isEmpty()) return false;

    {
        QWriteLocker locker(&m_lock);
        m_cache.insert(entity->entityId(), entity);
    }

    saveToDb(entity);
    emit entityAdded(entity);
    return true;
}

bool GenericEntityRepository::updateEntity(std::shared_ptr<GenericGisEntity> entity)
{
    if (!entity || entity->entityId().isEmpty()) return false;

    {
        QWriteLocker locker(&m_lock);
        m_cache.insert(entity->entityId(), entity);
    }

    saveToDb(entity);
    emit entityUpdated(entity);
    return true;
}

bool GenericEntityRepository::removeEntity(const QString &entityId)
{
    if (entityId.isEmpty()) return false;

    bool found = false;
    {
        QWriteLocker locker(&m_lock);
        found = m_cache.remove(entityId) > 0;
    }

    if (found) {
        removeFromDb(entityId);
        emit entityRemoved(entityId);
    }
    return found;
}

bool GenericEntityRepository::clearAll()
{
    {
        QWriteLocker locker(&m_lock);
        m_cache.clear();
    }

    auto db = DatabaseManager::instance()->database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.exec("DELETE FROM generic_entities;");
    }

    emit repositoryCleared();
    return true;
}

std::shared_ptr<GenericGisEntity> GenericEntityRepository::findById(const QString &entityId) const
{
    QReadLocker locker(&m_lock);
    return m_cache.value(entityId, nullptr);
}

QList<std::shared_ptr<GenericGisEntity>> GenericEntityRepository::findAll() const
{
    QReadLocker locker(&m_lock);
    return m_cache.values();
}

QList<std::shared_ptr<GenericGisEntity>> GenericEntityRepository::findByType(const QString &typeId) const
{
    QReadLocker locker(&m_lock);
    QList<std::shared_ptr<GenericGisEntity>> result;
    for (const auto &item : m_cache) {
        if (item && item->entityType() == typeId) {
            result.append(item);
        }
    }
    return result;
}

QList<std::shared_ptr<GenericGisEntity>> GenericEntityRepository::findByCategory(EntityCategory category) const
{
    QReadLocker locker(&m_lock);
    QList<std::shared_ptr<GenericGisEntity>> result;
    for (const auto &item : m_cache) {
        if (item && item->category() == category) {
            result.append(item);
        }
    }
    return result;
}

int GenericEntityRepository::count() const
{
    QReadLocker locker(&m_lock);
    return m_cache.size();
}

int GenericEntityRepository::addBatch(const QList<std::shared_ptr<GenericGisEntity>> &entities)
{
    int addedCount = 0;
    for (const auto &entity : entities) {
        if (addEntity(entity)) {
            addedCount++;
        }
    }
    return addedCount;
}

bool GenericEntityRepository::saveToDb(const std::shared_ptr<GenericGisEntity> &entity)
{
    auto db = DatabaseManager::instance()->database();
    if (!db.isOpen()) return false;

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT OR REPLACE INTO generic_entities 
        (id, name, type_id, category, geometry_json, style_json, properties_json, updated_at)
        VALUES (:id, :name, :type_id, :category, :geometry_json, :style_json, :properties_json, CURRENT_TIMESTAMP);
    )");

    q.bindValue(":id", entity->entityId());
    q.bindValue(":name", entity->entityName());
    q.bindValue(":type_id", entity->entityType());
    q.bindValue(":category", static_cast<int>(entity->category()));

    QJsonObject geomObj = entity->geometry() ? entity->geometry()->toGeoJsonGeometry() : QJsonObject();
    q.bindValue(":geometry_json", QString::fromUtf8(QJsonDocument(geomObj).toJson(QJsonDocument::Compact)));

    // Style serialization
    QJsonObject styleObj;
    auto style = entity->renderStyle();
    styleObj["stroke"] = style.strokeColor.name();
    styleObj["stroke_width"] = style.strokeWidth;
    styleObj["fill"] = style.fillColor.name();
    styleObj["heading"] = style.rotationHeading;
    styleObj["icon"] = style.iconPath;
    q.bindValue(":style_json", QString::fromUtf8(QJsonDocument(styleObj).toJson(QJsonDocument::Compact)));

    // Properties serialization
    QJsonObject propsObj;
    auto props = entity->properties();
    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
        propsObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    q.bindValue(":properties_json", QString::fromUtf8(QJsonDocument(propsObj).toJson(QJsonDocument::Compact)));

    return q.exec();
}

bool GenericEntityRepository::removeFromDb(const QString &entityId)
{
    auto db = DatabaseManager::instance()->database();
    if (!db.isOpen()) return false;

    QSqlQuery q(db);
    q.prepare("DELETE FROM generic_entities WHERE id = :id;");
    q.bindValue(":id", entityId);
    return q.exec();
}

void GenericEntityRepository::loadFromDb()
{
    auto db = DatabaseManager::instance()->database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT id, name, type_id, category, geometry_json, style_json, properties_json FROM generic_entities;", db);
    while (q.next()) {
        QString id = q.value(0).toString();
        QString name = q.value(1).toString();
        QString typeId = q.value(2).toString();
        EntityCategory cat = static_cast<EntityCategory>(q.value(3).toInt());
        QByteArray geomJson = q.value(4).toString().toUtf8();
        QByteArray styleJson = q.value(5).toString().toUtf8();
        QByteArray propsJson = q.value(6).toString().toUtf8();

        auto entity = std::make_shared<GenericGisEntity>(id, name, typeId);
        entity->setCategory(cat);

        // Deserialise Style
        QJsonObject sObj = QJsonDocument::fromJson(styleJson).object();
        EntityRenderStyle style;
        if (sObj.contains("stroke")) style.strokeColor = QColor(sObj["stroke"].toString());
        if (sObj.contains("stroke_width")) style.strokeWidth = sObj["stroke_width"].toDouble();
        if (sObj.contains("fill")) style.fillColor = QColor(sObj["fill"].toString());
        if (sObj.contains("heading")) style.rotationHeading = sObj["heading"].toDouble();
        if (sObj.contains("icon")) style.iconPath = sObj["icon"].toString();
        entity->setRenderStyle(style);

        // Deserialise Properties
        QJsonObject pObj = QJsonDocument::fromJson(propsJson).object();
        QVariantMap props;
        for (auto it = pObj.constBegin(); it != pObj.constEnd(); ++it) {
            props.insert(it.key(), it.value().toVariant());
        }
        entity->setProperties(props);

        // Parse Geometry (Point, Polyline, Polygon)
        QJsonObject gObj = QJsonDocument::fromJson(geomJson).object();
        QString gType = gObj["type"].toString();
        QJsonArray coords = gObj["coordinates"].toArray();

        if (gType == "Point" && coords.size() >= 2) {
            double lon = coords[0].toDouble();
            double lat = coords[1].toDouble();
            double alt = coords.size() > 2 ? coords[2].toDouble() : 0.0;
            entity->setGeometry(std::make_shared<PointGeometry>(lat, lon, alt));
        } else if (gType == "LineString") {
            QVector<Coordinate3D> polyCoords;
            for (const auto &val : coords) {
                QJsonArray c = val.toArray();
                if (c.size() >= 2) {
                    polyCoords.append(Coordinate3D(c[1].toDouble(), c[0].toDouble(), c.size() > 2 ? c[2].toDouble() : 0.0));
                }
            }
            entity->setGeometry(std::make_shared<PolylineGeometry>(polyCoords));
        } else if (gType == "Polygon" && !coords.isEmpty()) {
            QJsonArray outerRing = coords[0].toArray();
            QVector<Coordinate3D> polyCoords;
            for (const auto &val : outerRing) {
                QJsonArray c = val.toArray();
                if (c.size() >= 2) {
                    polyCoords.append(Coordinate3D(c[1].toDouble(), c[0].toDouble(), c.size() > 2 ? c[2].toDouble() : 0.0));
                }
            }
            entity->setGeometry(std::make_shared<PolygonGeometry>(polyCoords));
        }

        {
            QWriteLocker locker(&m_lock);
            m_cache.insert(entity->entityId(), entity);
        }
    }
}

} // namespace GISApp::Core::Repositories
