/****************************************************************************
** Meta object code from reading C++ file 'SyncBufferFeeder.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/StreamRequest/SyncPbMedia/SyncBufferFeeder.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncBufferFeeder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SyncBufferFeeder_t {
    QByteArrayData data[10];
    char stringdata0[119];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SyncBufferFeeder_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SyncBufferFeeder_t qt_meta_stringdata_SyncBufferFeeder = {
    {
QT_MOC_LITERAL(0, 0, 16), // "SyncBufferFeeder"
QT_MOC_LITERAL(1, 17, 17), // "sigFeederResponse"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(4, 56, 8), // "statusId"
QT_MOC_LITERAL(5, 65, 7), // "payload"
QT_MOC_LITERAL(6, 73, 8), // "feederId"
QT_MOC_LITERAL(7, 82, 16), // "sigSetLowestTime"
QT_MOC_LITERAL(8, 99, 9), // "feedFrame"
QT_MOC_LITERAL(9, 109, 9) // "frameTime"

    },
    "SyncBufferFeeder\0sigFeederResponse\0\0"
    "DEVICE_REPLY_TYPE_e\0statusId\0payload\0"
    "feederId\0sigSetLowestTime\0feedFrame\0"
    "frameTime"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SyncBufferFeeder[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   24,    2, 0x06 /* Public */,
       7,    3,   31,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::UInt, QMetaType::UChar,    4,    5,    6,
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool, QMetaType::ULongLong,    6,    8,    9,

       0        // eod
};

void SyncBufferFeeder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SyncBufferFeeder *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigFeederResponse((*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3]))); break;
        case 1: _t->sigSetLowestTime((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< quint64(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SyncBufferFeeder::*)(DEVICE_REPLY_TYPE_e , quint32 , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncBufferFeeder::sigFeederResponse)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SyncBufferFeeder::*)(quint8 , bool , quint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncBufferFeeder::sigSetLowestTime)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SyncBufferFeeder::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_SyncBufferFeeder.data,
    qt_meta_data_SyncBufferFeeder,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SyncBufferFeeder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SyncBufferFeeder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SyncBufferFeeder.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int SyncBufferFeeder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void SyncBufferFeeder::sigFeederResponse(DEVICE_REPLY_TYPE_e _t1, quint32 _t2, quint8 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SyncBufferFeeder::sigSetLowestTime(quint8 _t1, bool _t2, quint64 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
