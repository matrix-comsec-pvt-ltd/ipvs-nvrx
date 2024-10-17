/****************************************************************************
** Meta object code from reading C++ file 'AppearanceSetting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/AppearanceSetting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppearanceSetting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AppearanceSetting_t {
    QByteArrayData data[13];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AppearanceSetting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AppearanceSetting_t qt_meta_stringdata_AppearanceSetting = {
    {
QT_MOC_LITERAL(0, 0, 17), // "AppearanceSetting"
QT_MOC_LITERAL(1, 18, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 15), // "slotButtonClick"
QT_MOC_LITERAL(4, 51, 5), // "index"
QT_MOC_LITERAL(5, 57, 16), // "slotValueChanged"
QT_MOC_LITERAL(6, 74, 12), // "changedValue"
QT_MOC_LITERAL(7, 87, 11), // "indexInPage"
QT_MOC_LITERAL(8, 99, 10), // "sliderMove"
QT_MOC_LITERAL(9, 110, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(10, 135, 23), // "slotSpinBoxValueChanged"
QT_MOC_LITERAL(11, 159, 22), // "slotTextBoxValueChange"
QT_MOC_LITERAL(12, 182, 24) // "slotInfoPageCnfgBtnClick"

    },
    "AppearanceSetting\0sigObjectDelete\0\0"
    "slotButtonClick\0index\0slotValueChanged\0"
    "changedValue\0indexInPage\0sliderMove\0"
    "slotUpdateCurrentElement\0"
    "slotSpinBoxValueChanged\0slotTextBoxValueChange\0"
    "slotInfoPageCnfgBtnClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AppearanceSetting[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   50,    2, 0x0a /* Public */,
       5,    3,   53,    2, 0x0a /* Public */,
       9,    1,   60,    2, 0x0a /* Public */,
      10,    2,   63,    2, 0x0a /* Public */,
      11,    2,   68,    2, 0x0a /* Public */,
      12,    1,   73,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Bool,    6,    7,    8,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    7,
    QMetaType::Void, QMetaType::Int,    4,

       0        // eod
};

void AppearanceSetting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AppearanceSetting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigObjectDelete(); break;
        case 1: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotValueChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 3: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotSpinBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 5: _t->slotTextBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AppearanceSetting::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppearanceSetting::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AppearanceSetting::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_AppearanceSetting.data,
    qt_meta_data_AppearanceSetting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AppearanceSetting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppearanceSetting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AppearanceSetting.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int AppearanceSetting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void AppearanceSetting::sigObjectDelete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
