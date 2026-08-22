/**
 * @file mainwindow.h
 * @brief Main Window housing Map View, Toolbar, Status Bar, and Interactive Strategy Tools.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QToolBar>
#include <QComboBox>
#include <memory>
#include "map/MapLibreWidget.h"
#include "controllers/MapController.h"
#include "controllers/ToolManager.h"
#include "tools/MeasureTool.h"
#include "core/models/GeoCoordinate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onMouseCoordinateChanged(const GISApp::Core::Models::GeoCoordinate &coordinate);
    void onDistanceUpdated(double totalDistanceKm);

private:
    Ui::MainWindow *ui;
    GISApp::Map::MapLibreWidget *m_mapWidget;
    GISApp::Controllers::MapController *m_mapController;
    GISApp::Controllers::ToolManager *m_toolManager;

    std::shared_ptr<GISApp::Tools::MeasureTool> m_measureTool;

    QLabel *m_coordLabel;
    QLabel *m_measureLabel;
    QToolBar *m_toolBar;
    QComboBox *m_styleCombo;

    void setupMapView();
    void setupToolBar();
    void setupStatusBar();
};

#endif // MAINWINDOW_H
