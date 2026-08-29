/**
 * @file TracksTableDialog.h
 * @brief High-precision Dark-themed Qt Dialog displaying stored Track Entities in a QTableView.
 * Provides features for real-time search filtering, zooming to entity coordinates,
 * deleting records, importing CSVs, and dynamic Layer Tree registration.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef TRACKSTABLEDIALOG_H
#define TRACKSTABLEDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSortFilterProxyModel>

#include "../../core/repositories/ITrackRepository.h"
#include "TrackTableModel.h"

namespace QMapLibre {
    class Map;
}

namespace GISApp::Layers {
    class LayerManager;
}

namespace GISApp::Controllers {
    class MapController;
}

namespace GISApp::Core::Services {
    class MapLibreTrackAdapter;
}

namespace GISApp::UI::Tracks {

class TracksTableDialog : public QDialog {
    Q_OBJECT
public:
    explicit TracksTableDialog(
        Core::Repositories::ITrackRepository *repository,
        Layers::LayerManager *layerManager = nullptr,
        QMapLibre::Map *map = nullptr,
        Controllers::MapController *mapController = nullptr,
        Core::Services::MapLibreTrackAdapter *trackAdapter = nullptr,
        QWidget *parent = nullptr
    );
    ~TracksTableDialog() override = default;

private slots:
    void onClearDatabaseClicked();
    void onAddToLayerTreeClicked();
    void onZoomToSelectedClicked();
    void onDeleteSelectedClicked();
    void onRefreshClicked();
    void onSearchTextChanged(const QString &text);
    void onTableDoubleClicked(const QModelIndex &index);
    void updateCountAndLayerStatus();

private:
    void setupUi();
    void applyDarkTheme();
    bool isTrackLayerInTree() const;
    void ensureTracksLayerInTree();

    Core::Repositories::ITrackRepository *m_repository{nullptr};
    Layers::LayerManager *m_layerManager{nullptr};
    QMapLibre::Map *m_map{nullptr};
    Controllers::MapController *m_mapController{nullptr};
    Core::Services::MapLibreTrackAdapter *m_trackAdapter{nullptr};

    TrackTableModel *m_tableModel{nullptr};
    QSortFilterProxyModel *m_proxyModel{nullptr};

    QTableView *m_tableView{nullptr};
    QLineEdit *m_searchLineEdit{nullptr};

    QLabel *m_countLabel{nullptr};
    QLabel *m_layerStatusLabel{nullptr};
    QLabel *m_filterStatusLabel{nullptr};

    QPushButton *m_addToLayerTreeBtn{nullptr};
    QPushButton *m_zoomToSelectedBtn{nullptr};
    QPushButton *m_deleteSelectedBtn{nullptr};
    QPushButton *m_clearBtn{nullptr};
    QPushButton *m_refreshBtn{nullptr};
};

} // namespace GISApp::UI::Tracks

#endif // TRACKSTABLEDIALOG_H
