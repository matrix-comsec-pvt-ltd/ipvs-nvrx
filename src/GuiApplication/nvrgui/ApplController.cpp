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
//   Project      : NVR ( Network Video Recorder) GUI
//   Owner        : Shruti Sahni
//   File         : applController.cpp
//   Description  :
//                This module is the interface between GUI(front end) and the
//                backend. This class is registered in the QML system with the
//                ApplControl 1.0. Any click or other user interaction with GUI
//                invokes this class function which furthur initializes or calls
//                function to all other modules such as Device Communication,
//                Device Activity, Layout change etc.
//////////////////////////////////////////////////////////////////////////////

//******** Place all include files here ********
#include <unistd.h>
#include <QLocalSocket>

#include "ApplController.h"
#include <Layout/Layout.h>
#include "../DecoderLib/include/DecDispLib.h"

extern "C" {
#include "DebugLog.h"
}

//****** Defines and Data Types ******
#define DEFAULT_VOL_LEVEL   50

ApplController *ApplController::applController = NULL;

ApplController *ApplController::getInstance()
{
    if (IS_VALID_OBJ(applController))
    {
        return applController;
    }

    applController = new ApplController();
    return applController;
}

void ApplController::deleteAppCntrollrInstance()
{
    DELETE_OBJ(applController);
}

ApplController::ApplController(QObject* parent) : QObject(parent), m_audioLevel(0), m_currentMuteState(OFF_STATE)
{
    //Registers REQ_MSG_ID_e and returns the internal ID used by QMetaType
    qRegisterMetaType<REQ_MSG_ID_e>("REQ_MSG_ID_e");

    //Registers DEVICE_REPLY_TYPE_e and returns the internal ID used by QMetaType
    qRegisterMetaType<DEVICE_REPLY_TYPE_e>("DEVICE_REPLY_TYPE_e");

    //Registers SET_COMMAND_e and returns the internal ID used by QMetaType
    qRegisterMetaType<SET_COMMAND_e>("SET_COMMAND_e");

    //Registers STREAM_REQUEST_TYPE_e and returns the internal ID used by QMetaType
    qRegisterMetaType<STREAM_REQUEST_TYPE_e>("STREAM_REQUEST_TYPE_e");
    qRegisterMetaType<CONNECTION_TYPE_e>("CONNECTION_TYPE_e");
    qRegisterMetaType<LOG_EVENT_TYPE_e>("LOG_EVENT_TYPE_e");
    qRegisterMetaType<LOG_EVENT_SUBTYPE_e>("LOG_EVENT_SUBTYPE_e");
    qRegisterMetaType<LOG_EVENT_STATE_e>("LOG_EVENT_STATE_e");
    qRegisterMetaType<STREAM_COMMAND_TYPE_e>("STREAM_COMMAND_TYPE_e");
    qRegisterMetaType<SERVER_SESSION_INFO_t>("SERVER_SESSION_INFO_t");
    qRegisterMetaType<BUFFER_THRESHOLD_e>("BUFFER_THRESHOLD_e");

    connect(&audioCfg,
            SIGNAL(sigAudioCfgChanged(const AUDIO_CONFIG_t*, const AUDIO_CONFIG_t*)),
            this,
            SLOT(slotAudioCfgChanged(const AUDIO_CONFIG_t*, const AUDIO_CONFIG_t*)),
            Qt::QueuedConnection);
    connect(&deviceCfg,
            SIGNAL(sigDeviceCfgChanged(quint8, const DEVICE_CONFIG_t*, const DEVICE_CONFIG_t*)),
            this,
            SLOT(slotDeviceCfgChanged(quint8, const DEVICE_CONFIG_t*, const DEVICE_CONFIG_t*)),
            Qt::QueuedConnection);

    for (quint8 index = 0; index < MAX_DEVICES; index++)
    {
        deviceClient[index] = NULL;
    }

    displayCfg.InitConfig();
    audioCfg.InitConfig();
    deviceList.clear();
    deviceList.reserve(MAX_DEVICES);
    deviceList.append(LOCAL_DEVICE_NAME);

    for (quint8 index = 1; index <= MAX_REMOTE_DEVICES; index++)
    {
        deviceList.append("");
    }

    deviceCfg.initLocalDevice();
    deviceCfg.InitOtherLoginConfig();
    deviceCfg.InitWizardSettingConfig();

    CommandRequest::initializeCmdSes();
    PasswordResetRequest::initializeCmdSes();

    lastStreamRequestIndex = MAX_STREAM_SESSION;
    for (quint8 index = 0; index < MAX_STREAM_SESSION; index++)
    {
        isStreamRequestAllocated[index] = false;
        streamRequest[index] = NULL;
    }

    HdmiRegCallBack(hdmiCallBack);
    m_getUserRightsResponse = false;
    m_screenWidth = 1920;
    m_screenHeight = 1080;
    m_screenXPos = 0;
    m_screenYPos = 0;
    currentDisplayResolution = DISPLAY_RESOLUTION_1080P;
    memset(&deviceRespInfo, 0, sizeof(NVR_DEVICE_INFO_t));
}

ApplController::~ApplController()
{
    for (quint8 index = 0; index < MAX_DEVICES; index++)
    {
        if (deviceClient[index] != NULL)
        {
            deviceClient[index]->StopConnectRequest();
        }
    }

    for (quint8 index = 0; index < MAX_DEVICES; index++)
    {
        if (deviceClient[index] != NULL)
        {
            deleteDeviceClient(index);
        }
    }

    for (quint8 streamId = 0; streamId < MAX_STREAM_SESSION; streamId++)
    {
        deleteStreamRequest(streamId);
    }
}

bool ApplController::processActivity(ACTIVITY_TYPE_e activityType, void *param)
{
    QList<QVariant> paramList;
    return processActivity(LOCAL_DEVICE_NAME, activityType, paramList, param);
}

bool ApplController::processActivity(ACTIVITY_TYPE_e activityType, QList<QVariant> &paramList, void *param)
{
    return processActivity(LOCAL_DEVICE_NAME, activityType, paramList, param);
}

bool ApplController::processActivity(QString deviceName, ACTIVITY_TYPE_e activityType, void *param)
{
    QList<QVariant> paramList;
    return processActivity(deviceName, activityType, paramList, param);
}

