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
#include <QShortcut>
#include <QRandomGenerator>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include "src/ui/udl/UdlEntityStyleDialog.h"

#include "ui/publishing/PublishLayerDialog.h"
#include "ui/publishing/GroupManagerDialog.h"
#include "ui/tasks/BackgroundTaskDialog.h"
#include "ui/download/DownloadSatImageryDialog.h"
#include "core/SystemConfigManager.h"
#include "core/notifications/NotificationManager.h"

#include "core/Udp/handlers/UdpSampleEntityMessageHandler.h"
#include "core/Udp/UdpMessages/SampleEntityMessage.h"

// Track System MVC & SQLite Database Headers
#include "core/database/DatabaseManager.h"
#include "core/repositories/TrackRepository.h"
#include "core/services/MapLibreTrackAdapter.h"
#include "ui/tracks/TracksTableDialog.h"
#include "ui/tracks/TrackDetailDialog.h"
#include "core/repositories/AreaOfViewRepository.h"
#include "core/services/MapLibreAreaOfViewAdapter.h"

// UDL Component Headers
#include "src/ui/udl/CreateUdlLayerDialog.h"
#include "src/ui/udl/UdlToolbarWidget.h"
#include "src/ui/udl/UdlEntityTableDialog.h"
#include "src/tools/UdlDrawingTool.h"
#include "src/publishing/UdlRepositoryManager.h"
#include "publishing/LayerRegistryManager.h"
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
#include "core/Udp/handlers/UdpSensorMessageHandler.h"

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

    //-EXPERIEMENT BLOCK
    // Default ExpTable creation
    m_expTable = new GISApp::Experiment::ExpTable("exp_telemetry_table", this);

    // Default ExpMessage creation with ExpTable attached
    m_expMessage = new GISApp::Experiment::ExpMessage(m_expTable, this);

    // Default ExpLayer creation with layer name, group, and query (hardcoded: display all data)
    m_expLayer = new GISApp::Experiment::ExpLayer("Experiment Telemetry", "EXP group", "ALL", m_expTable, this);

    // Connect ExpLayer updated signal to map update delegate
    connect(m_expLayer, &GISApp::Experiment::ExpLayer::layerUpdated,
            this, [this](const QString &layerName, const QString &geoJsonPath) {
                this->onUdlLayerUpdated("vector-" + QString::number(qHash(layerName)), geoJsonPath);
            });

    // Directly receive raw UDP payload via signal-slot bypassing UdpMessageDispatcher
    if (m_udpMediator) {
        connect(m_udpMediator, &MediatorClass::rawUdpPayloadReceived,
                this, [this](const QByteArray &rawPayload) {
                    if (static_cast<size_t>(rawPayload.size()) < sizeof(STRUCT_MESSAGE_HEADER)) return;
                    STRUCT_MESSAGE_HEADER sysHeader;
                    std::memcpy(&sysHeader, rawPayload.constData(), sizeof(STRUCT_MESSAGE_HEADER));

                    if (sysHeader.message_id == EXP_MESSAGE_ID) {
                        qDebug() << "[MainWindow] 📡 Received EXP_MESSAGE_ID (903) datagram via direct signal-slot connection";
                        if (m_expMessage) {
                            m_expMessage->parseAndSaveToDb(rawPayload);
                        }
                    }
                });
    }
    //--EXPERIEMENT BLOCK


    // Create and Register Track Message Handler (ID: 613)
    if (m_trackRepository && m_udpMediator->dispatcher()) {
        auto trackHandler = std::make_shared<GISApp::Core::Udp::Handlers::UdpTrackMessageHandler>(m_trackRepository);
        m_udpMediator->dispatcher()->registerHandler(trackHandler);
    }

    // Create and Register Sensor Message Handler (ID: 902)
    if (m_udpMediator && m_udpMediator->dispatcher()) {
        auto sensorHandler = std::make_shared<GISApp::Core::Udp::Handlers::UdpSensorMessageHandler>();
        m_udpMediator->dispatcher()->registerHandler(sensorHandler);
    }

    // Universal signal connection: Any telemetry handler registered with UdpMessageDispatcher automatically updates MapLibre!
    if (m_udpMediator && m_udpMediator->dispatcher()) {
        connect(m_udpMediator->dispatcher(), &GISApp::Core::Udp::Handlers::UdpMessageDispatcher::telemetryLayerUpdated,
                this, [this](const QString &layerName, const QString &geoJsonPath) {
                    this->onUdlLayerUpdated("vector-" + QString::number(qHash(layerName)), geoJsonPath);
                });
    }



    // 1. Initialize Layout and Overlay Components
    setupMapView();
    setupToolBar();
    setupStatusBar();
    setupThemeMenu();
    setupUdlMenu();










    // Connect UdlRepositoryManager to MapLibre GeoJSON update handler
    connect(&GISApp::Publishing::UdlRepositoryManager::instance(), &GISApp::Publishing::UdlRepositoryManager::udlLayerUpdated,
            this, &MainWindow::onUdlLayerUpdated);

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
                    if (!m_styleDebounceTimer) {
                        m_styleDebounceTimer = new QTimer(this);
                        m_styleDebounceTimer->setSingleShot(true);
                        connect(m_styleDebounceTimer, &QTimer::timeout, this, [this]() {
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

                            GISApp::Layers::TacticalLayerProvider p;
                            p.setupTacticalLayers(m_mapWidget->map());
                            p.populateLayerTree(m_layerManager, m_mapWidget->map(), m_mapController);

                            restoreCustomEntityLayersAndGroups();

                            if (!m_boundaryAdapter) {
                                m_boundaryAdapter = new GISApp::Core::Services::MapLibreBoundaryAdapter(m_mapWidget->map(), this);
                            } else {
                                m_boundaryAdapter->setMap(m_mapWidget->map());
                            }
                            m_userBoundaries = m_boundaryAdapter->loadSavedBoundaries();
                            if (!m_userBoundaries.isEmpty()) {
                                m_boundaryAdapter->setBoundaries(m_userBoundaries);
                            }

                            m_mapWidget->updateMap();
                            static bool initialFocusDone = false;
                            if (!initialFocusDone) {
                                initialFocusDone = true;
                                QTimer::singleShot(500, this, &MainWindow::focusOnAreaOfView);
                            }
                        });
                    }
                    m_styleDebounceTimer->start(150); // Debounce to allow multiple style loading events to settle
                }
            });

            // Set Initial Offline Tactical Dark Style and Camera
            QString offlineStyle = QString("file://%1").arg(GISApp::Core::SystemConfigManager::instance().getOfflineStylePath());
            m_mapController->setStyle(offlineStyle);
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
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseDoubleClicked,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMouseDoubleClick);
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
    if (m_udlToolbar && m_udlToolbar->isVisible()) {
        m_udlToolbar->adjustSize();
        int x = (m_mapWidget->width() - m_udlToolbar->width()) / 2;
        int y = 16;
        m_udlToolbar->move(x, y);
        m_udlToolbar->raise();
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

    // UDL Active Drawing Context Actions
    if (m_toolManager && m_udlDrawingTool && m_toolManager->activeTool() == m_udlDrawingTool.get()) {
        auto geomType = m_udlDrawingTool->geometryType();
        bool canFinish = false;
        QString finishText = tr("✅ Finish Entity");

        if (geomType == GISApp::UI::UDL::UdlGeometryType::Polyline) {
            finishText = tr("✅ Finish Line (Polyline)");
            canFinish = m_udlDrawingTool->waypointsCount() >= 2;
        } else if (geomType == GISApp::UI::UDL::UdlGeometryType::Polygon) {
            finishText = tr("✅ Finish Polygon");
            canFinish = m_udlDrawingTool->waypointsCount() >= 3;
        } else if (geomType == GISApp::UI::UDL::UdlGeometryType::Circle) {
            finishText = tr("✅ Finish Circle");
            canFinish = m_udlDrawingTool->waypointsCount() >= 2;
        }

        if (canFinish) {
            QAction *finishAct = menu.addAction(finishText);
            connect(finishAct, &QAction::triggered, [this]() {
                if (m_udlDrawingTool) m_udlDrawingTool->finishShape();
            });
        }

        if (m_udlDrawingTool->hasWaypoints()) {
            QAction *undoAct = menu.addAction(tr("↩️ Undo Last Waypoint"));
            connect(undoAct, &QAction::triggered, [this]() {
                if (m_udlDrawingTool) m_udlDrawingTool->undoLastPoint();
            });

            QAction *cancelAct = menu.addAction(tr("❌ Cancel Drawing"));
            connect(cancelAct, &QAction::triggered, [this]() {
                if (m_udlDrawingTool) m_udlDrawingTool->clearWaypoints();
            });

            menu.addSeparator();
        }
    }

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

    // UDL Spatial Hit-Testing & Entity Context Menu Options
    auto udlEntities = GISApp::Publishing::UdlRepositoryManager::instance().getAllEntities();
    QList<GISApp::Publishing::UdlEntityItem> hitUdlEntities;
    const double udlTolerancePx = 25.0;

    for (const auto &ent : udlEntities) {
        QJsonObject geom = ent.geometryJson;
        QString type = geom.value("type").toString();
        QJsonArray coords = geom.value("coordinates").toArray();
        bool isHit = false;

        if (type == "Point" && coords.size() >= 2) {
            double lon = coords[0].toDouble();
            double lat = coords[1].toDouble();
            QPointF pt = m_mapWidget->map()->pixelForCoordinate({lat, lon});
            if (std::hypot(pt.x() - localPos.x(), pt.y() - localPos.y()) <= udlTolerancePx) {
                isHit = true;
            }
        } else if (type == "LineString") {
            for (int i = 0; i < coords.size(); ++i) {
                QJsonArray ptArr = coords[i].toArray();
                if (ptArr.size() >= 2) {
                    double lon = ptArr[0].toDouble();
                    double lat = ptArr[1].toDouble();
                    QPointF pt = m_mapWidget->map()->pixelForCoordinate({lat, lon});
                    if (std::hypot(pt.x() - localPos.x(), pt.y() - localPos.y()) <= udlTolerancePx) {
                        isHit = true; break;
                    }
                }
            }
        } else if (type == "Polygon") {
            if (!coords.isEmpty()) {
                QJsonArray ring = coords[0].toArray();
                for (int i = 0; i < ring.size(); ++i) {
                    QJsonArray ptArr = ring[i].toArray();
                    if (ptArr.size() >= 2) {
                        double lon = ptArr[0].toDouble();
                        double lat = ptArr[1].toDouble();
                        QPointF pt = m_mapWidget->map()->pixelForCoordinate({lat, lon});
                        if (std::hypot(pt.x() - localPos.x(), pt.y() - localPos.y()) <= udlTolerancePx) {
                            isHit = true; break;
                        }
                    }
                }
            }
        }

        if (isHit) {
            hitUdlEntities.append(ent);
        }
    }

    if (!hitUdlEntities.isEmpty()) {
        for (const auto &ent : hitUdlEntities) {
            QMenu *udlSubMenu = menu.addMenu(QString("🎨 UDL Entity: %1").arg(ent.entityName));
            udlSubMenu->setStyleSheet(menu.styleSheet());

            QAction *showDetailsAct = udlSubMenu->addAction(QString("ℹ️ Show Details"));
            connect(showDetailsAct, &QAction::triggered, [this, ent]() {
                showUdlEntityDetailsDialog(ent);
            });

            QAction *copyEntAct = udlSubMenu->addAction(QString("📋 Copy Entity"));
            connect(copyEntAct, &QAction::triggered, [ent]() {
                GISApp::Publishing::UdlRepositoryManager::instance().setCopiedEntity(ent);
                GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                    "Clipboard", QString("Copied UDL Entity '%1'").arg(ent.entityName));
            });

            QAction *editStyleAct = udlSubMenu->addAction(QString("✏️ Edit Style"));
            connect(editStyleAct, &QAction::triggered, [this, ent]() {
                GISApp::UI::UDL::UdlGeometryType gType = GISApp::UI::UDL::UdlGeometryType::Point;
                if (ent.entityType == "Polyline" || ent.entityType == "LineString") gType = GISApp::UI::UDL::UdlGeometryType::Polyline;
                else if (ent.entityType == "Polygon") gType = GISApp::UI::UDL::UdlGeometryType::Polygon;
                else if (ent.entityType == "Circle") gType = GISApp::UI::UDL::UdlGeometryType::Circle;
                else if (ent.entityType == "Text") gType = GISApp::UI::UDL::UdlGeometryType::Text;
                else if (ent.entityType == "Image") gType = GISApp::UI::UDL::UdlGeometryType::Image;

                GISApp::UI::UDL::UdlEntityStyleDialog styleDlg(gType, this);
                styleDlg.setEntityName(ent.entityName);
                styleDlg.setStyleJsonObject(ent.styleJson);

                if (styleDlg.exec() == QDialog::Accepted) {
                    GISApp::Publishing::UdlEntityItem updatedItem = ent;
                    updatedItem.entityName = styleDlg.entityName();
                    updatedItem.styleJson = styleDlg.styleJsonObject();
                    GISApp::Publishing::UdlRepositoryManager::instance().saveEntity(updatedItem);
                    GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                        "UDL Entity", QString("Updated entity '%1'").arg(updatedItem.entityName));
                }
            });

            QAction *delEntAct = udlSubMenu->addAction(QString("🗑️ Delete Entity"));
            connect(delEntAct, &QAction::triggered, [ent]() {
                GISApp::Publishing::UdlRepositoryManager::instance().deleteEntity(ent.entityId, ent.layerId);
                GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                    "UDL Entity", QString("Deleted entity '%1'").arg(ent.entityName));
            });

            QAction *clearLayerEntsAct = udlSubMenu->addAction(QString("🗑️ Clear All Entities in Layer"));
            connect(clearLayerEntsAct, &QAction::triggered, [ent, this]() {
                if (QMessageBox::question(this, tr("Clear Layer Entities"), tr("Delete all entities in layer '%1'?").arg(ent.layerId), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    GISApp::Publishing::UdlRepositoryManager::instance().clearAllEntities(ent.layerId);
                    GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                        "UDL Entities", QString("Cleared all entities for layer '%1'").arg(ent.layerId));
                }
            });
        }
        menu.addSeparator();
    }

    // UDL Paste Action
    if (GISApp::Publishing::UdlRepositoryManager::instance().hasCopiedEntity()) {
        auto copiedItem = GISApp::Publishing::UdlRepositoryManager::instance().copiedEntity();
        QString targetLayerId = m_udlToolbar ? m_udlToolbar->activeLayerId() : copiedItem.layerId;
        if (targetLayerId.isEmpty()) targetLayerId = copiedItem.layerId;
        QString targetLayerName = m_udlToolbar ? m_udlToolbar->activeLayerName() : targetLayerId;
        if (targetLayerName.isEmpty()) targetLayerName = targetLayerId;

        QAction *pasteUdlAct = menu.addAction(QString("📋 Paste UDL Entity ('%1') onto Layer '%2'").arg(copiedItem.entityName, targetLayerName));
        connect(pasteUdlAct, &QAction::triggered, [this, copiedItem, targetLayerId, targetLayerName, coordinate]() {
            GISApp::Publishing::UdlEntityItem newItem = copiedItem;
            // Generate unique different entity ID
            newItem.entityId = QString("udl_ent_%1_%2")
                                   .arg(QDateTime::currentMSecsSinceEpoch())
                                   .arg(QRandomGenerator::global()->bounded(1000, 9999));
            newItem.layerId = targetLayerId;
            newItem.entityName = QString("%1 (Copy)").arg(copiedItem.entityName);
            newItem.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);

            // Adjust geometry coordinates to right-click location
            QJsonObject geom = newItem.geometryJson;
            QString type = geom.value("type").toString();
            QJsonArray coords = geom.value("coordinates").toArray();

            if (type == "Point" && coords.size() >= 2) {
                QJsonArray newCoords;
                newCoords.append(coordinate.longitude());
                newCoords.append(coordinate.latitude());
                geom["coordinates"] = newCoords;
            } else if (type == "LineString" && !coords.isEmpty()) {
                QJsonArray refPt = coords[0].toArray();
                double dLon = coordinate.longitude() - refPt[0].toDouble();
                double dLat = coordinate.latitude() - refPt[1].toDouble();

                QJsonArray newCoords;
                for (int i = 0; i < coords.size(); ++i) {
                    QJsonArray pt = coords[i].toArray();
                    QJsonArray newPt;
                    newPt.append(pt[0].toDouble() + dLon);
                    newPt.append(pt[1].toDouble() + dLat);
                    newCoords.append(newPt);
                }
                geom["coordinates"] = newCoords;
            } else if (type == "Polygon" && !coords.isEmpty()) {
                QJsonArray ring = coords[0].toArray();
                if (!ring.isEmpty()) {
                    QJsonArray refPt = ring[0].toArray();
                    double dLon = coordinate.longitude() - refPt[0].toDouble();
                    double dLat = coordinate.latitude() - refPt[1].toDouble();

                    QJsonArray newRing;
                    for (int i = 0; i < ring.size(); ++i) {
                        QJsonArray pt = ring[i].toArray();
                        QJsonArray newPt;
                        newPt.append(pt[0].toDouble() + dLon);
                        newPt.append(pt[1].toDouble() + dLat);
                        newRing.append(newPt);
                    }
                    QJsonArray newPolygonCoords;
                    newPolygonCoords.append(newRing);
                    geom["coordinates"] = newPolygonCoords;
                }
                if (newItem.entityType == "Circle") {
                    QJsonObject style = newItem.styleJson;
                    style["centerLon"] = coordinate.longitude();
                    style["centerLat"] = coordinate.latitude();
                    newItem.styleJson = style;
                }
            }
            newItem.geometryJson = geom;

            GISApp::Publishing::UdlRepositoryManager::instance().saveEntity(newItem);
            GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                "UDL Entity", QString("Pasted '%1' with ID '%2' on layer '%3'").arg(newItem.entityName, newItem.entityId, targetLayerName));
        });

        QAction *clearClipAct = menu.addAction(QString("🧹 Clear UDL Clipboard ('%1')").arg(copiedItem.entityName));
        connect(clearClipAct, &QAction::triggered, []() {
            GISApp::Publishing::UdlRepositoryManager::instance().clearCopiedEntity();
            GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                "UDL Clipboard", "UDL Clipboard cleared.");
        });

        menu.addSeparator();
    }

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

