/**
 * @file mainwindow.cpp
 * @brief Implementation of MainWindow UI assembly, dynamic theme menu, and widget signals.
 */

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QMapLibre/Settings>
#include "tools/PanTool.h"
#include "layers/MapLibreLayerAdapter.h"
#include "ui/layertree/LayerTreeFloatingWidget.h"
#include "layers/TacticalLayerProvider.h"
#include <cmath>
#include <algorithm>
#include <QDir>

#include "ui/publishing/PublishLayerDialog.h"
#include "ui/publishing/GroupManagerDialog.h"
#include "ui/tasks/BackgroundTaskDialog.h"
#include "ui/download/DownloadSatImageryDialog.h"
#include "core/SystemConfigManager.h"
#include "core/notifications/NotificationManager.h"

// Track System MVC & SQLite Database Headers
#include "core/database/DatabaseManager.h"
#include "core/repositories/TrackRepository.h"
#include "core/services/MapLibreTrackAdapter.h"
#include "ui/tracks/TracksTableDialog.h"
#include "ui/tracks/TrackDetailDialog.h"
#include "core/repositories/AreaOfViewRepository.h"
#include "core/services/MapLibreAreaOfViewAdapter.h"
#include "core/services/XmlAreaOfViewIngestor.h"
#include "ui/area_of_view/AreaOfViewTableDialog.h"