bool ApplController::processActivity(QString deviceName, ACTIVITY_TYPE_e activityType, QList<QVariant> &paramList, void* param)
{
    bool    status = true;
    quint8  deviceIndex;

    //match the activity type
    switch (activityType)
    {
        case AUDIO_SETTING:
        {
            switch (paramList.at(SUB_ACTIVITY_TYPE).toInt())
            {
                case WRITE_AUDIO_ACTIVITY:
                    status = audioCfg.WriteConfig((AUDIO_CONFIG_t*)param);
                    break;

                case READ_AUDIO_ACTIVITY:
                    status = audioCfg.ReadConfig((AUDIO_CONFIG_t*)param);
                    break;

                default:
                    status = false;
                    break;
            }
        }
        break;

        case DEVICE_COMM:
        {
            status = getDeviceIndex(deviceName, deviceIndex);
            if (status == false)
            {
                /* Device not found in list. So give response to the UI. */
                emit sigDevCommGui(deviceName, (DevCommParam*)param);
                break;
            }

            /* Check device connection state */
            status = IsValidDeviceConnState(deviceIndex);
            if (status == false)
            {
                emit sigDevCommGui(deviceName, (DevCommParam*)param);
                break;
            }

            /* Send the request to the device client with the data */
            deviceClient[deviceIndex]->processDeviceRequest((DevCommParam*)param);
        }
        break;

        case DISPLAY_SETTING:
        {
            switch (paramList.at(SUB_ACTIVITY_TYPE).toInt())
            {
                case READ_DISP_ACTIVITY:
                {
                    status = displayCfg.ReadConfig((DISPLAY_TYPE_e)paramList.at(DISPLAY_ID).toInt(), (DISPLAY_CONFIG_t*)param,
                                                   (STYLE_TYPE_e)paramList.at(DISPLAY_STYLE_ID).toInt());
                }
                break;

                case WRITE_DISP_ACTIVITY:
                {
                    status = displayCfg.WriteConfig((DISPLAY_TYPE_e)paramList.at(DISPLAY_ID).toInt(), (DISPLAY_CONFIG_t*)param,
                                                    (STYLE_TYPE_e)paramList.at(DISPLAY_STYLE_ID).toInt());
                    if (status == false)
                    {
                        break;
                    }

                    if (paramList.at(DISPLAY_DEFAULT_STATUS).toBool() == true)
                    {
                        status = displayCfg.WriteDfltStyle((DISPLAY_TYPE_e)paramList.at(DISPLAY_ID).toInt(),
                                                           (STYLE_TYPE_e)paramList.at(DISPLAY_STYLE_ID).toInt());
                        break;
                    }

                    STYLE_TYPE_e styleIndx;
                    status = displayCfg.ReadDfltStyle((DISPLAY_TYPE_e)paramList.at(DISPLAY_ID).toInt(), &styleIndx);
                    if (status == false)
                    {
                        break;
                    }

                    if (styleIndx != (STYLE_TYPE_e)paramList.at(DISPLAY_STYLE_ID).toInt())
                    {
                        break;
                    }

                    // Write Default Style STYLE_TYPE_1
                    styleIndx = STYLE_TYPE_1;
                    status = displayCfg.WriteDfltStyle((DISPLAY_TYPE_e)paramList.at(DISPLAY_ID).toInt(), styleIndx);
                }
                break;

                case READ_DFLTSTYLE_ACTIVITY:
                {
                    status = displayCfg.ReadDfltStyle((DISPLAY_TYPE_e)paramList.at(DISPLAY_ID).toInt(), (STYLE_TYPE_e*)param);
                }
                break;

                default:
                {
                    status = false;
                }
                break;
            }
        }
        break;

        case CONNECT_DEVICE:
        {
            status = getDeviceIndex(deviceName, deviceIndex);
            if (status == false)
            {
                break;
            }

            if (deviceClient[deviceIndex] == NULL)
            {
                status = false;
                break;
            }

            deviceThread[deviceIndex].start();
            deviceClient[deviceIndex]->LoginToDevice();
        }
        break;

        case DISCONNECT_DEVICE:
        {
            status = getDeviceIndex(deviceName, deviceIndex);
            if (status == false)
            {
                break;
            }

            if (deviceClient[deviceIndex] == NULL)
            {
                status = false;
                break;
            }

            bool isLogOutNeedToSend = true;
            DEVICE_STATE_TYPE_e devState = LOGGED_OUT;
            if (paramList.length() > 0)
            {
                DEVICE_REPLY_TYPE_e devResp = (DEVICE_REPLY_TYPE_e)paramList.at(0).toInt();
                if (((devResp >= CMD_RESET_PASSWORD) && (devResp <= CMD_USER_ACCOUNT_LOCK)) || (devResp == CMD_DEV_CONFLICT))
                {
                    isLogOutNeedToSend = false;
                    devState = DISCONNECTED;
                }
            }

            DPRINT(DEVICE_CLIENT, "device disconnect: [device=%s], [index=%d], [state=%d]", deviceName.toUtf8().constData(), deviceIndex, devState);
            if (isLogOutNeedToSend)
            {
                deviceClient[deviceIndex]->LogoutFromDevice();
            }

            deviceClient[deviceIndex]->setDevConnState(devState);
        }
        break;

        case OTHER_LOGIN_ACTIVITY:
        {
            switch (paramList.at(SUB_ACTIVITY_TYPE).toInt())
            {
                case READ_OTHER_LOGIN_PARAM:
                    status = deviceCfg.ReadOtherLoginConfig((OTHER_LOGIN_CONFIG_t*)param);
                    break;

                case WRITE_OTHER_LOGIN_PARAM:
                    status = deviceCfg.WriteOtherLoginConfig((OTHER_LOGIN_CONFIG_t*)param);
                    break;

                case WRITE_WIZARD_PARAM:
                    status = deviceCfg.WriteWizOtherLoginConfig((WIZ_OPEN_CONFIG_t*)param);
                    break;

                case READ_WIZARD_PARAM:
                    status = deviceCfg.ReadWizOtherLoginConfig((WIZ_OPEN_CONFIG_t*)param);
                    break;

                default:
                    status = false;
                    break;
            }
        }
        break;

        case LOCAL_LOGIN_ACTIVITY:
        {
            QString username,password;
            status = deviceCfg.ReadLocalLoginConfig(username, password);
            if (status == false)
            {
                break;
            }

            paramList.insert(LOCAL_LOGIN_USERNAME, username);
            paramList.insert(LOCAL_LOGIN_PASSWORD, password);
        }
        break;

        case TST_CAM_IMG_DELETE:
        {
            if (unlink(TST_CAM_IMAGE_FILE_PATH) == 0)
            {
                status = true;
            }
        }
        break;

        default:
        {
            status = false;
        }
        break;
    }

    return status;
}

