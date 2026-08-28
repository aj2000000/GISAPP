#include "AreaOfViewRepository.h"
#include "../database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace GISApp::Core::Repositories {

AreaOfViewRepository::AreaOfViewRepository(QObject *parent)
    : IAreaOfViewRepository(parent)
{
}

static QString serializePoints(const QVector<Models::Coordinate3D> &points)
{
    QJsonArray arr;
    for (const auto &pt : points) {
        QJsonObject obj;
        obj["lat"] = pt.latitude;
        obj["lon"] = pt.longitude;
        obj["height"] = pt.altitude;
        arr.append(obj);
    }
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

static QVector<Models::Coordinate3D> deserializePoints(const QString &jsonStr)
{
    QVector<Models::Coordinate3D> points;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const auto &val : arr) {
            if (val.isObject()) {
                QJsonObject obj = val.toObject();
                double lat = obj.value("lat").toDouble();
                double lon = obj.value("lon").toDouble();
                double height = obj.value("height").toDouble();
                points.append(Models::Coordinate3D(lat, lon, height));
            } else if (val.isArray()) {
                QJsonArray ptArr = val.toArray();
                if (ptArr.size() >= 2) {
                    double lon = ptArr.at(0).toDouble();
                    double lat = ptArr.at(1).toDouble();
                    double height = (ptArr.size() >= 3) ? ptArr.at(2).toDouble() : 0.0;
                    points.append(Models::Coordinate3D(lat, lon, height));
                }
            }
        }
    }
    return points;
}

bool AreaOfViewRepository::insertOrUpdateInternal(const Models::AreaOfViewRecord &record)
{
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    QSqlQuery query(db);

    if (record.id > 0) {
        query.prepare(R"(
            UPDATE AREA_OF_VIEW SET
                NAME = :name,
                N_POINTS = :n_points,
                POINTS_JSON = :points_json
            WHERE ID = :id
        )");
        query.bindValue(":id", record.id);
    } else {
        query.prepare(R"(
            INSERT INTO AREA_OF_VIEW (NAME, N_POINTS, POINTS_JSON)
            VALUES (:name, :n_points, :points_json)
        )");
    }

    query.bindValue(":name", record.name);
    query.bindValue(":n_points", record.nPoints);
    query.bindValue(":points_json", serializePoints(record.points));

    if (!query.exec()) {
        qCritical() << "[AreaOfViewRepository] Insert/Update error:" << query.lastError().text();
        return false;
    }

    return true;
}

bool AreaOfViewRepository::insertOrUpdate(const Models::AreaOfViewRecord &record)
{
    if (record.id <= 0) {
        clearAll();
    }
    if (!insertOrUpdateInternal(record)) {
        return false;
    }
    emit areaOfViewUpdated();
    return true;
}

bool AreaOfViewRepository::insertBatch(const QVector<Models::AreaOfViewRecord> &records)
{
    clearAll();
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    if (!db.transaction()) {
        qWarning() << "[AreaOfViewRepository] Failed to start transaction";
    }

    for (const auto &rec : records) {
        if (!insertOrUpdateInternal(rec)) {
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qWarning() << "[AreaOfViewRepository] Failed to commit transaction";
    }

    emit areaOfViewUpdated();
    return true;
}

QVector<Models::AreaOfViewRecord> AreaOfViewRepository::getAll() const
{
    QVector<Models::AreaOfViewRecord> result;
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    QSqlQuery query("SELECT ID, NAME, N_POINTS, POINTS_JSON FROM AREA_OF_VIEW", db);

    while (query.next()) {
        Models::AreaOfViewRecord rec;
        rec.id = query.value("ID").toInt();
        rec.name = query.value("NAME").toString();
        rec.nPoints = query.value("N_POINTS").toInt();
        rec.points = deserializePoints(query.value("POINTS_JSON").toString());
        result.append(rec);
    }

    return result;
}

std::optional<Models::AreaOfViewRecord> AreaOfViewRepository::getById(int id) const
{
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    QSqlQuery query(db);
    query.prepare("SELECT ID, NAME, N_POINTS, POINTS_JSON FROM AREA_OF_VIEW WHERE ID = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Models::AreaOfViewRecord rec;
        rec.id = query.value("ID").toInt();
        rec.name = query.value("NAME").toString();
        rec.nPoints = query.value("N_POINTS").toInt();
        rec.points = deserializePoints(query.value("POINTS_JSON").toString());
        return rec;
    }

    return std::nullopt;
}

bool AreaOfViewRepository::deleteById(int id)
{
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM AREA_OF_VIEW WHERE ID = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qCritical() << "[AreaOfViewRepository] Delete error:" << query.lastError().text();
        return false;
    }

    emit areaOfViewUpdated();
    return true;
}

bool AreaOfViewRepository::clearAll()
{
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    QSqlQuery query("DELETE FROM AREA_OF_VIEW", db);
    if (!query.exec()) {
        return false;
    }
    emit areaOfViewUpdated();
    return true;
}

int AreaOfViewRepository::count() const
{
    QSqlDatabase db = Database::DatabaseManager::instance()->database();
    QSqlQuery query("SELECT COUNT(*) FROM AREA_OF_VIEW", db);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

} // namespace GISApp::Core::Repositories