// EMS Polymorphic Architecture Headers
#include "core/models/GisEntityRegistry.h"
#include "core/repositories/GenericEntityRepository.h"
#include "core/services/MapLibreGenericEntityAdapter.h"
#include "ui/entities/UniversalEntityEditorDialog.h"
#include "core/Udp/mediatorclass.h"
#include "core/Udp/handlers/UdpTrackMessageHandler.h"
#include "core/services/XmlTrackIngestor.h"
#include "core/services/CsvBoundaryIngestor.h"
#include "core/services/MapLibreBoundaryAdapter.h"
#include "ui/boundary/BoundaryTableDialog.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mapWidget(nullptr)
    , m_mapController(nullptr)
    , m_toolManager(nullptr)
    , m_leftSidebar(nullptr)
    , m_rightToolPanel(nullptr)
    , m_zoomControls(nullptr)
    , m_tacticalStatusBar(nullptr)
    , m_themeActionGroup(nullptr)
{
    ui->setupUi(this);

    // Initialize SQLite Database and Repositories
    GISApp::Core::Database::DatabaseManager::instance()->initialize();
    m_trackRepository = new GISApp::Core::Repositories::TrackRepository(this);
    m_areaOfViewRepository = new GISApp::Core::Repositories::AreaOfViewRepository(this);
    m_genericEntityRepository = new GISApp::Core::Repositories::GenericEntityRepository(this);
    
    // Initialize UDP Subsystem & Listener Thread
    m_udpMediator = new MediatorClass(this);


    // Create and Register Track Message Handler (ID: 613)
    if (m_trackRepository && m_udpMediator->dispatcher()) {
        auto trackHandler = std::make_shared<GISApp::Core::Udp::Handlers::UdpTrackMessageHandler>(m_trackRepository);
        m_udpMediator->dispatcher()->registerHandler(trackHandler);
    }

    // 1. Initialize Layout and Overlay Components
    setupMapView();
    setupToolBar();
    setupStatusBar();
    setupThemeMenu();

    // 2. Apply Default Tactical Dark Theme
    GISApp::UI::ThemeManager::instance().applyTheme(GISApp::UI::ThemeType::TacticalDark);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMapView()
{
    QWidget *centralWidget = ui->centralwidget;
    QVBoxLayout *outerLayout = new QVBoxLayout(centralWidget);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // 1. Add Sub-Header Bar below MenuBar
    m_headerBar = new GISApp::UI::HeaderBar(this);
    outerLayout->addWidget(m_headerBar);

    // 2. Main Body Layout (Left Sidebar + Map)
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    m_leftSidebar = new GISApp::UI::LeftSidebar(this);
    bodyLayout->addWidget(m_leftSidebar);

    QWidget *mapContainer = new QWidget(this);
    QVBoxLayout *mapLayout = new QVBoxLayout(mapContainer);
    mapLayout->setContentsMargins(0, 0, 0, 0);
    mapLayout->setSpacing(0);

    QMapLibre::Settings settings(QMapLibre::Settings::MapLibreProvider);
    m_mapWidget = new GISApp::Map::MapLibreWidget(settings, mapContainer);
    mapLayout->addWidget(m_mapWidget);
    bodyLayout->addWidget(mapContainer, 1);

    outerLayout->addLayout(bodyLayout, 1);

    // 3. Floating Overlay Widgets
    m_rightToolPanel = new GISApp::UI::RightToolPanel(m_mapWidget);
    m_zoomControls = new GISApp::UI::ZoomControlsWidget(m_mapWidget);
    m_layerFloatingPanel = new GISApp::UI::LayerTreeFloatingWidget(m_mapWidget);

    // 4. Map Controller & Tools Setup
    m_mapController = new GISApp::Controllers::MapController(m_mapWidget, this);

    m_layerManager = new GISApp::Layers::LayerManager(this);
    if (m_layerFloatingPanel) {
        m_layerFloatingPanel->setLayerManager(m_layerManager);
        connect(m_layerFloatingPanel, &GISApp::UI::LayerTreeFloatingWidget::ingestAreaOfViewRequested,
                this, &MainWindow::onUploadAreaOfViewTriggered);
        connect(m_layerFloatingPanel, &GISApp::UI::LayerTreeFloatingWidget::ingestTracksRequested,
                this, &MainWindow::onUploadTracksTriggered);
    }
    
    // Connect Left Sidebar 'Layers' Button to Toggle Floating Layer Panel
    if (m_leftSidebar) {
        connect(m_leftSidebar, &GISApp::UI::LeftSidebar::actionTriggered, [this](const QString &actionName) {
            if (actionName == "Layers" && m_layerFloatingPanel) {
                m_layerFloatingPanel->setVisible(!m_layerFloatingPanel->isVisible());
            }
        });
    }

    // Wait for the MapLibre Widget to initialize its internal Map object
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer]() {
        if (m_mapWidget && m_mapWidget->map()) {
            timer->stop();
            timer->deleteLater();
            
            // Helper lambda to pan and dynamically zoom to bounding box extent using MapLibre Qt native coordinateZoomForBounds
            auto handlePanToExtent = [this](const GISApp::Layers::LayerExtent &extent) {
                if (m_mapController && extent.isValid()) {
                    if (m_mapWidget && m_mapWidget->map()) {
                        QMapLibre::Coordinate sw(extent.southWest.latitude(), extent.southWest.longitude());
                        QMapLibre::Coordinate ne(extent.northEast.latitude(), extent.northEast.longitude());
                        QMapLibre::CoordinateZoom cz = m_mapWidget->map()->coordinateZoomForBounds(sw, ne, 0.0, 0.0);
                        m_mapWidget->map()->setBearing(0.0);
                        m_mapWidget->map()->setPitch(0.0);
                        m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(cz.first.first, cz.first.second), cz.second);
                    } else {
                        double centerLat = (extent.southWest.latitude() + extent.northEast.latitude()) / 2.0;
                        double centerLon = (extent.southWest.longitude() + extent.northEast.longitude()) / 2.0;
                        m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(centerLat, centerLon), 12.0);
                    }
                }
            };

            if (m_layerFloatingPanel && m_layerFloatingPanel->treeView()) {
                connect(m_layerFloatingPanel->treeView(), &GISApp::UI::LayerTreeView::zoomToExtentRequested, handlePanToExtent);
            }
            if (m_layerManager) {
                connect(m_layerManager, &GISApp::Layers::LayerManager::panToExtentRequested, handlePanToExtent);
            }
            
            // Connect model updates to force map view repaints for instant visibility & opacity updates
            if (m_layerManager && m_layerManager->model()) {
                connect(m_layerManager->model(), &QAbstractItemModel::layoutChanged, [this]() {
                    if (m_mapWidget) m_mapWidget->updateMap();
                });
                connect(m_layerManager->model(), &QAbstractItemModel::dataChanged, [this]() {
                    if (m_mapWidget) m_mapWidget->updateMap();
                });
            }
            
            // Connect style load to tactical layer population and track rendering
            connect(m_mapWidget->map(), &QMapLibre::Map::mapChanged, [this](QMapLibre::Map::MapChange change) {
                if (change == QMapLibre::Map::MapChangeDidFinishLoadingStyle) {
                    if (!m_trackAdapter && m_trackRepository) {
                        m_trackAdapter = new GISApp::Core::Services::MapLibreTrackAdapter(m_mapWidget->map(), m_trackRepository, this);
                        m_trackAdapter->setLayerManager(m_layerManager);
                    } else if (m_trackAdapter) {
                        m_trackAdapter->setMap(m_mapWidget->map());
                        m_trackAdapter->setLayerManager(m_layerManager);
                    }

                    if (!m_areaOfViewAdapter && m_areaOfViewRepository) {
                        m_areaOfViewAdapter = new GISApp::Core::Services::MapLibreAreaOfViewAdapter(m_mapWidget->map(), m_areaOfViewRepository, this);
                    } else if (m_areaOfViewAdapter) {
                        m_areaOfViewAdapter->setMap(m_mapWidget->map());
                    }

                    if (!m_genericEntityAdapter && m_genericEntityRepository) {
                        m_genericEntityAdapter = new GISApp::Core::Services::MapLibreGenericEntityAdapter(m_mapWidget->map(), m_genericEntityRepository, this);
                        m_genericEntityAdapter->setLayerManager(m_layerManager);
                    } else if (m_genericEntityAdapter) {
                        m_genericEntityAdapter->setMap(m_mapWidget->map());
                        m_genericEntityAdapter->setLayerManager(m_layerManager);
                    }

                    if (!m_boundaryAdapter) {
                        m_boundaryAdapter = new GISApp::Core::Services::MapLibreBoundaryAdapter(m_mapWidget->map(), this);
                    } else {
                        m_boundaryAdapter->setMap(m_mapWidget->map());
                    }
                    m_userBoundaries = m_boundaryAdapter->loadSavedBoundaries();
                    m_boundaryAdapter->setBoundaries(m_userBoundaries);

                    GISApp::Layers::TacticalLayerProvider p;
                    p.setupTacticalLayers(m_mapWidget->map());
                    p.populateLayerTree(m_layerManager, m_mapWidget->map(), m_mapController);

                    m_mapWidget->updateMap();
                    QTimer::singleShot(200, this, &MainWindow::focusOnAreaOfView);
                    QTimer::singleShot(600, this, &MainWindow::focusOnAreaOfView);
                }
            });

            // Set Initial Offline Tactical Dark Style and Camera
            QString offlineStyle = QString("file://%1").arg(GISApp::Core::SystemConfigManager::instance().getOfflineStylePath());
            m_mapController->setStyle(offlineStyle);
            QTimer::singleShot(200, this, &MainWindow::focusOnAreaOfView);
            QTimer::singleShot(600, this, &MainWindow::focusOnAreaOfView);
        }
    });
    timer->start(50);

    m_toolManager = new GISApp::Controllers::ToolManager(this);
    m_toolManager->registerDefaultTools(this);

    m_mapWidget->installEventFilter(this);

    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseCoordinateChanged,
            this, &MainWindow::onMouseCoordinateChanged);
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::cameraChanged,
            [this](double zoomLevel, double scaleDenominator, const GISApp::Core::Models::GeoCoordinate &center) {
                Q_UNUSED(center);
                if (m_tacticalStatusBar) {
                    m_tacticalStatusBar->updateZoomAndScale(zoomLevel, scaleDenominator);
                }
            });
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mousePressed,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMousePress);
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseMoved,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMouseMove);
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseReleased,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMouseRelease);
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::customContextMenuRequested,
            this, &MainWindow::onMapContextMenuRequested);

    connect(m_zoomControls, &GISApp::UI::ZoomControlsWidget::zoomInRequested, [this]() {
        if (m_mapController) m_mapController->zoomIn();
    });
    connect(m_zoomControls, &GISApp::UI::ZoomControlsWidget::zoomOutRequested, [this]() {
        if (m_mapController) m_mapController->zoomOut();
    });
    connect(m_zoomControls, &GISApp::UI::ZoomControlsWidget::resetCenterRequested, [this]() {
        focusOnAreaOfView();
    });
}

