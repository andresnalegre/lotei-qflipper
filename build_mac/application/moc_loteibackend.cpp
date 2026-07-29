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
        "backupProgress",
        "",
        "note",
        "frac",
        "backupFinished",
        "path",
        "items",
        "backupFailed",
        "error",
        "restoreProgress",
        "restoreFinished",
        "restoreFailed",
        "formatProgress",
        "formatFinished",
        "formatFailed",
        "sdFormattedChanged",
        "transferChanged",
        "replyReceived",
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
        "scriptSaveError",
        "message",
        "fileOpened",
        "content",
        "fileSaved",
        "fileEditError",
        "partialReceived",
        "extractScript",
        "saveScriptToFlipper",
        "folder",
        "filename",
        "openFileForEdit",
        "clearHistory",
        "writeFile",
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
        "logAction",
        "what",
        "reloadMemory",
        "syncMemoryToFlipper",
        "backupSdCard",
        "restoreSdCard",
        "folderUrl",
        "formatSdCard",
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
        "agentDir",
        "transferActive",
        "transferTitle",
        "transferNote",
        "transferProgress",
        "sdFormatted"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backupProgress'
        QtMocHelpers::SignalData<void(const QString &, double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Double, 4 },
        }}),
        // Signal 'backupFinished'
        QtMocHelpers::SignalData<void(const QString &, int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'backupFailed'
        QtMocHelpers::SignalData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Signal 'restoreProgress'
        QtMocHelpers::SignalData<void(const QString &, double)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Double, 4 },
        }}),
        // Signal 'restoreFinished'
        QtMocHelpers::SignalData<void(int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Signal 'restoreFailed'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Signal 'formatProgress'
        QtMocHelpers::SignalData<void(const QString &, double)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Double, 4 },
        }}),
        // Signal 'formatFinished'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'formatFailed'
        QtMocHelpers::SignalData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Signal 'sdFormattedChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'transferChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'replyReceived'
        QtMocHelpers::SignalData<void(const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Signal 'thinkingChanged'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mutedChanged'
        QtMocHelpers::SignalData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voiceChanged'
        QtMocHelpers::SignalData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modelChanged'
        QtMocHelpers::SignalData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voiceVolumeChanged'
        QtMocHelpers::SignalData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'musicVolumeChanged'
        QtMocHelpers::SignalData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'setupCompleteChanged'
        QtMocHelpers::SignalData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'manualNameChanged'
        QtMocHelpers::SignalData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'agentChanged'
        QtMocHelpers::SignalData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'scriptSaved'
        QtMocHelpers::SignalData<void(const QString &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'scriptSaveError'
        QtMocHelpers::SignalData<void(const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Signal 'fileOpened'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 34 },
        }}),
        // Signal 'fileSaved'
        QtMocHelpers::SignalData<void(const QString &)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'fileEditError'
        QtMocHelpers::SignalData<void(const QString &)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Signal 'partialReceived'
        QtMocHelpers::SignalData<void(const QString &)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Method 'extractScript'
        QtMocHelpers::MethodData<QString(const QString &) const>(38, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 19 },
        }}),
        // Method 'saveScriptToFlipper'
        QtMocHelpers::MethodData<void(const QString &, const QString &, const QString &)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 40 }, { QMetaType::QString, 41 }, { QMetaType::QString, 34 },
        }}),
        // Method 'openFileForEdit'
        QtMocHelpers::MethodData<void(const QString &)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Method 'clearHistory'
        QtMocHelpers::MethodData<void()>(43, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'writeFile'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 34 },
        }}),
        // Method 'send'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 46 }, { QMetaType::QString, 47 },
        }}),
        // Method 'reset'
        QtMocHelpers::MethodData<void()>(48, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cycleVoice'
        QtMocHelpers::MethodData<void()>(49, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cycleModel'
        QtMocHelpers::MethodData<void()>(50, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'setModel'
        QtMocHelpers::MethodData<void(const QString &)>(51, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 52 },
        }}),
        // Method 'availableModels'
        QtMocHelpers::MethodData<QStringList() const>(53, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Method 'musicFolderUrl'
        QtMocHelpers::MethodData<QString() const>(54, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'completeSetup'
        QtMocHelpers::MethodData<void()>(55, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetSetup'
        QtMocHelpers::MethodData<void()>(56, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'recheckOllama'
        QtMocHelpers::MethodData<void()>(57, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'personalityPresets'
        QtMocHelpers::MethodData<QStringList() const>(58, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Method 'applyPreset'
        QtMocHelpers::MethodData<void(const QString &)>(59, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 60 },
        }}),
        // Method 'applyNamePersonality'
        QtMocHelpers::MethodData<void()>(61, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'logAction'
        QtMocHelpers::MethodData<void(const QString &) const>(62, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 63 },
        }}),
        // Method 'reloadMemory'
        QtMocHelpers::MethodData<void()>(64, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'syncMemoryToFlipper'
        QtMocHelpers::MethodData<void()>(65, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'backupSdCard'
        QtMocHelpers::MethodData<void()>(66, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'restoreSdCard'
        QtMocHelpers::MethodData<void(const QString &)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 68 },
        }}),
        // Method 'formatSdCard'
        QtMocHelpers::MethodData<void()>(69, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'thinking'
        QtMocHelpers::PropertyData<bool>(70, QMetaType::Bool, QMC::DefaultPropertyFlags, 13),
        // property 'configured'
        QtMocHelpers::PropertyData<bool>(71, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'hasAudio'
        QtMocHelpers::PropertyData<bool>(72, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'muted'
        QtMocHelpers::PropertyData<bool>(73, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 14),
        // property 'voiceName'
        QtMocHelpers::PropertyData<QString>(74, QMetaType::QString, QMC::DefaultPropertyFlags, 15),
        // property 'modelName'
        QtMocHelpers::PropertyData<QString>(75, QMetaType::QString, QMC::DefaultPropertyFlags, 16),
        // property 'voiceVolume'
        QtMocHelpers::PropertyData<qreal>(76, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 17),
        // property 'musicVolume'
        QtMocHelpers::PropertyData<qreal>(77, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 18),
        // property 'setupComplete'
        QtMocHelpers::PropertyData<bool>(78, QMetaType::Bool, QMC::DefaultPropertyFlags, 19),
        // property 'ollamaOnline'
        QtMocHelpers::PropertyData<bool>(79, QMetaType::Bool, QMC::DefaultPropertyFlags, 16),
        // property 'manualName'
        QtMocHelpers::PropertyData<QString>(80, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 20),
        // property 'agentEnabled'
        QtMocHelpers::PropertyData<bool>(81, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 21),
        // property 'agentDir'
        QtMocHelpers::PropertyData<QString>(82, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 21),
        // property 'transferActive'
        QtMocHelpers::PropertyData<bool>(83, QMetaType::Bool, QMC::DefaultPropertyFlags, 10),
        // property 'transferTitle'
        QtMocHelpers::PropertyData<QString>(84, QMetaType::QString, QMC::DefaultPropertyFlags, 10),
        // property 'transferNote'
        QtMocHelpers::PropertyData<QString>(85, QMetaType::QString, QMC::DefaultPropertyFlags, 10),
        // property 'transferProgress'
        QtMocHelpers::PropertyData<double>(86, QMetaType::Double, QMC::DefaultPropertyFlags, 10),
        // property 'sdFormatted'
        QtMocHelpers::PropertyData<bool>(87, QMetaType::Bool, QMC::DefaultPropertyFlags, 9),
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
        case 0: _t->backupProgress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 1: _t->backupFinished((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->backupFailed((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->restoreProgress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 4: _t->restoreFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->restoreFailed((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->formatProgress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 7: _t->formatFinished(); break;
        case 8: _t->formatFailed((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->sdFormattedChanged(); break;
        case 10: _t->transferChanged(); break;
        case 11: _t->replyReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->thinkingChanged(); break;
        case 14: _t->mutedChanged(); break;
        case 15: _t->voiceChanged(); break;
        case 16: _t->modelChanged(); break;
        case 17: _t->voiceVolumeChanged(); break;
        case 18: _t->musicVolumeChanged(); break;
        case 19: _t->setupCompleteChanged(); break;
        case 20: _t->manualNameChanged(); break;
        case 21: _t->agentChanged(); break;
        case 22: _t->scriptSaved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 23: _t->scriptSaveError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 24: _t->fileOpened((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 25: _t->fileSaved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 26: _t->fileEditError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 27: _t->partialReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 28: { QString _r = _t->extractScript((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 29: _t->saveScriptToFlipper((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 30: _t->openFileForEdit((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 31: _t->clearHistory(); break;
        case 32: _t->writeFile((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 33: _t->send((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 34: _t->reset(); break;
        case 35: _t->cycleVoice(); break;
        case 36: _t->cycleModel(); break;
        case 37: _t->setModel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 38: { QStringList _r = _t->availableModels();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 39: { QString _r = _t->musicFolderUrl();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 40: _t->completeSetup(); break;
        case 41: _t->resetSetup(); break;
        case 42: _t->recheckOllama(); break;
        case 43: { QStringList _r = _t->personalityPresets();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 44: _t->applyPreset((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 45: _t->applyNamePersonality(); break;
        case 46: _t->logAction((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 47: _t->reloadMemory(); break;
        case 48: _t->syncMemoryToFlipper(); break;
        case 49: _t->backupSdCard(); break;
        case 50: _t->restoreSdCard((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 51: _t->formatSdCard(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & , double )>(_a, &LoteiBackend::backupProgress, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & , int )>(_a, &LoteiBackend::backupFinished, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::backupFailed, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & , double )>(_a, &LoteiBackend::restoreProgress, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(int )>(_a, &LoteiBackend::restoreFinished, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::restoreFailed, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & , double )>(_a, &LoteiBackend::formatProgress, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::formatFinished, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::formatFailed, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::sdFormattedChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::transferChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::replyReceived, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::errorOccurred, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::thinkingChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::mutedChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::voiceChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::modelChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::voiceVolumeChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::musicVolumeChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::setupCompleteChanged, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::manualNameChanged, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)()>(_a, &LoteiBackend::agentChanged, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::scriptSaved, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::scriptSaveError, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & , const QString & )>(_a, &LoteiBackend::fileOpened, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::fileSaved, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::fileEditError, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (LoteiBackend::*)(const QString & )>(_a, &LoteiBackend::partialReceived, 27))
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
        case 13: *reinterpret_cast<bool*>(_v) = _t->transferActive(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->transferTitle(); break;
        case 15: *reinterpret_cast<QString*>(_v) = _t->transferNote(); break;
        case 16: *reinterpret_cast<double*>(_v) = _t->transferProgress(); break;
        case 17: *reinterpret_cast<bool*>(_v) = _t->sdFormatted(); break;
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
        if (_id < 52)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 52;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 52)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 52;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void LoteiBackend::backupProgress(const QString & _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void LoteiBackend::backupFinished(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void LoteiBackend::backupFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void LoteiBackend::restoreProgress(const QString & _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void LoteiBackend::restoreFinished(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void LoteiBackend::restoreFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void LoteiBackend::formatProgress(const QString & _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void LoteiBackend::formatFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void LoteiBackend::formatFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void LoteiBackend::sdFormattedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void LoteiBackend::transferChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void LoteiBackend::replyReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void LoteiBackend::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void LoteiBackend::thinkingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void LoteiBackend::mutedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void LoteiBackend::voiceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void LoteiBackend::modelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void LoteiBackend::voiceVolumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}

// SIGNAL 18
void LoteiBackend::musicVolumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void LoteiBackend::setupCompleteChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void LoteiBackend::manualNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 20, nullptr);
}

// SIGNAL 21
void LoteiBackend::agentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 21, nullptr);
}

// SIGNAL 22
void LoteiBackend::scriptSaved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1);
}

// SIGNAL 23
void LoteiBackend::scriptSaveError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 23, nullptr, _t1);
}

// SIGNAL 24
void LoteiBackend::fileOpened(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1, _t2);
}

// SIGNAL 25
void LoteiBackend::fileSaved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1);
}

// SIGNAL 26
void LoteiBackend::fileEditError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 26, nullptr, _t1);
}

// SIGNAL 27
void LoteiBackend::partialReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 27, nullptr, _t1);
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
        "setInstalledChannel",
        "id",
        "reinstallInstalled",
        "select",
        "clearSelection",
        "installSelected",
        "refresh",
        "install",
        "cycleChannel",
        "open",
        "sources",
        "QVariantList",
        "busy",
        "deviceVersion",
        "installedIndex",
        "installedName",
        "installedLatest",
        "installedReady",
        "updateAvailable",
        "installedDate",
        "selectedIndex",
        "hasSelection",
        "selectedName",
        "selectedVersion",
        "selectedDate",
        "checking",
        "checkSummary",
        "installedChannels",
        "installedChannelModel",
        "installedChannel",
        "installedFromChannel",
        "channelSwitchPending"
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
        // Method 'setInstalledChannel'
        QtMocHelpers::MethodData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Method 'reinstallInstalled'
        QtMocHelpers::MethodData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'select'
        QtMocHelpers::MethodData<void(int)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'clearSelection'
        QtMocHelpers::MethodData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'installSelected'
        QtMocHelpers::MethodData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'install'
        QtMocHelpers::MethodData<void(int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'cycleChannel'
        QtMocHelpers::MethodData<void(int)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'open'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'sources'
        QtMocHelpers::PropertyData<QVariantList>(23, 0x80000000 | 24, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(25, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'deviceVersion'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'installedIndex'
        QtMocHelpers::PropertyData<int>(27, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'installedName'
        QtMocHelpers::PropertyData<QString>(28, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'installedLatest'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'installedReady'
        QtMocHelpers::PropertyData<bool>(30, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'updateAvailable'
        QtMocHelpers::PropertyData<bool>(31, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'installedDate'
        QtMocHelpers::PropertyData<QString>(32, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'selectedIndex'
        QtMocHelpers::PropertyData<int>(33, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'hasSelection'
        QtMocHelpers::PropertyData<bool>(34, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'selectedName'
        QtMocHelpers::PropertyData<QString>(35, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'selectedVersion'
        QtMocHelpers::PropertyData<QString>(36, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'selectedDate'
        QtMocHelpers::PropertyData<QString>(37, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'checking'
        QtMocHelpers::PropertyData<bool>(38, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'checkSummary'
        QtMocHelpers::PropertyData<QString>(39, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'installedChannels'
        QtMocHelpers::PropertyData<QStringList>(40, QMetaType::QStringList, QMC::DefaultPropertyFlags, 1),
        // property 'installedChannelModel'
        QtMocHelpers::PropertyData<QVariantList>(41, 0x80000000 | 24, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'installedChannel'
        QtMocHelpers::PropertyData<QString>(42, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'installedFromChannel'
        QtMocHelpers::PropertyData<QString>(43, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'channelSwitchPending'
        QtMocHelpers::PropertyData<bool>(44, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
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
        case 6: _t->setInstalledChannel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->reinstallInstalled(); break;
        case 8: _t->select((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->clearSelection(); break;
        case 10: _t->installSelected(); break;
        case 11: _t->refresh(); break;
        case 12: _t->install((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->cycleChannel((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
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
        case 3: *reinterpret_cast<QString*>(_v) = _t->deviceVersion(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->installedIndex(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->installedName(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->installedLatest(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->installedReady(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->updateAvailable(); break;
        case 9: *reinterpret_cast<QString*>(_v) = _t->installedDate(); break;
        case 10: *reinterpret_cast<int*>(_v) = _t->selectedIndex(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->hasSelection(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->selectedName(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->selectedVersion(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->selectedDate(); break;
        case 15: *reinterpret_cast<bool*>(_v) = _t->checking(); break;
        case 16: *reinterpret_cast<QString*>(_v) = _t->checkSummary(); break;
        case 17: *reinterpret_cast<QStringList*>(_v) = _t->installedChannels(); break;
        case 18: *reinterpret_cast<QVariantList*>(_v) = _t->installedChannelModel(); break;
        case 19: *reinterpret_cast<QString*>(_v) = _t->installedChannel(); break;
        case 20: *reinterpret_cast<QString*>(_v) = _t->installedFromChannel(); break;
        case 21: *reinterpret_cast<bool*>(_v) = _t->channelSwitchPending(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setOpen(*reinterpret_cast<bool*>(_v)); break;
        case 3: _t->setDeviceVersion(*reinterpret_cast<QString*>(_v)); break;
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
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
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
        "verboseChanged",
        "coloredChanged",
        "promptChanged",
        "completion",
        "line",
        "editRequested",
        "path",
        "content",
        "onReadyRead",
        "send",
        "cmd",
        "interrupt",
        "clearOutput",
        "complete",
        "clipboardText",
        "copyToClipboard",
        "text",
        "open",
        "active",
        "output",
        "status",
        "verbose",
        "colored",
        "promptText"
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
        // Signal 'verboseChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'coloredChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'promptChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'completion'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'editRequested'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { QMetaType::QString, 13 },
        }}),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'send'
        QtMocHelpers::MethodData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Method 'interrupt'
        QtMocHelpers::MethodData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'clearOutput'
        QtMocHelpers::MethodData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'complete'
        QtMocHelpers::MethodData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Method 'clipboardText'
        QtMocHelpers::MethodData<QString() const>(20, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'copyToClipboard'
        QtMocHelpers::MethodData<void(const QString &) const>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'open'
        QtMocHelpers::PropertyData<bool>(23, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'active'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'output'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'verbose'
        QtMocHelpers::PropertyData<bool>(27, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'colored'
        QtMocHelpers::PropertyData<bool>(28, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'promptText'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 6),
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
        case 4: _t->verboseChanged(); break;
        case 5: _t->coloredChanged(); break;
        case 6: _t->promptChanged(); break;
        case 7: _t->completion((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->editRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->onReadyRead(); break;
        case 10: _t->send((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->interrupt(); break;
        case 12: _t->clearOutput(); break;
        case 13: _t->complete((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: { QString _r = _t->clipboardText();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->copyToClipboard((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
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
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::verboseChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::coloredChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)()>(_a, &FlipperCli::promptChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)(const QString & )>(_a, &FlipperCli::completion, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (FlipperCli::*)(const QString & , const QString & )>(_a, &FlipperCli::editRequested, 8))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->isOpen(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->active(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->output(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->verbose(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->colored(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->promptText(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setOpen(*reinterpret_cast<bool*>(_v)); break;
        case 4: _t->setVerbose(*reinterpret_cast<bool*>(_v)); break;
        case 5: _t->setColored(*reinterpret_cast<bool*>(_v)); break;
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
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
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

// SIGNAL 4
void FlipperCli::verboseChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void FlipperCli::coloredChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void FlipperCli::promptChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void FlipperCli::completion(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void FlipperCli::editRequested(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}
QT_WARNING_POP
