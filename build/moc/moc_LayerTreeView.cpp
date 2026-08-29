/****************************************************************************
** Meta object code from reading C++ file 'LayerTreeView.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/ui/layertree/LayerTreeView.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LayerTreeView.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::UI::LayerTreeView::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::UI::LayerTreeView",
        "zoomToExtentRequested",
        "",
        "GISApp::Layers::LayerExtent",
        "extent",
        "ingestAreaOfViewRequested",
        "ingestTracksRequested"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'zoomToExtentRequested'
        QtMocHelpers::SignalData<void(const GISApp::Layers::LayerExtent &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'ingestAreaOfViewRequested'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ingestTracksRequested'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LayerTreeView, qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::UI::LayerTreeView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeView::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t>.metaTypes,
    nullptr
} };

void GISApp::UI::LayerTreeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LayerTreeView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->zoomToExtentRequested((*reinterpret_cast<std::add_pointer_t<GISApp::Layers::LayerExtent>>(_a[1]))); break;
        case 1: _t->ingestAreaOfViewRequested(); break;
        case 2: _t->ingestTracksRequested(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LayerTreeView::*)(const GISApp::Layers::LayerExtent & )>(_a, &LayerTreeView::zoomToExtentRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LayerTreeView::*)()>(_a, &LayerTreeView::ingestAreaOfViewRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LayerTreeView::*)()>(_a, &LayerTreeView::ingestTracksRequested, 2))
            return;
    }
}

const QMetaObject *GISApp::UI::LayerTreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::UI::LayerTreeView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI13LayerTreeViewE_t>.strings))
        return static_cast<void*>(this);
    return QTreeView::qt_metacast(_clname);
}

int GISApp::UI::LayerTreeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
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
void GISApp::UI::LayerTreeView::zoomToExtentRequested(const GISApp::Layers::LayerExtent & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GISApp::UI::LayerTreeView::ingestAreaOfViewRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void GISApp::UI::LayerTreeView::ingestTracksRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