void MainWindow::updateOverlayPositions()
{
    if (!m_mapWidget) return;
    if (m_layerFloatingPanel) {
        m_layerFloatingPanel->adjustSize();
        m_layerFloatingPanel->move(16, 16);
        m_layerFloatingPanel->show();
        m_layerFloatingPanel->raise();
    }
    if (m_rightToolPanel) {
        m_rightToolPanel->adjustSize();
        int x = m_mapWidget->width() - m_rightToolPanel->width() - 16;
        int y = 16;
        m_rightToolPanel->move(x, y);
        m_rightToolPanel->raise();
    }
    if (m_zoomControls) {
        m_zoomControls->adjustSize();
        int x = 16;
        int y = m_mapWidget->height() - m_zoomControls->height() - 16;
        m_zoomControls->move(x, y);
        m_zoomControls->raise();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_mapWidget && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
        updateOverlayPositions();
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateOverlayPositions();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    updateOverlayPositions();
}

void MainWindow::setupToolBar()
{
}

void MainWindow::setupStatusBar()
{
    m_tacticalStatusBar = new GISApp::UI::TacticalStatusBar(this);
    setStatusBar(m_tacticalStatusBar);
}

void MainWindow::setupThemeMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&FILE"));
    QMenu *editMenu = menuBar()->addMenu(tr("&EDIT"));
    QMenu *viewMenu = menuBar()->addMenu(tr("&VIEW"));
    QMenu *toolsMenu = menuBar()->addMenu(tr("&TOOLS"));
    QMenu *themeMenu = menuBar()->addMenu(tr("THE&MES"));
    QMenu *windowMenu = menuBar()->addMenu(tr("&WINDOW"));
    QMenu *helpMenu = menuBar()->addMenu(tr("&HELP"));
    QMenu *layersMenu = menuBar()->addMenu(tr("&LAYERS"));

    QAction *publishAction = layersMenu->addAction(tr("🚀 Publish Layer..."));
    QAction *manageGroupsAction = layersMenu->addAction(tr("📁 Manage Layer Groups..."));
    connect(publishAction, &QAction::triggered, [this]() {
        if (m_mapWidget && m_mapWidget->map()) {
            if (!m_publishDialog) {
                m_publishDialog = new GISApp::UI::Publishing::PublishLayerDialog(m_layerManager, m_mapWidget->map(), this);
                m_publishDialog->setAttribute(Qt::WA_DeleteOnClose);
                connect(m_publishDialog, &QDialog::destroyed, [this]() { m_publishDialog = nullptr; });
                m_publishDialog->show();
            } else {
                m_publishDialog->show();
                m_publishDialog->raise();
                m_publishDialog->activateWindow();
            }
        }
    });
    connect(manageGroupsAction, &QAction::triggered, [this]() {
        if (m_layerManager) {
            if (!m_groupManagerDialog) {
                m_groupManagerDialog = new GISApp::UI::Publishing::GroupManagerDialog(m_layerManager, this);
                m_groupManagerDialog->setAttribute(Qt::WA_DeleteOnClose);
                connect(m_groupManagerDialog, &QDialog::destroyed, [this]() { m_groupManagerDialog = nullptr; });
                m_groupManagerDialog->show();
            } else {
                m_groupManagerDialog->show();
                m_groupManagerDialog->raise();
                m_groupManagerDialog->activateWindow();
            }
        }
    });

    QAction *bgTasksAction = toolsMenu->addAction(tr("⚙️ Background Spatial Tasks Monitor..."));
    bgTasksAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(bgTasksAction, &QAction::triggered, [this]() {
        if (!m_backgroundTaskDialog) {
            m_backgroundTaskDialog = new GISApp::UI::Tasks::BackgroundTaskDialog(this);
            m_backgroundTaskDialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(m_backgroundTaskDialog, &QDialog::destroyed, [this]() { m_backgroundTaskDialog = nullptr; });
            m_backgroundTaskDialog->show();
        } else {
            m_backgroundTaskDialog->show();
            m_backgroundTaskDialog->raise();
            m_backgroundTaskDialog->activateWindow();
        }
    });

    toolsMenu->addSeparator();
    QAction *flashNotifAction = toolsMenu->addAction(tr("🔔 Test Flash Notification (5s Auto-Vanish)"));
    connect(flashNotifAction, &QAction::triggered, [this]() {
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Spatial Indexing Complete",
            "Vector dataset spatial cache loaded successfully in background thread.",
            5000,
            this
        );
    });

    QAction *criticalNotifAction = toolsMenu->addAction(tr("🚨 Test Critical Notification (User Ack Required)"));
    connect(criticalNotifAction, &QAction::triggered, [this]() {
        GISApp::Core::Notifications::NotificationManager::instance()->notifyCritical(
            "GPS Telemetry Connection Lost",
            "Lost connection to high-precision NMEA receiver. Please verify hardware port configuration immediately.",
            this
        );
    });

    QMenu *entitiesMenu = menuBar()->addMenu(tr("&ENTITIES"));
    QAction *viewTracksAction = entitiesMenu->addAction(tr("🎯 Tracks..."));
    connect(viewTracksAction, &QAction::triggered, this, &MainWindow::onViewTracksTriggered);
    QAction *viewAoVAction = entitiesMenu->addAction(tr("👁️ Area of View..."));
    connect(viewAoVAction, &QAction::triggered, this, &MainWindow::onViewAreaOfViewTriggered);
    QAction *viewBoundaryAction = entitiesMenu->addAction(tr("🇮🇳 National Boundary..."));
    connect(viewBoundaryAction, &QAction::triggered, this, &MainWindow::onViewBoundaryTriggered);

    QMenu *uploadMenu = menuBar()->addMenu(tr("&UPLOAD"));
    QAction *uploadTracksAction = uploadMenu->addAction(tr("📥 Tracks (XML)..."));
    connect(uploadTracksAction, &QAction::triggered, this, &MainWindow::onUploadTracksTriggered);
    QAction *uploadAoVAction = uploadMenu->addAction(tr("📥 Area of View (XML)..."));
    connect(uploadAoVAction, &QAction::triggered, this, &MainWindow::onUploadAreaOfViewTriggered);
    QAction *uploadBoundaryAction = uploadMenu->addAction(tr("📥 National Boundary (CSV)..."));
    connect(uploadBoundaryAction, &QAction::triggered, this, &MainWindow::onUploadBoundaryTriggered);

    QMenu *downloadMenu = menuBar()->addMenu(tr("&DOWNLOAD"));
    QAction *downloadSatAction = downloadMenu->addAction(tr("🌐 Download Google Sat Imagery..."));
    connect(downloadSatAction, &QAction::triggered, this, &MainWindow::onDownloadGoogleSatTriggered);

    (void)fileMenu; (void)editMenu; (void)viewMenu; (void)toolsMenu; (void)windowMenu; (void)helpMenu;

    QLabel *termLabel = new QLabel("PRECISION TERMINAL V2.4  ", this);
    termLabel->setStyleSheet("color: #6b7280; font-size: 11px; font-weight: bold;");
    menuBar()->setCornerWidget(termLabel, Qt::TopRightCorner);

    m_themeActionGroup = new QActionGroup(this);
    m_themeActionGroup->setExclusive(true);

    const std::vector<GISApp::UI::ThemeType> themes = {
        GISApp::UI::ThemeType::TacticalDark,
        GISApp::UI::ThemeType::CyberEmerald,
        GISApp::UI::ThemeType::MidnightBlue,
        GISApp::UI::ThemeType::HighContrastDark,
        GISApp::UI::ThemeType::LightOps
    };

    for (auto theme : themes) {
        QString name = GISApp::UI::ThemeManager::themeName(theme);
        QAction *action = themeMenu->addAction(name);
        action->setCheckable(true);
        m_themeActionGroup->addAction(action);

        if (theme == GISApp::UI::ThemeType::TacticalDark) {
            action->setChecked(true);
        }

        connect(action, &QAction::triggered, [theme]() {
            GISApp::UI::ThemeManager::instance().applyTheme(theme);
        });
    }
}

