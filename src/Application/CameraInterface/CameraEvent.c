//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file       CameraEvent.c
@brief      This module provide camera event polling interface to event handler
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "CameraEvent.h"
#include "SysTimer.h"
#include "EventHandler.h"
#include "Utils.h"
#include "UrlRequest.h"
#include "HttpClient.h"
#include "MxOnvifClient.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define CAM_EVENT_DEBOUNCE_TIME             10
#define CAM_EVENT_DEBOUNCE_TIME_ONE_ACTION  10
#define EV_REQ_TIME_OUT                     2
#define MAX_RENEW_CNT                       60
#define EVENT_NOT_ARRIVED                   -1
#define MAX_EVENT_WAIT_CNT                   2000

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    EVENT_SEND_NW_ONLY = 0,
    EVENT_SEND_NW_FILE,
    MAX_EVENT_SEND
}EVENT_SEND_TYPE_e;

/* To handle Event Polling Data */
typedef struct
{
    BOOL 		        	eventPollStatusFlag[MAX_CAMERA_EVENT];  // flag status of every event
    pthread_mutex_t    		evtPollStatusLock;
    TIMER_HANDLE        	evtTimerHandle[MAX_CAMERA_EVENT];
    pthread_mutex_t    		evtStatusLock;
    EVENT_RESULT_t  		evRes;                                  // to save the event result globally
    UINT8					redetDelay[MAX_CAMERA_EVENT];
    EVENT_SEND_TYPE_e		eventStatusNotify[MAX_CAMERA_EVENT];
}EVENT_POLL_t;

typedef struct
{
    BOOL 					evtStatus;
    pthread_mutex_t 		evStatusLock;
}EVENT_DET_t;

