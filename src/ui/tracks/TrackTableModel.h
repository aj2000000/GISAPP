/**
 * @file TrackTableModel.h
 * @brief Qt MVC Model bridging ITrackRepository to QTableView.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include <QAbstractTableModel>
#include "../../core/repositories/ITrackRepository.h"
#include <QVector>

namespace GISApp::UI::Tracks {

class TrackTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit TrackTableModel(Core::Repositories::ITrackRepository *repository, QObject *parent = nullptr);
    ~TrackTableModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Core::Models::TrackRecord getTrackAt(int row) const;

public slots:
    void reloadData();

private:
    Core::Repositories::ITrackRepository *m_repository{nullptr};
    QVector<Core::Models::TrackRecord> m_tracks;
    QStringList m_headers;
};

} // namespace GISApp::UI::Tracks

#endif // TRACKTABLEMODEL_H
