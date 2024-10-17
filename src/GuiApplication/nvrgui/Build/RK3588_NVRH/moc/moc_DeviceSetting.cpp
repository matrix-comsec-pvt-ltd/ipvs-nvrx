/****************************************************************************
** Meta object code from reading C++ file 'DeviceSetting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/Devices/DeviceSetting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeviceSetting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DeviceSetting_t {
    QByteArrayData data[13];
    char stringdata0[190];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeviceSetting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeviceSetting_t qt_meta_stringdata_DeviceSetting = {
    {
QT_MOC_LITERAL(0, 0, 13), // "DeviceSetting"
QT_MOC_LITERAL(1, 14, 27), // "slotDropDownBoxValueChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 6), // "string"
QT_MOC_LITERAL(4, 50, 11), // "indexInPage"
QT_MOC_LITERAL(5, 62, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(6, 86, 5), // "index"
QT_MOC_LITERAL(7, 92, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(8, 108, 7), // "msgType"
QT_MOC_LITERAL(9, 116, 24), // "slotControlButtonClicked"
QT_MOC_LITERAL(10, 141, 22), // "slotRadioButtonClicked"
QT_MOC_LITERAL(11, 164, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(12, 184, 5) // "state"

    },
    "DeviceSetting\0slotDropDownBoxValueChanged\0"
    "\0string\0indexInPage\0slotTextBoxLoadInfopage\0"
    "index\0INFO_MSG_TYPE_e\0msgType\0"
    "slotControlButtonClicked\0"
    "slotRadioButtonClicked\0OPTION_STATE_TYPE_e\0"
    "state"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeviceSetting[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x0a /* Public */,
       5,    2,   39,    2, 0x0a /* Public */,
       9,    1,   44,    2, 0x0a /* Public */,
      10,    2,   47,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    3,    4,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7,    6,    8,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,   12,    4,

       0        // eod
};

void DeviceSetting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeviceSetting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotDropDownBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 1: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 2: _t->slotControlButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotRadioButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DeviceSetting::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_DeviceSetting.data,
    qt_meta_data_DeviceSetting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DeviceSetting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeviceSetting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceSetting.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int DeviceSetting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
