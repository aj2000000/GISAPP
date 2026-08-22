/**
 * @file mainwindow.cpp
 * @brief Implementation of MainWindow UI, ToolManager registration, and measurement tool.
 */

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QStatusBar>
#include <QAction>
#include <QMapLibre/Settings>
#include "tools/PanTool.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mapWidget(nullptr)
    , m_mapController(nullptr)
    , m_toolManager(nullptr)
    , m_coordLabel(nullptr)
    , m_measureLabel(nullptr)
    , m_toolBar(nullptr)
    , m_styleCombo(nullptr)
{
    ui->setupUi(this);
    setupMapView();
    setupToolBar();
    setupStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMapView()
{
    if (!ui->centralwidget->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui->centralwidget);
        layout->setContentsMargins(0, 0, 0, 0);
        ui->centralwidget->setLayout(layout);
    }

    QMapLibre::Settings settings(QMapLibre::Settings::MapLibreProvider);

    m_mapWidget = new GISApp::Map::MapLibreWidget(settings, this);
    ui->centralwidget->layout()->addWidget(m_mapWidget);

    m_mapController = new GISApp::Controllers::MapController(m_mapWidget, this);
    m_mapController->setStyle("https://demotiles.maplibre.org/style.json");
    m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(28.6139, 77.2090), 10.0);

    // 1. Instantiate ToolManager
    m_toolManager = new GISApp::Controllers::ToolManager(this);

    // 2. Register Pan and Distance Measurement Strategy Tools
    auto panTool = std::make_shared<GISApp::Tools::PanTool>(this);
    m_measureTool = std::make_shared<GISApp::Tools::MeasureTool>(this);

    m_toolManager->registerTool(panTool);
    m_toolManager->registerTool(m_measureTool);
    m_toolManager->setActiveTool("PanTool");

    // 3. Connect Map Canvas Mouse Signals to ToolManager & Status Bar
    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseCoordinateChanged,
            this, &MainWindow::onMouseCoordinateChanged);

    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mousePressed,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMousePress);

    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseMoved,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMouseMove);

    connect(m_mapWidget, &GISApp::Map::MapLibreWidget::mouseReleased,
            m_toolManager, &GISApp::Controllers::ToolManager::handleMouseRelease);

    connect(m_measureTool.get(), &GISApp::Tools::MeasureTool::distanceUpdated,
            this, &MainWindow::onDistanceUpdated);
    connect(m_measureTool.get(), &GISApp::Tools::MeasureTool::waypointsUpdated,
            m_mapWidget, &GISApp::Map::MapLibreWidget::setWaypoints);


}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar(tr("Navigation Toolbar"));
    m_toolBar->setMovable(false);

    // 1. Zoom In Action
    QAction *zoomInAct = m_toolBar->addAction(tr("Zoom In (+)"));
    connect(zoomInAct, &QAction::triggered, [this]() {
        if (m_mapController) m_mapController->zoomIn();
    });

    // 2. Zoom Out Action
    QAction *zoomOutAct = m_toolBar->addAction(tr("Zoom Out (-)"));
    connect(zoomOutAct, &QAction::triggered, [this]() {
        if (m_mapController) m_mapController->zoomOut();
    });

    // 3. Reset View Action
    QAction *resetAct = m_toolBar->addAction(tr("Reset Center"));
    connect(resetAct, &QAction::triggered, [this]() {
        if (m_mapController) {
            m_mapController->centerOn(GISApp::Core::Models::GeoCoordinate(28.6139, 77.2090), 10.0);
        }
    });

    m_toolBar->addSeparator();

    // 4. Measure Distance Toggle Action
    QAction *measureAct = m_toolBar->addAction(tr("Measure Distance"));
    measureAct->setCheckable(true);
    connect(measureAct, &QAction::toggled, [this](bool checked) {
        if (m_toolManager) {
            m_toolManager->setActiveTool(checked ? "MeasureTool" : "PanTool");
        }
    });

    m_toolBar->addSeparator();

    // 5. Map Style Selector
    m_styleCombo = new QComboBox(this);
    m_styleCombo->addItem(tr("Demotiles (Vector)"), "https://demotiles.maplibre.org/style.json");
    m_styleCombo->addItem(tr("OSM Bright"), "https://basemaps.cartocdn.com/gl/positron-gl-style/style.json");
    m_styleCombo->addItem(tr("Dark Matter"), "https://basemaps.cartocdn.com/gl/dark-matter-gl-style/style.json");

    m_toolBar->addWidget(m_styleCombo);

    connect(m_styleCombo, &QComboBox::currentIndexChanged, [this](int index) {
        QString styleUrl = m_styleCombo->itemData(index).toString();
        if (m_mapController) {
            m_mapController->setStyle(styleUrl);
        }
    });

}

void MainWindow::setupStatusBar()
{
    m_coordLabel = new QLabel(tr("Lat: --.----- | Lon: --.-----"), this);
    m_coordLabel->setMinimumWidth(250);

    m_measureLabel = new QLabel(tr("Dist: 0.00 km"), this);
    m_measureLabel->setMinimumWidth(150);

    statusBar()->addPermanentWidget(m_measureLabel);
    statusBar()->addPermanentWidget(m_coordLabel);
}

void MainWindow::onMouseCoordinateChanged(const GISApp::Core::Models::GeoCoordinate &coordinate)
{
    if (m_coordLabel && coordinate.isValid()) {
        m_coordLabel->setText(QString("Lat: %1° | Lon: %2°")
                                  .arg(coordinate.latitude(), 0, 'f', 5)
                                  .arg(coordinate.longitude(), 0, 'f', 5));
    }
}

void MainWindow::onDistanceUpdated(double totalDistanceKm)
{
    if (m_measureLabel) {
        m_measureLabel->setText(QString("Dist: %1 km")
                                    .arg(totalDistanceKm, 0, 'f', 2));
    }
}
