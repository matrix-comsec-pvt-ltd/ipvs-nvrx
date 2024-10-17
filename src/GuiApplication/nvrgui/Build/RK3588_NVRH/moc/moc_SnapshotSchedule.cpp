/****************************************************************************
** Meta object code from reading C++ file 'SnapshotSchedule.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/SnapshotSchedule.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SnapshotSchedule.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SnapshotSchedule_t {
    QByteArrayData data[12];
    char stringdata0[194];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SnapshotSchedule_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SnapshotSchedule_t qt_meta_stringdata_SnapshotSchedule = {
    {
QT_MOC_LITERAL(0, 0, 16), // "SnapshotSchedule"
QT_MOC_LITERAL(1, 17, 20), // "slotPageOpenBtnClick"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 5), // "index"
QT_MOC_LITERAL(4, 45, 19), // "slotCopytoCamDelete"
QT_MOC_LITERAL(5, 65, 20), // "slotSchdObjectDelete"
QT_MOC_LITERAL(6, 86, 23), // "slotDropDownValueChange"
QT_MOC_LITERAL(7, 110, 19), // "slotCheckboxClicked"
QT_MOC_LITERAL(8, 130, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(9, 150, 19), // "slotTextBoxInfoPage"
QT_MOC_LITERAL(10, 170, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(11, 186, 7) // "msgType"

    },
    "SnapshotSchedule\0slotPageOpenBtnClick\0"
    "\0index\0slotCopytoCamDelete\0"
    "slotSchdObjectDelete\0slotDropDownValueChange\0"
    "slotCheckboxClicked\0OPTION_STATE_TYPE_e\0"
    "slotTextBoxInfoPage\0INFO_MSG_TYPE_e\0"
    "msgType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SnapshotSchedule[] = {

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
       5,    0,   50,    2, 0x0a /* Public */,
       6,    2,   51,    2, 0x0a /* Public */,
       7,    2,   56,    2, 0x0a /* Public */,
       9,    2,   61,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::UChar,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 10,    3,   11,

       0        // eod
};

void SnapshotSchedule::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SnapshotSchedule *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotPageOpenBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotCopytoCamDelete((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 2: _t->slotSchdObjectDelete(); break;
        case 3: _t->slotDropDownValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 4: _t->slotCheckboxClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->slotTextBoxInfoPage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SnapshotSchedule::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_SnapshotSchedule.data,
    qt_meta_data_SnapshotSchedule,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SnapshotSchedule::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SnapshotSchedule::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SnapshotSchedule.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int SnapshotSchedule::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
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
