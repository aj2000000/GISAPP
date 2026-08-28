/**
 * @file CsvTrackIngestor.h
 * @brief Concrete strategy for parsing CSV files and importing batch tracks into ITrackRepository.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef CSVTRACKINGESTOR_H
#define CSVTRACKINGESTOR_H

#include "ITrackIngestor.h"

namespace GISApp::Core::Services {

class CsvTrackIngestor : public ITrackIngestor {
public:
    CsvTrackIngestor() = default;
    ~CsvTrackIngestor() override = default;

    int ingest(const QString &csvFilePath, Repositories::ITrackRepository &repository) override;
};

} // namespace GISApp::Core::Services

#endif // CSVTRACKINGESTOR_H