/* Process Stream Activity for various Stream Reuqests. such as START_LIVE_STREAM, STOP_LIVEA_STREAM.. etc */
quint8 ApplController::processStreamActivity(STREAM_COMMAND_TYPE_e streamCommandType, SERVER_SESSION_INFO_t serverInfo,
                                             StreamRequestParam *streamRequestParam, quint8 *streamIndexOut)
{
    quint8  streamId = MAX_STREAM_SESSION;
    quint8  deviceIndex = MAX_DEVICES;
    quint8  pendingRequestCount = 0;

    if (streamRequestParam == NULL)
    {
        return pendingRequestCount;
    }

    if (false == getDeviceIndex(streamRequestParam->deviceName, deviceIndex))
    {
        EPRINT(APPL_CONTROLLER, "fail to get device index: [streamId=%d], [deviceName=%s]",
               streamRequestParam->streamId, streamRequestParam->deviceName.toUtf8().constData());
        deleteStreamRequest(streamRequestParam->streamId);
        return pendingRequestCount;
    }

    if (false == IsValidDeviceConnState(deviceIndex))
    {
        EPRINT(APPL_CONTROLLER, "device connection failed: [streamId=%d], [deviceName=%s], [deviceIndex=%d]",
               streamRequestParam->streamId, streamRequestParam->deviceName.toUtf8().constData(), deviceIndex);
        deleteStreamRequest(streamRequestParam->streamId);
        return pendingRequestCount;
    }

    switch (streamCommandType)
    {
        case START_STREAM_COMMAND:
        {
            if (findFreeStreamSession(streamId) == false)
            {
                EPRINT(APPL_CONTROLLER, "free session not found: [cameraId=%d]", streamRequestParam->channelId);
                break;
            }

            /* create stream request & thread object, move to thread and start thread */
            createStreamRequest(streamId);
            if (streamIndexOut != NULL)
            {
                *streamIndexOut = streamId;
            }

            streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
            pendingRequestCount++;
        }
        break;

        case STOP_STREAM_COMMAND:
        case CHANGE_STREAM_TYPE_COMMAND:
        case INCLUDE_AUDIO_COMMAND:
        case EXCLUDE_AUDIO_COMMAND:
        case INCL_EXCL_AUDIO_IN_COMMAND:
        {
            streamId = streamRequestParam->streamId;
            if (streamId < MAX_STREAM_SESSION)
            {
                if (streamRequest[streamId] != NULL)
                {
                    streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
                    pendingRequestCount++;
                    break;
                }
            }

            /* Added to give response if stream was not started */
            if ((streamCommandType == INCLUDE_AUDIO_COMMAND) || (streamCommandType == EXCLUDE_AUDIO_COMMAND) || (streamCommandType == INCL_EXCL_AUDIO_IN_COMMAND))
            {
                pendingRequestCount++;
                EPRINT(APPL_CONTROLLER, "stream not started: [streamId=%d], [streamCommandType=%d]", streamId, streamCommandType);
                emit sigStreamRequestResponse(streamCommandType, streamRequestParam, CMD_MAX_DEVICE_REPLY);
            }
        }
        break;

        case GET_PLAYBACK_ID_COMMAND:
        {
            if (findFreeStreamSession(streamId) == false)
            {
                break;
            }

            /* create stream request and thread */
            createStreamRequest(streamId);
            if (streamIndexOut != NULL)
            {
                *streamIndexOut = streamId;
            }

            streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
            pendingRequestCount++;
        }
        break;

        case PLAY_PLABACK_STREAM_COMMAND:
        case STEP_PLABACK_STREAM_COMMAND:
        case STOP_PLABACK_STREAM_COMMAND:
        case AUDIO_PLABACK_STREAM_COMMAND:
        case CLEAR_PLAYBACK_ID_COMMAND:
        {
            streamId = streamRequestParam->streamId;
            if (streamId >= MAX_STREAM_SESSION)
            {
                break;
            }

            if (streamRequest[streamId] != NULL)
            {
                if (streamRequest[streamId]->getClearFlag() == false)
                {
                    streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
                    pendingRequestCount++;
                }
                else if ((streamRequest[streamId]->getClearFlag() == true) && (streamCommandType != STOP_PLABACK_STREAM_COMMAND))
                {
                    streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
                    pendingRequestCount++;
                }
            }
        }
        break;

        case PLAY_SYNCPLABACK_STREAM_COMMAND:
        {
            if (findFreeStreamSession(streamId) == false)
            {
                break;
            }

            /* create stream request and thread */
            createStreamRequest(streamId);

            streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
            pendingRequestCount++;
        }
        break;

        case SYNCPLAY_SYNCPLABACK_STREAM_COMMAND:
        case SET_SPEED_SYNCPLABACK_STREAM_COMMAND:
        case STEPFORWARD_SYNCPLABACK_STREAM_COMMAND:
        case STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND:
        case PAUSE_SYNCPLABACK_STREAM_COMMAND:
        case STOP_SYNCPLABACK_STREAM_COMMAND:
        case CLEAR_SYNCPLABACK_STREAM_COMMAND:
        case AUDIO_SYNCPLABACK_STREAM_COMMAND:
        {
            streamId = streamRequestParam->streamId;
            if (streamId >= MAX_STREAM_SESSION)
            {
                break;
            }

            if (streamRequest[streamId] != NULL)
            {
                streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
                pendingRequestCount++;
            }
        }
        break;

        case START_INSTANTPLAYBACK_COMMAND:
        {
            if (findFreeStreamSession(streamId) == false)
            {
                break;
            }

            /* create stream request and thread */
            createStreamRequest(streamId);

            streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
            pendingRequestCount++;
        }
        break;

        case STOP_INSTANTPLAYBACK_COMMAND:
        case SEEK_INSTANTPLAYBACK_COMMAND:
        case PAUSE_INSTANTPLAYBACK_COMMAND:
        case AUDIO_INSTANTPLAYBACK_COMMAND:
        {
            streamId = streamRequestParam->streamId;
            if (streamId >= MAX_STREAM_SESSION)
            {
                break;
            }

            if (streamRequest[streamId] != NULL)
            {
                streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
                pendingRequestCount++;
            }
        }
        break;

        case STRT_CLNT_AUDIO_COMMAND:
        {
            if (findFreeStreamSession(streamId) == false)
            {
                break;
            }

            /* create stream request and thread */
            createStreamRequest(streamId);

            streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
            pendingRequestCount++;
        }
        break;

        case STOP_CLNT_AUDIO_COMMAND:
        {
            streamId = streamRequestParam->streamId;
            if (streamId >= MAX_STREAM_SESSION)
            {
                break;
            }

            if (streamRequest[streamId] != NULL)
            {
                streamRequest[streamId]->processRequest(streamCommandType, serverInfo, streamRequestParam);
                pendingRequestCount++;
            }
        }
        break;

        default:
            break;
    }

    return pendingRequestCount;
}

bool ApplController::GetAutoCloseRecFailAlertFlag(QString deviceName, bool &autoCloseFlag)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetAutoCloseRecFailAlertFlag(autoCloseFlag);
    return true;
}

quint8 ApplController::processStreamActivity(STREAM_COMMAND_TYPE_e streamCommandType, SERVER_SESSION_INFO_t serverInfo,
                                             StreamRequestParam *streamRequestParam, SERVER_SESSION_INFO_t nextServerInfo,
                                             StreamRequestParam *nextStreamRequestParam)
{
    quint8 streamId = MAX_STREAM_SESSION;
    quint8 deviceIndex = MAX_DEVICES;
    bool firstDeviceConnected, secondDeviceConnected;

    Q_UNUSED(serverInfo);
    Q_UNUSED(nextServerInfo);
    if ((streamCommandType != REPLACE_STREAM_COMMAND) || (streamRequestParam == NULL) || (nextStreamRequestParam == NULL))
    {
        return NO_STOP_NO_START;
    }

    firstDeviceConnected = IsValidDeviceConnState(streamRequestParam->deviceName, deviceIndex);
    if (false == firstDeviceConnected)
    {
        deleteStreamRequest(streamRequestParam->streamId);
    }

    secondDeviceConnected = IsValidDeviceConnState(nextStreamRequestParam->deviceName, deviceIndex);
    if (false == secondDeviceConnected)
    {
        deleteStreamRequest(nextStreamRequestParam->streamId);
    }

    if ((firstDeviceConnected == true) && (secondDeviceConnected == true))
    {
        streamId = streamRequestParam->streamId;
        if ((streamId >= MAX_STREAM_SESSION) || (streamRequest[streamId] == NULL))
        {
            return ONLY_SECOND_START;
        }

        if (true == streamRequest[streamId]->getDeletionProcessFlag())
        {
            return ONLY_SECOND_START;
        }

        return SECOND_REPLACE_FIRST;
    }

    if (firstDeviceConnected == true)
    {
        streamId = streamRequestParam->streamId;
        if ((streamId >= MAX_STREAM_SESSION) || (streamRequest[streamId] == NULL))
        {
            return NO_STOP_NO_START;
        }

        if (true == streamRequest[streamId]->getDeletionProcessFlag())
        {
            return NO_STOP_NO_START;
        }

        return SECOND_REPLACE_FIRST;
    }

    if (secondDeviceConnected == true)
    {
        return SECOND_REPLACE_FIRST;
    }

    return NO_STOP_NO_START;
}

bool ApplController::getDeviceIndex(QString devName, quint8 &deviceIndex)
{
    if (devName == "")
    {
        return false;
    }

    if (false == deviceList.contains(devName))
    {
        return false;
    }

    deviceIndex = deviceList.indexOf(devName);
    return true;
}

bool ApplController::IsValidDeviceConnState(QString devName, quint8 &deviceIndex)
{
    if (false == getDeviceIndex(devName, deviceIndex))
    {
        return false;
    }

    return IsValidDeviceConnState(deviceIndex);
}

bool ApplController::IsValidDeviceConnState(quint8 deviceIndex)
{
    if ((deviceIndex >= MAX_DEVICES) || (deviceClient[deviceIndex] == NULL))
    {
        return false;
    }

    DEVICE_STATE_TYPE_e deviceConnState = deviceClient[deviceIndex]->GetConnectionState();
    if ((deviceConnState == CONNECTED) || (deviceConnState == CONFLICT))
    {
        return true;
    }

    return false;
}

bool ApplController::GetTotalCamera(QString devName, quint8 &cameraCount)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        cameraCount = 0;
        return false;
    }

    deviceClient[deviceIndex]->GetMaxCamera(cameraCount);
    return true;
}

void ApplController::GetDevModel(QString devName, quint8 &devModel)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        devModel = 0;
        return;
    }

    deviceClient[deviceIndex]->GetDeviceModel(devModel);
}

QString ApplController::GetCameraNameOfDevice(QString deviceName, quint8 cameraId)
{
    quint8  deviceIndex;
    QString cameraName;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return QString("Camera - %1").arg(cameraId + 1);
    }

    deviceClient[deviceIndex]->GetCameraName(cameraId, cameraName);
    return cameraName;
}

bool ApplController::GetCameraInfoOfDevice(QString deviceName, quint8 cameraId, DEV_CAM_INFO_t &cameraInfo)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        cameraInfo.camName = QString("Camera-%1").arg(cameraId);
        cameraInfo.nameOsdPosition = TOP_LEFT;
        cameraInfo.statusOsdPosition = BOTTOM_RIGHT;
        return false;
    }

    deviceClient[deviceIndex]->GetDevCamList(cameraId, cameraInfo);
    return true;
}

