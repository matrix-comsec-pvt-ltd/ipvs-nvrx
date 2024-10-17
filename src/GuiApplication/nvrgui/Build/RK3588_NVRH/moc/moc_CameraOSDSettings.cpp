/****************************************************************************
** Meta object code from reading C++ file 'CameraOSDSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/CameraOSDSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraOSDSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CameraOSDSettings_t {
    QByteArrayData data[15];
    char stringdata0[270];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraOSDSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraOSDSettings_t qt_meta_stringdata_CameraOSDSettings = {
    {
QT_MOC_LITERAL(0, 0, 17), // "CameraOSDSettings"
QT_MOC_LITERAL(1, 18, 15), // "sigDeleteObject"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 11), // "indexInPage"
QT_MOC_LITERAL(4, 47, 24), // "slotOptionButtonSelected"
QT_MOC_LITERAL(5, 72, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(6, 92, 12), // "currentState"
QT_MOC_LITERAL(7, 105, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(8, 130, 23), // "slotConfigButtonClicked"
QT_MOC_LITERAL(9, 154, 22), // "slotCloseButtonClicked"
QT_MOC_LITERAL(10, 177, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(11, 198, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(12, 222, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(13, 238, 7), // "msgType"
QT_MOC_LITERAL(14, 246, 23) // "slotSpinBoxValueChanged"

    },
    "CameraOSDSettings\0sigDeleteObject\0\0"
    "indexInPage\0slotOptionButtonSelected\0"
    "OPTION_STATE_TYPE_e\0currentState\0"
    "slotUpdateCurrentElement\0"
    "slotConfigButtonClicked\0slotCloseButtonClicked\0"
    "slotInfoPageBtnclick\0slotTextBoxLoadInfopage\0"
    "INFO_MSG_TYPE_e\0msgType\0slotSpinBoxValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraOSDSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    2,   57,    2, 0x0a /* Public */,
       7,    1,   62,    2, 0x0a /* Public */,
       8,    1,   65,    2, 0x0a /* Public */,
       9,    1,   68,    2, 0x0a /* Public */,
      10,    1,   71,    2, 0x0a /* Public */,
      11,    2,   74,    2, 0x0a /* Public */,
      14,    2,   79,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::UChar,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5, QMetaType::Int,    6,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 12,    3,   13,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,

       0        // eod
};

void CameraOSDSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraOSDSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigDeleteObject((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 1: _t->slotOptionButtonSelected((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotConfigButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotCloseButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        case 7: _t->slotSpinBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CameraOSDSettings::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraOSDSettings::sigDeleteObject)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CameraOSDSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_CameraOSDSettings.data,
    qt_meta_data_CameraOSDSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CameraOSDSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraOSDSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CameraOSDSettings.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int CameraOSDSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void CameraOSDSettings::sigDeleteObject(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
