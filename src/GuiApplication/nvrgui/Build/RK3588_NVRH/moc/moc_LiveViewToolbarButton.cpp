/****************************************************************************
** Meta object code from reading C++ file 'LiveViewToolbarButton.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/LiveViewToolbar/LiveViewToolbarButton.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LiveViewToolbarButton.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LiveViewToolbarButton_t {
    QByteArrayData data[11];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LiveViewToolbarButton_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LiveViewToolbarButton_t qt_meta_stringdata_LiveViewToolbarButton = {
    {
QT_MOC_LITERAL(0, 0, 21), // "LiveViewToolbarButton"
QT_MOC_LITERAL(1, 22, 16), // "sigButtonClicked"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 30), // "LIVEVIEW_TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 71, 5), // "index"
QT_MOC_LITERAL(5, 77, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(6, 90, 5), // "state"
QT_MOC_LITERAL(7, 96, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(8, 120, 18), // "sigShowHideToolTip"
QT_MOC_LITERAL(9, 139, 13), // "toShowTooltip"
QT_MOC_LITERAL(10, 153, 22) // "slotClickEffectTimeOut"

    },
    "LiveViewToolbarButton\0sigButtonClicked\0"
    "\0LIVEVIEW_TOOLBAR_BUTTON_TYPE_e\0index\0"
    "STATE_TYPE_e\0state\0sigUpdateCurrentElement\0"
    "sigShowHideToolTip\0toShowTooltip\0"
    "slotClickEffectTimeOut"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LiveViewToolbarButton[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,
       7,    1,   39,    2, 0x06 /* Public */,
       8,    2,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    4,    9,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void LiveViewToolbarButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LiveViewToolbarButton *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigButtonClicked((*reinterpret_cast< LIVEVIEW_TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 1: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sigShowHideToolTip((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->slotClickEffectTimeOut(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LiveViewToolbarButton::*)(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e , STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LiveViewToolbarButton::sigButtonClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LiveViewToolbarButton::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LiveViewToolbarButton::sigUpdateCurrentElement)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LiveViewToolbarButton::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LiveViewToolbarButton::sigShowHideToolTip)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LiveViewToolbarButton::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_LiveViewToolbarButton.data,
    qt_meta_data_LiveViewToolbarButton,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LiveViewToolbarButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LiveViewToolbarButton::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LiveViewToolbarButton.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int LiveViewToolbarButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void LiveViewToolbarButton::sigButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e _t1, STATE_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LiveViewToolbarButton::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LiveViewToolbarButton::sigShowHideToolTip(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
