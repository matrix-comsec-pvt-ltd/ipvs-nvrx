/****************************************************************************
** Meta object code from reading C++ file 'AutoAddCamera.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/BasicSettings/AutoAddCamera.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AutoAddCamera.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AutoAddCamera_t {
    QByteArrayData data[10];
    char stringdata0[147];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AutoAddCamera_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AutoAddCamera_t qt_meta_stringdata_AutoAddCamera = {
    {
QT_MOC_LITERAL(0, 0, 13), // "AutoAddCamera"
QT_MOC_LITERAL(1, 14, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(4, 52, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(5, 77, 5), // "index"
QT_MOC_LITERAL(6, 83, 15), // "slotButtonClick"
QT_MOC_LITERAL(7, 99, 23), // "slotTextBoxLoadInfopage"
QT_MOC_LITERAL(8, 123, 15), // "INFO_MSG_TYPE_e"
QT_MOC_LITERAL(9, 139, 7) // "msgType"

    },
    "AutoAddCamera\0sigObjectDelete\0\0"
    "slotInfoPageBtnclick\0slotUpdateCurrentElement\0"
    "index\0slotButtonClick\0slotTextBoxLoadInfopage\0"
    "INFO_MSG_TYPE_e\0msgType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AutoAddCamera[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   40,    2, 0x0a /* Public */,
       4,    1,   43,    2, 0x0a /* Public */,
       6,    1,   46,    2, 0x0a /* Public */,
       7,    2,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    5,    9,

       0        // eod
};

void AutoAddCamera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AutoAddCamera *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigObjectDelete(); break;
        case 1: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotTextBoxLoadInfopage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< INFO_MSG_TYPE_e(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AutoAddCamera::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AutoAddCamera::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AutoAddCamera::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_AutoAddCamera.data,
    qt_meta_data_AutoAddCamera,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AutoAddCamera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AutoAddCamera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AutoAddCamera.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int AutoAddCamera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void AutoAddCamera::sigObjectDelete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
