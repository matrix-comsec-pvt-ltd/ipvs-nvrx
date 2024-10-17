/****************************************************************************
** Meta object code from reading C++ file 'GeneralSetting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/BasicSettings/GeneralSetting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GeneralSetting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GeneralSetting_t {
    QByteArrayData data[14];
    char stringdata0[210];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GeneralSetting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GeneralSetting_t qt_meta_stringdata_GeneralSetting = {
    {
QT_MOC_LITERAL(0, 0, 14), // "GeneralSetting"
QT_MOC_LITERAL(1, 15, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 5), // "index"
QT_MOC_LITERAL(4, 46, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(5, 62, 7), // "msgType"
QT_MOC_LITERAL(6, 70, 17), // "slotOptionClicked"
QT_MOC_LITERAL(7, 88, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(8, 108, 12), // "currentState"
QT_MOC_LITERAL(9, 121, 11), // "indexInPage"
QT_MOC_LITERAL(10, 133, 25), // "slotPageOpenButtonClicked"
QT_MOC_LITERAL(11, 159, 16), // "slotObjectDelete"
QT_MOC_LITERAL(12, 176, 19), // "slotImageMouseHover"
QT_MOC_LITERAL(13, 196, 13) // "isMouserHover"

    },
    "GeneralSetting\0slotTextBoxLoadInfopage\0"
    "\0index\0INFO_MSG_TYPE_e\0msgType\0"
    "slotOptionClicked\0OPTION_STATE_TYPE_e\0"
    "currentState\0indexInPage\0"
    "slotPageOpenButtonClicked\0slotObjectDelete\0"
    "slotImageMouseHover\0isMouserHover"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GeneralSetting[] = {

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
       6,    2,   44,    2, 0x0a /* Public */,
      10,    1,   49,    2, 0x0a /* Public */,
      11,    0,   52,    2, 0x0a /* Public */,
      12,    2,   53,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    8,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    2,   13,

       0        // eod
};

void GeneralSetting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GeneralSetting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 1: _t->slotOptionClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotPageOpenButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotObjectDelete(); break;
        case 4: _t->slotImageMouseHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GeneralSetting::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_GeneralSetting.data,
    qt_meta_data_GeneralSetting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GeneralSetting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GeneralSetting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GeneralSetting.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int GeneralSetting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
