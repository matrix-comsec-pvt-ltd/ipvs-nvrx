/****************************************************************************
** Meta object code from reading C++ file 'SliderControl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/SliderControl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SliderControl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SliderControl_t {
    QByteArrayData data[16];
    char stringdata0[240];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SliderControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SliderControl_t qt_meta_stringdata_SliderControl = {
    {
QT_MOC_LITERAL(0, 0, 13), // "SliderControl"
QT_MOC_LITERAL(1, 14, 21), // "sigHoverInOutOnSlider"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 9), // "isHoverIn"
QT_MOC_LITERAL(4, 47, 11), // "indexInPage"
QT_MOC_LITERAL(5, 59, 16), // "sigHoverOnSlider"
QT_MOC_LITERAL(6, 76, 5), // "value"
QT_MOC_LITERAL(7, 82, 23), // "sigMouseReleaseOnSlider"
QT_MOC_LITERAL(8, 106, 23), // "sigMousePressedOnSlider"
QT_MOC_LITERAL(9, 130, 15), // "sigValueChanged"
QT_MOC_LITERAL(10, 146, 12), // "changedValue"
QT_MOC_LITERAL(11, 159, 11), // "indexInpage"
QT_MOC_LITERAL(12, 171, 10), // "sliderMove"
QT_MOC_LITERAL(13, 182, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(14, 206, 5), // "index"
QT_MOC_LITERAL(15, 212, 27) // "slotClickEffectTimerTimeout"

    },
    "SliderControl\0sigHoverInOutOnSlider\0"
    "\0isHoverIn\0indexInPage\0sigHoverOnSlider\0"
    "value\0sigMouseReleaseOnSlider\0"
    "sigMousePressedOnSlider\0sigValueChanged\0"
    "changedValue\0indexInpage\0sliderMove\0"
    "sigUpdateCurrentElement\0index\0"
    "slotClickEffectTimerTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SliderControl[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   49,    2, 0x06 /* Public */,
       5,    2,   54,    2, 0x06 /* Public */,
       7,    2,   59,    2, 0x06 /* Public */,
       8,    2,   64,    2, 0x06 /* Public */,
       9,    3,   69,    2, 0x06 /* Public */,
      13,    1,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    0,   79,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   10,   11,   12,
    QMetaType::Void, QMetaType::Int,   14,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void SliderControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SliderControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigHoverInOutOnSlider((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->sigHoverOnSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->sigMouseReleaseOnSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->sigMousePressedOnSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->sigValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 5: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotClickEffectTimerTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SliderControl::*)(bool , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SliderControl::sigHoverInOutOnSlider)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SliderControl::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SliderControl::sigHoverOnSlider)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SliderControl::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SliderControl::sigMouseReleaseOnSlider)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SliderControl::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SliderControl::sigMousePressedOnSlider)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SliderControl::*)(int , int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SliderControl::sigValueChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (SliderControl::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SliderControl::sigUpdateCurrentElement)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SliderControl::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_SliderControl.data,
    qt_meta_data_SliderControl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SliderControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SliderControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SliderControl.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int SliderControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void SliderControl::sigHoverInOutOnSlider(bool _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SliderControl::sigHoverOnSlider(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SliderControl::sigMouseReleaseOnSlider(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SliderControl::sigMousePressedOnSlider(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void SliderControl::sigValueChanged(int _t1, int _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void SliderControl::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
