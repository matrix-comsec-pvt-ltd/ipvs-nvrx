/****************************************************************************
** Meta object code from reading C++ file 'VirtualKeypad.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../VirtualKeypad.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VirtualKeypad.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VirtualKeypad_t {
    QByteArrayData data[9];
    char stringdata0[93];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VirtualKeypad_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VirtualKeypad_t qt_meta_stringdata_VirtualKeypad = {
    {
QT_MOC_LITERAL(0, 0, 13), // "VirtualKeypad"
QT_MOC_LITERAL(1, 14, 14), // "sigKeyDetected"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 10), // "KEY_TYPE_e"
QT_MOC_LITERAL(4, 41, 7), // "keyType"
QT_MOC_LITERAL(5, 49, 3), // "str"
QT_MOC_LITERAL(6, 53, 16), // "slotKeyDeteceted"
QT_MOC_LITERAL(7, 70, 5), // "index"
QT_MOC_LITERAL(8, 76, 16) // "slotCloseClicked"

    },
    "VirtualKeypad\0sigKeyDetected\0\0KEY_TYPE_e\0"
    "keyType\0str\0slotKeyDeteceted\0index\0"
    "slotCloseClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VirtualKeypad[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   34,    2, 0x0a /* Public */,
       8,    1,   39,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::UShort,    4,    7,
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void VirtualKeypad::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VirtualKeypad *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigKeyDetected((*reinterpret_cast< KEY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->slotKeyDeteceted((*reinterpret_cast< KEY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 2: _t->slotCloseClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VirtualKeypad::*)(KEY_TYPE_e , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VirtualKeypad::sigKeyDetected)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VirtualKeypad::staticMetaObject = { {
    QMetaObject::SuperData::link<Rectangle::staticMetaObject>(),
    qt_meta_stringdata_VirtualKeypad.data,
    qt_meta_data_VirtualKeypad,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VirtualKeypad::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VirtualKeypad::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VirtualKeypad.stringdata0))
        return static_cast<void*>(this);
    return Rectangle::qt_metacast(_clname);
}

int VirtualKeypad::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Rectangle::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void VirtualKeypad::sigKeyDetected(KEY_TYPE_e _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
