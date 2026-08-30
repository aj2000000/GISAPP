/****************************************************************************
** Meta object code from reading C++ file 'UdlDrawingTool.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/tools/UdlDrawingTool.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UdlDrawingTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::Tools::UdlDrawingTool::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::Tools::UdlDrawingTool",
        "waypointsUpdated",
        "",
        "std::vector<GISApp::Core::Models::GeoCoordinate>",
        "waypoints",
        "previewUpdated",
        "GISApp::Core::Models::GeoCoordinate",
        "mouseCoord",
        "GISApp::UI::UDL::UdlGeometryType",
        "geomType",
        "active",
        "QColor",
        "strokeColor",
        "fillColor",
        "entityCreated",
        "entityId",
        "layerId"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'waypointsUpdated'
        QtMocHelpers::SignalData<void(const std::vector<GISApp::Core::Models::GeoCoordinate> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'previewUpdated'
        QtMocHelpers::SignalData<void(const std::vector<GISApp::Core::Models::GeoCoordinate> &, const GISApp::Core::Models::GeoCoordinate &, GISApp::UI::UDL::UdlGeometryType, bool, const QColor &, const QColor &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 6, 7 }, { 0x80000000 | 8, 9 }, { QMetaType::Bool, 10 },
            { 0x80000000 | 11, 12 }, { 0x80000000 | 11, 13 },
        }}),
        // Signal 'entityCreated'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 }, { QMetaType::QString, 16 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<UdlDrawingTool, qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::Tools::UdlDrawingTool::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t>.metaTypes,
    nullptr
} };

void GISApp::Tools::UdlDrawingTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UdlDrawingTool *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->waypointsUpdated((*reinterpret_cast<std::add_pointer_t<std::vector<GISApp::Core::Models::GeoCoordinate>>>(_a[1]))); break;
        case 1: _t->previewUpdated((*reinterpret_cast<std::add_pointer_t<std::vector<GISApp::Core::Models::GeoCoordinate>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<GISApp::Core::Models::GeoCoordinate>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<GISApp::UI::UDL::UdlGeometryType>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QColor>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<QColor>>(_a[6]))); break;
        case 2: _t->entityCreated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (UdlDrawingTool::*)(const std::vector<GISApp::Core::Models::GeoCoordinate> & )>(_a, &UdlDrawingTool::waypointsUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlDrawingTool::*)(const std::vector<GISApp::Core::Models::GeoCoordinate> & , const GISApp::Core::Models::GeoCoordinate & , GISApp::UI::UDL::UdlGeometryType , bool , const QColor & , const QColor & )>(_a, &UdlDrawingTool::previewUpdated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlDrawingTool::*)(const QString & , const QString & )>(_a, &UdlDrawingTool::entityCreated, 2))
            return;
    }
}

const QMetaObject *GISApp::Tools::UdlDrawingTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::Tools::UdlDrawingTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp5Tools14UdlDrawingToolE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "GISApp::Core::Interfaces::ITool"))
        return static_cast< GISApp::Core::Interfaces::ITool*>(this);
    return QObject::qt_metacast(_clname);
}

int GISApp::Tools::UdlDrawingTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void GISApp::Tools::UdlDrawingTool::waypointsUpdated(const std::vector<GISApp::Core::Models::GeoCoordinate> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GISApp::Tools::UdlDrawingTool::previewUpdated(const std::vector<GISApp::Core::Models::GeoCoordinate> & _t1, const GISApp::Core::Models::GeoCoordinate & _t2, GISApp::UI::UDL::UdlGeometryType _t3, bool _t4, const QColor & _t5, const QColor & _t6)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3, _t4, _t5, _t6);
}

// SIGNAL 2
void GISApp::Tools::UdlDrawingTool::entityCreated(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}
QT_WARNING_POP
