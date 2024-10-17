/****************************************************************************
** Meta object code from reading C++ file 'CameraList.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Controls/CameraList.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CameraList_t {
    QByteArrayData data[34];
    char stringdata0[561];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraList_t qt_meta_stringdata_CameraList = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CameraList"
QT_MOC_LITERAL(1, 11, 23), // "sigUpdateCurrentElement"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 5), // "index"
QT_MOC_LITERAL(4, 42, 22), // "sigCameraButtonClicked"
QT_MOC_LITERAL(5, 65, 11), // "cameraIndex"
QT_MOC_LITERAL(6, 77, 10), // "deviceName"
QT_MOC_LITERAL(7, 88, 19), // "CAMERA_STATE_TYPE_e"
QT_MOC_LITERAL(8, 108, 15), // "connectionState"
QT_MOC_LITERAL(9, 124, 14), // "pageSwitchFlag"
QT_MOC_LITERAL(10, 139, 17), // "isChangeSelection"
QT_MOC_LITERAL(11, 157, 28), // "sigCameraButtonClickedWinSeq"
QT_MOC_LITERAL(12, 186, 20), // "clearConnectionState"
QT_MOC_LITERAL(13, 207, 29), // "slotCameraButtonClickedAddCam"
QT_MOC_LITERAL(14, 237, 12), // "sigClosePage"
QT_MOC_LITERAL(15, 250, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(16, 272, 25), // "sigCameraConfigListUpdate"
QT_MOC_LITERAL(17, 298, 14), // "sigSwapWindows"
QT_MOC_LITERAL(18, 313, 22), // "sigStartStreamInWindow"
QT_MOC_LITERAL(19, 336, 14), // "DISPLAY_TYPE_e"
QT_MOC_LITERAL(20, 351, 11), // "displayType"
QT_MOC_LITERAL(21, 363, 9), // "channelId"
QT_MOC_LITERAL(22, 373, 8), // "windowId"
QT_MOC_LITERAL(23, 382, 23), // "slotCameraButtonClicked"
QT_MOC_LITERAL(24, 406, 11), // "deviceIndex"
QT_MOC_LITERAL(25, 418, 22), // "slotDeviceButtonCliked"
QT_MOC_LITERAL(26, 441, 26), // "slotDevStateChangeBtnClick"
QT_MOC_LITERAL(27, 468, 19), // "DEVICE_STATE_TYPE_e"
QT_MOC_LITERAL(28, 488, 10), // "slotScroll"
QT_MOC_LITERAL(29, 499, 13), // "numberOfSteps"
QT_MOC_LITERAL(30, 513, 19), // "slotShowHideTooltip"
QT_MOC_LITERAL(31, 533, 6), // "startX"
QT_MOC_LITERAL(32, 540, 6), // "startY"
QT_MOC_LITERAL(33, 547, 13) // "toShowTooltip"

    },
    "CameraList\0sigUpdateCurrentElement\0\0"
    "index\0sigCameraButtonClicked\0cameraIndex\0"
    "deviceName\0CAMERA_STATE_TYPE_e\0"
    "connectionState\0pageSwitchFlag\0"
    "isChangeSelection\0sigCameraButtonClickedWinSeq\0"
    "clearConnectionState\0slotCameraButtonClickedAddCam\0"
    "sigClosePage\0TOOLBAR_BUTTON_TYPE_e\0"
    "sigCameraConfigListUpdate\0sigSwapWindows\0"
    "sigStartStreamInWindow\0DISPLAY_TYPE_e\0"
    "displayType\0channelId\0windowId\0"
    "slotCameraButtonClicked\0deviceIndex\0"
    "slotDeviceButtonCliked\0"
    "slotDevStateChangeBtnClick\0"
    "DEVICE_STATE_TYPE_e\0slotScroll\0"
    "numberOfSteps\0slotShowHideTooltip\0"
    "startX\0startY\0toShowTooltip"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraList[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  104,    2, 0x06 /* Public */,
       4,    5,  107,    2, 0x06 /* Public */,
       4,    4,  118,    2, 0x26 /* Public | MethodCloned */,
      11,    4,  127,    2, 0x06 /* Public */,
      11,    3,  136,    2, 0x26 /* Public | MethodCloned */,
      13,    2,  143,    2, 0x06 /* Public */,
      13,    1,  148,    2, 0x26 /* Public | MethodCloned */,
      14,    1,  151,    2, 0x06 /* Public */,
      16,    0,  154,    2, 0x06 /* Public */,
      17,    2,  155,    2, 0x06 /* Public */,
      18,    4,  160,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      23,    3,  169,    2, 0x0a /* Public */,
      23,    2,  176,    2, 0x2a /* Public | MethodCloned */,
      23,    1,  181,    2, 0x2a /* Public | MethodCloned */,
      25,    1,  184,    2, 0x0a /* Public */,
      26,    2,  187,    2, 0x0a /* Public */,
      28,    1,  192,    2, 0x0a /* Public */,
      30,    5,  195,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, 0x80000000 | 7, QMetaType::Bool, QMetaType::Bool,    5,    6,    8,    9,   10,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, 0x80000000 | 7, QMetaType::Bool,    5,    6,    8,    9,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, 0x80000000 | 7, 0x80000000 | 7,    5,    6,    8,   12,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, 0x80000000 | 7,    5,    6,    8,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7,    3,    8,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, 0x80000000 | 15,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort,    2,    2,
    QMetaType::Void, 0x80000000 | 19, QMetaType::QString, QMetaType::UChar, QMetaType::UShort,   20,    6,   21,   22,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7, QMetaType::Int,    3,    8,   24,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7,    3,    8,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 27,    3,    8,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   31,   32,   24,    3,   33,

       0        // eod
};