CAMERA_TYPE_e ApplController::GetCameraType(QString deviceName, quint8 cameraId)
{
    quint8          deviceIndex;
    CAMERA_TYPE_e   camType;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return MAX_CAMERA_TYPE;
    }

    deviceClient[deviceIndex]->GetCameraType(cameraId, camType);
    return camType;
}

void ApplController::GetCameraRights(QString deviceName, quint8 camIndex, CAM_LIST_TYPE_e listType, bool &camRights)
{
    quint8 deviceIndex, cameraRights = 0;
    USRS_GROUP_e userType = ADMIN;

    if (false == GetUserGroupType(deviceName, userType))
    {
        return;
    }

    if (false == getDeviceIndex(deviceName, deviceIndex))
    {
        return;
    }

    if (deviceClient[deviceIndex] == NULL)
    {
        return;
    }

    if (deviceClient[deviceIndex]->GetConnectionState() != CONNECTED)
    {
        return;
    }

    deviceClient[deviceIndex]->GetCameraRights(camIndex, cameraRights);
    switch (listType)
    {
        case LIVE_CAM_LIST_TYPE:
        {
            if (((cameraRights >> MONITORING_BITPOSITION) & 0x01) == 0)
            {
                camRights = false;
            }
        }
        break;

        case PLAYBACK_CAM_LIST_TYPE:
        {
            if (((cameraRights >> PLAYBACK_BITPOSITION) & 0x01) == 0)
            {
                camRights = false;
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

void ApplController::setUserRightsResponseNotify(bool getUserRightsFlag)
{
    m_getUserRightsResponse = getUserRightsFlag;
}

bool ApplController::getUserRightsResponseNotify()
{
    return m_getUserRightsResponse;
}

bool ApplController::SetCameraRights(QString deviceName, quint8 camIndex, quint8 cameraRights)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->SetCameraRights(camIndex, cameraRights);
    return true;
}

void ApplController::hdmiLoadInfo(bool isHdmiInfoShow)
{
    emit sigHdmiInfoPage(isHdmiInfoShow);
}

bool ApplController::GetDeviceInfo(QString deviceName, DEV_TABLE_INFO_t &deviceInfo)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetDevTable(deviceInfo);
    return true;
}

bool ApplController::GetVideoStandard(QString deviceName, VIDEO_STANDARD_e &videoStandard)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        videoStandard = MAX_VIDEO_STANDARD;
        return false;
    }

    deviceClient[deviceIndex]->GetVideoStandard(videoStandard);
    return true;
}

bool ApplController::GetUserGroupType(QString deviceName, USRS_GROUP_e &userType)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetUserGroup(userType);
    return true;
}

bool ApplController::SetUserGroupType(QString deviceName, USRS_GROUP_e userType)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->SetUserGroup(userType);
    return true;
}

bool ApplController::GetMaxSensorInputSupport(QString deviceName, quint8 &totalInput)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetMaxSensorInputSupport(totalInput);
    return true;
}

bool ApplController::GetMaxAlarmOutputSupport(QString deviceName, quint8 &totalOutput)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetMaxAlarmOutputSupport(totalOutput);
    return true;
}

void ApplController::GetEnableDevList(quint8 &deviceCount, QStringList &list)
{
    list.clear();
    list.append(LOCAL_DEVICE_NAME);

    devInfoLock.lockForRead();
    for (quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        if ((strlen(rdevConfig[index].deviceName) != 0) && (rdevConfig[index].enable == true))
        {
            list.append(rdevConfig[index].deviceName);
        }
    }
    devInfoLock.unlock();
    deviceCount = list.count();
}

void ApplController::GetDevNameDropdownMapList(QMap<quint8, QString> &deviceMapList)
{
    quint8 index = 0, deviceIndex = 0;

    deviceMapList.clear();
    deviceMapList.insert(deviceIndex, GetLocalDeviceNameInfo());
    deviceIndex++;

    for (index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        if ((rdevConfig[index].deviceName[0] != '\0') && (rdevConfig[index].enable == true))
        {
            deviceMapList.insert(deviceIndex, rdevConfig[index].deviceName);
            deviceIndex++;
        }
    }
}

QString ApplController::GetRealDeviceName(QString deviceName)
{
    if (deviceName == GetLocalDeviceNameInfo())
    {
        return LOCAL_DEVICE_NAME;
    }

    return deviceName;
}

QString ApplController::GetDispDeviceName(QString deviceName)
{
    if (deviceName == LOCAL_DEVICE_NAME)
    {
        return GetLocalDeviceNameInfo();
    }

    return deviceName;
}

QString ApplController::GetLocalDeviceNameInfo(void)
{
    if (false == IsValidDeviceConnState(0))
    {
        return LOCAL_DEVICE_NAME;
    }

    return deviceClient[0]->GetDispDeviceName();
}

void ApplController::GetConfiguredDeviceList(quint8 &deviceCount, QStringList &list)
{
    list.clear();
    devInfoLock.lockForRead();
    for (quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        list.append(rdevConfig[index].deviceName);
    }
    devInfoLock.unlock();
    deviceCount = list.count();
}

DEVICE_STATE_TYPE_e ApplController::GetDeviceConnectionState(QString devName)
{
    quint8              devIndex;
    DEVICE_STATE_TYPE_e devState = DISCONNECTED;

    if ((getDeviceIndex(devName, devIndex) == true) && (deviceClient[devIndex] != NULL))
    {
        devState = deviceClient[devIndex]->GetConnectionState();
    }

    return devState;
}

quint16 ApplController::GetTotalCameraOfEnableDevices()
{
    quint8 tempCameraCount = 0;
    quint16 cameraCount = 0;

    // local Device Camera Count
    GetTotalCamera(LOCAL_DEVICE_NAME, tempCameraCount);
    cameraCount += tempCameraCount;

    // remote Device Count
    devInfoLock.lockForRead();
    for (quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        if ((strlen(rdevConfig[index].deviceName) != 0) && (rdevConfig[index].enable == true))
        {
            GetTotalCamera(rdevConfig[index].deviceName,tempCameraCount);
            cameraCount +=  tempCameraCount;
        }
    }
    devInfoLock.unlock();
    return cameraCount;
}

bool ApplController::UpdateHlthStatusAll(QString devName, QString param)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->updateDeviceHlthStatus(param);
    return true;
}

bool ApplController::GetHlthStatusAll(QString devName, quint8 *list)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetAllHlthStatusParam(list);
    return true;
}

bool ApplController::GetHlthStatusSingleParam(QString deviceName, quint8 *list, quint8 paramIndex)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetSinglePrmHlthStatus(list, paramIndex);
    return true;
}

bool ApplController::GetHealtStatusSingleParamSingleCamera(QString deviceName, quint8 &paramValue, quint8 paramIndex, quint8 cameraIndex)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetSigleParamSingleCameraStatus(paramValue, paramIndex, cameraIndex);
    return true;
}

bool ApplController::GetDeviceEventList(QString deviceName, QStringList &eventList)
{
    quint8 deviceIndex;

    if ((getDeviceIndex(deviceName, deviceIndex) == true) && (deviceClient[deviceIndex] != NULL))
    {
        return deviceClient[deviceIndex]->GetDeviceEvents(eventList);
    }

    return false;
}

bool ApplController::DeleteDeviceEventList(QString deviceName, QStringList eventList)
{
    quint8 deviceIndex;

    if ((getDeviceIndex(deviceName, deviceIndex) == true) && (deviceClient[deviceIndex] != NULL))
    {
        return deviceClient[deviceIndex]->DeleteDeviceEvents(eventList);
    }

    return false;
}

bool ApplController::getPasswordFrmDev(QString devName, QString &password)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetPassword(password);
    return true;
}


bool ApplController::getUsernameFrmDev(QString devName, QString &username)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetUserName(username);
    return true;
}

bool ApplController::getLiveStreamTypeFrmDev(QString devName, quint8 &liveStreamType)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        return false;
    }

    if (devName != LOCAL_DEVICE_NAME)
    {
        deviceClient[deviceIndex]->GetLiveStreamType(liveStreamType);
    }
    else
    {
        /* If Optimize BW flag in enable return MAX_LIVE_STREAM_TYPE */
        liveStreamType = (readBandwidthOptFlag() == true) ? MAX_LIVE_STREAM_TYPE : LIVE_STREAM_TYPE_MAIN;
    }

    return true;
}

