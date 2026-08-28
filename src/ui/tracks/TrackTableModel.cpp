#include "TrackTableModel.h"

namespace GISApp::UI::Tracks {

TrackTableModel::TrackTableModel(Core::Repositories::ITrackRepository *repository, QObject *parent)
    : QAbstractTableModel(parent), m_repository(repository)
{
    m_headers = {
        "ID", "Name", "Plot Type", "Int No", "Latitude", "Longitude", "Height", "Heading (°)",
        "Identity", "Type", "Sub-Type", "Class", "Strength",
        "Act Type", "Act Sub-Type", "Act Class", "System Type",
        "Sources", "Image", "Remarks"
    };

    if (m_repository) {
        connect(m_repository, &Core::Repositories::ITrackRepository::tracksUpdated,
                this, &TrackTableModel::reloadData);
        reloadData();
    }
}

void TrackTableModel::reloadData()
{
    beginResetModel();
    if (m_repository) {
        m_tracks = m_repository->getAllTracks();
    } else {
        m_tracks.clear();
    }
    endResetModel();
}

int TrackTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_tracks.size();
}

int TrackTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_headers.size();
}

QVariant TrackTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size()) {
        return QVariant();
    }

    const auto &track = m_tracks.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case 0: return track.trackId;
            case 1: return track.trackName;
            case 2: return track.trackPlotType;
            case 3: return track.intNo;
            case 4: return QString::number(track.trackLat, 'f', 6);
            case 5: return QString::number(track.trackLong, 'f', 6);
            case 6: return track.trackHeight;
            case 7: return QString::number(track.trackDir, 'f', 1);
            case 8: return track.trackIdentity;
            case 9: return track.trackType;
            case 10: return track.trackSubType;
            case 11: return track.trackClass;
            case 12: return track.trackStrength;
            case 13: return track.trackActType;
            case 14: return track.trackActSubType;
            case 15: return track.trackActClass;
            case 16: return track.trackSystemType;
            case 17: return track.trackSources;
            case 18: return track.trackImage;
            case 19: return track.trackRemarks;
            default: return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == 1 || index.column() == 17 || index.column() == 18 || index.column() == 19) {
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        return static_cast<int>(Qt::AlignCenter);
    }

    return QVariant();
}

QVariant TrackTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section >= 0 && section < m_headers.size()) {
            return m_headers.at(section);
        }
    }
    return QVariant();
}

Core::Models::TrackRecord TrackTableModel::getTrackAt(int row) const
{
    if (row >= 0 && row < m_tracks.size()) {
        return m_tracks.at(row);
    }
    return Core::Models::TrackRecord();
}

} // namespace GISApp::UI::Tracks

