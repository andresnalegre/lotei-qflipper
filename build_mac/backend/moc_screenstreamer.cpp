/****************************************************************************
** Meta object code from reading C++ file 'screenstreamer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/screenstreamer.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'screenstreamer.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::ScreenStreamer::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::ScreenStreamer",
        "streamStateChanged",
        "",
        "screenFrameChanged",
        "start",
        "stop",
        "onProtobufSessionStateChanged",
        "onBroadcastResponseReceived",
        "response",
        "sendInputEvent",
        "InputEvent::Key",
        "key",
        "InputEvent::Type",
        "type",
        "screenFrame",
        "ScreenFrame",
        "isEnabled",
        "isPaused",
        "StreamState",
        "Starting",
        "Running",
        "Stopping",
        "Stopped",
        "Paused"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'streamStateChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'screenFrameChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'start'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onProtobufSessionStateChanged'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBroadcastResponseReceived'
        QtMocHelpers::SlotData<void(QObject *)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 8 },
        }}),
        // Method 'sendInputEvent'
        QtMocHelpers::MethodData<void(InputEvent::Key, InputEvent::Type)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { 0x80000000 | 12, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'screenFrame'
        QtMocHelpers::PropertyData<ScreenFrame>(14, 0x80000000 | 15, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'isEnabled'
        QtMocHelpers::PropertyData<bool>(16, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable, 0),
        // property 'isPaused'
        QtMocHelpers::PropertyData<bool>(17, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable, 0),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'StreamState'
        QtMocHelpers::EnumData<enum StreamState>(18, 18, QMC::EnumFlags{}).add({
            {   19, StreamState::Starting },
            {   20, StreamState::Running },
            {   21, StreamState::Stopping },
            {   22, StreamState::Stopped },
            {   23, StreamState::Paused },
        }),
    };
    return QtMocHelpers::metaObjectData<ScreenStreamer, qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::ScreenStreamer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::ScreenStreamer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScreenStreamer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->streamStateChanged(); break;
        case 1: _t->screenFrameChanged(); break;
        case 2: _t->start(); break;
        case 3: _t->stop(); break;
        case 4: _t->onProtobufSessionStateChanged(); break;
        case 5: _t->onBroadcastResponseReceived((*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 6: _t->sendInputEvent((*reinterpret_cast<std::add_pointer_t<InputEvent::Key>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<InputEvent::Type>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScreenStreamer::*)()>(_a, &ScreenStreamer::streamStateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScreenStreamer::*)()>(_a, &ScreenStreamer::screenFrameChanged, 1))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ScreenFrame >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<ScreenFrame*>(_v) = _t->screenFrame(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->isEnabled(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isPaused(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setEnabled(*reinterpret_cast<bool*>(_v)); break;
        case 2: _t->setPaused(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *Flipper::Zero::ScreenStreamer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::ScreenStreamer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero14ScreenStreamerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Flipper::Zero::ScreenStreamer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Flipper::Zero::ScreenStreamer::streamStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Flipper::Zero::ScreenStreamer::screenFrameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
