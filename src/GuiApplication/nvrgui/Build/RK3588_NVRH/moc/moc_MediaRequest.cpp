/****************************************************************************
** Meta object code from reading C++ file 'MediaRequest.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/StreamRequest/MediaRequest.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MediaRequest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MediaRequest_t {
    QByteArrayData data[10];
    char stringdata0[115];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MediaRequest_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MediaRequest_t qt_meta_stringdata_MediaRequest = {
    {
QT_MOC_LITERAL(0, 0, 12), // "MediaRequest"
QT_MOC_LITERAL(1, 13, 16), // "sigMediaResponse"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 12), // "REQ_MSG_ID_e"
QT_MOC_LITERAL(4, 44, 9), // "requestId"
QT_MOC_LITERAL(5, 54, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(6, 68, 9), // "commandId"
QT_MOC_LITERAL(7, 78, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(8, 98, 8), // "statusId"
QT_MOC_LITERAL(9, 107, 7) // "payload"

    },
    "MediaRequest\0sigMediaResponse\0\0"
    "REQ_MSG_ID_e\0requestId\0SET_COMMAND_e\0"
    "commandId\0DEVICE_REPLY_TYPE_e\0statusId\0"
    "payload"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MediaRequest[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    4,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7, QMetaType::QString,    4,    6,    8,    9,

       0        // eod
};

void MediaRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MediaRequest *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigMediaResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MediaRequest::*)(REQ_MSG_ID_e , SET_COMMAND_e , DEVICE_REPLY_TYPE_e , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MediaRequest::sigMediaResponse)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MediaRequest::staticMetaObject = { {
    QMetaObject::SuperData::link<CommandRequest::staticMetaObject>(),
    qt_meta_stringdata_MediaRequest.data,
    qt_meta_data_MediaRequest,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MediaRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MediaRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MediaRequest.stringdata0))
        return static_cast<void*>(this);
    return CommandRequest::qt_metacast(_clname);
}

int MediaRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CommandRequest::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void MediaRequest::sigMediaResponse(REQ_MSG_ID_e _t1, SET_COMMAND_e _t2, DEVICE_REPLY_TYPE_e _t3, QString _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
