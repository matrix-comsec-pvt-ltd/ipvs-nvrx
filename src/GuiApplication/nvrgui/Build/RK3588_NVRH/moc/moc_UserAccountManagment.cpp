/****************************************************************************
** Meta object code from reading C++ file 'UserAccountManagment.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/UserAccountManagmentSettings/UserAccountManagment.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UserAccountManagment.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UserAccountManagment_t {
    QByteArrayData data[11];
    char stringdata0[187];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UserAccountManagment_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UserAccountManagment_t qt_meta_stringdata_UserAccountManagment = {
    {
QT_MOC_LITERAL(0, 0, 20), // "UserAccountManagment"
QT_MOC_LITERAL(1, 21, 19), // "slotCheckBoxClicked"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(4, 62, 22), // "slotSpinBoxValueChange"
QT_MOC_LITERAL(5, 85, 21), // "slotControlBtnClicked"
QT_MOC_LITERAL(6, 107, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(7, 131, 5), // "index"
QT_MOC_LITERAL(8, 137, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(9, 153, 7), // "msgType"
QT_MOC_LITERAL(10, 161, 25) // "slotPageNumberButtonClick"

    },
    "UserAccountManagment\0slotCheckBoxClicked\0"
    "\0OPTION_STATE_TYPE_e\0slotSpinBoxValueChange\0"
    "slotControlBtnClicked\0slotTextBoxLoadInfopage\0"
    "index\0INFO_MSG_TYPE_e\0msgType\0"
    "slotPageNumberButtonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UserAccountManagment[] = {

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
       1,    2,   39,    2, 0x0a /* Public */,
       4,    2,   44,    2, 0x0a /* Public */,
       5,    1,   49,    2, 0x0a /* Public */,
       6,    2,   52,    2, 0x0a /* Public */,
      10,    1,   57,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    7,    9,
    QMetaType::Void, QMetaType::QString,    2,

       0        // eod
};

void UserAccountManagment::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UserAccountManagment *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotCheckBoxClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 2: _t->slotControlBtnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 4: _t->slotPageNumberButtonClick((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UserAccountManagment::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_UserAccountManagment.data,
    qt_meta_data_UserAccountManagment,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UserAccountManagment::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UserAccountManagment::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UserAccountManagment.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int UserAccountManagment::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
