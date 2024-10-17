/****************************************************************************
** Meta object code from reading C++ file 'AutoAddCameraList.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/AutoAddCameraList.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AutoAddCameraList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AutoAddCameraList_t {
    QByteArrayData data[9];
    char stringdata0[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AutoAddCameraList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AutoAddCameraList_t qt_meta_stringdata_AutoAddCameraList = {
    {
QT_MOC_LITERAL(0, 0, 17), // "AutoAddCameraList"
QT_MOC_LITERAL(1, 18, 15), // "sigObjectDelete"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 15), // "slotButtonClick"
QT_MOC_LITERAL(4, 51, 25), // "slotOptionSelectionButton"
QT_MOC_LITERAL(5, 77, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(6, 97, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(7, 118, 5), // "index"
QT_MOC_LITERAL(8, 124, 24) // "slotUpdateCurrentElement"

    },
    "AutoAddCameraList\0sigObjectDelete\0\0"
    "slotButtonClick\0slotOptionSelectionButton\0"
    "OPTION_STATE_TYPE_e\0slotInfoPageBtnclick\0"
    "index\0slotUpdateCurrentElement"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AutoAddCameraList[] = {

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
       1,    1,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   42,    2, 0x0a /* Public */,
       4,    2,   45,    2, 0x0a /* Public */,
       6,    1,   50,    2, 0x0a /* Public */,
       8,    1,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void AutoAddCameraList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AutoAddCameraList *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigObjectDelete((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotOptionSelectionButton((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AutoAddCameraList::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AutoAddCameraList::sigObjectDelete)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AutoAddCameraList::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_AutoAddCameraList.data,
    qt_meta_data_AutoAddCameraList,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AutoAddCameraList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AutoAddCameraList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AutoAddCameraList.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int AutoAddCameraList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void AutoAddCameraList::sigObjectDelete(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
