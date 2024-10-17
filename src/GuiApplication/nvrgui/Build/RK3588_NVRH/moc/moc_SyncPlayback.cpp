/****************************************************************************
** Meta object code from reading C++ file 'SyncPlayback.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../SyncPlayback.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncPlayback.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SyncPlayback_t {
    QByteArrayData data[45];
    char stringdata0[838];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SyncPlayback_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SyncPlayback_t qt_meta_stringdata_SyncPlayback = {
    {
QT_MOC_LITERAL(0, 0, 12), // "SyncPlayback"
QT_MOC_LITERAL(1, 13, 12), // "sigClosePage"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 49, 5), // "index"
QT_MOC_LITERAL(5, 55, 15), // "sigChangeLayout"
QT_MOC_LITERAL(6, 71, 13), // "LAYOUT_TYPE_e"
QT_MOC_LITERAL(7, 85, 6), // "layout"
QT_MOC_LITERAL(8, 92, 14), // "DISPLAY_TYPE_e"
QT_MOC_LITERAL(9, 107, 9), // "displayId"
QT_MOC_LITERAL(10, 117, 11), // "windowIndex"
QT_MOC_LITERAL(11, 129, 14), // "ifAtcualWindow"
QT_MOC_LITERAL(12, 144, 12), // "ifUpdatePage"
QT_MOC_LITERAL(13, 157, 22), // "slotCloseButtonClicked"
QT_MOC_LITERAL(14, 180, 24), // "slotUpdateCurrentElement"
QT_MOC_LITERAL(15, 205, 21), // "slotDeviceNameChanged"
QT_MOC_LITERAL(16, 227, 6), // "string"
QT_MOC_LITERAL(17, 234, 26), // "slotRecDriveDropBoxChanged"
QT_MOC_LITERAL(18, 261, 34), // "slotFocusChangedFromCurrentEl..."
QT_MOC_LITERAL(19, 296, 17), // "isPreviousElement"
QT_MOC_LITERAL(20, 314, 24), // "slotFetchNewSelectedDate"
QT_MOC_LITERAL(21, 339, 25), // "slotFetchRecordForNewDate"
QT_MOC_LITERAL(22, 365, 23), // "slotSearchButtonClicked"
QT_MOC_LITERAL(23, 389, 25), // "slotCameraCheckboxClicked"
QT_MOC_LITERAL(24, 415, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(25, 435, 12), // "iButtonState"
QT_MOC_LITERAL(26, 448, 11), // "indexInPage"
QT_MOC_LITERAL(27, 460, 26), // "slotRecTypeCheckboxClicked"
QT_MOC_LITERAL(28, 487, 21), // "slotHourFormatChanged"
QT_MOC_LITERAL(29, 509, 12), // "currentState"
QT_MOC_LITERAL(30, 522, 25), // "slotSliderPositionChanged"
QT_MOC_LITERAL(31, 548, 30), // "slotSliderPositionChangedStart"
QT_MOC_LITERAL(32, 579, 24), // "slotToolbarButtonClicked"
QT_MOC_LITERAL(33, 604, 28), // "SYNCPB_TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(34, 633, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(35, 646, 5), // "state"
QT_MOC_LITERAL(36, 652, 27), // "slotCropAndBackupPageClosed"
QT_MOC_LITERAL(37, 680, 36), // "slotPreProcessingDoneForSyncP..."
QT_MOC_LITERAL(38, 717, 17), // "slotLayoutChanged"
QT_MOC_LITERAL(39, 735, 23), // "slotExitFromZoomFeature"
QT_MOC_LITERAL(40, 759, 18), // "slotScrollbarClick"
QT_MOC_LITERAL(41, 778, 13), // "numberOfSteps"
QT_MOC_LITERAL(42, 792, 16), // "slotRetryTimeOut"
QT_MOC_LITERAL(43, 809, 16), // "slotchangeLayout"
QT_MOC_LITERAL(44, 826, 11) // "iLayoutType"

    },
    "SyncPlayback\0sigClosePage\0\0"
    "TOOLBAR_BUTTON_TYPE_e\0index\0sigChangeLayout\0"
    "LAYOUT_TYPE_e\0layout\0DISPLAY_TYPE_e\0"
    "displayId\0windowIndex\0ifAtcualWindow\0"
    "ifUpdatePage\0slotCloseButtonClicked\0"
    "slotUpdateCurrentElement\0slotDeviceNameChanged\0"
    "string\0slotRecDriveDropBoxChanged\0"
    "slotFocusChangedFromCurrentElement\0"
    "isPreviousElement\0slotFetchNewSelectedDate\0"
    "slotFetchRecordForNewDate\0"
    "slotSearchButtonClicked\0"
    "slotCameraCheckboxClicked\0OPTION_STATE_TYPE_e\0"
    "iButtonState\0indexInPage\0"
    "slotRecTypeCheckboxClicked\0"
    "slotHourFormatChanged\0currentState\0"
    "slotSliderPositionChanged\0"
    "slotSliderPositionChangedStart\0"
    "slotToolbarButtonClicked\0"
    "SYNCPB_TOOLBAR_BUTTON_TYPE_e\0STATE_TYPE_e\0"
    "state\0slotCropAndBackupPageClosed\0"
    "slotPreProcessingDoneForSyncPlayback\0"
    "slotLayoutChanged\0slotExitFromZoomFeature\0"
    "slotScrollbarClick\0numberOfSteps\0"
    "slotRetryTimeOut\0slotchangeLayout\0"
    "iLayoutType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SyncPlayback[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  129,    2, 0x06 /* Public */,
       5,    5,  132,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    1,  143,    2, 0x0a /* Public */,
      14,    1,  146,    2, 0x0a /* Public */,
      15,    2,  149,    2, 0x0a /* Public */,
      17,    2,  154,    2, 0x0a /* Public */,
      18,    1,  159,    2, 0x0a /* Public */,
      20,    0,  162,    2, 0x0a /* Public */,
      21,    0,  163,    2, 0x0a /* Public */,
      22,    1,  164,    2, 0x0a /* Public */,
      23,    2,  167,    2, 0x0a /* Public */,
      27,    2,  172,    2, 0x0a /* Public */,
      28,    2,  177,    2, 0x0a /* Public */,
      30,    0,  182,    2, 0x0a /* Public */,
      31,    0,  183,    2, 0x0a /* Public */,
      32,    2,  184,    2, 0x0a /* Public */,
      36,    1,  189,    2, 0x0a /* Public */,
      37,    0,  192,    2, 0x0a /* Public */,
      38,    0,  193,    2, 0x0a /* Public */,
      39,    0,  194,    2, 0x0a /* Public */,
      40,    1,  195,    2, 0x0a /* Public */,
      42,    0,  198,    2, 0x0a /* Public */,
      43,    1,  199,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 8, QMetaType::UShort, QMetaType::Bool, QMetaType::Bool,    7,    9,   10,   11,   12,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,   16,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,   16,    2,
    QMetaType::Void, QMetaType::Bool,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 24, QMetaType::Int,   25,   26,
    QMetaType::Void, 0x80000000 | 24, QMetaType::Int,   25,   26,
    QMetaType::Void, 0x80000000 | 24, QMetaType::Int,   29,   26,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 33, 0x80000000 | 34,    4,   35,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   41,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,   44,

       0        // eod
};

