/**
 * @file ITrackRepository.h
 * @brief Abstract Data Access Object (DAO) interface for Track Entities.
 * Following Repository pattern and SOLID Dependency Inversion principle.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef ITRACKREPOSITORY_H
#define ITRACKREPOSITORY_H

#include "../models/TrackRecord.h"
#include <QVector>
#include <QObject>

namespace GISApp::Core::Repositories {

class ITrackRepository : public QObject {
    Q_OBJECT
public:
    explicit ITrackRepository(QObject *parent = nullptr) : QObject(parent) {}
    ~ITrackRepository() override = default;

    virtual bool insertOrUpdateTrack(const Models::TrackRecord &track) = 0;
    virtual bool insertBatch(const QVector<Models::TrackRecord> &tracks) = 0;
    virtual QVector<Models::TrackRecord> getAllTracks() const = 0;
    virtual std::optional<Models::TrackRecord> getTrackById(int trackId) const = 0;
    virtual bool deleteTrack(int trackId) = 0;
    virtual bool clearAllTracks() = 0;
    virtual int trackCount() const = 0;

signals:
    void tracksUpdated();
};

} // namespace GISApp::Core::Repositories

#endif // ITRACKREPOSITORY_H
