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
#include "core/models/BoundaryRecord.h"

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
#include "core/models/TrackRecord.h"


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
namespace GISApp::Core::Repositories {
    class TrackRepository;
    class AreaOfViewRepository;
    class GenericEntityRepository;
}
namespace GISApp::Core::Services {
    class MapLibreTrackAdapter;
    class MapLibreAreaOfViewAdapter;
    class MapLibreGenericEntityAdapter;
    class MapLibreBoundaryAdapter;
}
namespace GISApp::UI::Tracks {
    class TracksTableDialog;
}
namespace GISApp::UI::Boundary {
    class BoundaryTableDialog;
}
class MediatorClass;
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
    void onUploadTracksTriggered();
    void onViewTracksTriggered();
    void onUploadAreaOfViewTriggered();
    void onViewAreaOfViewTriggered();
    void onUploadBoundaryTriggered();
    void onViewBoundaryTriggered();
    void onMapContextMenuRequested(const QPoint &globalPos, const QPoint &localPos, const GISApp::Core::Models::GeoCoordinate &coordinate);

private:
    void showTrackDetailsDialog(const GISApp::Core::Models::TrackRecord &track);
    void showCoordinatesDialog(const GISApp::Core::Models::GeoCoordinate &coordinate);
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
    void focusOnAreaOfView();

    GISApp::Layers::LayerManager *m_layerManager{nullptr};
    GISApp::UI::LayerTreeFloatingWidget *m_layerFloatingPanel{nullptr};

    // Modeless Dialog Instances
    GISApp::UI::Publishing::PublishLayerDialog *m_publishDialog{nullptr};
    GISApp::UI::Publishing::GroupManagerDialog *m_groupManagerDialog{nullptr};
    GISApp::UI::Download::DownloadSatImageryDialog *m_downloadSatDialog{nullptr};
    GISApp::UI::Tasks::BackgroundTaskDialog *m_backgroundTaskDialog{nullptr};
    GISApp::Core::Repositories::TrackRepository *m_trackRepository{nullptr};
    GISApp::Core::Services::MapLibreTrackAdapter *m_trackAdapter{nullptr};
    GISApp::UI::Tracks::TracksTableDialog *m_tracksDialog{nullptr};
    GISApp::Core::Repositories::AreaOfViewRepository *m_areaOfViewRepository{nullptr};
    GISApp::Core::Services::MapLibreAreaOfViewAdapter *m_areaOfViewAdapter{nullptr};
    GISApp::Core::Repositories::GenericEntityRepository *m_genericEntityRepository{nullptr};
    GISApp::Core::Services::MapLibreGenericEntityAdapter *m_genericEntityAdapter{nullptr};
    GISApp::Core::Services::MapLibreBoundaryAdapter *m_boundaryAdapter{nullptr};
    GISApp::UI::Boundary::BoundaryTableDialog *m_boundaryDialog{nullptr};
    QVector<GISApp::Core::Models::BoundaryRecord> m_userBoundaries;


    MediatorClass *m_udpMediator{nullptr};
    QTimer *m_styleDebounceTimer{nullptr};
};

#endif // MAINWINDOW_H
