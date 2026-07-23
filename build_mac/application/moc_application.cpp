/****************************************************************************
** Meta object code from reading C++ file 'application.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../application/application.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/screenstreamer.h"
#include "flipperzero/virtualdisplay.h"
#include "flipperzero/filemanager.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'application.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11ApplicationE_t {};
} // unnamed namespace

template <> constexpr inline auto Application::qt_create_metaobjectdata<qt_meta_tag_ZN11ApplicationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Application",
        "updateStatusChanged",
        "",
        "onMessageReceived",
        "onLatestVersionChanged",
        "onCurrentDeviceChanged",
        "selfUpdate",
        "checkForUpdates",
        "applicationNameChanged",
        "applicationVersionChanged",
        "name",
        "version",
        "commit",
        "updater",
        "ApplicationUpdater*",
        "isDeveloperMode",
        "updateStatus",
        "UpdateStatus",
        "NoUpdates",
        "Checking",
        "CanUpdate"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'updateStatusChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onMessageReceived'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLatestVersionChanged'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCurrentDeviceChanged'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'selfUpdate'
        QtMocHelpers::MethodData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'checkForUpdates'
        QtMocHelpers::MethodData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'name'
        QtMocHelpers::PropertyData<QString>(10, QMetaType::QString, QMC::DefaultPropertyFlags, 0x70000000 | 8),
        // property 'version'
        QtMocHelpers::PropertyData<QString>(11, QMetaType::QString, QMC::DefaultPropertyFlags, 0x70000000 | 9),
        // property 'commit'
        QtMocHelpers::PropertyData<QString>(12, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'updater'
        QtMocHelpers::PropertyData<ApplicationUpdater*>(13, 0x80000000 | 14, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'isDeveloperMode'
        QtMocHelpers::PropertyData<bool>(15, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'updateStatus'
        QtMocHelpers::PropertyData<enum UpdateStatus>(16, 0x80000000 | 17, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'UpdateStatus'
        QtMocHelpers::EnumData<enum UpdateStatus>(17, 17, QMC::EnumIsScoped).add({
            {   18, UpdateStatus::NoUpdates },
            {   19, UpdateStatus::Checking },
            {   20, UpdateStatus::CanUpdate },
        }),
    };
    return QtMocHelpers::metaObjectData<Application, qt_meta_tag_ZN11ApplicationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Application::staticMetaObject = { {
    QMetaObject::SuperData::link<QtSingleApplication::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ApplicationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ApplicationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11ApplicationE_t>.metaTypes,
    nullptr
} };

void Application::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Application *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->updateStatusChanged(); break;
        case 1: _t->onMessageReceived(); break;
        case 2: _t->onLatestVersionChanged(); break;
        case 3: _t->onCurrentDeviceChanged(); break;
        case 4: _t->selfUpdate(); break;
        case 5: _t->checkForUpdates(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Application::*)()>(_a, &Application::updateStatusChanged, 0))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ApplicationUpdater* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->applicationName(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->applicationVersion(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->commitNumber(); break;
        case 3: *reinterpret_cast<ApplicationUpdater**>(_v) = _t->updater(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->isDeveloperMode(); break;
        case 5: *reinterpret_cast<enum UpdateStatus*>(_v) = _t->updateStatus(); break;
        default: break;
        }
    }
}

const QMetaObject *Application::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Application::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ApplicationE_t>.strings))
        return static_cast<void*>(this);
    return QtSingleApplication::qt_metacast(_clname);
}

int Application::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtSingleApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Application::updateStatusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace CheckNotifySignalValidity_ZN11ApplicationE {
template<typename T> using has_nullary_applicationNameChanged = decltype(std::declval<T>().applicationNameChanged());
template<typename T> using has_unary_applicationNameChanged = decltype(std::declval<T>().applicationNameChanged(std::declval<QString>()));
static_assert(qxp::is_detected_v<has_nullary_applicationNameChanged, Application> || qxp::is_detected_v<has_unary_applicationNameChanged, Application>,
              "NOTIFY signal applicationNameChanged does not exist in class (or is private in its parent)");
template<typename T> using has_nullary_applicationVersionChanged = decltype(std::declval<T>().applicationVersionChanged());
template<typename T> using has_unary_applicationVersionChanged = decltype(std::declval<T>().applicationVersionChanged(std::declval<QString>()));
static_assert(qxp::is_detected_v<has_nullary_applicationVersionChanged, Application> || qxp::is_detected_v<has_unary_applicationVersionChanged, Application>,
              "NOTIFY signal applicationVersionChanged does not exist in class (or is private in its parent)");
}
QT_WARNING_POP
