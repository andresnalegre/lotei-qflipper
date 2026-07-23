/****************************************************************************
** Meta object code from reading C++ file 'firmwareupdateregistry.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/firmwareupdateregistry.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'firmwareupdateregistry.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::FirmwareUpdateRegistry::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::FirmwareUpdateRegistry"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FirmwareUpdateRegistry, qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::FirmwareUpdateRegistry::staticMetaObject = { {
    QMetaObject::SuperData::link<UpdateRegistry::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t>.metaTypes,
    nullptr
} };

void Flipper::FirmwareUpdateRegistry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FirmwareUpdateRegistry *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *Flipper::FirmwareUpdateRegistry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::FirmwareUpdateRegistry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper22FirmwareUpdateRegistryE_t>.strings))
        return static_cast<void*>(this);
    return UpdateRegistry::qt_metacast(_clname);
}

int Flipper::FirmwareUpdateRegistry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = UpdateRegistry::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
