/****************************************************************************
** Meta object code from reading C++ file 'LayerItemDelegate.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/ui/layertree/LayerItemDelegate.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LayerItemDelegate.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::UI::LayerItemDelegate::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::UI::LayerItemDelegate",
        "visibilityToggleRequested",
        "",
        "QModelIndex",
        "index",
        "zoomToExtentRequested",
        "GISApp::Layers::LayerExtent",
        "extent",
        "opacityChangeRequested",
        "opacity"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'visibilityToggleRequested'
        QtMocHelpers::SignalData<void(const QModelIndex &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'zoomToExtentRequested'
        QtMocHelpers::SignalData<void(const GISApp::Layers::LayerExtent &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'opacityChangeRequested'
        QtMocHelpers::SignalData<void(const QModelIndex &, float)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Float, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LayerItemDelegate, qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::UI::LayerItemDelegate::staticMetaObject = { {
    QMetaObject::SuperData::link<QStyledItemDelegate::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t>.metaTypes,
    nullptr
} };

void GISApp::UI::LayerItemDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LayerItemDelegate *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->visibilityToggleRequested((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 1: _t->zoomToExtentRequested((*reinterpret_cast<std::add_pointer_t<GISApp::Layers::LayerExtent>>(_a[1]))); break;
        case 2: _t->opacityChangeRequested((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LayerItemDelegate::*)(const QModelIndex & )>(_a, &LayerItemDelegate::visibilityToggleRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LayerItemDelegate::*)(const GISApp::Layers::LayerExtent & )>(_a, &LayerItemDelegate::zoomToExtentRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LayerItemDelegate::*)(const QModelIndex & , float )>(_a, &LayerItemDelegate::opacityChangeRequested, 2))
            return;
    }
}

const QMetaObject *GISApp::UI::LayerItemDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::UI::LayerItemDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI17LayerItemDelegateE_t>.strings))
        return static_cast<void*>(this);
    return QStyledItemDelegate::qt_metacast(_clname);
}

int GISApp::UI::LayerItemDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
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
void GISApp::UI::LayerItemDelegate::visibilityToggleRequested(const QModelIndex & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GISApp::UI::LayerItemDelegate::zoomToExtentRequested(const GISApp::Layers::LayerExtent & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GISApp::UI::LayerItemDelegate::opacityChangeRequested(const QModelIndex & _t1, float _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}
QT_WARNING_POP