bool ApplController::GetAllCameraNamesOfDevice(QString deviceName, QStringList &camNameList, CAM_LIST_TYPE_e listType, quint8 *keys)
{
    bool camRights = true;

    /* get cameraNameList of input device */
    camNameList.clear();
    quint8 cameraCount = 0;
    quint8 listCount=0;

    if (false == GetTotalCamera(deviceName, cameraCount))
    {
        return false;
    }

    for (quint8 index = 0; index < cameraCount; index++)
    {
        camRights = true;
        if (listType < MAX_CAM_LIST_TYPE)
        {
            GetCameraRights(deviceName, index, listType, camRights);
        }

        if (camRights == true)
        {
            if (keys != NULL)
            {
                keys[listCount++] = index + 1;
            }

            camNameList.append(GetCameraNameOfDevice(deviceName, index));
        }
    }

    return true;
}

bool ApplController::GetDeviceAutoLoginstate(QString devName)
{
    bool autoLogstate = false;
    quint8 deviceIndex = 0;

    if ((getDeviceIndex(devName, deviceIndex)) && (deviceClient[deviceIndex] != NULL))
    {
        deviceClient[deviceIndex]->GetAutoLogState(autoLogstate);
    }

    return autoLogstate;
}

bool ApplController::GetDevicePreferNativeDeviceState(QString devName)
{
    bool nativeDeviceCredential = false;
    quint8 deviceIndex = 0;

    if ((getDeviceIndex(devName, deviceIndex)) && (deviceClient[deviceIndex] != NULL))
    {
        deviceClient[deviceIndex]->GetPreferNativeLoginState(nativeDeviceCredential);
    }

    return nativeDeviceCredential;
}

bool ApplController::GetServerSessionInfo(QString deviceName, SERVER_SESSION_INFO_t &serverSessionInfo)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetServerSessionInfo(serverSessionInfo);
    return true;
}

bool ApplController::GetPreVideoLossDuration(QString deviceName, quint8 &videoLossDuration)
{
    quint8 deviceIndex;

    if (false == IsValidDeviceConnState(deviceName, deviceIndex))
    {
        return false;
    }

    deviceClient[deviceIndex]->GetPreVideoLossDuration(videoLossDuration);
    return true;
}

quint64 ApplController::GetSyncPlaybackMonthRecord()
{
    return DeviceClient::GetMonthRec();
}

quint8 ApplController::GetCamRecType(quint8 iCameraId)
{
    return (DeviceClient::GetCamRecType(iCameraId));
}

bool ApplController::GetCamRecData(quint8 cameraIndex, QList<quint16> &recordList)
{
    return (DeviceClient::GetCamRecData(cameraIndex, recordList));
}

bool ApplController::IsCamRecAvailable(quint8 iCameraId, quint16 iStartTime, quint16 iEndTime)
{
    return (DeviceClient::IsCamRecAvailable(iCameraId, iStartTime, iEndTime));
}

void ApplController::GetMotionInfo(quint32 *motionInfo)
{
    return DeviceClient::GetMotionInfo(motionInfo);
}

DISPLAY_RESOLUTION_e ApplController::readResolution(DISPLAY_TYPE_e disId)
{
    DISPLAY_RESOLUTION_e resolution = DISPLAY_RESOLUTION_720P;
    displayCfg.ReadDfltResolution(disId, &resolution);
    return resolution;
}

void ApplController::writeResolution(DISPLAY_TYPE_e disId, DISPLAY_RESOLUTION_e resolutionId)
{
    displayCfg.WriteDfltResolution(disId, &resolutionId);
}

DISPLAY_RESOLUTION_e ApplController::getCurrentDisplayResolution(void)
{
    return currentDisplayResolution;
}

void ApplController::setCurrentDisplayResolution(DISPLAY_RESOLUTION_e resolutionId)
{
    currentDisplayResolution = resolutionId;
}

void ApplController::setDisplayParameters(PHYSICAL_DISPLAY_TYPE_e displayType, PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e paramIndex, quint32 paramValue)
{
    SetDisplayParameter(displayType, (DISPLAY_SCREEN_PARAM_TYPE_e)paramIndex, paramValue);
}

bool ApplController::readDisplayParameters(PHYSICAL_DISPLAY_TYPE_e displayType, DISPLAY_PARAM_t &displayParam)
{
    return displayCfg.ReadDispParam(displayType, displayParam);
}

bool ApplController::writeDisplayParameters(PHYSICAL_DISPLAY_TYPE_e displayType, DISPLAY_PARAM_t &displayParam)
{
    return displayCfg.WriteDispParam(displayType, displayParam);
}

bool ApplController::readTVApperanceParameters(quint32 &tvAdjustParam)
{
    return displayCfg.ReadTvAdjustConfig(tvAdjustParam);
}

bool ApplController::writeTVApperanceParameters(quint32 &tvAdjustParam)
{
    return displayCfg.WriteTvAdjustConfig(tvAdjustParam);
}

bool ApplController::readBandwidthOptFlag()
{
    return displayCfg.ReadBandwidthOptFlag();
}

bool ApplController::writeBandwidthOptFlag(bool flag)
{
    return displayCfg.WriteBandwidthOptFlag(flag);
}

LIVE_VIEW_TYPE_e ApplController::GetLiveViewType(void)
{
    return displayCfg.ReadLiveViewType();
}

bool ApplController::SetLiveViewType(LIVE_VIEW_TYPE_e liveViewType)
{
    return displayCfg.WriteLiveViewType(liveViewType);
}

void ApplController::setTVApperanceParameters(UINT32 offset)
{
    SetTVAdjustOffset(offset);
}

bool ApplController::readMaxWindowsForDisplay(quint16 &windowCount)
{
    return displayCfg.ReadMaxWindows(windowCount);
}

bool ApplController::writeMaxWindowsForDisplay(quint16 windowCount)
{
    return displayCfg.WriteMaxWindows(windowCount);
}

void ApplController::getFirmwareVersion(QString devName, QString &version)
{
    quint8 deviceIndex = 0;

    if (false == IsValidDeviceConnState(devName, deviceIndex))
    {
        return;
    }

    deviceClient[deviceIndex]->GetFirmwareVersion(version);
}

bool ApplController::swapWindows(DISPLAY_TYPE_e displayId, StreamRequestParam firstParam, StreamRequestParam secondParam)
{
    quint8 streamId1 = firstParam.streamId, streamId2 = secondParam.streamId;

    if (firstParam.windowId == secondParam.windowId)
    {
        return false;
    }

    if (false == SwapWindChannel(firstParam.windowId, secondParam.windowId, (DISPLAY_DEV_e)displayId))
    {
        return false;
    }

    do
    {
        if ((streamId1 >= MAX_STREAM_SESSION) || (streamId2 >= MAX_STREAM_SESSION))
        {
            break;
        }

        if (streamRequest[streamId1] == NULL)
        {
            streamId1 = MAX_STREAM_SESSION;
            break;
        }

        if (streamRequest[streamId2] == NULL)
        {
            streamId2 = MAX_STREAM_SESSION;
            break;
        }

        StreamRequest::swapWindowInfo(displayId, streamRequest[streamId1], streamRequest[streamId2]);
        return true;

    }while(0);

    if (streamId1 < MAX_STREAM_SESSION)
    {
        if (streamRequest[streamId1] != NULL)
        {
            streamRequest[streamId1]->setWindowInfo(displayId, secondParam.windowId, secondParam.actualWindowId, secondParam.timeStamp);
        }
    }
    else if (streamId2 < MAX_STREAM_SESSION)
    {
        if (streamRequest[streamId2] != NULL)
        {
            streamRequest[streamId2]->setWindowInfo(displayId, firstParam.windowId, firstParam.actualWindowId, firstParam.timeStamp);
        }
    }

    return true;
}

