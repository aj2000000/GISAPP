/****************************************************************************
** Meta object code from reading C++ file 'CUSTOM_MESSAGE.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/core/interfaces/CUSTOM_MESSAGE.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CUSTOM_MESSAGE.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::Core::Services::CUSTOM_MESSAGE::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::Core::Services::CUSTOM_MESSAGE",
        "messageProcessed",
        "",
        "entityCount",
        "layerUpdated",
        "layerName",
        "geoJsonPath",
        "tableDataUpdated",
        "tableName"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageProcessed'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'layerUpdated'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::QString, 6 },
        }}),
        // Signal 'tableDataUpdated'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CUSTOM_MESSAGE, qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::Core::Services::CUSTOM_MESSAGE::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t>.metaTypes,
    nullptr
} };

void GISApp::Core::Services::CUSTOM_MESSAGE::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CUSTOM_MESSAGE *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageProcessed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->layerUpdated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->tableDataUpdated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CUSTOM_MESSAGE::*)(int )>(_a, &CUSTOM_MESSAGE::messageProcessed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CUSTOM_MESSAGE::*)(const QString & , const QString & )>(_a, &CUSTOM_MESSAGE::layerUpdated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CUSTOM_MESSAGE::*)(const QString & )>(_a, &CUSTOM_MESSAGE::tableDataUpdated, 2))
            return;
    }
}

const QMetaObject *GISApp::Core::Services::CUSTOM_MESSAGE::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::Core::Services::CUSTOM_MESSAGE::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp4Core8Services14CUSTOM_MESSAGEE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GISApp::Core::Services::CUSTOM_MESSAGE::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void GISApp::Core::Services::CUSTOM_MESSAGE::messageProcessed(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GISApp::Core::Services::CUSTOM_MESSAGE::layerUpdated(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void GISApp::Core::Services::CUSTOM_MESSAGE::tableDataUpdated(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