void MainWindow::onDownloadGoogleSatTriggered()
{
    if (!m_downloadSatDialog) {
        m_downloadSatDialog = new GISApp::UI::Download::DownloadSatImageryDialog(m_mapWidget, this);
        m_downloadSatDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_downloadSatDialog, &QDialog::destroyed, [this]() { m_downloadSatDialog = nullptr; });
        m_downloadSatDialog->show();
    } else {
        m_downloadSatDialog->show();
        m_downloadSatDialog->raise();
        m_downloadSatDialog->activateWindow();
    }
}

void MainWindow::onMouseCoordinateChanged(const GISApp::Core::Models::GeoCoordinate &coordinate)
{
    if (m_tacticalStatusBar) {
        m_tacticalStatusBar->updateCoordinates(coordinate);
    }
}

void MainWindow::onDistanceUpdated(double totalDistanceKm)
{
    if (m_tacticalStatusBar) {
        m_tacticalStatusBar->showMessage(QString("Measurement: %1 km").arg(totalDistanceKm, 0, 'f', 2));
    }
}

void MainWindow::onUploadTracksTriggered()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Tracks XML File"),
        QDir::homePath(),
        tr("XML Files (*.xml);;All Files (*)")
        );
    if (filePath.isEmpty()) return;
    if (!m_trackRepository) {
        qWarning() << "[MainWindow] Track repository is null.";
        return;
    }
    GISApp::Core::Services::XmlTrackIngestor ingestor;
    int imported = ingestor.ingest(filePath, *m_trackRepository);
    if (imported >= 0) {
        if (m_layerManager && !m_layerManager->findLayerByLayerId("tracks-circle-layer")) {
            auto tacticalGroup = m_layerManager->findGroupByName("Tactical Operations");
            if (!tacticalGroup) tacticalGroup = m_layerManager->addGroup("🛡️ Tactical Operations");
            GISApp::Layers::LayerExtent indiaExtent{
                GISApp::Core::Models::GeoCoordinate(8.4, 68.7),
                GISApp::Core::Models::GeoCoordinate(37.6, 97.25)
            };
            auto tracksAdapterNode = std::make_shared<GISApp::Layers::MapLibreLayerAdapter>(
                "tracks-circle-layer", m_mapWidget ? m_mapWidget->map() : nullptr, indiaExtent);
            m_layerManager->addLayer("🎯 Tactical Tracks", tracksAdapterNode, tacticalGroup);
        }
        if (m_trackAdapter) {
            m_trackAdapter->refreshFromRepository();
        }
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "XML Ingestion Complete",
            QString("Successfully imported %1 dynamic tracks from XML into SQLite database.").arg(imported),
            5000,
            this
            );
    } else {
        QMessageBox::critical(this, tr("Import Error"), tr("Failed to parse and import Tracks XML file."));
    }
}

