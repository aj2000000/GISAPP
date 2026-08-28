#include "TrackRepository.h"
#include "../database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace GISApp::Core::Repositories {

TrackRepository::TrackRepository(QObject *parent)
    : ITrackRepository(parent)
{
}

QSqlDatabase TrackRepository::db() const
{
    return Database::DatabaseManager::instance()->database();
}

bool TrackRepository::insertOrUpdateTrack(const Models::TrackRecord &track)
{
    QSqlDatabase database = db();
    if (!database.isOpen()) {
        qWarning() << "[TrackRepository] Database connection is not open.";
        return false;
    }

    QSqlQuery query(database);
    query.prepare(R"(
        INSERT INTO TRACKS (
            TRACK_ID, TRACK_NAME, TRACK_PLOT_TYPE, INT_NO, TRACK_LAT, TRACK_LONG, TRACK_HEIGHT, TRACK_DIR,
            TRACK_IDENTITY, TRACK_TYPE, TRACK_SUB_TYPE, TRACK_CLASS, TRACK_STRENGTH,
            TRACK_ACT_TYPE, TRACK_ACT_SUB_TYPE, TRACK_ACT_CLASS, TRACK_SYSTEM_TYPE,
            TRACK_SOURCES, TRACK_IMAGE, TRACK_REMARKS
        ) VALUES (
            :id, :name, :plot_type, :int_no, :lat, :long, :height, :dir,
            :identity, :type, :sub_type, :class, :strength,
            :act_type, :act_sub_type, :act_class, :system_type,
            :sources, :image, :remarks
        )
        ON CONFLICT(TRACK_ID) DO UPDATE SET
            TRACK_NAME = excluded.TRACK_NAME,
            TRACK_PLOT_TYPE = excluded.TRACK_PLOT_TYPE,
            INT_NO = excluded.INT_NO,
            TRACK_LAT = excluded.TRACK_LAT,
            TRACK_LONG = excluded.TRACK_LONG,
            TRACK_HEIGHT = excluded.TRACK_HEIGHT,
            TRACK_DIR = excluded.TRACK_DIR,
            TRACK_IDENTITY = excluded.TRACK_IDENTITY,
            TRACK_TYPE = excluded.TRACK_TYPE,
            TRACK_SUB_TYPE = excluded.TRACK_SUB_TYPE,
            TRACK_CLASS = excluded.TRACK_CLASS,
            TRACK_STRENGTH = excluded.TRACK_STRENGTH,
            TRACK_ACT_TYPE = excluded.TRACK_ACT_TYPE,
            TRACK_ACT_SUB_TYPE = excluded.TRACK_ACT_SUB_TYPE,
            TRACK_ACT_CLASS = excluded.TRACK_ACT_CLASS,
            TRACK_SYSTEM_TYPE = excluded.TRACK_SYSTEM_TYPE,
            TRACK_SOURCES = excluded.TRACK_SOURCES,
            TRACK_IMAGE = excluded.TRACK_IMAGE,
            TRACK_REMARKS = excluded.TRACK_REMARKS;
    )");

    query.bindValue(":id", track.trackId);
    query.bindValue(":name", track.trackName);
    query.bindValue(":plot_type", track.trackPlotType);
    query.bindValue(":int_no", track.intNo);
    query.bindValue(":lat", track.trackLat);
    query.bindValue(":long", track.trackLong);
    query.bindValue(":height", track.trackHeight);
    query.bindValue(":dir", track.trackDir);
    query.bindValue(":identity", track.trackIdentity);
    query.bindValue(":type", track.trackType);
    query.bindValue(":sub_type", track.trackSubType);
    query.bindValue(":class", track.trackClass);
    query.bindValue(":strength", track.trackStrength);
    query.bindValue(":act_type", track.trackActType);
    query.bindValue(":act_sub_type", track.trackActSubType);
    query.bindValue(":act_class", track.trackActClass);
    query.bindValue(":system_type", track.trackSystemType);
    query.bindValue(":sources", track.trackSources);
    query.bindValue(":image", track.trackImage);
    query.bindValue(":remarks", track.trackRemarks);

    if (!query.exec()) {
        qCritical() << "[TrackRepository] Error inserting track:" << query.lastError().text();
        return false;
    }

    emit tracksUpdated();
    return true;
}

