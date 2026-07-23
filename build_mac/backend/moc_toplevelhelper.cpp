/****************************************************************************
** Meta object code from reading C++ file 'toplevelhelper.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/helper/toplevelhelper.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'toplevelhelper.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::AbstractTopLevelHelper::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::AbstractTopLevelHelper",
        "onUpdateRegistryStateChanged",
        ""
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onUpdateRegistryStateChanged'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AbstractTopLevelHelper, qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::AbstractTopLevelHelper::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractOperationHelper::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::AbstractTopLevelHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AbstractTopLevelHelper *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onUpdateRegistryStateChanged(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *Flipper::Zero::AbstractTopLevelHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::AbstractTopLevelHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero22AbstractTopLevelHelperE_t>.strings))
        return static_cast<void*>(this);
    return AbstractOperationHelper::qt_metacast(_clname);
}

int Flipper::Zero::AbstractTopLevelHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractOperationHelper::qt_metacall(_c, _id, _a);
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
struct qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::UpdateTopLevelHelper::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::UpdateTopLevelHelper"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<UpdateTopLevelHelper, qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::UpdateTopLevelHelper::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractTopLevelHelper::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::UpdateTopLevelHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UpdateTopLevelHelper *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::UpdateTopLevelHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::UpdateTopLevelHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20UpdateTopLevelHelperE_t>.strings))
        return static_cast<void*>(this);
    return AbstractTopLevelHelper::qt_metacast(_clname);
}

int Flipper::Zero::UpdateTopLevelHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractTopLevelHelper::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::RepairTopLevelHelper::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::RepairTopLevelHelper"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RepairTopLevelHelper, qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::RepairTopLevelHelper::staticMetaObject = { {
    QMetaObject::SuperData::link<AbstractTopLevelHelper::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::RepairTopLevelHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RepairTopLevelHelper *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::Zero::RepairTopLevelHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::RepairTopLevelHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero20RepairTopLevelHelperE_t>.strings))
        return static_cast<void*>(this);
    return AbstractTopLevelHelper::qt_metacast(_clname);
}

int Flipper::Zero::RepairTopLevelHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractTopLevelHelper::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
