/****************************************************************************
** Meta object code from reading C++ file 'deviceregistry.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/deviceregistry.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'deviceregistry.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper14DeviceRegistryE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::DeviceRegistry::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper14DeviceRegistryE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::DeviceRegistry",
        "isQueryInProgressChanged",
        "",
        "currentDeviceChanged",
        "deviceCountChanged",
        "errorOccured",
        "insertDevice",
        "USBDeviceInfo",
        "info",
        "removeDevice",
        "removeOfflineDevices",
        "connectBleDevice",
        "name",
        "Flipper::Zero::TransportFactory",
        "factory",
        "removeBleDevice",
        "processDevice"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'isQueryInProgressChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentDeviceChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'deviceCountChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'errorOccured'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'insertDevice'
        QtMocHelpers::SlotData<void(const USBDeviceInfo &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'removeDevice'
        QtMocHelpers::SlotData<void(const USBDeviceInfo &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'removeOfflineDevices'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectBleDevice'
        QtMocHelpers::SlotData<void(const QString &, const Flipper::Zero::TransportFactory &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { 0x80000000 | 13, 14 },
        }}),
        // Slot 'removeBleDevice'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processDevice'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DeviceRegistry, qt_meta_tag_ZN7Flipper14DeviceRegistryE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::DeviceRegistry::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper14DeviceRegistryE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper14DeviceRegistryE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper14DeviceRegistryE_t>.metaTypes,
    nullptr
} };

void Flipper::DeviceRegistry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DeviceRegistry *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->isQueryInProgressChanged(); break;
        case 1: _t->currentDeviceChanged(); break;
        case 2: _t->deviceCountChanged(); break;
        case 3: _t->errorOccured(); break;
        case 4: _t->insertDevice((*reinterpret_cast<std::add_pointer_t<USBDeviceInfo>>(_a[1]))); break;
        case 5: _t->removeDevice((*reinterpret_cast<std::add_pointer_t<USBDeviceInfo>>(_a[1]))); break;
        case 6: _t->removeOfflineDevices(); break;
        case 7: _t->connectBleDevice((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Flipper::Zero::TransportFactory>>(_a[2]))); break;
        case 8: _t->removeBleDevice(); break;
        case 9: _t->processDevice(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< USBDeviceInfo >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< USBDeviceInfo >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DeviceRegistry::*)()>(_a, &DeviceRegistry::isQueryInProgressChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceRegistry::*)()>(_a, &DeviceRegistry::currentDeviceChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceRegistry::*)()>(_a, &DeviceRegistry::deviceCountChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DeviceRegistry::*)()>(_a, &DeviceRegistry::errorOccured, 3))
            return;
    }
}

const QMetaObject *Flipper::DeviceRegistry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::DeviceRegistry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper14DeviceRegistryE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Flipper::DeviceRegistry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Flipper::DeviceRegistry::isQueryInProgressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Flipper::DeviceRegistry::currentDeviceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Flipper::DeviceRegistry::deviceCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Flipper::DeviceRegistry::errorOccured()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
