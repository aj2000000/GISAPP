/****************************************************************************
** Meta object code from reading C++ file 'UdlToolbarWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/ui/udl/UdlToolbarWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UdlToolbarWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::UI::UDL::UdlToolbarWidget::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::UI::UDL::UdlToolbarWidget",
        "activeLayerChanged",
        "",
        "layerId",
        "layerName",
        "toolSelected",
        "UdlGeometryType",
        "type",
        "toolDeactivated",
        "createLayerRequested",
        "manageEntitiesRequested",
        "undoRequested",
        "pendingTextLabelChanged",
        "text",
        "colorsChanged",
        "QColor",
        "strokeColor",
        "fillColor",
        "continuousModeChanged",
        "enabled",
        "quickNameModeChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'activeLayerChanged'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'toolSelected'
        QtMocHelpers::SignalData<void(UdlGeometryType)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'toolDeactivated'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'createLayerRequested'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'manageEntitiesRequested'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'undoRequested'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pendingTextLabelChanged'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'colorsChanged'
        QtMocHelpers::SignalData<void(const QColor &, const QColor &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 16 }, { 0x80000000 | 15, 17 },
        }}),
        // Signal 'continuousModeChanged'
        QtMocHelpers::SignalData<void(bool)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 19 },
        }}),
        // Signal 'quickNameModeChanged'
        QtMocHelpers::SignalData<void(bool)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<UdlToolbarWidget, qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::UI::UDL::UdlToolbarWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t>.metaTypes,
    nullptr
} };

void GISApp::UI::UDL::UdlToolbarWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UdlToolbarWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->activeLayerChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->toolSelected((*reinterpret_cast<std::add_pointer_t<UdlGeometryType>>(_a[1]))); break;
        case 2: _t->toolDeactivated(); break;
        case 3: _t->createLayerRequested(); break;
        case 4: _t->manageEntitiesRequested(); break;
        case 5: _t->undoRequested(); break;
        case 6: _t->pendingTextLabelChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->colorsChanged((*reinterpret_cast<std::add_pointer_t<QColor>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QColor>>(_a[2]))); break;
        case 8: _t->continuousModeChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->quickNameModeChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)(const QString & , const QString & )>(_a, &UdlToolbarWidget::activeLayerChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)(UdlGeometryType )>(_a, &UdlToolbarWidget::toolSelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)()>(_a, &UdlToolbarWidget::toolDeactivated, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)()>(_a, &UdlToolbarWidget::createLayerRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)()>(_a, &UdlToolbarWidget::manageEntitiesRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)()>(_a, &UdlToolbarWidget::undoRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)(const QString & )>(_a, &UdlToolbarWidget::pendingTextLabelChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)(const QColor & , const QColor & )>(_a, &UdlToolbarWidget::colorsChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)(bool )>(_a, &UdlToolbarWidget::continuousModeChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (UdlToolbarWidget::*)(bool )>(_a, &UdlToolbarWidget::quickNameModeChanged, 9))
            return;
    }
}

const QMetaObject *GISApp::UI::UDL::UdlToolbarWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::UI::UDL::UdlToolbarWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI3UDL16UdlToolbarWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int GISApp::UI::UDL::UdlToolbarWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void GISApp::UI::UDL::UdlToolbarWidget::activeLayerChanged(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void GISApp::UI::UDL::UdlToolbarWidget::toolSelected(UdlGeometryType _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GISApp::UI::UDL::UdlToolbarWidget::toolDeactivated()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GISApp::UI::UDL::UdlToolbarWidget::createLayerRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void GISApp::UI::UDL::UdlToolbarWidget::manageEntitiesRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void GISApp::UI::UDL::UdlToolbarWidget::undoRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void GISApp::UI::UDL::UdlToolbarWidget::pendingTextLabelChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void GISApp::UI::UDL::UdlToolbarWidget::colorsChanged(const QColor & _t1, const QColor & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}

// SIGNAL 8
void GISApp::UI::UDL::UdlToolbarWidget::continuousModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void GISApp::UI::UDL::UdlToolbarWidget::quickNameModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}
QT_WARNING_POP
