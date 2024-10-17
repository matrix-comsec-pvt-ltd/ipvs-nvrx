/****************************************************************************
** Meta object code from reading C++ file 'InstantPlaybackFrameReceiver.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/StreamRequest/InstantPlaybackMedia/InstantPlaybackFrameReceiver.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'InstantPlaybackFrameReceiver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_InstantPlaybackFrameReceiver_t {
    QByteArrayData data[11];
    char stringdata0[174];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_InstantPlaybackFrameReceiver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_InstantPlaybackFrameReceiver_t qt_meta_stringdata_InstantPlaybackFrameReceiver = {
    {
QT_MOC_LITERAL(0, 0, 28), // "InstantPlaybackFrameReceiver"
QT_MOC_LITERAL(1, 29, 18), // "sigBufferThreshold"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 18), // "BUFFER_THRESHOLD_e"
QT_MOC_LITERAL(4, 68, 10), // "thresholds"
QT_MOC_LITERAL(5, 79, 19), // "slotBufferThreshold"
QT_MOC_LITERAL(6, 99, 9), // "threshold"
QT_MOC_LITERAL(7, 109, 18), // "slotFeederResponse"
QT_MOC_LITERAL(8, 128, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(9, 148, 9), // "tStatusId"
QT_MOC_LITERAL(10, 158, 15) // "responsePayload"

    },
    "InstantPlaybackFrameReceiver\0"
    "sigBufferThreshold\0\0BUFFER_THRESHOLD_e\0"
    "thresholds\0slotBufferThreshold\0threshold\0"
    "slotFeederResponse\0DEVICE_REPLY_TYPE_e\0"
    "tStatusId\0responsePayload"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_InstantPlaybackFrameReceiver[] = {

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
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   32,    2, 0x0a /* Public */,
       7,    2,   35,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::QString,    9,   10,

       0        // eod
};

void InstantPlaybackFrameReceiver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<InstantPlaybackFrameReceiver *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigBufferThreshold((*reinterpret_cast< BUFFER_THRESHOLD_e(*)>(_a[1]))); break;
        case 1: _t->slotBufferThreshold((*reinterpret_cast< BUFFER_THRESHOLD_e(*)>(_a[1]))); break;
        case 2: _t->slotFeederResponse((*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (InstantPlaybackFrameReceiver::*)(BUFFER_THRESHOLD_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&InstantPlaybackFrameReceiver::sigBufferThreshold)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject InstantPlaybackFrameReceiver::staticMetaObject = { {
    QMetaObject::SuperData::link<MediaRequest::staticMetaObject>(),
    qt_meta_stringdata_InstantPlaybackFrameReceiver.data,
    qt_meta_data_InstantPlaybackFrameReceiver,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *InstantPlaybackFrameReceiver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InstantPlaybackFrameReceiver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_InstantPlaybackFrameReceiver.stringdata0))
        return static_cast<void*>(this);
    return MediaRequest::qt_metacast(_clname);
}

int InstantPlaybackFrameReceiver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void InstantPlaybackFrameReceiver::sigBufferThreshold(BUFFER_THRESHOLD_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
