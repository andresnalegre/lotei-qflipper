/****************************************************************************
** Meta object code from reading C++ file 'setbootmodeoperation.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/recovery/setbootmodeoperation.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'setbootmodeoperation.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::SetBootModeOperation::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::SetBootModeOperation",
        "nextStateLogic",
        "",
        "onOperationTimeout"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'nextStateLogic'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOperationTimeout'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SetBootModeOperation, qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::SetBootModeOperation::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractRecoveryOperation::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::SetBootModeOperation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SetBootModeOperation *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->nextStateLogic(); break;
        case 1: _t->onOperationTimeout(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *Flipper::Zero::SetBootModeOperation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::SetBootModeOperation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20SetBootModeOperationE_t>.strings))
        return static_cast<void*>(this);
    return AbstractRecoveryOperation::qt_metacast(_clname);
}

int Flipper::Zero::SetBootModeOperation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractRecoveryOperation::qt_metacall(_c, _id, _a);
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
namespace {
struct qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::SetRecoveryBootOperation::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::SetRecoveryBootOperation"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SetRecoveryBootOperation, qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::SetRecoveryBootOperation::staticMetaObject = { {
    QMetaObject::SuperData::link<SetBootModeOperation::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::SetRecoveryBootOperation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SetRecoveryBootOperation *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::SetRecoveryBootOperation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::SetRecoveryBootOperation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero24SetRecoveryBootOperationE_t>.strings))
        return static_cast<void*>(this);
    return SetBootModeOperation::qt_metacast(_clname);
}

int Flipper::Zero::SetRecoveryBootOperation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SetBootModeOperation::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::SetOSBootOperation::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::SetOSBootOperation"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SetOSBootOperation, qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::SetOSBootOperation::staticMetaObject = { {
    QMetaObject::SuperData::link<SetBootModeOperation::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::SetOSBootOperation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SetOSBootOperation *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::SetOSBootOperation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::SetOSBootOperation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero18SetOSBootOperationE_t>.strings))
        return static_cast<void*>(this);
    return SetBootModeOperation::qt_metacast(_clname);
}

int Flipper::Zero::SetOSBootOperation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SetBootModeOperation::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
