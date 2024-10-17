/****************************************************************************
** Meta object code from reading C++ file 'ConfigureCamera.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Wizard/ConfigureCamera.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConfigureCamera.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ConfigureCamera_t {
    QByteArrayData data[13];
    char stringdata0[208];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ConfigureCamera_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ConfigureCamera_t qt_meta_stringdata_ConfigureCamera = {
    {
QT_MOC_LITERAL(0, 0, 15), // "ConfigureCamera"
QT_MOC_LITERAL(1, 16, 23), // "slotOptSelButtonClicked"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(4, 61, 5), // "state"
QT_MOC_LITERAL(5, 67, 5), // "index"
QT_MOC_LITERAL(6, 73, 22), // "slotIpAddressEntryDone"
QT_MOC_LITERAL(7, 96, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(8, 120, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(9, 136, 7), // "infoMsg"
QT_MOC_LITERAL(10, 144, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(11, 165, 25), // "slotIpAddressLoadInfoPage"
QT_MOC_LITERAL(12, 191, 16) // "slotValueChanged"

    },
    "ConfigureCamera\0slotOptSelButtonClicked\0"
    "\0OPTION_STATE_TYPE_e\0state\0index\0"
    "slotIpAddressEntryDone\0slotTextBoxLoadInfopage\0"
    "INFO_MSG_TYPE_e\0infoMsg\0slotInfoPageBtnclick\0"
    "slotIpAddressLoadInfoPage\0slotValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ConfigureCamera[] = {

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
       1,    2,   44,    2, 0x0a /* Public */,
       6,    1,   49,    2, 0x0a /* Public */,
       7,    2,   52,    2, 0x0a /* Public */,
      10,    1,   57,    2, 0x0a /* Public */,
      11,    1,   60,    2, 0x0a /* Public */,
      12,    3,   63,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    5,    9,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Int,    2,    2,    5,

       0        // eod
};

void ConfigureCamera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ConfigureCamera *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotOptSelButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->slotIpAddressEntryDone((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 2: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotIpAddressLoadInfoPage((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 5: _t->slotValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ConfigureCamera::staticMetaObject = { {
    QMetaObject::SuperData::link<WizardCommon::staticMetaObject>(),
    qt_meta_stringdata_ConfigureCamera.data,
    qt_meta_data_ConfigureCamera,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ConfigureCamera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConfigureCamera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ConfigureCamera.stringdata0))
        return static_cast<void*>(this);
    return WizardCommon::qt_metacast(_clname);
}

int ConfigureCamera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
