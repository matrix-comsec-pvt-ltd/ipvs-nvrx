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
//   Project      : NVR(Network Video Recorder)
//   Owner        : Shruti Sahni
//                : Aekam Parmar
//   File         : DeviceClient.cpp
//   Description  : This file contains DeviceClient class member
//                  functions. This class stores the device information such
//                  as ip addres, port no etc and starts the polling thread
//                  It's member function starts the device request thread
//                  (for the request other than login and poll request).
//                  It also catches the polling Reply signal and device reply
//                  signal from the respective thread class and transmits them
//                  with its device index.
/////////////////////////////////////////////////////////////////////////////

//******** Include Files *************
#include <stdio.h>
#include <unistd.h>
#include <QCoreApplication>
#include <QRegExp>
#include <QFile>

#include "ConfigField.h"
#include "DeviceClient.h"
#include "ValidationMessage.h"
#include "ConfigPages/CameraSettings/CameraSettings.h"
#include "ConfigPages/Devices/DeviceSetting.h"

#define MODEL_DELIMITER                 '-'
#define DFLT_LOGIN_RESP_TIME            10
#define MAX_LIVE_EVENT_LIMIT            50
#define MAX_SIZE_TO_CHECK_NEXT_FSP      5
#define NO_OF_BYTES_SINGLE_DAY_REC      180
#define NO_OF_BITS_SINGLE_DAY_REC       1440
#define MAX_EVENTS_FOR_REC              4
#define LOCAL_DEFAULT_MONITORING_RIGHTS 0x48  // Only For Local monitoring default rights Enable

NVR_DEVICE_INFO_t deviceRespInfo;

// login response parameter order
typedef enum
{
    LOGIN_SESSION_ID = 0,
    LOGIN_MODEL,
    LOGIN_SW_VERSION,
    LOGIN_COMM_VERSION,
    LOGIN_KLV_TIME,
    LOGIN_RESP_TIME,
    MAX_LOGIN_RESP_ORDER
}LOGIN_RESP_ORDER_e;

quint64 DeviceClient::recInMonth = 0;
quint16 DeviceClient::liveEventCount = 0;
QMutex DeviceClient::eventCountAccess;
QString DeviceClient::recInMinutes[MAX_CAMERAS];
quint32 DeviceClient::motionInfo[MAX_MOTION_BYTE + MAX_MOTION_INFO_CONFIG_OPTION] = {0};

//*****************************************************************************
//  DeviceClient()
//      Param:
//          IN : quint8 deviceIndex
//               DeviceInfo deviceInfo
//          OUT: NONE
//	Returns:
//		Not Applicable
//      Description:
//          This API is constructor of class DeviceClient. It initializes the
//          newly created object with device index and device information.
//          Also if auto login is enable, automatically connects to the device.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
DeviceClient::DeviceClient(quint8 deviceIndex, const DEVICE_CONFIG_t *deviceConfig)
{
    setObjectName("DEV_CLT");
    respTime = 0;

    // store device parameter
    devIndex = deviceIndex;

    devConfigLock.lockForWrite();
    memcpy(&devConfig, deviceConfig, sizeof(DEVICE_CONFIG_t));
    devConfigLock.unlock();

    // initialize request object pointers
    INIT_OBJ(connectRequest);

    loginAfterLogoutNeeded = false;

    memset(&healthStatus, 0, sizeof(healthStatus));

    tableInfo.autoCloseRecFailAlert = false;

    for(quint8 index = 0; index < MAX_CMD_SESSION; index++)
    {
        commandRequest [index] = NULL;
    }

    for(quint8 index = 0; index < MAX_GEN_REQ_SESSION; index++)
	{
		configRequest [index] = NULL;
	}

    for(quint8 index = 0; index < MAX_CAMERAS; index++)
    {
        camInfo[index].cameraRights = LOCAL_DEFAULT_MONITORING_RIGHTS; // only monitoring rights as local user
    }

    for(quint8 index = 0; index < PWD_RST_CMD_SESSION_MAX; index++)
    {
        pwdRstCmdRequest [index] = NULL;
    }

    m_dispDevName = LOCAL_DEVICE_NAME;
    devConnState = DISCONNECTED;
    winId = MAX_WIN_ID;
}

void DeviceClient::changeDeviceConfig(const DEVICE_CONFIG_t *deviceConfig)
{
    quint16 port,forwardedPort;

    devConfigLock.lockForWrite();
    memcpy(&devConfig, deviceConfig, sizeof(DEVICE_CONFIG_t));
    port = devConfig.port;
    forwardedPort = devConfig.forwardedTcpPort;
    devConfigLock.unlock();

    if(IS_VALID_OBJ(connectRequest))
    {
        connectRequest->changeDeviceconfigForPortUpdate(port, forwardedPort);
    }
}

//*****************************************************************************
//  ~DeviceClient
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//	Returns:
//		Not Applicable
//      Description:
//
//	[Pre-condition:]
//          This API is destructor of class DeviceClient.
//          As of now it does no functionality, kept for future use.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
DeviceClient::~DeviceClient()
{

}

//*****************************************************************************
//  LoginToDevice()
//  Param:
//      IN : None
//      OUT: None
//  Returns:
//      None
//  Description:
//
//*****************************************************************************
void DeviceClient::LoginToDevice(void)
{
    // create payload for login request
    QString payload = "";
    DevCommParam * devCommParam = new DevCommParam();

    devCommParam->msgType = MSG_REQ_LOG;
    devCommParam->cmdType = MAX_NET_COMMAND;

    sessionInfoLock.lockForWrite();
    loginRespParam.insert(LOGIN_SESSION_ID, NVR_SMART_CODE);
    sessionInfoLock.unlock();

    devConfigLock.lockForRead();
    payload.append(devConfig.username);
    payload.append(FSP);
    payload.append(devConfig.password);
    payload.append(FSP);
    devCommParam->payload = payload;
    if(strcmp(devConfig.deviceName, LOCAL_DEVICE_NAME) == 0)
    {
        sleep(5);
    }
    devConfigLock.unlock();

    // call process device request
    DPRINT(DEVICE_CLIENT, "login request: [device=%s], [user=%s], [ip=%s]", devConfig.deviceName, devConfig.username, devConfig.ipAddress);
    processDeviceRequest(devCommParam);
}

//*****************************************************************************
//  LogoutFromDevice()
//  Param:
//      IN : None
//      OUT: None
//  Returns:
//      None
//  Description:
//
//*****************************************************************************
void DeviceClient::LogoutFromDevice(void)
{
    // create payload for logout request
    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_SET_CMD;
    param->cmdType = LOGOUT;
    param->payload = "";

    // call process device request
    DPRINT(DEVICE_CLIENT, "logout request: [device=%s], [ip=%s]", devConfig.deviceName, devConfig.ipAddress);
    processDeviceRequest(param);
}

//*****************************************************************************
//  processDeviceRequest()
//      Param:
//          IN : QList<QVariant> requestParam
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API processes request for device, like connect, configuration,
//          command and stream.
//          Each time a new request is received, request object is created.
//          It is initialized with necessary parameters and thread to does
//          communication with server is started.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::processDeviceRequest(DevCommParam * param)
{
    if(IS_VALID_OBJ(param))
    {
        emit SigProcessRequest(param);
    }
}

void DeviceClient::SlotProcessRequest(DevCommParam* param)
{
    bool status = false;
    SERVER_INFO_t serverInfo;
    REQ_INFO_t requestInfo;
    DEVICE_REPLY_TYPE_e response = CMD_SUCCESS;
    REQ_MSG_ID_e requestId;
    SET_COMMAND_e setCmdId;
    PWD_RST_CMD_e pwdRstCmdId;
    QString payload;
    quint8 *bytePayload;
    quint32 payloadSize;
    const char *arrayRes;
    quint8 reqSesId;
    quint8 portIndex = 0;

    if(IS_VALID_OBJ(connectRequest))
    {
        portIndex = connectRequest->getPortIndex();
    }

    requestId = param->msgType;
    setCmdId = param->cmdType;
    pwdRstCmdId = param->pwdRstCmdType;

    bytePayload = param->bytePayload;
    payload = param->payload;
    winId = param->windowId;
    DELETE_OBJ(param);
    devConfigLock.lockForRead();

    // if request id is login or device is connected store server patameters
    serverInfo.ipAddress = devConfig.ipAddress;
    if(IS_VALID_OBJ(connectRequest))
    {
        serverInfo.tcpPort =(portIndex == 0) ? devConfig.port : devConfig.forwardedTcpPort;
    }
    else
    {
        serverInfo.tcpPort = devConfig.port;
    }
    devConfigLock.unlock();

    // store request parameters
    requestInfo.requestId = requestId;
    requestInfo.payload = payload;
    requestInfo.bytePayload = bytePayload;
    requestInfo.windowId = winId;

    sessionInfoLock.lockForRead();
    requestInfo.sessionId = loginRespParam.value(LOGIN_SESSION_ID);

    // if it is login request, store default login timeout else store timeout as per received in login response argument
    requestInfo.timeout = (requestInfo.requestId == MSG_REQ_LOG) ? DFLT_LOGIN_RESP_TIME : respTime;
    sessionInfoLock.unlock();

    switch(requestInfo.requestId)
    {
        // if it is connect request
        case MSG_REQ_LOG:
        {
            // create connection request object
            status = createConnectReq(serverInfo, requestInfo);

            // if object created successfully
            if(status == true)
            {
                if(IS_VALID_OBJ(connectRequest))
                {
                    // start request thread
                    connectRequest->start();
                }
            }
            else
            {
                // set status id to resource limit
                response = CMD_INTERNAL_RESOURCE_LIMIT;
            }
        }
        break;

        // if it is configuration request
        case MSG_GET_CFG:
        case MSG_SET_CFG:
        case MSG_DEF_CFG:
        {
            // create configuration request object
            status = createConfigReq(serverInfo, requestInfo, reqSesId);

            // if object created succesfully
            if(status == true)
            {
                // start request thread
                configRequest[reqSesId]->start();
            }
            else
            {
                // set status id to resource limit
                response = CMD_INTERNAL_RESOURCE_LIMIT;
            }
        }
        break;

        // if it is set command request
        case MSG_SET_CMD:
        {
            switch(setCmdId)
            {
                case PLYBCK_SRCH_MNTH:
                case PLYBCK_SRCH_DAY:
                {
                    status = createCommandReq(serverInfo, requestInfo, setCmdId, reqSesId);
                    if(status == true)
                    {
                        arrayRes = commandRequest[reqSesId]->getResWithoutChekEom(payloadSize, response);
                        if(response == CMD_SUCCESS)
                        {
                            if(setCmdId == PLYBCK_SRCH_MNTH)
                            {
                                storeRecStatusMonth(arrayRes, payloadSize);
                            }
                            else
                            {
                                storeRecStatusDay(arrayRes, payloadSize);
                            }
                        }

                        deleteCommandReq(reqSesId);
                        status = false;
                    }
                    else
                    {
                        response = CMD_INTERNAL_RESOURCE_LIMIT;
                    }
                }
                break;

                case GET_MOTION_WINDOW:
                {
                    status = createCommandReq(serverInfo, requestInfo, setCmdId, reqSesId);
                    if(status == true)
                    {
                        arrayRes = commandRequest[reqSesId]->getResWithoutChekEom(payloadSize, response);
                        if(response == CMD_SUCCESS)
                        {
                            storeMotionInfo(arrayRes, payloadSize);
                        }

                        deleteCommandReq(reqSesId);
                        status = false;
                    }
                    else
                    {
                        response = CMD_INTERNAL_RESOURCE_LIMIT;
                    }
                }
                break;

                case TST_CAM:
                {
                    status = createCommandReq(serverInfo, requestInfo, setCmdId, reqSesId);
                    if(status == true)
                    {
                        arrayRes = commandRequest[reqSesId]->getResWithoutChekEom(payloadSize, response, MAX_RCV_BUFFER_SIZE_FOR_IMAGE);
                        if(response == CMD_SUCCESS)
                        {
                            storeJPEGImgData(arrayRes);
                        }

                        deleteCommandReq(reqSesId);
                        status = false;
                    }
                    else
                    {
                        response = CMD_INTERNAL_RESOURCE_LIMIT;
                    }
                }
                break;

                // if it is general set command request
                default:
                {
                    // create set command request object
                    status = createCommandReq(serverInfo, requestInfo, setCmdId, reqSesId);

                    // if object created successfully
                    if(status == false)
                    {
                        // set status id to resource limit
                        response = CMD_INTERNAL_RESOURCE_LIMIT;
                        break;
                    }

                    // start request thread
                    commandRequest[reqSesId]->start();

                    switch(setCmdId)
                    {
                        case CNG_PWD:
                        case CNG_USER:
                        {
                            tempPayload = requestInfo.payload;
                        }
                        break;

                        case LOGOUT:
                        {
                            if(IS_VALID_OBJ(connectRequest))
                            {
                                connectRequest->setRunflag(false);
                            }
                        }
                        break;

                        default:
                        {
                            /* Nothing to do */
                        }
                        break;
                    }
                }
                break;
            }
        }
        break;

        case MSG_PWD_RST:
        {
            // create password reset command request object
            status = createPwdRstReq(serverInfo, requestInfo, pwdRstCmdId, reqSesId);

            // if object created successfully
            if(status == false)
            {
                // set status id to resource limit
                response = CMD_INTERNAL_RESOURCE_LIMIT;
                break;
            }

            // start request thread
            pwdRstCmdRequest[reqSesId]->start();
        }
        break;

        default:
        {
            // default nothing
        }
        break;
    }

    // if any internal error occured
    if(status == false)
    {
        switch(requestId)
        {
            // for connect request then call connect response slot
            case MSG_REQ_LOG:
                slotConnectResponse(requestId, response);
                break;

            // for config request then call connect response slot
            case MSG_GET_CFG:
            case MSG_SET_CFG:
            case MSG_DEF_CFG:
                slotConfigResponse(requestId, response);
                break;

            // for set command request then call connect response slot
            case MSG_SET_CMD:
                slotCommandResponse(requestId, setCmdId, response, "", reqSesId);
                break;

            // for password reset command request then call connect response slot
            case MSG_PWD_RST:
                slotPwdRstCmdResponse(requestId, pwdRstCmdId, response, "", reqSesId);
                break;

            default:
                break;
        }
    }
}

