/****************************************************************************
** Meta object code from reading C++ file 'ModifyPassword.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ManagePages/ModifyPassword.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModifyPassword.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ModifyPassword_t {
    QByteArrayData data[7];
    char stringdata0[88];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ModifyPassword_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ModifyPassword_t qt_meta_stringdata_ModifyPassword = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ModifyPassword"
QT_MOC_LITERAL(1, 15, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 5), // "index"
QT_MOC_LITERAL(4, 46, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(5, 62, 7), // "msgType"
QT_MOC_LITERAL(6, 70, 17) // "slotOkButtonClick"

    },
    "ModifyPassword\0slotTextBoxLoadInfopage\0"
    "\0index\0INFO_MSG_TYPE_e\0msgType\0"
    "slotOkButtonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ModifyPassword[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x0a /* Public */,
       6,    1,   29,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void ModifyPassword::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ModifyPassword *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 1: _t->slotOkButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ModifyPassword::staticMetaObject = { {
    QMetaObject::SuperData::link<ManageMenuOptions::staticMetaObject>(),
    qt_meta_stringdata_ModifyPassword.data,
    qt_meta_data_ModifyPassword,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ModifyPassword::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ModifyPassword::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ModifyPassword.stringdata0))
        return static_cast<void*>(this);
    return ManageMenuOptions::qt_metacast(_clname);
}

int ModifyPassword::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ManageMenuOptions::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