void MainWindow::showUdlEntityDetailsDialog(const GISApp::Publishing::UdlEntityItem &entity)
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("UDL Entity Details — %1").arg(entity.entityName));
    dlg->setMinimumWidth(480);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet(
        "QDialog { background-color: #1e1e2e; color: #cdd6f4; }"
        "QLabel { color: #cdd6f4; font-size: 12px; font-weight: bold; }"
        "QGroupBox { font-weight: bold; color: #89b4fa; border: 1px solid #45475a; border-radius: 6px; margin-top: 10px; padding-top: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QLineEdit { background-color: #181825; color: #f3f4f6; border: 1px solid #45475a; border-radius: 4px; padding: 4px 8px; font-weight: normal; }"
        "QPushButton { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; padding: 6px 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45475a; color: #ffffff; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);
    mainLayout->setSpacing(12);

    // General Attributes Group
    QGroupBox *genGroup = new QGroupBox(tr("General Information"), dlg);
    QFormLayout *genLayout = new QFormLayout(genGroup);

    QLineEdit *txtId = new QLineEdit(entity.entityId, dlg);
    txtId->setReadOnly(true);
    genLayout->addRow(tr("Entity ID:"), txtId);

    QLineEdit *txtName = new QLineEdit(entity.entityName, dlg);
    txtName->setReadOnly(true);
    genLayout->addRow(tr("Entity Name:"), txtName);

    QLineEdit *txtType = new QLineEdit(entity.entityType, dlg);
    txtType->setReadOnly(true);
    genLayout->addRow(tr("Geometry Type:"), txtType);

    QLineEdit *txtLayer = new QLineEdit(entity.layerId, dlg);
    txtLayer->setReadOnly(true);
    genLayout->addRow(tr("Layer ID:"), txtLayer);

    QLineEdit *txtTime = new QLineEdit(entity.createdAt, dlg);
    txtTime->setReadOnly(true);
    genLayout->addRow(tr("Created At:"), txtTime);

    mainLayout->addWidget(genGroup);

    // Style Attributes Group
    QGroupBox *styleGroup = new QGroupBox(tr("Visual Style Attributes"), dlg);
    QFormLayout *styleLayout = new QFormLayout(styleGroup);

    QJsonObject sJson = entity.styleJson;
    for (auto it = sJson.begin(); it != sJson.end(); ++it) {
        QLineEdit *valEdit = new QLineEdit(it.value().toVariant().toString(), dlg);
        valEdit->setReadOnly(true);
        styleLayout->addRow(QString("%1:").arg(it.key()), valEdit);
    }
    mainLayout->addWidget(styleGroup);

    // Action Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *btnCopy = new QPushButton(tr("📋 Copy Entity"), dlg);
    btnCopy->setStyleSheet("background-color: #2563eb; color: #ffffff;");
    connect(btnCopy, &QPushButton::clicked, [entity, dlg]() {
        GISApp::Publishing::UdlRepositoryManager::instance().setCopiedEntity(entity);
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "Clipboard", QString("Copied UDL Entity '%1'").arg(entity.entityName));
    });

    QPushButton *btnEdit = new QPushButton(tr("🎨 Edit Style"), dlg);
    btnEdit->setStyleSheet("background-color: #059669; color: #ffffff;");
    connect(btnEdit, &QPushButton::clicked, [this, entity, dlg]() {
        GISApp::UI::UDL::UdlGeometryType gType = GISApp::UI::UDL::UdlGeometryType::Point;
        if (entity.entityType == "Polyline" || entity.entityType == "LineString") gType = GISApp::UI::UDL::UdlGeometryType::Polyline;
        else if (entity.entityType == "Polygon") gType = GISApp::UI::UDL::UdlGeometryType::Polygon;
        else if (entity.entityType == "Circle") gType = GISApp::UI::UDL::UdlGeometryType::Circle;
        else if (entity.entityType == "Text") gType = GISApp::UI::UDL::UdlGeometryType::Text;
        else if (entity.entityType == "Image") gType = GISApp::UI::UDL::UdlGeometryType::Image;

        GISApp::UI::UDL::UdlEntityStyleDialog styleDlg(gType, this);
        styleDlg.setEntityName(entity.entityName);
        styleDlg.setStyleJsonObject(entity.styleJson);

        if (styleDlg.exec() == QDialog::Accepted) {
            GISApp::Publishing::UdlEntityItem updatedItem = entity;
            updatedItem.entityName = styleDlg.entityName();
            updatedItem.styleJson = styleDlg.styleJsonObject();
            GISApp::Publishing::UdlRepositoryManager::instance().saveEntity(updatedItem);
            GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                "UDL Entity", QString("Updated entity '%1'").arg(updatedItem.entityName));
            dlg->accept();
        }
    });

    QPushButton *btnClear = new QPushButton(tr("🧹 Clear Clipboard"), dlg);
    btnClear->setStyleSheet("background-color: #45475a; color: #ffffff;");
    connect(btnClear, &QPushButton::clicked, []() {
        GISApp::Publishing::UdlRepositoryManager::instance().clearCopiedEntity();
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "UDL Clipboard", "UDL Clipboard cleared.");
    });

    QPushButton *btnClose = new QPushButton(tr("Close"), dlg);
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::accept);

    btnLayout->addWidget(btnCopy);
    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnClear);
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);

    dlg->exec();
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