void CameraList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraList *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sigCameraButtonClicked((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 2: _t->sigCameraButtonClicked((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 3: _t->sigCameraButtonClickedWinSeq((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[3])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[4]))); break;
        case 4: _t->sigCameraButtonClickedWinSeq((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[3]))); break;
        case 5: _t->slotCameraButtonClickedAddCam((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[2]))); break;
        case 6: _t->slotCameraButtonClickedAddCam((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->sigClosePage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 8: _t->sigCameraConfigListUpdate(); break;
        case 9: _t->sigSwapWindows((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 10: _t->sigStartStreamInWindow((*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< quint16(*)>(_a[4]))); break;
        case 11: _t->slotCameraButtonClicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 12: _t->slotCameraButtonClicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[2]))); break;
        case 13: _t->slotCameraButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->slotDeviceButtonCliked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->slotDevStateChangeBtnClick((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< DEVICE_STATE_TYPE_e(*)>(_a[2]))); break;
        case 16: _t->slotScroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->slotShowHideTooltip((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CameraList::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigUpdateCurrentElement)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CameraList::*)(quint8 , QString , CAMERA_STATE_TYPE_e , bool , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigCameraButtonClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CameraList::*)(quint8 , QString , CAMERA_STATE_TYPE_e , CAMERA_STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigCameraButtonClickedWinSeq)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CameraList::*)(int , CAMERA_STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::slotCameraButtonClickedAddCam)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (CameraList::*)(TOOLBAR_BUTTON_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigClosePage)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (CameraList::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigCameraConfigListUpdate)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (CameraList::*)(quint16 , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigSwapWindows)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (CameraList::*)(DISPLAY_TYPE_e , QString , quint8 , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraList::sigStartStreamInWindow)) {
                *result = 10;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CameraList::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_CameraList.data,
    qt_meta_data_CameraList,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CameraList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CameraList.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "NavigationControl"))
        return static_cast< NavigationControl*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int CameraList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void CameraList::sigUpdateCurrentElement(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CameraList::sigCameraButtonClicked(quint8 _t1, QString _t2, CAMERA_STATE_TYPE_e _t3, bool _t4, bool _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 3
void CameraList::sigCameraButtonClickedWinSeq(quint8 _t1, QString _t2, CAMERA_STATE_TYPE_e _t3, CAMERA_STATE_TYPE_e _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 5
void CameraList::slotCameraButtonClickedAddCam(int _t1, CAMERA_STATE_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 7
void CameraList::sigClosePage(TOOLBAR_BUTTON_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void CameraList::sigCameraConfigListUpdate()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void CameraList::sigSwapWindows(quint16 _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void CameraList::sigStartStreamInWindow(DISPLAY_TYPE_e _t1, QString _t2, quint8 _t3, quint16 _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
