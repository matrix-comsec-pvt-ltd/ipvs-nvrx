/****************************************************************************
** Meta object code from reading C++ file 'DisplaySetting.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../DisplaySetting.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DisplaySetting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DisplaySetting_t {
    QByteArrayData data[64];
    char stringdata0[1081];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DisplaySetting_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DisplaySetting_t qt_meta_stringdata_DisplaySetting = {
    {
QT_MOC_LITERAL(0, 0, 14), // "DisplaySetting"
QT_MOC_LITERAL(1, 15, 15), // "sigChangeLayout"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 13), // "LAYOUT_TYPE_e"
QT_MOC_LITERAL(4, 46, 5), // "index"
QT_MOC_LITERAL(5, 52, 14), // "DISPLAY_TYPE_e"
QT_MOC_LITERAL(6, 67, 5), // "disId"
QT_MOC_LITERAL(7, 73, 11), // "windowIndex"
QT_MOC_LITERAL(8, 85, 14), // "ifAtcualWindow"
QT_MOC_LITERAL(9, 100, 12), // "ifUpdatePage"
QT_MOC_LITERAL(10, 113, 28), // "sigProcessApplyStyleToLayout"
QT_MOC_LITERAL(11, 142, 11), // "displayType"
QT_MOC_LITERAL(12, 154, 16), // "DISPLAY_CONFIG_t"
QT_MOC_LITERAL(13, 171, 15), // "m_displayConfig"
QT_MOC_LITERAL(14, 187, 12), // "STYLE_TYPE_e"
QT_MOC_LITERAL(15, 200, 7), // "styleNo"
QT_MOC_LITERAL(16, 208, 27), // "sigToolbarStyleChnageNotify"
QT_MOC_LITERAL(17, 236, 20), // "sigliveViewAudiostop"
QT_MOC_LITERAL(18, 257, 25), // "slotUpadateCurrentElement"
QT_MOC_LITERAL(19, 283, 23), // "slotPickListButtonClick"
QT_MOC_LITERAL(20, 307, 30), // "slotSettingOptSelButtonClicked"
QT_MOC_LITERAL(21, 338, 19), // "OPTION_STATE_TYPE_e"
QT_MOC_LITERAL(22, 358, 5), // "state"
QT_MOC_LITERAL(23, 364, 24), // "slotPicklistValueChanged"
QT_MOC_LITERAL(24, 389, 5), // "value"
QT_MOC_LITERAL(25, 395, 11), // "indexInPage"
QT_MOC_LITERAL(26, 407, 16), // "slotChangeLayout"
QT_MOC_LITERAL(27, 424, 8), // "layoutId"
QT_MOC_LITERAL(28, 433, 19), // "slotCnfgButtonClick"
QT_MOC_LITERAL(29, 453, 24), // "slotInfoPageCnfgBtnClick"
QT_MOC_LITERAL(30, 478, 21), // "slotNextPrevPageClick"
QT_MOC_LITERAL(31, 500, 24), // "slotStyleSelCnfgBtnClick"
QT_MOC_LITERAL(32, 525, 18), // "slotWindowSelected"
QT_MOC_LITERAL(33, 544, 22), // "slotWindowImageClicked"
QT_MOC_LITERAL(34, 567, 19), // "WINDOW_IMAGE_TYPE_e"
QT_MOC_LITERAL(35, 587, 9), // "imageType"
QT_MOC_LITERAL(36, 597, 20), // "slotWindowImageHover"
QT_MOC_LITERAL(37, 618, 7), // "isHover"
QT_MOC_LITERAL(38, 626, 15), // "slotSwapWindows"
QT_MOC_LITERAL(39, 642, 11), // "firstWindow"
QT_MOC_LITERAL(40, 654, 12), // "secondWindow"
QT_MOC_LITERAL(41, 667, 22), // "slotDragStartStopEvent"
QT_MOC_LITERAL(42, 690, 7), // "isStart"
QT_MOC_LITERAL(43, 698, 39), // "slotWindowResponseToDisplaySe..."
QT_MOC_LITERAL(44, 738, 9), // "displayId"
QT_MOC_LITERAL(45, 748, 10), // "deviceName"
QT_MOC_LITERAL(46, 759, 8), // "cameraId"
QT_MOC_LITERAL(47, 768, 8), // "windowId"
QT_MOC_LITERAL(48, 777, 25), // "slotAppearanceButtonClick"
QT_MOC_LITERAL(49, 803, 16), // "slotObjectDelete"
QT_MOC_LITERAL(50, 820, 25), // "slotLayoutResponseOnApply"
QT_MOC_LITERAL(51, 846, 14), // "isCurrentStyle"
QT_MOC_LITERAL(52, 861, 23), // "slotCameraButtonClicked"
QT_MOC_LITERAL(53, 885, 11), // "cameraIndex"
QT_MOC_LITERAL(54, 897, 19), // "CAMERA_STATE_TYPE_e"
QT_MOC_LITERAL(55, 917, 15), // "connectionState"
QT_MOC_LITERAL(56, 933, 14), // "pageSwitchFlag"
QT_MOC_LITERAL(57, 948, 17), // "isChangeSelection"
QT_MOC_LITERAL(58, 966, 25), // "slotPageNumberButtonClick"
QT_MOC_LITERAL(59, 992, 19), // "slotOkButtonClicked"
QT_MOC_LITERAL(60, 1012, 8), // "username"
QT_MOC_LITERAL(61, 1021, 8), // "password"
QT_MOC_LITERAL(62, 1030, 26), // "slotCameraConfigListUpdate"
QT_MOC_LITERAL(63, 1057, 23) // "slotSpinBoxValueChanged"

    },
    "DisplaySetting\0sigChangeLayout\0\0"
    "LAYOUT_TYPE_e\0index\0DISPLAY_TYPE_e\0"
    "disId\0windowIndex\0ifAtcualWindow\0"
    "ifUpdatePage\0sigProcessApplyStyleToLayout\0"
    "displayType\0DISPLAY_CONFIG_t\0"
    "m_displayConfig\0STYLE_TYPE_e\0styleNo\0"
    "sigToolbarStyleChnageNotify\0"
    "sigliveViewAudiostop\0slotUpadateCurrentElement\0"
    "slotPickListButtonClick\0"
    "slotSettingOptSelButtonClicked\0"
    "OPTION_STATE_TYPE_e\0state\0"
    "slotPicklistValueChanged\0value\0"
    "indexInPage\0slotChangeLayout\0layoutId\0"
    "slotCnfgButtonClick\0slotInfoPageCnfgBtnClick\0"
    "slotNextPrevPageClick\0slotStyleSelCnfgBtnClick\0"
    "slotWindowSelected\0slotWindowImageClicked\0"
    "WINDOW_IMAGE_TYPE_e\0imageType\0"
    "slotWindowImageHover\0isHover\0"
    "slotSwapWindows\0firstWindow\0secondWindow\0"
    "slotDragStartStopEvent\0isStart\0"
    "slotWindowResponseToDisplaySettingsPage\0"
    "displayId\0deviceName\0cameraId\0windowId\0"
    "slotAppearanceButtonClick\0slotObjectDelete\0"
    "slotLayoutResponseOnApply\0isCurrentStyle\0"
    "slotCameraButtonClicked\0cameraIndex\0"
    "CAMERA_STATE_TYPE_e\0connectionState\0"
    "pageSwitchFlag\0isChangeSelection\0"
    "slotPageNumberButtonClick\0slotOkButtonClicked\0"
    "username\0password\0slotCameraConfigListUpdate\0"
    "slotSpinBoxValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DisplaySetting[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      27,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    5,  149,    2, 0x06 /* Public */,
      10,    3,  160,    2, 0x06 /* Public */,
      16,    1,  167,    2, 0x06 /* Public */,
      17,    0,  170,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      18,    1,  171,    2, 0x0a /* Public */,
      19,    1,  174,    2, 0x0a /* Public */,
      20,    2,  177,    2, 0x0a /* Public */,
      23,    3,  182,    2, 0x0a /* Public */,
      26,    1,  189,    2, 0x0a /* Public */,
      28,    1,  192,    2, 0x0a /* Public */,
      29,    1,  195,    2, 0x0a /* Public */,
      30,    1,  198,    2, 0x0a /* Public */,
      31,    2,  201,    2, 0x0a /* Public */,
      32,    1,  206,    2, 0x0a /* Public */,
      33,    2,  209,    2, 0x0a /* Public */,
      36,    3,  214,    2, 0x0a /* Public */,
      38,    2,  221,    2, 0x0a /* Public */,
      41,    1,  226,    2, 0x0a /* Public */,
      43,    4,  229,    2, 0x0a /* Public */,
      48,    1,  238,    2, 0x0a /* Public */,
      49,    0,  241,    2, 0x0a /* Public */,
      50,    2,  242,    2, 0x0a /* Public */,
      52,    5,  247,    2, 0x0a /* Public */,
      58,    1,  258,    2, 0x0a /* Public */,
      59,    2,  261,    2, 0x0a /* Public */,
      62,    0,  266,    2, 0x0a /* Public */,
      63,    2,  267,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::UShort, QMetaType::Bool, QMetaType::Bool,    4,    6,    7,    8,    9,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 12, 0x80000000 | 14,   11,   13,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 21, QMetaType::Int,   22,    4,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, QMetaType::Int,    4,   24,   25,
    QMetaType::Void, 0x80000000 | 3,   27,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::UChar,    4,   15,
    QMetaType::Void, QMetaType::UShort,    4,
    QMetaType::Void, 0x80000000 | 34, QMetaType::UShort,   35,    7,
    QMetaType::Void, 0x80000000 | 34, QMetaType::UShort, QMetaType::Bool,   35,    7,   37,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort,   39,   40,
    QMetaType::Void, QMetaType::Bool,   42,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QString, QMetaType::UChar, QMetaType::UShort,   44,   45,   46,   47,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Bool,   11,   51,
    QMetaType::Void, QMetaType::UChar, QMetaType::QString, 0x80000000 | 54, QMetaType::Bool, QMetaType::Bool,   53,   45,   55,   56,   57,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   60,   61,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt,    2,    4,

       0        // eod
};

