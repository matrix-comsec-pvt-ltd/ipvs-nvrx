#ifndef DEVICECLIENT_H
#define DEVICECLIENT_H
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
//   Project      : NVR [Network Video Recorder]
//   Owner        : Shruti Sahni
//                : Aekam Parmar
//   File         : deviceClient.h
//   Description  : This file contains the DeviceInfo class and DeviceClient
//                  Thread class declaration. Device Info class contains all
//                  the device related parameters and the Device Client Thread
//                  class contains the deviceInfo instance, polling thread and
//                  device request thread instance.This class initiates the
//                  polling thread. It also initiates the device request thread
//                  recives the polling and device reply signal from the
//                  two classes and retransmits with its device index.
/////////////////////////////////////////////////////////////////////////////

//******** Include Files *************
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

#include "EnumFile.h"
#include "DataStructure.h"
#include "Configuration/DeviceConfig.h"
#include "GenericRequest/GenericRequest.h"
#include "CommandRequest/CommandRequest.h"
#include "CommandRequest/PasswordResetRequest.h"
#include "ConnectRequest/ConnectRequest.h"

#define TST_CAM_IMAGE_FILE_PATH          "/tmp/Tst_cam.jpeg"
#define MAX_MOTION_INFO_CONFIG_OPTION    5

/////////////////////////////////////////////////////////////////////////////
//Class:  DeviceInfo
//Description:
//            This class contains the deviceInfo instance, polling thread and
//            device request thread instance.This class initiates the
//            polling thread. It also initiates the device request thread.
//            it receives the polling and device reply signal from the
//            two classes and retransmits with its device index.
/////////////////////////////////////////////////////////////////////////////
class DeviceClient : public QObject
{
    Q_OBJECT

public:
    // This API is constructor of class DeviceClient
    // It initializes object with device index and device information
    DeviceClient (quint8 deviceIndex, const DEVICE_CONFIG_t *deviceConfig);

    // This API is destructor of class DeviceClient
    ~DeviceClient();

    // sets device connectivity state
    void setDevConnState (DEVICE_STATE_TYPE_e deviceState);
    void changeDeviceConfig(const DEVICE_CONFIG_t *deviceConfig);

    // This API processes the device related requests
    void processDeviceRequest(DevCommParam * param);

    // This API outputs device related live events
    bool GetDeviceEvents (QStringList &liveEvents);
    bool DeleteDeviceEvents (QStringList liveEvents);

    void updateDeviceHlthStatus(QString healthStr);
    void updateSinglePrmHlthSts(quint8 param, quint8 index, LOG_EVENT_STATE_e state);
    void GetSinglePrmHlthStatus(quint8 *ptr, quint8 paramIndex);
    void GetAllHlthStatusParam(quint8 *ptr);
    void GetSigleParamSingleCameraStatus(quint8 &paramvalue, quint8 paramIndex, quint8 cameraIndex);

    // api to login to device
    void LoginToDevice (void);

    // api to logout from device
    void LogoutFromDevice (void);

    void GetName (QString &deviceName);
    void GetDeviceModel(quint8 &devModel);
    DEVICE_STATE_TYPE_e GetConnectionState(void);
    void GetServerSessionInfo(SERVER_SESSION_INFO_t &serverSessionInfo);
    void GetAutoLogState (bool &autoLogState);
    void GetPreferNativeLoginState(bool &preferNativeDevice);
    void GetFirmwareVersion (QString &version);
    void GetPassword (QString &password);
    void GetUserName (QString &username);
    void UpdateAutoLogin (bool autologflag);
    void GetLiveStreamType(quint8 &liveStreamType);

	static quint8 GetCamRecType(quint8 iCameraId);
	static bool GetCamRecData(quint8 iCameraId, QList<quint16> &pCamRecData);
	static bool IsCamRecAvailable(quint8 iCameraId, quint16 iStartTime, quint16 iEndTime);
    static quint64 GetMonthRec(void);
    static void GetMotionInfo(quint32 *motionInfo);

    void GetDevCamList(quint8 camIndex, DEV_CAM_INFO_t &nameList);
    void GetDevTable(DEV_TABLE_INFO_t &table);
    void GetVideoStandard(VIDEO_STANDARD_e& videoStandard);
    void GetMaxCamera(quint8 &cameraCount);
    void GetCameraName(quint8 cameraIndex, QString &cameraName);
    void GetCameraType(quint8 cameraIndex, CAMERA_TYPE_e &camType);

    // this api set data of ch id & speed
    // and also maintain Pbmedia control commands
    void setActionSyncCmd(SET_COMMAND_e cmdId, QString payload);
    void setLoginAfterLogoutFlag(bool flag);
    void GetUserGroup(USRS_GROUP_e &groupType);
    void SetUserGroup(USRS_GROUP_e groupType);

    void GetAutoCloseRecFailAlertFlag(bool &autoCloseFlag);
    void GetCameraRights(quint8 cameraIndex, quint8 &camRights);
    void SetCameraRights(quint8 cameraIndex, quint8 camRights);
    void deleteDevClntInstants(void);
    void StopConnectRequest(void);
    void GetMaxSensorInputSupport(quint8 &totalInput);
    void GetMaxAlarmOutputSupport(quint8 &totalOutput);
    QString GetDispDeviceName(void);
    void GetPreVideoLossDuration(quint8 &videoLossDuration);

protected:

    bool loginAfterLogoutNeeded;
    WIN_ID_TYPE_e winId;
    // device index
    quint8 devIndex;
    // timeout for requests [defined in login response]
    quint8 respTime;

    // device configuration
    DEVICE_CONFIG_t devConfig;
    QReadWriteLock devConfigLock;

    DEVICE_CONFIG_t rdevTempConfig[MAX_REMOTE_DEVICES];

    static quint16 liveEventCount;

    // read write access lock for device information
    QReadWriteLock devInfoLock;

    // device connection status
    DEVICE_STATE_TYPE_e devConnState;

    // string list to store device type [NVR, DVR, Hybrid]
    // It is used for device verification during login process
    // string list to store response parameters of login request
    QStringList loginRespParam;

    // temporary password string
    QString tempPayload;
    static QMutex eventCountAccess;
    QMutex liveEventListLock;

    // string list to store live events
    QStringList liveEventList;

    QMutex healthStatusLock;
    quint8 healthStatus[MAX_PARAM_STS][MAX_CAMERAS];

    QMutex devCamInfoLock;
    DEV_CAM_INFO_t camInfo[MAX_CAMERAS];

    QMutex deviceTableInfoLock;
    DEV_TABLE_INFO_t tableInfo;
    QString m_dispDevName;

    QReadWriteLock sessionInfoLock;

    // playback string for every minute record of particular day
    // camera & event wise
    static QString recInMinutes[MAX_CAMERAS];
    static quint64 recInMonth;
    static quint32 motionInfo[MAX_MOTION_BYTE + MAX_MOTION_INFO_CONFIG_OPTION];

    // connect request
    ConnectRequest *connectRequest;

    // config request
    GenericRequest *configRequest[MAX_GEN_REQ_SESSION];

    // command request
    CommandRequest *commandRequest[MAX_CMD_SESSION];

    // Password reset request
    PasswordResetRequest *pwdRstCmdRequest[PWD_RST_CMD_SESSION_MAX];

    // verifies device parameters against those entered by user
    bool verifyDeviceLogInInit (QString payload);

    // stores live events to bufer
    bool storeLvEvtAndHealthSts (QString payload);

    void storeRecStatusDay(const char *payload, quint64 size);
    void storeRecStatusMonth(const char *payload, quint32 size);
    void storeMotionInfo(const char *payload, quint16 size);
    void storeJPEGImgData (const char *str);

    bool ParseString(const char **src, char *dest, unsigned char maxDestSize, quint8 &dataSize);

    // creates a connectivity request object
    bool createConnectReq (SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo);

    // creates a configuration request object
    bool createConfigReq (SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, quint8 &genReqSesId);

    // creates a set command request object
    bool createCommandReq (SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId, quint8 &cmdSesionId);

    // creates a password reset command request object
    bool createPwdRstReq (SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, PWD_RST_CMD_e commandId, quint8 &cmdSesionId);

    // deletes a connectivity request object
    bool deleteConnectReq (void);

    // deletes a configuration request object
	bool deleteConfigReq (quint8 genReqSesId);

    // deletes a set command request object
    bool deleteCommandReq (quint8 cmdSesionId);

    // deletes a password reset command request object
    bool deletePwdRstReq (quint8 cmdSesionId);

    bool getHeathStsFrmDev(void);
    bool getCommonCfg(quint8 tableId, bool isUpdateOnLiveEvent = false);
    bool storeCamCfg(QString payload);
    bool storeRemoteDeviceCfg(QString payload, bool isUpdateOnLiveEvent = false);
    void storeGeneralCfg(QString payload);

signals:
    // signal to application controller regarding requests
    void sigEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool);
    void sigPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8);
    void sigDeviceResponse (QString deviceName, DevCommParam *);
    void SigProcessRequest(DevCommParam *);

    void sigExitThread();
    void sigDeleteStreamRequest(QString deviceName);

    // signal to notify change in device configuration
    void sigDeviceCfgUpdate (quint8 index, DEVICE_CONFIG_t *deviceConfig, bool isUpdateOnLiveEvent = false);

public slots:
    // slot for connect request
    void slotConnectResponse (REQ_MSG_ID_e requestId, DEVICE_REPLY_TYPE_e statusId, QString payload = "", QString ipAddr = "", quint16 tcpPort = 0);

    // slot for config request
    void slotConfigResponse (REQ_MSG_ID_e requestId, DEVICE_REPLY_TYPE_e statusId, QString payload = "", quint8 genReqSesId = MAX_GEN_REQ_SESSION);

    // slot for set command request
    void slotCommandResponse (REQ_MSG_ID_e requestId, SET_COMMAND_e commandId, DEVICE_REPLY_TYPE_e statusId, QString payload = "", quint8 cmdSesId = 0);

    // slot for password reset command request
    void slotPwdRstCmdResponse(REQ_MSG_ID_e requestId, PWD_RST_CMD_e commandId, DEVICE_REPLY_TYPE_e statusId, QString payload = "", quint8 cmdSesId = 0);

    //slot for device Request
    void SlotProcessRequest(DevCommParam*);
};

#endif // DEVICECLIENT_H
