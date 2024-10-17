/****************************************************************************
** Meta object code from reading C++ file 'DisplayMode.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DisplayMode.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DisplayMode.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DisplayMode_t {
    QByteArrayData data[16];
    char stringdata0[222];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DisplayMode_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DisplayMode_t qt_meta_stringdata_DisplayMode = {
    {
QT_MOC_LITERAL(0, 0, 11), // "DisplayMode"
QT_MOC_LITERAL(1, 12, 12), // "sigClosePage"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 48, 11), // "buttonIndex"
QT_MOC_LITERAL(5, 60, 17), // "sigApplyNewLayout"
QT_MOC_LITERAL(6, 78, 14), // "DISPLAY_TYPE_e"
QT_MOC_LITERAL(7, 93, 11), // "displayType"
QT_MOC_LITERAL(8, 105, 16), // "DISPLAY_CONFIG_t"
QT_MOC_LITERAL(9, 122, 13), // "displayConfig"
QT_MOC_LITERAL(10, 136, 12), // "STYLE_TYPE_e"
QT_MOC_LITERAL(11, 149, 7), // "styleNo"
QT_MOC_LITERAL(12, 157, 27), // "sigToolbarStyleChnageNotify"
QT_MOC_LITERAL(13, 185, 16), // "slotChangeLayout"
QT_MOC_LITERAL(14, 202, 13), // "LAYOUT_TYPE_e"
QT_MOC_LITERAL(15, 216, 5) // "index"

    },
    "DisplayMode\0sigClosePage\0\0"
    "TOOLBAR_BUTTON_TYPE_e\0buttonIndex\0"
    "sigApplyNewLayout\0DISPLAY_TYPE_e\0"
    "displayType\0DISPLAY_CONFIG_t\0displayConfig\0"
    "STYLE_TYPE_e\0styleNo\0sigToolbarStyleChnageNotify\0"
    "slotChangeLayout\0LAYOUT_TYPE_e\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DisplayMode[] = {

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
       1,    1,   34,    2, 0x06 /* Public */,
       5,    3,   37,    2, 0x06 /* Public */,
      12,    1,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    1,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 8, 0x80000000 | 10,    7,    9,   11,
    QMetaType::Void, 0x80000000 | 10,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 14,   15,

       0        // eod
};

void DisplayMode::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DisplayMode *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigClosePage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 1: _t->sigApplyNewLayout((*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< DISPLAY_CONFIG_t(*)>(_a[2])),(*reinterpret_cast< STYLE_TYPE_e(*)>(_a[3]))); break;
        case 2: _t->sigToolbarStyleChnageNotify((*reinterpret_cast< STYLE_TYPE_e(*)>(_a[1]))); break;
        case 3: _t->slotChangeLayout((*reinterpret_cast< LAYOUT_TYPE_e(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DisplayMode::*)(TOOLBAR_BUTTON_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplayMode::sigClosePage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DisplayMode::*)(DISPLAY_TYPE_e , DISPLAY_CONFIG_t , STYLE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplayMode::sigApplyNewLayout)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DisplayMode::*)(STYLE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplayMode::sigToolbarStyleChnageNotify)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DisplayMode::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_DisplayMode.data,
    qt_meta_data_DisplayMode,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DisplayMode::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisplayMode::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DisplayMode.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int DisplayMode::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DisplayMode::sigClosePage(TOOLBAR_BUTTON_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DisplayMode::sigApplyNewLayout(DISPLAY_TYPE_e _t1, DISPLAY_CONFIG_t _t2, STYLE_TYPE_e _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DisplayMode::sigToolbarStyleChnageNotify(STYLE_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
