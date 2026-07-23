/****************************************************************************
** Meta object code from reading C++ file 'flipperupdates.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperupdates.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'flipperupdates.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper7Updates8FileInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Updates::FileInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper7Updates8FileInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Updates::FileInfo"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FileInfo, qt_meta_tag_ZN7Flipper7Updates8FileInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Updates::FileInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper7Updates8FileInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper7Updates8FileInfoE_t>.data,
    nullptr,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper7Updates8FileInfoE_t>.metaTypes,
    nullptr
} };

namespace {
struct qt_meta_tag_ZN7Flipper7Updates11VersionInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Updates::VersionInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper7Updates11VersionInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Updates::VersionInfo",
        "number",
        "changelog",
        "date",
        "files",
        "QList<Flipper::Updates::FileInfo>"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'number'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'changelog'
        QtMocHelpers::PropertyData<QString>(2, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'date'
        QtMocHelpers::PropertyData<QDate>(3, QMetaType::QDate, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'files'
        QtMocHelpers::PropertyData<QList<Flipper::Updates::FileInfo>>(4, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<VersionInfo, qt_meta_tag_ZN7Flipper7Updates11VersionInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Updates::VersionInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper7Updates11VersionInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper7Updates11VersionInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper7Updates11VersionInfoE_t>.metaTypes,
    nullptr
} };

void Flipper::Updates::VersionInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<VersionInfo *>(_o);
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<Flipper::Updates::FileInfo> >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->number(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->changelog(); break;
        case 2: *reinterpret_cast<QDate*>(_v) = _t->date(); break;
        case 3: *reinterpret_cast<QList<Flipper::Updates::FileInfo>*>(_v) = _t->files(); break;
        default: break;
        }
    }
}
namespace {
struct qt_meta_tag_ZN7Flipper7Updates11ChannelInfoE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Updates::ChannelInfo::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper7Updates11ChannelInfoE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Updates::ChannelInfo",
        "name",
        "title",
        "description",
        "versions",
        "QList<Flipper::Updates::VersionInfo>",
        "latestVersion",
        "Flipper::Updates::VersionInfo"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'name'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'title'
        QtMocHelpers::PropertyData<QString>(2, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'description'
        QtMocHelpers::PropertyData<QString>(3, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'versions'
        QtMocHelpers::PropertyData<QList<Flipper::Updates::VersionInfo>>(4, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'latestVersion'
        QtMocHelpers::PropertyData<Flipper::Updates::VersionInfo>(6, 0x80000000 | 7, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ChannelInfo, qt_meta_tag_ZN7Flipper7Updates11ChannelInfoE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Updates::ChannelInfo::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper7Updates11ChannelInfoE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper7Updates11ChannelInfoE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper7Updates11ChannelInfoE_t>.metaTypes,
    nullptr
} };

void Flipper::Updates::ChannelInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<ChannelInfo *>(_o);
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Flipper::Updates::VersionInfo >(); break;
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<Flipper::Updates::VersionInfo> >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->name(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->title(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->description(); break;
        case 3: *reinterpret_cast<QList<Flipper::Updates::VersionInfo>*>(_v) = _t->versions(); break;
        case 4: *reinterpret_cast<Flipper::Updates::VersionInfo*>(_v) = _t->latestVersion(); break;
        default: break;
        }
    }
}
QT_WARNING_POP