/* Event Request info */
typedef struct
{
    EVENT_DET_t				evDet[MAX_CAMERA_EVENT + 1];    // Send Url Number to Get Ev Status
    EVENT_RESP_INFO_t		evRespInfo;                     // Event response info multipart/single part
    CAMERA_EVENT_REQUEST_t  evPollReq;                      // Camera event request
    TIMER_HANDLE			onvifEvtTmrHandle;
    UINT32					onvifRenewReqCnt;
    INT32					onvifEvtRespCnt[MAX_CAMERA_EVENT];
}EVENT_REQ_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void makeCamEventOff(UINT8 camIndex, CAMERA_EVENT_e camEvent);
//-------------------------------------------------------------------------------------------------
static BOOL startEvtTimer(UINT8 camIndex, CAMERA_EVENT_e camEvent, UINT32 startTime);
//-------------------------------------------------------------------------------------------------
static void declareEvtNormal(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL checkEvReqStatus(EVENT_REQ_INFO_t *evReqInfo, CAMERA_EVENT_e camEvent);
//-------------------------------------------------------------------------------------------------
static void httpEvRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void declareEvtInactive(UINT8 cameraIndex, CAMERA_EVENT_e camEvent);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendEvReqToCam(UINT8 camIndex, IP_CAMERA_CONFIG_t *ipCamCfg);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendOnvifEventCommand(ONVIF_REQ_PARA_t *onvifReqParaPtr, IP_CAMERA_CONFIG_t *ipCamCfg);
//-------------------------------------------------------------------------------------------------
static BOOL onvifStartEventCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static void getOnvifEvtNotifyTmrCb(UINT32 cameraIndex);
//-------------------------------------------------------------------------------------------------
static BOOL getEvtNotificationCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifEvtUnsubscribeCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifEvtRenewCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static LOG_EVENT_SUBTYPE_e getEvtLogSubType(CAMERA_EVENT_e camEvent);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static EVENT_POLL_t         evPoll[MAX_CAMERA];
static EVENT_REQ_INFO_t     evRqInfo[MAX_CAMERA];
static const CHARPTR        cameraEvtStr[MAX_CAMERA_EVENT] =
{
    "Motion",
    "Tamper",
    "Sensor 1",
    "Sensor 2",
    "Sensor 3",
    "Camera Offline",
    "Recording Failed",
    "Line Cross",
    "Object Intrution",
    "Audio Exception",
    "Missing Object",
    "Suspicious Object",
    "Loitering",
    "Camera Online",
    "Recording Start",
    "Object Counting",
    "No Motion",
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize all camera event poll related parameter
 */
void InitCameraEventPoll(void)
{
    UINT8           camIndex, requestCount;
    CAMERA_EVENT_e  camEvent;

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        for(camEvent = 0; camEvent < MAX_CAMERA_EVENT; camEvent++)
        {
            evPoll[camIndex].evtTimerHandle[camEvent] = INVALID_TIMER_HANDLE;
            evPoll[camIndex].eventPollStatusFlag[camEvent] = OFF;
            evPoll[camIndex].evRes.eventStatus[camEvent] = OFF;
            evPoll[camIndex].evRes.eventHealthStatus[camEvent] = OFF;
            evPoll[camIndex].redetDelay[camEvent] = 0;
            evPoll[camIndex].eventStatusNotify[camEvent] = EVENT_SEND_NW_FILE;
        }

        MUTEX_INIT(evPoll[camIndex].evtPollStatusLock, NULL);
        MUTEX_INIT(evPoll[camIndex].evtStatusLock, NULL);

        /* Initialisation Related Camera Event Request */
        for(camEvent = 0; camEvent < (MAX_CAMERA_EVENT + 1); camEvent++)
        {
            evRqInfo[camIndex].evDet[camEvent].evtStatus = OFF;
            MUTEX_INIT(evRqInfo[camIndex].evDet[camEvent].evStatusLock, NULL);
        }

        for(requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
        {
            evRqInfo[camIndex].evPollReq.handle[requestCount] = INVALID_HTTP_HANDLE;
        }

        evRqInfo[camIndex].onvifEvtTmrHandle = INVALID_TIMER_HANDLE;
        memset(evRqInfo[camIndex].evPollReq.httpCallback, '\0', sizeof(evRqInfo[camIndex].evPollReq.httpCallback));
        evRqInfo[camIndex].evPollReq.httpCallback[CAM_REQ_CONTROL] = httpEvRequestCb;
        evRqInfo[camIndex].evPollReq.numOfRequest = 0;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This will start timer function of each camera for event polling and make each event polling ON
 * @param   camIndex
 * @param   camEvent
 * @return  Status
 */
NET_CMD_STATUS_e StartCameraEventPoll(UINT8 camIndex, CAMERA_EVENT_e camEvent)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_PROCESS_ERROR;
    UINT8			 	sendUrlNo;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    ONVIF_REQ_PARA_t 	onvifReqPara;
    BOOL 				camType = MAX_CAMERA_TYPE;

    /* Check camera index. It should be in valid range */
    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAM_EVENT, "invld camera found: [camera=%d]", camIndex);
        return CMD_PROCESS_ERROR;
    }

    /* Validate input params */
    if ((camEvent >= MAX_CAMERA_EVENT) || ((camType = CameraType(camIndex)) >= MAX_CAMERA_TYPE))
    {
        return CMD_PROCESS_ERROR;
    }

    /* Don't proceed, if added using CI because camera will send event directly */
    if (AUTO_ADDED_CAMERA == camType)
    {
        return CMD_PROCESS_ERROR;
    }

    do
    {
        /* Check whether event polling already started or not */
        MUTEX_LOCK(evPoll[camIndex].evtPollStatusLock);
        if(evPoll[camIndex].eventPollStatusFlag[camEvent] == ON)
        {
            /* Camera event poll already started. Return with Success. */
            MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);
            requestStatus = CMD_SUCCESS;
            break;
        }
        evPoll[camIndex].eventPollStatusFlag[camEvent] = ON;
        MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);

        DPRINT(CAM_EVENT, "polling start: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);

        /* Following events - CONNECTION_FAILURE, RECORDING_FAIL/START, CAMERA_ONLINE are not actually camera events.
         * These events are generated from NVR itself. No need to poll for these events. Return with SUCCESS. */
        if((camEvent == CONNECTION_FAILURE) || (camEvent == RECORDING_FAIL) || (camEvent == RECORDING_START) || (camEvent == CAMERA_ONLINE))
        {
            requestStatus = CMD_SUCCESS;
            break;
        }

        /* Do not get event if camera is disconnected */
        if (GetCamEventStatus(camIndex, CONNECTION_FAILURE) == INACTIVE)
        {
            requestStatus = CMD_CAM_DISCONNECTED;
            break;
        }

        /* Read Camera configuration */
        ReadSingleIpCameraConfig(camIndex, &ipCamCfg);

        /* If camera is ONVIF then make polling for MAX_CAMERA_EVENT ON and send start event request to ONVIF */
        if (ipCamCfg.onvifSupportF == TRUE)
        {
            /* While Creating pull point subscription for onvif events, we are not specifying any particular event
             * for  subscription. That is why to maintain the status of event we check at MAX_CAMERA_EVENT index. */
            MUTEX_LOCK(evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evStatusLock);
            if(evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evtStatus == ON)
            {
                MUTEX_UNLOCK(evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evStatusLock);
                requestStatus = CMD_SUCCESS;
                break;
            }
            evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evtStatus = ON;
            MUTEX_UNLOCK(evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evStatusLock);

            /* Assign notify callback which is further used to receive onvif event status */
            evRqInfo[camIndex].evRespInfo.evRespType = EV_RESP_ONVIF;
            onvifReqPara.camIndex = camIndex;
            onvifReqPara.onvifReq = ONVIF_START_EVENT;
            onvifReqPara.onvifCallback = onvifStartEventCb;

            /* Send ONVIF_START_EVENT command to camera */
            requestStatus = sendOnvifEventCommand(&onvifReqPara, &ipCamCfg);
            if(requestStatus != CMD_SUCCESS)
            {
                /* Make Polling OFF for request error */
                MUTEX_LOCK(evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evStatusLock);
                evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evtStatus = OFF;
                MUTEX_UNLOCK(evRqInfo[camIndex].evDet[MAX_CAMERA_EVENT].evStatusLock);
            }

            /* Done for ONVIF */
            break;
        }

        /* For brand/model event handling, If handle at MAX_CAMERA_EVENT is a valid handle, event polling
         * for all the supported events on camera is already going on. No need to send another request. */
        if(checkEvReqStatus(&evRqInfo[camIndex], MAX_CAMERA_EVENT) == ON)
        {
            requestStatus = CMD_SUCCESS;
            DPRINT(CAM_EVENT, "polling already started: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
            break;
        }

        requestStatus = GetCameraBrandModelUrl(camIndex, REQ_URL_GET_EVENT, NULL, &evRqInfo[camIndex].evPollReq,
                                               &camEvent, &evRqInfo[camIndex].evRespInfo, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAM_EVENT, "fail to get event status url: [camera=%d], [event=%s], [status=%d]", camIndex, cameraEvtStr[camEvent], requestStatus);
            break;
        }

        /* Retrieve event url number which is used to retrieve event status from camera. If requestType of
         * MAX_CAMERA_EVENT is not MAX_CAMERA_EVENT then a valid URL is filled in this index and it indicates
         * that only one URL will give all the supported event's status */
        if(evRqInfo[camIndex].evPollReq.url[MAX_CAMERA_EVENT].requestType != MAX_CAM_REQ_TYPE)
        {
            sendUrlNo = MAX_CAMERA_EVENT;
        }
        else if(evRqInfo[camIndex].evPollReq.url[camEvent].requestType != MAX_CAM_REQ_TYPE)
        {
            sendUrlNo = camEvent;
        }
        else
        {
            /* Not a valid event */
            requestStatus = CMD_FEATURE_NOT_SUPPORTED;
            EPRINT(CAM_EVENT, "event status http url not found: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
            break;
        }

        /* Check whether event polling is already on or not */
        if((evRqInfo[camIndex].evRespInfo.evRespType == EV_RESP_MULTI_PART) && (evRqInfo[camIndex].evPollReq.handle[sendUrlNo] != INVALID_HTTP_HANDLE))
        {
            requestStatus = CMD_SUCCESS;
            EPRINT(CAM_EVENT, "event polling already running: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
            break;
        }

        /* Now send request to camera */
        evRqInfo[camIndex].evPollReq.requestCount = sendUrlNo;
        requestStatus = sendEvReqToCam(camIndex, &ipCamCfg);
    }
    while(0);

    if(requestStatus != CMD_SUCCESS)
    {
        /* Make event poll status flag OFF */
        makeCamEventOff(camIndex, camEvent);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It stops the camera event polling for particular camera If changes occurs sends the notification
 * @param   camIndex
 * @param   camEvent
 */
void StopCameraEventPoll(UINT8 camIndex, CAMERA_EVENT_e camEvent)
{
    UINT8 stopReq;

    /* Check Invalid camera number or camera event number */
    if ((camIndex >= getMaxCameraForCurrentVariant()) || (camEvent >= MAX_CAMERA_EVENT) || (CameraType(camIndex) >= MAX_CAMERA_TYPE))
    {
        return;
    }

    /* If polling is already OFF then no need to procedure for stop request */
    MUTEX_LOCK(evPoll[camIndex].evtPollStatusLock);
    if(evPoll[camIndex].eventPollStatusFlag[camEvent] == OFF)
    {
        MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);
        return;
    }
    MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);

    DPRINT(CAM_EVENT, "polling stop: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
    if (checkEvReqStatus(&evRqInfo[camIndex], MAX_CAMERA_EVENT) == ON)
    {
        /* If polling of any event is ON, normalize event. Give callback so that polling flag will be reset.
         * Note: This process will also be performed when camera is ONVIF */
        if(GetEventPollStatus(camIndex, MAX_CAMERA_EVENT) == ON)
        {
            EventDetectFunc(camIndex, camEvent, INACTIVE);
            makeCamEventOff(camIndex, camEvent);
        }

        /* Now if not a single event is being polled, close connection */
        if(GetEventPollStatus(camIndex, MAX_CAMERA_EVENT) == ON)
        {
            /* Other events are also required to poll */
            return;
        }

        /* Stop camera event polling */
        stopReq = MAX_CAMERA_EVENT;
    }
    else
    {
        /* If handler of the event is valid then that event is being polled */
        if(checkEvReqStatus(&evRqInfo[camIndex], camEvent) == OFF)
        {
            /* Event polling is already stopped */
            return;
        }

        /* Normalise event and give callback to stop polling */
        EventDetectFunc(camIndex, camEvent, INACTIVE);
        makeCamEventOff(camIndex, camEvent);
        stopReq = camEvent;
    }

    DPRINT(CAM_EVENT, "polling stopped: [camera=%d], [event=%d], [connection=%d], [resp=%d]",
           camIndex, camEvent, stopReq, evRqInfo[camIndex].evRespInfo.evRespType);

    /* If Event Response type is for onvif camera */
    if (evRqInfo[camIndex].evRespInfo.evRespType == EV_RESP_ONVIF)
    {
        MUTEX_LOCK(evRqInfo[camIndex].evDet[stopReq].evStatusLock);
        if(evRqInfo[camIndex].evDet[stopReq].evtStatus == ON)
        {
            evRqInfo[camIndex].evDet[stopReq].evtStatus = OFF;
        }
        MUTEX_UNLOCK(evRqInfo[camIndex].evDet[stopReq].evStatusLock);
    }
    else
    {
        StopHttp(evRqInfo[camIndex].evPollReq.handle[stopReq]);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will give the polling status for the given event of the given camera
 * @param   camIndex
 * @param   camEvent
 * @return
 */
BOOL GetEventPollStatus(UINT8 camIndex, CAMERA_EVENT_e camEvent)
{
    UINT8   event;
    BOOL	status = OFF;

    /* If request to get event poll status for MAX_EVENTS check for all the events that needs
     * to be polled. Connection failure doesn't need to be polled hence don't consider it */
    if(camEvent == MAX_CAMERA_EVENT)
    {
        MUTEX_LOCK(evPoll[camIndex].evtPollStatusLock);
        for(event = 0; event < MAX_CAMERA_EVENT; event++)
        {
            if(evPoll[camIndex].eventPollStatusFlag[event] == ON)
            {
                break;
            }
        }
        MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);

        if(event < MAX_CAMERA_EVENT)
        {
            status = ON;
        }
    }
    else
    {
        MUTEX_LOCK(evPoll[camIndex].evtPollStatusLock);
        if(evPoll[camIndex].eventPollStatusFlag[camEvent] == ON)
        {
            status = ON;
        }
        MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Checks the input local event status result with global event status result If changes
 *          occurs sends the notification
 * @param   camIndex
 * @param   camEvent
 * @param   status
 * @return
 */
BOOL EventDetectFunc(UINT8 camIndex, CAMERA_EVENT_e camEvent, BOOL status)
{
    BOOL 					sendEvent = TRUE;
    BOOL					retVal = TRUE;
    BOOL					eventStatus = status;
    LOG_EVENT_SUBTYPE_e		eventSubType = LOG_MAX_CAMERA_EVENT;
    CHAR 					detail[MAX_EVENT_DETAIL_SIZE];
    CAMERA_CONFIG_t			camConfig;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (camEvent >= MAX_CAMERA_EVENT))
    {
        return FALSE;
    }

    MUTEX_LOCK(evPoll[camIndex].evtStatusLock);
    if ((camEvent != CONNECTION_FAILURE) && (evPoll[camIndex].eventPollStatusFlag[camEvent] == OFF) && (CameraType(camIndex) != AUTO_ADDED_CAMERA))
    {
        MUTEX_UNLOCK(evPoll[camIndex].evtStatusLock);
        return FALSE;
    }

    ReadSingleCameraConfig(camIndex, &camConfig);
    if(((camEvent == MOTION_DETECT) || (camEvent == NO_MOTION_DETECTION)) && (camConfig.motionDetectionStatus == FALSE))
    {
        /* In case of MOTION_DETECT if motion detection is disabled in configuration than update event health status only. */
        if((evPoll[camIndex].evRes.eventHealthStatus[camEvent] == OFF) || (status != INACTIVE))
        {
            MUTEX_UNLOCK(evPoll[camIndex].evtStatusLock);
            return FALSE;
        }

        evPoll[camIndex].evRes.eventHealthStatus[camEvent] = status;
    }

    /* compare current and previous status */
    if (status == evPoll[camIndex].evRes.eventStatus[camEvent])
    {
        MUTEX_UNLOCK(evPoll[camIndex].evtStatusLock);
        return FALSE;
    }

    evPoll[camIndex].evRes.eventStatus[camEvent] = status;
    if((camEvent == CONNECTION_FAILURE) || (status == ACTIVE))
    {
        evPoll[camIndex].evRes.eventHealthStatus[camEvent] = status;
    }
    MUTEX_UNLOCK(evPoll[camIndex].evtStatusLock);

    if(camEvent == CONNECTION_FAILURE)
    {
        DPRINT(CAM_EVENT, "camera connectivity: [camera=%d], [status=%s]", camIndex, status ? "online" : "offline");
        if(evPoll[camIndex].evtTimerHandle[camEvent] != INVALID_TIMER_HANDLE)
        {
            DeleteTimer(&evPoll[camIndex].evtTimerHandle[camEvent]);
            DPRINT(CAM_EVENT, "camera connectivity timer deleted: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
        }

        /* Mark camera connectivity event status as ACTIVE to take action */
        eventSubType = LOG_CONNECTIVITY;
        eventStatus = ACTIVE;
        if(camConfig.camera == ENABLE)
        {
            if(ACTIVE == status)
            {
                camEvent = CAMERA_ONLINE;
            }
        }
        else
        {
            if(ACTIVE == status)
            {
                /* If camera is disabled than CONNECTION_FAILURE event with ACTIVE status should not be
                 * sent as CONNECTION_FAILURE with ACTIVE status is considered as CAMERA_ONLINE. */
                sendEvent = FALSE;
                evPoll[camIndex].eventStatusNotify[camEvent] = MAX_EVENT_SEND;

                /* Do not switch unnecessary ACTIVE state while camera in DISABLE state,
                 * it will entertain the next  INACTIVE status coming from connection monitor thread */
                evPoll[camIndex].evRes.eventStatus[camEvent] = !status;
                evPoll[camIndex].evRes.eventHealthStatus[camEvent] = !status;
            }
        }
        evPoll[camIndex].eventStatusNotify[camEvent] = EVENT_SEND_NW_FILE;
    }
    else
    {
        /* Get subtype of event log */
        eventSubType = getEvtLogSubType(camEvent);

        if(status == ACTIVE)
        {
            /* If any previous timer was configured, then delete it. */
            if(evPoll[camIndex].evtTimerHandle[camEvent] != INVALID_TIMER_HANDLE)
            {
                DeleteTimer(&evPoll[camIndex].evtTimerHandle[camEvent]);
                sendEvent = FALSE;
                DPRINT(CAM_EVENT, "event active, inactive wait timer deleted: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
            }
            else
            {
                /* In case of MOTION_DETECTION use redetection time as configured in camera settings else use CAM_EVENT_DEBOUNCE_TIME. */
                if ((camEvent == MOTION_DETECT) || (camEvent == NO_MOTION_DETECTION))
                {
                    evPoll[camIndex].eventStatusNotify[camEvent] = (camConfig.logMotionEvents == TRUE) ? EVENT_SEND_NW_FILE : EVENT_SEND_NW_ONLY;
                    evPoll[camIndex].redetDelay[camEvent] = (camEvent == MOTION_DETECT) ? camConfig.motionReDetectionDelay : 1;
                }
                else
                {
                    evPoll[camIndex].eventStatusNotify[camEvent] = EVENT_SEND_NW_FILE;
                    evPoll[camIndex].redetDelay[camEvent] = CAM_EVENT_DEBOUNCE_TIME;
                }

                switch(camEvent)
                {
                    case LINE_CROSS:
                    case LOITERING:
                    case OBJECT_COUNTING:
                    case OBJECT_INTRUSION:
                    {
                        /* Note: These events don't have logic of ACTIVE/NORMAL in general. Once activated,
                         * we will not receive any notification to normalised the state. We have to start
                         * timer after which we declare event as normal, so that configured actions can be stopped. */
                        startEvtTimer(camIndex, camEvent, evPoll[camIndex].redetDelay[camEvent]);
                        DPRINT(CAM_EVENT, "event active, inactive wait timer started: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
                        break;
                    }

                    default:
                    {
                        /* Nothing to do for other events */
                    }
                    break;
                }

                DPRINT(CAM_EVENT, "event status active: [camera=%d], [event=%s], [logEvent=%d]", camIndex, cameraEvtStr[camEvent], sendEvent);
            }
        }
        else
        {
            /* If status is INACTIVE due to camera config disabled then declare normal immediately */
            sendEvent = FALSE;
            if ((camConfig.camera == ENABLE) && (GetCamEventStatus(camIndex, CONNECTION_FAILURE) == ACTIVE))
            {
                startEvtTimer(camIndex, camEvent, evPoll[camIndex].redetDelay[camEvent]);
                DPRINT(CAM_EVENT, "event inactive, post wait timer started: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
            }
            else
            {
                declareEvtNormal(((UINT32)(camIndex << 16) | camEvent));
                DPRINT(CAM_EVENT, "camera disabled/disconnected, make event normal: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
            }
        }
    }

    if (ACTIVE == status)
    {
        switch(camEvent)
        {
            case RECORDING_START:
                sendEvent = FALSE;
                if (evPoll[camIndex].evtTimerHandle[camEvent] == INVALID_TIMER_HANDLE)
                {
                    CameraEventNotify(camIndex, camEvent, eventStatus);
                }
                evPoll[camIndex].redetDelay[camEvent] = CAM_EVENT_DEBOUNCE_TIME_ONE_ACTION;
                startEvtTimer(camIndex, camEvent,evPoll[camIndex].redetDelay[camEvent]);
                break;

            case OBJECT_COUNTING:
                if(CameraType(camIndex) == AUTO_ADDED_CAMERA)
                {
                    startEvtTimer(camIndex, camEvent, CAM_EVENT_DEBOUNCE_TIME);
                }
                break;

            default:
                break;
        }
    }

    if(sendEvent == TRUE)
    {
        switch(evPoll[camIndex].eventStatusNotify[camEvent])
        {
            case EVENT_SEND_NW_ONLY:
                snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
                SendNwEvent(LOG_CAMERA_EVENT, eventSubType, detail, status);
                break;

            case EVENT_SEND_NW_FILE:
                snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
                WriteEvent(LOG_CAMERA_EVENT, eventSubType, detail, NULL, status);
                break;

            default:
                break;
        }

        CameraEventNotify(camIndex, camEvent, eventStatus);
        switch(camEvent)
        {
            case CONNECTION_FAILURE:
            case CAMERA_ONLINE:
                evPoll[camIndex].redetDelay[camEvent] = CAM_EVENT_DEBOUNCE_TIME_ONE_ACTION;
                evPoll[camIndex].eventStatusNotify[camEvent] = MAX_EVENT_SEND;
                startEvtTimer(camIndex, camEvent, evPoll[camIndex].redetDelay[camEvent]);
                DPRINT(CAM_EVENT, "event debounce post wait timer started: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
                break;

            default:
                break;
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Make all camera events inactive due to camera offline
 * @param   camIndex
 */
void MakeAllCameraEventInactive(UINT8 camIndex)
{
    CAMERA_EVENT_e cameraEvent;

    /* Validate input parameters */
    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAM_EVENT, "invld camera index: [camera=%d]", camIndex);
        return;
    }

    /* When camera goes offline, we will make all camera events inactive if it currently in active state. */
    for (cameraEvent = MOTION_DETECT; cameraEvent < MAX_CAMERA_EVENT; cameraEvent++)
    {
        /* Skip all internal events to perform on camera offline */
        if ((cameraEvent == CONNECTION_FAILURE) || (cameraEvent == RECORDING_FAIL) || (cameraEvent == RECORDING_START) || (cameraEvent == CAMERA_ONLINE))
        {
            continue;
        }

        /* Check current event status */
        MUTEX_LOCK(evPoll[camIndex].evtStatusLock);
        if (evPoll[camIndex].evRes.eventStatus[cameraEvent] == INACTIVE)
        {
            /* Nothing to do beacuse event is inactive */
            MUTEX_UNLOCK(evPoll[camIndex].evtStatusLock);
            continue;
        }
        MUTEX_UNLOCK(evPoll[camIndex].evtStatusLock);

        /* Make event inactive forcefully due to camera offline */
        DPRINT(CAM_EVENT, "make event inactive forcefully: [camera=%d], [event=%s]", camIndex, cameraEvtStr[cameraEvent]);
        StopCameraEventPoll(camIndex, cameraEvent);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API starts debounce timer for an event.
 * @param   camIndex
 * @param   camEvent
 * @param   startTime
 * @return
 */
static BOOL startEvtTimer(UINT8 camIndex, CAMERA_EVENT_e camEvent, UINT32 startTime)
{
    TIMER_INFO_t evtTimerInfo;

    if (evPoll[camIndex].evtTimerHandle[camEvent] != INVALID_TIMER_HANDLE)
    {
        return SUCCESS;
    }

    evtTimerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(startTime);
    evtTimerInfo.funcPtr = declareEvtNormal;
    evtTimerInfo.data = ((UINT32)(camIndex << 16) | camEvent);
    if((StartTimer(evtTimerInfo, &evPoll[camIndex].evtTimerHandle[camEvent])) == FAIL)
    {
        EPRINT(CAM_EVENT, "fail to start timer: [camera=%d], [event=%s]", camIndex, cameraEvtStr[camEvent]);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback of the debounce timer. It will delete the timer and Notify for the event normalize.
 * @param   data
 */
static void declareEvtNormal(UINT32 data)
{
    UINT16 					cameraIndex = (data >> 16);
    CAMERA_EVENT_e			camEvent = (data & 0x0000FFFF);
    CHAR					detail[MAX_EVENT_DETAIL_SIZE];
    LOG_EVENT_SUBTYPE_e		eventSubType = LOG_MAX_CAMERA_EVENT;

    if ((cameraIndex >= MAX_CAMERA) || (camEvent >= MAX_CAMERA_EVENT))
    {
        return;
    }

    DeleteTimer(&evPoll[cameraIndex].evtTimerHandle[camEvent]);

    if (camEvent == RECORDING_START)
    {
        evPoll[cameraIndex].eventStatusNotify[camEvent] = MAX_EVENT_SEND;
    }
    else
    {
        eventSubType = getEvtLogSubType(camEvent);
    }

    MUTEX_LOCK(evPoll[cameraIndex].evtStatusLock);
    evPoll[cameraIndex].evRes.eventStatus[camEvent] = INACTIVE;
    evPoll[cameraIndex].evRes.eventHealthStatus[camEvent] = OFF;
    MUTEX_UNLOCK(evPoll[cameraIndex].evtStatusLock);

    switch(evPoll[cameraIndex].eventStatusNotify[camEvent])
    {
        case EVENT_SEND_NW_ONLY:
            snprintf(detail,MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
            SendNwEvent(LOG_CAMERA_EVENT, eventSubType, detail, EVENT_STOP);
            break;

        case EVENT_SEND_NW_FILE:
            snprintf(detail,MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
            WriteEvent(LOG_CAMERA_EVENT, eventSubType, detail, NULL, EVENT_STOP);
            break;

        default:
            break;
    }

    CameraEventNotify(cameraIndex, camEvent, INACTIVE);
    DPRINT(CAM_EVENT, "event inactive: [camera=%d], [event=%s]", cameraIndex, cameraEvtStr[camEvent]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API returns the current event state of the specified camera
 * @param   cameraIndex
 * @param   eventIndex
 * @return
 */
BOOL GetCamEventStatus(UINT8 cameraIndex, UINT8 eventIndex)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return OFF;
    }

    MUTEX_LOCK(evPoll[cameraIndex].evtStatusLock);
    BOOL retVal = evPoll[cameraIndex].evRes.eventHealthStatus[eventIndex];
    MUTEX_UNLOCK(evPoll[cameraIndex].evtStatusLock);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API returns the event health status for motion & no motion event.
 *          Health status for motion & no motion event is common. It return true
 *          if any of the event (Motion/No Motion) is active.
 * @param   cameraIndex
 * @param   eventIndex
 * @return
 */
BOOL GetCamEventMotionHealthSts(UINT8 cameraIndex, UINT8 eventIndex)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return OFF;
    }

    if ((MOTION_DETECT != eventIndex) && (NO_MOTION_DETECTION != eventIndex))
    {
        return OFF;
    }

    MUTEX_LOCK(evPoll[cameraIndex].evtStatusLock);
    BOOL retVal = (evPoll[cameraIndex].evRes.eventHealthStatus[MOTION_DETECT]) || (evPoll[cameraIndex].evRes.eventHealthStatus[NO_MOTION_DETECTION]);
    MUTEX_UNLOCK(evPoll[cameraIndex].evtStatusLock);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   make particular camera event flag OFF
 * @param   camIndex
 * @param   camEvent
 */
static void makeCamEventOff(UINT8 camIndex, CAMERA_EVENT_e camEvent)
{
    if (camIndex >= MAX_CAMERA)
    {
        return;
    }

    MUTEX_LOCK(evPoll[camIndex].evtPollStatusLock);
    evPoll[camIndex].eventPollStatusFlag[camEvent] = OFF;
    MUTEX_UNLOCK(evPoll[camIndex].evtPollStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function provides the status of event request whether it is on or not.
 * @param   evReqInfo
 * @param   camEvent
 * @return  SUCCESS/FAIL
 */
static BOOL checkEvReqStatus(EVENT_REQ_INFO_t *evReqInfo, CAMERA_EVENT_e camEvent)
{
    if((evReqInfo->evRespInfo.evRespType == EV_RESP_MULTI_PART) && (evReqInfo->evPollReq.handle[camEvent] != INVALID_HTTP_HANDLE))
    {
        return ON;
    }

    /* If Single Part or ONVIF */
    MUTEX_LOCK(evReqInfo->evDet[camEvent].evStatusLock);
    BOOL status = evReqInfo->evDet[camEvent].evtStatus;
    MUTEX_UNLOCK(evReqInfo->evDet[camEvent].evStatusLock);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends commands to the camera over RTSP or HTTP whichever is selected.
 * @param   camIndex
 * @param   ipCamCfg
 * @return  Status
 */
static NET_CMD_STATUS_e sendEvReqToCam(UINT8 camIndex, IP_CAMERA_CONFIG_t *ipCamCfg)
{
    UINT32					userData;
    CAM_REQUEST_TYPE_e 		requestType;
    CAMERA_EVENT_REQUEST_t 	*request;
    HTTP_INFO_t 			httpInfo;

    DPRINT(CAM_EVENT, "event polling start req: [camera=%d]", camIndex);
    if ((GetCamIpAddress(camIndex, httpInfo.ipAddress) == INACTIVE) || (httpInfo.ipAddress[0] == '\0'))
    {
        EPRINT(CAM_EVENT, "camera not reachable: [camera=%d]", camIndex);
        return CMD_CAM_DISCONNECTED;
    }

    request = &evRqInfo[camIndex].evPollReq;
    requestType = request->url[request->requestCount].requestType;
    snprintf(httpInfo.httpUsrPwd.username,MAX_CAMERA_USERNAME_WIDTH,"%s", ipCamCfg->username);
    snprintf(httpInfo.httpUsrPwd.password,MAX_CAMERA_PASSWORD_WIDTH,"%s", ipCamCfg->password);
    httpInfo.authMethod = request->url[request->requestCount].authMethod;
    httpInfo.userAgent = CURL_USER_AGENT;
    httpInfo.interface = MAX_HTTP_INTERFACE;
    httpInfo.port = ipCamCfg->httpPort;
    httpInfo.maxConnTime = HTTP_CONNECTION_TIMEOUT;
    httpInfo.maxFrameTime = HTTP_FRAME_TIMEOUT;
    snprintf(httpInfo.relativeUrl,MAX_RELATIVE_URL_WIDTH,"%s", request->url[request->requestCount].relativeUrl);

    DPRINT(CAM_EVENT, "evt req info: [camera=%d], [cnt=%d], [type=%d], [url=http://%s:%d%s]", camIndex, request->requestCount,
           request->url[request-> requestCount].requestType, httpInfo.ipAddress, httpInfo.port, request->url[request->requestCount].relativeUrl);

    userData = ((camIndex << 16) | request->requestCount);
    if(StartHttp(request->url[request->requestCount].httpRequestType, &httpInfo,
                 request->httpCallback[requestType], userData, &request->handle[request->requestCount]) == FAIL)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   httpEvRequestCb
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpEvRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8 				cameraIndex, requestCount;
    NET_CMD_STATUS_e	retVal;
    EVENT_RESULT_t 		evResLoc;
    CAMERA_EVENT_e		camEvent;
    CAMERA_BRAND_e		brand;
    CAMERA_MODEL_e		model;
    PARSER_FUNC			*parseFunc;

    if (dataInfo == NULL)
    {
        return;
    }

    cameraIndex = (UINT8)(dataInfo->userData >> 16);
    requestCount = (UINT8)(dataInfo->userData & 0xFFFF);
    if(cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            /* If data received, parse it and update the event status */
            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                EPRINT(CAM_EVENT, "event status query resp with no data from server: [camera=%d], [handle=%d]", cameraIndex, httpHandle);
                break;
            }

            GetCameraBrandModelNumber(cameraIndex, &brand, &model);
            parseFunc = ParseFunc(brand);
            if(parseFunc == NULL)
            {
                break;
            }

            /* Retrive multiple event response from data. Make all event to unknown state */
            for(camEvent = 0; camEvent < MAX_CAMERA_EVENT; camEvent++)
            {
                evResLoc.eventStatus[camEvent] = UNKNOWN;
            }

            retVal = (*parseFunc)(model, dataInfo->frameSize, dataInfo->storagePtr, (VOIDPTR)(&evResLoc));
            if(retVal != CMD_SUCCESS)
            {
                break;
            }

            for(camEvent = 0; camEvent < MAX_CAMERA_EVENT; camEvent++)
            {
                if(evResLoc.eventStatus[camEvent] != UNKNOWN)
                {
                    EventDetectFunc(cameraIndex, camEvent, evResLoc.eventStatus[camEvent]);
                }
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            evRqInfo[cameraIndex].evPollReq.handle[requestCount] = INVALID_HTTP_HANDLE;
            evRqInfo[cameraIndex].evPollReq.url[requestCount].requestType = MAX_CAM_REQ_TYPE;
            declareEvtInactive(cameraIndex, (CAMERA_EVENT_e)requestCount);
            DPRINT(CAM_EVENT, "request close with success: [camera=%d], [cnt=%d]", cameraIndex, requestCount);
        }
        break;

        default:
        case HTTP_ERROR :
        case HTTP_CLOSE_ON_ERROR:
        {
            MUTEX_LOCK(evRqInfo[cameraIndex].evDet[requestCount].evStatusLock);
            evRqInfo[cameraIndex].evDet[requestCount].evtStatus = OFF;
            MUTEX_UNLOCK(evRqInfo[cameraIndex].evDet[requestCount].evStatusLock);

            evRqInfo[cameraIndex].evPollReq.handle[requestCount] = INVALID_HTTP_HANDLE;
            evRqInfo[cameraIndex].evPollReq.url[requestCount].requestType = MAX_CAM_REQ_TYPE;
            declareEvtInactive(cameraIndex, (CAMERA_EVENT_e)requestCount);
            EPRINT(CAM_EVENT, "request returned with error: [camera=%d], [cnt=%d], [resp=%d]", cameraIndex, requestCount, dataInfo->httpResponse);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will declare all the events which depends on the given event and the event itself to normal.
 * @param   cameraIndex
 * @param   camEvent
 */
static void declareEvtInactive(UINT8 cameraIndex, CAMERA_EVENT_e camEvent)
{
    CAMERA_EVENT_e eventCnt, startCamEvent = camEvent, endCamEvent = camEvent;

    /* All the status were being received from one URL response. hence, declare all events normal */
    if (camEvent == MAX_CAMERA_EVENT)
    {
        startCamEvent = MOTION_DETECT;
        endCamEvent = MAX_CAMERA_EVENT - 1;
    }
    else
    {
        startCamEvent = camEvent;
        endCamEvent = camEvent;
    }

    DPRINT(CAM_EVENT, "event inactive: [camera=%d], [start=%d], [end=%d]", cameraIndex, startCamEvent, endCamEvent);
    for(eventCnt = startCamEvent; eventCnt <= endCamEvent; eventCnt++)
    {
        /* Skip all internal events to set inactive */
        if ((eventCnt == CONNECTION_FAILURE) || (eventCnt == RECORDING_FAIL) || (eventCnt == RECORDING_START) || (eventCnt == CAMERA_ONLINE))
        {
            continue;
        }

        EventDetectFunc(cameraIndex, eventCnt, INACTIVE);
        makeCamEventOff(cameraIndex, eventCnt);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will send event relatedONVIF request to the camera
 * @param   onvifReqParaPtr
 * @param   ipCamCfg
 * @return  NET_CMD_STATUS_e
 */
static NET_CMD_STATUS_e sendOnvifEventCommand(ONVIF_REQ_PARA_t *onvifReqParaPtr, IP_CAMERA_CONFIG_t *ipCamCfg)
{
    ONVIF_HANDLE    handle;
    BOOL            connState;

    connState = GetCamIpAddress(onvifReqParaPtr->camIndex, onvifReqParaPtr->camPara.ipAddr);
    if ((connState == INACTIVE) || (onvifReqParaPtr->camPara.ipAddr[0] == '\0'))
    {
        EPRINT(CAM_EVENT, "camera not reachable: [camera=%d], [onvifReq=%d]", onvifReqParaPtr->camIndex, onvifReqParaPtr->onvifReq);
        return CMD_CAM_DISCONNECTED;
    }

    onvifReqParaPtr->camPara.port = ipCamCfg->onvifPort;
    snprintf(onvifReqParaPtr->camPara.name,MAX_CAMERA_USERNAME_WIDTH,"%s", ipCamCfg->username);
    snprintf(onvifReqParaPtr->camPara.pwd,MAX_CAMERA_PASSWORD_WIDTH,"%s", ipCamCfg->password);

    /* Start Detached ONVIF client thread to send ONVIF command to camera. */
    if (StartOnvifClient(onvifReqParaPtr, &handle) == FAIL)
    {
        EPRINT(CAM_EVENT, "failed to start onvif client: [camera=%d], [onvifReq=%d]", onvifReqParaPtr->camIndex, onvifReqParaPtr->onvifReq);
        return CMD_RESOURCE_LIMIT;
    }

    DPRINT(CAM_EVENT, "sending onvif request: [camera=%d], [onvifReq=%s], [onvifHandle=%d]",
           onvifReqParaPtr->camIndex, GetOnvifReqName(onvifReqParaPtr->onvifReq), handle);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function of ONVIF_START_EVENT Command.
 *          It will send ONVIF_GET_EVENT_NOTIFICATION to request avaiable events.
 *          ONVIF client thread will process the request, after receiving the respones
 *          event status will be parsed and getEvtNotificationCb will be called as callback.
 * @param   responseData
 * @return
 */
static BOOL onvifStartEventCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    CAMERA_EVENT_e 		camEvent;
    ONVIF_REQ_PARA_t	onvifReq;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    DPRINT(CAM_EVENT, "start onvif command status: [camera=%d], [response=%d], [TerminationTime=%dsec]",
           responseData->cameraIndex, responseData->response, *((UINT32PTR)responseData->data));

    do
    {
        if (responseData->response != ONVIF_CMD_SUCCESS)
        {
            break;
        }

        /* Initialize counts related renew request/ onvif response cnt */
        evRqInfo[responseData->cameraIndex].onvifRenewReqCnt = 0;
        for(camEvent = 0; camEvent < MAX_CAMERA_EVENT; camEvent++)
        {
            evRqInfo[responseData->cameraIndex].onvifEvtRespCnt[camEvent] = EVENT_NOT_ARRIVED;
        }

        /* If polling is ON then only send getEventNotification command */
        camEvent = MAX_CAMERA_EVENT;
        MUTEX_LOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
        if(evRqInfo[responseData->cameraIndex].evDet[camEvent].evtStatus == OFF)
        {
            MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
            break;
        }
        MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);

        ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
        onvifReq.onvifReq = ONVIF_GET_EVENT_NOTIFICATION;
        onvifReq.camIndex = responseData->cameraIndex;
        onvifReq.onvifCallback = getEvtNotificationCb;

        /* send ONVIF_GET_EVENT_NOTIFICATION command to camera. */
        if (CMD_SUCCESS != sendOnvifEventCommand(&onvifReq, &ipCamCfg))
        {
            break;
        }

        /* Successfully sent get event onvif command */
        return SUCCESS;

    }while(0);

    camEvent = MAX_CAMERA_EVENT;

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
    if(evRqInfo[responseData->cameraIndex].evDet[camEvent].evtStatus == ON)
    {
        evRqInfo[responseData->cameraIndex].evDet[camEvent].evtStatus = OFF;
        MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
        declareEvtInactive(responseData->cameraIndex, camEvent);
    }
    else
    {
        MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is the callback function for ONVIF_GET_EVENT_NOTIFICATION command. When response
 *          is received events status will be parsed from the XML and local data will be set accordingly.
 * @param   responseData
 * @return
 */
static BOOL getEvtNotificationCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    BOOL 			 	*evtStatus;
    CAMERA_EVENT_e	 	camEvent;
    ONVIF_REQ_PARA_t 	onvifReq;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    TIMER_INFO_t	 	getEvNotifyTimer;

    if (responseData->cameraIndex >= MAX_CAMERA)
    {
        EPRINT(CAM_EVENT, "invld camera index: [camera=%d]", responseData->cameraIndex);
        return FAIL;
    }

    do
    {
        if (responseData->response != ONVIF_CMD_SUCCESS)
        {
            break;
        }

        evtStatus = (BOOL*)(responseData->data);
        if (evtStatus != NULL)
        {
            for(camEvent = 0; camEvent < MAX_CAMERA_EVENT; camEvent++)
            {
                /* Note: If count is considerd for LINE_CROSSING and OBEJCT_COUNTING then following logic needs to be alterd.
                 * As evtStatus[] is of boolean type. */
                if(evtStatus[camEvent] != UNKNOWN)
                {
                    /* After getting event status through ONVIF, forward it to EventDetectFunc()*/
                    EventDetectFunc(responseData->cameraIndex, camEvent, evtStatus[camEvent]);
                    evRqInfo[responseData->cameraIndex].onvifEvtRespCnt[camEvent] = 0;
                }
                else if(evRqInfo[responseData->cameraIndex].onvifEvtRespCnt[camEvent] != EVENT_NOT_ARRIVED)
                {
                    evRqInfo[responseData->cameraIndex].onvifEvtRespCnt[camEvent]++;
                    if(evRqInfo[responseData->cameraIndex].onvifEvtRespCnt[camEvent] >= MAX_EVENT_WAIT_CNT)
                    {
                        /* change event status to INACTIVE */
                        EventDetectFunc(responseData->cameraIndex, camEvent, INACTIVE);
                        evRqInfo[responseData->cameraIndex].onvifEvtRespCnt[camEvent] = EVENT_NOT_ARRIVED;
                    }
                }
            }
        }

        /* If event polling is ON then only start timer */
        camEvent = MAX_CAMERA_EVENT;
        MUTEX_LOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
        if(evRqInfo[responseData->cameraIndex].evDet[camEvent].evtStatus == OFF)
        {
            MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
            break;
        }
        MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);

        evRqInfo[responseData->cameraIndex].onvifRenewReqCnt++;
        if(evRqInfo[responseData->cameraIndex].onvifRenewReqCnt >= MAX_RENEW_CNT)
        {
            ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
            onvifReq.onvifReq = ONVIF_RENEW_EVENT_REQ;
            onvifReq.camIndex = responseData->cameraIndex;
            onvifReq.onvifCallback = onvifEvtRenewCb;

            /* Send ONVIF_RENEW_EVENT_REQ */
            if(sendOnvifEventCommand(&onvifReq, &ipCamCfg) == CMD_SUCCESS)
            {
                evRqInfo[responseData->cameraIndex].onvifRenewReqCnt = 0;
            }
        }

        /* Keep Polling for ONVIF_GET_EVENT_NOTIFICATION at every 2 second interval */
        getEvNotifyTimer.count = CONVERT_SEC_TO_TIMER_COUNT(EV_REQ_TIME_OUT);
        getEvNotifyTimer.data = (UINT32)(responseData->cameraIndex);
        getEvNotifyTimer.funcPtr = getOnvifEvtNotifyTmrCb;

        if (evRqInfo[responseData->cameraIndex].onvifEvtTmrHandle != INVALID_TIMER_HANDLE)
        {
            EPRINT(CAM_EVENT, "camera event timer is already running: [camera=%d]", responseData->cameraIndex);
            break;
        }

        if (FAIL == StartTimer(getEvNotifyTimer, &evRqInfo[responseData->cameraIndex].onvifEvtTmrHandle))
        {
            EPRINT(CAM_EVENT, "fail to start camera event timer: [camera=%d]", responseData->cameraIndex);
            break;
        }

        return SUCCESS;

    }while(0);

    /* If event notify timer is not started unsubscribe event and change status to INACTIVE */
    camEvent = MAX_CAMERA_EVENT;

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);
    if(evRqInfo[responseData->cameraIndex].evDet[camEvent].evtStatus == ON)
    {
        evRqInfo[responseData->cameraIndex].evDet[camEvent].evtStatus = OFF;
    }
    MUTEX_UNLOCK(evRqInfo[responseData->cameraIndex].evDet[camEvent].evStatusLock);

    declareEvtInactive(responseData->cameraIndex, camEvent);
    ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);

    /* Call UnSubscribe command */
    onvifReq.onvifReq = ONVIF_UNSUBSCRIBE_EVENT_REQ;
    onvifReq.camIndex = responseData->cameraIndex;
    onvifReq.onvifCallback = onvifEvtUnsubscribeCb;
    sendOnvifEventCommand(&onvifReq, &ipCamCfg);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function. It will be triggerd after event notify timer expires. Timer
 *          is set for 2 seconds(event polling) once we get response of previous request.
 *          Every 2 second we will send ONVIF_GET_EVENT_NOTIFICATION request after .
 * @param   cameraIndex
 * @return
 */
static void getOnvifEvtNotifyTmrCb(UINT32 cameraIndex)
{
    CAMERA_EVENT_e      camEvent = MAX_CAMERA_EVENT;
    ONVIF_REQ_PARA_t    onvifReq;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    /* send command only if polling is ON */
    MUTEX_LOCK(evRqInfo[cameraIndex].evDet[camEvent].evStatusLock);
    if(evRqInfo[cameraIndex].evDet[camEvent].evtStatus == ON)
    {
        MUTEX_UNLOCK(evRqInfo[cameraIndex].evDet[camEvent].evStatusLock);
        onvifReq.onvifReq = ONVIF_GET_EVENT_NOTIFICATION;
        onvifReq.camIndex = cameraIndex;
        onvifReq.onvifCallback = getEvtNotificationCb;
    }
    else
    {
        MUTEX_UNLOCK(evRqInfo[cameraIndex].evDet[camEvent].evStatusLock);
        declareEvtInactive(cameraIndex, camEvent);
        onvifReq.onvifReq = ONVIF_UNSUBSCRIBE_EVENT_REQ;
        onvifReq.camIndex = cameraIndex;
        onvifReq.onvifCallback = onvifEvtUnsubscribeCb;
    }

    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    switch(sendOnvifEventCommand(&onvifReq, &ipCamCfg))
    {
        case CMD_SUCCESS:
            DeleteTimer(&evRqInfo[cameraIndex].onvifEvtTmrHandle);
            break;

        case CMD_CAM_DISCONNECTED:
            MUTEX_LOCK(evRqInfo[cameraIndex].evDet[camEvent].evStatusLock);
            evRqInfo[cameraIndex].evDet[camEvent].evtStatus = OFF;
            MUTEX_UNLOCK(evRqInfo[cameraIndex].evDet[camEvent].evStatusLock);
            declareEvtInactive(cameraIndex, camEvent);
            DeleteTimer(&evRqInfo[cameraIndex].onvifEvtTmrHandle);
            break;

        case CMD_RESOURCE_LIMIT:
        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Callback function of event UnSubscribe Command
 * @param   responseData
 * @return
 */
static BOOL onvifEvtUnsubscribeCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    DPRINT(CAM_EVENT, "unsubscribe onvif command status: [camera=%d], [response=%d]", responseData->cameraIndex, responseData->response);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Callback function of event renew Command
 * @param   responseData
 * @return
 */
static BOOL onvifEvtRenewCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    DPRINT(CAM_EVENT, "renew onvif command status: [camera=%d], [response=%d], [TerminationTime=%dsec]",
           responseData->cameraIndex, responseData->response, *((UINT32PTR)responseData->data));
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get event log subtype from camera event
 * @param   camEvent
 * @return  Event log subtype
 */
static LOG_EVENT_SUBTYPE_e getEvtLogSubType(CAMERA_EVENT_e camEvent)
{
    switch(camEvent)
    {
        case LINE_CROSS:
            return LOG_LINE_CROSS;

        case OBJECT_INTRUSION:
            return LOG_OBJECT_INTRUTION;

        case AUDIO_EXCEPTION:
            return LOG_AUDIO_EXCEPTION;

        case MISSING_OBJECT:
            return LOG_MISSING_OBJECT;

        case SUSPICIOUS_OBJECT:
            return LOG_SUSPICIOUS_OBJECT;

        case LOITERING:
            return LOG_LOITERING;

        case OBJECT_COUNTING:
            return LOG_OBJECT_COUNTING;

        case NO_MOTION_DETECTION:
            return LOG_NO_MOTION_DETECTION;

        default:
            return (camEvent + 1);
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
