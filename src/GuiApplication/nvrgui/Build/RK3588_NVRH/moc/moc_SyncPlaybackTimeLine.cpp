/****************************************************************************
** Meta object code from reading C++ file 'SyncPlaybackTimeLine.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/SyncPlayback/SyncPlaybackTimeLine.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncPlaybackTimeLine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SyncPlaybackTimeLine_t {
    QByteArrayData data[18];
    char stringdata0[325];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SyncPlaybackTimeLine_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SyncPlaybackTimeLine_t qt_meta_stringdata_SyncPlaybackTimeLine = {
    {
QT_MOC_LITERAL(0, 0, 20), // "SyncPlaybackTimeLine"
QT_MOC_LITERAL(1, 21, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 5), // "index"
QT_MOC_LITERAL(4, 52, 22), // "sigFocusToOtherElement"
QT_MOC_LITERAL(5, 75, 17), // "isPrevoiusElement"
QT_MOC_LITERAL(6, 93, 24), // "sigSliderPositionChanged"
QT_MOC_LITERAL(7, 118, 29), // "sigSliderPositionChangedStart"
QT_MOC_LITERAL(8, 148, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(9, 173, 22), // "slotSliderValueChanged"
QT_MOC_LITERAL(10, 196, 12), // "changedValue"
QT_MOC_LITERAL(11, 209, 11), // "indexInpage"
QT_MOC_LITERAL(12, 221, 10), // "sliderMove"
QT_MOC_LITERAL(13, 232, 24), // "slotControlButtonClicked"
QT_MOC_LITERAL(14, 257, 11), // "indexInPage"
QT_MOC_LITERAL(15, 269, 24), // "slotMouseReleaseOnSlider"
QT_MOC_LITERAL(16, 294, 5), // "value"
QT_MOC_LITERAL(17, 300, 24) // "slotMousePressedOnSlider"

    },
    "SyncPlaybackTimeLine\0sigUpdateCurrentElement\0"
    "\0index\0sigFocusToOtherElement\0"
    "isPrevoiusElement\0sigSliderPositionChanged\0"
    "sigSliderPositionChangedStart\0"
    "slotUpdateCurrentElement\0"
    "slotSliderValueChanged\0changedValue\0"
    "indexInpage\0sliderMove\0slotControlButtonClicked\0"
    "indexInPage\0slotMouseReleaseOnSlider\0"
    "value\0slotMousePressedOnSlider"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SyncPlaybackTimeLine[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    1,   62,    2, 0x06 /* Public */,
       6,    0,   65,    2, 0x06 /* Public */,
       7,    0,   66,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   67,    2, 0x0a /* Public */,
       9,    3,   70,    2, 0x0a /* Public */,
      13,    1,   77,    2, 0x0a /* Public */,
      15,    2,   80,    2, 0x0a /* Public */,
      17,    2,   85,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   10,   11,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   16,   14,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   16,   14,

       0        // eod
};

void SyncPlaybackTimeLine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SyncPlaybackTimeLine *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigFocusToOtherElement((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->sigSliderPositionChanged(); break;
        case 3: _t->sigSliderPositionChangedStart(); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotSliderValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 6: _t->slotControlButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotMouseReleaseOnSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->slotMousePressedOnSlider((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SyncPlaybackTimeLine::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackTimeLine::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SyncPlaybackTimeLine::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackTimeLine::sigFocusToOtherElement)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SyncPlaybackTimeLine::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackTimeLine::sigSliderPositionChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SyncPlaybackTimeLine::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackTimeLine::sigSliderPositionChangedStart)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SyncPlaybackTimeLine::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_SyncPlaybackTimeLine.data,
    qt_meta_data_SyncPlaybackTimeLine,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SyncPlaybackTimeLine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SyncPlaybackTimeLine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SyncPlaybackTimeLine.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int SyncPlaybackTimeLine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void SyncPlaybackTimeLine::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SyncPlaybackTimeLine::sigFocusToOtherElement(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SyncPlaybackTimeLine::sigSliderPositionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SyncPlaybackTimeLine::sigSliderPositionChangedStart()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
