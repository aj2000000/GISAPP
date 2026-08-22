/****************************************************************************
** Meta object code from reading C++ file 'MapLibreWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/map/MapLibreWidget.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MapLibreWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::Map::MapLibreWidget::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::Map::MapLibreWidget",
        "mouseCoordinateChanged",
        "",
        "GISApp::Core::Models::GeoCoordinate",
        "coordinate",
        "mouseMoved",
        "QMouseEvent*",
        "event",
        "mousePressed",
        "mouseReleased"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'mouseCoordinateChanged'
        QtMocHelpers::SignalData<void(const GISApp::Core::Models::GeoCoordinate &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'mouseMoved'
        QtMocHelpers::SignalData<void(QMouseEvent *, const GISApp::Core::Models::GeoCoordinate &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 }, { 0x80000000 | 3, 4 },
        }}),
        // Signal 'mousePressed'
        QtMocHelpers::SignalData<void(QMouseEvent *, const GISApp::Core::Models::GeoCoordinate &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 }, { 0x80000000 | 3, 4 },
        }}),
        // Signal 'mouseReleased'
        QtMocHelpers::SignalData<void(QMouseEvent *, const GISApp::Core::Models::GeoCoordinate &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 }, { 0x80000000 | 3, 4 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MapLibreWidget, qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::Map::MapLibreWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t>.metaTypes,
    nullptr
} };

void GISApp::Map::MapLibreWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MapLibreWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->mouseCoordinateChanged((*reinterpret_cast<std::add_pointer_t<GISApp::Core::Models::GeoCoordinate>>(_a[1]))); break;
        case 1: _t->mouseMoved((*reinterpret_cast<std::add_pointer_t<QMouseEvent*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<GISApp::Core::Models::GeoCoordinate>>(_a[2]))); break;
        case 2: _t->mousePressed((*reinterpret_cast<std::add_pointer_t<QMouseEvent*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<GISApp::Core::Models::GeoCoordinate>>(_a[2]))); break;
        case 3: _t->mouseReleased((*reinterpret_cast<std::add_pointer_t<QMouseEvent*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<GISApp::Core::Models::GeoCoordinate>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MapLibreWidget::*)(const GISApp::Core::Models::GeoCoordinate & )>(_a, &MapLibreWidget::mouseCoordinateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapLibreWidget::*)(QMouseEvent * , const GISApp::Core::Models::GeoCoordinate & )>(_a, &MapLibreWidget::mouseMoved, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapLibreWidget::*)(QMouseEvent * , const GISApp::Core::Models::GeoCoordinate & )>(_a, &MapLibreWidget::mousePressed, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapLibreWidget::*)(QMouseEvent * , const GISApp::Core::Models::GeoCoordinate & )>(_a, &MapLibreWidget::mouseReleased, 3))
            return;
    }
}

const QMetaObject *GISApp::Map::MapLibreWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::Map::MapLibreWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp3Map14MapLibreWidgetE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "GISApp::Core::Interfaces::IMapView"))
        return static_cast< GISApp::Core::Interfaces::IMapView*>(this);
    return QWidget::qt_metacast(_clname);
}

int GISApp::Map::MapLibreWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void GISApp::Map::MapLibreWidget::mouseCoordinateChanged(const GISApp::Core::Models::GeoCoordinate & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GISApp::Map::MapLibreWidget::mouseMoved(QMouseEvent * _t1, const GISApp::Core::Models::GeoCoordinate & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void GISApp::Map::MapLibreWidget::mousePressed(QMouseEvent * _t1, const GISApp::Core::Models::GeoCoordinate & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void GISApp::Map::MapLibreWidget::mouseReleased(QMouseEvent * _t1, const GISApp::Core::Models::GeoCoordinate & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