void MainWindow::onViewTracksTriggered()
{
    if (!m_tracksDialog) {
        m_tracksDialog = new GISApp::UI::Tracks::TracksTableDialog(
            m_trackRepository,
            m_layerManager,
            m_mapWidget ? m_mapWidget->map() : nullptr,
            m_mapController,
            m_trackAdapter,
            this
        );
        m_tracksDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_tracksDialog, &QDialog::destroyed, [this]() { m_tracksDialog = nullptr; });
        m_tracksDialog->show();
    } else {
        m_tracksDialog->show();
        m_tracksDialog->raise();
        m_tracksDialog->activateWindow();
    }
}



void MainWindow::onUploadAreaOfViewTriggered()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Area of View XML File"),
        QDir::homePath(),
        tr("XML Files (*.xml);;All Files (*)")
    );

    if (filePath.isEmpty()) return;

    if (!m_areaOfViewRepository) {
        qWarning() << "[MainWindow] Area of View repository is null.";
        return;
    }

    GISApp::Core::Services::XmlAreaOfViewIngestor ingestor(m_areaOfViewRepository, this);
    bool ok = ingestor.parseAndSaveXmlFile(filePath);

    if (ok) {
        if (m_layerManager && !m_layerManager->findLayerByLayerId("area-of-view-fill-layer")) {
            auto tacticalGroup = m_layerManager->findGroupByName("Tactical Operations");
            if (!tacticalGroup) tacticalGroup = m_layerManager->addGroup("🛡️ Tactical Operations");

            GISApp::Layers::LayerExtent indiaExtent{
                GISApp::Core::Models::GeoCoordinate(8.4, 68.7),
                GISApp::Core::Models::GeoCoordinate(37.6, 97.25)
            };
            auto aovAdapterNode = std::make_shared<GISApp::Layers::MapLibreLayerAdapter>(
                "area-of-view-fill-layer", m_mapWidget ? m_mapWidget->map() : nullptr, indiaExtent);
            m_layerManager->addLayer("👁️ Area of View", aovAdapterNode, tacticalGroup);
        }

        if (m_areaOfViewAdapter) {
            m_areaOfViewAdapter->refreshFromRepository();
        }
        focusOnAreaOfView();

        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Area of View XML Ingested",
            QString("Successfully ingested Area of View polygon data from XML into SQLite database."),
            5000,
            this
        );
    } else {
        QMessageBox::critical(this, tr("Import Error"), tr("Failed to parse and import Area of View XML file."));
    }
}

