#ifndef CSVBOUNDARYINGESTOR_H
#define CSVBOUNDARYINGESTOR_H

#include "models/BoundaryRecord.h"
#include <QString>
#include <QVector>
#include <QObject>

namespace GISApp::Core::Services {

class CsvBoundaryIngestor : public QObject {
    Q_OBJECT
public:
    explicit CsvBoundaryIngestor(QObject *parent = nullptr);
    ~CsvBoundaryIngestor() override = default;

    QVector<Models::BoundaryRecord> ingestCsv(const QString &csvPath);
};

} // namespace GISApp::Core::Services

#endif // CSVBOUNDARYINGESTOR_H
