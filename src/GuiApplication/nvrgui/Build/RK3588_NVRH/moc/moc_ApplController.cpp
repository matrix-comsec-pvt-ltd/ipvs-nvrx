/****************************************************************************
** Meta object code from reading C++ file 'ApplController.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ApplController.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ApplController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ApplController_t {
    QByteArrayData data[74];
    char stringdata0[1105];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ApplController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ApplController_t qt_meta_stringdata_ApplController = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ApplController"
QT_MOC_LITERAL(1, 15, 13), // "sigDevCommGui"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 10), // "deviceName"
QT_MOC_LITERAL(4, 41, 13), // "DevCommParam*"
QT_MOC_LITERAL(5, 55, 5), // "param"
QT_MOC_LITERAL(6, 61, 13), // "sigEventToGui"
QT_MOC_LITERAL(7, 75, 11), // "deviceIndex"
QT_MOC_LITERAL(8, 87, 7), // "evtType"
QT_MOC_LITERAL(9, 95, 10), // "evtSubType"
QT_MOC_LITERAL(10, 106, 8), // "evtIndex"
QT_MOC_LITERAL(11, 115, 8), // "evtState"
QT_MOC_LITERAL(12, 124, 16), // "evtAdvanceDetail"
QT_MOC_LITERAL(13, 141, 7), // "isLvEvt"
QT_MOC_LITERAL(14, 149, 18), // "sigPopUpEventToGui"
QT_MOC_LITERAL(15, 168, 7), // "devName"
QT_MOC_LITERAL(16, 176, 5), // "camNo"
QT_MOC_LITERAL(17, 182, 8), // "userName"
QT_MOC_LITERAL(18, 191, 9), // "popUpTime"
QT_MOC_LITERAL(19, 201, 6), // "userId"
QT_MOC_LITERAL(20, 208, 8), // "doorName"
QT_MOC_LITERAL(21, 217, 7), // "evtCode"
QT_MOC_LITERAL(22, 225, 24), // "sigStreamRequestResponse"
QT_MOC_LITERAL(23, 250, 21), // "STREAM_COMMAND_TYPE_e"
QT_MOC_LITERAL(24, 272, 17), // "streamCommandType"
QT_MOC_LITERAL(25, 290, 19), // "StreamRequestParam*"
QT_MOC_LITERAL(26, 310, 18), // "streamRequestParam"
QT_MOC_LITERAL(27, 329, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(28, 349, 11), // "deviceReply"
QT_MOC_LITERAL(29, 361, 24), // "sigDeviceListChangeToGui"
QT_MOC_LITERAL(30, 386, 22), // "sigLanguageCfgModified"
QT_MOC_LITERAL(31, 409, 3), // "str"
QT_MOC_LITERAL(32, 413, 21), // "sigStreamObjectDelete"
QT_MOC_LITERAL(33, 435, 15), // "DISPLAY_TYPE_e*"
QT_MOC_LITERAL(34, 451, 20), // "displayTypeForDelete"
QT_MOC_LITERAL(35, 472, 8), // "quint16*"
QT_MOC_LITERAL(36, 481, 23), // "actualWindowIdForDelete"
QT_MOC_LITERAL(37, 505, 15), // "sigHdmiInfoPage"
QT_MOC_LITERAL(38, 521, 14), // "isHdmiInfoShow"
QT_MOC_LITERAL(39, 536, 23), // "sigChangeAudButtonState"
QT_MOC_LITERAL(40, 560, 19), // "devCommActivitySlot"
QT_MOC_LITERAL(41, 580, 19), // "slotAudioCfgChanged"
QT_MOC_LITERAL(42, 600, 21), // "const AUDIO_CONFIG_t*"
QT_MOC_LITERAL(43, 622, 13), // "currentConfig"
QT_MOC_LITERAL(44, 636, 9), // "newConfig"
QT_MOC_LITERAL(45, 646, 19), // "slotDeviceCfgUpdate"
QT_MOC_LITERAL(46, 666, 17), // "remoteDeviceIndex"
QT_MOC_LITERAL(47, 684, 16), // "DEVICE_CONFIG_t*"
QT_MOC_LITERAL(48, 701, 12), // "deviceConfig"
QT_MOC_LITERAL(49, 714, 19), // "isUpdateOnLiveEvent"
QT_MOC_LITERAL(50, 734, 20), // "slotDeviceCfgChanged"
QT_MOC_LITERAL(51, 755, 8), // "devIndex"
QT_MOC_LITERAL(52, 764, 22), // "const DEVICE_CONFIG_t*"
QT_MOC_LITERAL(53, 787, 22), // "slotLanguageCfgChanged"
QT_MOC_LITERAL(54, 810, 9), // "slotEvent"
QT_MOC_LITERAL(55, 820, 16), // "LOG_EVENT_TYPE_e"
QT_MOC_LITERAL(56, 837, 9), // "eventType"
QT_MOC_LITERAL(57, 847, 19), // "LOG_EVENT_SUBTYPE_e"
QT_MOC_LITERAL(58, 867, 12), // "eventSubType"
QT_MOC_LITERAL(59, 880, 7), // "camIndx"
QT_MOC_LITERAL(60, 888, 17), // "LOG_EVENT_STATE_e"
QT_MOC_LITERAL(61, 906, 10), // "eventState"
QT_MOC_LITERAL(62, 917, 18), // "eventAdvanceDetail"
QT_MOC_LITERAL(63, 936, 11), // "isLiveEvent"
QT_MOC_LITERAL(64, 948, 14), // "slotPopUpEvent"
QT_MOC_LITERAL(65, 963, 8), // "camIndex"
QT_MOC_LITERAL(66, 972, 10), // "usrNameStr"
QT_MOC_LITERAL(67, 983, 12), // "popUpTimeStr"
QT_MOC_LITERAL(68, 996, 12), // "evtCodeIndex"
QT_MOC_LITERAL(69, 1009, 25), // "slotStreamRequestResponse"
QT_MOC_LITERAL(70, 1035, 23), // "slotDeleteStreamRequest"
QT_MOC_LITERAL(71, 1059, 8), // "streamId"
QT_MOC_LITERAL(72, 1068, 17), // "slotDelMedControl"
QT_MOC_LITERAL(73, 1086, 18) // "slotChangeAudState"

    },
    "ApplController\0sigDevCommGui\0\0deviceName\0"
    "DevCommParam*\0param\0sigEventToGui\0"
    "deviceIndex\0evtType\0evtSubType\0evtIndex\0"
    "evtState\0evtAdvanceDetail\0isLvEvt\0"
    "sigPopUpEventToGui\0devName\0camNo\0"
    "userName\0popUpTime\0userId\0doorName\0"
    "evtCode\0sigStreamRequestResponse\0"
    "STREAM_COMMAND_TYPE_e\0streamCommandType\0"
    "StreamRequestParam*\0streamRequestParam\0"
    "DEVICE_REPLY_TYPE_e\0deviceReply\0"
    "sigDeviceListChangeToGui\0"
    "sigLanguageCfgModified\0str\0"
    "sigStreamObjectDelete\0DISPLAY_TYPE_e*\0"
    "displayTypeForDelete\0quint16*\0"
    "actualWindowIdForDelete\0sigHdmiInfoPage\0"
    "isHdmiInfoShow\0sigChangeAudButtonState\0"
    "devCommActivitySlot\0slotAudioCfgChanged\0"
    "const AUDIO_CONFIG_t*\0currentConfig\0"
    "newConfig\0slotDeviceCfgUpdate\0"
    "remoteDeviceIndex\0DEVICE_CONFIG_t*\0"
    "deviceConfig\0isUpdateOnLiveEvent\0"
    "slotDeviceCfgChanged\0devIndex\0"
    "const DEVICE_CONFIG_t*\0slotLanguageCfgChanged\0"
    "slotEvent\0LOG_EVENT_TYPE_e\0eventType\0"
    "LOG_EVENT_SUBTYPE_e\0eventSubType\0"
    "camIndx\0LOG_EVENT_STATE_e\0eventState\0"
    "eventAdvanceDetail\0isLiveEvent\0"
    "slotPopUpEvent\0camIndex\0usrNameStr\0"
    "popUpTimeStr\0evtCodeIndex\0"
    "slotStreamRequestResponse\0"
    "slotDeleteStreamRequest\0streamId\0"
    "slotDelMedControl\0slotChangeAudState"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ApplController[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,  119,    2, 0x06 /* Public */,
       6,    7,  124,    2, 0x06 /* Public */,
      14,    7,  139,    2, 0x06 /* Public */,
      22,    3,  154,    2, 0x06 /* Public */,
      29,    0,  161,    2, 0x06 /* Public */,
      30,    1,  162,    2, 0x06 /* Public */,
      32,    2,  165,    2, 0x06 /* Public */,
      37,    1,  170,    2, 0x06 /* Public */,
      39,    0,  173,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      40,    2,  174,    2, 0x0a /* Public */,
      41,    2,  179,    2, 0x0a /* Public */,
      45,    3,  184,    2, 0x0a /* Public */,
      50,    3,  191,    2, 0x0a /* Public */,
      53,    1,  198,    2, 0x0a /* Public */,
      54,    7,  201,    2, 0x0a /* Public */,
      64,    7,  216,    2, 0x0a /* Public */,
      69,    3,  231,    2, 0x0a /* Public */,
      70,    1,  238,    2, 0x0a /* Public */,
      70,    1,  241,    2, 0x0a /* Public */,
      72,    1,  244,    2, 0x0a /* Public */,
      73,    0,  247,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::UChar, QMetaType::UChar, QMetaType::UChar, QMetaType::UChar, QMetaType::QString, QMetaType::Bool,    7,    8,    9,   10,   11,   12,   13,
    QMetaType::Void, QMetaType::QString, QMetaType::UChar, QMetaType::QString, QMetaType::UInt, QMetaType::QString, QMetaType::QString, QMetaType::UChar,   15,   16,   17,   18,   19,   20,   21,
    QMetaType::Void, 0x80000000 | 23, 0x80000000 | 25, 0x80000000 | 27,   24,   26,   28,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void, 0x80000000 | 33, 0x80000000 | 35,   34,   36,
    QMetaType::Void, QMetaType::Bool,   38,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4,    3,    5,
    QMetaType::Void, 0x80000000 | 42, 0x80000000 | 42,   43,   44,
    QMetaType::Void, QMetaType::UChar, 0x80000000 | 47, QMetaType::Bool,   46,   48,   49,
    QMetaType::Void, QMetaType::UChar, 0x80000000 | 52, 0x80000000 | 52,   51,   43,   44,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 55, 0x80000000 | 57, QMetaType::UChar, 0x80000000 | 60, QMetaType::QString, QMetaType::Bool,   51,   56,   58,   59,   61,   62,   63,
    QMetaType::Void, QMetaType::QString, QMetaType::UChar, QMetaType::QString, QMetaType::UInt, QMetaType::QString, QMetaType::QString, QMetaType::UChar,   51,   65,   66,   67,   19,   20,   68,
    QMetaType::Void, 0x80000000 | 23, 0x80000000 | 25, 0x80000000 | 27,   24,   26,   28,
    QMetaType::Void, QMetaType::UChar,   71,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::UChar,   71,
    QMetaType::Void,

       0        // eod
};

