QT += core gui widgets opengl network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Build Output Directories
DESTDIR     = $$PWD/build/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR     = $$PWD/build/moc
UI_DIR      = $$PWD/build/ui


INCLUDEPATH += /home/crl/maplibre-install/include \
               /home/crl/maplibre-install/include/QMapLibre \
               /home/crl/maplibre-install/include/QMapLibreWidgets

LIBS += -L/home/crl/maplibre-install/lib -lQMapLibre -lQMapLibreWidgets

QMAKE_LFLAGS += -Wl,-rpath,/home/crl/maplibre-install/lib


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
    src/map/OverlayWidget.cpp




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
    src/map/OverlayWidget.h




FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
