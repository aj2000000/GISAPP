QT += core gui widgets opengl network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Build Output Directories
DESTDIR     = $$PWD/build/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR     = $$PWD/build/moc
UI_DIR      = $$PWD/build/ui


INCLUDEPATH += /home/aman/maplibre-install/include \
               /home/aman/maplibre-install/include/QMapLibre \
               /home/aman/maplibre-install/include/QMapLibreWidgets

LIBS += -L/home/aman/maplibre-install/lib -lQMapLibre -lQMapLibreWidgets

QMAKE_LFLAGS += -Wl,-rpath,/home/aman/maplibre-install/lib


INCLUDEPATH += $$PWD/src

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
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
    src/ui/layertree/LayerItemDelegate.cpp


HEADERS += \
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
    src/ui/layertree/LayerItemDelegate.h



FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