//*****************************************************************************
//  getDeviceEvents()
//      Param:
//          IN : NONE
//          OUT: QStringList &liveEvents
//	Returns:
//		bool [true / false]
//      Description:
//          outputs the live events.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::GetDeviceEvents(QStringList &liveEvents)
{
    liveEventListLock.lock();
    liveEvents = liveEventList;
    liveEventListLock.unlock();
    return true;
}

bool DeviceClient::DeleteDeviceEvents(QStringList deletionList)
{
    QString eventToDelete;

    liveEventListLock.lock();
    eventCountAccess.lock();

    for(quint8 index = 0; index < deletionList.length(); index++)
    {
        liveEventCount--;
        eventToDelete = deletionList.value(index);
        liveEventList.removeOne(eventToDelete);
    }

    eventCountAccess.unlock();
    liveEventListLock.unlock();
    return true;
}

//*****************************************************************************
//  getDevConnState()
//      Param:
//          IN : DEVICE_STATE_TYPE_e &deviceState
//          OUT: NONE
//	Returns:
//          NONE
//      Description:
//          This API outputs device connectivity status
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::GetName(QString &deviceName)
{
    devConfigLock.lockForRead();
    deviceName = devConfig.deviceName;
    devConfigLock.unlock();
}

//*****************************************************************************
//  getDevConnState()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//          Device Connection State
//      Description:
//          This API outputs device connectivity status
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
DEVICE_STATE_TYPE_e DeviceClient::GetConnectionState(void)
{
    DEVICE_STATE_TYPE_e deviceState;
    devInfoLock.lockForRead();
    deviceState = devConnState;
    devInfoLock.unlock();
    return deviceState;
}

void DeviceClient::GetServerSessionInfo(SERVER_SESSION_INFO_t &serverSessionInfo)
{
    quint8 portIndex = 0;

    if(IS_VALID_OBJ(connectRequest))
    {
        portIndex = connectRequest->getPortIndex();
    }

    sessionInfoLock.lockForRead();
    serverSessionInfo.sessionInfo.sessionId = loginRespParam.at(LOGIN_SESSION_ID);
    serverSessionInfo.sessionInfo.timeout = respTime;
    sessionInfoLock.unlock();

    devConfigLock.lockForRead();
    serverSessionInfo.serverInfo.ipAddress = QString(devConfig.ipAddress);
    serverSessionInfo.serverInfo.tcpPort =(portIndex == 0) ? devConfig.port : devConfig.forwardedTcpPort;
    devConfigLock.unlock();
}

//*****************************************************************************
//  getDevConnState()
//      Param:
//          IN : DEVICE_STATE_TYPE_e &deviceState
//          OUT: NONE
//	Returns:
//          NONE
//      Description:
//          This API sets device connectivity status
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::setDevConnState(DEVICE_STATE_TYPE_e deviceState)
{
    devInfoLock.lockForWrite();
    devConnState = deviceState;
    devInfoLock.unlock();
}

//*****************************************************************************
//  getDevConnState()
//      Param:
//          IN : DEVICE_STATE_TYPE_e &deviceState
//          OUT: NONE
//	Returns:
//          NONE
//      Description:
//          This API sets device connectivity status
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::GetAutoLogState(bool &autoLogState)
{
    devConfigLock.lockForRead();
    autoLogState = devConfig.autoLogin;
    devConfigLock.unlock();
}

void DeviceClient::GetPreferNativeLoginState(bool &preferNativeDevice)
{
    devConfigLock.lockForRead();
    preferNativeDevice = devConfig.nativeDeviceCredential;
    devConfigLock.unlock();
}

void DeviceClient::GetFirmwareVersion(QString &version)
{
    version = QString("%1").arg(deviceRespInfo.softwareVersion) + QString(".");
    version += QString("%1").arg(deviceRespInfo.softwareRevision) + QString(".");
    version += QString("%1").arg(deviceRespInfo.productSubRevision);
}

void DeviceClient::GetPassword(QString &password)
{
    devConfigLock.lockForRead();
    password = devConfig.password;
    devConfigLock.unlock();
}

void DeviceClient::GetUserName(QString &username)
{
    devConfigLock.lockForRead();
    username = devConfig.username;
    devConfigLock.unlock();
}

void DeviceClient::UpdateAutoLogin(bool autologflag)
{
    devConfigLock.lockForWrite();
    devConfig.autoLogin = autologflag;
    devConfigLock.unlock();

    if(IS_VALID_OBJ(connectRequest))
    {
        connectRequest->UpdateAutoLogin(autologflag);
    }
}

void DeviceClient::GetLiveStreamType(quint8 &liveStreamType)
{
    devConfigLock.lockForRead();
    liveStreamType = devConfig.liveStreamType;
    devConfigLock.unlock();
}

//*****************************************************************************
//  createConnectReq()
//      Param:
//          IN : SERVER_INFO_t serverInfo
//               REQ_INFO_t requestInfo
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API creates a connection request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::createConnectReq(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo)
{
    // if request object is not created
    if (IS_VALID_OBJ(connectRequest))
    {
        return false;
    }

    /* PARASOFT: Memory Deallocated in delete Connect Req */
    devConfigLock.lockForRead();
    connectRequest = new ConnectRequest(serverInfo, requestInfo, devConfig.forwardedTcpPort, devConfig.autoLogin, devConfig.connType);
    devConfigLock.unlock();

    // connect signal of request object to the slot of device client
    connect(connectRequest,
            SIGNAL(sigConnectResponse(REQ_MSG_ID_e,
                                      DEVICE_REPLY_TYPE_e,
                                      QString,
                                      QString,
                                      quint16)),
            this,
            SLOT(slotConnectResponse(REQ_MSG_ID_e,
                                     DEVICE_REPLY_TYPE_e,
                                     QString,
                                     QString,
                                     quint16)));
    // return status
    return true;
}

//*****************************************************************************
//  createConfigReq()
//      Param:
//          IN : SERVER_INFO_t serverInfo
//               REQ_INFO_t requestInfo
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API creates a configuration request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::createConfigReq(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, quint8 &genReqSesId)
{
	int index;

	/* Notes: INSP2-1198/INSP2-1199
	 * On receiving LOG_USER_EVENT[Type-LOG_CONFIG_CHANGE] event from Server Application,
	 * When response of SET_CFG request is not received, Sometimes UI Application does not send GET_CFG request.
	 * It does not result to config change, config is already saved but effect of it is not seen in local UI.
	 * That also faced when config change on local UI only. If config changes from DC, then such instance wont arise.
	 * Above Issue observed for Live View Enable flag and Add N/W cascaded Device.
	 *
	 * Changes: Device client can use multiple configRequest at a time instead of single request.
	 * Applicable Pages: GENERAL_TABLE_INDEX, CAMERA_TABLE_INDEX, NETWORK_DEVICE_SETTING_TABLE_INDEX
	 * */
	// Find Free GenReqSesId
    for(index = 0; index < MAX_GEN_REQ_SESSION; ++index)
	{
		// if request object is not created
        if(configRequest[index] == NULL)
		{
			// break the loop
			genReqSesId = index;
			break;
		}
	}

	if(index >= MAX_GEN_REQ_SESSION)
	{
        EPRINT(DEVICE_CLIENT, "free index not available for config request");
        return false;
	}

    requestInfo.timeout = CMD_SYNC_PB_FRAME_RECV_TIMEOUT;

    // create a request object and initialize it
    configRequest[genReqSesId] = new GenericRequest(serverInfo, requestInfo, genReqSesId);

    // connect signal of request object to the slot of device client
    connect(configRequest[genReqSesId],
            SIGNAL(sigGenericResponse(REQ_MSG_ID_e,
                                      DEVICE_REPLY_TYPE_e,
                                      QString,
                                      quint8)),
            this,
            SLOT(slotConfigResponse(REQ_MSG_ID_e,
                                    DEVICE_REPLY_TYPE_e,
                                    QString,
                                    quint8)));
	// return status
    return true;
}

//*****************************************************************************
//  createCommandReq()
//      Param:
//          IN : SERVER_INFO_t serverInfo
//               REQ_INFO_t requestInfo
//               SET_COMMAND_e commandId
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API creates a set command request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::createCommandReq(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId, quint8 &cmdSesionId)
{
    /* Is free session available? */
    if (false == CommandRequest::getFreeCmdSession(cmdSesionId))
    {
        return false;
    }

    // if request object is not created
    if (IS_VALID_OBJ(commandRequest[cmdSesionId]))
    {
        return false;
    }

    // create a request object and initialize it
    commandRequest[cmdSesionId] = new CommandRequest(serverInfo, requestInfo, commandId, cmdSesionId);

    // connect signal of request object to the slot of device client
    connect(commandRequest[cmdSesionId],
            SIGNAL(sigCommandResponse(REQ_MSG_ID_e,
                                      SET_COMMAND_e,
                                      DEVICE_REPLY_TYPE_e,
                                      QString,
                                      quint8)),
            this,
            SLOT(slotCommandResponse(REQ_MSG_ID_e,
                                     SET_COMMAND_e,
                                     DEVICE_REPLY_TYPE_e,
                                     QString,
                                     quint8)));
    // return status
    return true;
}

//*****************************************************************************
//  createPwdRstReq()
//      Param:
//          IN : SERVER_INFO_t serverInfo
//               REQ_INFO_t requestInfo
//               SET_COMMAND_e commandId
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API creates a password reset command request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::createPwdRstReq(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, PWD_RST_CMD_e commandId, quint8 &cmdSesionId)
{
    /* Is free session available? */
    if (false == PasswordResetRequest::getFreeCmdSession(cmdSesionId))
    {
        return false;
    }

    // if request object is not created
    if (IS_VALID_OBJ(pwdRstCmdRequest[cmdSesionId]))
    {
        return false;
    }

    // create a request object and initialize it
    pwdRstCmdRequest[cmdSesionId] = new PasswordResetRequest(serverInfo, requestInfo, commandId, cmdSesionId);

    // connect signal of request object to the slot of device client
    connect(pwdRstCmdRequest[cmdSesionId],
            SIGNAL(sigPwdRstCmdResponse(REQ_MSG_ID_e,
                                        PWD_RST_CMD_e,
                                        DEVICE_REPLY_TYPE_e,
                                        QString,
                                        quint8)),
            this,
            SLOT(slotPwdRstCmdResponse(REQ_MSG_ID_e,
                                       PWD_RST_CMD_e,
                                       DEVICE_REPLY_TYPE_e,
                                       QString,
                                       quint8)));
    // return status
    return true;
}

//*****************************************************************************
//  deleteConnectReq()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API deletes a connection request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::deleteConnectReq(void)
{
    // if request object is already created
    if (connectRequest == NULL)
    {
        return false;
    }

    connectRequest->setRunflag(false);

    // wait for request thread to return
    connectRequest->wait();

    // disconnect signal of request object from slot of device client
    disconnect(connectRequest,
               SIGNAL(sigConnectResponse(REQ_MSG_ID_e,
                                         DEVICE_REPLY_TYPE_e,
                                         QString,
                                         QString,
                                         quint16)),
               this,
               SLOT(slotConnectResponse(REQ_MSG_ID_e,
                                        DEVICE_REPLY_TYPE_e,
                                        QString,
                                        QString,
                                        quint16)));

    // delete request object
    DELETE_OBJ(connectRequest);

    // return status
    return true;
}

