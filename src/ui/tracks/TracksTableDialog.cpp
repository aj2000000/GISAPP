#include "TracksTableDialog.h"
#include "../../core/services/CsvTrackIngestor.h"
#include "../../core/services/MapLibreTrackAdapter.h"
#include "../../core/notifications/NotificationManager.h"
#include "../../layers/LayerManager.h"
#include "../../layers/MapLibreLayerAdapter.h"
#include "../../controllers/MapController.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

namespace GISApp::UI::Tracks {

TracksTableDialog::TracksTableDialog(
    Core::Repositories::ITrackRepository *repository,
    Layers::LayerManager *layerManager,
    QMapLibre::Map *map,
    Controllers::MapController *mapController,
    Core::Services::MapLibreTrackAdapter *trackAdapter,
    QWidget *parent
)
    : QDialog(parent),
      m_repository(repository),
      m_layerManager(layerManager),
      m_map(map),
      m_mapController(mapController),
      m_trackAdapter(trackAdapter)
{
    setWindowTitle(tr("🎯 Tactical Entities — SQLite Tracks Database"));
    resize(1280, 720);

    m_tableModel = new TrackTableModel(m_repository, this);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_tableModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // Search across all columns

    setupUi();
    applyDarkTheme();

    if (m_repository) {
        connect(m_repository, &Core::Repositories::ITrackRepository::tracksUpdated,
                this, &TracksTableDialog::updateCountAndLayerStatus);
    }

    updateCountAndLayerStatus();
}

void TracksTableDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // 1. Header Layout
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QVBoxLayout *titleTextLayout = new QVBoxLayout();
    QLabel *titleLabel = new QLabel("🎯 TACTICAL TRACK ENTITIES DATABASE", this);
    titleLabel->setStyleSheet("font-size: 17px; font-weight: bold; color: #38bdf8; letter-spacing: 0.5px;");
    
    QLabel *subtitleLabel = new QLabel("SQLite Persistence • Dynamic Layer Tree Integration • Real-Time Map Telemetry", this);
    subtitleLabel->setStyleSheet("font-size: 11px; color: #94a3b8;");
    
    titleTextLayout->addWidget(titleLabel);
    titleTextLayout->addWidget(subtitleLabel);
    headerLayout->addLayout(titleTextLayout);
    headerLayout->addStretch();

    m_countLabel = new QLabel("Total Records: 0", this);
    m_countLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #a3e635; background: #1e293b; padding: 6px 12px; border: 1px solid #334155; border-radius: 6px;");

    m_layerStatusLabel = new QLabel("Layer Tree: Not Added", this);
    m_layerStatusLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #f59e0b; background: #1e293b; padding: 6px 12px; border: 1px solid #334155; border-radius: 6px;");

    headerLayout->addWidget(m_countLabel);
    headerLayout->addWidget(m_layerStatusLabel);
    mainLayout->addLayout(headerLayout);