bool ApplController::ShiftWindows(DISPLAY_TYPE_e displayId, qint8 offset, quint8 newSelectedWindow, quint8 newLayout)
{
    quint8 streamId[MAX_WINDOWS];
    quint8 off = offset;

    for (quint8 windowIndex = 0; windowIndex < MAX_WINDOWS; windowIndex++)
    {
        streamId[windowIndex] = MAX_STREAM_SESSION;
        for (quint8 streamIndex = 0; streamIndex < MAX_STREAM_SESSION; streamIndex++)
        {
            quint8 windowId = MAX_WINDOWS;
            quint16 actualWindowId;
            qint64 timestamp;

            if (streamRequest[streamIndex] == NULL)
            {
                continue;
            }

            streamRequest[streamIndex]->getWindowInfo(displayId, windowId, actualWindowId, timestamp);
            if (windowId == windowIndex)
            {
                streamId[windowIndex] = streamIndex;
                break;
            }
        }
    }

    WINDOW_SHIFT_DIR_e shiftDirection = MAX_CHANNEL_SHIFT;

    if (offset > 0)
    {
        shiftDirection = CHANNEL_DOWN_SHIFT;
        off = offset;
    }
    else
    {
        shiftDirection = CHANNEL_UP_SHIFT;
        off = ((~offset) + 1);
    }

    if (false == UpdateChannelToWindowMap((DISPLAY_DEV_e)displayId, off, shiftDirection, newSelectedWindow, (WIND_LAYOUT_ID_e)newLayout))
    {
        return false;
    }

    if (offset > 0)
    {
        for (quint8 windowIndex = 0, tmpShift = 0; windowIndex < MAX_WINDOWS; windowIndex++, tmpShift++)
        {
            quint8 streamIndex = streamId[windowIndex];

            if (streamIndex >= MAX_STREAM_SESSION)
            {
                continue;
            }

            quint8 decoderWindow = ((windowIndex + off) < MAX_WINDOWS) ? (windowIndex + off) : tmpShift;

            if (streamRequest[streamIndex] != NULL)
            {
                streamRequest[streamIndex]->setWindowInfo(displayId, decoderWindow);
            }
        }
    }
    else
    {
        for(quint8 windowIndex = 0, tmpShift = (MAX_WINDOWS - off); windowIndex < MAX_WINDOWS; windowIndex++, tmpShift++)
        {
            quint8 streamIndex = streamId[windowIndex];

            if (streamIndex >= MAX_STREAM_SESSION)
            {
                continue;
            }

            quint8 decoderWindow = (windowIndex < off) ? tmpShift : (windowIndex - off);

            if (streamRequest[streamIndex] != NULL)
            {
                streamRequest[streamIndex]->setWindowInfo(displayId, decoderWindow);
            }
        }
    }

    return true;
}

bool ApplController::findFreeStreamSession(quint8 &streamId)
{
    bool status = false;

    streamRequestAllocationMutex.lock();
    streamId = lastStreamRequestIndex;
    for(quint8 index = 0; index < MAX_STREAM_SESSION; index++)
    {
        streamId++;
        if (streamId >= MAX_STREAM_SESSION)
        {
            streamId = 0;
        }

        if (true == isStreamRequestAllocated[streamId])
        {
            continue;
        }

        if (true == streamRequestThread[streamId].isRunning())
        {
            continue;
        }

        status = true;
        isStreamRequestAllocated[streamId] = true;
        lastStreamRequestIndex = streamId;
        break;
    }
    streamRequestAllocationMutex.unlock();

    return status;
}

/* Create new stream request object, move object to thread and start the thread */
void ApplController::createStreamRequest(quint8 streamId)
{
    streamRequest[streamId] = new StreamRequest(streamId);
    streamRequest[streamId]->moveToThread(&streamRequestThread[streamId]);

    /* set thread name and stack size */
    streamRequestThread[streamId].setObjectName("STRM_" + QString("%1").arg(streamId));
    streamRequestThread[streamId].setStackSize(STREAM_REQUEST_THREAD_STACK_SIZE);

    connect(streamRequest[streamId],
            SIGNAL(sigProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*)),
            streamRequest[streamId],
            SLOT(slotProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*)),
            Qt::QueuedConnection);
    connect(streamRequest[streamId],
            SIGNAL(sigProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*,
                                     SERVER_SESSION_INFO_t, StreamRequestParam*)),
            streamRequest[streamId],
            SLOT(slotProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*,
                                    SERVER_SESSION_INFO_t, StreamRequestParam*)),
            Qt::QueuedConnection);
    connect(streamRequest[streamId],
            SIGNAL(sigStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)),
            this,
            SLOT(slotStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)));
    connect(streamRequest[streamId],
            SIGNAL(sigDeleteStreamRequest(quint8)),
            this,
            SLOT(slotDeleteStreamRequest(quint8)));
    connect(streamRequest[streamId],
            SIGNAL(sigDelMedControl(quint8)),
            this,
            SLOT(slotDelMedControl(quint8)));
    connect(streamRequest[streamId],
            SIGNAL(sigChangeAudState()),
            this,
            SLOT(slotChangeAudState()));

    streamRequestThread[streamId].start();
}

void ApplController::deleteStreamRequest(quint8 streamId)
{
    if (streamId >= MAX_STREAM_SESSION)
    {
        return;
    }

    if (streamRequest[streamId] == NULL)
    {
        return;
    }

    disconnect(streamRequest[streamId],
               SIGNAL(sigProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*)),
               streamRequest[streamId],
               SLOT(slotProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*)));
    disconnect(streamRequest[streamId],
               SIGNAL(sigProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*, SERVER_SESSION_INFO_t, StreamRequestParam*)),
               streamRequest[streamId],
               SLOT(slotProcessRequest(STREAM_COMMAND_TYPE_e, SERVER_SESSION_INFO_t, StreamRequestParam*, SERVER_SESSION_INFO_t, StreamRequestParam*)));
    disconnect(streamRequest[streamId],
               SIGNAL(sigStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)),
               this,
               SLOT(slotStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)));
    disconnect(streamRequest[streamId],
               SIGNAL(sigDeleteStreamRequest(quint8)),
               this,
               SLOT(slotDeleteStreamRequest(quint8)));
    disconnect(streamRequest[streamId],
               SIGNAL(sigDelMedControl(quint8)),
               this,
               SLOT(slotDelMedControl(quint8)));
    disconnect(streamRequest[streamId],
               SIGNAL(sigChangeAudState()),
               this,
               SLOT(slotChangeAudState()));

    streamRequest[streamId]->deleteStreamRequest();
    streamRequest[streamId]->deleteLater();
    streamRequestThread[streamId].quit();
    streamRequestThread[streamId].wait(1000);
    streamRequest[streamId] = NULL;

    streamRequestAllocationMutex.lock();
    isStreamRequestAllocated[streamId] = false;
    streamRequestAllocationMutex.unlock();
}

void ApplController::devCommActivitySlot(QString deviceName, DevCommParam* param)
{
    emit sigDevCommGui(deviceName, param);
}

void ApplController::slotEvent(QString devName, LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubType, quint8 camIndx,
                                LOG_EVENT_STATE_e eventState, QString eventAdvanceDetail, bool isLiveEvent)
{
    emit sigEventToGui(devName, (quint8)eventType, (quint8)eventSubType, camIndx, (quint8)eventState, eventAdvanceDetail, isLiveEvent);
}

void ApplController::slotPopUpEvent(QString devIndex, quint8 camIndex, QString userName, quint32 popUpTimeStr, QString userId,
                                    QString doorName, quint8 evtCodeIndex)
{
    emit sigPopUpEventToGui(devIndex, camIndex, userName, popUpTimeStr, userId, doorName, evtCodeIndex);
}

void ApplController::slotAudioCfgChanged(const AUDIO_CONFIG_t *oldConfig, const AUDIO_CONFIG_t *newConfig)
{
    if ((oldConfig == NULL) || (oldConfig->level != newConfig->level))
    {
        SetAudioVolume(newConfig->level);
    }

    if ((oldConfig == NULL) || (oldConfig->muteStatus != newConfig->muteStatus))
    {
        SetAudioMuteUnMute((AUDIO_STATE_e)newConfig->muteStatus);
    }
}