bool TrackRepository::insertBatch(const QVector<Models::TrackRecord> &tracks)
{
    if (tracks.isEmpty()) {
        return true;
    }

    QSqlDatabase database = db();
    if (!database.isOpen()) {
        qWarning() << "[TrackRepository] Database connection is not open for batch import.";
        return false;
    }

    database.transaction();

    QSqlQuery query(database);
    query.prepare(R"(
        INSERT INTO TRACKS (
            TRACK_ID, TRACK_NAME, TRACK_PLOT_TYPE, INT_NO, TRACK_LAT, TRACK_LONG, TRACK_HEIGHT, TRACK_DIR,
            TRACK_IDENTITY, TRACK_TYPE, TRACK_SUB_TYPE, TRACK_CLASS, TRACK_STRENGTH,
            TRACK_ACT_TYPE, TRACK_ACT_SUB_TYPE, TRACK_ACT_CLASS, TRACK_SYSTEM_TYPE,
            TRACK_SOURCES, TRACK_IMAGE, TRACK_REMARKS
        ) VALUES (
            :id, :name, :plot_type, :int_no, :lat, :long, :height, :dir,
            :identity, :type, :sub_type, :class, :strength,
            :act_type, :act_sub_type, :act_class, :system_type,
            :sources, :image, :remarks
        )
        ON CONFLICT(TRACK_ID) DO UPDATE SET
            TRACK_NAME = excluded.TRACK_NAME,
            TRACK_PLOT_TYPE = excluded.TRACK_PLOT_TYPE,
            INT_NO = excluded.INT_NO,
            TRACK_LAT = excluded.TRACK_LAT,
            TRACK_LONG = excluded.TRACK_LONG,
            TRACK_HEIGHT = excluded.TRACK_HEIGHT,
            TRACK_DIR = excluded.TRACK_DIR,
            TRACK_IDENTITY = excluded.TRACK_IDENTITY,
            TRACK_TYPE = excluded.TRACK_TYPE,
            TRACK_SUB_TYPE = excluded.TRACK_SUB_TYPE,
            TRACK_CLASS = excluded.TRACK_CLASS,
            TRACK_STRENGTH = excluded.TRACK_STRENGTH,
            TRACK_ACT_TYPE = excluded.TRACK_ACT_TYPE,
            TRACK_ACT_SUB_TYPE = excluded.TRACK_ACT_SUB_TYPE,
            TRACK_ACT_CLASS = excluded.TRACK_ACT_CLASS,
            TRACK_SYSTEM_TYPE = excluded.TRACK_SYSTEM_TYPE,
            TRACK_SOURCES = excluded.TRACK_SOURCES,
            TRACK_IMAGE = excluded.TRACK_IMAGE,
            TRACK_REMARKS = excluded.TRACK_REMARKS;
    )");

    for (const auto &track : tracks) {
        query.bindValue(":id", track.trackId);
        query.bindValue(":name", track.trackName);
        query.bindValue(":plot_type", track.trackPlotType);
        query.bindValue(":int_no", track.intNo);
        query.bindValue(":lat", track.trackLat);
        query.bindValue(":long", track.trackLong);
        query.bindValue(":height", track.trackHeight);
        query.bindValue(":dir", track.trackDir);
        query.bindValue(":identity", track.trackIdentity);
        query.bindValue(":type", track.trackType);
        query.bindValue(":sub_type", track.trackSubType);
        query.bindValue(":class", track.trackClass);
        query.bindValue(":strength", track.trackStrength);
        query.bindValue(":act_type", track.trackActType);
        query.bindValue(":act_sub_type", track.trackActSubType);
        query.bindValue(":act_class", track.trackActClass);
        query.bindValue(":system_type", track.trackSystemType);
        query.bindValue(":sources", track.trackSources);
        query.bindValue(":image", track.trackImage);
        query.bindValue(":remarks", track.trackRemarks);

        if (!query.exec()) {
            qCritical() << "[TrackRepository] Batch row error:" << query.lastError().text();
            database.rollback();
            return false;
        }
    }

    if (!database.commit()) {
        qCritical() << "[TrackRepository] Transaction commit failed:" << database.lastError().text();
        return false;
    }

    qDebug() << "[TrackRepository] Successfully committed batch of" << tracks.size() << "tracks.";
    emit tracksUpdated();
    return true;
}

