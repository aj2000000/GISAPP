/**
 * @file ITrackIngestor.h
 * @brief Strategy interface for data ingestion pipelines (CSV, UDP, REST API).
 * Following Strategy Design Pattern and SOLID Open/Closed Principle.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef ITRACKINGESTOR_H
#define ITRACKINGESTOR_H

#include "../repositories/ITrackRepository.h"
#include <QString>

namespace GISApp::Core::Services {

class ITrackIngestor {
public:
    virtual ~ITrackIngestor() = default;

    /**
     * @brief Ingest tracks from a data source into the target repository.
     * @param sourceLocation File path, socket URL, or data payload.
     * @param repository Destination track repository interface.
     * @return Number of successfully ingested tracks, or -1 on error.
     */
    virtual int ingest(const QString &sourceLocation, Repositories::ITrackRepository &repository) = 0;
};

} // namespace GISApp::Core::Services

#endif // ITRACKINGESTOR_H
