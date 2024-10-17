/****************************************************************************
** Meta object code from reading C++ file 'Lan1Setting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/NetworkSettings/Lan1Setting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Lan1Setting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Lan1Setting_t {
    QByteArrayData data[9];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Lan1Setting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Lan1Setting_t qt_meta_stringdata_Lan1Setting = {
    {
QT_MOC_LITERAL(0, 0, 11), // "Lan1Setting"
QT_MOC_LITERAL(1, 12, 29), // "slotIpAssignModeButtonClicked"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(4, 63, 5), // "state"
QT_MOC_LITERAL(5, 69, 5), // "index"
QT_MOC_LITERAL(6, 75, 25), // "slotIpTextBoxLoadInfopage"
QT_MOC_LITERAL(7, 101, 22), // "slotIpTextBoxEntryDone"
QT_MOC_LITERAL(8, 124, 23) // "slotSpinBoxValueChanged"

    },
    "Lan1Setting\0slotIpAssignModeButtonClicked\0"
    "\0OPTION_STATE_TYPE_e\0state\0index\0"
    "slotIpTextBoxLoadInfopage\0"
    "slotIpTextBoxEntryDone\0slotSpinBoxValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Lan1Setting[] = {

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
       6,    1,   39,    2, 0x0a /* Public */,
       7,    1,   42,    2, 0x0a /* Public */,
       8,    2,   45,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, QMetaType::UInt,    5,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    5,

       0        // eod
};

void Lan1Setting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Lan1Setting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotIpAssignModeButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->slotIpTextBoxLoadInfopage((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 2: _t->slotIpTextBoxEntryDone((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 3: _t->slotSpinBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Lan1Setting::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_Lan1Setting.data,
    qt_meta_data_Lan1Setting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Lan1Setting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Lan1Setting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Lan1Setting.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int Lan1Setting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