void MainWindow::setupUdlMenu()
{
    QMenu *udlMenu = menuBar()->addMenu(tr("&UDL"));

    QAction *createLayerAction = udlMenu->addAction(tr("➕ Create UDL Layer..."));
    connect(createLayerAction, &QAction::triggered, this, &MainWindow::onCreateUdlLayerTriggered);

    QAction *toggleToolbarAction = udlMenu->addAction(tr("🛠️ Toggle Digitizing Toolbar"));
    connect(toggleToolbarAction, &QAction::triggered, this, &MainWindow::onToggleUdlToolbarTriggered);

    QAction *manageEntitiesAction = udlMenu->addAction(tr("📊 Manage UDL Entities..."));
    connect(manageEntitiesAction, &QAction::triggered, this, &MainWindow::onManageUdlEntitiesTriggered);

    // Initialize UDL Toolbar Widget
    m_udlToolbar = new GISApp::UI::UDL::UdlToolbarWidget(m_mapWidget);
    m_udlToolbar->hide();

    // Initialize UDL Drawing Tool
    m_udlDrawingTool = std::make_shared<GISApp::Tools::UdlDrawingTool>(this);
    if (m_toolManager) {
        m_toolManager->registerTool(m_udlDrawingTool);
    }
    if (m_udlDrawingTool && m_mapWidget) {
        connect(m_udlDrawingTool.get(), &GISApp::Tools::UdlDrawingTool::previewUpdated,
                m_mapWidget, &GISApp::Map::MapLibreWidget::setUdlPreview);
    }

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::createLayerRequested, this, &MainWindow::onCreateUdlLayerTriggered);
    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::manageEntitiesRequested, this, &MainWindow::onManageUdlEntitiesTriggered);

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::pendingTextLabelChanged, this, [this](const QString &text) {
        if (m_udlDrawingTool) {
            m_udlDrawingTool->setPendingTextLabel(text);
        }
    });

    auto handleUndo = [this]() {
        if (m_udlDrawingTool && m_udlDrawingTool->hasWaypoints()) {
            m_udlDrawingTool->undoLastPoint();
        } else {
            GISApp::Publishing::UdlRepositoryManager::instance().undoLastAction();
        }
    };

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::undoRequested, this, handleUndo);

    auto *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, handleUndo);

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::colorsChanged, this, [this](const QColor &stroke, const QColor &fill) {
        if (m_udlDrawingTool) {
            m_udlDrawingTool->setPresetColors(stroke, fill);
        }
    });

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::continuousModeChanged, this, [this](bool enabled) {
        if (m_udlDrawingTool) {
            m_udlDrawingTool->setContinuousMode(enabled);
        }
    });

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::quickNameModeChanged, this, [this](bool enabled) {
        if (m_udlDrawingTool) {
            m_udlDrawingTool->setQuickNameMode(enabled);
        }
    });

    connect(m_udlToolbar, &GISApp::UI::UDL::UdlToolbarWidget::toolSelected, this, [this](GISApp::UI::UDL::UdlGeometryType type) {
        if (!m_udlDrawingTool || !m_toolManager) return;
        m_udlDrawingTool->setTargetLayerId(m_udlToolbar->activeLayerId());
        m_udlDrawingTool->setGeometryType(type);
        m_udlDrawingTool->setPendingTextLabel(m_udlToolbar->pendingTextLabel());
        m_udlDrawingTool->setContinuousMode(m_udlToolbar->isContinuousMode());
        m_udlDrawingTool->setQuickNameMode(m_udlToolbar->isQuickNameMode());
        m_udlDrawingTool->setPresetColors(m_udlToolbar->currentStrokeColor(), m_udlToolbar->currentFillColor());
        m_toolManager->setActiveTool(m_udlDrawingTool->toolName());
    });
}

