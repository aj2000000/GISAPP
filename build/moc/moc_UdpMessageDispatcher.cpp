/****************************************************************************
** Meta object code from reading C++ file 'UdpMessageDispatcher.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/core/Udp/handlers/UdpMessageDispatcher.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UdpMessageDispatcher.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::Core::Udp::Handlers::UdpMessageDispatcher::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::Core::Udp::Handlers::UdpMessageDispatcher",
        "telemetryLayerUpdated",
        "",
        "layerName",
        "geoJsonPath",
        "dispatchMessage",
        "payload"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'telemetryLayerUpdated'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 4 },
        }}),
        // Slot 'dispatchMessage'
        QtMocHelpers::SlotData<void(const QByteArray &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QByteArray, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<UdpMessageDispatcher, qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::Core::Udp::Handlers::UdpMessageDispatcher::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t>.metaTypes,
    nullptr
} };

void GISApp::Core::Udp::Handlers::UdpMessageDispatcher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UdpMessageDispatcher *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->telemetryLayerUpdated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->dispatchMessage((*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (UdpMessageDispatcher::*)(const QString & , const QString & )>(_a, &UdpMessageDispatcher::telemetryLayerUpdated, 0))
            return;
    }
}

const QMetaObject *GISApp::Core::Udp::Handlers::UdpMessageDispatcher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::Core::Udp::Handlers::UdpMessageDispatcher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core3Udp8Handlers20UdpMessageDispatcherE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GISApp::Core::Udp::Handlers::UdpMessageDispatcher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void GISApp::Core::Udp::Handlers::UdpMessageDispatcher::telemetryLayerUpdated(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}
QT_WARNING_POP