void MainWindow::onViewAreaOfViewTriggered()
{
    auto dialog = new GISApp::UI::AreaOfView::AreaOfViewTableDialog(
        m_areaOfViewRepository,
        m_mapController,
        m_areaOfViewAdapter,
        this
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::onUploadBoundaryTriggered()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select National Boundary CSV File"),
        QDir::homePath(),
        tr("CSV Files (*.csv);;All Files (*)")
    );

    if (filePath.isEmpty()) return;

    GISApp::Core::Services::CsvBoundaryIngestor ingestor;
    auto boundaries = ingestor.ingestCsv(filePath);

    if (!boundaries.isEmpty()) {
        m_userBoundaries = boundaries;
        if (!m_boundaryAdapter && m_mapWidget && m_mapWidget->map()) {
            m_boundaryAdapter = new GISApp::Core::Services::MapLibreBoundaryAdapter(m_mapWidget->map(), this);
        }
        if (m_boundaryAdapter) {
            m_boundaryAdapter->setBoundaries(m_userBoundaries);
        }
        if (m_boundaryDialog) {
            m_boundaryDialog->setBoundaries(m_userBoundaries);
        }

        if (m_mapController && !boundaries.first().points.isEmpty()) {
            double minLat = 90.0, maxLat = -90.0;
            double minLon = 180.0, maxLon = -180.0;
            for (const auto &pt : boundaries.first().points) {
                if (pt.latitude < minLat) minLat = pt.latitude;
                if (pt.latitude > maxLat) maxLat = pt.latitude;
                if (pt.longitude < minLon) minLon = pt.longitude;
                if (pt.longitude > maxLon) maxLon = pt.longitude;
            }
            double centerLat = (minLat + maxLat) / 2.0;
            double centerLon = (minLon + maxLon) / 2.0;
            m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(centerLat, centerLon), 6.5);
        }

        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Boundary CSV Ingested",
            QString("Successfully loaded %1 boundary line features with Green/Saffron dual stroke.").arg(boundaries.size()),
            5000,
            this
        );
    } else {
        QMessageBox::critical(this, tr("Import Error"), tr("Failed to parse and import Boundary CSV file."));
    }
}

void MainWindow::onViewBoundaryTriggered()
{
    if (!m_boundaryDialog) {
        m_boundaryDialog = new GISApp::UI::Boundary::BoundaryTableDialog(
            m_userBoundaries,
            m_mapController,
            m_boundaryAdapter,
            this
        );
        m_boundaryDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_boundaryDialog, &QDialog::destroyed, [this]() { m_boundaryDialog = nullptr; });
        connect(m_boundaryDialog, &GISApp::UI::Boundary::BoundaryTableDialog::boundaryCleared, [this]() {
            m_userBoundaries.clear();
        });
        m_boundaryDialog->show();
    } else {
        m_boundaryDialog->setBoundaries(m_userBoundaries);
        m_boundaryDialog->show();
        m_boundaryDialog->raise();
        m_boundaryDialog->activateWindow();
    }
}

void MainWindow::focusOnAreaOfView()
{
    if (!m_areaOfViewRepository || !m_mapController) {
        return;
    }

    auto records = m_areaOfViewRepository->getAll();
    if (records.isEmpty()) {
        m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(28.6139, 77.2090), 10.0);
        return;
    }

    const auto &rec = records.last();
    if (rec.points.isEmpty()) {
        return;
    }

    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;
    for (const auto &pt : rec.points) {
        if (pt.latitude < minLat) minLat = pt.latitude;
        if (pt.latitude > maxLat) maxLat = pt.latitude;
        if (pt.longitude < minLon) minLon = pt.longitude;
        if (pt.longitude > maxLon) maxLon = pt.longitude;
    }

    GISApp::Layers::LayerExtent polyExtent{
        GISApp::Core::Models::GeoCoordinate(minLat, minLon),
        GISApp::Core::Models::GeoCoordinate(maxLat, maxLon)
    };

    if (m_layerManager) {
        auto aovNode = m_layerManager->findLayerByLayerId("area-of-view-fill-layer");
        if (aovNode && aovNode->adapter()) {
            aovNode->adapter()->setExtent(polyExtent);
        }
    }

    if (m_mapWidget && m_mapWidget->map()) {
        QMapLibre::Coordinate sw(minLat, minLon);
        QMapLibre::Coordinate ne(maxLat, maxLon);
        QMapLibre::CoordinateZoom cz = m_mapWidget->map()->coordinateZoomForBounds(sw, ne, 0.0, 0.0);
        m_mapWidget->map()->setBearing(0.0);
        m_mapWidget->map()->setPitch(0.0);
        m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(cz.first.first, cz.first.second), cz.second);
        qDebug() << "[MainWindow] Focused map using MapLibre coordinateZoomForBounds:" << rec.name
                 << "| Bounds: [" << minLat << "," << minLon << "] to [" << maxLat << "," << maxLon << "]"
                 << "| Center:" << cz.first.first << cz.first.second << "| Exact Fit Zoom:" << cz.second;
    } else {
        double centerLat = (minLat + maxLat) / 2.0;
        double centerLon = (minLon + maxLon) / 2.0;
        m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(centerLat, centerLon), 12.0);
    }
}

