/**
 * @file XmlTrackIngestor.cpp
 * @brief Implementation of dynamic XML Track parser using QXmlStreamReader.
 */

#include "XmlTrackIngestor.h"
#include <QFile>
#include <QDebug>

namespace GISApp::Core::Services {

XmlTrackIngestor::XmlTrackIngestor(QObject *parent)
    : QObject(parent)
{
}

int XmlTrackIngestor::ingest(const QString &sourceLocation, Repositories::ITrackRepository &repository)
{
    QFile file(sourceLocation);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[XmlTrackIngestor] Failed to open XML file:" << sourceLocation << file.errorString();
        return -1;
    }

    QXmlStreamReader xml(&file);
    QVector<Models::TrackRecord> records;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString().toLower();
            if (name == "track" || name == "trackrecord") {
                Models::TrackRecord record = parseTrackElement(xml);
                records.append(record);
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "[XmlTrackIngestor] XML parsing error in file:" << sourceLocation << xml.errorString();
        file.close();
        return -1;
    }

    file.close();

    // Perform batch insertion into SQLite repository
    bool ok = repository.insertBatch(records);
    if (!ok) {
        qWarning() << "[XmlTrackIngestor] Repository batch insert failed for file:" << sourceLocation;
        return -1;
    }

    qDebug() << "[XmlTrackIngestor] Successfully ingested" << records.size() << "tracks from XML file:" << sourceLocation;
    return records.size();
}

Models::TrackRecord XmlTrackIngestor::parseTrackElement(QXmlStreamReader &xml)
{
    Models::TrackRecord rec;

    // Read attributes if present on <track id="..." name="...">
    QXmlStreamAttributes attrs = xml.attributes();
    if (attrs.hasAttribute("id")) rec.trackId = attrs.value("id").toInt();
    if (attrs.hasAttribute("trackId")) rec.trackId = attrs.value("trackId").toInt();
    if (attrs.hasAttribute("name")) rec.trackName = attrs.value("name").toString();

    // Process dynamic sub-elements until end of </track>
    while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
            (xml.name().toString().toLower() == "track" || xml.name().toString().toLower() == "trackrecord")))
    {
        xml.readNext();
        if (xml.isStartElement()) {
            QString tag = xml.name().toString().toLower();
            QString text = xml.readElementText().trimmed();

            if (tag == "trackid" || tag == "id") rec.trackId = text.toInt();
            else if (tag == "trackname" || tag == "name") rec.trackName = text;
            else if (tag == "tracklat" || tag == "lat" || tag == "latitude") rec.trackLat = text.toDouble();
            else if (tag == "tracklong" || tag == "long" || tag == "lon" || tag == "longitude") rec.trackLong = text.toDouble();
            else if (tag == "trackheight" || tag == "height" || tag == "alt" || tag == "altitude") rec.trackHeight = text.toDouble();
            else if (tag == "trackdir" || tag == "dir" || tag == "heading" || tag == "bearing") rec.trackDir = text.toDouble();
            else if (tag == "trackidentity" || tag == "identity") rec.trackIdentity = text.toInt();
            else if (tag == "tracktype" || tag == "type") rec.trackType = text.toInt();
            else if (tag == "tracksubtype" || tag == "subtype") rec.trackSubType = text.toInt();
            else if (tag == "trackclass" || tag == "class" || tag == "classification") rec.trackClass = text.toInt();
            else if (tag == "trackstrength" || tag == "strength") rec.trackStrength = text.toInt();
            else if (tag == "trackacttype" || tag == "acttype") rec.trackActType = text.toInt();
            else if (tag == "trackactsubtype" || tag == "actsubtype") rec.trackActSubType = text.toInt();
            else if (tag == "trackactclass" || tag == "actclass") rec.trackActClass = text.toInt();
            else if (tag == "tracksystemtype" || tag == "systemtype") rec.trackSystemType = text.toInt();
            else if (tag == "tracksources" || tag == "sources" || tag == "source") rec.trackSources = text;
            else if (tag == "trackimage" || tag == "image" || tag == "symbol") rec.trackImage = text;
            else if (tag == "trackremarks" || tag == "remarks") rec.trackRemarks = text;
            else if (tag == "trackreporttime" || tag == "reporttime" || tag == "time") rec.trackReportTime = text;

        }
    }

    return rec;
}

} // namespace GISApp::Core::Services
