/**
 * @file mainwindow.h
 * @brief Main Window housing Map View, Left Sidebar, Floating Toolbars, Tactical Status Bar, and Theme Manager.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <memory>

#include "map/MapLibreWidget.h"
#include "controllers/MapController.h"
#include "controllers/ToolManager.h"
#include "tools/MeasureTool.h"
#include "core/models/GeoCoordinate.h"

// Tactical UI Widgets & Theme Engine
#include "ui/ThemeManager.h"
#include "ui/HeaderBar.h"
#include "ui/LeftSidebar.h"
#include "ui/RightToolPanel.h"
#include "ui/ZoomControlsWidget.h"
#include "ui/TacticalStatusBar.h"

#include "layers/LayerManager.h"
#include "layers/TacticalLayerProvider.h"
#include "ui/layertree/LayerTreeView.h"
#include "ui/layertree/LayerTreeFloatingWidget.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace GISApp::UI::Publishing {
    class PublishLayerDialog;
    class GroupManagerDialog;
}
namespace GISApp::UI::Download {
    class DownloadSatImageryDialog;
}
namespace GISApp::UI::Tasks {
    class BackgroundTaskDialog;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onMouseCoordinateChanged(const GISApp::Core::Models::GeoCoordinate &coordinate);
    void onDistanceUpdated(double totalDistanceKm);
    void onDownloadGoogleSatTriggered();

private:
    Ui::MainWindow *ui;
    
    // Core Engine Widgets & Controllers
    GISApp::Map::MapLibreWidget *m_mapWidget;
    GISApp::Controllers::MapController *m_mapController;
    GISApp::Controllers::ToolManager *m_toolManager;

    std::shared_ptr<GISApp::Tools::MeasureTool> m_measureTool;

    // Tactical UI Components
    GISApp::UI::HeaderBar *m_headerBar;
    GISApp::UI::LeftSidebar *m_leftSidebar;
    GISApp::UI::RightToolPanel *m_rightToolPanel;
    GISApp::UI::ZoomControlsWidget *m_zoomControls;
    GISApp::UI::TacticalStatusBar *m_tacticalStatusBar;

    QActionGroup *m_themeActionGroup;

    void setupMapView();
    void setupToolBar();
    void setupStatusBar();
    void setupThemeMenu();
    void updateOverlayPositions();

    GISApp::Layers::LayerManager *m_layerManager{nullptr};
    GISApp::UI::LayerTreeFloatingWidget *m_layerFloatingPanel{nullptr};

    // Modeless Dialog Instances
    GISApp::UI::Publishing::PublishLayerDialog *m_publishDialog{nullptr};
    GISApp::UI::Publishing::GroupManagerDialog *m_groupManagerDialog{nullptr};
    GISApp::UI::Download::DownloadSatImageryDialog *m_downloadSatDialog{nullptr};
    GISApp::UI::Tasks::BackgroundTaskDialog *m_backgroundTaskDialog{nullptr};
};

#endif // MAINWINDOW_H
