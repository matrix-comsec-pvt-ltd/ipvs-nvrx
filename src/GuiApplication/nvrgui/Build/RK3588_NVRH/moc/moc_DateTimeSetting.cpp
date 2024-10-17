/****************************************************************************
** Meta object code from reading C++ file 'DateTimeSetting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/BasicSettings/DateTimeSetting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DateTimeSetting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DateTimeSetting_t {
    QByteArrayData data[12];
    char stringdata0[195];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DateTimeSetting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DateTimeSetting_t qt_meta_stringdata_DateTimeSetting = {
    {
QT_MOC_LITERAL(0, 0, 15), // "DateTimeSetting"
QT_MOC_LITERAL(1, 16, 25), // "slotTimeDateSetBtnClicked"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 5), // "index"
QT_MOC_LITERAL(4, 49, 25), // "slotAutoSyncButtonClicked"
QT_MOC_LITERAL(5, 75, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(6, 95, 5), // "state"
QT_MOC_LITERAL(7, 101, 20), // "slotNtpServerChanged"
QT_MOC_LITERAL(8, 122, 6), // "string"
QT_MOC_LITERAL(9, 129, 11), // "indexInPage"
QT_MOC_LITERAL(10, 141, 37), // "slotSyncCameraTimeOptionButto..."
QT_MOC_LITERAL(11, 179, 15) // "slotDateChanged"

    },
    "DateTimeSetting\0slotTimeDateSetBtnClicked\0"
    "\0index\0slotAutoSyncButtonClicked\0"
    "OPTION_STATE_TYPE_e\0state\0"
    "slotNtpServerChanged\0string\0indexInPage\0"
    "slotSyncCameraTimeOptionButtonClicked\0"
    "slotDateChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DateTimeSetting[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x0a /* Public */,
       4,    2,   42,    2, 0x0a /* Public */,
       7,    2,   47,    2, 0x0a /* Public */,
      10,    2,   52,    2, 0x0a /* Public */,
      11,    0,   57,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Int,    6,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    8,    9,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Int,    6,    3,
    QMetaType::Void,

       0        // eod
};

void DateTimeSetting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DateTimeSetting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotTimeDateSetBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotAutoSyncButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotNtpServerChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 3: _t->slotSyncCameraTimeOptionButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->slotDateChanged(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DateTimeSetting::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_DateTimeSetting.data,
    qt_meta_data_DateTimeSetting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DateTimeSetting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DateTimeSetting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DateTimeSetting.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int DateTimeSetting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