void MainWindow::onCreateUdlLayerTriggered()
{
    GISApp::UI::UDL::CreateUdlLayerDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.layerName();
        if (name.isEmpty()) return;

        QString layerId = QString("udl_%1").arg(QDateTime::currentMSecsSinceEpoch());
        QString jsonPath = GISApp::Publishing::UdlRepositoryManager::instance().getUdlGeoJsonPath(layerId);
        GISApp::Publishing::LayerRegistryManager::instance().registerUserDefinedLayer(layerId, name, 1.0f, "🎨 User Defined Layers", jsonPath);
        
        GISApp::Publishing::UdlRepositoryManager::instance().syncGeoJsonFile(layerId);
        if (m_udlToolbar) {
            m_udlToolbar->refreshLayerList();
            m_udlToolbar->setActiveLayer(layerId);
            m_udlToolbar->show();
            updateOverlayPositions();
        }
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "UDL Layer Created",
            QString("User Defined Layer '%1' created successfully.").arg(name),
            4000,
            this
        );
    }
}

void MainWindow::onToggleUdlToolbarTriggered()
{
    if (!m_udlToolbar) return;
    if (m_udlToolbar->isVisible()) {
        m_udlToolbar->hide();
    } else {
        m_udlToolbar->refreshLayerList();
        m_udlToolbar->show();
        updateOverlayPositions();
    }
}

