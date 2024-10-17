/****************************************************************************
** Meta object code from reading C++ file 'SetSchedule.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/SetSchedule.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SetSchedule.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SetSchedule_t {
    QByteArrayData data[10];
    char stringdata0[168];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SetSchedule_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SetSchedule_t qt_meta_stringdata_SetSchedule = {
    {
QT_MOC_LITERAL(0, 0, 11), // "SetSchedule"
QT_MOC_LITERAL(1, 12, 15), // "sigDeleteObject"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 20), // "slotCloseButtonClick"
QT_MOC_LITERAL(4, 50, 24), // "slotInfoPageCnfgBtnClick"
QT_MOC_LITERAL(5, 75, 17), // "slotOkButtonClick"
QT_MOC_LITERAL(6, 93, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(7, 118, 5), // "index"
QT_MOC_LITERAL(8, 124, 23), // "slotOptionButtonClicked"
QT_MOC_LITERAL(9, 148, 19) // "OPTION_STATE_TYPE_e"

    },
    "SetSchedule\0sigDeleteObject\0\0"
    "slotCloseButtonClick\0slotInfoPageCnfgBtnClick\0"
    "slotOkButtonClick\0slotUpdateCurrentElement\0"
    "index\0slotOptionButtonClicked\0"
    "OPTION_STATE_TYPE_e"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SetSchedule[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   45,    2, 0x0a /* Public */,
       4,    1,   48,    2, 0x0a /* Public */,
       5,    1,   51,    2, 0x0a /* Public */,
       6,    1,   54,    2, 0x0a /* Public */,
       8,    2,   57,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, 0x80000000 | 9, QMetaType::Int,    2,    2,

       0        // eod
};

void SetSchedule::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SetSchedule *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigDeleteObject(); break;
        case 1: _t->slotCloseButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotOkButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotOptionButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SetSchedule::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SetSchedule::sigDeleteObject)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SetSchedule::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_SetSchedule.data,
    qt_meta_data_SetSchedule,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SetSchedule::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SetSchedule::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SetSchedule.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int SetSchedule::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void SetSchedule::sigDeleteObject()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
