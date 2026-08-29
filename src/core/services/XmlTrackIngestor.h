/**
 * @file XmlTrackIngestor.h
 * @brief Dynamic XML Ingestion strategy for loading tactical tracks into ITrackRepository.
 * Implements ITrackIngestor interface following Strategy & Open/Closed SOLID Principles.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#ifndef XMLTRACKINGESTOR_H
#define XMLTRACKINGESTOR_H

#include "ITrackIngestor.h"
#include <QObject>
#include <QXmlStreamReader>

namespace GISApp::Core::Services {

/**
 * @class XmlTrackIngestor
 * @brief Service responsible for parsing dynamic XML track files and saving records to database.
 */
class XmlTrackIngestor : public QObject, public ITrackIngestor {
    Q_OBJECT

public:
    explicit XmlTrackIngestor(QObject *parent = nullptr);
    ~XmlTrackIngestor() override = default;

    /**
     * @brief Ingests track data from an XML file into target track repository.
     * @param sourceLocation Absolute file path to the XML file.
     * @param repository Target ITrackRepository implementation.
     * @return Number of ingested tracks, or -1 on failure.
     */
    int ingest(const QString &sourceLocation, Repositories::ITrackRepository &repository) override;

private:
    /**
     * @brief Helper to parse a single <track> or <Track> XML block dynamically.
     * @param xml Reader positioned at start element of track.
     * @return Fully populated TrackRecord model.
     */
    Models::TrackRecord parseTrackElement(QXmlStreamReader &xml);
};

} // namespace GISApp::Core::Services

#endif // XMLTRACKINGESTOR_H