void MainWindow::onManageUdlEntitiesTriggered()
{
    if (!m_udlEntityTableDialog) {
        m_udlEntityTableDialog = new GISApp::UI::UDL::UdlEntityTableDialog(this);
        connect(m_udlEntityTableDialog, &GISApp::UI::UDL::UdlEntityTableDialog::zoomToEntityRequested, this, [this](double lat, double lon) {
            if (m_mapController) {
                m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(lat, lon), 14.0);
            }
        });
    }
    m_udlEntityTableDialog->refreshData();
    m_udlEntityTableDialog->show();
    m_udlEntityTableDialog->raise();
    m_udlEntityTableDialog->activateWindow();
}

void MainWindow::onUdlLayerUpdated(const QString &layerId, const QString &geojsonPath)
{
    if (!m_mapWidget || !m_mapWidget->map()) return;

    auto map = m_mapWidget->map();
    QString sanitizedId = QString("udl-%1").arg(qHash(layerId));
    QString srcId = sanitizedId + "-src";

    if (map->layerExists(sanitizedId + "-symbol")) map->removeLayer(sanitizedId + "-symbol");
    if (map->layerExists(sanitizedId + "-image")) map->removeLayer(sanitizedId + "-image");
    if (map->layerExists(sanitizedId + "-circle")) map->removeLayer(sanitizedId + "-circle");
    if (map->layerExists(sanitizedId + "-line")) map->removeLayer(sanitizedId + "-line");
    if (map->layerExists(sanitizedId + "-fill")) map->removeLayer(sanitizedId + "-fill");
    if (map->sourceExists(srcId)) map->removeSource(srcId);

    if (geojsonPath.isEmpty() || !QFile::exists(geojsonPath)) {
        if (m_udlToolbar) {
            m_udlToolbar->refreshLayerList();
        }
        if (m_udlEntityTableDialog) {
            m_udlEntityTableDialog->refreshData();
        }
        return;
    }

    QFile geoJsonFile(geojsonPath);
    QByteArray geoJsonData;
    if (geoJsonFile.open(QIODevice::ReadOnly)) {
        geoJsonData = geoJsonFile.readAll();
        geoJsonFile.close();
    }

    QVariantMap sourceParams;
    sourceParams["type"] = "geojson";
    if (!geoJsonData.isEmpty()) {
        sourceParams["data"] = geoJsonData;
    } else {
        sourceParams["data"] = QString("file://%1").arg(geojsonPath);
    }
    map->addSource(srcId, sourceParams);

    // Add Polygon Fill layer
    QVariantMap fillParams;
    fillParams["id"] = sanitizedId + "-fill";
    fillParams["type"] = "fill";
    fillParams["source"] = srcId;
    QVariantMap fillPaint;
    fillPaint["fill-color"] = QVariantList{"to-color", QVariantList{"coalesce", QVariantList{"get", "fillColor"}, QVariantList{"get", "strokeColor"}, "#ff9933"}};
    fillPaint["fill-opacity"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, 0.35}};
    fillParams["paint"] = fillPaint;

    QVariantList fillFilter;
    fillFilter.append("==");
    fillFilter.append("$type");
    fillFilter.append("Polygon");
    fillParams["filter"] = fillFilter;
    map->addLayer(sanitizedId + "-fill", fillParams);

    // Add Line / Outline layer
    QVariantMap lineParams;
    lineParams["id"] = sanitizedId + "-line";
    lineParams["type"] = "line";
    lineParams["source"] = srcId;
    QVariantMap linePaint;
    linePaint["line-color"] = QVariantList{"to-color", QVariantList{"coalesce", QVariantList{"get", "strokeColor"}, "#f59e0b"}};
    linePaint["line-width"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "lineWidth"}, 3.0}};
    linePaint["line-opacity"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "strokeOpacity"}, QVariantList{"get", "lineOpacity"}, 1.0}};
    lineParams["paint"] = linePaint;
    map->addLayer(sanitizedId + "-line", lineParams);

    // Add Point Circle layer for non-text / non-image Point entities
    QVariantMap circleParams;
    circleParams["id"] = sanitizedId + "-circle";
    circleParams["type"] = "circle";
    circleParams["source"] = srcId;
    QVariantMap circlePaint;
    QString defaultCircleColor = (sanitizedId.contains("sensor") || geojsonPath.contains("sensor")) ? "#ffff00" : "#f59e0b";
    circlePaint["circle-color"] = QVariantList{"to-color", QVariantList{"coalesce", QVariantList{"get", "fillColor"}, QVariantList{"get", "strokeColor"}, defaultCircleColor}};
    circlePaint["circle-opacity"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, QVariantList{"get", "strokeOpacity"}, 1.0}};
    circlePaint["circle-radius"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "pointRadius"}, QVariantList{"get", "lineWidth"}, 6.0}};
    circlePaint["circle-stroke-color"] = QVariantList{"to-color", QVariantList{"coalesce", QVariantList{"get", "fillColor"}, QVariantList{"get", "strokeColor"}, defaultCircleColor}};
    circlePaint["circle-stroke-opacity"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fillOpacity"}, QVariantList{"get", "strokeOpacity"}, 1.0}};
    circlePaint["circle-stroke-width"] = 0.0;
    circleParams["paint"] = circlePaint;

    QVariantList circleFilter{"all", QVariantList{"==", "$type", "Point"}, QVariantList{"!=", "entityType", "Text"}, QVariantList{"!=", "entityType", "Image"}};
    circleParams["filter"] = circleFilter;
    map->addLayer(sanitizedId + "-circle", circleParams);

    // Add Symbol Text layer for Text entities
    QVariantMap symbolParams;
    symbolParams["id"] = sanitizedId + "-symbol";
    symbolParams["type"] = "symbol";
    symbolParams["source"] = srcId;

    QVariantMap symbolLayout;
    symbolLayout["text-field"] = QString("{textContent}");
    symbolLayout["text-size"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "fontSize"}, 15.0}};
    symbolLayout["text-font"] = QVariantList{"Open Sans Regular", "Arial Unicode MS"};
    symbolLayout["text-anchor"] = "center";
    symbolLayout["text-allow-overlap"] = true;
    symbolLayout["text-ignore-placement"] = true;
    symbolParams["layout"] = symbolLayout;

    QVariantMap symbolPaint;
    symbolPaint["text-color"] = QVariantList{"to-color", QVariantList{"coalesce", QVariantList{"get", "textColor"}, QVariantList{"get", "strokeColor"}, "#ffffff"}};
    symbolPaint["text-opacity"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "textOpacity"}, 1.0}};
    symbolPaint["text-halo-color"] = QVariantList{"to-color", QVariantList{"coalesce", QVariantList{"get", "bgColor"}, "#000000"}};
    symbolPaint["text-halo-width"] = 2.5;
    symbolParams["paint"] = symbolPaint;

    QVariantList symbolFilter{"==", "entityType", "Text"};
    symbolParams["filter"] = symbolFilter;
    map->addLayer(sanitizedId + "-symbol", symbolParams);

    // Register images and Add Image Symbol layer for Image entities
    auto layerEntities = GISApp::Publishing::UdlRepositoryManager::instance().getEntitiesForLayer(layerId);
    for (const auto &item : layerEntities) {
        if (item.entityType == "Image") {
            QString path = item.styleJson.value("imagePath").toString();
            if (!path.isEmpty() && QFile::exists(path)) {
                QString iconId = QString("udl-icon-%1").arg(qHash(path));
                QImage img(path);
                if (!img.isNull()) {
                    int targetW = item.styleJson.value("imageWidth").toInt(64);
                    int targetH = item.styleJson.value("imageHeight").toInt(64);
                    if (targetW > 0 && targetH > 0 && (img.width() != targetW || img.height() != targetH)) {
                        img = img.scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    }
                    map->addImage(iconId, img);
                }
            }
        }
    }

    QVariantMap imgParams;
    imgParams["id"] = sanitizedId + "-image";
    imgParams["type"] = "symbol";
    imgParams["source"] = srcId;

    QVariantMap imgLayout;
    imgLayout["icon-image"] = QString("{iconId}");
    imgLayout["icon-allow-overlap"] = true;
    imgLayout["icon-ignore-placement"] = true;
    imgParams["layout"] = imgLayout;

    QVariantMap imgPaint;
    imgPaint["icon-opacity"] = QVariantList{"to-number", QVariantList{"coalesce", QVariantList{"get", "imageOpacity"}, 1.0}};
    imgParams["paint"] = imgPaint;

    QVariantList imgFilter{"==", "entityType", "Image"};
    imgParams["filter"] = imgFilter;
    map->addLayer(sanitizedId + "-image", imgParams);

    // Ensure LayerManager has a LayerNode with a valid MapLibreLayerAdapter for this UDL layer!
    if (m_layerManager) {
        QString displayName;
        QString groupName;
        for (const auto &meta : GISApp::Publishing::LayerRegistryManager::instance().getSavedLayers()) {
            if ((meta.layerId == layerId || meta.folderPath == geojsonPath) && !meta.name.isEmpty()) {
                displayName = meta.name;
                groupName = meta.groupName;
                break;
            }
        }

        if (displayName.isEmpty() || displayName == layerId) {
            if (geojsonPath.contains("exp_layer")) {
                displayName = m_expLayer ? m_expLayer->layerName() : "Experiment entity";
                groupName = m_expLayer ? m_expLayer->layerGroup() : "EXP group1";
            } else if (geojsonPath.contains("sensor_telemetry")) {
                displayName = "sensor";
                groupName = "SensorGroup";
            } else if (geojsonPath.contains("sample_telemetry")) {
                displayName = "⚡ Sample Entities";
                groupName = "🛡️ Tactical Operations";
            } else {
                displayName = layerId;
                groupName = "🎨 User Defined Layers";
            }

            GISApp::Publishing::LayerRegistryManager::instance().registerPublishedLayer(
                GISApp::Publishing::LayerType::Vector,
                geojsonPath,
                displayName,
                groupName
            );
        }

        if (groupName.isEmpty()) groupName = "🎨 User Defined Layers";

        auto udlExtent = GISApp::Publishing::UdlRepositoryManager::instance().calculateLayerExtent(layerId);
        auto adapter = std::make_shared<GISApp::Layers::MapLibreLayerAdapter>(
            sanitizedId,
            map,
            udlExtent,
            QVariantMap{}, QVariantMap{}, QVariantMap{},
            layerId
        );

        GISApp::Layers::LayerNode *node = m_layerManager->findLayerByLayerId(sanitizedId);
        if (!node && m_layerManager->model()) {
            auto root = m_layerManager->model()->rootNode();
            std::function<void(GISApp::Layers::LayerTreeNode*)> findNode = [&](GISApp::Layers::LayerTreeNode *n) {
                if (!n || node) return;
                if (n->nodeType() == GISApp::Layers::NodeType::Layer) {
                    auto lNode = static_cast<GISApp::Layers::LayerNode*>(n);
                    auto mlAdapter = std::dynamic_pointer_cast<GISApp::Layers::MapLibreLayerAdapter>(lNode->adapter());
                    if (lNode->name() == displayName || lNode->name() == layerId || (mlAdapter && mlAdapter->rawUdlLayerId() == layerId)) {
                        node = lNode;
                        return;
                    }
                }
                if (n->nodeType() == GISApp::Layers::NodeType::Group) {
                    auto g = static_cast<GISApp::Layers::LayerGroupNode*>(n);
                    for (int i = 0; i < g->childCount(); ++i) findNode(g->child(i));
                }
            };
            for (int i = 0; i < root->childCount(); ++i) findNode(root->child(i));
        }

        if (!node) {
            auto group = m_layerManager->findGroupByName(groupName);
            if (!group) group = m_layerManager->addGroup(groupName);
            node = m_layerManager->addLayer(displayName, adapter, group);
        } else {
            node->setAdapter(adapter);
        }

        if (node) {
            adapter->setVisibility(node->checkState() == Qt::Checked);
            adapter->setOpacity(node->opacity());
        }
    }

    map->triggerRepaint();
}

