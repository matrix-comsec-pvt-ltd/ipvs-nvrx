/****************************************************************************
** Meta object code from reading C++ file 'FtpClient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/NetworkSettings/FtpClient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FtpClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FtpClient_t {
    QByteArrayData data[11];
    char stringdata0[156];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FtpClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FtpClient_t qt_meta_stringdata_FtpClient = {
    {
QT_MOC_LITERAL(0, 0, 9), // "FtpClient"
QT_MOC_LITERAL(1, 10, 23), // "slotSpinboxValueChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 23), // "slotEnableButtonClicked"
QT_MOC_LITERAL(4, 59, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(5, 79, 5), // "state"
QT_MOC_LITERAL(6, 85, 5), // "index"
QT_MOC_LITERAL(7, 91, 16), // "slotTestBtnClick"
QT_MOC_LITERAL(8, 108, 23), // "slotTextboxLoadInfopage"
QT_MOC_LITERAL(9, 132, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(10, 148, 7) // "msgType"

    },
    "FtpClient\0slotSpinboxValueChanged\0\0"
    "slotEnableButtonClicked\0OPTION_STATE_TYPE_e\0"
    "state\0index\0slotTestBtnClick\0"
    "slotTextboxLoadInfopage\0INFO_MSG_TYPE_e\0"
    "msgType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FtpClient[] = {

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
       3,    2,   39,    2, 0x0a /* Public */,
       7,    1,   44,    2, 0x0a /* Public */,
       8,    2,   47,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Int,    5,    6,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,    6,   10,

       0        // eod
};

void FtpClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FtpClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotSpinboxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 1: _t->slotEnableButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotTestBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotTextboxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FtpClient::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_FtpClient.data,
    qt_meta_data_FtpClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FtpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FtpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FtpClient.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int FtpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
