/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[99];
    char stringdata0[1647];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 12), // "sigClosePage"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 21), // "TOOLBAR_BUTTON_TYPE_e"
QT_MOC_LITERAL(4, 47, 5), // "index"
QT_MOC_LITERAL(5, 53, 19), // "sigNotifyStatusIcon"
QT_MOC_LITERAL(6, 73, 5), // "state"
QT_MOC_LITERAL(7, 79, 27), // "sigChangeToolbarButtonState"
QT_MOC_LITERAL(8, 107, 12), // "STATE_TYPE_e"
QT_MOC_LITERAL(9, 120, 12), // "currentState"
QT_MOC_LITERAL(10, 133, 24), // "sigChangeToolbarBtnState"
QT_MOC_LITERAL(11, 158, 19), // "slotOpenToolbarPage"
QT_MOC_LITERAL(12, 178, 20), // "slotCloseToolbarPage"
QT_MOC_LITERAL(13, 199, 14), // "slotDevCommGui"
QT_MOC_LITERAL(14, 214, 10), // "deviceName"
QT_MOC_LITERAL(15, 225, 13), // "DevCommParam*"
QT_MOC_LITERAL(16, 239, 5), // "param"
QT_MOC_LITERAL(17, 245, 14), // "slotEventToGui"
QT_MOC_LITERAL(18, 260, 10), // "tEventType"
QT_MOC_LITERAL(19, 271, 12), // "eventSubType"
QT_MOC_LITERAL(20, 284, 10), // "eventIndex"
QT_MOC_LITERAL(21, 295, 10), // "eventState"
QT_MOC_LITERAL(22, 306, 18), // "eventAdvanceDetail"
QT_MOC_LITERAL(23, 325, 11), // "isLiveEvent"
QT_MOC_LITERAL(24, 337, 17), // "slotPopUpEvtToGui"
QT_MOC_LITERAL(25, 355, 11), // "cameraIndex"
QT_MOC_LITERAL(26, 367, 8), // "userName"
QT_MOC_LITERAL(27, 376, 9), // "popUpTime"
QT_MOC_LITERAL(28, 386, 6), // "userId"
QT_MOC_LITERAL(29, 393, 8), // "doorName"
QT_MOC_LITERAL(30, 402, 14), // "eventCodeIndex"
QT_MOC_LITERAL(31, 417, 25), // "slotStreamRequestResponse"
QT_MOC_LITERAL(32, 443, 21), // "STREAM_COMMAND_TYPE_e"
QT_MOC_LITERAL(33, 465, 17), // "streamCommandType"
QT_MOC_LITERAL(34, 483, 19), // "StreamRequestParam*"
QT_MOC_LITERAL(35, 503, 18), // "streamRequestParam"
QT_MOC_LITERAL(36, 522, 19), // "DEVICE_REPLY_TYPE_e"
QT_MOC_LITERAL(37, 542, 11), // "deviceReply"
QT_MOC_LITERAL(38, 554, 21), // "slotOpenCameraFeature"
QT_MOC_LITERAL(39, 576, 21), // "CAMERA_FEATURE_TYPE_e"
QT_MOC_LITERAL(40, 598, 11), // "featureType"
QT_MOC_LITERAL(41, 610, 10), // "cameraInde"
QT_MOC_LITERAL(42, 621, 11), // "configParam"
QT_MOC_LITERAL(43, 633, 7), // "devName"
QT_MOC_LITERAL(44, 641, 28), // "slotCloseAnalogCameraFeature"
QT_MOC_LITERAL(45, 670, 26), // "slotApplicationModeChanged"
QT_MOC_LITERAL(46, 697, 19), // "slotCloseCosecPopUp"
QT_MOC_LITERAL(47, 717, 17), // "slotUnloadToolbar"
QT_MOC_LITERAL(48, 735, 19), // "slotHideToolbarPage"
QT_MOC_LITERAL(49, 755, 25), // "slotInfoPageButtonClicked"
QT_MOC_LITERAL(50, 781, 25), // "slotDeviceListChangeToGui"
QT_MOC_LITERAL(51, 807, 18), // "slotLoadProcessBar"
QT_MOC_LITERAL(52, 826, 22), // "slotStreamObjectDelete"
QT_MOC_LITERAL(53, 849, 15), // "DISPLAY_TYPE_e*"
QT_MOC_LITERAL(54, 865, 20), // "displayTypeForDelete"
QT_MOC_LITERAL(55, 886, 8), // "quint16*"
QT_MOC_LITERAL(56, 895, 23), // "actualWindowIdForDelete"
QT_MOC_LITERAL(57, 919, 21), // "slotMessageAlertClose"
QT_MOC_LITERAL(58, 941, 15), // "slotRaiseWidget"
QT_MOC_LITERAL(59, 957, 16), // "slotAutoAddClose"
QT_MOC_LITERAL(60, 974, 23), // "slotHddCleanInfoTimeout"
QT_MOC_LITERAL(61, 998, 25), // "slotAutoAddCamPopUpAction"
QT_MOC_LITERAL(62, 1024, 31), // "MX_AUTO_ADD_CAM_SEARCH_ACTION_e"
QT_MOC_LITERAL(63, 1056, 6), // "action"
QT_MOC_LITERAL(64, 1063, 19), // "slotPopUpAlertClose"
QT_MOC_LITERAL(65, 1083, 12), // "isQueueEmpty"
QT_MOC_LITERAL(66, 1096, 20), // "slotReportTableClose"
QT_MOC_LITERAL(67, 1117, 6), // "pageNo"
QT_MOC_LITERAL(68, 1124, 29), // "slotAutoTmznRbtInfoPgBtnClick"
QT_MOC_LITERAL(69, 1154, 16), // "slotHdmiInfoPage"
QT_MOC_LITERAL(70, 1171, 14), // "isHdmiInfoShow"
QT_MOC_LITERAL(71, 1186, 23), // "slotLanguageCfgModified"
QT_MOC_LITERAL(72, 1210, 7), // "langStr"
QT_MOC_LITERAL(73, 1218, 25), // "slotUpdateQuickbackupFlag"
QT_MOC_LITERAL(74, 1244, 4), // "flag"
QT_MOC_LITERAL(75, 1249, 20), // "slotClientAudInclude"
QT_MOC_LITERAL(76, 1270, 15), // "slotStopClntAud"
QT_MOC_LITERAL(77, 1286, 8), // "statusId"
QT_MOC_LITERAL(78, 1295, 30), // "slotStartLiveViewInfoPageClick"
QT_MOC_LITERAL(79, 1326, 15), // "slotPrevRecords"
QT_MOC_LITERAL(80, 1342, 19), // "PlaybackRecordData*"
QT_MOC_LITERAL(81, 1362, 10), // "tempRecord"
QT_MOC_LITERAL(82, 1373, 15), // "totalTempRecord"
QT_MOC_LITERAL(83, 1389, 25), // "TEMP_PLAYBACK_REC_DATA_t*"
QT_MOC_LITERAL(84, 1415, 8), // "tempData"
QT_MOC_LITERAL(85, 1424, 18), // "slotFindNextRecord"
QT_MOC_LITERAL(86, 1443, 6), // "curRec"
QT_MOC_LITERAL(87, 1450, 8), // "windIndx"
QT_MOC_LITERAL(88, 1459, 18), // "slotFindPrevRecord"
QT_MOC_LITERAL(89, 1478, 21), // "slotGetNextPrevRecord"
QT_MOC_LITERAL(90, 1500, 11), // "windowIndex"
QT_MOC_LITERAL(91, 1512, 5), // "camId"
QT_MOC_LITERAL(92, 1518, 20), // "slotUpdateUIGeometry"
QT_MOC_LITERAL(93, 1539, 8), // "isRedraw"
QT_MOC_LITERAL(94, 1548, 24), // "slotChangeAudButtonState"
QT_MOC_LITERAL(95, 1573, 17), // "slotAdvanceOption"
QT_MOC_LITERAL(96, 1591, 23), // "QUICK_BACKUP_ELEMENTS_e"
QT_MOC_LITERAL(97, 1615, 16), // "slotQuitSetupWiz"
QT_MOC_LITERAL(98, 1632, 14) // "slotOpenWizard"

    },
    "MainWindow\0sigClosePage\0\0TOOLBAR_BUTTON_TYPE_e\0"
    "index\0sigNotifyStatusIcon\0state\0"
    "sigChangeToolbarButtonState\0STATE_TYPE_e\0"
    "currentState\0sigChangeToolbarBtnState\0"
    "slotOpenToolbarPage\0slotCloseToolbarPage\0"
    "slotDevCommGui\0deviceName\0DevCommParam*\0"
    "param\0slotEventToGui\0tEventType\0"
    "eventSubType\0eventIndex\0eventState\0"
    "eventAdvanceDetail\0isLiveEvent\0"
    "slotPopUpEvtToGui\0cameraIndex\0userName\0"
    "popUpTime\0userId\0doorName\0eventCodeIndex\0"
    "slotStreamRequestResponse\0"
    "STREAM_COMMAND_TYPE_e\0streamCommandType\0"
    "StreamRequestParam*\0streamRequestParam\0"
    "DEVICE_REPLY_TYPE_e\0deviceReply\0"
    "slotOpenCameraFeature\0CAMERA_FEATURE_TYPE_e\0"
    "featureType\0cameraInde\0configParam\0"
    "devName\0slotCloseAnalogCameraFeature\0"
    "slotApplicationModeChanged\0"
    "slotCloseCosecPopUp\0slotUnloadToolbar\0"
    "slotHideToolbarPage\0slotInfoPageButtonClicked\0"
    "slotDeviceListChangeToGui\0slotLoadProcessBar\0"
    "slotStreamObjectDelete\0DISPLAY_TYPE_e*\0"
    "displayTypeForDelete\0quint16*\0"
    "actualWindowIdForDelete\0slotMessageAlertClose\0"
    "slotRaiseWidget\0slotAutoAddClose\0"
    "slotHddCleanInfoTimeout\0"
    "slotAutoAddCamPopUpAction\0"
    "MX_AUTO_ADD_CAM_SEARCH_ACTION_e\0action\0"
    "slotPopUpAlertClose\0isQueueEmpty\0"
    "slotReportTableClose\0pageNo\0"
    "slotAutoTmznRbtInfoPgBtnClick\0"
    "slotHdmiInfoPage\0isHdmiInfoShow\0"
    "slotLanguageCfgModified\0langStr\0"
    "slotUpdateQuickbackupFlag\0flag\0"
    "slotClientAudInclude\0slotStopClntAud\0"
    "statusId\0slotStartLiveViewInfoPageClick\0"
    "slotPrevRecords\0PlaybackRecordData*\0"
    "tempRecord\0totalTempRecord\0"
    "TEMP_PLAYBACK_REC_DATA_t*\0tempData\0"
    "slotFindNextRecord\0curRec\0windIndx\0"
    "slotFindPrevRecord\0slotGetNextPrevRecord\0"
    "windowIndex\0camId\0slotUpdateUIGeometry\0"
    "isRedraw\0slotChangeAudButtonState\0"
    "slotAdvanceOption\0QUICK_BACKUP_ELEMENTS_e\0"
    "slotQuitSetupWiz\0slotOpenWizard"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      43,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  229,    2, 0x06 /* Public */,
       5,    2,  232,    2, 0x06 /* Public */,
       7,    2,  237,    2, 0x06 /* Public */,
      10,    2,  242,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    1,  247,    2, 0x0a /* Public */,
      12,    1,  250,    2, 0x0a /* Public */,
      13,    2,  253,    2, 0x0a /* Public */,
      17,    7,  258,    2, 0x0a /* Public */,
      24,    7,  273,    2, 0x0a /* Public */,
      31,    3,  288,    2, 0x0a /* Public */,
      38,    5,  295,    2, 0x0a /* Public */,
      44,    0,  306,    2, 0x0a /* Public */,
      45,    0,  307,    2, 0x0a /* Public */,
      46,    0,  308,    2, 0x0a /* Public */,
      47,    0,  309,    2, 0x0a /* Public */,
      48,    0,  310,    2, 0x0a /* Public */,
      49,    1,  311,    2, 0x0a /* Public */,
      50,    0,  314,    2, 0x0a /* Public */,
      51,    0,  315,    2, 0x0a /* Public */,
      52,    2,  316,    2, 0x0a /* Public */,
      57,    0,  321,    2, 0x0a /* Public */,
      58,    0,  322,    2, 0x0a /* Public */,
      59,    0,  323,    2, 0x0a /* Public */,
      60,    0,  324,    2, 0x0a /* Public */,
      61,    1,  325,    2, 0x0a /* Public */,
      64,    2,  328,    2, 0x0a /* Public */,
      66,    1,  333,    2, 0x0a /* Public */,
      68,    1,  336,    2, 0x0a /* Public */,
      69,    1,  339,    2, 0x0a /* Public */,
      71,    1,  342,    2, 0x0a /* Public */,
      73,    1,  345,    2, 0x0a /* Public */,
      75,    0,  348,    2, 0x0a /* Public */,
      76,    1,  349,    2, 0x0a /* Public */,
      78,    1,  352,    2, 0x0a /* Public */,
      79,    3,  355,    2, 0x0a /* Public */,
      85,    2,  362,    2, 0x0a /* Public */,
      88,    2,  367,    2, 0x0a /* Public */,
      89,    3,  372,    2, 0x0a /* Public */,
      92,    1,  379,    2, 0x0a /* Public */,
      94,    0,  382,    2, 0x0a /* Public */,
      95,    1,  383,    2, 0x0a /* Public */,
      97,    0,  386,    2, 0x0a /* Public */,
      98,    0,  387,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    6,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 8,    4,    9,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 15,   14,   16,
    QMetaType::Void, QMetaType::QString, QMetaType::UChar, QMetaType::UChar, QMetaType::UChar, QMetaType::UChar, QMetaType::QString, QMetaType::Bool,   14,   18,   19,   20,   21,   22,   23,
    QMetaType::Void, QMetaType::QString, QMetaType::UChar, QMetaType::QString, QMetaType::UInt, QMetaType::QString, QMetaType::QString, QMetaType::UChar,   14,   25,   26,   27,   28,   29,   30,
    QMetaType::Void, 0x80000000 | 32, 0x80000000 | 34, 0x80000000 | 36,   33,   35,   37,
    QMetaType::Void, QMetaType::VoidStar, 0x80000000 | 39, QMetaType::UChar, QMetaType::VoidStar, QMetaType::QString,   16,   40,   41,   42,   43,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 53, 0x80000000 | 55,   54,   56,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 62,   63,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    4,   65,
    QMetaType::Void, QMetaType::UChar,   67,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Bool,   70,
    QMetaType::Void, QMetaType::QString,   72,
    QMetaType::Void, QMetaType::Bool,   74,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 36,   77,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 80, QMetaType::UChar, 0x80000000 | 83,   81,   82,   84,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort,   86,   87,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort,   86,   87,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort, QMetaType::UShort,   86,   90,   91,
    QMetaType::Void, QMetaType::Bool,   93,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 96,    4,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sigClosePage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 1: _t->sigNotifyStatusIcon((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->sigChangeToolbarButtonState((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< STATE_TYPE_e(*)>(_a[2]))); break;
        case 3: _t->sigChangeToolbarBtnState((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->slotOpenToolbarPage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 5: _t->slotCloseToolbarPage((*reinterpret_cast< TOOLBAR_BUTTON_TYPE_e(*)>(_a[1]))); break;
        case 6: _t->slotDevCommGui((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< DevCommParam*(*)>(_a[2]))); break;
        case 7: _t->slotEventToGui((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4])),(*reinterpret_cast< quint8(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7]))); break;
        case 8: _t->slotPopUpEvtToGui((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< quint32(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6])),(*reinterpret_cast< quint8(*)>(_a[7]))); break;
        case 9: _t->slotStreamRequestResponse((*reinterpret_cast< STREAM_COMMAND_TYPE_e(*)>(_a[1])),(*reinterpret_cast< StreamRequestParam*(*)>(_a[2])),(*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[3]))); break;
        case 10: _t->slotOpenCameraFeature((*reinterpret_cast< void*(*)>(_a[1])),(*reinterpret_cast< CAMERA_FEATURE_TYPE_e(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 11: _t->slotCloseAnalogCameraFeature(); break;
        case 12: _t->slotApplicationModeChanged(); break;
        case 13: _t->slotCloseCosecPopUp(); break;
        case 14: _t->slotUnloadToolbar(); break;
        case 15: _t->slotHideToolbarPage(); break;
        case 16: _t->slotInfoPageButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->slotDeviceListChangeToGui(); break;
        case 18: _t->slotLoadProcessBar(); break;
        case 19: _t->slotStreamObjectDelete((*reinterpret_cast< DISPLAY_TYPE_e*(*)>(_a[1])),(*reinterpret_cast< quint16*(*)>(_a[2]))); break;
        case 20: _t->slotMessageAlertClose(); break;
        case 21: _t->slotRaiseWidget(); break;
        case 22: _t->slotAutoAddClose(); break;
        case 23: _t->slotHddCleanInfoTimeout(); break;
        case 24: _t->slotAutoAddCamPopUpAction((*reinterpret_cast< MX_AUTO_ADD_CAM_SEARCH_ACTION_e(*)>(_a[1]))); break;
        case 25: _t->slotPopUpAlertClose((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 26: _t->slotReportTableClose((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 27: _t->slotAutoTmznRbtInfoPgBtnClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: _t->slotHdmiInfoPage((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 29: _t->slotLanguageCfgModified((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 30: _t->slotUpdateQuickbackupFlag((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 31: _t->slotClientAudInclude(); break;
        case 32: _t->slotStopClntAud((*reinterpret_cast< DEVICE_REPLY_TYPE_e(*)>(_a[1]))); break;
        case 33: _t->slotStartLiveViewInfoPageClick((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 34: _t->slotPrevRecords((*reinterpret_cast< PlaybackRecordData*(*)>(_a[1])),(*reinterpret_cast< quint8(*)>(_a[2])),(*reinterpret_cast< TEMP_PLAYBACK_REC_DATA_t*(*)>(_a[3]))); break;
        case 35: _t->slotFindNextRecord((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 36: _t->slotFindPrevRecord((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 37: _t->slotGetNextPrevRecord((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        case 38: _t->slotUpdateUIGeometry((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 39: _t->slotChangeAudButtonState(); break;
        case 40: _t->slotAdvanceOption((*reinterpret_cast< QUICK_BACKUP_ELEMENTS_e(*)>(_a[1]))); break;
        case 41: _t->slotQuitSetupWiz(); break;
        case 42: _t->slotOpenWizard(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)(TOOLBAR_BUTTON_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::sigClosePage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(TOOLBAR_BUTTON_TYPE_e , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::sigNotifyStatusIcon)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(TOOLBAR_BUTTON_TYPE_e , STATE_TYPE_e );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::sigChangeToolbarButtonState)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(TOOLBAR_BUTTON_TYPE_e , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::sigChangeToolbarBtnState)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 43)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 43;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 43)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 43;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::sigClosePage(TOOLBAR_BUTTON_TYPE_e _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MainWindow::sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e _t1, STATE_TYPE_e _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MainWindow::sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
