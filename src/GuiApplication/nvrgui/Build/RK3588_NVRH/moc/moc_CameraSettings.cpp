/****************************************************************************
** Meta object code from reading C++ file 'CameraSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/CameraSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CameraSettings_t {
    QByteArrayData data[18];
    char stringdata0[349];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraSettings_t qt_meta_stringdata_CameraSettings = {
    {
QT_MOC_LITERAL(0, 0, 14), // "CameraSettings"
QT_MOC_LITERAL(1, 15, 18), // "sigToolTipShowHide"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 23), // "slotSpinBoxValueChanged"
QT_MOC_LITERAL(4, 59, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(5, 83, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(6, 99, 23), // "slotOptionButtonClicked"
QT_MOC_LITERAL(7, 123, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(8, 143, 20), // "slotCreateCMDRequest"
QT_MOC_LITERAL(9, 164, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(10, 178, 15), // "slotButtonClick"
QT_MOC_LITERAL(11, 194, 20), // "slotPopupPageDeleted"
QT_MOC_LITERAL(12, 215, 11), // "indexInPage"
QT_MOC_LITERAL(13, 227, 32), // "slotcameraListUpdateTimerTimeout"
QT_MOC_LITERAL(14, 260, 25), // "slotIpAddressChangeDelete"
QT_MOC_LITERAL(15, 286, 23), // "slotIpAddressChangeData"
QT_MOC_LITERAL(16, 310, 19), // "slotToolTipShowHide"
QT_MOC_LITERAL(17, 330, 18) // "slotValueListEmpty"

    },
    "CameraSettings\0sigToolTipShowHide\0\0"
    "slotSpinBoxValueChanged\0slotTextBoxLoadInfopage\0"
    "INFO_MSG_TYPE_e\0slotOptionButtonClicked\0"
    "OPTION_STATE_TYPE_e\0slotCreateCMDRequest\0"
    "SET_COMMAND_e\0slotButtonClick\0"
    "slotPopupPageDeleted\0indexInPage\0"
    "slotcameraListUpdateTimerTimeout\0"
    "slotIpAddressChangeDelete\0"
    "slotIpAddressChangeData\0slotToolTipShowHide\0"
    "slotValueListEmpty"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    2,   77,    2, 0x0a /* Public */,
       4,    2,   82,    2, 0x0a /* Public */,
       6,    2,   87,    2, 0x0a /* Public */,
       8,    2,   92,    2, 0x0a /* Public */,
      10,    1,   97,    2, 0x0a /* Public */,
      11,    1,  100,    2, 0x0a /* Public */,
      13,    0,  103,    2, 0x0a /* Public */,
      14,    0,  104,    2, 0x0a /* Public */,
      15,    3,  105,    2, 0x0a /* Public */,
      16,    1,  112,    2, 0x0a /* Public */,
      17,    1,  115,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 5,    2,    2,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 9, QMetaType::UChar,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UChar,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,    2,    2,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::UChar,    2,

       0        // eod
};

void CameraSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigToolTipShowHide((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->slotSpinBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 2: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->slotOptionButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->slotCreateCMDRequest((*reinterpret_cast< SET_COMMAND_e(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 5: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotPopupPageDeleted((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 7: _t->slotcameraListUpdateTimerTimeout(); break;
        case 8: _t->slotIpAddressChangeDelete(); break;
        case 9: _t->slotIpAddressChangeData((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 10: _t->slotToolTipShowHide((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->slotValueListEmpty((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CameraSettings::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraSettings::sigToolTipShowHide)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CameraSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_CameraSettings.data,
    qt_meta_data_CameraSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CameraSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CameraSettings.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int CameraSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void CameraSettings::sigToolTipShowHide(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
