#include "CsvTrackIngestor.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QMap>

namespace GISApp::Core::Services {

static QString normalizeHeaderKey(const QString &header)
{
    QString key = header.toLower().trimmed();
    key.replace(" ", "_");
    key.replace("-", "_");
    return key;
}

static int getColIndex(const QMap<QString, int> &headerMap, const QStringList &aliases, int fallbackIdx)
{
    for (const QString &alias : aliases) {
        if (headerMap.contains(alias)) {
            return headerMap.value(alias);
        }
    }
    return fallbackIdx;
}

int CsvTrackIngestor::ingest(const QString &csvFilePath, Repositories::ITrackRepository &repository)
{
    QFile file(csvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "[CsvTrackIngestor] Failed to open CSV file:" << csvFilePath;
        return -1;
    }

    QTextStream stream(&file);
    QVector<Models::TrackRecord> batchRecords;

    QMap<QString, int> headerMap;
    bool hasHeader = false;
    bool firstLine = true;

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }

        // Split by comma
        QStringList tokens = line.split(',');
        for (int i = 0; i < tokens.size(); ++i) {
            tokens[i] = tokens[i].trimmed();
            if (tokens[i].startsWith('"') && tokens[i].endsWith('"') && tokens[i].length() >= 2) {
                tokens[i] = tokens[i].mid(1, tokens[i].length() - 2);
            }
        }

        // Check first line for header
        if (firstLine) {
            firstLine = false;
            // If first column contains "TRACK_ID" or "ID", treat line as header
            if (!tokens.isEmpty() && (tokens[0].contains("TRACK_ID", Qt::CaseInsensitive) || tokens[0].contains("ID", Qt::CaseInsensitive))) {
                hasHeader = true;
                for (int i = 0; i < tokens.size(); ++i) {
                    headerMap.insert(normalizeHeaderKey(tokens[i]), i);
                }
                qDebug() << "[CsvTrackIngestor] Detected header with" << headerMap.size() << "columns.";
                continue;
            }
        }

        if (tokens.size() < 4) {
            continue;
        }

        // Determine indices based on headerMap or column count fallback
        int idIdx       = getColIndex(headerMap, {"track_id", "id"}, 0);
        int nameIdx     = getColIndex(headerMap, {"track_name", "name"}, 1);
        
        bool has20Cols  = (tokens.size() >= 20 || (hasHeader && headerMap.contains("track_plot_type")));
        int plotTypeIdx = getColIndex(headerMap, {"track_plot_type", "plot_type"}, has20Cols ? 2 : -1);
        int intNoIdx    = getColIndex(headerMap, {"int_no", "int_number", "int"}, has20Cols ? 3 : -1);

        int latIdx      = getColIndex(headerMap, {"track_lat", "lat", "latitude"}, has20Cols ? 4 : 2);
        int longIdx     = getColIndex(headerMap, {"track_long", "track_lng", "long", "longitude", "lng"}, has20Cols ? 5 : 3);
        int heightIdx   = getColIndex(headerMap, {"track_height", "height", "alt", "altitude"}, has20Cols ? 6 : 4);
        int dirIdx      = getColIndex(headerMap, {"track_dir", "dir", "heading", "bearing"}, has20Cols ? 7 : 5);
        int identityIdx = getColIndex(headerMap, {"track_identity", "identity"}, has20Cols ? 8 : 6);
        int typeIdx     = getColIndex(headerMap, {"track_type", "type"}, has20Cols ? 9 : 7);
        int subTypeIdx  = getColIndex(headerMap, {"track_sub_type", "sub_type"}, has20Cols ? 10 : 8);
        int classIdx    = getColIndex(headerMap, {"track_class", "class"}, has20Cols ? 11 : 9);
        int strengthIdx = getColIndex(headerMap, {"track_strength", "strength"}, has20Cols ? 12 : 10);
        int actTypeIdx  = getColIndex(headerMap, {"track_act_type", "act_type"}, has20Cols ? 13 : 11);
        int actSubIdx   = getColIndex(headerMap, {"track_act_sub_type", "act_sub_type"}, has20Cols ? 14 : 12);
        int actClassIdx = getColIndex(headerMap, {"track_act_class", "act_class"}, has20Cols ? 15 : 13);
        int sysTypeIdx  = getColIndex(headerMap, {"track_system_type", "system_type"}, has20Cols ? 16 : 14);
        int sourcesIdx  = getColIndex(headerMap, {"track_sources", "sources", "source"}, has20Cols ? 17 : 15);
        int imageIdx    = getColIndex(headerMap, {"track_image", "image", "icon"}, has20Cols ? 18 : 16);
        int remarksIdx  = getColIndex(headerMap, {"track_remarks", "remarks", "comment"}, has20Cols ? 19 : 17);