//*****************************************************************************
//  deleteConfigReq()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API deletes a configuration request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::deleteConfigReq(quint8 genReqSesId)
{
    if (genReqSesId >= MAX_GEN_REQ_SESSION)
	{
        EPRINT(DEVICE_CLIENT, "invld config session id: [sessionId=%d]", genReqSesId);
        return false;
	}

	// if request object is already created
    if (configRequest[genReqSesId] == NULL)
	{
        return false;
    }

    // wait for request thread to return
    configRequest[genReqSesId]->wait();

    // disconnect signal of request object from slot of device client
    disconnect(configRequest[genReqSesId],
               SIGNAL(sigGenericResponse(REQ_MSG_ID_e,
                                           DEVICE_REPLY_TYPE_e,
                                           QString,
                                           quint8)),
               this,
               SLOT(slotConfigResponse(REQ_MSG_ID_e,
                                           DEVICE_REPLY_TYPE_e,
                                           QString,
                                           quint8)));

    // delete request object
    DELETE_OBJ(configRequest[genReqSesId]);

	// return status
    return true;
}

//*****************************************************************************
//  deleteCommandReq()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API deletes a set command request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::deleteCommandReq(quint8 cmdSesionId)
{
    // if request object is not already created
    if(commandRequest[cmdSesionId] == NULL)
    {
        return false;
    }

    // wait for request thread to return
    commandRequest[cmdSesionId]->wait();

    // disconnect signal of request object from slot of device client
    disconnect(commandRequest[cmdSesionId],
               SIGNAL(sigCommandResponse(REQ_MSG_ID_e,
                                         SET_COMMAND_e,
                                         DEVICE_REPLY_TYPE_e,
                                         QString,
                                         quint8)),
               this,
               SLOT(slotCommandResponse(REQ_MSG_ID_e,
                                        SET_COMMAND_e,
                                        DEVICE_REPLY_TYPE_e,
                                        QString,
                                        quint8)));

    // delete request object
    DELETE_OBJ(commandRequest[cmdSesionId]);
    CommandRequest::setCmdSessionFree(cmdSesionId);

    // return status
    return true;
}

//*****************************************************************************
//  deletePwdRstReq()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API deletes a password reset command request object.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::deletePwdRstReq(quint8 cmdSesionId)
{
    // if request object is not already created
    if(pwdRstCmdRequest[cmdSesionId] == NULL)
    {
        return false;
    }

    // wait for request thread to return
    pwdRstCmdRequest[cmdSesionId]->wait();

    // disconnect signal of request object from slot of device client
    disconnect(pwdRstCmdRequest[cmdSesionId],
               SIGNAL(sigPwdRstCmdResponse(REQ_MSG_ID_e,
                                           PWD_RST_CMD_e,
                                           DEVICE_REPLY_TYPE_e,
                                           QString,
                                           quint8)),
               this,
               SLOT(slotPwdRstCmdResponse(REQ_MSG_ID_e,
                                          PWD_RST_CMD_e,
                                          DEVICE_REPLY_TYPE_e,
                                          QString,
                                          quint8)));

    // delete request object
    DELETE_OBJ(pwdRstCmdRequest[cmdSesionId]);
    PasswordResetRequest::setCmdSessionFree(cmdSesionId);

    // return status
    return true;
}

//*****************************************************************************
//  verifyDevice()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API verifies the device for login response payload against the
//          parameters set by user.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::verifyDeviceLogInInit(QString payload)
{
    QString sessionId;
    NVR_DEVICE_INFO_t deviceResp;

    sessionId = payload.mid(3, 6);
    deviceResp.softwareVersion =(quint32)payload.mid(9, 2).toUInt();
    deviceResp.softwareRevision =(quint32)payload.mid(11, 2).toUInt();
    deviceResp.commVersion =(quint32)payload.mid(13, 2).toUInt();
    deviceResp.commRevision =(quint32)payload.mid(15, 2).toUInt();
    deviceResp.responseTime =(quint32)payload.mid(17, 2).toUInt();
    deviceResp.KLVTime =(quint32)payload.mid(19, 2).toUInt();
    deviceResp.maxCameras =(quint32)payload.mid(21, 2).toUInt();
    deviceResp.maxAnalogCameras =(quint32)payload.mid(23, 2).toUInt();
    deviceResp.maxIpCameras =(quint32)payload.mid(25, 2).toUInt();
    deviceResp.configuredAnalogCameras =(quint32)payload.mid(27, 2).toUInt();
    deviceResp.configuredIpCameras =(quint32)payload.mid(29, 2).toUInt();
    deviceResp.maxSensorInput =(quint32)payload.mid(31, 2).toUInt();
    deviceResp.maxAlarmOutput =(quint32)payload.mid(33, 2).toUInt();
    deviceResp.audioIn =(quint32)payload.mid(35, 2).toUInt();
    deviceResp.audioOut =(quint32)payload.mid(37, 1).toUInt();
    deviceResp.noOfHdd =(quint32)payload.mid(38, 1).toUInt();
    deviceResp.noOfNdd =(quint32)payload.mid(39, 1).toUInt();
    deviceResp.noOfLanPort =(quint32)payload.mid(40, 1).toUInt();
    deviceResp.noOfVGA =(quint32)payload.mid(41, 1).toUInt();
    deviceResp.hdmi1 =(quint32)payload.mid(42, 1).toUInt();
    deviceResp.hdmi2 =(quint32)payload.mid(43, 1).toUInt();
    deviceResp.CVBSMain =(quint32)payload.mid(44, 1).toUInt();
    deviceResp.CVBSSpot =(quint32)payload.mid(45, 1).toUInt();
    deviceResp.CVBSSpotAnalog =(quint32)payload.mid(46, 1).toUInt();
    deviceResp.anlogPTZSupport =(quint32)payload.mid(47, 1).toUInt();
    deviceResp.USBPort =(quint32)payload.mid(48, 1).toUInt();
    deviceResp.maxMainAnalogResolution =(quint32)payload.mid(49, 1).toUInt();
    deviceResp.maxSubAnalogResolution =(quint32)payload.mid(50, 1).toUInt();
    deviceResp.videoStandard =(quint32)payload.mid(51, 1).toUInt();
    deviceResp.maxMainEncodingCap =(quint32)payload.mid(52, 4).toUInt();
    deviceResp.maxSubEncodingCap =(quint32)payload.mid(56, 4).toUInt();
    deviceResp.diskCheckingCount =(quint32)payload.mid(60, 1).toUInt();
    deviceResp.userGroup =(quint32)payload.mid(61, 1).toUInt();
    deviceResp.passwordPolicyLockTime =(quint32)payload.mid(62, 3).toUInt();
    deviceResp.passwordExpirationTime =(quint32)payload.mid(65, 1).toUInt();
    deviceResp.productVariant =(quint32)payload.mid(66, 2).toUInt();
    deviceResp.productSubRevision =(quint32)payload.mid(68, 2).toUInt();
    deviceResp.maxDisplayOutput = MAIN_DISPLAY;

    devConfigLock.lockForWrite();
    if(strcmp(devConfig.deviceName, LOCAL_DEVICE_NAME) == 0)
    {
        deviceResp.maxDisplayOutput = MAX_DISPLAY_TYPE;
        memcpy(&deviceRespInfo, &deviceResp, sizeof(NVR_DEVICE_INFO_t));
    }
    devConfigLock.unlock();

    deviceTableInfoLock.lock();

    tableInfo.sensors = deviceResp.maxSensorInput;
    tableInfo.alarms = deviceResp.maxAlarmOutput;
    tableInfo.audioIn = deviceResp.audioIn;

    tableInfo.ipCams = (deviceResp.configuredIpCameras > MAX_CAMERAS) ? MAX_CAMERAS : deviceResp.configuredIpCameras;
    tableInfo.analogCams = deviceResp.configuredAnalogCameras;
    tableInfo.totalCams = (deviceResp.maxCameras > MAX_CAMERAS) ? MAX_CAMERAS : deviceResp.maxCameras;
    tableInfo.videoStd =(VIDEO_STANDARD_e)deviceResp.videoStandard;
    tableInfo.mainEncodingCapacity = deviceResp.maxMainEncodingCap;
    tableInfo.subEncodingCapacity = deviceResp.maxSubEncodingCap;
    tableInfo.maxAnalogCam = deviceResp.maxAnalogCameras;
    tableInfo.maxIpCam = (deviceResp.maxIpCameras > MAX_CAMERAS) ? MAX_CAMERAS : deviceResp.maxIpCameras;
    tableInfo.numOfHdd = deviceResp.noOfHdd;
    tableInfo.numOfLan = deviceResp.noOfLanPort;
    tableInfo.maxMainAnalogResolution = deviceResp.maxMainAnalogResolution;
    tableInfo.maxSubAnalogResolution = deviceResp.maxSubAnalogResolution;
    tableInfo.productVariant = deviceResp.productVariant;

    /* Note : in Server usertype of local not get on login response */
    tableInfo.userGroupType =(strcmp(devConfig.deviceName, LOCAL_DEVICE_NAME) == 0) ? VIEWER : (USRS_GROUP_e)deviceResp.userGroup;

    sessionInfoLock.lockForWrite();
    respTime = deviceResp.responseTime;
    loginRespParam.insert(LOGIN_SESSION_ID, sessionId);
    sessionInfoLock.unlock();

    if(IS_VALID_OBJ(connectRequest))
    {
        connectRequest->setSessionId(sessionId);
        connectRequest->setKeepAliveTime(deviceRespInfo.KLVTime);
        connectRequest->setResponseTime(deviceRespInfo.responseTime);
    }

    deviceTableInfoLock.unlock();

    if ((deviceResp.maxCameras > MAX_CAMERAS) || (deviceResp.maxIpCameras > MAX_CAMERAS) || (deviceResp.configuredIpCameras > MAX_CAMERAS))
    {
        EPRINT(DEVICE_CLIENT, "more than supported cameras found: [supported=%d], [maxCameras=%d], [maxIpCameras=%d], [configuredIpCameras=%d]",
               MAX_CAMERAS, deviceResp.maxCameras, deviceResp.maxIpCameras, deviceResp.configuredIpCameras);
    }

    if ((deviceResp.commVersion != COMMUNICATION_VERSION) || (deviceResp.commRevision != COMMUNICATION_REVISION))
    {
        if((deviceResp.commVersion > COMMUNICATION_VERSION) ||(deviceResp.commRevision > COMMUNICATION_REVISION))
        {
            tableInfo.deviceConflictType = MX_DEVICE_CONFLICT_TYPE_SERVER_OLD;
        }
        else
        {
            tableInfo.deviceConflictType = MX_DEVICE_CONFLICT_TYPE_SERVER_NEW;
        }

        EPRINT(DEVICE_CLIENT, "firmware mismatch: [deviceName=%s], [SwVer=%d.%d.%d], [ServerComVer=V%dR%d], [AppComVer=V%dR%d]",
               devConfig.deviceName, SOFTWARE_VERSION, SOFTWARE_REVISION, PRODUCT_SUB_REVISION,
               deviceResp.commVersion, deviceResp.commRevision, COMMUNICATION_VERSION, COMMUNICATION_REVISION);
        return false;
    }

    tableInfo.deviceConflictType = MAX_MX_DEVICE_CONFLICT_TYPE;
    DPRINT(DEVICE_CLIENT, "login verification success: [deviceName=%s], [SwVer=%d.%d.%d], [ComVer=V%dR%d], [AnalogCamera=%d], "
                          "[IpCamera=%d], [TotalCamera=%d], [Sensors=%d], [Alarams=%d], [userGroup=%d], [AudioIn=%d], [sessionId=%s]",
           devConfig.deviceName, SOFTWARE_VERSION, SOFTWARE_REVISION, PRODUCT_SUB_REVISION, deviceResp.commVersion, deviceResp.commRevision,
           tableInfo.analogCams, tableInfo.ipCams, tableInfo.totalCams, tableInfo.sensors, tableInfo.alarms, tableInfo.userGroupType,
           tableInfo.audioIn, sessionId.toUtf8().constData());
    return true;
}

