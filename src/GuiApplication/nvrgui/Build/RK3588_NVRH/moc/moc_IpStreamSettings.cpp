/****************************************************************************
** Meta object code from reading C++ file 'IpStreamSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/IpStreamSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IpStreamSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IpStreamSettings_t {
    QByteArrayData data[19];
    char stringdata0[307];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IpStreamSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IpStreamSettings_t qt_meta_stringdata_IpStreamSettings = {
    {
QT_MOC_LITERAL(0, 0, 16), // "IpStreamSettings"
QT_MOC_LITERAL(1, 17, 23), // "slotSpinboxValueChanged"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 22), // "slotLoadPickListLoader"
QT_MOC_LITERAL(4, 65, 11), // "indexInPage"
QT_MOC_LITERAL(5, 77, 24), // "slotPickListValueChanged"
QT_MOC_LITERAL(6, 102, 3), // "key"
QT_MOC_LITERAL(7, 106, 5), // "value"
QT_MOC_LITERAL(8, 112, 22), // "slotRadioButtonClicked"
QT_MOC_LITERAL(9, 135, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(10, 155, 12), // "currentState"
QT_MOC_LITERAL(11, 168, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(12, 192, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(13, 208, 7), // "msgType"
QT_MOC_LITERAL(14, 216, 24), // "slotTextBoxValueAppended"
QT_MOC_LITERAL(15, 241, 24), // "slotCopyToCamButtonClick"
QT_MOC_LITERAL(16, 266, 5), // "index"
QT_MOC_LITERAL(17, 272, 19), // "slotSubObjectDelete"
QT_MOC_LITERAL(18, 292, 14) // "validateRecord"

    },
    "IpStreamSettings\0slotSpinboxValueChanged\0"
    "\0slotLoadPickListLoader\0indexInPage\0"
    "slotPickListValueChanged\0key\0value\0"
    "slotRadioButtonClicked\0OPTION_STATE_TYPE_e\0"
    "currentState\0slotTextBoxLoadInfopage\0"
    "INFO_MSG_TYPE_e\0msgType\0"
    "slotTextBoxValueAppended\0"
    "slotCopyToCamButtonClick\0index\0"
    "slotSubObjectDelete\0validateRecord"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IpStreamSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   59,    2, 0x0a /* Public */,
       3,    1,   64,    2, 0x0a /* Public */,
       5,    3,   67,    2, 0x0a /* Public */,
       8,    2,   74,    2, 0x0a /* Public */,
      11,    2,   79,    2, 0x0a /* Public */,
      14,    2,   84,    2, 0x0a /* Public */,
      15,    1,   89,    2, 0x0a /* Public */,
      17,    1,   92,    2, 0x0a /* Public */,
      18,    0,   95,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Int,    6,    7,    4,
    QMetaType::Void, 0x80000000 | 9, QMetaType::Int,   10,    4,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 12,    4,   13,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, QMetaType::UChar,    2,
    QMetaType::Bool,

       0        // eod
};

void IpStreamSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IpStreamSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotSpinboxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 1: _t->slotLoadPickListLoader((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotPickListValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: _t->slotRadioButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 5: _t->slotTextBoxValueAppended((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->slotCopyToCamButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotSubObjectDelete((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 8: { bool _r = _t->validateRecord();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IpStreamSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_IpStreamSettings.data,
    qt_meta_data_IpStreamSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IpStreamSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IpStreamSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IpStreamSettings.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int IpStreamSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
