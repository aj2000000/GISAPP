/**
 * @file TrackRepository.h
 * @brief Concrete SQLite repository for Track entities with transaction-backed batch support.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef TRACKREPOSITORY_H
#define TRACKREPOSITORY_H

#include "ITrackRepository.h"
#include <QSqlDatabase>

namespace GISApp::Core::Repositories {

class TrackRepository : public ITrackRepository {
    Q_OBJECT
public:
    explicit TrackRepository(QObject *parent = nullptr);
    ~TrackRepository() override = default;

    bool insertOrUpdateTrack(const Models::TrackRecord &track) override;
    bool insertBatch(const QVector<Models::TrackRecord> &tracks) override;
    QVector<Models::TrackRecord> getAllTracks() const override;
    std::optional<Models::TrackRecord> getTrackById(int trackId) const override;
    bool deleteTrack(int trackId) override;
    bool clearAllTracks() override;
    int trackCount() const override;

private:
    QSqlDatabase db() const;
};

} // namespace GISApp::Core::Repositories

#endif // TRACKREPOSITORY_H
