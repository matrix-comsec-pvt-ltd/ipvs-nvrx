/****************************************************************************
** Meta object code from reading C++ file 'EventLogSearch.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../EventLogSearch.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EventLogSearch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_EventLogSearch_t {
    QByteArrayData data[11];
    char stringdata0[173];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EventLogSearch_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EventLogSearch_t qt_meta_stringdata_EventLogSearch = {
    {
QT_MOC_LITERAL(0, 0, 14), // "EventLogSearch"
QT_MOC_LITERAL(1, 15, 23), // "sigPlaybackPlayBtnClick"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 18), // "PlaybackRecordData"
QT_MOC_LITERAL(4, 59, 7), // "recData"
QT_MOC_LITERAL(5, 67, 7), // "devName"
QT_MOC_LITERAL(6, 75, 6), // "status"
QT_MOC_LITERAL(7, 82, 25), // "slotUpadateCurrentElement"
QT_MOC_LITERAL(8, 108, 15), // "slotButtonClick"
QT_MOC_LITERAL(9, 124, 24), // "slotInfoPageCnfgBtnClick"
QT_MOC_LITERAL(10, 149, 23) // "slotEventScrollbarClick"

    },
    "EventLogSearch\0sigPlaybackPlayBtnClick\0"
    "\0PlaybackRecordData\0recData\0devName\0"
    "status\0slotUpadateCurrentElement\0"
    "slotButtonClick\0slotInfoPageCnfgBtnClick\0"
    "slotEventScrollbarClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EventLogSearch[] = {

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
       1,    3,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   46,    2, 0x0a /* Public */,
       8,    1,   49,    2, 0x0a /* Public */,
       9,    1,   52,    2, 0x0a /* Public */,
      10,    1,   55,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString, QMetaType::Bool,    4,    5,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void EventLogSearch::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EventLogSearch *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigPlaybackPlayBtnClick((*reinterpret_cast< PlaybackRecordData(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 1: _t->slotUpadateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotEventScrollbarClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EventLogSearch::*)(PlaybackRecordData , QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EventLogSearch::sigPlaybackPlayBtnClick)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject EventLogSearch::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_EventLogSearch.data,
    qt_meta_data_EventLogSearch,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *EventLogSearch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EventLogSearch::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EventLogSearch.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int EventLogSearch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
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
void EventLogSearch::sigPlaybackPlayBtnClick(PlaybackRecordData _t1, QString _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