    // 2. Control & Search Bar
    QHBoxLayout *searchLayout = new QHBoxLayout();
    
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("🔍 Search tracks by Name, Source, Remarks, Int No, Lat, Long, Identity...");
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &TracksTableDialog::onSearchTextChanged);

    m_filterStatusLabel = new QLabel("Showing 0 tracks", this);
    m_filterStatusLabel->setStyleSheet("font-size: 11px; color: #94a3b8; font-weight: bold; margin-left: 8px;");

    searchLayout->addWidget(m_searchLineEdit, 1);
    searchLayout->addWidget(m_filterStatusLabel);
    mainLayout->addLayout(searchLayout);

    // 3. Table View
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setVisible(false);
    
    connect(m_tableView, &QTableView::doubleClicked, this, &TracksTableDialog::onTableDoubleClicked);

    mainLayout->addWidget(m_tableView);

    // 4. Bottom Action Bar
    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_importBtn = new QPushButton("📥 Upload CSV...", this);
    m_addToLayerTreeBtn = new QPushButton("➕ Add to Layer Tree", this);
    m_zoomToSelectedBtn = new QPushButton("🎯 Zoom to Selected", this);
    m_deleteSelectedBtn = new QPushButton("🗑️ Delete Selected", this);
    m_clearBtn = new QPushButton("🧹 Clear All", this);
    m_refreshBtn = new QPushButton("🔄 Refresh", this);
    QPushButton *closeBtn = new QPushButton("Close", this);

    connect(m_importBtn, &QPushButton::clicked, this, &TracksTableDialog::onImportCsvClicked);
    connect(m_addToLayerTreeBtn, &QPushButton::clicked, this, &TracksTableDialog::onAddToLayerTreeClicked);
    connect(m_zoomToSelectedBtn, &QPushButton::clicked, this, &TracksTableDialog::onZoomToSelectedClicked);
    connect(m_deleteSelectedBtn, &QPushButton::clicked, this, &TracksTableDialog::onDeleteSelectedClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &TracksTableDialog::onClearDatabaseClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &TracksTableDialog::onRefreshClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    actionLayout->addWidget(m_importBtn);
    actionLayout->addWidget(m_addToLayerTreeBtn);
    actionLayout->addWidget(m_zoomToSelectedBtn);
    actionLayout->addWidget(m_deleteSelectedBtn);
    actionLayout->addWidget(m_clearBtn);
    actionLayout->addWidget(m_refreshBtn);
    actionLayout->addStretch();
    actionLayout->addWidget(closeBtn);

    mainLayout->addLayout(actionLayout);
}

void TracksTableDialog::applyDarkTheme()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #0f172a;
            color: #f8fafc;
        }
        QLabel {
            color: #f8fafc;
            font-size: 12px;
        }
        QMessageBox {
            background-color: #0f172a;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 8px;
        }
        QMessageBox QLabel {
            color: #f8fafc;
            font-size: 13px;
            font-weight: 500;
            background: transparent;
        }
        QMessageBox QPushButton {
            background-color: #1e293b;
            color: #38bdf8;
            border: 1px solid #0284c7;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: bold;
            font-size: 12px;
            min-width: 75px;
        }
        QMessageBox QPushButton:hover {
            background-color: #0284c7;
            color: #ffffff;
        }
        QLineEdit {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            padding: 8px 12px;
            border-radius: 6px;
            font-size: 13px;
        }
        QLineEdit:focus {
            border-color: #38bdf8;
        }
        QTableView {
            background-color: #1e293b;
            alternate-background-color: #0f172a;
            color: #f1f5f9;
            gridline-color: #334155;
            border: 1px solid #334155;
            border-radius: 6px;
            selection-background-color: #0284c7;
            selection-color: #ffffff;
            font-size: 12px;
        }
        QTableView::item {
            color: #f1f5f9;
            background-color: #1e293b;
            padding: 4px;
        }
        QTableView::item:alternate {
            background-color: #0f172a;
            color: #f1f5f9;
        }
        QTableView::item:selected {
            background-color: #0284c7;
            color: #ffffff;
        }
        QTableView::item:hover {
            background-color: #334155;
            color: #38bdf8;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #38bdf8;
            padding: 8px;
            font-weight: bold;
            font-size: 12px;
            border: 1px solid #334155;
        }
        QHeaderView::section:hover {
            background-color: #1e293b;
            color: #7dd3fc;
        }
        QPushButton {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #475569;
            padding: 8px 14px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #334155;
            border-color: #38bdf8;
            color: #38bdf8;
        }
        QPushButton:pressed {
            background-color: #0284c7;
            color: #ffffff;
        }
        QPushButton:disabled {
            background-color: #0f172a;
            color: #475569;
            border-color: #1e293b;
        }
        QScrollBar:vertical {
            background: #0f172a;
            width: 10px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #334155;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: #38bdf8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background: #0f172a;
            height: 10px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: #334155;
            min-width: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #38bdf8;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )");
}

bool TracksTableDialog::isTrackLayerInTree() const
{
    if (!m_layerManager) return false;
    return m_layerManager->findLayerByLayerId("tracks-circle-layer") != nullptr;
}

