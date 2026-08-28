QT += core gui widgets opengl network sql svg concurrent xml


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Build Output Directories
DESTDIR     = $$PWD/build/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR     = $$PWD/build/moc
UI_DIR      = $$PWD/build/ui


MAPLIBRE_INSTALL_DIR = $$(MAPLIBRE_INSTALL_DIR)
isEmpty(MAPLIBRE_INSTALL_DIR) {
    MAPLIBRE_INSTALL_DIR = $$(HOME)/maplibre-install
}

INCLUDEPATH += $$MAPLIBRE_INSTALL_DIR/include \
               $$MAPLIBRE_INSTALL_DIR/include/QMapLibre \
               $$MAPLIBRE_INSTALL_DIR/include/QMapLibreWidgets \
               /usr/include/gdal

LIBS += -L$$MAPLIBRE_INSTALL_DIR/lib -lQMapLibre -lQMapLibreWidgets -lgdal

QMAKE_LFLAGS += -Wl,-rpath,$$MAPLIBRE_INSTALL_DIR/lib



INCLUDEPATH += $$PWD/src \
               $$PWD/src/core \
               $$PWD/src/publishing \
               $$PWD/src/ui/publishing \
               $$PWD/src/core/tasks \
               $$PWD/src/ui/tasks \
               $$PWD/src/ui/download \
               $$PWD/src/core/notifications \
               $$PWD/src/ui/notifications \
               $$PWD/src/core/models \
               $$PWD/src/core/database \
               $$PWD/src/core/repositories \
               $$PWD/src/core/services \
               $$PWD/src/core/renderers \
               $$PWD/src/ui/tracks \
               $$PWD/src/ui/area_of_view \
               $$PWD/src/ui/entities

SOURCES += \
    src/main.cpp \
    src/core/SystemConfigManager.cpp \
    src/ui/mainwindow.cpp \
    src/map/MapLibreWidget.cpp \
    src/controllers/MapController.cpp \
    src/tools/MeasureTool.cpp \
    src/tools/PanTool.cpp \
    src/controllers/ToolManager.cpp \
    src/map/OverlayWidget.cpp \
    src/ui/ThemeManager.cpp \
    src/ui/LeftSidebar.cpp \
    src/ui/RightToolPanel.cpp \
    src/ui/ZoomControlsWidget.cpp \
    src/ui/TacticalStatusBar.cpp \
    src/ui/HeaderBar.cpp \
    src/layers/MapLibreLayerAdapter.cpp \
    src/layers/LayerTreeNode.cpp \
    src/layers/LayerTreeModel.cpp \
    src/layers/LayerManager.cpp \
    src/layers/TacticalLayerProvider.cpp \
    src/ui/layertree/LayerTreeView.cpp \
    src/ui/layertree/LayerTreeFloatingWidget.cpp \
    src/ui/layertree/LayerItemDelegate.cpp \
    src/publishing/RasterLayerPublisher.cpp \
    src/publishing/VectorLayerPublisher.cpp \
    src/publishing/LayerPublishingService.cpp \
    src/ui/publishing/PublishLayerDialog.cpp \
    src/ui/publishing/GroupManagerDialog.cpp \
    src/publishing/LocalTileServer.cpp \
    src/publishing/LayerRegistryManager.cpp \
    src/publishing/PublisherFactory.cpp \
    src/core/tasks/FunctionalTask.cpp \
    src/core/tasks/BackgroundTaskManager.cpp \
    src/ui/tasks/BackgroundTaskDialog.cpp \
    src/core/tasks/GoogleSatDownloaderTask.cpp \
    src/ui/download/DownloadSatImageryDialog.cpp \
    src/core/notifications/NotificationFactory.cpp \
    src/core/notifications/NotificationManager.cpp \
    src/ui/notifications/CriticalNotificationStrategy.cpp \
    src/ui/notifications/FlashNotificationStrategy.cpp \
    src/core/models/TrackRecord.cpp \
    src/core/database/DatabaseManager.cpp \
    src/core/repositories/TrackRepository.cpp \
    src/core/services/CsvTrackIngestor.cpp \
    src/core/services/MapLibreTrackAdapter.cpp \
    src/ui/tracks/TrackTableModel.cpp \
    src/ui/tracks/TracksTableDialog.cpp \
    src/ui/tracks/TrackDetailDialog.cpp \
    src/core/models/AreaOfViewRecord.cpp \
    src/core/repositories/AreaOfViewRepository.cpp \
    src/core/services/XmlAreaOfViewIngestor.cpp \
    src/core/services/MapLibreAreaOfViewAdapter.cpp \
    src/ui/area_of_view/AreaOfViewTableDialog.cpp \
    src/core/models/GenericGisEntity.cpp \
    src/core/models/GisEntityRegistry.cpp \
    src/core/repositories/GenericEntityRepository.cpp \
    src/core/services/MapLibreGenericEntityAdapter.cpp \
    src/ui/entities/UniversalEntityEditorDialog.cpp

