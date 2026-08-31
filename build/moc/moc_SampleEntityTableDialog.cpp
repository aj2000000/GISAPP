/****************************************************************************
** Meta object code from reading C++ file 'SampleEntityTableDialog.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/ui/entities/SampleEntityTableDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SampleEntityTableDialog.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto GISApp::UI::Entities::SampleEntityTableDialog::qt_create_metaobjectdata<qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GISApp::UI::Entities::SampleEntityTableDialog",
        "zoomToEntityRequested",
        "",
        "lat",
        "lon",
        "refreshData",
        "onZoomToSelected",
        "onDeleteSelected",
        "onClearAll"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'zoomToEntityRequested'
        QtMocHelpers::SignalData<void(double, double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 }, { QMetaType::Double, 4 },
        }}),
        // Slot 'refreshData'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onZoomToSelected'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeleteSelected'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onClearAll'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SampleEntityTableDialog, qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GISApp::UI::Entities::SampleEntityTableDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t>.metaTypes,
    nullptr
} };

void GISApp::UI::Entities::SampleEntityTableDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SampleEntityTableDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->zoomToEntityRequested((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 1: _t->refreshData(); break;
        case 2: _t->onZoomToSelected(); break;
        case 3: _t->onDeleteSelected(); break;
        case 4: _t->onClearAll(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SampleEntityTableDialog::*)(double , double )>(_a, &SampleEntityTableDialog::zoomToEntityRequested, 0))
            return;
    }
}

const QMetaObject *GISApp::UI::Entities::SampleEntityTableDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GISApp::UI::Entities::SampleEntityTableDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6GISApp2UI8Entities23SampleEntityTableDialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int GISApp::UI::Entities::SampleEntityTableDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void GISApp::UI::Entities::SampleEntityTableDialog::zoomToEntityRequested(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}
QT_WARNING_POP