        Models::TrackRecord track;
        if (idIdx >= 0 && idIdx < tokens.size()) track.trackId = tokens[idIdx].toInt();
        if (nameIdx >= 0 && nameIdx < tokens.size()) track.trackName = tokens[nameIdx];
        if (plotTypeIdx >= 0 && plotTypeIdx < tokens.size()) track.trackPlotType = tokens[plotTypeIdx].toDouble();
        if (intNoIdx >= 0 && intNoIdx < tokens.size()) track.intNo = tokens[intNoIdx].toInt();
        if (latIdx >= 0 && latIdx < tokens.size()) track.trackLat = tokens[latIdx].toDouble();
        if (longIdx >= 0 && longIdx < tokens.size()) track.trackLong = tokens[longIdx].toDouble();
        if (heightIdx >= 0 && heightIdx < tokens.size()) track.trackHeight = tokens[heightIdx].toDouble();
        if (dirIdx >= 0 && dirIdx < tokens.size()) track.trackDir = tokens[dirIdx].toDouble();
        if (identityIdx >= 0 && identityIdx < tokens.size()) track.trackIdentity = tokens[identityIdx].toInt();
        if (typeIdx >= 0 && typeIdx < tokens.size()) track.trackType = tokens[typeIdx].toInt();
        if (subTypeIdx >= 0 && subTypeIdx < tokens.size()) track.trackSubType = tokens[subTypeIdx].toInt();
        if (classIdx >= 0 && classIdx < tokens.size()) track.trackClass = tokens[classIdx].toInt();
        if (strengthIdx >= 0 && strengthIdx < tokens.size()) track.trackStrength = tokens[strengthIdx].toInt();
        if (actTypeIdx >= 0 && actTypeIdx < tokens.size()) track.trackActType = tokens[actTypeIdx].toInt();
        if (actSubIdx >= 0 && actSubIdx < tokens.size()) track.trackActSubType = tokens[actSubIdx].toInt();
        if (actClassIdx >= 0 && actClassIdx < tokens.size()) track.trackActClass = tokens[actClassIdx].toInt();
        if (sysTypeIdx >= 0 && sysTypeIdx < tokens.size()) track.trackSystemType = tokens[sysTypeIdx].toInt();
        if (sourcesIdx >= 0 && sourcesIdx < tokens.size()) track.trackSources = tokens[sourcesIdx];
        if (imageIdx >= 0 && imageIdx < tokens.size()) track.trackImage = tokens[imageIdx];
        if (remarksIdx >= 0 && remarksIdx < tokens.size()) track.trackRemarks = tokens[remarksIdx];

        batchRecords.append(track);
    }

    file.close();

    if (batchRecords.isEmpty()) {
        qWarning() << "[CsvTrackIngestor] No valid track records parsed from:" << csvFilePath;
        return 0;
    }

    qDebug() << "[CsvTrackIngestor] Parsed" << batchRecords.size() << "records. Committing to repository...";
    if (!repository.insertBatch(batchRecords)) {
        qCritical() << "[CsvTrackIngestor] Repository batch insertion failed.";
        return -1;
    }

    return batchRecords.size();
}

} // namespace GISApp::Core::Services
