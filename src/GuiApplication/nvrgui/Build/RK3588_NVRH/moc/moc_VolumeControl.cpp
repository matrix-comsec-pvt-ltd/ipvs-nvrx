/****************************************************************************
** Meta object code from reading C++ file 'VolumeControl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../VolumeControl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VolumeControl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VolumeControl_t {
    QByteArrayData data[16];
    char stringdata0[235];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VolumeControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VolumeControl_t qt_meta_stringdata_VolumeControl = {
    {
QT_MOC_LITERAL(0, 0, 13), // "VolumeControl"
QT_MOC_LITERAL(1, 14, 27), // "sigChangeToolbarButtonState"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 65, 11), // "buttonIndex"
QT_MOC_LITERAL(5, 77, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(6, 90, 5), // "state"
QT_MOC_LITERAL(7, 96, 21), // "slotMuteButtonClicked"
QT_MOC_LITERAL(8, 118, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(9, 138, 12), // "currentState"
QT_MOC_LITERAL(10, 151, 11), // "indexInPage"
QT_MOC_LITERAL(11, 163, 16), // "slotValueChanged"
QT_MOC_LITERAL(12, 180, 12), // "changedValue"
QT_MOC_LITERAL(13, 193, 10), // "sliderMove"
QT_MOC_LITERAL(14, 204, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(15, 229, 5) // "index"

    },
    "VolumeControl\0sigChangeToolbarButtonState\0"
    "\0TOOLBAR_BUTTON_TYPE_e\0buttonIndex\0"
    "STATE_TYPE_e\0state\0slotMuteButtonClicked\0"
    "OPTION_STATE_TYPE_e\0currentState\0"
    "indexInPage\0slotValueChanged\0changedValue\0"
    "sliderMove\0slotUpdateCurrentElement\0"
    "index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VolumeControl[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   39,    2, 0x0a /* Public */,
      11,    3,   44,    2, 0x0a /* Public */,
      14,    1,   51,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,   10,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   12,   10,   13,
    QMetaType::Void, QMetaType::Int,   15,

       0        // eod
};

void VolumeControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VolumeControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigChangeToolbarButtonState((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 1: _t->slotMuteButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 3: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VolumeControl::*)(TOOLBAR_BUTTON_TYPE_e , STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VolumeControl::sigChangeToolbarButtonState)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VolumeControl::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_VolumeControl.data,
    qt_meta_data_VolumeControl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VolumeControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VolumeControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VolumeControl.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int VolumeControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void VolumeControl::sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e _t1, STATE_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
