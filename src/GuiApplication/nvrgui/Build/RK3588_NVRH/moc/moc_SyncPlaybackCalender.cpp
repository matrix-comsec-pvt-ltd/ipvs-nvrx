/****************************************************************************
** Meta object code from reading C++ file 'SyncPlaybackCalender.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/SyncPlayback/SyncPlaybackCalender.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncPlaybackCalender.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SyncPlaybackCalender_t {
    QByteArrayData data[11];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SyncPlaybackCalender_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SyncPlaybackCalender_t qt_meta_stringdata_SyncPlaybackCalender = {
    {
QT_MOC_LITERAL(0, 0, 20), // "SyncPlaybackCalender"
QT_MOC_LITERAL(1, 21, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 5), // "index"
QT_MOC_LITERAL(4, 52, 22), // "sigFocusToOtherElement"
QT_MOC_LITERAL(5, 75, 17), // "isPrevoiusElement"
QT_MOC_LITERAL(6, 93, 23), // "sigFetchNewSelectedDate"
QT_MOC_LITERAL(7, 117, 24), // "sigFetchRecordForNewDate"
QT_MOC_LITERAL(8, 142, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(9, 167, 22), // "slotChangeCalenderDate"
QT_MOC_LITERAL(10, 190, 16) // "slotDateSelected"

    },
    "SyncPlaybackCalender\0sigUpdateCurrentElement\0"
    "\0index\0sigFocusToOtherElement\0"
    "isPrevoiusElement\0sigFetchNewSelectedDate\0"
    "sigFetchRecordForNewDate\0"
    "slotUpdateCurrentElement\0"
    "slotChangeCalenderDate\0slotDateSelected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SyncPlaybackCalender[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    1,   52,    2, 0x06 /* Public */,
       6,    0,   55,    2, 0x06 /* Public */,
       7,    0,   56,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   57,    2, 0x0a /* Public */,
       9,    1,   60,    2, 0x0a /* Public */,
      10,    1,   63,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::UInt,    3,

       0        // eod
};

void SyncPlaybackCalender::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SyncPlaybackCalender *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigFocusToOtherElement((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->sigFetchNewSelectedDate(); break;
        case 3: _t->sigFetchRecordForNewDate(); break;
        case 4: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotChangeCalenderDate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotDateSelected((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SyncPlaybackCalender::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackCalender::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SyncPlaybackCalender::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackCalender::sigFocusToOtherElement)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SyncPlaybackCalender::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackCalender::sigFetchNewSelectedDate)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SyncPlaybackCalender::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlaybackCalender::sigFetchRecordForNewDate)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SyncPlaybackCalender::staticMetaObject = { {
    QMetaObject::SuperData::link<LayoutWindowRectangle::staticMetaObject>(),
    qt_meta_stringdata_SyncPlaybackCalender.data,
    qt_meta_data_SyncPlaybackCalender,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SyncPlaybackCalender::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SyncPlaybackCalender::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SyncPlaybackCalender.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return LayoutWindowRectangle::qt_metacast(_clname);
}

int SyncPlaybackCalender::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LayoutWindowRectangle::qt_metacall(_c, _id, _a);
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
void SyncPlaybackCalender::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SyncPlaybackCalender::sigFocusToOtherElement(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SyncPlaybackCalender::sigFetchNewSelectedDate()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SyncPlaybackCalender::sigFetchRecordForNewDate()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