void DisplaySetting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DisplaySetting *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigChangeLayout((*reinterpret_cast< LAYOUT_TYPE_e(*)>(_a[1])),(*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 1: _t->sigProcessApplyStyleToLayout((*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< DISPLAY_CONFIG_t(*)>(_a[2])),(*reinterpret_cast< STYLE_TYPE_e(*)>(_a[3]))); break;
        case 2: _t->sigToolbarStyleChnageNotify((*reinterpret_cast< STYLE_TYPE_e(*)>(_a[1]))); break;
        case 3: _t->sigliveViewAudiostop(); break;
        case 4: _t->slotUpadateCurrentElement((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotPickListButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotSettingOptSelButtonClicked((*reinterpret_cast< OPTION_STATE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->slotPicklistValueChanged((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 8: _t->slotChangeLayout((*reinterpret_cast< LAYOUT_TYPE_e(*)>(_a[1]))); break;
        case 9: _t->slotCnfgButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slotInfoPageCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->slotNextPrevPageClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->slotStyleSelCnfgBtnClick((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2]))); break;
        case 13: _t->slotWindowSelected((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 14: _t->slotWindowImageClicked((*reinterpret_cast< WINDOW_IMAGE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 15: _t->slotWindowImageHover((*reinterpret_cast< WINDOW_IMAGE_TYPE_e(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 16: _t->slotSwapWindows((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 17: _t->slotDragStartStopEvent((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->slotWindowResponseToDisplaySettingsPage((*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< quint16(*)>(_a[4]))); break;
        case 19: _t->slotAppearanceButtonClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->slotObjectDelete(); break;
        case 21: _t->slotLayoutResponseOnApply((*reinterpret_cast< DISPLAY_TYPE_e(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 22: _t->slotCameraButtonClicked((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< CAMERA_STATE_TYPE_e(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 23: _t->slotPageNumberButtonClick((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 24: _t->slotOkButtonClicked((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 25: _t->slotCameraConfigListUpdate(); break;
        case 26: _t->slotSpinBoxValueChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DisplaySetting::*)(LAYOUT_TYPE_e , DISPLAY_TYPE_e , quint16 , bool , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplaySetting::sigChangeLayout)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DisplaySetting::*)(DISPLAY_TYPE_e , DISPLAY_CONFIG_t , STYLE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplaySetting::sigProcessApplyStyleToLayout)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DisplaySetting::*)(STYLE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplaySetting::sigToolbarStyleChnageNotify)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DisplaySetting::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplaySetting::sigliveViewAudiostop)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DisplaySetting::staticMetaObject = { {
    QMetaObject::SuperData::link<BackGround::staticMetaObject>(),
    qt_meta_stringdata_DisplaySetting.data,
    qt_meta_data_DisplaySetting,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DisplaySetting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisplaySetting::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DisplaySetting.stringdata0))
        return static_cast<void*>(this);
    return BackGround::qt_metacast(_clname);
}

int DisplaySetting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BackGround::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 27;
    }
    return _id;
}

// SIGNAL 0
void DisplaySetting::sigChangeLayout(LAYOUT_TYPE_e _t1, DISPLAY_TYPE_e _t2, quint16 _t3, bool _t4, bool _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DisplaySetting::sigProcessApplyStyleToLayout(DISPLAY_TYPE_e _t1, DISPLAY_CONFIG_t _t2, STYLE_TYPE_e _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DisplaySetting::sigToolbarStyleChnageNotify(STYLE_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DisplaySetting::sigliveViewAudiostop()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
