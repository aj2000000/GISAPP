#include "CsvBoundaryIngestor.h"
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QDebug>

namespace GISApp::Core::Services {

CsvBoundaryIngestor::CsvBoundaryIngestor(QObject *parent)
    : QObject(parent)
{
}

QVector<Models::BoundaryRecord> CsvBoundaryIngestor::ingestCsv(const QString &csvPath)
{
    QVector<Models::BoundaryRecord> results;
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[CsvBoundaryIngestor] Failed to open boundary CSV file at:" << csvPath;
        return results;
    }

    QTextStream in(&file);
    if (in.atEnd()) return results;

    // Read header line (BOUNDARY_ID,LATITUDE,LONGITUDE)
    QString headerLine = in.readLine();

    QMap<int, Models::BoundaryRecord> boundaryMap;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(',');
        if (parts.size() >= 3) {
            bool idOk = false, latOk = false, lonOk = false;
            int bId = parts[0].trimmed().toInt(&idOk);
            double lat = parts[1].trimmed().toDouble(&latOk);
            double lon = parts[2].trimmed().toDouble(&lonOk);

            if (idOk && latOk && lonOk) {
                if (!boundaryMap.contains(bId)) {
                    Models::BoundaryRecord rec;
                    rec.boundaryId = bId;
                    rec.name = QString("National Boundary #%1").arg(bId);
                    boundaryMap[bId] = rec;
                }
                boundaryMap[bId].points.append(Models::Coordinate3D(lat, lon, 0.0));
            }
        }
    }
    file.close();

    for (const auto &rec : boundaryMap) {
        results.append(rec);
    }

    qWarning() << "[CsvBoundaryIngestor] Successfully ingested" << results.size() << "boundary lines from CSV:" << csvPath;
    return results;
}

} // namespace GISApp::Core::Services
