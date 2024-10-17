#ifndef APPLCONTROLLER_H
#define APPLCONTROLLER_H
///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : Multi-NVR GUI
//   Owner        : Shruti Sahni
//   File         : ApplController.h
//   Description  : This file contains ApplController class
//                  which behaves as interface between between GUI(front end)
//                  and the backend.
/////////////////////////////////////////////////////////////////////////////
//*** Place all include files here ***
#include <QtCore>
#include <QApplication>
#include "DataStructure.h"
#include "DeviceClient/DeviceClient.h"
#include "Configuration/DeviceConfig.h"
#include "Configuration/AudioConfig.h"
#include "Configuration/DisplayConfig.h"
#include "DeviceClient/StreamRequest/StreamRequest.h"
/////////////////////////////////////////////////////////////////////////////
//Class:  ApplController
//Description:
//      This is the class which behaves as an interface between gui and
//      backend.This class is registered in the QML system with the
//      ApplControl 1.0. Any click or other user interaction with GUI invokes
//      this class processActivity function,  which furthur calls other
//      functions of the class according to the request made by GUI.
//      Any information to be sent to GUI is sent by signal through this class
//[Pre-condition:] (optional)
//       It is inherited by QDeclarative so it can be accessed by qml.
/////////////////////////////////////////////////////////////////////////////
void hdmiCallBack(BOOL IsHdmiInfoShow);

class ApplController : public QObject
{
    //macro must appear that declares its own signals and slots or that uses
    //other services provided by Qt's meta-object system.
    Q_OBJECT
private:
    //******Data Types ******
    explicit ApplController(QObject* parent = 0);              //constructor
     ~ApplController();

    static ApplController*                  applController;
    DeviceClient*                           deviceClient[MAX_DEVICES];    //device client objects
    StreamRequest*                          streamRequest[MAX_STREAM_SESSION];
    DisplayConfig                           displayCfg;
    AudioConfig                             audioCfg;
    DeviceConfig                            deviceCfg;

    // read write access lock for device information
    QReadWriteLock                          devInfoLock;
    QStringList                             deviceList;
    QThread                                 deviceThread[MAX_DEVICES];

    quint8                                  lastStreamRequestIndex;
    QMutex                                  streamRequestAllocationMutex;
    bool                                    isStreamRequestAllocated[MAX_STREAM_SESSION];
    QThread                                 streamRequestThread[MAX_STREAM_SESSION];

    bool                                    m_getUserRightsResponse;

    quint32                                 m_screenHeight;
    quint32                                 m_screenWidth;
    quint32                                 m_screenXPos;
    quint32                                 m_screenYPos;
    DISPLAY_RESOLUTION_e                    currentDisplayResolution;

