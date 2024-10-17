/****************************************************************************
** Meta object code from reading C++ file 'CalendarTile.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/CalendarTile.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CalendarTile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CalendarTile_t {
    QByteArrayData data[10];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CalendarTile_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CalendarTile_t qt_meta_stringdata_CalendarTile = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CalendarTile"
QT_MOC_LITERAL(1, 13, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 5), // "index"
QT_MOC_LITERAL(4, 44, 14), // "sigDateChanged"
QT_MOC_LITERAL(5, 59, 14), // "slotUpdateDate"
QT_MOC_LITERAL(6, 74, 15), // "DDMMYY_PARAM_t*"
QT_MOC_LITERAL(7, 90, 5), // "param"
QT_MOC_LITERAL(8, 96, 19), // "slotDestroyCalendar"
QT_MOC_LITERAL(9, 116, 31) // "slotDestroyCalendarOnOuterClick"

    },
    "CalendarTile\0sigUpdateCurrentElement\0"
    "\0index\0sigDateChanged\0slotUpdateDate\0"
    "DDMMYY_PARAM_t*\0param\0slotDestroyCalendar\0"
    "slotDestroyCalendarOnOuterClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CalendarTile[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    0,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   43,    2, 0x0a /* Public */,
       8,    0,   46,    2, 0x0a /* Public */,
       9,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CalendarTile::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CalendarTile *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigDateChanged(); break;
        case 2: _t->slotUpdateDate((*reinterpret_cast< DDMMYY_PARAM_t*(*)>(_a[1]))); break;
        case 3: _t->slotDestroyCalendar(); break;
        case 4: _t->slotDestroyCalendarOnOuterClick(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CalendarTile::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CalendarTile::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CalendarTile::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CalendarTile::sigDateChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CalendarTile::staticMetaObject = { {
    QMetaObject::SuperData::link<BgTile::staticMetaObject>(),
    qt_meta_stringdata_CalendarTile.data,
    qt_meta_data_CalendarTile,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CalendarTile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CalendarTile::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CalendarTile.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return BgTile::qt_metacast(_clname);
}

int CalendarTile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BgTile::qt_metacall(_c, _id, _a);
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
void CalendarTile::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CalendarTile::sigDateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
