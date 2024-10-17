/****************************************************************************
** Meta object code from reading C++ file 'StreamRequest.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/StreamRequest/StreamRequest.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StreamRequest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StreamRequest_t {
    QByteArrayData data[26];
    char stringdata0[390];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StreamRequest_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StreamRequest_t qt_meta_stringdata_StreamRequest = {
    {
QT_MOC_LITERAL(0, 0, 13), // "StreamRequest"
QT_MOC_LITERAL(1, 14, 17), // "sigProcessRequest"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 21), // "STREAM_COMMAND_TYPE_e"
QT_MOC_LITERAL(4, 55, 17), // "streamCommandType"
QT_MOC_LITERAL(5, 73, 21), // "SERVER_SESSION_INFO_t"
QT_MOC_LITERAL(6, 95, 10), // "serverInfo"
QT_MOC_LITERAL(7, 106, 19), // "StreamRequestParam*"
QT_MOC_LITERAL(8, 126, 18), // "streamRequestParam"
QT_MOC_LITERAL(9, 145, 8), // "nextInfo"
QT_MOC_LITERAL(10, 154, 9), // "nextParam"
QT_MOC_LITERAL(11, 164, 24), // "sigStreamRequestResponse"
QT_MOC_LITERAL(12, 189, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(13, 209, 11), // "deviceReply"
QT_MOC_LITERAL(14, 221, 22), // "sigDeleteStreamRequest"
QT_MOC_LITERAL(15, 244, 8), // "streamId"
QT_MOC_LITERAL(16, 253, 16), // "sigDelMedControl"
QT_MOC_LITERAL(17, 270, 17), // "sigChangeAudState"
QT_MOC_LITERAL(18, 288, 18), // "slotProcessRequest"
QT_MOC_LITERAL(19, 307, 18), // "slotStreamResponse"
QT_MOC_LITERAL(20, 326, 12), // "REQ_MSG_ID_e"
QT_MOC_LITERAL(21, 339, 9), // "requestId"
QT_MOC_LITERAL(22, 349, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(23, 363, 9), // "commandId"
QT_MOC_LITERAL(24, 373, 8), // "statusId"
QT_MOC_LITERAL(25, 382, 7) // "payload"

    },
    "StreamRequest\0sigProcessRequest\0\0"
    "STREAM_COMMAND_TYPE_e\0streamCommandType\0"
    "SERVER_SESSION_INFO_t\0serverInfo\0"
    "StreamRequestParam*\0streamRequestParam\0"
    "nextInfo\0nextParam\0sigStreamRequestResponse\0"
    "DEVICE_REPLY_TYPE_e\0deviceReply\0"
    "sigDeleteStreamRequest\0streamId\0"
    "sigDelMedControl\0sigChangeAudState\0"
    "slotProcessRequest\0slotStreamResponse\0"
    "REQ_MSG_ID_e\0requestId\0SET_COMMAND_e\0"
    "commandId\0statusId\0payload"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StreamRequest[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   59,    2, 0x06 /* Public */,
       1,    5,   66,    2, 0x06 /* Public */,
      11,    3,   77,    2, 0x06 /* Public */,
      14,    1,   84,    2, 0x06 /* Public */,
      16,    1,   87,    2, 0x06 /* Public */,
      17,    0,   90,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      18,    3,   91,    2, 0x0a /* Public */,
      18,    5,   98,    2, 0x0a /* Public */,
      19,    4,  109,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7,    4,    6,    8,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7, 0x80000000 | 5, 0x80000000 | 7,    4,    6,    8,    9,   10,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 7, 0x80000000 | 12,    4,    8,   13,
    QMetaType::Void, QMetaType::UChar,   15,
    QMetaType::Void, QMetaType::UChar,   15,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7,    4,    6,    8,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7, 0x80000000 | 5, 0x80000000 | 7,    4,    6,    8,    9,   10,
    QMetaType::Void, 0x80000000 | 20, 0x80000000 | 22, 0x80000000 | 12, QMetaType::QString,   21,   23,   24,   25,

       0        // eod
};

void StreamRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StreamRequest *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigProcessRequest((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< SERVER_SESSION_INFO_t(*)>(_a[2])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[3]))); break;
        case 1: _t->sigProcessRequest((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< SERVER_SESSION_INFO_t(*)>(_a[2])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[3])),(*reinterpret_cast< SERVER_SESSION_INFO_t(*)>(_a[4])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[5]))); break;
        case 2: _t->sigStreamRequestResponse((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3]))); break;
        case 3: _t->sigDeleteStreamRequest((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 4: _t->sigDelMedControl((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 5: _t->sigChangeAudState(); break;
        case 6: _t->slotProcessRequest((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< SERVER_SESSION_INFO_t(*)>(_a[2])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[3]))); break;
        case 7: _t->slotProcessRequest((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< SERVER_SESSION_INFO_t(*)>(_a[2])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[3])),(*reinterpret_cast< SERVER_SESSION_INFO_t(*)>(_a[4])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[5]))); break;
        case 8: _t->slotStreamResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StreamRequest::*)(STREAM_COMMAND_TYPE_e , SERVER_SESSION_INFO_t , StreamRequestParam * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamRequest::sigProcessRequest)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StreamRequest::*)(STREAM_COMMAND_TYPE_e , SERVER_SESSION_INFO_t , StreamRequestParam * , SERVER_SESSION_INFO_t , StreamRequestParam * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamRequest::sigProcessRequest)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StreamRequest::*)(STREAM_COMMAND_TYPE_e , StreamRequestParam * , DEVICE_REPLY_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamRequest::sigStreamRequestResponse)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StreamRequest::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamRequest::sigDeleteStreamRequest)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (StreamRequest::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamRequest::sigDelMedControl)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (StreamRequest::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StreamRequest::sigChangeAudState)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject StreamRequest::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_StreamRequest.data,
    qt_meta_data_StreamRequest,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *StreamRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StreamRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StreamRequest.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int StreamRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void StreamRequest::sigProcessRequest(STREAM_COMMAND_TYPE_e _t1, SERVER_SESSION_INFO_t _t2, StreamRequestParam * _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void StreamRequest::sigProcessRequest(STREAM_COMMAND_TYPE_e _t1, SERVER_SESSION_INFO_t _t2, StreamRequestParam * _t3, SERVER_SESSION_INFO_t _t4, StreamRequestParam * _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void StreamRequest::sigStreamRequestResponse(STREAM_COMMAND_TYPE_e _t1, StreamRequestParam * _t2, DEVICE_REPLY_TYPE_e _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void StreamRequest::sigDeleteStreamRequest(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void StreamRequest::sigDelMedControl(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void StreamRequest::sigChangeAudState()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