void MainWindow::onMapContextMenuRequested(const QPoint &globalPos, const QPoint &localPos, const GISApp::Core::Models::GeoCoordinate &coordinate)
{
    if (!m_mapWidget || !m_mapWidget->map()) return;

    // 1. Spatial Hit-Test active tracks in repository
    GISApp::Core::Repositories::TrackRepository trackRepo;
    auto tracks = trackRepo.getAllTracks();

    QVector<GISApp::Core::Models::TrackRecord> hitTracks;
    const double clickPxX = localPos.x();
    const double clickPxY = localPos.y();
    const double tolerancePixels = 15.0; // 15-pixel click radius

    for (const auto &tr : tracks) {
        QPointF pt = m_mapWidget->map()->pixelForCoordinate({tr.trackLat, tr.trackLong});
        double dx = pt.x() - clickPxX;
        double dy = pt.y() - clickPxY;
        double dist = std::hypot(dx, dy);
        if (dist <= tolerancePixels) {
            hitTracks.append(tr);
        }
    }

    // 2. Build dark-themed QMenu
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #0F172A; color: #F8FAFC; border: 1px solid #334155; border-radius: 6px; padding: 4px; }"
        "QMenu::item { padding: 6px 20px; border-radius: 4px; font-size: 12px; }"
        "QMenu::item:selected { background-color: #2563EB; color: #FFFFFF; }"
        "QMenu::separator { height: 1px; background: #334155; margin: 4px 0px; }"
        "QMenu::right-arrow { margin: 5px; }"
    );

    // Track Hit Resolution
    if (hitTracks.size() == 1) {
        const auto singleTrack = hitTracks.first();
        QString trackTitle = singleTrack.trackName.isEmpty() ? QString("Track #%1").arg(singleTrack.trackId) : singleTrack.trackName;
        
        QAction *detailAct = menu.addAction(QString("🎯 Show Detail: %1").arg(trackTitle));
        connect(detailAct, &QAction::triggered, [this, singleTrack]() {
            showTrackDetailsDialog(singleTrack);
        });

        QAction *zoomAct = menu.addAction(QString("🔍 Zoom to %1").arg(trackTitle));
        connect(zoomAct, &QAction::triggered, [this, singleTrack]() {
            if (m_mapController) {
                m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(singleTrack.trackLat, singleTrack.trackLong), 14.0);
            }
        });

        menu.addSeparator();
    }
    else if (hitTracks.size() > 1) {
        QMenu *subMenu = menu.addMenu(QString("🎯 Select Track / Show Details (%1 Overlapping) ▶").arg(hitTracks.size()));
        subMenu->setStyleSheet(menu.styleSheet());

        for (const auto &tr : hitTracks) {
            QString name = tr.trackName.isEmpty() ? QString("Track #%1").arg(tr.trackId) : tr.trackName;
            QString label = QString("✈️ %1  (Alt: %2m, Dir: %3°)").arg(name).arg(tr.trackHeight, 0, 'f', 0).arg(tr.trackDir, 0, 'f', 0);
            QAction *trAct = subMenu->addAction(label);
            connect(trAct, &QAction::triggered, [this, tr]() {
                showTrackDetailsDialog(tr);
            });
        }

        QAction *zoomClusterAct = menu.addAction(QString("🔍 Zoom to Track Cluster (%1 Entities)").arg(hitTracks.size()));
        connect(zoomClusterAct, &QAction::triggered, [this, hitTracks]() {
            double minLat = 90.0, maxLat = -90.0, minLon = 180.0, maxLon = -180.0;
            for (const auto &tr : hitTracks) {
                if (tr.trackLat < minLat) minLat = tr.trackLat;
                if (tr.trackLat > maxLat) maxLat = tr.trackLat;
                if (tr.trackLong < minLon) minLon = tr.trackLong;
                if (tr.trackLong > maxLon) maxLon = tr.trackLong;
            }
            if (std::abs(maxLat - minLat) < 0.001) { minLat -= 0.005; maxLat += 0.005; }
            if (std::abs(maxLon - minLon) < 0.001) { minLon -= 0.005; maxLon += 0.005; }
            
            QMapLibre::Coordinate sw(minLat, minLon);
            QMapLibre::Coordinate ne(maxLat, maxLon);
            auto cz = m_mapWidget->map()->coordinateZoomForBounds(sw, ne);
            if (m_mapController) {
                m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(cz.first.first, cz.first.second), cz.second);
            }
        });

        menu.addSeparator();
    }

    // General Map Actions (Copy & Show Coordinates)
    QString coordStr = QString("%1, %2")
                           .arg(coordinate.latitude(), 0, 'f', 6)
                           .arg(coordinate.longitude(), 0, 'f', 6);

    QAction *copyAct = menu.addAction(QString("📋 Copy Coordinates (%1)").arg(coordStr));
    connect(copyAct, &QAction::triggered, [this, coordStr]() {
        QGuiApplication::clipboard()->setText(coordStr);
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Clipboard", QString("Coordinates copied to clipboard: %1").arg(coordStr));
    });

    QAction *showAct = menu.addAction("🗺️ Show Coordinates");
    connect(showAct, &QAction::triggered, [this, coordinate]() {
        showCoordinatesDialog(coordinate);
    });

    menu.addSeparator();

    // EMS Generic Entity Context Actions
    if (m_genericEntityRepository) {
        auto allEntities = m_genericEntityRepository->findAll();
        for (const auto &entity : allEntities) {
            if (!entity || !entity->geometry()) continue;
            auto pt = std::dynamic_pointer_cast<GISApp::Core::Models::PointGeometry>(entity->geometry());
            if (pt) {
                double dist = std::hypot(pt->coordinate().latitude - coordinate.latitude(), pt->coordinate().longitude - coordinate.longitude());
                if (dist < 0.05) {
                    QAction *editAct = menu.addAction(QString("✏️ Edit EMS Entity: %1").arg(entity->entityName()));
                    connect(editAct, &QAction::triggered, [this, entity]() {
                        GISApp::UI::Entities::UniversalEntityEditorDialog dlg(entity, this);
                        if (dlg.exec() == QDialog::Accepted) {
                            if (m_genericEntityRepository) {
                                m_genericEntityRepository->updateEntity(entity);
                                GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                                    "EMS Entity", QString("Updated entity: %1").arg(entity->entityName()));
                            }
                        }
                    });

                    QAction *delAct = menu.addAction(QString("🗑️ Delete EMS Entity: %1").arg(entity->entityName()));
                    connect(delAct, &QAction::triggered, [this, entity]() {
                        if (m_genericEntityRepository) {
                            m_genericEntityRepository->removeEntity(entity->entityId());
                            GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                                "EMS Entity", QString("Deleted entity: %1").arg(entity->entityName()));
                        }
                    });
                }
            }
        }
    }

    QMenu *addEntityMenu = menu.addMenu("➕ Create Custom EMS Entity");
    addEntityMenu->setStyleSheet(menu.styleSheet());

    auto descriptors = GISApp::Core::Models::GisEntityRegistry::instance().registeredTypes();
    for (const auto &desc : descriptors) {
        QAction *actAdd = addEntityMenu->addAction(QString("%1 (%2)").arg(desc.displayName, desc.typeId));
        connect(actAdd, &QAction::triggered, [this, desc, coordinate]() {
            auto geom = std::make_shared<GISApp::Core::Models::PointGeometry>(coordinate.latitude(), coordinate.longitude());
            auto entity = GISApp::Core::Models::GisEntityRegistry::instance().createEntity(desc.typeId, QString("New %1").arg(desc.displayName));
            if (entity) {
                entity->setGeometry(geom);
                GISApp::UI::Entities::UniversalEntityEditorDialog dlg(entity, this);
                if (dlg.exec() == QDialog::Accepted) {
                    if (m_genericEntityRepository) {
                        m_genericEntityRepository->addEntity(entity);
                        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                            "EMS Entity", QString("Saved %1 entity: %2").arg(desc.displayName, entity->entityName()));
                    }
                }
            }
        });
    }

    menu.exec(globalPos);
}

