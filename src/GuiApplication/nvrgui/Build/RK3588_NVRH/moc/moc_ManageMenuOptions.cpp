/****************************************************************************
** Meta object code from reading C++ file 'ManageMenuOptions.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ManagePages/ManageMenuOptions.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ManageMenuOptions.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ManageMenuOptions_t {
    QByteArrayData data[10];
    char stringdata0[158];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ManageMenuOptions_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ManageMenuOptions_t qt_meta_stringdata_ManageMenuOptions = {
    {
QT_MOC_LITERAL(0, 0, 17), // "ManageMenuOptions"
QT_MOC_LITERAL(1, 18, 19), // "sigSubHeadingChange"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 21), // "sigLanguageCfgChanged"
QT_MOC_LITERAL(4, 61, 3), // "str"
QT_MOC_LITERAL(5, 65, 22), // "sigFocusToOtherElement"
QT_MOC_LITERAL(6, 88, 17), // "isPrevoiusElement"
QT_MOC_LITERAL(7, 106, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(8, 127, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(9, 152, 5) // "index"

    },
    "ManageMenuOptions\0sigSubHeadingChange\0"
    "\0sigLanguageCfgChanged\0str\0"
    "sigFocusToOtherElement\0isPrevoiusElement\0"
    "slotInfoPageBtnclick\0slotUpdateCurrentElement\0"
    "index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ManageMenuOptions[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       3,    1,   42,    2, 0x06 /* Public */,
       5,    1,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   48,    2, 0x0a /* Public */,
       8,    1,   51,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::Bool,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    9,

       0        // eod
};

void ManageMenuOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ManageMenuOptions *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigSubHeadingChange((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->sigLanguageCfgChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->sigFocusToOtherElement((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ManageMenuOptions::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ManageMenuOptions::sigSubHeadingChange)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ManageMenuOptions::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ManageMenuOptions::sigLanguageCfgChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ManageMenuOptions::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ManageMenuOptions::sigFocusToOtherElement)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ManageMenuOptions::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_ManageMenuOptions.data,
    qt_meta_data_ManageMenuOptions,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ManageMenuOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ManageMenuOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ManageMenuOptions.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int ManageMenuOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void ManageMenuOptions::sigSubHeadingChange(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ManageMenuOptions::sigLanguageCfgChanged(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ManageMenuOptions::sigFocusToOtherElement(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
