/****************************************************************************
** Meta object code from reading C++ file 'inputevent.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/inputevent.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'inputevent.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10InputEventE_t {};
} // unnamed namespace

template <> constexpr inline auto InputEvent::qt_create_metaobjectdata<qt_meta_tag_ZN10InputEventE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "InputEvent",
        "Key",
        "Up",
        "Down",
        "Right",
        "Left",
        "Ok",
        "Back",
        "Type",
        "Press",
        "Release",
        "Short",
        "Long",
        "Repeat"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Key'
        QtMocHelpers::EnumData<enum Key>(1, 1, QMC::EnumFlags{}).add({
            {    2, Key::Up },
            {    3, Key::Down },
            {    4, Key::Right },
            {    5, Key::Left },
            {    6, Key::Ok },
            {    7, Key::Back },
        }),
        // enum 'Type'
        QtMocHelpers::EnumData<enum Type>(8, 8, QMC::EnumFlags{}).add({
            {    9, Type::Press },
            {   10, Type::Release },
            {   11, Type::Short },
            {   12, Type::Long },
            {   13, Type::Repeat },
        }),
    };
    return QtMocHelpers::metaObjectData<InputEvent, qt_meta_tag_ZN10InputEventE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject InputEvent::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10InputEventE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10InputEventE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10InputEventE_t>.metaTypes,
    nullptr
} };

void InputEvent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<InputEvent *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *InputEvent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InputEvent::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10InputEventE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int InputEvent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