void MainWindow::showTrackDetailsDialog(const GISApp::Core::Models::TrackRecord &track)
{
    auto dlg = new GISApp::UI::Tracks::TrackDetailDialog(track, this);
    connect(dlg, &GISApp::UI::Tracks::TrackDetailDialog::zoomToTrackRequested,
            [this](const GISApp::Core::Models::TrackRecord &tr) {
                if (m_mapController) {
                    m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(tr.trackLat, tr.trackLong), 14.0);
                }
            });
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::showCoordinatesDialog(const GISApp::Core::Models::GeoCoordinate &coordinate)
{
    auto formatDms = [](double val, bool isLat) {
        char dir = isLat ? (val >= 0 ? 'N' : 'S') : (val >= 0 ? 'E' : 'W');
        val = std::abs(val);
        int deg = static_cast<int>(val);
        double minVal = (val - deg) * 60.0;
        int min = static_cast<int>(minVal);
        double sec = (minVal - min) * 60.0;
        return QString("%1° %2' %3\" %4").arg(deg).arg(min, 2, 10, QChar('0')).arg(sec, 5, 'f', 2, QChar('0')).arg(dir);
    };

    QString msg = QString("<b>Decimal Degrees:</b><br>%1, %2<br><br><b>DMS Format:</b><br>%3, %4")
                      .arg(coordinate.latitude(), 0, 'f', 6)
                      .arg(coordinate.longitude(), 0, 'f', 6)
                      .arg(formatDms(coordinate.latitude(), true))
                      .arg(formatDms(coordinate.longitude(), false));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Map Coordinates");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(msg);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #0F172A; color: #F8FAFC; }"
        "QLabel { color: #F8FAFC; font-size: 13px; }"
        "QPushButton { background-color: #2563EB; color: white; border-radius: 4px; padding: 6px 14px; font-weight: bold; }"
    );
    msgBox.exec();
}
