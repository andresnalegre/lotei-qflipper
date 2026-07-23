/****************************************************************************
** Meta object code from reading C++ file 'wirelessstackupdateoperation.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/toplevel/wirelessstackupdateoperation.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wirelessstackupdateoperation.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::AbstractCore2UpdateOperation::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::AbstractCore2UpdateOperation",
        "nextStateLogic",
        ""
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'nextStateLogic'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AbstractCore2UpdateOperation, qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::AbstractCore2UpdateOperation::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractTopLevelOperation::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::AbstractCore2UpdateOperation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AbstractCore2UpdateOperation *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->nextStateLogic(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *Flipper::Zero::AbstractCore2UpdateOperation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::AbstractCore2UpdateOperation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero28AbstractCore2UpdateOperationE_t>.strings))
        return static_cast<void*>(this);
    return AbstractTopLevelOperation::qt_metacast(_clname);
}

int Flipper::Zero::AbstractCore2UpdateOperation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractTopLevelOperation::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::WirelessStackUpdateOperation::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::WirelessStackUpdateOperation"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WirelessStackUpdateOperation, qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::WirelessStackUpdateOperation::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractCore2UpdateOperation::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::WirelessStackUpdateOperation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WirelessStackUpdateOperation *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::WirelessStackUpdateOperation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::WirelessStackUpdateOperation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero28WirelessStackUpdateOperationE_t>.strings))
        return static_cast<void*>(this);
    return AbstractCore2UpdateOperation::qt_metacast(_clname);
}

int Flipper::Zero::WirelessStackUpdateOperation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractCore2UpdateOperation::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::FUSUpdateOperation::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::FUSUpdateOperation"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FUSUpdateOperation, qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::FUSUpdateOperation::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractCore2UpdateOperation::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::FUSUpdateOperation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FUSUpdateOperation *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::FUSUpdateOperation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::FUSUpdateOperation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero18FUSUpdateOperationE_t>.strings))
        return static_cast<void*>(this);
    return AbstractCore2UpdateOperation::qt_metacast(_clname);
}

int Flipper::Zero::FUSUpdateOperation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractCore2UpdateOperation::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