void ApplController::slotLanguageCfgChanged(QString str)
{
    emit sigLanguageCfgModified(str);
}

void ApplController::slotDeviceCfgChanged(quint8 devIndex, const DEVICE_CONFIG_t *oldConfig, const DEVICE_CONFIG_t *newConfig)
{
    if (oldConfig != NULL)
    {
        DPRINT(DEVICE_CLIENT, "device old config: [index=%d], [name=%s], [enable=%d], [autoLogin=%d], [nativeDeviceCredential=%d], [username=%s]",
               devIndex, oldConfig->deviceName, oldConfig->enable, oldConfig->autoLogin, oldConfig->nativeDeviceCredential, oldConfig->username);
    }

    if (newConfig != NULL)
    {
        DPRINT(DEVICE_CLIENT, "device new config: [index=%d], [name=%s], [enable=%d], [autoLogin=%d], [nativeDeviceCredential=%d], [username=%s]",
               devIndex, newConfig->deviceName, newConfig->enable, newConfig->autoLogin, newConfig->nativeDeviceCredential, newConfig->username);
    }

    do
    {
        /* Nothing to do if device client is not valid */
        if (deviceClient[devIndex] == NULL)
        {
            break;
        }

        /* Is run time configuration changed? */
        if ((oldConfig == NULL) || (newConfig == NULL))
        {
            break;
        }

        if ((strcmp(oldConfig->deviceName, newConfig->deviceName) != 0) || (oldConfig->enable != newConfig->enable)
                || (oldConfig->connType != newConfig->connType) || (strcmp(oldConfig->ipAddress, newConfig->ipAddress) != 0)
                || (oldConfig->nativeDeviceCredential != newConfig->nativeDeviceCredential)
                || (strcmp(oldConfig->username, newConfig->username) != 0) || (strcmp(oldConfig->password, newConfig->password) != 0))
        {
            deviceClient[devIndex]->UpdateAutoLogin(false);
            displayCfg.DeleteDeviceFromConfig(oldConfig->deviceName);

            if (true == IsValidDeviceConnState(devIndex))
            {
                deviceClient[devIndex]->setDevConnState(DELETED);
                deviceClient[devIndex]->LogoutFromDevice();

                while (deviceThread[devIndex].wait(10) == false)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                }
            }
            else
            {
                deviceClient[devIndex]->setDevConnState(DELETED);
                DevCommParam* param = new DevCommParam();
                param->msgType = MSG_REQ_LOG;
                param->deviceStatus = CMD_DEV_DELETED;
                emit sigDevCommGui(oldConfig->deviceName, param);

                /* Stop connect request and exit device thread if running */
                deviceClient[devIndex]->StopConnectRequest();
                if (true == deviceThread[devIndex].isRunning())
                {
                    deviceThread[devIndex].quit();
                    deviceThread[devIndex].wait(1000);
                }
            }

            disconnect(deviceClient[devIndex],
                       SIGNAL(sigPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8)),
                       this,
                       SLOT(slotPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8)));
            disconnect(deviceClient[devIndex],
                       SIGNAL(sigEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool)),
                       this,
                       SLOT(slotEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool)));
            disconnect(deviceClient[devIndex],
                       SIGNAL(sigDeviceResponse(QString,DevCommParam*)),
                       this,
                       SLOT(devCommActivitySlot(QString,DevCommParam*)));

            /* device connection for request queue */
            disconnect(deviceClient[devIndex],
                       SIGNAL(SigProcessRequest(DevCommParam*)),
                       deviceClient[devIndex],
                       SLOT(SlotProcessRequest(DevCommParam*)));

            /* device connection with thread */
            disconnect(deviceClient[devIndex],
                       SIGNAL(sigExitThread()),
                       &deviceThread[devIndex],
                       SLOT(quit()));
            disconnect(deviceClient[devIndex],
                       SIGNAL(sigDeleteStreamRequest(QString)),
                       this,
                       SLOT(slotDeleteStreamRequest(QString)));
            disconnect(deviceClient[devIndex],
                       SIGNAL(sigDeviceCfgUpdate(quint8,DEVICE_CONFIG_t*, bool)),
                       this,
                       SLOT(slotDeviceCfgUpdate(quint8,DEVICE_CONFIG_t*, bool)));

            /* delete device client instance */
            DELETE_OBJ(deviceClient[devIndex]);
        }
        else
        {
            /* Just update the new configuration if auto login is disabled in new config */
            deviceClient[devIndex]->changeDeviceConfig(newConfig);
            deviceClient[devIndex]->UpdateAutoLogin(newConfig->autoLogin);
            if (false == newConfig->autoLogin)
            {
                /* Auto login is diabled */
                break;
            }

            if ((oldConfig->autoLogin == newConfig->autoLogin) && (oldConfig->port == newConfig->port)
                    && (oldConfig->forwardedTcpPort == newConfig->forwardedTcpPort))
            {
                /* No need to change anything in login */
                break;
            }

            /* Initially, We don't start the thread if auto login is disabled */
            if (oldConfig->autoLogin == false)
            {
                /* Now auto login is enabled, start the thread */
                deviceThread[devIndex].start();
            }

            /* If device is already logged-in then first logout and then login again */
            if (true == IsValidDeviceConnState(devIndex))
            {
                deviceClient[devIndex]->UpdateAutoLogin(false);
                deviceClient[devIndex]->LogoutFromDevice();
                deviceClient[devIndex]->changeDeviceConfig(newConfig);
                deviceClient[devIndex]->setLoginAfterLogoutFlag(true);
            }
            else
            {
                /* Device is not logged-in, start the login */
                if (deviceClient[devIndex]->GetConnectionState() == DISCONNECTED)
                {
                    deviceClient[devIndex]->LoginToDevice();
                }
            }
        }

    }while(0);

    /* Is device client obj already created? */
    if (IS_VALID_OBJ(deviceClient[devIndex]))
    {
        return;
    }

    /* Device is not created and config change to enable device */
    if ((newConfig == NULL) || (newConfig->enable == false) || (newConfig->deviceName[0] == '\0'))
    {
        return;
    }

    /* create a new device client instance */
    deviceClient[devIndex] = new DeviceClient(devIndex, newConfig);
    deviceClient[devIndex]->moveToThread(&deviceThread[devIndex]);
    deviceThread[devIndex].setObjectName("DC_" + ((devIndex == 0) ? "LOCAL" : QString(newConfig->deviceName)));

    /* device connection with thread */
    connect(deviceClient[devIndex],
            SIGNAL(sigExitThread()),
            &deviceThread[devIndex],
            SLOT(quit()));
    connect(deviceClient[devIndex],
            SIGNAL(sigDeleteStreamRequest(QString)),
            this,
            SLOT(slotDeleteStreamRequest(QString)));

    /* device connection for request queue */
    connect(deviceClient[devIndex],
            SIGNAL(SigProcessRequest(DevCommParam*)),
            deviceClient[devIndex],
            SLOT(SlotProcessRequest(DevCommParam*)),
            Qt::QueuedConnection);

    /* device connection with application controller */
    connect(deviceClient[devIndex],
            SIGNAL(sigDeviceResponse(QString, DevCommParam*)),
            this,
            SLOT(devCommActivitySlot(QString, DevCommParam*)));
    connect(deviceClient[devIndex],
            SIGNAL(sigEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool)),
            this,
            SLOT(slotEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool)));
    connect(deviceClient[devIndex],
            SIGNAL(sigPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8)),
            this,
            SLOT(slotPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8)));
    connect(deviceClient[devIndex],
            SIGNAL(sigDeviceCfgUpdate(quint8, DEVICE_CONFIG_t*, bool)),
            this,
            SLOT(slotDeviceCfgUpdate(quint8, DEVICE_CONFIG_t*, bool)));

    /* if auto login is disabled then nothing to do now */
    if (false == newConfig->autoLogin)
    {
        return;
    }

    /* Start device thread and do login */
    deviceThread[devIndex].start();
    deviceClient[devIndex]->LoginToDevice();
}

void ApplController::slotStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam,
                                               DEVICE_REPLY_TYPE_e deviceReply)
{
    emit sigStreamRequestResponse(streamCommandType, streamRequestParam, deviceReply);
}

