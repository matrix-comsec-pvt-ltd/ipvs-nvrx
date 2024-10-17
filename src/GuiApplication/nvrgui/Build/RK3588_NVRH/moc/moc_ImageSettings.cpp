/****************************************************************************
** Meta object code from reading C++ file 'ImageSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/ImageSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ImageSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ImageSettings_t {
    QByteArrayData data[11];
    char stringdata0[156];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ImageSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ImageSettings_t qt_meta_stringdata_ImageSettings = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ImageSettings"
QT_MOC_LITERAL(1, 14, 22), // "slotSpinboxValueChange"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 16), // "slotValueChanged"
QT_MOC_LITERAL(4, 55, 12), // "changedValue"
QT_MOC_LITERAL(5, 68, 11), // "indexInPage"
QT_MOC_LITERAL(6, 80, 24), // "slotSliderBoxValueChange"
QT_MOC_LITERAL(7, 105, 3), // "str"
QT_MOC_LITERAL(8, 109, 20), // "slotPageOpenBtnClick"
QT_MOC_LITERAL(9, 130, 5), // "index"
QT_MOC_LITERAL(10, 136, 19) // "slotCopytoCamDelete"

    },
    "ImageSettings\0slotSpinboxValueChange\0"
    "\0slotValueChanged\0changedValue\0"
    "indexInPage\0slotSliderBoxValueChange\0"
    "str\0slotPageOpenBtnClick\0index\0"
    "slotCopytoCamDelete"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ImageSettings[] = {

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
       3,    3,   44,    2, 0x0a /* Public */,
       6,    2,   51,    2, 0x0a /* Public */,
       8,    1,   56,    2, 0x0a /* Public */,
      10,    1,   59,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Bool,    4,    5,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    7,    5,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::UChar,    2,

       0        // eod
};

void ImageSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ImageSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotSpinboxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 1: _t->slotValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 2: _t->slotSliderBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->slotPageOpenBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotCopytoCamDelete((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ImageSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_ImageSettings.data,
    qt_meta_data_ImageSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ImageSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImageSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ImageSettings.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int ImageSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