void MainWindow::restoreCustomEntityLayersAndGroups()
{
    if (!m_layerManager || !m_mapWidget || !m_mapWidget->map()) return;

    // 1. Register custom groups in LayerRegistryManager
    QString expGroup = m_expLayer ? m_expLayer->layerGroup() : "EXP group";
    GISApp::Publishing::LayerRegistryManager::instance().registerGroup("SensorGroup");
    GISApp::Publishing::LayerRegistryManager::instance().registerGroup("🛡️ Tactical Operations");
    GISApp::Publishing::LayerRegistryManager::instance().registerGroup(expGroup);

    // 2. Restore saved layers from published_layers.json
    auto publishingService = new GISApp::Publishing::LayerPublishingService(m_layerManager);
    GISApp::Publishing::LayerRegistryManager::instance().restoreSavedLayers(
        m_layerManager, m_mapWidget->map(), publishingService
    );

    //-EXPERIEMENT BLOCK
    if (m_expLayer) {
        m_expLayer->updateLayer();
    }
    //--EXPERIEMENT BLOCK

    // 3. Scan and auto-load existing telemetry GeoJSON files on app restart
    QString mapDataDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QString udlDir = mapDataDir + "/udl_layers";

    struct TelemetryLayerConfig {
        QString fileName;
        QString layerName;
        QString groupName;
    };

    QList<TelemetryLayerConfig> telemetryLayers = {
        {"sensor_telemetry.geojson", "sensor", "SensorGroup"},
        {"sample_telemetry.geojson", "⚡ Sample Entities", "🛡️ Tactical Operations"},
        {"exp_layer_experiment_telemetry.geojson", "Experiment Telemetry", expGroup}
    };

    for (const auto &cfg : telemetryLayers) {
        QString geojsonPath = udlDir + "/" + cfg.fileName;
        if (QFile::exists(geojsonPath)) {
            QString layerId = "vector-" + QString::number(qHash(cfg.layerName));

            GISApp::Publishing::LayerRegistryManager::instance().registerPublishedLayer(
                GISApp::Publishing::LayerType::Vector,
                geojsonPath,
                cfg.layerName,
                cfg.groupName
            );

            this->onUdlLayerUpdated(layerId, geojsonPath);
        }
    }
}

