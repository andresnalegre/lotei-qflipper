/****************************************************************************
** Meta object code from reading C++ file 'applicationbackend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/applicationbackend.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/screenstreamer.h"
#include "flipperzero/virtualdisplay.h"
#include "flipperzero/filemanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'applicationbackend.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN18ApplicationBackendE_t {};
} // unnamed namespace

template <> constexpr inline auto ApplicationBackend::qt_create_metaobjectdata<qt_meta_tag_ZN18ApplicationBackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ApplicationBackend",
        "errorTypeChanged",
        "",
        "currentDeviceChanged",
        "backendStateChanged",
        "firmwareUpdateStateChanged",
        "isQueryInProgressChanged",
        "portReleasedChanged",
        "onCurrentDeviceChanged",
        "onDeviceInfoChanged",
        "onDeviceOperationFinished",
        "onDeviceRegistryErrorOccured",
        "onFileManagerErrorOccured",
        "onScreenStreamerStateChanged",
        "mainAction",
        "createBackup",
        "QUrl",
        "backupUrl",
        "restoreBackup",
        "factoryReset",
        "installFirmware",
        "fileUrl",
        "installWirelessStack",
        "installFUS",
        "uint32_t",
        "address",
        "startFullScreenStreaming",
        "stopFullScreenStreaming",
        "refreshStorageInfo",
        "checkFirmwareUpdates",
        "finalizeOperation",
        "reportExternalResult",
        "ok",
        "errorType",
        "releasePort",
        "reacquirePort",
        "backendState",
        "BackendState",
        "deviceState",
        "Flipper::Zero::DeviceState*",
        "screenStreamer",
        "Flipper::Zero::ScreenStreamer*",
        "virtualDisplay",
        "Flipper::Zero::VirtualDisplay*",
        "fileManager",
        "Flipper::Zero::FileManager*",
        "firmwareUpdateState",
        "FirmwareUpdateState",
        "firmwareUpdateModel",
        "QAbstractListModel*",
        "latestFirmwareVersion",
        "Flipper::Updates::VersionInfo",
        "BackendError::ErrorType",
        "isQueryInProgress",
        "portReleased",
        "WaitingForDevices",
        "Ready",
        "ScreenStreaming",
        "UpdatingDevice",
        "RepairingDevice",
        "CreatingBackup",
        "RestoringBackup",
        "FactoryResetting",
        "InstallingFirmware",
        "InstallingWirelessStack",
        "InstallingFUS",
        "Finished",
        "ErrorOccured",
        "Unknown",
        "Checking",
        "CanUpdate",
        "CanInstall",
        "CanRepair",
        "NoUpdates"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'errorTypeChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentDeviceChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'backendStateChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'firmwareUpdateStateChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isQueryInProgressChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'portReleasedChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onCurrentDeviceChanged'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeviceInfoChanged'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeviceOperationFinished'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeviceRegistryErrorOccured'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFileManagerErrorOccured'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScreenStreamerStateChanged'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'mainAction'
        QtMocHelpers::MethodData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'createBackup'
        QtMocHelpers::MethodData<void(const QUrl &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Method 'restoreBackup'
        QtMocHelpers::MethodData<void(const QUrl &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Method 'factoryReset'
        QtMocHelpers::MethodData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'installFirmware'
        QtMocHelpers::MethodData<void(const QUrl &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 21 },
        }}),
        // Method 'installWirelessStack'
        QtMocHelpers::MethodData<void(const QUrl &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 21 },
        }}),
        // Method 'installFUS'
        QtMocHelpers::MethodData<void(const QUrl &, uint32_t)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 21 }, { 0x80000000 | 24, 25 },
        }}),
        // Method 'startFullScreenStreaming'
        QtMocHelpers::MethodData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stopFullScreenStreaming'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshStorageInfo'
        QtMocHelpers::MethodData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'checkFirmwareUpdates'
        QtMocHelpers::MethodData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'finalizeOperation'
        QtMocHelpers::MethodData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'reportExternalResult'
        QtMocHelpers::MethodData<void(bool, int)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 32 }, { QMetaType::Int, 33 },
        }}),
        // Method 'releasePort'
        QtMocHelpers::MethodData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'reacquirePort'
        QtMocHelpers::MethodData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'backendState'
        QtMocHelpers::PropertyData<enum BackendState>(36, 0x80000000 | 37, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'deviceState'
        QtMocHelpers::PropertyData<Flipper::Zero::DeviceState*>(38, 0x80000000 | 39, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'screenStreamer'
        QtMocHelpers::PropertyData<Flipper::Zero::ScreenStreamer*>(40, 0x80000000 | 41, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'virtualDisplay'
        QtMocHelpers::PropertyData<Flipper::Zero::VirtualDisplay*>(42, 0x80000000 | 43, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'fileManager'
        QtMocHelpers::PropertyData<Flipper::Zero::FileManager*>(44, 0x80000000 | 45, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'firmwareUpdateState'
        QtMocHelpers::PropertyData<enum FirmwareUpdateState>(46, 0x80000000 | 47, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'firmwareUpdateModel'
        QtMocHelpers::PropertyData<QAbstractListModel*>(48, 0x80000000 | 49, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'latestFirmwareVersion'
        QtMocHelpers::PropertyData<Flipper::Updates::VersionInfo>(50, 0x80000000 | 51, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'errorType'
        QtMocHelpers::PropertyData<BackendError::ErrorType>(33, 0x80000000 | 52, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'isQueryInProgress'
        QtMocHelpers::PropertyData<bool>(53, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
        // property 'portReleased'
        QtMocHelpers::PropertyData<bool>(54, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'BackendState'
        QtMocHelpers::EnumData<enum BackendState>(37, 37, QMC::EnumIsScoped).add({
            {   55, BackendState::WaitingForDevices },
            {   56, BackendState::Ready },
            {   57, BackendState::ScreenStreaming },
            {   58, BackendState::UpdatingDevice },
            {   59, BackendState::RepairingDevice },
            {   60, BackendState::CreatingBackup },
            {   61, BackendState::RestoringBackup },
            {   62, BackendState::FactoryResetting },
            {   63, BackendState::InstallingFirmware },
            {   64, BackendState::InstallingWirelessStack },
            {   65, BackendState::InstallingFUS },
            {   66, BackendState::Finished },
            {   67, BackendState::ErrorOccured },
        }),
        // enum 'FirmwareUpdateState'
        QtMocHelpers::EnumData<enum FirmwareUpdateState>(47, 47, QMC::EnumIsScoped).add({
            {   68, FirmwareUpdateState::Unknown },
            {   69, FirmwareUpdateState::Checking },
            {   70, FirmwareUpdateState::CanUpdate },
            {   71, FirmwareUpdateState::CanInstall },
            {   72, FirmwareUpdateState::CanRepair },
            {   73, FirmwareUpdateState::NoUpdates },
            {   67, FirmwareUpdateState::ErrorOccured },
        }),
    };
    return QtMocHelpers::metaObjectData<ApplicationBackend, qt_meta_tag_ZN18ApplicationBackendE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT static const QMetaObject::SuperData qt_meta_extradata_ZN18ApplicationBackendE[] = {
    QMetaObject::SuperData::link<BackendError::staticMetaObject>(),
    nullptr
};

Q_CONSTINIT const QMetaObject ApplicationBackend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ApplicationBackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ApplicationBackendE_t>.data,
    qt_static_metacall,
    qt_meta_extradata_ZN18ApplicationBackendE,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18ApplicationBackendE_t>.metaTypes,
    nullptr
} };

void ApplicationBackend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ApplicationBackend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->errorTypeChanged(); break;
        case 1: _t->currentDeviceChanged(); break;
        case 2: _t->backendStateChanged(); break;
        case 3: _t->firmwareUpdateStateChanged(); break;
        case 4: _t->isQueryInProgressChanged(); break;
        case 5: _t->portReleasedChanged(); break;
        case 6: _t->onCurrentDeviceChanged(); break;
        case 7: _t->onDeviceInfoChanged(); break;
        case 8: _t->onDeviceOperationFinished(); break;
        case 9: _t->onDeviceRegistryErrorOccured(); break;
        case 10: _t->onFileManagerErrorOccured(); break;
        case 11: _t->onScreenStreamerStateChanged(); break;
        case 12: _t->mainAction(); break;
        case 13: _t->createBackup((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 14: _t->restoreBackup((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 15: _t->factoryReset(); break;
        case 16: _t->installFirmware((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 17: _t->installWirelessStack((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 18: _t->installFUS((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<uint32_t>>(_a[2]))); break;
        case 19: _t->startFullScreenStreaming(); break;
        case 20: _t->stopFullScreenStreaming(); break;
        case 21: _t->refreshStorageInfo(); break;
        case 22: _t->checkFirmwareUpdates(); break;
        case 23: _t->finalizeOperation(); break;
        case 24: _t->reportExternalResult((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 25: _t->releasePort(); break;
        case 26: _t->reacquirePort(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ApplicationBackend::*)()>(_a, &ApplicationBackend::errorTypeChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApplicationBackend::*)()>(_a, &ApplicationBackend::currentDeviceChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApplicationBackend::*)()>(_a, &ApplicationBackend::backendStateChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApplicationBackend::*)()>(_a, &ApplicationBackend::firmwareUpdateStateChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApplicationBackend::*)()>(_a, &ApplicationBackend::isQueryInProgressChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApplicationBackend::*)()>(_a, &ApplicationBackend::portReleasedChanged, 5))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 7:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Flipper::Updates::VersionInfo >(); break;
        case 6:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractListModel* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<enum BackendState*>(_v) = _t->backendState(); break;
        case 1: *reinterpret_cast<Flipper::Zero::DeviceState**>(_v) = _t->deviceState(); break;
        case 2: *reinterpret_cast<Flipper::Zero::ScreenStreamer**>(_v) = _t->screenStreamer(); break;
        case 3: *reinterpret_cast<Flipper::Zero::VirtualDisplay**>(_v) = _t->virtualDisplay(); break;
        case 4: *reinterpret_cast<Flipper::Zero::FileManager**>(_v) = _t->fileManager(); break;
        case 5: *reinterpret_cast<enum FirmwareUpdateState*>(_v) = _t->firmwareUpdateState(); break;
        case 6: *reinterpret_cast<QAbstractListModel**>(_v) = _t->firmwareUpdateModel(); break;
        case 7: *reinterpret_cast<Flipper::Updates::VersionInfo*>(_v) = _t->latestFirmwareVersion(); break;
        case 8: *reinterpret_cast<BackendError::ErrorType*>(_v) = _t->errorType(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->isQueryInProgress(); break;
        case 10: *reinterpret_cast<bool*>(_v) = _t->portReleased(); break;
        default: break;
        }
    }
}

const QMetaObject *ApplicationBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ApplicationBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ApplicationBackendE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ApplicationBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 27;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void ApplicationBackend::errorTypeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ApplicationBackend::currentDeviceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ApplicationBackend::backendStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ApplicationBackend::firmwareUpdateStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ApplicationBackend::isQueryInProgressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ApplicationBackend::portReleasedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