HEADERS += \
    src/core/SystemConfigManager.h \
    src/ui/mainwindow.h \
    src/core/models/GeoCoordinate.h \
    src/core/interfaces/IMapView.h \
    src/map/MapLibreWidget.h \
    src/controllers/MapController.h \
    src/core/interfaces/ITool.h \
    src/tools/MeasureTool.h \
    src/tools/PanTool.h \
    src/controllers/ToolManager.h \
    src/map/OverlayWidget.h \
    src/ui/ThemeManager.h \
    src/ui/LeftSidebar.h \
    src/ui/RightToolPanel.h \
    src/ui/ZoomControlsWidget.h \
    src/ui/TacticalStatusBar.h \
    src/ui/HeaderBar.h \
    src/layers/ILayerAdapter.h \
    src/layers/LayerTreeNode.h \
    src/layers/LayerTreeModel.h \
    src/layers/LayerManager.h \
    src/layers/TacticalLayerProvider.h \
    src/ui/layertree/LayerTreeView.h \
    src/ui/layertree/LayerTreeFloatingWidget.h \
    src/ui/layertree/LayerItemDelegate.h \
    src/publishing/IPublisherStrategy.h \
    src/publishing/PublisherFactory.h \
    src/publishing/RasterLayerPublisher.h \
    src/publishing/VectorLayerPublisher.h \
    src/publishing/LayerPublishingService.h \
    src/ui/publishing/PublishLayerDialog.h \
    src/ui/publishing/GroupManagerDialog.h \
    src/publishing/LocalTileServer.h \
    src/publishing/LayerRegistryManager.h \
    src/core/tasks/IBackgroundTask.h \
    src/core/tasks/FunctionalTask.h \
    src/core/tasks/BackgroundTaskManager.h \
    src/ui/tasks/BackgroundTaskDialog.h \
    src/core/tasks/GoogleSatDownloaderTask.h \
    src/ui/download/DownloadSatImageryDialog.h \
    src/core/notifications/NotificationTypes.h \
    src/core/notifications/INotificationStrategy.h \
    src/core/notifications/INotificationObserver.h \
    src/core/notifications/NotificationFactory.h \
    src/core/notifications/NotificationManager.h \
    src/ui/notifications/CriticalNotificationStrategy.h \
    src/ui/notifications/FlashNotificationStrategy.h \
    src/core/models/IGisGeometry.h \
    src/core/models/IGisEntity.h \
    src/core/models/TrackRecord.h \
    src/core/renderers/IMapRendererAdapter.h \
    src/core/database/DatabaseManager.h \
    src/core/repositories/ITrackRepository.h \
    src/core/repositories/TrackRepository.h \
    src/core/services/ITrackIngestor.h \
    src/core/services/CsvTrackIngestor.h \
    src/core/services/MapLibreTrackAdapter.h \
    src/ui/tracks/TrackTableModel.h \
    src/ui/tracks/TracksTableDialog.h \
    src/ui/tracks/TrackDetailDialog.h \
    src/core/models/AreaOfViewRecord.h \
    src/core/repositories/IAreaOfViewRepository.h \
    src/core/repositories/AreaOfViewRepository.h \
    src/core/services/XmlAreaOfViewIngestor.h \
    src/core/services/MapLibreAreaOfViewAdapter.h \
    src/ui/area_of_view/AreaOfViewTableDialog.h \
    src/core/models/GenericGisEntity.h \
    src/core/models/GisEntityRegistry.h \
    src/core/repositories/IGisEntityRepository.h \
    src/core/repositories/GenericEntityRepository.h \
    src/core/renderers/IEntityPainter.h \
    src/core/services/MapLibreGenericEntityAdapter.h \
    src/ui/entities/UniversalEntityEditorDialog.h



FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
