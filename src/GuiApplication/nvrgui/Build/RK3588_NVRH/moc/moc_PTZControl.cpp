/****************************************************************************
** Meta object code from reading C++ file 'PTZControl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/PTZControl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PTZControl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PTZControl_t {
    QByteArrayData data[8];
    char stringdata0[135];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PTZControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PTZControl_t qt_meta_stringdata_PTZControl = {
    {
QT_MOC_LITERAL(0, 0, 10), // "PTZControl"
QT_MOC_LITERAL(1, 11, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 17), // "slotButtonClicked"
QT_MOC_LITERAL(4, 46, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(5, 67, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(6, 92, 19), // "slotImageMouseHover"
QT_MOC_LITERAL(7, 112, 22) // "slotSpinBoxValueChange"

    },
    "PTZControl\0sigObjectDelete\0\0"
    "slotButtonClicked\0slotInfoPageBtnclick\0"
    "slotUpdateCurrentElement\0slotImageMouseHover\0"
    "slotSpinBoxValueChange"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PTZControl[] = {

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
       6,    2,   54,    2, 0x0a /* Public */,
       7,    2,   59,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,

       0        // eod
};

void PTZControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PTZControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigObjectDelete(); break;
        case 1: _t->slotButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotImageMouseHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->slotSpinBoxValueChange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PTZControl::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PTZControl::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PTZControl::staticMetaObject = { {
    QMetaObject::SuperData::link<Rectangle::staticMetaObject>(),
    qt_meta_stringdata_PTZControl.data,
    qt_meta_data_PTZControl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PTZControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PTZControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PTZControl.stringdata0))
        return static_cast<void*>(this);
    return Rectangle::qt_metacast(_clname);
}

int PTZControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Rectangle::qt_metacall(_c, _id, _a);
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
void PTZControl::sigObjectDelete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
