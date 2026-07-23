/****************************************************************************
** Meta object code from reading C++ file 'deviceinfo.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/deviceinfo.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'deviceinfo.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero12HardwareInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::HardwareInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero12HardwareInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::HardwareInfo",
        "version",
        "target",
        "body",
        "connect"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'version'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'target'
        QtMocHelpers::PropertyData<QString>(2, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'body'
        QtMocHelpers::PropertyData<QString>(3, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'connect'
        QtMocHelpers::PropertyData<QString>(4, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<HardwareInfo, qt_meta_tag_ZN7Flipper4Zero12HardwareInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::HardwareInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero12HardwareInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero12HardwareInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero12HardwareInfoE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::HardwareInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<HardwareInfo *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->version; break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->target; break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->body; break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->connect; break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: QtMocHelpers::setProperty(_t->version, *reinterpret_cast<QString*>(_v)); break;
        case 1: QtMocHelpers::setProperty(_t->target, *reinterpret_cast<QString*>(_v)); break;
        case 2: QtMocHelpers::setProperty(_t->body, *reinterpret_cast<QString*>(_v)); break;
        case 3: QtMocHelpers::setProperty(_t->connect, *reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero12SoftwareInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::SoftwareInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero12SoftwareInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::SoftwareInfo",
        "version",
        "commit",
        "branch",
        "channel",
        "date"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'version'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'commit'
        QtMocHelpers::PropertyData<QString>(2, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'branch'
        QtMocHelpers::PropertyData<QString>(3, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'channel'
        QtMocHelpers::PropertyData<QString>(4, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'date'
        QtMocHelpers::PropertyData<QDate>(5, QMetaType::QDate, QMC::DefaultPropertyFlags | QMC::Writable),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SoftwareInfo, qt_meta_tag_ZN7Flipper4Zero12SoftwareInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::SoftwareInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero12SoftwareInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero12SoftwareInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero12SoftwareInfoE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::SoftwareInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<SoftwareInfo *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->version; break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->commit; break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->branch; break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->channel; break;
        case 4: *reinterpret_cast<QDate*>(_v) = _t->date; break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: QtMocHelpers::setProperty(_t->version, *reinterpret_cast<QString*>(_v)); break;
        case 1: QtMocHelpers::setProperty(_t->commit, *reinterpret_cast<QString*>(_v)); break;
        case 2: QtMocHelpers::setProperty(_t->branch, *reinterpret_cast<QString*>(_v)); break;
        case 3: QtMocHelpers::setProperty(_t->channel, *reinterpret_cast<QString*>(_v)); break;
        case 4: QtMocHelpers::setProperty(_t->date, *reinterpret_cast<QDate*>(_v)); break;
        default: break;
        }
    }
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero11StorageInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::StorageInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero11StorageInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::StorageInfo",
        "internalFree",
        "externalFree",
        "isExternalPresent",
        "isAssetsInstalled"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'internalFree'
        QtMocHelpers::PropertyData<int>(1, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'externalFree'
        QtMocHelpers::PropertyData<int>(2, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'isExternalPresent'
        QtMocHelpers::PropertyData<bool>(3, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'isAssetsInstalled'
        QtMocHelpers::PropertyData<bool>(4, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<StorageInfo, qt_meta_tag_ZN7Flipper4Zero11StorageInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::StorageInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11StorageInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11StorageInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero11StorageInfoE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::StorageInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<StorageInfo *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->internalFree; break;
        case 1: *reinterpret_cast<int*>(_v) = _t->externalFree; break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isExternalPresent; break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->isAssetsInstalled; break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: QtMocHelpers::setProperty(_t->internalFree, *reinterpret_cast<int*>(_v)); break;
        case 1: QtMocHelpers::setProperty(_t->externalFree, *reinterpret_cast<int*>(_v)); break;
        case 2: QtMocHelpers::setProperty(_t->isExternalPresent, *reinterpret_cast<bool*>(_v)); break;
        case 3: QtMocHelpers::setProperty(_t->isAssetsInstalled, *reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero10DeviceInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::DeviceInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero10DeviceInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::DeviceInfo",
        "name",
        "model",
        "fusVersion",
        "radioVersion",
        "systemLocation",
        "hardware",
        "Flipper::Zero::HardwareInfo",
        "firmware",
        "Flipper::Zero::SoftwareInfo",
        "storage",
        "Flipper::Zero::StorageInfo",
        "stackType"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'name'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'model'
        QtMocHelpers::PropertyData<QString>(2, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'fusVersion'
        QtMocHelpers::PropertyData<QString>(3, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'radioVersion'
        QtMocHelpers::PropertyData<QString>(4, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'systemLocation'
        QtMocHelpers::PropertyData<QString>(5, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'hardware'
        QtMocHelpers::PropertyData<Flipper::Zero::HardwareInfo>(6, 0x80000000 | 7, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag),
        // property 'firmware'
        QtMocHelpers::PropertyData<Flipper::Zero::SoftwareInfo>(8, 0x80000000 | 9, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag),
        // property 'storage'
        QtMocHelpers::PropertyData<Flipper::Zero::StorageInfo>(10, 0x80000000 | 11, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag),
        // property 'stackType'
        QtMocHelpers::PropertyData<int>(12, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DeviceInfo, qt_meta_tag_ZN7Flipper4Zero10DeviceInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::DeviceInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero10DeviceInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero10DeviceInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero10DeviceInfoE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::DeviceInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<DeviceInfo *>(_o);
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 5:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Flipper::Zero::HardwareInfo >(); break;
        case 6:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Flipper::Zero::SoftwareInfo >(); break;
        case 7:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Flipper::Zero::StorageInfo >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->name; break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->model; break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->fusVersion; break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->radioVersion; break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->systemLocation; break;
        case 5: *reinterpret_cast<Flipper::Zero::HardwareInfo*>(_v) = _t->hardware; break;
        case 6: *reinterpret_cast<Flipper::Zero::SoftwareInfo*>(_v) = _t->firmware; break;
        case 7: *reinterpret_cast<Flipper::Zero::StorageInfo*>(_v) = _t->storage; break;
        case 8: *reinterpret_cast<int*>(_v) = _t->stackType; break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: QtMocHelpers::setProperty(_t->name, *reinterpret_cast<QString*>(_v)); break;
        case 1: QtMocHelpers::setProperty(_t->model, *reinterpret_cast<QString*>(_v)); break;
        case 2: QtMocHelpers::setProperty(_t->fusVersion, *reinterpret_cast<QString*>(_v)); break;
        case 3: QtMocHelpers::setProperty(_t->radioVersion, *reinterpret_cast<QString*>(_v)); break;
        case 4: QtMocHelpers::setProperty(_t->systemLocation, *reinterpret_cast<QString*>(_v)); break;
        case 5: QtMocHelpers::setProperty(_t->hardware, *reinterpret_cast<Flipper::Zero::HardwareInfo*>(_v)); break;
        case 6: QtMocHelpers::setProperty(_t->firmware, *reinterpret_cast<Flipper::Zero::SoftwareInfo*>(_v)); break;
        case 7: QtMocHelpers::setProperty(_t->storage, *reinterpret_cast<Flipper::Zero::StorageInfo*>(_v)); break;
        case 8: QtMocHelpers::setProperty(_t->stackType, *reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}
QT_WARNING_POP
