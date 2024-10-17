/****************************************************************************
** Meta object code from reading C++ file 'DateAndTime.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Wizard/DateAndTime.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DateAndTime.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DateAndTime_t {
    QByteArrayData data[12];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DateAndTime_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DateAndTime_t qt_meta_stringdata_DateAndTime = {
    {
QT_MOC_LITERAL(0, 0, 11), // "DateAndTime"
QT_MOC_LITERAL(1, 12, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 5), // "index"
QT_MOC_LITERAL(4, 40, 25), // "slotTimeDateSetBtnClicked"
QT_MOC_LITERAL(5, 66, 27), // "slotDropDownBoxValueChanged"
QT_MOC_LITERAL(6, 94, 6), // "string"
QT_MOC_LITERAL(7, 101, 25), // "slotAutoSyncButtonClicked"
QT_MOC_LITERAL(8, 127, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(9, 147, 5), // "state"
QT_MOC_LITERAL(10, 153, 37), // "slotSyncCameraTimeOptionButto..."
QT_MOC_LITERAL(11, 191, 15) // "slotDateChanged"

    },
    "DateAndTime\0slotInfoPageBtnclick\0\0"
    "index\0slotTimeDateSetBtnClicked\0"
    "slotDropDownBoxValueChanged\0string\0"
    "slotAutoSyncButtonClicked\0OPTION_STATE_TYPE_e\0"
    "state\0slotSyncCameraTimeOptionButtonClicked\0"
    "slotDateChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DateAndTime[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x0a /* Public */,
       4,    1,   47,    2, 0x0a /* Public */,
       5,    2,   50,    2, 0x0a /* Public */,
       7,    2,   55,    2, 0x0a /* Public */,
      10,    2,   60,    2, 0x0a /* Public */,
      11,    0,   65,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    6,    2,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,    2,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,    3,
    QMetaType::Void,

       0        // eod
};

void DateAndTime::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DateAndTime *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotTimeDateSetBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotDropDownBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 3: _t->slotAutoSyncButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->slotSyncCameraTimeOptionButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->slotDateChanged(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DateAndTime::staticMetaObject = { {
    QMetaObject::SuperData::link<WizardCommon::staticMetaObject>(),
    qt_meta_stringdata_DateAndTime.data,
    qt_meta_data_DateAndTime,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DateAndTime::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DateAndTime::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DateAndTime.stringdata0))
        return static_cast<void*>(this);
    return WizardCommon::qt_metacast(_clname);
}

int DateAndTime::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = WizardCommon::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
