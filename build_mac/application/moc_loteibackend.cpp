/****************************************************************************
** Meta object code from reading C++ file 'loteibackend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../application/loteibackend.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'loteibackend.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12LoteiBackendE_t {};
} // unnamed namespace

template <> constexpr inline auto LoteiBackend::qt_create_metaobjectdata<qt_meta_tag_ZN12LoteiBackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LoteiBackend",
        "replyReceived",
        "",
        "text",
        "errorOccurred",
        "thinkingChanged",
        "mutedChanged",
        "voiceChanged",
        "modelChanged",
        "voiceVolumeChanged",
        "musicVolumeChanged",
        "setupCompleteChanged",
        "manualNameChanged",
        "agentChanged",
        "scriptSaved",
        "path",
        "scriptSaveError",
        "message",
        "partialReceived",
        "extractScript",
        "saveScriptToFlipper",
        "folder",
        "filename",
        "content",
        "send",
        "userText",
        "deviceContext",
        "reset",
        "cycleVoice",
        "cycleModel",
        "setModel",
        "model",
        "availableModels",
        "musicFolderUrl",
        "completeSetup",
        "resetSetup",
        "recheckOllama",
        "personalityPresets",
        "applyPreset",
        "name",
        "applyNamePersonality",
        "thinking",
        "configured",
        "hasAudio",
        "muted",
        "voiceName",
        "modelName",
        "voiceVolume",
        "musicVolume",
        "setupComplete",
        "ollamaOnline",
        "manualName",
        "agentEnabled",
        "agentDir"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'replyReceived'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'thinkingChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mutedChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voiceChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modelChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voiceVolumeChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'musicVolumeChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'setupCompleteChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'manualNameChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'agentChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'scriptSaved'
        QtMocHelpers::SignalData<void(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 },
        }}),
        // Signal 'scriptSaveError'
        QtMocHelpers::SignalData<void(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Signal 'partialReceived'
        QtMocHelpers::SignalData<void(const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Method 'extractScript'
        QtMocHelpers::MethodData<QString(const QString &) const>(19, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 3 },
        }}),
        // Method 'saveScriptToFlipper'
        QtMocHelpers::MethodData<void(const QString &, const QString &, const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 }, { QMetaType::QString, 22 }, { QMetaType::QString, 23 },
        }}),
        // Method 'send'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 25 }, { QMetaType::QString, 26 },
        }}),
        // Method 'reset'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cycleVoice'
        QtMocHelpers::MethodData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cycleModel'
        QtMocHelpers::MethodData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'setModel'
        QtMocHelpers::MethodData<void(const QString &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
        // Method 'availableModels'
        QtMocHelpers::MethodData<QStringList() const>(32, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Method 'musicFolderUrl'
        QtMocHelpers::MethodData<QString() const>(33, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'completeSetup'
        QtMocHelpers::MethodData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetSetup'
        QtMocHelpers::MethodData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'recheckOllama'
        QtMocHelpers::MethodData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'personalityPresets'
        QtMocHelpers::MethodData<QStringList() const>(37, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Method 'applyPreset'
        QtMocHelpers::MethodData<void(const QString &)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 39 },
        }}),
        // Method 'applyNamePersonality'
        QtMocHelpers::MethodData<void()>(40, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'thinking'
        QtMocHelpers::PropertyData<bool>(41, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'configured'
        QtMocHelpers::PropertyData<bool>(42, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'hasAudio'
        QtMocHelpers::PropertyData<bool>(43, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'muted'
        QtMocHelpers::PropertyData<bool>(44, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'voiceName'
        QtMocHelpers::PropertyData<QString>(45, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
        // property 'modelName'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'voiceVolume'
        QtMocHelpers::PropertyData<qreal>(47, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 6),
        // property 'musicVolume'
        QtMocHelpers::PropertyData<qreal>(48, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'setupComplete'
        QtMocHelpers::PropertyData<bool>(49, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
        // property 'ollamaOnline'
        QtMocHelpers::PropertyData<bool>(50, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'manualName'
        QtMocHelpers::PropertyData<QString>(51, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'agentEnabled'
        QtMocHelpers::PropertyData<bool>(52, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
        // property 'agentDir'
        QtMocHelpers::PropertyData<QString>(53, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LoteiBackend, qt_meta_tag_ZN12LoteiBackendE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LoteiBackend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LoteiBackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LoteiBackendE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12LoteiBackendE_t>.metaTypes,
    nullptr
} };

void LoteiBackend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LoteiBackend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->replyReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->thinkingChanged(); break;
        case 3: _t->mutedChanged(); break;
        case 4: _t->voiceChanged(); break;
        case 5: _t->modelChanged(); break;
        case 6: _t->voiceVolumeChanged(); break;
        case 7: _t->musicVolumeChanged(); break;
        case 8: _t->setupCompleteChanged(); break;
        case 9: _t->manualNameChanged(); break;
        case 10: _t->agentChanged(); break;
        case 11: _t->scriptSaved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->scriptSaveError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->partialReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: { QString _r = _t->extractScript((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->saveScriptToFlipper((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 16: _t->send((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 17: _t->reset(); break;
        case 18: _t->cycleVoice(); break;
        case 19: _t->cycleModel(); break;
        case 20: _t->setModel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: { QStringList _r = _t->availableModels();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 22: { QString _r = _t->musicFolderUrl();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 23: _t->completeSetup(); break;
        case 24: _t->resetSetup(); break;
        case 25: _t->recheckOllama(); break;
        case 26: { QStringList _r = _t->personalityPresets();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 27: _t->applyPreset((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 28: _t->applyNamePersonality(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::replyReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::errorOccurred, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::thinkingChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::mutedChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::voiceChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::modelChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::voiceVolumeChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::musicVolumeChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::setupCompleteChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::manualNameChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::agentChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::scriptSaved, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::scriptSaveError, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::partialReceived, 13))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->thinking(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->configured(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->hasAudio(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->muted(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->voiceName(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->modelName(); break;
        case 6: *reinterpret_cast<qreal*>(_v) = _t->voiceVolume(); break;
        case 7: *reinterpret_cast<qreal*>(_v) = _t->musicVolume(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->setupComplete(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->ollamaOnline(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->manualName(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->agentEnabled(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->agentDir(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setMuted(*reinterpret_cast<bool*>(_v)); break;
        case 6: _t->setVoiceVolume(*reinterpret_cast<qreal*>(_v)); break;
        case 7: _t->setMusicVolume(*reinterpret_cast<qreal*>(_v)); break;
        case 10: _t->setManualName(*reinterpret_cast<QString*>(_v)); break;
        case 11: _t->setAgentEnabled(*reinterpret_cast<bool*>(_v)); break;
        case 12: _t->setAgentDir(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *LoteiBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LoteiBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LoteiBackendE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LoteiBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 29)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 29;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 29)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 29;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void LoteiBackend::replyReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void LoteiBackend::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void LoteiBackend::thinkingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LoteiBackend::mutedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void LoteiBackend::voiceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void LoteiBackend::modelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void LoteiBackend::voiceVolumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void LoteiBackend::musicVolumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void LoteiBackend::setupCompleteChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void LoteiBackend::manualNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void LoteiBackend::agentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void LoteiBackend::scriptSaved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void LoteiBackend::scriptSaveError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void LoteiBackend::partialReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN12LoteiPaletteE_t {};
} // unnamed namespace

template <> constexpr inline auto LoteiPalette::qt_create_metaobjectdata<qt_meta_tag_ZN12LoteiPaletteE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LoteiPalette",
        "changed",
        "",
        "names",
        "hex",
        "name",
        "setColor",
        "QColor",
        "c",
        "reset",
        "colors",
        "QVariantMap"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'names'
        QtMocHelpers::MethodData<QStringList() const>(3, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Method 'hex'
        QtMocHelpers::MethodData<QString(const QString &) const>(4, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 5 },
        }}),
        // Method 'setColor'
        QtMocHelpers::MethodData<void(const QString &, const QColor &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { 0x80000000 | 7, 8 },
        }}),
        // Method 'reset'
        QtMocHelpers::MethodData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'colors'
        QtMocHelpers::PropertyData<QVariantMap>(10, 0x80000000 | 11, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LoteiPalette, qt_meta_tag_ZN12LoteiPaletteE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LoteiPalette::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LoteiPaletteE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LoteiPaletteE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12LoteiPaletteE_t>.metaTypes,
    nullptr
} };

void LoteiPalette::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LoteiPalette *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->changed(); break;
        case 1: { QStringList _r = _t->names();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 2: { QString _r = _t->hex((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->setColor((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QColor>>(_a[2]))); break;
        case 4: _t->reset(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LoteiPalette::*)()>(_a, &LoteiPalette::changed, 0))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantMap*>(_v) = _t->colors(); break;
        default: break;
        }
    }
}

const QMetaObject *LoteiPalette::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LoteiPalette::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LoteiPaletteE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LoteiPalette::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void LoteiPalette::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace {
struct qt_meta_tag_ZN13FirmwareStoreE_t {};
} // unnamed namespace

template <> constexpr inline auto FirmwareStore::qt_create_metaobjectdata<qt_meta_tag_ZN13FirmwareStoreE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "FirmwareStore",
        "openChanged",
        "",
        "changed",
        "busyChanged",
        "readyToInstall",
        "fileUrl",
        "progress",
        "index",
        "frac",
        "note",
        "failed",
        "message",
        "refresh",
        "install",
        "cycleChannel",
        "open",
        "sources",
        "QVariantList",
        "busy"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'openChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'busyChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'readyToInstall'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'progress'
        QtMocHelpers::SignalData<void(int, qreal, const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::QReal, 9 }, { QMetaType::QString, 10 },
        }}),
        // Signal 'failed'
        QtMocHelpers::SignalData<void(int, const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::QString, 12 },
        }}),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'install'
        QtMocHelpers::MethodData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'cycleChannel'
        QtMocHelpers::MethodData<void(int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'open'
        QtMocHelpers::PropertyData<bool>(16, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'sources'
        QtMocHelpers::PropertyData<QVariantList>(17, 0x80000000 | 18, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(19, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FirmwareStore, qt_meta_tag_ZN13FirmwareStoreE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject FirmwareStore::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13FirmwareStoreE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13FirmwareStoreE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13FirmwareStoreE_t>.metaTypes,
    nullptr
} };

void FirmwareStore::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FirmwareStore *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->openChanged(); break;
        case 1: _t->changed(); break;
        case 2: _t->busyChanged(); break;
        case 3: _t->readyToInstall((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->progress((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 5: _t->failed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->refresh(); break;
        case 7: _t->install((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->cycleChannel((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FirmwareStore::*)()>(_a, &FirmwareStore::openChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FirmwareStore::*)()>(_a, &FirmwareStore::changed, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FirmwareStore::*)()>(_a, &FirmwareStore::busyChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (FirmwareStore::*)(const QString & )>(_a, &FirmwareStore::readyToInstall, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (FirmwareStore::*)(int , qreal , const QString & )>(_a, &FirmwareStore::progress, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (FirmwareStore::*)(int , const QString & )>(_a, &FirmwareStore::failed, 5))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->isOpen(); break;
        case 1: *reinterpret_cast<QVariantList*>(_v) = _t->sources(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setOpen(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *FirmwareStore::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FirmwareStore::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13FirmwareStoreE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FirmwareStore::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
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
void FirmwareStore::openChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void FirmwareStore::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void FirmwareStore::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void FirmwareStore::readyToInstall(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void FirmwareStore::progress(int _t1, qreal _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3);
}

// SIGNAL 5
void FirmwareStore::failed(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}
namespace {
struct qt_meta_tag_ZN10FlipperCliE_t {};
} // unnamed namespace

template <> constexpr inline auto FlipperCli::qt_create_metaobjectdata<qt_meta_tag_ZN10FlipperCliE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "FlipperCli",
        "openChanged",
        "",
        "activeChanged",
        "outputChanged",
        "statusChanged",
        "onReadyRead",
        "send",
        "cmd",
        "interrupt",
        "clearOutput",
        "open",
        "active",
        "output",
        "status"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'openChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activeChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'outputChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'send'
        QtMocHelpers::MethodData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Method 'interrupt'
        QtMocHelpers::MethodData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'clearOutput'
        QtMocHelpers::MethodData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'open'
        QtMocHelpers::PropertyData<bool>(11, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'active'
        QtMocHelpers::PropertyData<bool>(12, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'output'
        QtMocHelpers::PropertyData<QString>(13, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(14, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FlipperCli, qt_meta_tag_ZN10FlipperCliE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject FlipperCli::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FlipperCliE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FlipperCliE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10FlipperCliE_t>.metaTypes,
    nullptr
} };

void FlipperCli::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FlipperCli *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->openChanged(); break;
        case 1: _t->activeChanged(); break;
        case 2: _t->outputChanged(); break;
        case 3: _t->statusChanged(); break;
        case 4: _t->onReadyRead(); break;
        case 5: _t->send((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->interrupt(); break;
        case 7: _t->clearOutput(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::openChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::activeChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::outputChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::statusChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->isOpen(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->active(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->output(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setOpen(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *FlipperCli::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FlipperCli::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FlipperCliE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FlipperCli::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void FlipperCli::openChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void FlipperCli::activeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void FlipperCli::outputChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void FlipperCli::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