QVector<Models::TrackRecord> TrackRepository::getAllTracks() const
{
    QVector<Models::TrackRecord> result;
    QSqlDatabase database = db();
    if (!database.isOpen()) {
        return result;
    }

    QSqlQuery query("SELECT TRACK_ID, TRACK_NAME, TRACK_PLOT_TYPE, INT_NO, TRACK_LAT, TRACK_LONG, TRACK_HEIGHT, TRACK_DIR, TRACK_IDENTITY, TRACK_TYPE, TRACK_SUB_TYPE, TRACK_CLASS, TRACK_STRENGTH, TRACK_ACT_TYPE, TRACK_ACT_SUB_TYPE, TRACK_ACT_CLASS, TRACK_SYSTEM_TYPE, TRACK_SOURCES, TRACK_IMAGE, TRACK_REMARKS FROM TRACKS ORDER BY TRACK_ID ASC", database);

    while (query.next()) {
        Models::TrackRecord track;
        track.trackId = query.value(0).toInt();
        track.trackName = query.value(1).toString();
        track.trackPlotType = query.value(2).toDouble();
        track.intNo = query.value(3).toInt();
        track.trackLat = query.value(4).toDouble();
        track.trackLong = query.value(5).toDouble();
        track.trackHeight = query.value(6).toDouble();
        track.trackDir = query.value(7).toDouble();
        track.trackIdentity = query.value(8).toInt();
        track.trackType = query.value(9).toInt();
        track.trackSubType = query.value(10).toInt();
        track.trackClass = query.value(11).toInt();
        track.trackStrength = query.value(12).toInt();
        track.trackActType = query.value(13).toInt();
        track.trackActSubType = query.value(14).toInt();
        track.trackActClass = query.value(15).toInt();
        track.trackSystemType = query.value(16).toInt();
        track.trackSources = query.value(17).toString();
        track.trackImage = query.value(18).toString();
        track.trackRemarks = query.value(19).toString();
        result.append(track);
    }

    return result;
}

std::optional<Models::TrackRecord> TrackRepository::getTrackById(int trackId) const
{
    QSqlDatabase database = db();
    if (!database.isOpen()) {
        return std::nullopt;
    }

    QSqlQuery query(database);
    query.prepare("SELECT TRACK_ID, TRACK_NAME, TRACK_PLOT_TYPE, INT_NO, TRACK_LAT, TRACK_LONG, TRACK_HEIGHT, TRACK_DIR, TRACK_IDENTITY, TRACK_TYPE, TRACK_SUB_TYPE, TRACK_CLASS, TRACK_STRENGTH, TRACK_ACT_TYPE, TRACK_ACT_SUB_TYPE, TRACK_ACT_CLASS, TRACK_SYSTEM_TYPE, TRACK_SOURCES, TRACK_IMAGE, TRACK_REMARKS FROM TRACKS WHERE TRACK_ID = :id");
    query.bindValue(":id", trackId);

    if (query.exec() && query.next()) {
        Models::TrackRecord track;
        track.trackId = query.value(0).toInt();
        track.trackName = query.value(1).toString();
        track.trackPlotType = query.value(2).toDouble();
        track.intNo = query.value(3).toInt();
        track.trackLat = query.value(4).toDouble();
        track.trackLong = query.value(5).toDouble();
        track.trackHeight = query.value(6).toDouble();
        track.trackDir = query.value(7).toDouble();
        track.trackIdentity = query.value(8).toInt();
        track.trackType = query.value(9).toInt();
        track.trackSubType = query.value(10).toInt();
        track.trackClass = query.value(11).toInt();
        track.trackStrength = query.value(12).toInt();
        track.trackActType = query.value(13).toInt();
        track.trackActSubType = query.value(14).toInt();
        track.trackActClass = query.value(15).toInt();
        track.trackSystemType = query.value(16).toInt();
        track.trackSources = query.value(17).toString();
        track.trackImage = query.value(18).toString();
        track.trackRemarks = query.value(19).toString();
        return track;
    }

    return std::nullopt;
}

bool TrackRepository::deleteTrack(int trackId)
{
    QSqlDatabase database = db();
    if (!database.isOpen()) {
        return false;
    }

    QSqlQuery query(database);
    query.prepare("DELETE FROM TRACKS WHERE TRACK_ID = :id");
    query.bindValue(":id", trackId);

    if (query.exec()) {
        emit tracksUpdated();
        return true;
    }
    return false;
}

bool TrackRepository::clearAllTracks()
{
    QSqlDatabase database = db();
    if (!database.isOpen()) {
        return false;
    }

    QSqlQuery query("DELETE FROM TRACKS", database);
    if (query.exec()) {
        emit tracksUpdated();
        return true;
    }
    return false;
}

int TrackRepository::trackCount() const
{
    QSqlDatabase database = db();
    if (!database.isOpen()) {
        return 0;
    }

    QSqlQuery query("SELECT COUNT(*) FROM TRACKS", database);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

} // namespace GISApp::Core::Repositories
