/****************************************************************************
** Meta object code from reading C++ file 'screencanvas.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../application/screencanvas.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'screencanvas.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12ScreenCanvasE_t {};
} // unnamed namespace

template <> constexpr inline auto ScreenCanvas::qt_create_metaobjectdata<qt_meta_tag_ZN12ScreenCanvasE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScreenCanvas",
        "zoomFactorChanged",
        "",
        "canvasSizeChanged",
        "foregroundColorChanged",
        "backgroundColorChanged",
        "frameChanged",
        "updateImplicitSize",
        "saveImage",
        "QUrl",
        "url",
        "scale",
        "copyToClipboard",
        "zoomFactor",
        "foregroundColor",
        "QColor",
        "backgroundColor",
        "frame",
        "ScreenFrame"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'zoomFactorChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'canvasSizeChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'foregroundColorChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'backgroundColorChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'frameChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateImplicitSize'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'saveImage'
        QtMocHelpers::MethodData<bool(const QUrl &, int)>(8, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 9, 10 }, { QMetaType::Int, 11 },
        }}),
        // Method 'saveImage'
        QtMocHelpers::MethodData<bool(const QUrl &)>(8, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Bool, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Method 'copyToClipboard'
        QtMocHelpers::MethodData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 },
        }}),
        // Method 'copyToClipboard'
        QtMocHelpers::MethodData<void()>(12, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'zoomFactor'
        QtMocHelpers::PropertyData<qreal>(13, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'foregroundColor'
        QtMocHelpers::PropertyData<QColor>(14, 0x80000000 | 15, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 2),
        // property 'backgroundColor'
        QtMocHelpers::PropertyData<QColor>(16, 0x80000000 | 15, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 3),
        // property 'frame'
        QtMocHelpers::PropertyData<ScreenFrame>(17, 0x80000000 | 18, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 4),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScreenCanvas, qt_meta_tag_ZN12ScreenCanvasE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScreenCanvas::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickPaintedItem::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12ScreenCanvasE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12ScreenCanvasE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12ScreenCanvasE_t>.metaTypes,
    nullptr
} };

void ScreenCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScreenCanvas *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->zoomFactorChanged(); break;
        case 1: _t->canvasSizeChanged(); break;
        case 2: _t->foregroundColorChanged(); break;
        case 3: _t->backgroundColorChanged(); break;
        case 4: _t->frameChanged(); break;
        case 5: _t->updateImplicitSize(); break;
        case 6: { bool _r = _t->saveImage((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 7: { bool _r = _t->saveImage((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 8: _t->copyToClipboard((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->copyToClipboard(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScreenCanvas::*)()>(_a, &ScreenCanvas::zoomFactorChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScreenCanvas::*)()>(_a, &ScreenCanvas::canvasSizeChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScreenCanvas::*)()>(_a, &ScreenCanvas::foregroundColorChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScreenCanvas::*)()>(_a, &ScreenCanvas::backgroundColorChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScreenCanvas::*)()>(_a, &ScreenCanvas::frameChanged, 4))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ScreenFrame >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<qreal*>(_v) = _t->zoomFactor(); break;
        case 1: *reinterpret_cast<QColor*>(_v) = _t->foregroundColor(); break;
        case 2: *reinterpret_cast<QColor*>(_v) = _t->backgroundColor(); break;
        case 3: *reinterpret_cast<ScreenFrame*>(_v) = _t->frame(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setZoomFactor(*reinterpret_cast<qreal*>(_v)); break;
        case 1: _t->setForegroundColor(*reinterpret_cast<QColor*>(_v)); break;
        case 2: _t->setBackgroundColor(*reinterpret_cast<QColor*>(_v)); break;
        case 3: _t->setFrame(*reinterpret_cast<ScreenFrame*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *ScreenCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScreenCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12ScreenCanvasE_t>.strings))
        return static_cast<void*>(this);
    return QQuickPaintedItem::qt_metacast(_clname);
}

int ScreenCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickPaintedItem::qt_metacall(_c, _id, _a);
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
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ScreenCanvas::zoomFactorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ScreenCanvas::canvasSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ScreenCanvas::foregroundColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ScreenCanvas::backgroundColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ScreenCanvas::frameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
