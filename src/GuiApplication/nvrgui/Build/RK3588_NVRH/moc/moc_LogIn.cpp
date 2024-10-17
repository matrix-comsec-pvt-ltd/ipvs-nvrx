/****************************************************************************
** Meta object code from reading C++ file 'LogIn.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../LogIn.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogIn.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LogIn_t {
    QByteArrayData data[14];
    char stringdata0[208];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LogIn_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LogIn_t qt_meta_stringdata_LogIn = {
    {
QT_MOC_LITERAL(0, 0, 5), // "LogIn"
QT_MOC_LITERAL(1, 6, 27), // "sigChangeToolbarButtonState"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 57, 11), // "buttonIndex"
QT_MOC_LITERAL(5, 69, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(6, 82, 5), // "state"
QT_MOC_LITERAL(7, 88, 12), // "slotExitPage"
QT_MOC_LITERAL(8, 101, 15), // "slotButtonClick"
QT_MOC_LITERAL(9, 117, 20), // "slotInfoPageBtnclick"
QT_MOC_LITERAL(10, 138, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(11, 163, 15), // "slotTextClicked"
QT_MOC_LITERAL(12, 179, 18), // "slotTextLableHover"
QT_MOC_LITERAL(13, 198, 9) // "isHoverIn"

    },
    "LogIn\0sigChangeToolbarButtonState\0\0"
    "TOOLBAR_BUTTON_TYPE_e\0buttonIndex\0"
    "STATE_TYPE_e\0state\0slotExitPage\0"
    "slotButtonClick\0slotInfoPageBtnclick\0"
    "slotUpdateCurrentElement\0slotTextClicked\0"
    "slotTextLableHover\0isHoverIn"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LogIn[] = {

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
       1,    2,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   54,    2, 0x0a /* Public */,
       8,    1,   55,    2, 0x0a /* Public */,
       9,    1,   58,    2, 0x0a /* Public */,
      10,    1,   61,    2, 0x0a /* Public */,
      11,    1,   64,    2, 0x0a /* Public */,
      12,    2,   67,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    2,   13,

       0        // eod
};

void LogIn::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogIn *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigChangeToolbarButtonState((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 1: _t->slotExitPage(); break;
        case 2: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotInfoPageBtnclick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotTextClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotTextLableHover((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LogIn::*)(TOOLBAR_BUTTON_TYPE_e , STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LogIn::sigChangeToolbarButtonState)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LogIn::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_LogIn.data,
    qt_meta_data_LogIn,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LogIn::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogIn::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogIn.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int LogIn::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
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
void LogIn::sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e _t1, STATE_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