void SyncPlayback::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SyncPlayback *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigClosePage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 1: _t->sigChangeLayout((*reinterpret_cast< LAYOUT_TYPE_e(*)>(_a[1])),(*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 2: _t->slotCloseButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slotUpdateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotDeviceNameChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 5: _t->slotRecDriveDropBoxChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 6: _t->slotFocusChangedFromCurrentElement((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->slotFetchNewSelectedDate(); break;
        case 8: _t->slotFetchRecordForNewDate(); break;
        case 9: _t->slotSearchButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slotCameraCheckboxClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: _t->slotRecTypeCheckboxClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 12: _t->slotHourFormatChanged((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 13: _t->slotSliderPositionChanged(); break;
        case 14: _t->slotSliderPositionChangedStart(); break;
        case 15: _t->slotToolbarButtonClicked((*reinterpret_cast< SYNCPB_TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 16: _t->slotCropAndBackupPageClosed((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 17: _t->slotPreProcessingDoneForSyncPlayback(); break;
        case 18: _t->slotLayoutChanged(); break;
        case 19: _t->slotExitFromZoomFeature(); break;
        case 20: _t->slotScrollbarClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->slotRetryTimeOut(); break;
        case 22: _t->slotchangeLayout((*reinterpret_cast< LAYOUT_TYPE_e(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SyncPlayback::*)(TOOLBAR_BUTTON_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlayback::sigClosePage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SyncPlayback::*)(LAYOUT_TYPE_e , DISPLAY_TYPE_e , quint16 , bool , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SyncPlayback::sigChangeLayout)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SyncPlayback::staticMetaObject = { {
    QMetaObject::SuperData::link<KeyBoard::staticMetaObject>(),
    qt_meta_stringdata_SyncPlayback.data,
    qt_meta_data_SyncPlayback,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SyncPlayback::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SyncPlayback::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SyncPlayback.stringdata0))
        return static_cast<void*>(this);
    return KeyBoard::qt_metacast(_clname);
}

int SyncPlayback::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = KeyBoard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 23;
    }
    return _id;
}

// SIGNAL 0
void SyncPlayback::sigClosePage(TOOLBAR_BUTTON_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SyncPlayback::sigChangeLayout(LAYOUT_TYPE_e _t1, DISPLAY_TYPE_e _t2, quint16 _t3, bool _t4, bool _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