    void deleteDeviceClient(quint8 index);
    bool findFreeStreamSession(quint8 &streamId);
    void createStreamRequest(quint8 streamId);
    void deleteStreamRequest(quint8 streamId);

public:
    //******** Function Prototypes ******
    static ApplController* getInstance();
    bool processActivity(QString deviceName, ACTIVITY_TYPE_e activityType, QList<QVariant> &paramList, void* param);
    bool processActivity(QString deviceName, ACTIVITY_TYPE_e activityType, void* param);
    bool processActivity(ACTIVITY_TYPE_e activityType, void* param);
    bool processActivity(ACTIVITY_TYPE_e activityType, QList<QVariant> &paramList, void* param);
    quint8 processStreamActivity(STREAM_COMMAND_TYPE_e streamCommandType, SERVER_SESSION_INFO_t serverInfo,
                                 StreamRequestParam *streamRequestParam, quint8 *streamIndexOut = NULL);
    quint8 processStreamActivity(STREAM_COMMAND_TYPE_e streamCommandType, SERVER_SESSION_INFO_t serverInfo,
                                 StreamRequestParam *streamRequestParam, SERVER_SESSION_INFO_t nextServerInfo,
                                 StreamRequestParam *nextStreamRequestParam);
    bool GetTotalCamera(QString devName, quint8 &cameraCount);
    void GetDevModel(QString devName, quint8 &devModel);
    QString GetCameraNameOfDevice(QString deviceName, quint8 cameraId);
    bool GetCameraInfoOfDevice(QString deviceName, quint8 cameraId, DEV_CAM_INFO_t &cameraInfo);
    bool GetDeviceInfo(QString deviceName, DEV_TABLE_INFO_t &deviceInfo);
    bool GetVideoStandard(QString deviceName, VIDEO_STANDARD_e &videoStandard);
    void GetEnableDevList(quint8 &deviceCount, QStringList &list);
    bool getDeviceIndex(QString devName, quint8 &deviceIndex);
    bool IsValidDeviceConnState(QString devName, quint8 &deviceIndex);
    bool IsValidDeviceConnState(quint8 deviceIndex);
    CAMERA_TYPE_e GetCameraType(QString deviceName, quint8 cameraId);
    DEVICE_STATE_TYPE_e GetDeviceConnectionState(QString devName);
    void GetConfiguredDeviceList(quint8 &deviceCount, QStringList &list);
    quint16 GetTotalCameraOfEnableDevices();
    bool GetHlthStatusAll(QString devName, quint8 *list);
    bool UpdateHlthStatusAll(QString devName, QString param);
    bool GetHlthStatusSingleParam(QString deviceName, quint8 *list, quint8 paramIndex);
    bool GetHealtStatusSingleParamSingleCamera(QString deviceName, quint8 &paramValue, quint8 paramIndex, quint8 cameraIndex);
    bool GetDeviceEventList(QString deviceName, QStringList &eventList);
    bool DeleteDeviceEventList(QString deviceName, QStringList eventList);
    bool getPasswordFrmDev(QString devName, QString &password);
    bool getUsernameFrmDev(QString devName, QString &username);
    bool getLiveStreamTypeFrmDev(QString devName, quint8 &liveStreamType);
    void getFirmwareVersion(QString devName,QString &version);
    bool GetAllCameraNamesOfDevice(QString deviceName, QStringList &camNameList,
                                   CAM_LIST_TYPE_e listType = MAX_CAM_LIST_TYPE, quint8 *keys=NULL);
    bool GetDeviceAutoLoginstate(QString devName);
    bool GetDevicePreferNativeDeviceState(QString devName);
    bool GetServerSessionInfo(QString deviceName, SERVER_SESSION_INFO_t &serverSessionInfo);
    bool GetPreVideoLossDuration(QString deviceName, quint8 &videoLossDuration);
    quint64 GetSyncPlaybackMonthRecord();
	quint8 GetCamRecType(quint8 iCameraId);
	bool GetCamRecData(quint8 cameraIndex, QList<quint16> &recordList);
	bool IsCamRecAvailable(quint8 iCameraId, quint16 iStartTime, quint16 iEndTime);
    void GetMotionInfo(quint32 *motionInfo);
    DISPLAY_RESOLUTION_e readResolution(DISPLAY_TYPE_e disId);
    void writeResolution(DISPLAY_TYPE_e disId, DISPLAY_RESOLUTION_e resolutionId);
    DISPLAY_RESOLUTION_e getCurrentDisplayResolution(void);
    void setCurrentDisplayResolution(DISPLAY_RESOLUTION_e resolutionId);
    bool writeMaxWindowsForDisplay(quint16 windowCount);
    bool readMaxWindowsForDisplay(quint16 &windowCount);
    bool readBandwidthOptFlag();
    bool writeBandwidthOptFlag(bool flag);
    LIVE_VIEW_TYPE_e GetLiveViewType(void);
    bool SetLiveViewType(LIVE_VIEW_TYPE_e liveViewType);
    bool readTVApperanceParameters(quint32 &tvAdjustParam);
    bool writeTVApperanceParameters(quint32 &tvAdjustParam);
    void setDisplayParameters(PHYSICAL_DISPLAY_TYPE_e displayType, PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e paramIndex, quint32 paramValue);
    bool readDisplayParameters(PHYSICAL_DISPLAY_TYPE_e displayType, DISPLAY_PARAM_t &displayParam);
    bool writeDisplayParameters(PHYSICAL_DISPLAY_TYPE_e displayType, DISPLAY_PARAM_t &displayParam);
    void setTVApperanceParameters(UINT32 offset);
    bool swapWindows(DISPLAY_TYPE_e displayId, StreamRequestParam firstParam, StreamRequestParam secondParam);
    bool ShiftWindows(DISPLAY_TYPE_e displayId, qint8 offset, quint8 newSelectedWindow, quint8 newLayout);
    void deleteStreamRequestForDevice(QString deviceName);
    bool GetUserGroupType(QString deviceName, USRS_GROUP_e &userType);
    bool SetUserGroupType(QString deviceName, USRS_GROUP_e userType);
    bool GetMaxSensorInputSupport(QString deviceName, quint8 &totalInput);
    bool GetMaxAlarmOutputSupport(QString deviceName, quint8 &totalOutput);
    bool GetAutoCloseRecFailAlertFlag(QString deviceName, bool &autoCloseFlag);
    void GetCameraRights(QString devName, quint8 camIndex, CAM_LIST_TYPE_e listType, bool &camRights);
    bool SetCameraRights(QString devName,quint8 camIndex, quint8 cameraRights);
    void hdmiLoadInfo(bool isHdmiInfoShow);
    void setUserRightsResponseNotify(bool getUserRightsFlag);
    bool getUserRightsResponseNotify();    
    static void deleteAppCntrollrInstance();
    static quint32 getHeightOfScreen();
    static quint32 getWidthOfScreen();
    static quint32 getXPosOfScreen();
    static quint32 getYPosOfScreen();
    bool getOriginofScreen(LAYOUT_TYPE_e layoutIndex, DISPLAY_TYPE_e displayType);
    void readAudioConfig();
    void writeAudioConfig(int currentState, int audioLevel);
    void GetDevNameDropdownMapList(QMap<quint8, QString> &deviceMapList);
    QString GetRealDeviceName(QString deviceName);
    QString GetDispDeviceName(QString deviceName);
    QString GetLocalDeviceNameInfo(void);

