/****************************************************************************
** Meta object code from reading C++ file 'InstantPlaybackMedia.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/StreamRequest/InstantPlaybackMedia/InstantPlaybackMedia.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'InstantPlaybackMedia.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_InstantPlaybackMedia_t {
    QByteArrayData data[14];
    char stringdata0[190];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_InstantPlaybackMedia_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_InstantPlaybackMedia_t qt_meta_stringdata_InstantPlaybackMedia = {
    {
QT_MOC_LITERAL(0, 0, 20), // "InstantPlaybackMedia"
QT_MOC_LITERAL(1, 21, 16), // "sigMediaResponse"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 12), // "REQ_MSG_ID_e"
QT_MOC_LITERAL(4, 52, 9), // "requestId"
QT_MOC_LITERAL(5, 62, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(6, 76, 9), // "commandId"
QT_MOC_LITERAL(7, 86, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(8, 106, 8), // "statusId"
QT_MOC_LITERAL(9, 115, 7), // "payload"
QT_MOC_LITERAL(10, 123, 19), // "slotBufferThreshold"
QT_MOC_LITERAL(11, 143, 18), // "BUFFER_THRESHOLD_e"
QT_MOC_LITERAL(12, 162, 9), // "threshold"
QT_MOC_LITERAL(13, 172, 17) // "slotMediaResponse"

    },
    "InstantPlaybackMedia\0sigMediaResponse\0"
    "\0REQ_MSG_ID_e\0requestId\0SET_COMMAND_e\0"
    "commandId\0DEVICE_REPLY_TYPE_e\0statusId\0"
    "payload\0slotBufferThreshold\0"
    "BUFFER_THRESHOLD_e\0threshold\0"
    "slotMediaResponse"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_InstantPlaybackMedia[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    4,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   38,    2, 0x0a /* Public */,
      13,    4,   41,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7, QMetaType::QString,    4,    6,    8,    9,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7, QMetaType::QString,    4,    6,    8,    9,

       0        // eod
};

void InstantPlaybackMedia::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<InstantPlaybackMedia *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigMediaResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 1: _t->slotBufferThreshold((*reinterpret_cast< BUFFER_THRESHOLD_e(*)>(_a[1]))); break;
        case 2: _t->slotMediaResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (InstantPlaybackMedia::*)(REQ_MSG_ID_e , SET_COMMAND_e , DEVICE_REPLY_TYPE_e , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&InstantPlaybackMedia::sigMediaResponse)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject InstantPlaybackMedia::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_InstantPlaybackMedia.data,
    qt_meta_data_InstantPlaybackMedia,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *InstantPlaybackMedia::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InstantPlaybackMedia::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_InstantPlaybackMedia.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int InstantPlaybackMedia::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void InstantPlaybackMedia::sigMediaResponse(REQ_MSG_ID_e _t1, SET_COMMAND_e _t2, DEVICE_REPLY_TYPE_e _t3, QString _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
