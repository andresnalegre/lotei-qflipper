/****************************************************************************
** Meta object code from reading C++ file 'filemanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../backend/flipperzero/filemanager.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filemanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto Flipper::Zero::FileManager::qt_create_metaobjectdata<qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Flipper::Zero::FileManager",
        "isBusyChanged",
        "",
        "currentPathChanged",
        "newDirectoryIndexChanged",
        "errorOccured",
        "onBusyTimerTimeout",
        "reset",
        "refresh",
        "cd",
        "dirName",
        "historyForward",
        "historyBack",
        "rename",
        "oldName",
        "newName",
        "remove",
        "fileName",
        "recursive",
        "fileNameAt",
        "row",
        "isDirectoryAt",
        "beginMkDir",
        "commitMkDir",
        "upload",
        "QList<QUrl>",
        "urlList",
        "uploadTo",
        "remoteDirName",
        "download",
        "remoteFileName",
        "QUrl",
        "localUrl",
        "isTooLarge",
        "isBusy",
        "isRoot",
        "canGoBack",
        "canGoForward",
        "currentPath",
        "newDirectoryIndex",
        "FieldRole",
        "FileName",
        "FilePath",
        "FileType",
        "FileSize"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'isBusyChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentPathChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'newDirectoryIndexChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'errorOccured'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onBusyTimerTimeout'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'reset'
        QtMocHelpers::MethodData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cd'
        QtMocHelpers::MethodData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Method 'historyForward'
        QtMocHelpers::MethodData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'historyBack'
        QtMocHelpers::MethodData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'rename'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 }, { QMetaType::QString, 15 },
        }}),
        // Method 'remove'
        QtMocHelpers::MethodData<void(const QString &, bool)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 17 }, { QMetaType::Bool, 18 },
        }}),
        // Method 'remove'
        QtMocHelpers::MethodData<void(const QString &)>(16, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Method 'fileNameAt'
        QtMocHelpers::MethodData<QString(int) const>(19, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::Int, 20 },
        }}),
        // Method 'isDirectoryAt'
        QtMocHelpers::MethodData<bool(int) const>(21, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 20 },
        }}),
        // Method 'beginMkDir'
        QtMocHelpers::MethodData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'commitMkDir'
        QtMocHelpers::MethodData<void(const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Method 'upload'
        QtMocHelpers::MethodData<void(const QList<QUrl> &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 25, 26 },
        }}),
        // Method 'uploadTo'
        QtMocHelpers::MethodData<void(const QString &, const QList<QUrl> &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 28 }, { 0x80000000 | 25, 26 },
        }}),
        // Method 'download'
        QtMocHelpers::MethodData<void(const QString &, const QUrl &, bool)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 30 }, { 0x80000000 | 31, 32 }, { QMetaType::Bool, 18 },
        }}),
        // Method 'download'
        QtMocHelpers::MethodData<void(const QString &, const QUrl &)>(29, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 30 }, { 0x80000000 | 31, 32 },
        }}),
        // Method 'isTooLarge'
        QtMocHelpers::MethodData<bool(const QList<QUrl> &) const>(33, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 25, 26 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'isBusy'
        QtMocHelpers::PropertyData<bool>(34, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'isRoot'
        QtMocHelpers::PropertyData<bool>(35, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'canGoBack'
        QtMocHelpers::PropertyData<bool>(36, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'canGoForward'
        QtMocHelpers::PropertyData<bool>(37, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'currentPath'
        QtMocHelpers::PropertyData<QString>(38, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'newDirectoryIndex'
        QtMocHelpers::PropertyData<int>(39, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'FieldRole'
        QtMocHelpers::EnumData<enum FieldRole>(40, 40, QMC::EnumFlags{}).add({
            {   41, FieldRole::FileName },
            {   42, FieldRole::FilePath },
            {   43, FieldRole::FileType },
            {   44, FieldRole::FileSize },
        }),
    };
    return QtMocHelpers::metaObjectData<FileManager, qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Flipper::Zero::FileManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t>.metaTypes,
    nullptr
} };

void Flipper::Zero::FileManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FileManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->isBusyChanged(); break;
        case 1: _t->currentPathChanged(); break;
        case 2: _t->newDirectoryIndexChanged(); break;
        case 3: _t->errorOccured(); break;
        case 4: _t->onBusyTimerTimeout(); break;
        case 5: _t->reset(); break;
        case 6: _t->refresh(); break;
        case 7: _t->cd((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->historyForward(); break;
        case 9: _t->historyBack(); break;
        case 10: _t->rename((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->remove((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 12: _t->remove((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: { QString _r = _t->fileNameAt((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 14: { bool _r = _t->isDirectoryAt((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->beginMkDir(); break;
        case 16: _t->commitMkDir((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->upload((*reinterpret_cast<std::add_pointer_t<QList<QUrl>>>(_a[1]))); break;
        case 18: _t->uploadTo((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<QUrl>>>(_a[2]))); break;
        case 19: _t->download((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3]))); break;
        case 20: _t->download((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[2]))); break;
        case 21: { bool _r = _t->isTooLarge((*reinterpret_cast<std::add_pointer_t<QList<QUrl>>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FileManager::*)()>(_a, &FileManager::isBusyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FileManager::*)()>(_a, &FileManager::currentPathChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FileManager::*)()>(_a, &FileManager::newDirectoryIndexChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (FileManager::*)()>(_a, &FileManager::errorOccured, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->isBusy(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->isRoot(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->canGoBack(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->canGoForward(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->currentPath(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->newDirectoryIndex(); break;
        default: break;
        }
    }
}

const QMetaObject *Flipper::Zero::FileManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Flipper::Zero::FileManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Flipper4Zero11FileManagerE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Failable"))
        return static_cast< Failable*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int Flipper::Zero::FileManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
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
void Flipper::Zero::FileManager::isBusyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Flipper::Zero::FileManager::currentPathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Flipper::Zero::FileManager::newDirectoryIndexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Flipper::Zero::FileManager::errorOccured()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
