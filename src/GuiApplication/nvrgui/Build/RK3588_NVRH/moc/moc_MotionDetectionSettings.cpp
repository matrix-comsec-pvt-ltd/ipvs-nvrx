/****************************************************************************
** Meta object code from reading C++ file 'MotionDetectionSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/MotionDetectionSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MotionDetectionSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MotionDetectionSettings_t {
    QByteArrayData data[12];
    char stringdata0[171];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MotionDetectionSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MotionDetectionSettings_t qt_meta_stringdata_MotionDetectionSettings = {
    {
QT_MOC_LITERAL(0, 0, 23), // "MotionDetectionSettings"
QT_MOC_LITERAL(1, 24, 22), // "slotMenuButtonSelected"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 9), // "menuLabel"
QT_MOC_LITERAL(4, 58, 9), // "menuIndex"
QT_MOC_LITERAL(5, 68, 21), // "slotMenuListDestroyed"
QT_MOC_LITERAL(6, 90, 25), // "slotInfoPageButtonClicked"
QT_MOC_LITERAL(7, 116, 5), // "index"
QT_MOC_LITERAL(8, 122, 16), // "slotValueChanged"
QT_MOC_LITERAL(9, 139, 3), // "key"
QT_MOC_LITERAL(10, 143, 5), // "value"
QT_MOC_LITERAL(11, 149, 21) // "slotPickListDestroyed"

    },
    "MotionDetectionSettings\0slotMenuButtonSelected\0"
    "\0menuLabel\0menuIndex\0slotMenuListDestroyed\0"
    "slotInfoPageButtonClicked\0index\0"
    "slotValueChanged\0key\0value\0"
    "slotPickListDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MotionDetectionSettings[] = {

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
       5,    0,   44,    2, 0x0a /* Public */,
       6,    1,   45,    2, 0x0a /* Public */,
       8,    3,   48,    2, 0x0a /* Public */,
      11,    0,   55,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UChar,    3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Bool,    9,   10,    2,
    QMetaType::Void,

       0        // eod
};

void MotionDetectionSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MotionDetectionSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotMenuButtonSelected((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 1: _t->slotMenuListDestroyed(); break;
        case 2: _t->slotInfoPageButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 4: _t->slotPickListDestroyed(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MotionDetectionSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MotionDetectionSettings.data,
    qt_meta_data_MotionDetectionSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MotionDetectionSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MotionDetectionSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MotionDetectionSettings.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MotionDetectionSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
