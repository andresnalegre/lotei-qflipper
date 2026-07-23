/****************************************************************************
** Meta object code from reading C++ file 'protobufsession.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/protobufsession.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'protobufsession.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::ProtobufSession::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::ProtobufSession",
        "sessionStateChanged",
        "",
        "broadcastResponseReceived",
        "response",
        "startSession",
        "stopSession",
        "onTransportReady",
        "onSerialPortReadyRead",
        "onSerialPortBytesWriten",
        "nbytes",
        "onSerialPortErrorOccured",
        "processQueue",
        "writeToPort",
        "doStopSession",
        "onCurrentOperationFinished"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sessionStateChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'broadcastResponseReceived'
        QtMocHelpers::SignalData<void(QObject *)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 4 },
        }}),
        // Slot 'startSession'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stopSession'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTransportReady'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSerialPortReadyRead'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSerialPortBytesWriten'
        QtMocHelpers::SlotData<void(qint64)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 10 },
        }}),
        // Slot 'onSerialPortErrorOccured'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'processQueue'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'writeToPort'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'doStopSession'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCurrentOperationFinished'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ProtobufSession, qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::ProtobufSession::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::ProtobufSession::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ProtobufSession *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sessionStateChanged(); break;
        case 1: _t->broadcastResponseReceived((*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->startSession(); break;
        case 3: _t->stopSession(); break;
        case 4: _t->onTransportReady(); break;
        case 5: _t->onSerialPortReadyRead(); break;
        case 6: _t->onSerialPortBytesWriten((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 7: _t->onSerialPortErrorOccured(); break;
        case 8: _t->processQueue(); break;
        case 9: _t->writeToPort(); break;
        case 10: _t->doStopSession(); break;
        case 11: _t->onCurrentOperationFinished(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ProtobufSession::*)()>(_a, &ProtobufSession::sessionStateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ProtobufSession::*)(QObject * )>(_a, &ProtobufSession::broadcastResponseReceived, 1))
            return;
    }
}

const QMetaObject *Flipper::Zero::ProtobufSession::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::ProtobufSession::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero15ProtobufSessionE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Failable"))
        return static_cast< Failable*>(this);
    return QObject::qt_metacast(_clname);
}

int Flipper::Zero::ProtobufSession::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void Flipper::Zero::ProtobufSession::sessionStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Flipper::Zero::ProtobufSession::broadcastResponseReceived(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
