/****************************************************************************
** Meta object code from reading C++ file 'PlaybackMedia.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/StreamRequest/PlaybackMedia/PlaybackMedia.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlaybackMedia.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlaybackMedia_t {
    QByteArrayData data[15];
    char stringdata0[200];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlaybackMedia_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlaybackMedia_t qt_meta_stringdata_PlaybackMedia = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PlaybackMedia"
QT_MOC_LITERAL(1, 14, 19), // "slotBufferThreshold"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 18), // "BUFFER_THRESHOLD_e"
QT_MOC_LITERAL(4, 54, 9), // "threshold"
QT_MOC_LITERAL(5, 64, 20), // "slotThrottleResponse"
QT_MOC_LITERAL(6, 85, 12), // "REQ_MSG_ID_e"
QT_MOC_LITERAL(7, 98, 9), // "requestId"
QT_MOC_LITERAL(8, 108, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(9, 122, 9), // "commandId"
QT_MOC_LITERAL(10, 132, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(11, 152, 9), // "tStatusId"
QT_MOC_LITERAL(12, 162, 7), // "payload"
QT_MOC_LITERAL(13, 170, 19), // "slotPbMediaResponse"
QT_MOC_LITERAL(14, 190, 9) // "frameTime"

    },
    "PlaybackMedia\0slotBufferThreshold\0\0"
    "BUFFER_THRESHOLD_e\0threshold\0"
    "slotThrottleResponse\0REQ_MSG_ID_e\0"
    "requestId\0SET_COMMAND_e\0commandId\0"
    "DEVICE_REPLY_TYPE_e\0tStatusId\0payload\0"
    "slotPbMediaResponse\0frameTime"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlaybackMedia[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x0a /* Public */,
       5,    4,   32,    2, 0x0a /* Public */,
      13,    2,   41,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 8, 0x80000000 | 10, QMetaType::QString,    7,    9,   11,   12,
    QMetaType::Void, 0x80000000 | 10, QMetaType::QString,   11,   14,

       0        // eod
};

void PlaybackMedia::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlaybackMedia *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotBufferThreshold((*reinterpret_cast< BUFFER_THRESHOLD_e(*)>(_a[1]))); break;
        case 1: _t->slotThrottleResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 2: _t->slotPbMediaResponse((*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PlaybackMedia::staticMetaObject = { {
    QMetaObject::SuperData::link<MediaRequest::staticMetaObject>(),
    qt_meta_stringdata_PlaybackMedia.data,
    qt_meta_data_PlaybackMedia,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlaybackMedia::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlaybackMedia::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlaybackMedia.stringdata0))
        return static_cast<void*>(this);
    return MediaRequest::qt_metacast(_clname);
}

int PlaybackMedia::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MediaRequest::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