void ApplController::deleteStreamRequestForDevice(QString deviceName)
{
   slotDeleteStreamRequest(deviceName);
}

void ApplController::slotDeleteStreamRequest(quint8 streamId)
{
    deleteStreamRequest(streamId);
}

void ApplController::slotDelMedControl(quint8 streamId)
{
    if (streamId >= MAX_STREAM_SESSION)
    {
        return;
    }

    if (streamRequest[streamId] != NULL)
    {
        streamRequest[streamId]->delMedControl();
    }
}

void ApplController::slotDeleteStreamRequest(QString deviceName)
{
    DISPLAY_TYPE_e displayTypeForDelete[MAX_DISPLAY_TYPE];
    quint16 actualWindowIdForDelete[MAX_DISPLAY_TYPE];

    for (quint8 streamId = 0; streamId < MAX_STREAM_SESSION; streamId++)
    {
        if (streamRequest[streamId] == NULL)
        {
            continue;
        }

        if (streamRequest[streamId]->getStreamDeviceName() != deviceName)
        {
            continue;
        }

        streamRequest[streamId]->getStreamWindowInfo(displayTypeForDelete, actualWindowIdForDelete);

        emit sigStreamObjectDelete(displayTypeForDelete, actualWindowIdForDelete);
        deleteStreamRequest(streamId);
    }
}

void ApplController::slotDeviceCfgUpdate(quint8 remoteDeviceIndex, DEVICE_CONFIG_t *deviceConfig, bool isUpdateOnLiveEvent)
{
    DEVICE_CONFIG_t tempDeviceConfig;

    deviceList.replace(remoteDeviceIndex, deviceConfig->deviceName);
    if ((remoteDeviceIndex > 0) && ((remoteDeviceIndex - 1) < MAX_REMOTE_DEVICES))
    {
        if (isUpdateOnLiveEvent) // update On live Events......
        {
            memcpy(&tempDeviceConfig, deviceConfig, sizeof(DEVICE_CONFIG_t));
            if ((tempDeviceConfig.deviceName[0] != '\0') || (rdevConfig[remoteDeviceIndex-1].deviceName[0] != '\0'))
            {
                if ((tempDeviceConfig.enable) && (tempDeviceConfig.nativeDeviceCredential))
                {
                    QString username, password;
                    getUsernameFrmDev(LOCAL_DEVICE_NAME, username);
                    getPasswordFrmDev(LOCAL_DEVICE_NAME, password);

                    if (username !=  DEFAULT_LOGIN_USER)
                    {
                        snprintf(tempDeviceConfig.username, MAX_USERNAME_SIZE, "%s", username.toUtf8().constData());
                        snprintf(deviceConfig->username, MAX_USERNAME_SIZE, "%s", tempDeviceConfig.username);
                        snprintf(tempDeviceConfig.password, MAX_PASSWORD_SIZE, "%s", password.toUtf8().constData());
                        snprintf(deviceConfig->password, MAX_PASSWORD_SIZE, "%s", tempDeviceConfig.password);
                    }
                }

                slotDeviceCfgChanged(remoteDeviceIndex, &rdevConfig[remoteDeviceIndex-1], &tempDeviceConfig);
            }

            memcpy(&rdevConfig[remoteDeviceIndex-1], deviceConfig, sizeof(DEVICE_CONFIG_t));
        }
        else  // update on init
        {
            memcpy(&rdevConfig[remoteDeviceIndex-1], deviceConfig, sizeof(DEVICE_CONFIG_t));
            if (rdevConfig[remoteDeviceIndex-1].deviceName[0] != '\0')
            {
                slotDeviceCfgChanged(remoteDeviceIndex, NULL, &rdevConfig[remoteDeviceIndex-1]);
            }
        }
    }

    emit sigDeviceListChangeToGui();
}

void hdmiCallBack(BOOL IsHdmiInfoShow)
{
    ApplController* appController = ApplController::getInstance();
    appController->hdmiLoadInfo((bool)IsHdmiInfoShow);
}

void ApplController::deleteDeviceClient(quint8 index)
{
    if (deviceClient[index] == NULL)
    {
        return;
    }

    deviceClient[index]->deleteDevClntInstants();
    deviceThread[index].quit();
    deviceThread[index].wait();
    disconnect(deviceClient[index],
               SIGNAL(sigPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8)),
               this,
               SLOT(slotPopUpEvent(QString, quint8, QString, quint32, QString, QString, quint8)));
    disconnect(deviceClient[index],
               SIGNAL(sigEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool)),
               this,
               SLOT(slotEvent(QString, LOG_EVENT_TYPE_e, LOG_EVENT_SUBTYPE_e, quint8, LOG_EVENT_STATE_e, QString, bool)));
    disconnect(deviceClient[index],
               SIGNAL(sigDeviceResponse(QString, DevCommParam*)),
               this,
               SLOT(devCommActivitySlot(QString, DevCommParam*)));

    // device connection for request queue
    disconnect(deviceClient[index],
               SIGNAL(SigProcessRequest(DevCommParam*)),
               deviceClient[index],
               SLOT(SlotProcessRequest(DevCommParam*)));

    // device connection with thread
    disconnect(deviceClient[index],
               SIGNAL(sigExitThread()),
               &deviceThread[index],
               SLOT(quit()));
    disconnect(deviceClient[index],
               SIGNAL(sigDeleteStreamRequest(QString)),
               this,
               SLOT(slotDeleteStreamRequest(QString)));
    disconnect(deviceClient[index],
               SIGNAL(sigDeviceCfgUpdate(quint8, DEVICE_CONFIG_t*, bool)),
               this,
               SLOT(slotDeviceCfgUpdate(quint8, DEVICE_CONFIG_t*, bool)));

    // delete device client instance
    DELETE_OBJ(deviceClient[index]);
}

quint32 ApplController::getHeightOfScreen()
{
    return applController->m_screenHeight;
}

quint32 ApplController::getWidthOfScreen()
{
    return applController->m_screenWidth;
}

quint32 ApplController::getXPosOfScreen()
{
    return applController->m_screenXPos;
}

quint32 ApplController::getYPosOfScreen()
{
    return applController->m_screenYPos;
}

bool ApplController::getOriginofScreen(LAYOUT_TYPE_e layoutIndex, DISPLAY_TYPE_e displayType)
{
    VALID_SCREEN_INFO_t screenInfo;

    if (false == GetValidScreenInfo((DISPLAY_DEV_e)displayType, &screenInfo, (WIND_LAYOUT_ID_e)layoutIndex))
    {
        return false;
    }

    m_screenXPos = screenInfo.actualStartX;
    m_screenYPos = screenInfo.actualStartY;
    m_screenWidth = screenInfo.actualWidth;
    m_screenHeight = screenInfo.actualHeight;
    return true;
}

void ApplController::readAudioConfig()
{
    AUDIO_CONFIG_t audioConfig = {DEFAULT_VOL_LEVEL, AUDIO_UNMUTE};
    QList<QVariant> paramList;

    paramList.append(READ_AUDIO_ACTIVITY);
    if (processActivity(AUDIO_SETTING, paramList, &audioConfig))
    {
        m_audioLevel = audioConfig.level;
        m_currentMuteState = (audioConfig.muteStatus == AUDIO_MUTE ? ON_STATE : OFF_STATE);
    }
    paramList.clear();
}

void ApplController::writeAudioConfig(int currentState, int audioLevel)
{
    AUDIO_CONFIG_t audioConfig;
    QList<QVariant> paramList;

    paramList.append(WRITE_AUDIO_ACTIVITY);
    audioConfig.level = audioLevel;
    audioConfig.muteStatus = (currentState == OFF_STATE ? AUDIO_UNMUTE : AUDIO_MUTE);

    if (processActivity(AUDIO_SETTING, paramList, &audioConfig))
    {
        m_currentMuteState = currentState;
    }

    paramList.clear();
}

void ApplController::slotChangeAudState()
{
    readAudioConfig();

    if (m_currentMuteState == TRUE)
    {
        if (m_audioLevel == 0)
        {
            m_audioLevel = DEFAULT_VOL_LEVEL;
        }

        writeAudioConfig(FALSE, m_audioLevel);
        emit sigChangeAudButtonState();
    }
}
