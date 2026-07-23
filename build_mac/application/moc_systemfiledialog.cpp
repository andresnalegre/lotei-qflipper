/****************************************************************************
** Meta object code from reading C++ file 'systemfiledialog.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../application/systemfiledialog.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'systemfiledialog.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16SystemFileDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto SystemFileDialog::qt_create_metaobjectdata<qt_meta_tag_ZN16SystemFileDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SystemFileDialog",
        "finished",
        "",
        "accepted",
        "onFileDialogAccepted",
        "onFileDialogFinished",
        "beginOpenFiles",
        "SystemFileDialog::StandardLocation",
        "openLocation",
        "nameFilters",
        "beginOpenFile",
        "beginOpenDir",
        "beginSaveFile",
        "defaultFileName",
        "beginSaveDir",
        "close",
        "selectedNameFilter",
        "fileUrl",
        "QUrl",
        "fileUrls",
        "QList<QUrl>",
        "StandardLocation",
        "LastLocation",
        "HomeLocation",
        "DownloadsLocation",
        "PicturesLocation",
        "DocumentsLocation",
        "DefaultLocation"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'finished'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'accepted'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onFileDialogAccepted'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFileDialogFinished'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'beginOpenFiles'
        QtMocHelpers::MethodData<void(SystemFileDialog::StandardLocation, const QStringList &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 }, { QMetaType::QStringList, 9 },
        }}),
        // Method 'beginOpenFile'
        QtMocHelpers::MethodData<void(SystemFileDialog::StandardLocation, const QStringList &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 }, { QMetaType::QStringList, 9 },
        }}),
        // Method 'beginOpenDir'
        QtMocHelpers::MethodData<void(SystemFileDialog::StandardLocation)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Method 'beginSaveFile'
        QtMocHelpers::MethodData<void(SystemFileDialog::StandardLocation, const QStringList &, const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 }, { QMetaType::QStringList, 9 }, { QMetaType::QString, 13 },
        }}),
        // Method 'beginSaveDir'
        QtMocHelpers::MethodData<void(SystemFileDialog::StandardLocation)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Method 'close'
        QtMocHelpers::MethodData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'selectedNameFilter'
        QtMocHelpers::PropertyData<QString>(16, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'fileUrl'
        QtMocHelpers::PropertyData<QUrl>(17, 0x80000000 | 18, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'fileUrls'
        QtMocHelpers::PropertyData<QList<QUrl>>(19, 0x80000000 | 20, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'StandardLocation'
        QtMocHelpers::EnumData<enum StandardLocation>(21, 21, QMC::EnumFlags{}).add({
            {   22, StandardLocation::LastLocation },
            {   23, StandardLocation::HomeLocation },
            {   24, StandardLocation::DownloadsLocation },
            {   25, StandardLocation::PicturesLocation },
            {   26, StandardLocation::DocumentsLocation },
            {   27, StandardLocation::DefaultLocation },
        }),
    };
    return QtMocHelpers::metaObjectData<SystemFileDialog, qt_meta_tag_ZN16SystemFileDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SystemFileDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SystemFileDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SystemFileDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16SystemFileDialogE_t>.metaTypes,
    nullptr
} };

void SystemFileDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SystemFileDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->finished(); break;
        case 1: _t->accepted(); break;
        case 2: _t->onFileDialogAccepted(); break;
        case 3: _t->onFileDialogFinished(); break;
        case 4: _t->beginOpenFiles((*reinterpret_cast<std::add_pointer_t<SystemFileDialog::StandardLocation>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[2]))); break;
        case 5: _t->beginOpenFile((*reinterpret_cast<std::add_pointer_t<SystemFileDialog::StandardLocation>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[2]))); break;
        case 6: _t->beginOpenDir((*reinterpret_cast<std::add_pointer_t<SystemFileDialog::StandardLocation>>(_a[1]))); break;
        case 7: _t->beginSaveFile((*reinterpret_cast<std::add_pointer_t<SystemFileDialog::StandardLocation>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 8: _t->beginSaveDir((*reinterpret_cast<std::add_pointer_t<SystemFileDialog::StandardLocation>>(_a[1]))); break;
        case 9: _t->close(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SystemFileDialog::*)()>(_a, &SystemFileDialog::finished, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SystemFileDialog::*)()>(_a, &SystemFileDialog::accepted, 1))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->selectedNameFilter(); break;
        case 1: *reinterpret_cast<QUrl*>(_v) = _t->fileUrl(); break;
        case 2: *reinterpret_cast<QList<QUrl>*>(_v) = _t->fileUrls(); break;
        default: break;
        }
    }
}

const QMetaObject *SystemFileDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SystemFileDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SystemFileDialogE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SystemFileDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
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
void SystemFileDialog::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SystemFileDialog::accepted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
