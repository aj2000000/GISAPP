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
#include <QMapLibre/Settings>
#include "tools/PanTool.h"
#include "layers/MapLibreLayerAdapter.h"
#include "ui/layertree/LayerTreeFloatingWidget.h"
#include "layers/TacticalLayerProvider.h"
#include <cmath>
#include <QDir>

#include "ui/publishing/PublishLayerDialog.h"
#include "ui/publishing/GroupManagerDialog.h"
#include "ui/tasks/BackgroundTaskDialog.h"
#include "ui/download/DownloadSatImageryDialog.h"
#include "core/SystemConfigManager.h"

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
            
            // Helper lambda to pan and dynamically zoom to bounding box extent
            auto handlePanToExtent = [this](const GISApp::Layers::LayerExtent &extent) {
                if (m_mapController && extent.isValid()) {
                    double centerLat = (extent.southWest.latitude() + extent.northEast.latitude()) / 2.0;
                    double centerLon = (extent.southWest.longitude() + extent.northEast.longitude()) / 2.0;

                    double latDiff = std::max(0.0001, extent.northEast.latitude() - extent.southWest.latitude());
                    double lonDiff = std::max(0.0001, extent.northEast.longitude() - extent.southWest.longitude());

                    double zoomLon = std::log2(360.0 / lonDiff);
                    double zoomLat = std::log2(180.0 / latDiff);
                    double fitZoom = std::clamp(std::min(zoomLon, zoomLat) + 0.2, 1.5, 16.0);

                    m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(centerLat, centerLon), fitZoom);
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
            
            // Connect style load to tactical layer population
            connect(m_mapWidget->map(), &QMapLibre::Map::mapChanged, [this](QMapLibre::Map::MapChange change) {
                if (change == QMapLibre::Map::MapChangeDidFinishLoadingStyle) {
                    GISApp::Layers::TacticalLayerProvider p;
                    p.setupTacticalLayers(m_mapWidget->map());
                    p.populateLayerTree(m_layerManager, m_mapWidget->map(), m_mapController);
                    m_mapWidget->updateMap();
                }
            });

            // Set Initial Offline Tactical Dark Style and Camera
            QString offlineStyle = QString("file://%1").arg(GISApp::Core::SystemConfigManager::instance().getOfflineStylePath());
            m_mapController->setStyle(offlineStyle);
            m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(28.6139, 77.2090), 10.0);
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

    connect(m_zoomControls, &GISApp::UI::ZoomControlsWidget::zoomInRequested, [this]() {
        if (m_mapController) m_mapController->zoomIn();
    });
    connect(m_zoomControls, &GISApp::UI::ZoomControlsWidget::zoomOutRequested, [this]() {
        if (m_mapController) m_mapController->zoomOut();
    });
    connect(m_zoomControls, &GISApp::UI::ZoomControlsWidget::resetCenterRequested, [this]() {
        if (m_mapController) {
            m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(28.6139, 77.2090), 10.0);
        }
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