void ApplController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ApplController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigDevCommGui((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< DevCommParam*(*)>(_a[2]))); break;
        case 1: _t->sigEventToGui((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4])),(*reinterpret_cast< quint8(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7]))); break;
        case 2: _t->sigPopUpEventToGui((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< quint32(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< quint8(*)>(_a[7]))); break;
        case 3: _t->sigStreamRequestResponse((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3]))); break;
        case 4: _t->sigDeviceListChangeToGui(); break;
        case 5: _t->sigLanguageCfgModified((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->sigStreamObjectDelete((*reinterpret_cast< DISPLAY_TYPE_e*(*)>(_a[1])),(*reinterpret_cast< quint16*(*)>(_a[2]))); break;
        case 7: _t->sigHdmiInfoPage((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->sigChangeAudButtonState(); break;
        case 9: _t->devCommActivitySlot((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< DevCommParam*(*)>(_a[2]))); break;
        case 10: _t->slotAudioCfgChanged((*reinterpret_cast< const AUDIO_CONFIG_t*(*)>(_a[1])),(*reinterpret_cast< const AUDIO_CONFIG_t*(*)>(_a[2]))); break;
        case 11: _t->slotDeviceCfgUpdate((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< DEVICE_CONFIG_t*(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 12: _t->slotDeviceCfgChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< const DEVICE_CONFIG_t*(*)>(_a[2])),(*reinterpret_cast< const DEVICE_CONFIG_t*(*)>(_a[3]))); break;
        case 13: _t->slotLanguageCfgChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 14: _t->slotEvent((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< LOG_EVENT_TYPE_e(*)>(_a[2])),(*reinterpret_cast< LOG_EVENT_SUBTYPE_e(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4])),(*reinterpret_cast< LOG_EVENT_STATE_e(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7]))); break;
        case 15: _t->slotPopUpEvent((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< quint32(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< quint8(*)>(_a[7]))); break;
        case 16: _t->slotStreamRequestResponse((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3]))); break;
        case 17: _t->slotDeleteStreamRequest((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 18: _t->slotDeleteStreamRequest((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 19: _t->slotDelMedControl((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 20: _t->slotChangeAudState(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ApplController::*)(QString , DevCommParam * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigDevCommGui)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ApplController::*)(QString , quint8 , quint8 , quint8 , quint8 , QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigEventToGui)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ApplController::*)(QString , quint8 , QString , quint32 , QString , QString , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigPopUpEventToGui)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ApplController::*)(STREAM_COMMAND_TYPE_e , StreamRequestParam * , DEVICE_REPLY_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigStreamRequestResponse)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ApplController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigDeviceListChangeToGui)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ApplController::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigLanguageCfgModified)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ApplController::*)(DISPLAY_TYPE_e * , quint16 * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigStreamObjectDelete)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (ApplController::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigHdmiInfoPage)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (ApplController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplController::sigChangeAudButtonState)) {
                *result = 8;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ApplController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ApplController.data,
    qt_meta_data_ApplController,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ApplController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ApplController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ApplController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ApplController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 21)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void ApplController::sigDevCommGui(QString _t1, DevCommParam * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ApplController::sigEventToGui(QString _t1, quint8 _t2, quint8 _t3, quint8 _t4, quint8 _t5, QString _t6, bool _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ApplController::sigPopUpEventToGui(QString _t1, quint8 _t2, QString _t3, quint32 _t4, QString _t5, QString _t6, quint8 _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ApplController::sigStreamRequestResponse(STREAM_COMMAND_TYPE_e _t1, StreamRequestParam * _t2, DEVICE_REPLY_TYPE_e _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ApplController::sigDeviceListChangeToGui()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ApplController::sigLanguageCfgModified(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ApplController::sigStreamObjectDelete(DISPLAY_TYPE_e * _t1, quint16 * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ApplController::sigHdmiInfoPage(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void ApplController::sigChangeAudButtonState()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