void DeviceClient::GetMaxCamera(quint8 &cameraCount)
{
    deviceTableInfoLock.lock();
    cameraCount = tableInfo.totalCams;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetDeviceModel(quint8 &devModel)
{
    deviceTableInfoLock.lock();
    devModel =(quint8)tableInfo.productVariant;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetCameraName(quint8 cameraIndex, QString &cameraName)
{
    devCamInfoLock.lock();
    cameraName = (cameraIndex < MAX_CAMERAS) ? camInfo[cameraIndex].camName : "";
    devCamInfoLock.unlock();
}

void DeviceClient::setLoginAfterLogoutFlag(bool flag)
{
    loginAfterLogoutNeeded = flag;
}

void DeviceClient::GetCameraType(quint8 cameraIndex,CAMERA_TYPE_e &camType)
{
    devCamInfoLock.lock();
    camType = (cameraIndex < MAX_CAMERAS) ? camInfo[cameraIndex].camType : MAX_CAMERA_TYPE;
    devCamInfoLock.unlock();
}

//*****************************************************************************
//  storeRecStatusDay()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API stores the live events received in event request.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::storeRecStatusMonth(const char * str, quint32)
{
    quint8  dataLength;
    char    dataOut[MAX_SIZE_TO_CHECK_NEXT_FSP];

    recInMonth = 0;

    //Skip SOI
    str++;

    // parse index SOI
    if((ParseString(&str, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength)) == true)
    {
        recInMonth |=(((quint64)str[0] << 24) & 0xff000000);
        recInMonth |=(((quint64)str[1] << 16) & 0x00ff0000);
        recInMonth |=(((quint64)str[2] << 8) & 0x0000ff00);
        recInMonth |=(((quint64)str[3] & 0x000000ff));
    }
}

//*****************************************************************************
//  storeRecStatusDay()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API stores the live events received in event request.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::storeRecStatusDay(const char * str, quint64 size)
{
    quint8              camIndex ,camIndexRec, eventType, evtIndx;
    quint8              index = 0;
    quint8              dataLength;
    quint64             payloadSize = 0;
    quint8              singleByte;
    CAMERA_BIT_MASK_t   cameraMask;   // To store camera & event bitwise
    quint8              eventTotal[MAX_CAMERAS] = {0};

    memset(&cameraMask, 0, sizeof(cameraMask));
    // event According ot priority & Related Char To Append
    //      priority                Append Char
    //      shedule (4)             '1'
    //      Manual  (1)             '2'
    //      COSEC   (8)             '3'
    //      Alarm   (2)             '4'
    quint8  eventParse[MAX_EVENTS_FOR_REC] = {4, 1, 8, 2};
    char    appendChar[MAX_EVENTS_FOR_REC] = {'1', '2', '3', '4'};
    quint8  byteData[MAX_CAMERAS][MAX_SYNC_PB_SESSION][NO_OF_BYTES_SINGLE_DAY_REC];
    char    dataOut[MAX_SIZE_TO_CHECK_NEXT_FSP];
    quint8  overlapByte[MAX_CAMERAS] = {0};

    do
    {
        //Skip SOI
        str++;
        payloadSize++;

        // parse index SOI
        if((ParseString(&str, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            break;
        }
        payloadSize += dataLength;
        payloadSize++;

        //parse cam No
        if((ParseString(&str, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            break;
        }
        payloadSize += dataLength;
        payloadSize++;
        camIndexRec = atoi(dataOut);

        // parse event type
        if((ParseString(&str, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            break;
        }
        payloadSize += dataLength;
        payloadSize++;
        eventType = atoi(dataOut);

        if (camIndexRec == 0)
        {
            break;
        }

        camIndex = camIndexRec - 1;
        SET_CAMERA_MASK_BIT(cameraMask, camIndex);
        /* This array according to the cameras available type and users selected type, shifts the bits
         * as per the the available recording of each camera and user selected recording type, for storing type of recording  */
        eventTotal[camIndex] |= (1 << (eventType - 1));

        for(evtIndx = 0; evtIndx < MAX_EVENTS_FOR_REC; evtIndx++)
        {
            if(eventParse[evtIndx] == eventType)
            {
                break;
            }
        }

        for(index = 0; index < NO_OF_BYTES_SINGLE_DAY_REC; index++)
        {
            byteData[camIndex][evtIndx][index] =(quint8)(*(str + index));
        }

        // jump to 180 bytes + FSP
        str +=(NO_OF_BYTES_SINGLE_DAY_REC + 1);
        payloadSize +=(NO_OF_BYTES_SINGLE_DAY_REC + 1);

        //parse cam No
        if((ParseString(&str, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            break;
        }
        payloadSize += dataLength;
        payloadSize++;
        overlapByte[camIndex] |= atoi(dataOut);

        // remove EOI
        str ++;
        payloadSize ++;

    }while(payloadSize < size);

	for(camIndex = 0; camIndex < MAX_CAMERAS; camIndex++)
	{
        recInMinutes[camIndex].fill('0',(NO_OF_BITS_SINGLE_DAY_REC + 1));
        if(GET_CAMERA_MASK_BIT(cameraMask, camIndex) == 0)
		{
            continue;
        }

        for(quint8 evtNo = 0; evtNo < MAX_EVENTS_FOR_REC; evtNo++)
        {
            if ((eventTotal[camIndex] & (1 <<(eventParse[evtNo] - 1))) == 0)
            {
                continue;
            }

            for(quint8 loop = 0; loop < NO_OF_BYTES_SINGLE_DAY_REC; loop++)
            {
                singleByte = byteData[camIndex][evtNo][loop];
                for(quint8 bitIndex = 0; bitIndex < 8; bitIndex++)
                {
                    if(((quint64)1 << bitIndex) & singleByte)
                    {
                        recInMinutes[camIndex].replace(((loop * 8) +(bitIndex)), 1, appendChar[evtNo]);
                    }
                }
            }
        }

        if(overlapByte[camIndex] == 1)
        {
            recInMinutes[camIndex].replace(NO_OF_BITS_SINGLE_DAY_REC, 1, '1');
        }
    }
}

void DeviceClient::storeMotionInfo(const char *payload, quint16 size)
{
    char    dataOut[MAX_SIZE_TO_CHECK_NEXT_FSP];
    quint8  dataLength;
    quint16 payloadSize = 0;
    quint8  supportType;

    Q_UNUSED(size);
    memset(motionInfo, 0, sizeof(motionInfo));

    //Skip SOI
    payload++;
    payloadSize++;

    // parse index SOI
    if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
    {
        return;
    }
    payloadSize += dataLength;
    payloadSize++;

    //support method
    if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
    {
        return;
    }
    payloadSize += dataLength;
    payloadSize++;
    supportType = atoi(dataOut);

    motionInfo[0] = supportType;
    if(supportType == BLOCK_METHOD)
    {
        //sensitivity
        if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            return;
        }
        payloadSize += dataLength;
        payloadSize++;
        motionInfo[1] = atoi(dataOut);

        //isNoMotionEventSupport
        if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            return;
        }
        payloadSize += dataLength;
        payloadSize++;
        motionInfo[2] = atoi(dataOut);

        //MotionEvent
        if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            return;
        }
        payloadSize += dataLength;
        payloadSize++;
        motionInfo[3] = atoi(dataOut);

        //NoMotionDuration
        if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
        {
            return;
        }
        payloadSize += dataLength;
        payloadSize++;
        motionInfo[4] = atoi(dataOut);

        QString tempData = payload;

        //byteInfo
        for(quint16 index = 0; index < MAX_MOTION_BYTE; index++)
        {
            bool ok;
            motionInfo[index + MAX_MOTION_INFO_CONFIG_OPTION] = tempData.mid((index*2), 2).toUInt(&ok,16);
        }

        // jump to 180 bytes + FSP
        payload +=(MAX_MOTION_BYTE + 1);
        payloadSize +=(MAX_MOTION_BYTE + 1);

        // remove EOI
        payload++;
        payloadSize++;
    }
    else if(supportType == POINT_METHOD)
    {
        for(quint8 index = 0, fieldIndex = 0; index < MAX_MOTIONDETECTION_AREA; index++)
        {
            // startX
            fieldIndex = 0;
            if((ParseString(&payload, dataOut,MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
            {
                break;
            }
            payloadSize += dataLength;
            payloadSize++;
            motionInfo[((index * 5)  +(fieldIndex++) + 2)] = atoi(dataOut);

            // startY
            if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
            {
                break;
            }
            payloadSize += dataLength;
            payloadSize++;
            motionInfo[((index * 5)  +(fieldIndex++) + 2)] = atoi(dataOut);

            // width
            if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
            {
                break;
            }
            payloadSize += dataLength;
            payloadSize++;
            motionInfo[((index * 5)  +(fieldIndex++) +2)] = atoi(dataOut);

            // height
            if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
            {
                break;
            }
            payloadSize += dataLength;
            payloadSize++;
            motionInfo[((index * 5)  +(fieldIndex++) + 2)] = atoi(dataOut);

            // sentivity
            if((ParseString(&payload, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== false)
            {
                break;
            }
            payloadSize += dataLength;
            payloadSize++;
            motionInfo[((index * 5)  +(fieldIndex++) + 2)] = atoi(dataOut);
        }

        // jump to 20 data + FSP
        payload +=(20 + 1);
        payloadSize +=(20 + 1);

        // remove EOI
        payload++;
        payloadSize++;
    }
}

//*****************************************************************************
//  storeJPEGImgData()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API stores the live events received in event request.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::storeJPEGImgData(const char *str)
{
    quint8  dataLength;
    quint32 bytesToWrite = 0;
    char    dataOut[MAX_SIZE_TO_CHECK_NEXT_FSP];
    QFile   jpegFilePath;

    //Skip SOI
    str++;

    // parse index SOI
    if((ParseString(&str, dataOut, MAX_SIZE_TO_CHECK_NEXT_FSP, dataLength))== true)
    {
        bytesToWrite |=((quint32)(str[3] << 24) & 0xff000000);
        bytesToWrite |=((quint32)(str[2] << 16) & 0x00ff0000);
        bytesToWrite |=((quint32)(str[1] << 8) & 0x0000ff00);
        bytesToWrite |=((quint32)(str[0] & 0x000000ff));
    }

    // 4 byte data + FSP
    str += 5;

    jpegFilePath.setFileName(TST_CAM_IMAGE_FILE_PATH);
    jpegFilePath.open(QIODevice::ReadWrite);
    jpegFilePath.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);

    // read raw data in QByte array of received length
    jpegFilePath.write(reinterpret_cast <const char*>(str), bytesToWrite);
    jpegFilePath.flush();
    jpegFilePath.close();
}

//*****************************************************************************
//  storeLiveEvent()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API stores the live events received in event request.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::storeLvEvtAndHealthSts(QString payload)
{
    QStringList currEvtList;
    QString regString;
    QStringList eventFields;
    bool removeEvent = false;
    bool sendEvent = true;
    HEALTH_STS_PARAM_e evtParam = MAX_PARAM_STS;
    quint8 evtIndex = 0;
    LOG_EVENT_STATE_e evtState = EVENT_NORMAL;
    bool writeToHlthSts = false;
    QString popUpEvtDetail;
    QStringList popUpDetailList;
    quint8 camRights = 0;

    regString.append('[');
    regString.append(QChar(SOT));
    regString.append(QChar(SOI));
    regString.append(QChar(EOI));
    regString.append(QChar(EOT));
    regString.append(']');

    QRegExp regExp(regString);

    // split response payload in individual event string
    currEvtList = payload.split(regExp, QString::SkipEmptyParts);

    for(qint16 index = 0, maxLimit = currEvtList.length(); index < maxLimit; index++, removeEvent = false)
    {
        eventFields = currEvtList.value(index).split(FSP, QString::KeepEmptyParts);
        switch(eventFields.value(LV_EVT_TYPE).toInt())
        {
            case LOG_CAMERA_EVENT:
            {
                evtIndex =((eventFields.at(LV_EVT_DETAIL).toInt()) - 1);
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_MOTION_DETECTION:
                    case LOG_VIEW_TEMPERING:
                    {
                        evtParam =(HEALTH_STS_PARAM_e)(eventFields.value(LV_EVT_SUB_TYPE).toInt() - 1);
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState != EVENT_ACTIVE)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_LINE_CROSSING:
                    case LOG_INTRUSION_DETECTION:
                    case LOG_AUDIO_EXCEPTION_DETECTION:
                    case LOG_MISSING_OBJECT:
                    case LOG_SUSPICIOUS_OBJECT:
                    case LOG_LOITERING:
                    case LOG_OBJECT_COUNTING:
                    case LOG_NO_MOTION_DETECTION:
                    {
                        quint8 evntSubType =(eventFields.value(LV_EVT_SUB_TYPE).toInt());
                        switch(evntSubType)
                        {
                            case LOG_LINE_CROSSING:
                                evtParam = TRIP_WIRE_STS;
                                break;

                            case LOG_INTRUSION_DETECTION:
                                evtParam = OBJECT_INTRUSION_STS;
                                break;

                            case LOG_AUDIO_EXCEPTION_DETECTION:
                                evtParam = AUDIO_EXCEPTION_STS;
                                break;

                            case LOG_MISSING_OBJECT:
                                evtParam = MISSING_OBJJECT_STS;
                                break;

                            case LOG_SUSPICIOUS_OBJECT:
                                evtParam = SUSPIOUS_OBJECT_STS;
                                break;

                            case LOG_LOITERING:
                                evtParam = LOITERING_OBJECT_STS;
                                break;

                            case LOG_OBJECT_COUNTING:
                                evtParam = OBJECT_COUNTING_STS;
                                 break;

                            case LOG_NO_MOTION_DETECTION:
                                evtParam = MOTION_DETECTION_STS;
                                 break;

                            default:
                                evtParam =(HEALTH_STS_PARAM_e)(eventFields.value(LV_EVT_SUB_TYPE).toInt() - 1);
                                break;
                        }

                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState != EVENT_ACTIVE)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_CAMERA_SENSOR_1:
                    case LOG_CAMERA_SENSOR_2:
                    case LOG_CAMERA_SENSOR_3:
                    case LOG_CAMERA_ALARM_1:
                    case LOG_CAMERA_ALARM_2:
                    case LOG_CAMERA_ALARM_3:
                    {
                        evtParam =(HEALTH_STS_PARAM_e)eventFields.value(LV_EVT_SUB_TYPE).toInt();
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState != EVENT_ACTIVE)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_CONNECTIVITY:
                    {
                        evtParam = CAM_CONN_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState != EVENT_DISCONNECT)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_MANUAL_RECORDING:
                    case LOG_ALARM_RECORDING:
                    case LOG_SCHEDULE_RECORDING:
                    case LOG_SNAPSHOT_SCHEDULE:
                    {
                        evtParam =(HEALTH_STS_PARAM_e)(eventFields.value(LV_EVT_SUB_TYPE).toInt() - 1);
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_PRESET_TOUR:
                    {
                        evtParam = PTZ_TOUR_TYPE;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_VIDEO_POP_UP:
                    {
                        evtIndex =((eventFields.at(LV_EVT_DETAIL).toInt()) - 1);
                        USRS_GROUP_e groupType;
                        removeEvent = true;
                        sendEvent = false;
                        GetCameraRights(evtIndex,camRights);
                        GetUserGroup(groupType);

                        if(((camRights >> MONITORING_BITPOSITION)& 0x01) &&((camRights >> VIDEO_POPUP_BITPOSITION)& 0x01))
                        {
                            sendEvent = true;
                        }
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            case LOG_SENSOR_EVENT:
            {
                evtIndex =((eventFields.at(LV_EVT_DETAIL).toInt()) - 1);
                evtParam = SENSOR_STS;
                evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                if(evtState != EVENT_ACTIVE)
                {
                    removeEvent = true;
                }
                writeToHlthSts = true;
            }
            break;

            case LOG_ALARM_EVENT:
            {
                evtIndex =((eventFields.at(LV_EVT_DETAIL).toInt()) - 1);
                evtParam = ALARM_STS;
                evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                if(evtState != EVENT_ACTIVE)
                {
                    removeEvent =true;
                }
                writeToHlthSts = true;
            }
            break;

            case LOG_SYSTEM_EVENT:
            {
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_MAINS_EVENT:
                    {
                        evtIndex = 0;
                        evtParam = MAINS_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState != EVENT_FAIL)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_RTC_UPDATE:
                    {
                        if(eventFields.value(LV_EVT_STATE).toInt() != EVENT_AUTO)
                        {
                            removeEvent = true;
                        }
                    }
                    break;

                    case LOG_SHUTDOWN:
                    case LOG_RESTART:
                    case LOG_SYSTEM_RESET:
                    case LOG_UNAUTH_IP_ACCESS:
                    case LOG_RECORDING_RESTART:
                    case LOG_FIRMWARE_UPGRADE:
                    {
                        /* Nothing to do */
                    }
                    break;

                    case LOG_SCHEDULE_BACKUP:
                    {
                        evtIndex = 0;
                        evtParam = SCHEDULE_BACKUP_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState == EVENT_COMPLETE)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_AUTO_CFG_STS_REPORT:
                    {
                        removeEvent = true;
                    }
                    break;

                    case LOG_TIME_ZONE_UPDATE:
                    {
                        removeEvent = true;
                    }
                    break;

                    case LOG_TWO_WAY_AUDIO:
                    {
                        // Checked whether event is for local device or not.
                        // If it is for local device then only we have to process further.
                        if(devIndex != 0)
                        {
                            sendEvent = false;
                        }
                        removeEvent = true;
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            case LOG_STORAGE_EVENT:
            {
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_HDD_STATUS:
                    {
                        evtIndex = 0;
                        evtParam = DISK_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState == EVENT_NORMAL)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_HDD_VOL_CLEAN_UP:
                    case LOG_HDD_VOLUME:
                    {
                        /* Nothing to do */
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            case LOG_NETWORK_EVENT:
            {
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_ETHERNET_LINK:
                    case LOG_UPLOAD_IMAGE:
                    case LOG_EMAIL_NOTIFICATION:
                    case LOG_TCP_NOTIFICATION:
                    case LOG_SMS_NOTIFICATION:
                    {
                        if(eventFields.value(LV_EVT_STATE).toInt() != EVENT_FAIL)
                        {
                            removeEvent = true;
                        }
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            case LOG_OTHER_EVENT:
            {
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_UPGRADE_START:
                    case LOG_RESTORE_CONFIG_STRAT:
                    {
                        if(eventFields.value(LV_EVT_STATE).toInt() != EVENT_ALERT)
                        {
                            removeEvent = true;
                        }
                    }
                    break;

                    case LOG_UPGRADE_RESULT:
                    case LOG_RESTORE_CONFIG_RESULT:
                    {
                        if(eventFields.value(LV_EVT_STATE).toInt() != EVENT_FAIL)
                        {
                            removeEvent = true;
                        }
                    }
                    break;

                    case LOG_BUZZER_STATUS:
                    {
                        evtIndex = 0;
                        evtParam = BUZZER_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if((evtState == EVENT_NORMAL) ||(eventFields.at(LV_EVT_ADV_DETAIL) == ""))
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_USB_STATUS:
                    {
                        evtIndex =(eventFields.at(LV_EVT_DETAIL).toInt() - 1);
                        evtParam = USB_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        removeEvent = true;
                        writeToHlthSts = true;
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            case LOG_USER_EVENT:
            {
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_MANUAL_TRIGGER:
                    {
                        evtIndex = 0;
                        evtParam = MANUAL_TRIGGER_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        if(evtState != EVENT_ACTIVE)
                        {
                            removeEvent = true;
                        }
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_CONFIG_CHANGE:
                    {
                        DPRINT(CONFIG_PAGES, "config change event: [table=%d]", eventFields.value(LV_EVT_DETAIL).toInt());
                        if((eventFields.value(LV_EVT_STATE).toInt() != EVENT_CHANGE)
                                || (eventFields.value(LV_EVT_DETAIL).toInt() != GENERAL_TABLE_INDEX)
                                || (eventFields.value(LV_EVT_DETAIL).toInt() != CAMERA_TABLE_INDEX)
                                || (eventFields.value(LV_EVT_DETAIL).toInt() != NETWORK_DEVICE_SETTING_TABLE_INDEX))
                        {
                            removeEvent = true;
                        }

                        if(eventFields.value(LV_EVT_DETAIL).toInt() == GENERAL_TABLE_INDEX)
                        {
                            getCommonCfg(GENERAL_TABLE_INDEX);
                        }
                        else if(eventFields.value(LV_EVT_DETAIL).toInt() == CAMERA_TABLE_INDEX)
                        {
                            getCommonCfg(CAMERA_TABLE_INDEX);
                        }
                        else if((strcmp(devConfig.deviceName,LOCAL_DEVICE_NAME) == 0)
                                && (eventFields.value(LV_EVT_DETAIL).toInt() == NETWORK_DEVICE_SETTING_TABLE_INDEX))
                        {
                            getCommonCfg(NETWORK_DEVICE_SETTING_TABLE_INDEX,true);
                        }
                    }
                    break;

                    case LOG_SYS_CONFIGURATION:
                    {
                        /* Nothing to do */
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            case LOG_COSEC_EVENT:
            {
                switch(eventFields.value(LV_EVT_SUB_TYPE).toInt())
                {
                    case LOG_COSEC_RECORDING:
                    {
                        evtIndex =((eventFields.at(LV_EVT_DETAIL).toInt()) - 1);
                        evtParam = COSEC_RECORDING_STS;
                        evtState =(LOG_EVENT_STATE_e)eventFields.at(LV_EVT_STATE).toInt();
                        writeToHlthSts = true;
                    }
                    break;

                    case LOG_COSEC_VIDEO_POP_UP:
                    {
                        popUpEvtDetail = eventFields.at(LV_EVT_DETAIL);
                        popUpDetailList = popUpEvtDetail.split('/');

                        devConfigLock.lockForRead();
                        emit sigPopUpEvent(devConfig.deviceName,
                                           popUpDetailList.at(CAMERA_INDEX).toInt(),
                                           popUpDetailList.at(USER_NAME),
                                           popUpDetailList.at(POP_UP_TIME).toInt(),
                                           popUpDetailList.at(USER_ID),
                                           eventFields.at(LV_EVT_ADV_DETAIL),
                                           eventFields.at(LV_EVT_STATE).toInt());
                        devConfigLock.unlock();
                        removeEvent = true;
                    }
                    break;

                    default:
                    {
                        removeEvent = true;
                    }
                    break;
                }
            }
            break;

            default:
            {
                removeEvent = true;
            }
            break;
        }

        if(writeToHlthSts == true)
        {
            if((evtParam < MAX_PARAM_STS) &&(evtIndex < MAX_CAMERAS))
            {
                updateSinglePrmHlthSts(evtParam, evtIndex, evtState);
            }
        }

        if(((eventFields.value(LV_EVT_TYPE).toInt() != LOG_COSEC_EVENT)
            || (eventFields.value(LV_EVT_SUB_TYPE).toInt() != LOG_COSEC_VIDEO_POP_UP)) && (sendEvent))
        {
            devConfigLock.lockForRead();
            emit sigEvent(devConfig.deviceName,
                          ((LOG_EVENT_TYPE_e)eventFields.value(LV_EVT_TYPE).toInt()),
                          ((LOG_EVENT_SUBTYPE_e)eventFields.value(LV_EVT_SUB_TYPE).toInt()),
                          (eventFields.value(LV_EVT_DETAIL).toInt()),
                          ((LOG_EVENT_STATE_e)eventFields.value(LV_EVT_STATE).toInt()),
                           eventFields.value(LV_EVT_ADV_DETAIL),
                          ((bool)(!(removeEvent))));
            devConfigLock.unlock();
        }

        if(removeEvent == true)
        {
            currEvtList.removeAt(index);
            index--;
            maxLimit--;
        }
    }

    // make room for new events to fit in buffer
    liveEventListLock.lock();
    while((liveEventList.count() + currEvtList.count()) > MAX_LIVE_EVENT_LIMIT)
    {
        if(liveEventList.isEmpty() == false)
        {
            liveEventList.removeLast();
        }
        else
        {
            currEvtList.removeFirst();
        }
    }

    // write events to list
    eventCountAccess.lock();
    for (quint8 index = 0, maxLimit = currEvtList.length(); index < maxLimit; index++)
    {
        liveEventCount++;
        liveEventList.prepend(currEvtList.value(index));
    }
    eventCountAccess.unlock();
    liveEventListLock.unlock();

    // return status
    return true;
}

//*****************************************************************************
//  GetDeviceHlthStatus()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API return health status of particular device.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::GetSinglePrmHlthStatus(quint8 *list, quint8 paramIndex)
{
    healthStatusLock.lock();
    memcpy(list, healthStatus[paramIndex], sizeof(healthStatus[paramIndex]));
    healthStatusLock.unlock();
}

void DeviceClient::GetAllHlthStatusParam(quint8 *ptr)
{
    healthStatusLock.lock();
    memcpy(ptr, healthStatus, sizeof(healthStatus));
    healthStatusLock.unlock();
}

void DeviceClient::GetSigleParamSingleCameraStatus(quint8 &paramvalue, quint8 paramIndex, quint8 cameraIndex)
{
    healthStatusLock.lock();
    paramvalue = healthStatus[paramIndex][cameraIndex];
    healthStatusLock.unlock();
}

//*****************************************************************************
//  updateDeviceHlthStatus()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API update health status of particular device.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::updateDeviceHlthStatus(QString healtPayload)
{
    quint8 param, index;
    QString tempStr, singleStr;
    QStringList healthList;
    QString regString;

    regString.append('[');
    regString.append(QChar(SOI));
    regString.append(QChar(FSP));
    regString.append(QChar(EOI));
    regString.append(']');

    QRegExp regExp(regString);

    // split response payload in individual event string
    healthList = healtPayload.split(regExp, QString::SkipEmptyParts);
    healthList.removeFirst();

    healthStatusLock.lock();
    memset(&healthStatus,0,sizeof(healthStatus));

    for(param = 0; ((param < APPL_MAX_PARAM_STS) && (param < healthList.length())); param++)
    {
        if(healthList.length() == param)
        {
            break;
        }

        tempStr = healthList.at(param);
        switch(param)
        {
            case APPL_MOTION_DETECTION_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[MOTION_DETECTION_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_VIEW_TAMPERED_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[VIEW_TAMPERED_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_CONN_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_CONN_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_SENSOR1_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_SENSOR1_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_SENSOR2_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_SENSOR2_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_SENSOR3_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_SENSOR3_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_ALARM1_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_ALARM1_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_ALARM2_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_ALARM2_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_CAM_ALARM3_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_ALARM3_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_MANUAL_RECORDING_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[MANUAL_RECORDING_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_ADAPTIVE_RECORDING_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[ADAPTIVE_RECORDING_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_ALARM_RECORDING_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[ALARM_RECORDING_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_SCHEDULE_RECORDING_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[SCHEDULE_RECORDING_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_PTZ_TOUR_TYPE:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[PTZ_TOUR_TYPE][index] = singleStr.toInt();
                }
                break;

            case APPL_SENSOR_STS:
                for(index = 0; index < MAX_DEV_SENSOR; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[SENSOR_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_ALARM_STS:
                for(index = 0; index < MAX_DEV_ALARM; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[ALARM_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_DISK_STS:
                singleStr = tempStr.mid(0, 1);
                healthStatus[DISK_STS][0] = singleStr.toInt();
                break;

            case APPL_SCHEDULE_BACKUP_STS:
                singleStr = tempStr.mid(0, 1);
                healthStatus[SCHEDULE_BACKUP_STS][0] = singleStr.toInt();
                break;

            case APPL_MANUAL_TRIGGER_STS:
                singleStr = tempStr.mid(0, 1);
                healthStatus[MANUAL_TRIGGER_STS][0] = singleStr.toInt();
                break;

            case APPL_MAINS_STS:
                break;

            case APPL_BUZZER_STS:
                singleStr = tempStr.mid(0, 1);
                healthStatus[BUZZER_STS][0] = singleStr.toInt();
                break;

            case APPL_USB_STS:
                for(index = 0; index < MAX_USB_STS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[USB_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_COSEC_RECORDING_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[COSEC_RECORDING_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_STREAM_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[CAM_STREAM_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_TRIP_WIRE_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[TRIP_WIRE_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_OBJECT_INTRUSION_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[OBJECT_INTRUSION_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_AUDIO_EXCEPTION_STS:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[AUDIO_EXCEPTION_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_MISSING_OBJECT:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[MISSING_OBJJECT_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_SUSPIOUS_OBJECT:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[SUSPIOUS_OBJECT_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_LOTERING:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[LOITERING_OBJECT_STS][index] = singleStr.toInt();
                }
                break;

            case APPL_OBJECT_COUNTING:
                for(index = 0; index < MAX_CAMERAS; index++)
                {
                    //update for all max camera
                    singleStr = tempStr.mid(index, 1);
                    healthStatus[OBJECT_COUNTING_STS][index] = singleStr.toInt();
                }
                break;

            default:
                break;
        }
    }
    healthStatusLock.unlock();
}

//*****************************************************************************
//  updateSinglePrmHlthSts()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API update health status single parameter of particular device.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::updateSinglePrmHlthSts(quint8 param, quint8 index, LOG_EVENT_STATE_e state)
{
    healthStatusLock.lock();
    if(param < MAX_PARAM_STS)
    {
        healthStatus[param][index] = quint8(state);
    }
    healthStatusLock.unlock();
}

quint8 DeviceClient::GetCamRecType(quint8 iCameraId)
{
    quint8  tCamRecType = 0;

	if(iCameraId >= MAX_CAMERAS)
	{
        EPRINT(DEVICE_CLIENT, "invld camera id: [cameraId=%d]", iCameraId);
        return tCamRecType;
	}

	/* Scan CamRec for Whole Day Time  */
	for(quint16 tIndex = 0; tIndex < 1440; tIndex++)
	{
		/* LSB bit not to be made 1 */
		if(recInMinutes[iCameraId].at(tIndex).digitValue() != 0)
		{
			/* Update RecType */
            tCamRecType |=(1 <<(recInMinutes[iCameraId].at(tIndex).digitValue()));
		}
	}

    return tCamRecType;
}

bool DeviceClient::GetCamRecData(quint8 iCameraId, QList<quint16> &pCamRecData)
{
    bool    tIsRecAvailableF = false;
    quint16 tTotalRect = 0;
    quint8  tPrevColor = 5;
    quint8  tReservePlace = 1; // 1st for overlapfalg
    quint8  tEndingIndexField = 2;

	if((iCameraId >= MAX_CAMERAS))
	{
        EPRINT(DEVICE_CLIENT, "invld camera id: [cameraId=%d]", iCameraId);
        return false;
	}

	pCamRecData.clear();
    pCamRecData.append(recInMinutes[iCameraId].at(1440).digitValue());

	/* Scan CamRec for Whole Day Time  */
	for(quint16 tIndex = 0; tIndex < 1440; tIndex++)
	{
		if(recInMinutes[iCameraId].at(tIndex).digitValue() != tPrevColor)//this digitValue is the type of recording found
		{
			tPrevColor = recInMinutes[iCameraId].at(tIndex).digitValue();

			/* Append color */
			pCamRecData.append(tPrevColor);

			/* Append StartTime */
			pCamRecData.append(tIndex);

			/* Append EndTime */
			pCamRecData.append(tIndex);
			tTotalRect++;
		}
		else
		{
			pCamRecData.replace((((tTotalRect -1) * 3) + tEndingIndexField + tReservePlace), tIndex);
		}

		if(false == tIsRecAvailableF)
		{
            if(	(recInMinutes[iCameraId].at(tIndex).digitValue() != 0) && (recInMinutes[iCameraId].at(tIndex).digitValue() < 5))
			{
				tIsRecAvailableF = true;
			}
		}
	}

    return tIsRecAvailableF;
}

bool DeviceClient::IsCamRecAvailable(quint8 iCameraId, quint16 iStartTime, quint16 iEndTime)
{
    if((iCameraId >= MAX_CAMERAS) || (iStartTime > 1440) || (iEndTime > 1440))
	{
        EPRINT(DEVICE_CLIENT, "invld input param: [cameraId=%d], [startTime=%d], [endTime=%d]", iCameraId, iStartTime, iEndTime);
        return false;
	}

	/* Scan CamRec for given StartTime and EndTime */
	for(quint16 index = iStartTime; index <= iEndTime; index++)
	{
		/* Return If CamRec is available */
        if(	(recInMinutes[iCameraId].at(index).digitValue() != 0) && (recInMinutes[iCameraId].at(index).digitValue() < 5))
		{
            return true;
		}
	}

    return false;
}

//*****************************************************************************
//  getMonthRec()
//      Param:
//          IN : QString payload
//          OUT: NONE
//	Returns:qml column
//		bool [true / false]
//      Description:
//          This API return recording status of particular month.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
quint64 DeviceClient::GetMonthRec(void)
{
    return recInMonth;
}

void DeviceClient::GetMotionInfo(quint32 *resArray)
{
    memcpy(resArray, motionInfo, sizeof(motionInfo));
}

//*****************************************************************************
//	ParseString()
//	Param:
//		IN:  src - source ponter from which to parse string.
//	             maxDestSize - Length of string buffer to be filled.
//		OUT: dest - Pointer to destination buffer where parse string
//                          is stored.
//	Returns:
//		None
//	Description:
//		This function will parse string from source buffer at current
//              pointer till FSP and copy to destination buffer
//
//*****************************************************************************
bool DeviceClient::ParseString(const char **src, char *dest, unsigned char maxDestSize, quint8 &dataSize)
{
    const char *sptr;
    char *eptr;
    quint8 length;

    sptr = *src;
    eptr =(char *)memchr(*src, FSP, maxDestSize);
    if(eptr == NULL)
    {
        return false;
    }

    length = eptr - sptr;
    dataSize = length;

    if (length >= maxDestSize)
    {
        return false;
    }

    strncpy(dest, *src, length);
    dest[length] = 0;
    *src +=(length + 1);
    return true;
}

//*****************************************************************************
//  getHeathStsFrmDev()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for connect request signal.
//          In response of login request it verifies device against the
//          parameters set by user.
//          In response of polling request it determines the device connectivity
//          state.
//          In response of event request it stores the received events to
//          internal buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::getHeathStsFrmDev()
{
    QString payload = "";
    quint8 portIndex = 0;

    if(IS_VALID_OBJ(connectRequest))
    {
        portIndex = connectRequest->getPortIndex();
    }

    SERVER_INFO_t serverInfo;
    REQ_INFO_t requestInfo;
    quint8 cmdSesId;
    DEVICE_REPLY_TYPE_e response = CMD_SUCCESS;

    // store server patameters
    devConfigLock.lockForRead();
    serverInfo.ipAddress = devConfig.ipAddress;
    serverInfo.tcpPort =(portIndex == 0) ? devConfig.port : devConfig.forwardedTcpPort;
    devConfigLock.unlock();

    // store request parameters
    sessionInfoLock.lockForRead();
    requestInfo.sessionId = loginRespParam.value(LOGIN_SESSION_ID);
    sessionInfoLock.unlock();
    requestInfo.requestId = MSG_SET_CMD;
    requestInfo.payload = payload;
    requestInfo.bytePayload = NULL;

    if (false == createCommandReq(serverInfo, requestInfo, HEALTH_STS, cmdSesId))
    {
        return false;
    }

    commandRequest[cmdSesId]->getBlockingRes(payload, response);
    deleteCommandReq(cmdSesId);
    if (response != CMD_SUCCESS)
    {
        return false;
    }

    updateDeviceHlthStatus(payload);
    return true;
}

//*****************************************************************************
//  getCommonCfg()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for connect request signal.
//          In response of login request it verifies device against the
//          parameters set by user.
//          In response of polling request it determines the device connectivity
//          state.
//          In response of event request it stores the received events to
//          internal buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::getCommonCfg(quint8 tableId, bool isUpdateOnLiveEvent)
{
    QString payload;
    SERVER_INFO_t serverInfo;
    REQ_INFO_t requestInfo;
    DEVICE_REPLY_TYPE_e response = CMD_SUCCESS;
    quint8 portIndex = 0;
	quint8 genReqSesId = MAX_GEN_REQ_SESSION;

    if(IS_VALID_OBJ(connectRequest))
    {
        portIndex = connectRequest->getPortIndex();
    }

    payload.clear();
    switch(tableId)
    {
        case GENERAL_TABLE_INDEX:
            payload.append(SOT);
            payload.append(QString("%1").arg(tableId));    // table id
            payload.append(FSP);
            payload.append("1");    // frm index
            payload.append(FSP);
            payload.append("1");    // to index
            payload.append(FSP);
            payload.append("1");    // frm field
            payload.append(FSP);
            payload.append(QString("%1").arg(FIELD_START_LIVE_VIEW_FLAG+1));    // to field
            payload.append(FSP);
            payload.append(EOT);
            break;

        case CAMERA_TABLE_INDEX:
            payload.append(SOT);
            payload.append(QString("%1").arg(tableId)); // table id
            payload.append(FSP);
            payload.append("1");    // frm index
            payload.append(FSP);
            deviceTableInfoLock.lock();
            payload.append(QString("%1").arg(tableInfo.totalCams)); // max camera to index
            deviceTableInfoLock.unlock();
            payload.append(FSP);
            payload.append(QString("%1").arg(CAMERA_SETTINGS_IS_ENABLE_STATUS+1));          // frm field
            payload.append(FSP);
            payload.append(QString("%1").arg(CAMERA_SETTINGS_CAMERA_TEXT_POSITION_0+1));    // to field
            payload.append(FSP);
            payload.append(EOT);
            break;

        case NETWORK_DEVICE_SETTING_TABLE_INDEX:
            payload.append(SOT);
            payload.append(QString("%1").arg(tableId));    // table id
            payload.append(FSP);
            payload.append("1");   // frm index
            payload.append(FSP);
            payload.append(QString("%1").arg(MAX_REMOTE_DEVICES));   // max Remote Devices
            payload.append(FSP);
            payload.append("1");   // frm field
            payload.append(FSP);
            payload.append(QString("%1").arg(MAX_DEVICE_SETTINGS_FIELDS));  // to field
            payload.append(FSP);
            payload.append(EOT);
            break;

        default:
            break;
    }

    // store server patameters
    devConfigLock.lockForRead();
    serverInfo.ipAddress = devConfig.ipAddress;
    serverInfo.tcpPort =(portIndex == 0) ? devConfig.port : devConfig.forwardedTcpPort;
    devConfigLock.unlock();

    // store request parameters
    sessionInfoLock.lockForRead();
    requestInfo.sessionId = loginRespParam.value(LOGIN_SESSION_ID);
    sessionInfoLock.unlock();
    requestInfo.requestId = MSG_GET_CFG;
    requestInfo.payload = payload;
    requestInfo.bytePayload = NULL;
    requestInfo.windowId = winId;
    if (false == createConfigReq(serverInfo, requestInfo, genReqSesId))
    {
        return false;
    }

    configRequest[genReqSesId]->getBlockingRes(payload, response);
    deleteConfigReq(genReqSesId);
    if(response != CMD_SUCCESS)
    {
        return false;
    }

    switch(tableId)
    {
        case CAMERA_TABLE_INDEX:
            storeCamCfg(payload);
            break;

        case NETWORK_DEVICE_SETTING_TABLE_INDEX:
            storeRemoteDeviceCfg(payload, isUpdateOnLiveEvent);
            break;

        case GENERAL_TABLE_INDEX:
            storeGeneralCfg(payload);
            break;

        default:
            break;
    }

    return true;
}

//*****************************************************************************
//  storeCamCfg()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for connect request signal.
//          In response of login request it verifies device against the
//          parameters set by user.
//          In response of polling request it determines the device connectivity
//          state.
//          In response of event request it stores the received events to
//          internal buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::storeCamCfg(QString payload)
{
    QString regString;
    QStringList cfgList, cfgFields;
    quint8 camIndex;
    QStringList fieldAndValue;
    quint8 fieldNo;
    QString fieldValue;

    regString.append('[');
    regString.append(QChar(SOT));
    regString.append(QChar(SOI));
    regString.append(QChar(EOI));
    regString.append(QChar(EOT));
    regString.append(']');

    QRegExp regExp(regString);

    cfgList = payload.split(regExp, QString::SkipEmptyParts);
    cfgList.removeFirst();

    deviceTableInfoLock.lock();
    quint8 totalCam = tableInfo.totalCams;
    deviceTableInfoLock.unlock();

    devCamInfoLock.lock();
    for(quint8 index = 0; index < totalCam; index++)
    {
        cfgFields = cfgList.at(index).split(FSP, QString::KeepEmptyParts);
        camIndex = cfgFields.at(0).toUInt();
        camIndex--;

        for(quint8 field = 1; field <(cfgFields.length() - 1); field++)
        {
            fieldAndValue = cfgFields.at(field).split(FVS);
            fieldNo = fieldAndValue.at(0).toUInt();
            fieldValue = fieldAndValue.at(1);
            fieldNo--;

            switch(fieldNo)
            {
                case CAMERA_SETTINGS_IS_ENABLE_STATUS:
                    camInfo[camIndex].camStatus =(fieldValue.toUInt() == 0) ? false : true;
                    break;

                case CAMERA_SETTINGS_CAMERA_NAME:
                    camInfo[camIndex].camName = fieldValue;
                    break;

                case CAMERA_SETTINGS_CAMERA_TYPE:
                    camInfo[camIndex].camType =(CAMERA_TYPE_e)fieldValue.toUInt();
                    break;

                case CAMERA_SETTINGS_CAMERA_NAME_OSD:
                    camInfo[camIndex].nameOsdPosition =(OSD_POSITION_e)fieldValue.toUInt();
                    break;

                case CAMERA_SETTINGS_CAMERA_STATUS_OSD:
                    camInfo[camIndex].statusOsdPosition =(OSD_POSITION_e)fieldValue.toUInt();
                    break;

                case CAMERA_SETTINGS_CAMERA_DATE_TIME_OVERLAY:
                    camInfo[camIndex].dateTimerOverlay =((fieldValue.toUInt() == 0) ? false : true);
                    break;

                case CAMERA_SETTINGS_CAMERA_DATE_TIME_POSITION:
                    camInfo[camIndex].dateTimePosition =(OSD_POSITION_e)fieldValue.toUInt();
                    break;

                case CAMERA_SETTINGS_CAMERA_TEXT_OVERLAY:
                    camInfo[camIndex].textOverlay =((fieldValue.toUInt() == 0) ? false : true);
                    break;

                case CAMERA_SETTINGS_CAMERA_TEXT_0:
                    camInfo[camIndex].camText = fieldValue;
                    break;

                case CAMERA_SETTINGS_CAMERA_TEXT_POSITION_0:
                    camInfo[camIndex].textPosition =(OSD_POSITION_e)fieldValue.toUInt();
                    break;

                default:
                    break;
            }
        }
    }

    devCamInfoLock.unlock();
    DPRINT(CONFIG_PAGES, "camera list updated");
    return true;
}

//*****************************************************************************
//  storeRemoteDeviceCfg()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for connect request signal.
//          In response of login request it verifies device against the
//          parameters set by user.
//          In response of polling request it determines the device connectivity
//          state.
//          In response of event request it stores the received events to
//          internal buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool DeviceClient::storeRemoteDeviceCfg(QString payload, bool isUpdateOnLiveEvent)
{
    QString regString;
    QStringList cfgList, cfgFields;
    quint8 remoteDeviceIndex;
    QStringList fieldAndValue;
    quint8 fieldNo;
    QString fieldValue;

    regString.append('[');
    regString.append(QChar(SOT));
    regString.append(QChar(SOI));
    regString.append(QChar(EOI));
    regString.append(QChar(EOT));
    regString.append(']');

    QRegExp regExp(regString);

    cfgList = payload.split(regExp, QString::SkipEmptyParts);
    cfgList.removeFirst();

    for(quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        cfgFields = cfgList.at(index).split(FSP, QString::KeepEmptyParts);
        remoteDeviceIndex = cfgFields.at(0).toUInt();

        for(quint8 field = 1; field < (cfgFields.length() - 1); field++)
        {
            fieldAndValue = cfgFields.at(field).split(FVS);
            fieldNo = fieldAndValue.at(0).toUInt();
            fieldValue = fieldAndValue.at(1);
            fieldNo--;

            switch(fieldNo)
            {
                case DEV_SET_DEVICE_NAME:
                    snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].deviceName, MAX_DEVICE_NAME_SIZE, "%s", fieldValue.toUtf8().constData());
                    break;

                case DEV_SET_REGISTER_MODE:
                    rdevTempConfig[(remoteDeviceIndex - 1)].connType =(CONNECTION_TYPE_e)fieldValue.toUInt();
                    break;

                case DEV_SET_REGISTER_MODE_ADDRESS:
                     snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].ipAddress, MAX_IP_ADDRESS_SIZE, "%s", fieldValue.toLatin1().constData());
                    break;

                case DEV_SET_PORT:
                    rdevTempConfig[(remoteDeviceIndex - 1)].port = fieldValue.toUInt();
                    break;

                case DEV_SET_USERNAME:
                    snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].username, MAX_USERNAME_SIZE, "%s", fieldValue.toLatin1().constData());
                    break;

                case DEV_SET_PASSWORD:
                    snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].password, MAX_PASSWORD_SIZE, "%s", fieldValue.toLatin1().constData());
                    break;

                case DEV_SET_ENABLE:
                    rdevTempConfig[(remoteDeviceIndex - 1)].enable =((fieldValue.toUInt() == 0) ? false : true);
                    break;

                case DEV_SET_AUTOLOGIN:
                    rdevTempConfig[(remoteDeviceIndex - 1)].autoLogin =((fieldValue.toUInt() == 0) ? false : true);
                    break;

                case DEV_SET_LIVE_STREAM:
                    rdevTempConfig[(remoteDeviceIndex - 1)].liveStreamType = fieldValue.toUInt();
                    break;

                case DEV_SET_PREFER_NATIVE_DEV_CREDENTIAL:
                    rdevTempConfig[(remoteDeviceIndex - 1)].nativeDeviceCredential = ((fieldValue.toUInt() == 0) ? false : true);
                    break;

                case DEV_SET_FORWARDED_PORT:
                    rdevTempConfig[(remoteDeviceIndex - 1)].forwardedTcpPort = fieldValue.toUInt();
                    break;

                default:
                    break;
            }
        }

        // signal to notify change in device update
        emit sigDeviceCfgUpdate(remoteDeviceIndex, &rdevTempConfig[(remoteDeviceIndex-1)], isUpdateOnLiveEvent);
    }

    return true;
}

void DeviceClient::storeGeneralCfg(QString payload)
{
    QString regString;
    QStringList cfgList, cfgFields;
    QStringList fieldAndValue;
    quint8 fieldNo;
    QString fieldValue;

    regString.append('[');
    regString.append(QChar(SOT));
    regString.append(QChar(SOI));
    regString.append(QChar(EOI));
    regString.append(QChar(EOT));
    regString.append(']');

    QRegExp regExp(regString);

    cfgList = payload.split(regExp, QString::SkipEmptyParts);
    cfgList.removeFirst();
    cfgFields = cfgList.at(0).split(FSP, QString::KeepEmptyParts);

    for(quint8 field = 1; field < (cfgFields.length() - 1); field++)
    {
        fieldAndValue = cfgFields.at(field).split(FVS);
        fieldNo = fieldAndValue.at(0).toUInt();
        fieldValue = fieldAndValue.at(1);
        fieldNo--;

        switch(fieldNo)
        {
            case FIELD_DEV_NAME:
                deviceTableInfoLock.lock();
                m_dispDevName = fieldValue;
                deviceTableInfoLock.unlock();
                break;

            case FIELD_AUTO_CLOSE_REC_FAIL_ALERT:
                deviceTableInfoLock.lock();
                tableInfo.autoCloseRecFailAlert =(fieldValue.toInt() ==  0) ? false : true;
                deviceTableInfoLock.unlock();
                break;

            case FIELD_VIDEO_POP_UP_DURATION:
                deviceTableInfoLock.lock();
                tableInfo.videoPopUpDuration = fieldValue.toInt();
                deviceTableInfoLock.unlock();
                break;

            case FIELD_PRE_VIDEO_LOSS_DURATION:
                deviceTableInfoLock.lock();
                tableInfo.preVideoLossDuration = fieldValue.toInt();
                deviceTableInfoLock.unlock();
                break;

            case FIELD_START_LIVE_VIEW_FLAG:
                deviceTableInfoLock.lock();
                tableInfo.startLiveView =(fieldValue.toInt() ==  0) ? false : true;
                deviceTableInfoLock.unlock();
                break;

            case FIELD_RECORDING_FORMAT:
                deviceTableInfoLock.lock();
                tableInfo.recordFormatType = fieldValue.toInt();
                deviceTableInfoLock.unlock();
                break;

            default:
                break;
        }
    }
}

//*****************************************************************************
//  GetDevCamList()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for connect request signal.
//          In response of login request it verifies device against the
//          parameters set by user.
//          In response of polling request it determines the device connectivity
//          state.
//          In response of event request it stores the received events to
//          internal buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::GetDevCamList(quint8 camIndex, DEV_CAM_INFO_t &nameList)
{
    camIndex = camIndex - 1;
    if (camIndex >= MAX_CAMERAS)
    {
        return;
    }

    devCamInfoLock.lock();
    nameList.camStatus = camInfo[camIndex].camStatus;
    nameList.camName = camInfo[camIndex].camName;
    nameList.nameOsdPosition = camInfo[camIndex].nameOsdPosition;
    nameList.statusOsdPosition = camInfo[camIndex].statusOsdPosition;
    nameList.camType = camInfo[camIndex].camType;
    nameList.dateTimerOverlay = camInfo[camIndex].dateTimerOverlay;
    nameList.dateTimePosition = camInfo[camIndex].dateTimePosition;
    nameList.textOverlay = camInfo[camIndex].textOverlay;
    nameList.camText = camInfo[camIndex].camText;
    nameList.textPosition = camInfo[camIndex].textPosition;
    devCamInfoLock.unlock();
}

void DeviceClient::GetDevTable(DEV_TABLE_INFO_t &table)
{
    deviceTableInfoLock.lock();
    table = tableInfo;
    deviceTableInfoLock.unlock();
}

QString DeviceClient::GetDispDeviceName(void)
{
    return m_dispDevName;
}

void DeviceClient::GetPreVideoLossDuration(quint8 &videoLossDuration)
{
    deviceTableInfoLock.lock();
    videoLossDuration = tableInfo.preVideoLossDuration;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetVideoStandard(VIDEO_STANDARD_e &videoStandard)
{
    deviceTableInfoLock.lock();
    videoStandard = tableInfo.videoStd;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetUserGroup(USRS_GROUP_e &groupType)
{
    deviceTableInfoLock.lock();
    groupType = tableInfo.userGroupType;
    deviceTableInfoLock.unlock();
}

void DeviceClient::SetUserGroup(USRS_GROUP_e groupType)
{
    deviceTableInfoLock.lock();
    tableInfo.userGroupType = groupType;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetAutoCloseRecFailAlertFlag(bool &autoCloseFlag)
{
    deviceTableInfoLock.lock();
    autoCloseFlag =  tableInfo.autoCloseRecFailAlert;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetCameraRights(quint8 cameraIndex, quint8 &camRights)
{
    devCamInfoLock.lock();                                                  /* All Rights */
    camRights = (cameraIndex < MAX_CAMERAS) ? camInfo[cameraIndex].cameraRights : 0x1F;
    devCamInfoLock.unlock();
}

void DeviceClient::SetCameraRights(quint8 cameraIndex, quint8 camRights)
{
    if (cameraIndex < MAX_CAMERAS)
    {
        devCamInfoLock.lock();
        camInfo[cameraIndex].cameraRights = camRights;
        devCamInfoLock.unlock();
    }
}

//*****************************************************************************
//  slotConnectResponse()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for connect request signal.
//          In response of login request it verifies device against the
//          parameters set by user.
//          In response of polling request it determines the device connectivity
//          state.
//          In response of event request it stores the received events to
//          internal buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::slotConnectResponse(REQ_MSG_ID_e requestId, DEVICE_REPLY_TYPE_e statusId, QString payload, QString ipAddr, quint16 tcpPort)
{
    bool    status = false;
    bool    devVerify = false;
    QString usrName, passwrd;
    quint8  portIndex = 0;

    if(connectRequest != NULL)
    {
        portIndex = connectRequest->getPortIndex();
    }

    switch(requestId)
    {
        // if it is login response
        case MSG_REQ_LOG:
        {
            // if login response is success
            if((statusId == CMD_SUCCESS) || ((statusId >= CMD_RESET_PASSWORD) && (statusId <= CMD_USER_ACCOUNT_LOCK)))
            {
                // verify device parameter against those of entered by user
                devVerify = verifyDeviceLogInInit(payload);

                if((statusId == CMD_SUCCESS) || (statusId == CMD_RESET_PASSWORD))
                {
                    if(tcpPort != 0)
                    {
                        devConfigLock.lockForWrite();
                        snprintf(devConfig.ipAddress, MAX_IP_ADDRESS_SIZE, "%s", ipAddr.toLatin1().constData());
                        if(portIndex == 0)
                        {
                            devConfig.port = tcpPort;
                        }
                        else
                        {
                            devConfig.forwardedTcpPort = tcpPort;
                        }

                        if(connectRequest != NULL)
                        {
                            connectRequest->getUsrnamePassword(usrName, passwrd);
                        }
                        snprintf(devConfig.username, MAX_USERNAME_SIZE, "%s", usrName.toLatin1().constData());
                        snprintf(devConfig.password, MAX_PASSWORD_SIZE, "%s", passwrd.toLatin1().constData());
                        devConfigLock.unlock();
                    }

                    if(getHeathStsFrmDev() == true)
                    {
                        if(getCommonCfg(CAMERA_TABLE_INDEX) == true)
                        {
                            // if conflict found in response then set device state to conflict else mark device as connected
                            setDevConnState((devVerify == false) ? CONFLICT : CONNECTED);
                            getCommonCfg(GENERAL_TABLE_INDEX);

                            DevCommParam *param = new DevCommParam();
                            param->msgType = requestId;
                            param->cmdType = MAX_NET_COMMAND;
                            param->windowId = winId;

                            devConfigLock.lockForRead();
                            if((strcmp(devConfig.deviceName, LOCAL_DEVICE_NAME) == 0) &&(deviceRespInfo.diskCheckingCount != 0))
                            {
                                param->deviceStatus = CMD_DISK_CLEANUP_REQUIRED;
                            }
                            else
                            {
                                param->deviceStatus = CMD_DEV_CONNECTED;
                            }

                            if(statusId == CMD_RESET_PASSWORD)
                            {
                                param->deviceStatus = CMD_RESET_PASSWORD;
                            }
                            param->payload = payload;

                            emit sigDeviceResponse(devConfig.deviceName, param);
                            devConfigLock.unlock();

                            // getRemoteDevTable
                            if((strcmp(devConfig.deviceName, LOCAL_DEVICE_NAME) == 0))
                            {
                                getCommonCfg(NETWORK_DEVICE_SETTING_TABLE_INDEX);
                            }

                            status = true;
                        }
                    }
                }
                else
                {
                    // set device state to conflict in case of login not success
                    setDevConnState(CONFLICT);

                    DevCommParam *param = new DevCommParam();
                    param->msgType = MSG_REQ_LOG;
                    param->cmdType = MAX_NET_COMMAND;
                    param->deviceStatus = statusId;
                    param->payload = payload;

                    if(param->deviceStatus < CMD_MAX_DEVICE_REPLY)
                    {
                        devConfigLock.lockForRead();
                        emit sigDeviceResponse(devConfig.deviceName, param);
                        devConfigLock.unlock();
                    }
                }
            }

            if(connectRequest != NULL)
            {
                // set polling flag
                connectRequest->setPollFlag(status);

                //send poll signal
                connectRequest->sendPollSignal();
            }
        }
        break;

        // if it is polling response
        case MSG_REQ_POL:
        {
            DevCommParam *param = new DevCommParam();
            param->msgType = MSG_REQ_LOG;
            param->cmdType = MAX_NET_COMMAND;

            // if resopnse status is success
            if(statusId == CMD_SUCCESS)
            {
                // Change::if verification fail eventhogh internally signal get as success from connect so misbehaving occurs so removed.
                // mark device as connected
                if(strcmp(devConfig.deviceName, LOCAL_DEVICE_NAME) == 0)
                {
                    setDevConnState(CONNECTED);
                }
                param->deviceStatus = statusId;
            }
            else
            {
                DEVICE_STATE_TYPE_e deviceConnectStatus = GetConnectionState();
                if((deviceConnectStatus == LOGGED_OUT) ||(deviceConnectStatus == DISCONNECTED))
                {
                    if((statusId != CMD_IP_BLOCKED) && (statusId != CMD_INVALID_CREDENTIAL) && (statusId != CMD_USER_DISABLED)
                            && (statusId != CMD_USER_BLOCKED) && (statusId != CMD_MULTILOGIN) && (statusId != CMD_MAX_USER_SESSION))
                    {
                        param->deviceStatus = CMD_DEV_LOGGEDOUT;
                    }
                    else
                    {
                        param->deviceStatus = statusId;
                    }
                }
                else if(deviceConnectStatus == DELETED)
                {
                    param->deviceStatus = CMD_DEV_DELETED;
                }
                else if(deviceConnectStatus != CONFLICT)
                {
                    // mark device as disconnected
                    setDevConnState(DISCONNECTED);
                    param->deviceStatus = statusId;
                }
            }

            param->payload = payload;
            if(param->deviceStatus >= CMD_MAX_DEVICE_REPLY)
            {
                DELETE_OBJ(param);
                break;
            }

            devConfigLock.lockForRead();
            emit sigDeviceResponse(devConfig.deviceName, param);
            devConfigLock.unlock();
        }
        break;

        // if it is event request
        case MSG_REQ_EVT:
        {
            // if resopnse status is succes
            if(statusId == CMD_SUCCESS)
            {
                // store events to buffer
                storeLvEvtAndHealthSts(payload);
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    devConfigLock.lockForRead();
    // if response status is failure and auto login is disabled
    if(((requestId == MSG_REQ_LOG) ||(requestId == MSG_REQ_POL)) && (statusId != CMD_SUCCESS)
            && (statusId != CMD_INTERNAL_RESOURCE_LIMIT) && (devConfig.autoLogin == false))
    {
        devConfigLock.unlock();
        for(quint8 index = 0; index < MAX_CMD_SESSION; index++)
        {
            deleteCommandReq(index);
        }

        for(quint8 loop = 0; loop < MAX_GEN_REQ_SESSION; loop++)
		{
            deleteConfigReq(loop);
		}
        deleteConnectReq();

        eventCountAccess.lock();
        liveEventCount -= liveEventList.count();
        eventCountAccess.unlock();

        //Now flush the event queue before thread exit,
        // otherwise these events are served first on this device thread restart.
        QCoreApplication::sendPostedEvents(this, 0);

        QString deviceName;
        devConfigLock.lockForRead();
        deviceName = devConfig.deviceName;
        devConfigLock.unlock();

        emit sigDeleteStreamRequest(deviceName);
        emit sigExitThread();
    }
    else
    {
        devConfigLock.unlock();
    }
}

//*****************************************************************************
//  slotConfigResponse()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for configuration request signal.
//          It deletes the configuration request object and emits a signal
//          to notify response.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::slotConfigResponse(REQ_MSG_ID_e requestId, DEVICE_REPLY_TYPE_e statusId, QString payload, quint8 genReqSesId)
{
    // delete request object
    if(statusId != CMD_INTERNAL_RESOURCE_LIMIT)
    {
        deleteConfigReq(genReqSesId);
    }

    DevCommParam *param = new DevCommParam();
    param->msgType = requestId;
    param->deviceStatus = statusId;
    param->payload = payload;
    param->windowId = winId;

    // emit signal to notify the response
    devConfigLock.lockForRead();
    emit sigDeviceResponse(devConfig.deviceName, param);
    devConfigLock.unlock();
}

//*****************************************************************************
//  slotCommandResponse()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               SET_COMMAND_e commandId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for set command request signal.
//          It deletes the command request object and emits a signal
//          to notify response.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::slotCommandResponse(REQ_MSG_ID_e requestId, SET_COMMAND_e commandId, DEVICE_REPLY_TYPE_e statusId, QString payload, quint8 cmdSesId)
{
    if(!((commandId == PLYBCK_SRCH_MNTH) || (commandId == PLYBCK_SRCH_DAY) || (commandId == TST_CAM)))
    {
        // delete reuqest object
        if((statusId != CMD_INTERNAL_RESOURCE_LIMIT))
        {
            deleteCommandReq(cmdSesId);
        }
    }

    switch(commandId)
    {
        case LOGOUT:
        {
            if (GetConnectionState() != DELETED)
            {
                setDevConnState(DISCONNECTED);
            }

            deleteConnectReq();
            if(loginAfterLogoutNeeded)
            {
                loginAfterLogoutNeeded = false;
                LoginToDevice();
            }
        }
        break;

        case CNG_PWD:
        {
            if(statusId == CMD_SUCCESS)
            {
                QStringList payloadField = tempPayload.split(FSP);
                devConfigLock.lockForWrite();
                snprintf(devConfig.password, MAX_PASSWORD_SIZE, "%s", payloadField.value(0).toLatin1().constData());
                devConfigLock.unlock();
            }
        }
        break;

        case CNG_USER:
        {
            if(statusId == CMD_SUCCESS)
            {
                QStringList payloadField = tempPayload.split(FSP);
                devConfigLock.lockForWrite();
                snprintf(devConfig.username, MAX_USERNAME_SIZE, "%s", payloadField.value(0).toLatin1().constData());
                snprintf(devConfig.password, MAX_PASSWORD_SIZE, "%s", payloadField.value(1).toLatin1().constData());
                devConfigLock.unlock();
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    DevCommParam *param = new DevCommParam();
    param->msgType = requestId;
    param->cmdType = commandId;
    param->deviceStatus = statusId;
    param->payload = payload;
    param->windowId = winId;

    // emit signal to notify response
    devConfigLock.lockForRead();
    emit sigDeviceResponse(devConfig.deviceName, param);
    devConfigLock.unlock();
}

//*****************************************************************************
//  slotPwdRstCmdResponse()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               PWD_RST_CMD_e commandId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API is slot for password reset command request signal.
//          It deletes the command request object and emits a signal
//          to notify response.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void DeviceClient::slotPwdRstCmdResponse(REQ_MSG_ID_e requestId, PWD_RST_CMD_e commandId, DEVICE_REPLY_TYPE_e statusId, QString payload, quint8 cmdSesId)
{
    /* Delete reuqest object */
    if(statusId != CMD_INTERNAL_RESOURCE_LIMIT)
    {
        deletePwdRstReq(cmdSesId);
    }

    DevCommParam *param = new DevCommParam();
    param->msgType = requestId;
    param->pwdRstCmdType = commandId;
    param->deviceStatus = statusId;
    param->payload = payload;
    param->windowId = winId;

    /* emit signal to notify response */
    devConfigLock.lockForRead();
    emit sigDeviceResponse(devConfig.deviceName, param);
    devConfigLock.unlock();
}

void DeviceClient::deleteDevClntInstants(void)
{
    for(quint8 loop = 0; loop < MAX_CMD_SESSION; loop++)
    {
        deleteCommandReq(loop);
    }

    for(quint8 loop = 0; loop < MAX_GEN_REQ_SESSION; loop++)
	{
        deleteConfigReq(loop);
	}

    if(connectRequest != NULL)
    {
        connectRequest->UpdateAutoLogin(false);
        deleteConnectReq();
    }
}

void DeviceClient::StopConnectRequest(void)
{
    if(connectRequest != NULL)
    {
        connectRequest->setRunflag(false);
        connectRequest->UpdateAutoLogin(false);
    }
}

void DeviceClient::GetMaxSensorInputSupport(quint8 &totalInput)
{
    deviceTableInfoLock.lock();
    totalInput =  tableInfo.sensors;
    deviceTableInfoLock.unlock();
}

void DeviceClient::GetMaxAlarmOutputSupport(quint8 &totalOutput)
{
    deviceTableInfoLock.lock();
    totalOutput =  tableInfo.alarms;
    deviceTableInfoLock.unlock();
}
