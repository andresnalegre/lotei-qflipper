/****************************************************************************
** Meta object code from reading C++ file 'devicestate.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/devicestate.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'devicestate.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::DeviceState::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::DeviceState",
        "deviceInfoChanged",
        "",
        "isPersistentChanged",
        "isOnlineChanged",
        "statusStringChanged",
        "isErrorChanged",
        "progressChanged",
        "onDeviceInfoChanged",
        "onIsOnlineChanged",
        "info",
        "Flipper::Zero::DeviceInfo",
        "isPersistent",
        "isOnline",
        "isError",
        "isRecoveryMode",
        "statusString",
        "errorString",
        "progress"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'deviceInfoChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isPersistentChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isOnlineChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusStringChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isErrorChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'progressChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDeviceInfoChanged'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onIsOnlineChanged'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'info'
        QtMocHelpers::PropertyData<Flipper::Zero::DeviceInfo>(10, 0x80000000 | 11, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'isPersistent'
        QtMocHelpers::PropertyData<bool>(12, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'isOnline'
        QtMocHelpers::PropertyData<bool>(13, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'isError'
        QtMocHelpers::PropertyData<bool>(14, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
        // property 'isRecoveryMode'
        QtMocHelpers::PropertyData<bool>(15, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'statusString'
        QtMocHelpers::PropertyData<QString>(16, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'errorString'
        QtMocHelpers::PropertyData<QString>(17, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
        // property 'progress'
        QtMocHelpers::PropertyData<double>(18, QMetaType::Double, QMC::DefaultPropertyFlags, 5),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DeviceState, qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::DeviceState::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::DeviceState::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DeviceState *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->deviceInfoChanged(); break;
        case 1: _t->isPersistentChanged(); break;
        case 2: _t->isOnlineChanged(); break;
        case 3: _t->statusStringChanged(); break;
        case 4: _t->isErrorChanged(); break;
        case 5: _t->progressChanged(); break;
        case 6: _t->onDeviceInfoChanged(); break;
        case 7: _t->onIsOnlineChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DeviceState::*)()>(_a, &DeviceState::deviceInfoChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceState::*)()>(_a, &DeviceState::isPersistentChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceState::*)()>(_a, &DeviceState::isOnlineChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceState::*)()>(_a, &DeviceState::statusStringChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceState::*)()>(_a, &DeviceState::isErrorChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceState::*)()>(_a, &DeviceState::progressChanged, 5))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Flipper::Zero::DeviceInfo >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<Flipper::Zero::DeviceInfo*>(_v) = _t->deviceInfo(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->isPersistent(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isOnline(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->isError(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->isRecoveryMode(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->statusString(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->errorString(); break;
        case 7: *reinterpret_cast<double*>(_v) = _t->progress(); break;
        default: break;
        }
    }
}

const QMetaObject *Flipper::Zero::DeviceState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::DeviceState::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11DeviceStateE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Flipper::Zero::DeviceState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Flipper::Zero::DeviceState::deviceInfoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Flipper::Zero::DeviceState::isPersistentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Flipper::Zero::DeviceState::isOnlineChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Flipper::Zero::DeviceState::statusStringChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Flipper::Zero::DeviceState::isErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Flipper::Zero::DeviceState::progressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
