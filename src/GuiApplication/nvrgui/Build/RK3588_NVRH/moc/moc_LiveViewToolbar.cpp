/****************************************************************************
** Meta object code from reading C++ file 'LiveViewToolbar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/LiveViewToolbar/LiveViewToolbar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LiveViewToolbar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LiveViewToolbar_t {
    QByteArrayData data[14];
    char stringdata0[231];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LiveViewToolbar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LiveViewToolbar_t qt_meta_stringdata_LiveViewToolbar = {
    {
QT_MOC_LITERAL(0, 0, 15), // "LiveViewToolbar"
QT_MOC_LITERAL(1, 16, 23), // "sigToolbarButtonClicked"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 30), // "LIVEVIEW_TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 72, 5), // "index"
QT_MOC_LITERAL(5, 78, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(6, 91, 5), // "state"
QT_MOC_LITERAL(7, 97, 11), // "windowIndex"
QT_MOC_LITERAL(8, 109, 23), // "sigCloseLiveViewToolbar"
QT_MOC_LITERAL(9, 133, 17), // "slotButtonClicked"
QT_MOC_LITERAL(10, 151, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(11, 176, 19), // "slotShowHideTooltip"
QT_MOC_LITERAL(12, 196, 13), // "toShowTooltip"
QT_MOC_LITERAL(13, 210, 20) // "slotKeepAliveTimeOut"

    },
    "LiveViewToolbar\0sigToolbarButtonClicked\0"
    "\0LIVEVIEW_TOOLBAR_BUTTON_TYPE_e\0index\0"
    "STATE_TYPE_e\0state\0windowIndex\0"
    "sigCloseLiveViewToolbar\0slotButtonClicked\0"
    "slotUpdateCurrentElement\0slotShowHideTooltip\0"
    "toShowTooltip\0slotKeepAliveTimeOut"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LiveViewToolbar[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   44,    2, 0x06 /* Public */,
       8,    0,   51,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    2,   52,    2, 0x0a /* Public */,
      10,    1,   57,    2, 0x0a /* Public */,
      11,    2,   60,    2, 0x0a /* Public */,
      13,    0,   65,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::UShort,    4,    6,    7,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    4,   12,
    QMetaType::Void,

       0        // eod
};

void LiveViewToolbar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LiveViewToolbar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigToolbarButtonClicked((*reinterpret_cast< LIVEVIEW_TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        case 1: _t->sigCloseLiveViewToolbar(); break;
        case 2: _t->slotButtonClicked((*reinterpret_cast< LIVEVIEW_TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotShowHideTooltip((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->slotKeepAliveTimeOut(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LiveViewToolbar::*)(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e , STATE_TYPE_e , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LiveViewToolbar::sigToolbarButtonClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LiveViewToolbar::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LiveViewToolbar::sigCloseLiveViewToolbar)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LiveViewToolbar::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_LiveViewToolbar.data,
    qt_meta_data_LiveViewToolbar,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LiveViewToolbar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LiveViewToolbar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LiveViewToolbar.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int LiveViewToolbar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void LiveViewToolbar::sigToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e _t1, STATE_TYPE_e _t2, quint16 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LiveViewToolbar::sigCloseLiveViewToolbar()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