void TracksTableDialog::ensureTracksLayerInTree()
{
    if (!m_layerManager) {
        qWarning() << "[TracksTableDialog] Cannot add to layer tree: LayerManager is null";
        return;
    }

    if (isTrackLayerInTree()) {
        updateCountAndLayerStatus();
        return;
    }

    // Find or create group "🛡️ Tactical Operations"
    Layers::LayerGroupNode *tacticalGroup = m_layerManager->findGroupByName("Tactical Operations");
    if (!tacticalGroup) {
        tacticalGroup = m_layerManager->addGroup("🛡️ Tactical Operations");
    }

    Layers::LayerExtent trackExtent{
        GISApp::Core::Models::GeoCoordinate(8.4, 68.7),
        GISApp::Core::Models::GeoCoordinate(37.6, 97.25)
    };

    if (m_repository) {
        auto tracks = m_repository->getAllTracks();
        if (!tracks.isEmpty()) {
            double minLat = 90.0, maxLat = -90.0;
            double minLon = 180.0, maxLon = -180.0;
            for (const auto &track : tracks) {
                if (track.trackLat < minLat) minLat = track.trackLat;
                if (track.trackLat > maxLat) maxLat = track.trackLat;
                if (track.trackLong < minLon) minLon = track.trackLong;
                if (track.trackLong > maxLon) maxLon = track.trackLong;
            }
            if (std::abs(maxLat - minLat) < 0.0001) {
                minLat -= 0.005;
                maxLat += 0.005;
            }
            if (std::abs(maxLon - minLon) < 0.0001) {
                minLon -= 0.005;
                maxLon += 0.005;
            }
            trackExtent = Layers::LayerExtent{
                GISApp::Core::Models::GeoCoordinate(minLat, minLon),
                GISApp::Core::Models::GeoCoordinate(maxLat, maxLon)
            };
        }
    }

    auto tracksAdapterNode = std::make_shared<Layers::MapLibreLayerAdapter>(
        "tracks-circle-layer", m_map, trackExtent);

    m_layerManager->addLayer("🎯 Tactical Tracks", tracksAdapterNode, tacticalGroup);

    if (m_trackAdapter) {
        m_trackAdapter->setLayerManager(m_layerManager);
        m_trackAdapter->refreshFromRepository();
    }

    Core::Notifications::NotificationManager::instance()->notifyFlash(
        "Layer Tree Updated",
        "🎯 Tactical Tracks layer added to Layer Management tree successfully!",
        5000,
        this
    );

    updateCountAndLayerStatus();
}

void TracksTableDialog::updateCountAndLayerStatus()
{
    int totalCount = m_repository ? m_repository->trackCount() : 0;
    m_countLabel->setText(QString("📊 Total Records: %1").arg(totalCount));

    bool inTree = isTrackLayerInTree();
    if (inTree) {
        m_layerStatusLabel->setText("🌲 Status: In Layer Tree");
        m_layerStatusLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #a3e635; background: #1e293b; padding: 6px 12px; border: 1px solid #334155; border-radius: 6px;");
        m_addToLayerTreeBtn->setText("✔ In Layer Tree");
        m_addToLayerTreeBtn->setStyleSheet("background-color: #14532d; color: #86efac; border: 1px solid #22c55e; padding: 8px 14px; border-radius: 6px; font-weight: bold;");
    } else {
        m_layerStatusLabel->setText("⚠️ Status: Not in Layer Tree");
        m_layerStatusLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #f59e0b; background: #1e293b; padding: 6px 12px; border: 1px solid #334155; border-radius: 6px;");
        m_addToLayerTreeBtn->setText("➕ Add to Layer Tree");
        m_addToLayerTreeBtn->setStyleSheet("");
    }

    int visibleCount = m_proxyModel ? m_proxyModel->rowCount() : totalCount;
    m_filterStatusLabel->setText(QString("Showing %1 of %2 tracks").arg(visibleCount).arg(totalCount));
}

void TracksTableDialog::onSearchTextChanged(const QString &text)
{
    if (m_proxyModel) {
        m_proxyModel->setFilterFixedString(text);
        updateCountAndLayerStatus();
    }
}

