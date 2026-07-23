/****************************************************************************
** Meta object code from reading C++ file 'deviceinfohelper.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/helper/deviceinfohelper.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'deviceinfohelper.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::AbstractDeviceInfoHelper::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::AbstractDeviceInfoHelper"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AbstractDeviceInfoHelper, qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::AbstractDeviceInfoHelper::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractOperationHelper::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::AbstractDeviceInfoHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AbstractDeviceInfoHelper *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::AbstractDeviceInfoHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::AbstractDeviceInfoHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero24AbstractDeviceInfoHelperE_t>.strings))
        return static_cast<void*>(this);
    return AbstractOperationHelper::qt_metacast(_clname);
}

int Flipper::Zero::AbstractDeviceInfoHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractOperationHelper::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::VCPDeviceInfoHelper::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::VCPDeviceInfoHelper",
        "onSessionStatusChanged",
        ""
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onSessionStatusChanged'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<VCPDeviceInfoHelper, qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::VCPDeviceInfoHelper::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractDeviceInfoHelper::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::VCPDeviceInfoHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VCPDeviceInfoHelper *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onSessionStatusChanged(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *Flipper::Zero::VCPDeviceInfoHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::VCPDeviceInfoHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero19VCPDeviceInfoHelperE_t>.strings))
        return static_cast<void*>(this);
    return AbstractDeviceInfoHelper::qt_metacast(_clname);
}

int Flipper::Zero::VCPDeviceInfoHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractDeviceInfoHelper::qt_metacall(_c, _id, _a);
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
struct qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::DFUDeviceInfoHelper::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::DFUDeviceInfoHelper"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DFUDeviceInfoHelper, qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::DFUDeviceInfoHelper::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractDeviceInfoHelper::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::DFUDeviceInfoHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DFUDeviceInfoHelper *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::DFUDeviceInfoHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::DFUDeviceInfoHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero19DFUDeviceInfoHelperE_t>.strings))
        return static_cast<void*>(this);
    return AbstractDeviceInfoHelper::qt_metacast(_clname);
}

int Flipper::Zero::DFUDeviceInfoHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractDeviceInfoHelper::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
