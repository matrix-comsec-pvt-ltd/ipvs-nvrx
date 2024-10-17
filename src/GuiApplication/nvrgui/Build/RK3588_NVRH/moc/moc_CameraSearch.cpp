/****************************************************************************
** Meta object code from reading C++ file 'CameraSearch.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ConfigPages/CameraSettings/CameraSearch.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraSearch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CameraSearch_t {
    QByteArrayData data[33];
    char stringdata0[524];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraSearch_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraSearch_t qt_meta_stringdata_CameraSearch = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CameraSearch"
QT_MOC_LITERAL(1, 13, 15), // "slotButtonClick"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 19), // "slotAddCameraDelete"
QT_MOC_LITERAL(4, 50, 9), // "listIndex"
QT_MOC_LITERAL(5, 60, 14), // "saveCameraFlag"
QT_MOC_LITERAL(6, 75, 15), // "ipAddressString"
QT_MOC_LITERAL(7, 91, 11), // "httpPortStr"
QT_MOC_LITERAL(8, 103, 15), // "onvifPortString"
QT_MOC_LITERAL(9, 119, 13), // "onvifSupportF"
QT_MOC_LITERAL(10, 133, 12), // "brandListStr"
QT_MOC_LITERAL(11, 146, 12), // "modelListStr"
QT_MOC_LITERAL(12, 159, 7), // "camName"
QT_MOC_LITERAL(13, 167, 8), // "userName"
QT_MOC_LITERAL(14, 176, 9), // "tPassword"
QT_MOC_LITERAL(15, 186, 8), // "selIndex"
QT_MOC_LITERAL(16, 195, 12), // "currentIndex"
QT_MOC_LITERAL(17, 208, 20), // "slotCreateCMDRequest"
QT_MOC_LITERAL(18, 229, 13), // "SET_COMMAND_e"
QT_MOC_LITERAL(19, 243, 7), // "cmdType"
QT_MOC_LITERAL(20, 251, 11), // "totalFeilds"
QT_MOC_LITERAL(21, 263, 17), // "slotTestCamDelete"
QT_MOC_LITERAL(22, 281, 23), // "slotAdvanceSearchDelete"
QT_MOC_LITERAL(23, 305, 20), // "slotFailReportDelete"
QT_MOC_LITERAL(24, 326, 22), // "slotFilterValueChanged"
QT_MOC_LITERAL(25, 349, 25), // "slotOptionSelectionButton"
QT_MOC_LITERAL(26, 375, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(27, 395, 18), // "slotAddButtonClick"
QT_MOC_LITERAL(28, 414, 19), // "slotTestButtonClick"
QT_MOC_LITERAL(29, 434, 22), // "slotautoAddIconTimeOut"
QT_MOC_LITERAL(30, 457, 22), // "slotAdvanceSearchRange"
QT_MOC_LITERAL(31, 480, 26), // "slotGetAcqListTimerTimeout"
QT_MOC_LITERAL(32, 507, 16) // "slotObjectDelete"

    },
    "CameraSearch\0slotButtonClick\0\0"
    "slotAddCameraDelete\0listIndex\0"
    "saveCameraFlag\0ipAddressString\0"
    "httpPortStr\0onvifPortString\0onvifSupportF\0"
    "brandListStr\0modelListStr\0camName\0"
    "userName\0tPassword\0selIndex\0currentIndex\0"
    "slotCreateCMDRequest\0SET_COMMAND_e\0"
    "cmdType\0totalFeilds\0slotTestCamDelete\0"
    "slotAdvanceSearchDelete\0slotFailReportDelete\0"
    "slotFilterValueChanged\0slotOptionSelectionButton\0"
    "OPTION_STATE_TYPE_e\0slotAddButtonClick\0"
    "slotTestButtonClick\0slotautoAddIconTimeOut\0"
    "slotAdvanceSearchRange\0"
    "slotGetAcqListTimerTimeout\0slotObjectDelete"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraSearch[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x0a /* Public */,
       3,   13,   87,    2, 0x0a /* Public */,
      17,    2,  114,    2, 0x0a /* Public */,
      21,    1,  119,    2, 0x0a /* Public */,
      22,    1,  122,    2, 0x0a /* Public */,
      23,    0,  125,    2, 0x0a /* Public */,
      24,    2,  126,    2, 0x0a /* Public */,
      25,    2,  131,    2, 0x0a /* Public */,
      27,    1,  136,    2, 0x0a /* Public */,
      28,    1,  139,    2, 0x0a /* Public */,
      29,    0,  142,    2, 0x0a /* Public */,
      30,    3,  143,    2, 0x0a /* Public */,
      31,    0,  150,    2, 0x0a /* Public */,
      32,    0,  151,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::UChar, QMetaType::Bool, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::UChar, QMetaType::UChar,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
    QMetaType::Void, 0x80000000 | 18, QMetaType::UChar,   19,   20,
    QMetaType::Void, QMetaType::UChar,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    2,
    QMetaType::Void, 0x80000000 | 26, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CameraSearch::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraSearch *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->slotButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotAddCameraDelete((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< QString(*)>(_a[7])),(*reinterpret_cast< QString(*)>(_a[8])),(*reinterpret_cast< QString(*)>(_a[9])),(*reinterpret_cast< QString(*)>(_a[10])),(*reinterpret_cast< QString(*)>(_a[11])),(*reinterpret_cast< quint8(*)>(_a[12])),(*reinterpret_cast< quint8(*)>(_a[13]))); break;
        case 2: _t->slotCreateCMDRequest((*reinterpret_cast< SET_COMMAND_e(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 3: _t->slotTestCamDelete((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 4: _t->slotAdvanceSearchDelete((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->slotFailReportDelete(); break;
        case 6: _t->slotFilterValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 7: _t->slotOptionSelectionButton((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->slotAddButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->slotTestButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slotautoAddIconTimeOut(); break;
        case 11: _t->slotAdvanceSearchRange((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 12: _t->slotGetAcqListTimerTimeout(); break;
        case 13: _t->slotObjectDelete(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CameraSearch::staticMetaObject = { {
    QMetaObject::SuperData::link<ConfigPageControl::staticMetaObject>(),
    qt_meta_stringdata_CameraSearch.data,
    qt_meta_data_CameraSearch,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CameraSearch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraSearch::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CameraSearch.stringdata0))
        return static_cast<void*>(this);
    return ConfigPageControl::qt_metacast(_clname);
}

int CameraSearch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConfigPageControl::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
