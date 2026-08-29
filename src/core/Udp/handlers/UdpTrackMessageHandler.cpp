/**
 * @file UdpTrackMessageHandler.cpp
 * @brief Strategy handler for deserializing MAIN_LITE_TRACK_MSG (ID: 613) UDP packets.
 */

#include "UdpTrackMessageHandler.h"
#include "UdpMessages/MessageId.h"
#include "UdpMessages/Structures.h"
#include "core/models/TrackRecord.h"
#include <QDebug>
#include <QStringList>

namespace GISApp::Core::Udp::Handlers {

UdpTrackMessageHandler::UdpTrackMessageHandler(Repositories::ITrackRepository *trackRepo)
    : m_trackRepo(trackRepo)
{
}

MESSAGE_ID UdpTrackMessageHandler::messageId() const
{
    return MAIN_LITE_TRACK_MSG_ID; // 613
}

bool UdpTrackMessageHandler::processPayload(const QByteArray &payload)
{
    const char *dataPtr = payload.constData();
    qsizetype totalBytes = payload.size();
    qsizetype offset = 0;

    if (totalBytes < static_cast<qsizetype>(sizeof(STRUCT_MESSAGE_HEADER) + sizeof(UINT_16))) {
        qWarning() << "[UdpTrackMessageHandler] Payload too short for track header!";
        return false;
    }

    // Read Message Header
    STRUCT_MESSAGE_HEADER header;
    SMEMCPY(&header, dataPtr + offset, sizeof(STRUCT_MESSAGE_HEADER));
    offset += sizeof(STRUCT_MESSAGE_HEADER);

    // Read Track Count
    UINT_16 noOfTracks = 0;
    SMEMCPY(&noOfTracks, dataPtr + offset, sizeof(UINT_16));
    offset += sizeof(UINT_16);

    qDebug() << "[UdpTrackMessageHandler] Deserializing" << noOfTracks << "tracks from UDP payload.";

    if (!m_trackRepo) return false;

    for (UINT_16 i = 0; i < noOfTracks; ++i) {
        if (offset + static_cast<qsizetype>(sizeof(STRUCT_TRACK)) > totalBytes) {
            qWarning() << "[UdpTrackMessageHandler] Payload truncated at track index:" << i;
            break;
        }

        STRUCT_TRACK rawTrack;
        SMEMCPY(&rawTrack, dataPtr + offset, sizeof(STRUCT_TRACK));
        offset += sizeof(STRUCT_TRACK);

        // Map fields to Domain Model
        Models::TrackRecord record;
        record.trackId         = static_cast<int>(rawTrack.track_id);
        record.trackName       = QString::fromUtf8(rawTrack.track_name).trimmed();
        record.trackLat        = rawTrack.track_loc.latatitude;
        record.trackLong       = rawTrack.track_loc.longitude;
        record.trackHeight     = rawTrack.track_loc.height;
        record.trackDir        = rawTrack.track_loc.dir;
        record.trackIdentity   = static_cast<int>(rawTrack.track_identity);
        record.trackType       = static_cast<int>(rawTrack.track_attributes.type);
        record.trackSubType    = static_cast<int>(rawTrack.track_attributes.sub_type);
        record.trackClass      = static_cast<int>(rawTrack.track_attributes.classification);
        record.trackStrength   = static_cast<int>(rawTrack.track_attributes.strength);
        record.trackActType    = static_cast<int>(rawTrack.track_attributes.act_type);
        record.trackActSubType = static_cast<int>(rawTrack.track_attributes.act_sub_type);
        record.trackActClass   = static_cast<int>(rawTrack.track_attributes.act_classification);
        record.trackSystemType = static_cast<int>(rawTrack.sys_track_type);

        // Format Sources List
        QStringList sourcesList;
        for (const auto &src : rawTrack.track_sources) {
            QString srcName = QString::fromUtf8(src.source_id).trimmed();
            if (!srcName.isEmpty()) sourcesList.append(srcName);
        }
        record.trackSources = sourcesList.join(", ");

        record.trackImage = QString::fromUtf8(rawTrack.track_symbol.symbol_name).trimmed();

        // Format Report Date & Time from STRUCT_DATE_TIME
        QString reportTimeStr = QString("[%1-%2-%3 %4:%5:%6]")
            .arg(rawTrack.track_report_time.date.year, 4, 10, QChar('0'))
            .arg(rawTrack.track_report_time.date.month, 2, 10, QChar('0'))
            .arg(rawTrack.track_report_time.date.day, 2, 10, QChar('0'))
            .arg(rawTrack.track_report_time.time.hour, 2, 10, QChar('0'))
            .arg(rawTrack.track_report_time.time.minute, 2, 10, QChar('0'))
            .arg(rawTrack.track_report_time.time.second, 2, 10, QChar('0'));

        QString rawRemarks = QString::fromUtf8(rawTrack.track_remarks).trimmed();
        record.trackReportTime = reportTimeStr;
        record.trackRemarks = rawRemarks.isEmpty() ? reportTimeStr : QString("%1 %2").arg(reportTimeStr).arg(rawRemarks);

        // Ingest into repository (Triggers MapLibre map auto-refresh)
        m_trackRepo->insertOrUpdateTrack(record);
    }

    return true;
}

} // namespace GISApp::Core::Udp::Handlers
