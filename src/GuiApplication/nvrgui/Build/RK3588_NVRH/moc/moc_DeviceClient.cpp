/****************************************************************************
** Meta object code from reading C++ file 'DeviceClient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DeviceClient/DeviceClient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeviceClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DeviceClient_t {
    QByteArrayData data[35];
    char stringdata0[499];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeviceClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeviceClient_t qt_meta_stringdata_DeviceClient = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DeviceClient"
QT_MOC_LITERAL(1, 13, 8), // "sigEvent"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 16), // "LOG_EVENT_TYPE_e"
QT_MOC_LITERAL(4, 40, 19), // "LOG_EVENT_SUBTYPE_e"
QT_MOC_LITERAL(5, 60, 17), // "LOG_EVENT_STATE_e"
QT_MOC_LITERAL(6, 78, 13), // "sigPopUpEvent"
QT_MOC_LITERAL(7, 92, 17), // "sigDeviceResponse"
QT_MOC_LITERAL(8, 110, 10), // "deviceName"
QT_MOC_LITERAL(9, 121, 13), // "DevCommParam*"
QT_MOC_LITERAL(10, 135, 17), // "SigProcessRequest"
QT_MOC_LITERAL(11, 153, 13), // "sigExitThread"
QT_MOC_LITERAL(12, 167, 22), // "sigDeleteStreamRequest"
QT_MOC_LITERAL(13, 190, 18), // "sigDeviceCfgUpdate"
QT_MOC_LITERAL(14, 209, 5), // "index"
QT_MOC_LITERAL(15, 215, 16), // "DEVICE_CONFIG_t*"
QT_MOC_LITERAL(16, 232, 12), // "deviceConfig"
QT_MOC_LITERAL(17, 245, 19), // "isUpdateOnLiveEvent"
QT_MOC_LITERAL(18, 265, 19), // "slotConnectResponse"
QT_MOC_LITERAL(19, 285, 12), // "REQ_MSG_ID_e"
QT_MOC_LITERAL(20, 298, 9), // "requestId"
QT_MOC_LITERAL(21, 308, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(22, 328, 8), // "statusId"
QT_MOC_LITERAL(23, 337, 7), // "payload"
QT_MOC_LITERAL(24, 345, 6), // "ipAddr"
QT_MOC_LITERAL(25, 352, 7), // "tcpPort"
QT_MOC_LITERAL(26, 360, 18), // "slotConfigResponse"
QT_MOC_LITERAL(27, 379, 11), // "genReqSesId"
QT_MOC_LITERAL(28, 391, 19), // "slotCommandResponse"
QT_MOC_LITERAL(29, 411, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(30, 425, 9), // "commandId"
QT_MOC_LITERAL(31, 435, 8), // "cmdSesId"
QT_MOC_LITERAL(32, 444, 21), // "slotPwdRstCmdResponse"
QT_MOC_LITERAL(33, 466, 13), // "PWD_RST_CMD_e"
QT_MOC_LITERAL(34, 480, 18) // "SlotProcessRequest"

    },
    "DeviceClient\0sigEvent\0\0LOG_EVENT_TYPE_e\0"
    "LOG_EVENT_SUBTYPE_e\0LOG_EVENT_STATE_e\0"
    "sigPopUpEvent\0sigDeviceResponse\0"
    "deviceName\0DevCommParam*\0SigProcessRequest\0"
    "sigExitThread\0sigDeleteStreamRequest\0"
    "sigDeviceCfgUpdate\0index\0DEVICE_CONFIG_t*\0"
    "deviceConfig\0isUpdateOnLiveEvent\0"
    "slotConnectResponse\0REQ_MSG_ID_e\0"
    "requestId\0DEVICE_REPLY_TYPE_e\0statusId\0"
    "payload\0ipAddr\0tcpPort\0slotConfigResponse\0"
    "genReqSesId\0slotCommandResponse\0"
    "SET_COMMAND_e\0commandId\0cmdSesId\0"
    "slotPwdRstCmdResponse\0PWD_RST_CMD_e\0"
    "SlotProcessRequest"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeviceClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    7,  124,    2, 0x06 /* Public */,
       6,    7,  139,    2, 0x06 /* Public */,
       7,    2,  154,    2, 0x06 /* Public */,
      10,    1,  159,    2, 0x06 /* Public */,
      11,    0,  162,    2, 0x06 /* Public */,
      12,    1,  163,    2, 0x06 /* Public */,
      13,    3,  166,    2, 0x06 /* Public */,
      13,    2,  173,    2, 0x26 /* Public | MethodCloned */,

 // slots: name, argc, parameters, tag, flags
      18,    5,  178,    2, 0x0a /* Public */,
      18,    4,  189,    2, 0x2a /* Public | MethodCloned */,
      18,    3,  198,    2, 0x2a /* Public | MethodCloned */,
      18,    2,  205,    2, 0x2a /* Public | MethodCloned */,
      26,    4,  210,    2, 0x0a /* Public */,
      26,    3,  219,    2, 0x2a /* Public | MethodCloned */,
      26,    2,  226,    2, 0x2a /* Public | MethodCloned */,
      28,    5,  231,    2, 0x0a /* Public */,
      28,    4,  242,    2, 0x2a /* Public | MethodCloned */,
      28,    3,  251,    2, 0x2a /* Public | MethodCloned */,
      32,    5,  258,    2, 0x0a /* Public */,
      32,    4,  269,    2, 0x2a /* Public | MethodCloned */,
      32,    3,  278,    2, 0x2a /* Public | MethodCloned */,
      34,    1,  285,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 3, 0x80000000 | 4, QMetaType::UChar, 0x80000000 | 5, QMetaType::QString, QMetaType::Bool,    2,    2,    2,    2,    2,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UChar, QMetaType::QString, QMetaType::UInt, QMetaType::QString, QMetaType::QString, QMetaType::UChar,    2,    2,    2,    2,    2,    2,    2,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 9,    8,    2,
    QMetaType::Void, 0x80000000 | 9,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::UChar, 0x80000000 | 15, QMetaType::Bool,   14,   16,   17,
    QMetaType::Void, QMetaType::UChar, 0x80000000 | 15,   14,   16,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21, QMetaType::QString, QMetaType::QString, QMetaType::UShort,   20,   22,   23,   24,   25,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21, QMetaType::QString, QMetaType::QString,   20,   22,   23,   24,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21, QMetaType::QString,   20,   22,   23,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21,   20,   22,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21, QMetaType::QString, QMetaType::UChar,   20,   22,   23,   27,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21, QMetaType::QString,   20,   22,   23,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 21,   20,   22,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 29, 0x80000000 | 21, QMetaType::QString, QMetaType::UChar,   20,   30,   22,   23,   31,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 29, 0x80000000 | 21, QMetaType::QString,   20,   30,   22,   23,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 29, 0x80000000 | 21,   20,   30,   22,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 33, 0x80000000 | 21, QMetaType::QString, QMetaType::UChar,   20,   30,   22,   23,   31,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 33, 0x80000000 | 21, QMetaType::QString,   20,   30,   22,   23,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 33, 0x80000000 | 21,   20,   30,   22,
    QMetaType::Void, 0x80000000 | 9,    2,

       0        // eod
};

void DeviceClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeviceClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigEvent((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< LOG_EVENT_TYPE_e(*)>(_a[2])),(*reinterpret_cast< LOG_EVENT_SUBTYPE_e(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4])),(*reinterpret_cast< LOG_EVENT_STATE_e(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7]))); break;
        case 1: _t->sigPopUpEvent((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< quint32(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< quint8(*)>(_a[7]))); break;
        case 2: _t->sigDeviceResponse((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< DevCommParam*(*)>(_a[2]))); break;
        case 3: _t->SigProcessRequest((*reinterpret_cast< DevCommParam*(*)>(_a[1]))); break;
        case 4: _t->sigExitThread(); break;
        case 5: _t->sigDeleteStreamRequest((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->sigDeviceCfgUpdate((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< DEVICE_CONFIG_t*(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 7: _t->sigDeviceCfgUpdate((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< DEVICE_CONFIG_t*(*)>(_a[2]))); break;
        case 8: _t->slotConnectResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< quint16(*)>(_a[5]))); break;
        case 9: _t->slotConnectResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 10: _t->slotConnectResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 11: _t->slotConnectResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2]))); break;
        case 12: _t->slotConfigResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4]))); break;
        case 13: _t->slotConfigResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 14: _t->slotConfigResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[2]))); break;
        case 15: _t->slotCommandResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< quint8(*)>(_a[5]))); break;
        case 16: _t->slotCommandResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 17: _t->slotCommandResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< SET_COMMAND_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3]))); break;
        case 18: _t->slotPwdRstCmdResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< PWD_RST_CMD_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< quint8(*)>(_a[5]))); break;
        case 19: _t->slotPwdRstCmdResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< PWD_RST_CMD_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 20: _t->slotPwdRstCmdResponse((*reinterpret_cast< REQ_MSG_ID_e(*)>(_a[1])),(*reinterpret_cast< PWD_RST_CMD_e(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3]))); break;
        case 21: _t->SlotProcessRequest((*reinterpret_cast< DevCommParam*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DeviceClient::*)(QString , LOG_EVENT_TYPE_e , LOG_EVENT_SUBTYPE_e , quint8 , LOG_EVENT_STATE_e , QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::sigEvent)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DeviceClient::*)(QString , quint8 , QString , quint32 , QString , QString , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::sigPopUpEvent)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DeviceClient::*)(QString , DevCommParam * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::sigDeviceResponse)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DeviceClient::*)(DevCommParam * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::SigProcessRequest)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DeviceClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::sigExitThread)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DeviceClient::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::sigDeleteStreamRequest)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DeviceClient::*)(quint8 , DEVICE_CONFIG_t * , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceClient::sigDeviceCfgUpdate)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DeviceClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DeviceClient.data,
    qt_meta_data_DeviceClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DeviceClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeviceClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DeviceClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void DeviceClient::sigEvent(QString _t1, LOG_EVENT_TYPE_e _t2, LOG_EVENT_SUBTYPE_e _t3, quint8 _t4, LOG_EVENT_STATE_e _t5, QString _t6, bool _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DeviceClient::sigPopUpEvent(QString _t1, quint8 _t2, QString _t3, quint32 _t4, QString _t5, QString _t6, quint8 _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DeviceClient::sigDeviceResponse(QString _t1, DevCommParam * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DeviceClient::SigProcessRequest(DevCommParam * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DeviceClient::sigExitThread()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DeviceClient::sigDeleteStreamRequest(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void DeviceClient::sigDeviceCfgUpdate(quint8 _t1, DEVICE_CONFIG_t * _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