    int m_audioLevel, m_currentMuteState;

signals:
    //signal to emit device reply to gui
    void sigDevCommGui(QString deviceName, DevCommParam* param);
    void sigEventToGui(QString deviceIndex, quint8 evtType, quint8 evtSubType, quint8 evtIndex, quint8 evtState,
                       QString evtAdvanceDetail, bool isLvEvt);
    void sigPopUpEventToGui(QString devName, quint8 camNo, QString userName, quint32 popUpTime, QString userId,
                            QString doorName, quint8 evtCode);
    void sigStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam,
                                  DEVICE_REPLY_TYPE_e deviceReply);
    void sigDeviceListChangeToGui();
    void sigLanguageCfgModified(QString str);
    void sigStreamObjectDelete(DISPLAY_TYPE_e *displayTypeForDelete, quint16 *actualWindowIdForDelete);
	void sigHdmiInfoPage(bool isHdmiInfoShow);    
    void sigChangeAudButtonState();

public slots:
    //slot to catch the device client's device response signal
    void devCommActivitySlot(QString deviceName, DevCommParam* param);
    void slotAudioCfgChanged(const AUDIO_CONFIG_t *currentConfig, const AUDIO_CONFIG_t *newConfig);
    void slotDeviceCfgUpdate(quint8 remoteDeviceIndex, DEVICE_CONFIG_t* deviceConfig, bool isUpdateOnLiveEvent);
    void slotDeviceCfgChanged(quint8 devIndex, const DEVICE_CONFIG_t *currentConfig, const DEVICE_CONFIG_t *newConfig);
    void slotLanguageCfgChanged(QString str);
    void slotEvent(QString devIndex, LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubType, quint8 camIndx,
                   LOG_EVENT_STATE_e eventState, QString eventAdvanceDetail, bool isLiveEvent);
    void slotPopUpEvent(QString devIndex, quint8 camIndex, QString usrNameStr, quint32 popUpTimeStr, QString userId,
                        QString doorName, quint8 evtCodeIndex);
    void slotStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam,
                                   DEVICE_REPLY_TYPE_e deviceReply);
    void slotDeleteStreamRequest(quint8 streamId);
    void slotDeleteStreamRequest(QString deviceName);
    void slotDelMedControl(quint8 streamId);
    void slotChangeAudState();
};

#endif // APPLCONTROLLER_H