void TracksTableDialog::onAddToLayerTreeClicked()
{
    if (isTrackLayerInTree()) {
        Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Layer Tree Info",
            "🎯 Tactical Tracks layer is already active in the Layer Management tree.",
            4000,
            this
        );
    } else {
        ensureTracksLayerInTree();
    }
}

void TracksTableDialog::onZoomToSelectedClicked()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, tr("Selection Required"), tr("Please select a track from the table first."));
        return;
    }

    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

    Core::Models::TrackRecord track = m_tableModel->getTrackAt(sourceIndex.row());
    if (track.trackId <= 0) {
        qWarning() << "[TracksTableDialog] Invalid track selected.";
        return;
    }

    if (m_mapController) {
        m_mapController->centerOn(Core::Models::GeoCoordinate(track.trackLat, track.trackLong), 12.0);
        Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Map Panned",
            QString("Centered map on %1 (Lat: %2, Long: %3)")
                .arg(track.trackName)
                .arg(track.trackLat, 0, 'f', 4)
                .arg(track.trackLong, 0, 'f', 4),
            4000,
            this
        );
    } else if (m_map) {
        m_map->setCoordinateZoom(QMapLibre::Coordinate(track.trackLat, track.trackLong), 12.0);
    }
}

void TracksTableDialog::onDeleteSelectedClicked()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, tr("Selection Required"), tr("Please select a track row to delete."));
        return;
    }

    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

    Core::Models::TrackRecord track = m_tableModel->getTrackAt(sourceIndex.row());
    if (track.trackId <= 0) return;

    auto result = QMessageBox::question(
        this,
        tr("Confirm Deletion"),
        tr("Are you sure you want to delete track '%1' (ID: %2) from SQLite database?")
            .arg(track.trackName)
            .arg(track.trackId),
        QMessageBox::Yes | QMessageBox::No
    );

    if (result == QMessageBox::Yes && m_repository) {
        if (m_repository->deleteTrack(track.trackId)) {
            Core::Notifications::NotificationManager::instance()->notifyFlash(
                "Track Deleted",
                QString("Successfully deleted track '%1'.").arg(track.trackName),
                4000,
                this
            );
            updateCountAndLayerStatus();
        }
    }
}

void TracksTableDialog::onTableDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    onZoomToSelectedClicked();
}

void TracksTableDialog::onRefreshClicked()
{
    if (m_tableModel) {
        m_tableModel->reloadData();
    }
    updateCountAndLayerStatus();
}

void TracksTableDialog::onImportCsvClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Tracks CSV File"),
        QDir::homePath(),
        tr("CSV Files (*.csv);;All Files (*)")
    );

    if (filePath.isEmpty()) return;

    Core::Services::CsvTrackIngestor ingestor;
    int imported = ingestor.ingest(filePath, *m_repository);

    if (imported >= 0) {
        // Automatically ensure layer is in layer tree when tracks are uploaded
        ensureTracksLayerInTree();

        if (m_tableModel) {
            m_tableModel->reloadData();
        }

        if (m_trackAdapter) {
            m_trackAdapter->refreshFromRepository();
        }

        Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Bulk Ingestion Complete",
            QString("Successfully ingested %1 tracks into SQLite database.").arg(imported),
            5000,
            this
        );
        updateCountAndLayerStatus();
    } else {
        QMessageBox::critical(this, tr("Import Error"), tr("Failed to parse and import CSV file."));
    }
}

void TracksTableDialog::onClearDatabaseClicked()
{
    if (QMessageBox::question(this, tr("Confirm Clear"), tr("Are you sure you want to delete all track records from SQLite database?")) == QMessageBox::Yes) {
        if (m_repository) {
            m_repository->clearAllTracks();
            Core::Notifications::NotificationManager::instance()->notifyFlash(
                "Database Cleared",
                "All track entries removed from SQLite database.",
                4000,
                this
            );
            updateCountAndLayerStatus();
        }
    }
}

} // namespace GISApp::UI::Tracks
