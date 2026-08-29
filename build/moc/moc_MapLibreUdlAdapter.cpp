/****************************************************************************
** Meta object code from reading C++ file 'MapLibreUdlAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/core/services/MapLibreUdlAdapter.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MapLibreUdlAdapter.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::Core::Services::MapLibreUdlAdapter::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::Core::Services::MapLibreUdlAdapter",
        "refreshFromRepository",
        "",
        "onEntityAdded",
        "std::shared_ptr<GenericGisEntity>",
        "entity",
        "onEntityUpdated",
        "onEntityRemoved",
        "entityId"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'refreshFromRepository'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onEntityAdded'
        QtMocHelpers::SlotData<void(std::shared_ptr<GenericGisEntity>)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'onEntityUpdated'
        QtMocHelpers::SlotData<void(std::shared_ptr<GenericGisEntity>)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'onEntityRemoved'
        QtMocHelpers::SlotData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MapLibreUdlAdapter, qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::Core::Services::MapLibreUdlAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t>.metaTypes,
    nullptr
} };

void GISApp::Core::Services::MapLibreUdlAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MapLibreUdlAdapter *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->refreshFromRepository(); break;
        case 1: _t->onEntityAdded((*reinterpret_cast<std::add_pointer_t<std::shared_ptr<GenericGisEntity>>>(_a[1]))); break;
        case 2: _t->onEntityUpdated((*reinterpret_cast<std::add_pointer_t<std::shared_ptr<GenericGisEntity>>>(_a[1]))); break;
        case 3: _t->onEntityRemoved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *GISApp::Core::Services::MapLibreUdlAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::Core::Services::MapLibreUdlAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core8Services18MapLibreUdlAdapterE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Renderers::IMapRendererAdapter"))
        return static_cast< Renderers::IMapRendererAdapter*>(this);
    if (!strcmp(_clname, "Layers::ILayerAdapter"))
        return static_cast< Layers::ILayerAdapter*>(this);
    return QObject::qt_metacast(_clname);
}

int GISApp::Core::Services::MapLibreUdlAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
