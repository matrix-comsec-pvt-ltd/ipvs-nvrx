//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		PtzTour.c
@brief      PTZ module provides API to start and stop manual PTZ tour as configured for a camera.
            It provides timer which runs periodically checking scheduled time period for PTZ tour
            of all the cameras. If any PTZ tour of a camera matches at the instance to start the
            configured tour, it starts schedule PTZ tour with the configured tour patter and configured
            preset positions and view time of the tour. In the same way,if any PTZ tour for a camera
            found to end the tour at the instance, it ends scheduled tour started. The preset position
            is set for a camera according to the configured tour pattern for on ongoing tour in a timer.
            The above timer is available for each camera. Therefore there are MAX_CAMERA number such timers.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "PtzTour.h"
#include "DebugLog.h"
#include "Utils.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define NO_PTZ_TOUR				0
#define EMPTY_POSITION			0
#define MIN_PRESET_POSITION     2
#define SUSPEND_INTERVAL		300		//5 mins=300secs
#define TOTAL_SEC_IN_MIN		60
#define INVALID_PTZ_POS			255

//#################################################################################################
// @DATA TYPES
//#################################################################################################
//Possible Zigzag Pattern
typedef enum
{
    ZIGZAG_FWD = 0,
    ZIGZAG_BWD,
    ZIGZAG_NONE
} ZIGZAG_e;

//Possible types of a tour
typedef enum
{
    PTZ_TOUR_SCH_1 = 0,
    PTZ_TOUR_SCH_2,
    PTZ_TOUR_ENTIRE_DAY,
    PTZ_TOUR_MANUAL,
    PTZ_TOUR_IDLE
} TOUR_TYPE_e;

//Relevant PTZ Tour Information Structure
typedef struct
{
    BOOL 			isSuspended;						//Whether Tour is suspended
    UINT8 			currTour;							//current Tour activated for a camera
    UINT8 			currPositionIndex;					//current index no. of Preset Position
    UINT8 			totConfPos;							//Cam Tour's Total active Preset Positions
    ZIGZAG_e 		currZigZagDir;						//current Tour Zigzag's Dir FWD/BWD
    TOUR_TYPE_e 	currTourType;						//current type of a tour
    UINT32			randomSeed;							//Seed value for random no
    TIMER_HANDLE	ptzTourTmrHandle;					//Tour timer handle
    ORDER_NUMBER_t 	activePositions[MAX_ORDER_COUNT];	//CamTour active PresetPositions array
    pthread_mutex_t	tourInfoMutex;						//Mutex
    BOOL 			isTourPaused;						//Whether Tour is pause
    BOOL 			lastTourState;						//last state check for the call back
} CAM_TOUR_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL checkSchPtzTour(UINT32 arg);
//-------------------------------------------------------------------------------------------------
static BOOL verifyPtzPosition(UINT8 presetTour, PRESET_TOUR_CONFIG_t * tourCfgPtr);
//-------------------------------------------------------------------------------------------------
static BOOL canTourBeStarted(UINT8 camId, UINT8 presetTour, PRESET_TOUR_CONFIG_t * tourCfgPtr);
//-------------------------------------------------------------------------------------------------
static void initCurrCamTourInfo(UINT8 camId, UINT8 tourNo, TOUR_TYPE_e tourType, PRESET_TOUR_CONFIG_t * tourCfgPtr);
//-------------------------------------------------------------------------------------------------
static void initCurrOrderArray(UINT8 camId, UINT8 tourNo, PRESET_TOUR_CONFIG_t * tourCfgPtr);
//-------------------------------------------------------------------------------------------------
static UINT8 generateRandomNo(UINT32PTR no);
//-------------------------------------------------------------------------------------------------
static void runPtzTour(UINT32 camId);
//-------------------------------------------------------------------------------------------------
static void reloadPtzTimer(UINT16 interval, UINT8 ptzTmrNo);
//-------------------------------------------------------------------------------------------------
static void startSchPtzTour(UINT8 camId, UINT8 presetTour, TOUR_TYPE_e tourType, PRESET_TOUR_CONFIG_t * tourCfgPtr);
//-------------------------------------------------------------------------------------------------
static void stopPtzTour(UINT8 camId, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static CAM_TOUR_INFO_t  currCamTourInfo[MAX_CAMERA];

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes the global variables of the ptz tour. It should be called prior to other APIs.
 */
void InitPtzTour(void)
{
    UINT8 				camNo;
    ONE_MIN_NOTIFY_t	oneMinFun;

    for(camNo = 0; camNo < getMaxCameraForCurrentVariant(); camNo++)
    {
        currCamTourInfo[camNo].isSuspended = FALSE;
        currCamTourInfo[camNo].currZigZagDir = ZIGZAG_NONE;
        currCamTourInfo[camNo].currTourType = PTZ_TOUR_IDLE;
        currCamTourInfo[camNo].currTour = NO_PTZ_TOUR;
        currCamTourInfo[camNo].currPositionIndex = 0;
        currCamTourInfo[camNo].totConfPos = 0;
        currCamTourInfo[camNo].randomSeed = 0;
        currCamTourInfo[camNo].ptzTourTmrHandle = INVALID_TIMER_HANDLE;
        currCamTourInfo[camNo].isTourPaused = FALSE;
        memset(&currCamTourInfo[camNo].activePositions, 0, sizeof(currCamTourInfo[camNo].activePositions));
        MUTEX_INIT(currCamTourInfo[camNo].tourInfoMutex, NULL);
    }

    oneMinFun.funcPtr = checkSchPtzTour;
    oneMinFun.userData = 0;
    if(RegisterOnMinFun(&oneMinFun) != SUCCESS)
    {
        EPRINT(PTZ_TOUR, "fail to register one minute function");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts configured manual PTZ tour for specified camera, if there is no
 *          ongoing schedule tour for the specified camera at the instance or if the above manual
 *          tour is configured to override any existing schedule tour.
 * @param   camId
 * @param   advncDetail
 * @return
 */
NET_CMD_STATUS_e StartManualPtzTour(UINT8 camId, CHARPTR advncDetail)
{
    UINT8					presetCnt = 0;
    UINT8					presetIdx;
    UINT8					tourNo;
    TIMER_INFO_t 			ptzTourTmr;
    CHAR					eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR					eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    PRESET_TOUR_CONFIG_t	tourCfg;

    ReadSinglePresetTourConfig(camId, &tourCfg);
    if ((tourCfg.manualTour == NO_PTZ_TOUR) || (tourCfg.manualTour > MAX_TOUR_NUMBER))   //if user has a configured PTZ Tour
    {
        return CMD_TOUR_NOT_SET;
    }

    /* Derive configured preset position in current tour */
    tourNo = (tourCfg.manualTour - 1);
    for (presetIdx = 0; presetIdx < MAX_ORDER_COUNT; presetIdx++)
    {
        if (tourCfg.tour[tourNo].ptz[presetIdx].presetPosition != EMPTY_POSITION)
        {
            presetCnt++;
        }
    }

    /* Minimum 2 positions required to perform the tour */
    if (presetCnt < MIN_PRESET_POSITION)
    {
        return CMD_TOUR_NOT_SET;
    }

    //if a tour for the camera specified is already running
    MUTEX_LOCK(currCamTourInfo[camId].tourInfoMutex);
    if(currCamTourInfo[camId].ptzTourTmrHandle != INVALID_TIMER_HANDLE)
    {
        //can we override the running tour
        if(tourCfg.manualTourOverride == TRUE)
        {
            //if user starts the same tour again
            if(currCamTourInfo[camId].currTourType == PTZ_TOUR_MANUAL)
            {
                MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
                return CMD_MANUAL_TOUR_ON;
            }

            snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camId].currTour].name);
            stopPtzTour(camId, eventAdvDetail);
        }
        else
        {
            if(currCamTourInfo[camId].currTourType == PTZ_TOUR_MANUAL)
            {
                MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
                return CMD_MANUAL_TOUR_ON;
            }
            else
            {
                MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
                return CMD_SCHEDULE_TOUR_ON;
            }
        }
    }

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camId));
    snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, MANUAL_TOUR_ADV_DETAIL, advncDetail, tourCfg.tour[tourNo].name);
    WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_MANUAL);

    //initialize PTZ_INFO_t struct. Of camId
    initCurrCamTourInfo(camId, tourNo, PTZ_TOUR_MANUAL, &tourCfg);

    //Start Timer to run PTZ tour
    ptzTourTmr.count = CONVERT_MSEC_TO_TIMER_COUNT(100);
    ptzTourTmr.data = camId;
    ptzTourTmr.funcPtr = runPtzTour;
    StartTimer(ptzTourTmr, &currCamTourInfo[camId].ptzTourTmrHandle);
    DPRINT(PTZ_TOUR, "ptz manual tour started: [camera=%d]", camId);
    MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops the ongoing manual PTZ tour for specified camera.
 * @param   camId
 * @param   advncDetail
 * @return
 */
NET_CMD_STATUS_e StopManualPtzTour(UINT8 camId, CHARPTR advncDetail)
{
    CHAR					eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    PRESET_TOUR_CONFIG_t	tourCfg;

    //if an input camera's tour is running
    MUTEX_LOCK(currCamTourInfo[camId].tourInfoMutex);
    if((currCamTourInfo[camId].ptzTourTmrHandle != INVALID_TIMER_HANDLE) && (currCamTourInfo[camId].currTourType == PTZ_TOUR_MANUAL))
    {
        ReadSinglePresetTourConfig(camId, &tourCfg);
        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, MANUAL_TOUR_ADV_DETAIL, advncDetail, tourCfg.tour[currCamTourInfo[camId].currTour].name);
        stopPtzTour(camId, eventAdvDetail);
        MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
        return CMD_SUCCESS;
    }
    MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
    return CMD_MANUAL_TOUR_OFF;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates  tour configurations's changes made by user within the PTZ local working copy.
 * @param   camIndex
 * @param   newTourCfg
 * @param   oldPresetPtr
 */
void UpdateTourCfg(UINT8 camIndex, PRESET_TOUR_CONFIG_t newTourCfg, PRESET_TOUR_CONFIG_t * oldPresetPtr)
{
    UINT8 	tourIndex;
    CHAR	eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    //if the camera's tour is running
    MUTEX_LOCK(currCamTourInfo[camIndex].tourInfoMutex);
    if(currCamTourInfo[camIndex].ptzTourTmrHandle == INVALID_TIMER_HANDLE)
    {
        MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
        return;
    }

    if((newTourCfg.manualTour != oldPresetPtr->manualTour) && (currCamTourInfo[camIndex].currTourType == PTZ_TOUR_MANUAL))
    {
        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, oldPresetPtr->tour[currCamTourInfo[camIndex].currTour].name);
        stopPtzTour(camIndex, eventAdvDetail);
        MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
        return;
    }

    //find tour no whose Cfg are changed
    for(tourIndex = 0; tourIndex < MAX_TOUR_NUMBER; tourIndex++)
    {
        if(memcmp(&newTourCfg.tour[tourIndex], &oldPresetPtr->tour[tourIndex], sizeof(TOUR_t)) == STATUS_OK)
        {
            continue;
        }

        if(currCamTourInfo[camIndex].currTour != tourIndex)
        {
            continue;
        }

        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, oldPresetPtr->tour[currCamTourInfo[camIndex].currTour].name);
        stopPtzTour(camIndex, eventAdvDetail);
        break;
    }
    MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates  tour schedule configurations's changes made by user within the
 *          PTZTour local working copy.
 * @param   camIndex
 * @param   newTourSchCfg
 * @param   oldSchedule
 * @note    If PTZ sch tour is not running & we have to start sch. acc. to new Cfg, It is taken care
 *          by checkSchPtzTourTimer
 */
void UpdateTourScheduleCfg(UINT8 camIndex, TOUR_SCHEDULE_CONFIG_t newTourSchCfg, TOUR_SCHEDULE_CONFIG_t * oldSchedule)
{
    struct tm               currTime = { 0 };
    CHAR                    eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    PRESET_TOUR_CONFIG_t	tourCfg;

    //Get the current time from DateTime in broken Format
    GetLocalTimeInBrokenTm(&currTime);

    //find if today's tour Sch Cfg are changed
    if(memcmp(&newTourSchCfg.tour[currTime.tm_wday], &oldSchedule->tour[currTime.tm_wday], sizeof(DAILY_TOUR_SCHEDULE_t)) == STATUS_OK)
    {
        return;
    }

    //if PTZ tour is running
    MUTEX_LOCK(currCamTourInfo[camIndex].tourInfoMutex);
    if(currCamTourInfo[camIndex].ptzTourTmrHandle == INVALID_TIMER_HANDLE)
    {
        MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
        return;
    }

    ReadSinglePresetTourConfig(camIndex, &tourCfg);
    snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
    stopPtzTour(camIndex, eventAdvDetail);
    MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns YES if the camera tour is running otherwise NO.
 * @param   camId
 * @param   dummy
 * @return
 */
BOOL GetCamTourStatus(UINT8 camId, UINT8 dummy)
{
    BOOL tourStatus = PTZ_HLT_NO_TOUR;

    if(camId >= getMaxCameraForCurrentVariant())
    {
        return PTZ_HLT_NO_TOUR;
    }

    MUTEX_LOCK(currCamTourInfo[camId].tourInfoMutex);
    if(currCamTourInfo[camId].isTourPaused == FALSE)
    {
        if(currCamTourInfo[camId].ptzTourTmrHandle != INVALID_TIMER_HANDLE)
        {
            if(currCamTourInfo[camId].currTourType == PTZ_TOUR_MANUAL)
            {
                tourStatus = PTZ_HLT_MAN_TOUR;
            }
            else
            {
                tourStatus = PTZ_HLT_SCH_TOUR;
            }
        }
    }
    else if((currCamTourInfo[camId].currTourType != PTZ_TOUR_MANUAL) && (currCamTourInfo[camId].currTourType != PTZ_TOUR_IDLE))
    {
        tourStatus = PTZ_HLT_PAUS_TOUR;
    }
    MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
    return tourStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is a callback function of the timer within which we periodically check
 *          whether we have to start or stop any of the scheduled tour according to schedule tour configurations.
 * @param   arg
 * @return
 */
static BOOL checkSchPtzTour(UINT32 arg)
{
    UINT8 					camIndex, schNo;
    struct tm 				currTimeStruct = { 0 };
    TIME_HH_MM_t 			currTime;
    TOUR_SCHEDULE_CONFIG_t 	tourSch;
    PRESET_TOUR_CONFIG_t	tourCfg;
    CHAR                    eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    //Get the current time from DateTime in broken Format
    GetLocalTimeInBrokenTm(&currTimeStruct);

    //LOOP to MAX_CAMERA
    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        //Read Camera TourConfig for each camera
        ReadSingleTourScheduleConfig(camIndex, &tourSch);
        ReadSinglePresetTourConfig(camIndex, &tourCfg);

        MUTEX_LOCK(currCamTourInfo[camIndex].tourInfoMutex);
        if((currCamTourInfo[camIndex].currTourType == PTZ_TOUR_IDLE) || (currCamTourInfo[camIndex].currTourType == PTZ_TOUR_MANUAL))
        {
            //If camIndex Tour is scheduled for curr. weekday for entire day
            if(tourSch.tour[currTimeStruct.tm_wday].entireDay == TRUE)
            {
                //find whether this tour can be started
                if(canTourBeStarted(camIndex, tourSch.tour[currTimeStruct.tm_wday].presetTour, &tourCfg) == YES)
                {
                    //start Schedule PTZ Tour of camIndex
                    startSchPtzTour(camIndex, (tourSch.tour[currTimeStruct.tm_wday].presetTour - 1), PTZ_TOUR_ENTIRE_DAY, &tourCfg);
                }
            }
            else    //scheduled Tour & not entire day tour
            {
                for(schNo = 0; schNo < MAX_TOUR_SCHEDULE; schNo++)
                {
                    currTime.hour = currTimeStruct.tm_hour;
                    currTime.minute = currTimeStruct.tm_min;

                    //If current time between start and end time of the schedule tour for a camera
                    if(IsGivenTimeInWindow(currTime, tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.startTime,
                                           tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.endTime) == NO)
                    {
                        continue;
                    }

                    //find whether this tour can be started
                    if(canTourBeStarted(camIndex, tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].presetTour, &tourCfg) == NO)
                    {
                        continue;
                    }

                    //start Schedule PTZ Tour of camIndex
                    startSchPtzTour(camIndex, (tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].presetTour - 1), schNo, &tourCfg);
                }
            }
        }
        else if(currCamTourInfo[camIndex].currTourType == PTZ_TOUR_ENTIRE_DAY)
        {
            if(tourSch.tour[currTimeStruct.tm_wday].entireDay == FALSE)
            {
                snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
                stopPtzTour(camIndex, eventAdvDetail);
            }
        }
        else
        {
            for(schNo = 0; schNo < MAX_TOUR_SCHEDULE; schNo++)	 //scheduled Tour
            {
                currTime.hour = currTimeStruct.tm_hour;
                currTime.minute = currTimeStruct.tm_min;

                if(IsGivenTimeInWindow(currTime,tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.startTime,
                                       tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.endTime) == YES)
                {
                    continue;
                }

                //is the running sch. tour the one we should stop
                if(currCamTourInfo[camIndex].currTourType != schNo)
                {
                    continue;
                }

                snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
                stopPtzTour(camIndex, eventAdvDetail);
            }
        }
        MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks is there any valid ptz position was present.
 * @param   presetTour
 * @param   tourCfgPtr
 * @return
 */
static BOOL verifyPtzPosition(UINT8 presetTour, PRESET_TOUR_CONFIG_t * tourCfgPtr)
{
    UINT8   tourPos;
    UINT8   emptyPos = 0;

    for(tourPos = 0; tourPos < MAX_ORDER_COUNT; tourPos++)
    {
        if(tourCfgPtr->tour[presetTour].ptz[tourPos].presetPosition != EMPTY_POSITION)
        {
            emptyPos++;
        }
    }

    // Min 2 entry required to start tour.
    return (emptyPos >= 2) ? SUCCESS : FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks if there is already running tour corresponding to the input camId.
 *          And if so, is it a manual tour which cannot override scheduled tour.If this condition
 *          is true, it returns YES otherwise NO is returned.
 * @param   camId
 * @param   presetTour
 * @param   tourCfgPtr
 * @return
 */
static BOOL canTourBeStarted(UINT8 camId, UINT8 presetTour, PRESET_TOUR_CONFIG_t * tourCfgPtr)
{
    //If input preset tour exist
    if(presetTour == NO_PTZ_TOUR)
    {
        return NO;
    }

    if(verifyPtzPosition((presetTour - 1), tourCfgPtr) == FAIL)
    {
        EPRINT(PTZ_TOUR, "valid ptz position not present: [camera=%d], [presetTour=%d]", camId, presetTour);
        return NO;
    }

    //If there is a Cam's running PTZ tour at instance
    if(currCamTourInfo[camId].ptzTourTmrHandle == INVALID_TIMER_HANDLE)
    {
        //no PTZ tour corresponding to camId is running
        return YES;
    }

    //If the running tour is Manual Tour
    if (currCamTourInfo[camId].currTourType == PTZ_TOUR_MANUAL)
    {
        //If running manual tour cannot override our schedule tour
        if(tourCfgPtr->manualTourOverride == FALSE)
        {
            return YES;
        }
    }

    return NO;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes the Cam Tour Info structure for a camera.
 * @param   camId
 * @param   tourNo
 * @param   tourType
 * @param   tourCfgPtr
 */
static void initCurrCamTourInfo(UINT8 camId, UINT8 tourNo, TOUR_TYPE_e tourType, PRESET_TOUR_CONFIG_t * tourCfgPtr)
{
    //Init the Cam Tour Struct of camIndex store the current tour number of the camera
    currCamTourInfo[camId].currTour = tourNo;

    //store the camera's current tour type
    currCamTourInfo[camId].currTourType = tourType;

    /* Init the camera's current tour active order array if tour pattern is RANDOM,
     * runPtzTour init tour order array every time tour is ran from 0 to max positions */
    if(tourCfgPtr->tour[tourNo].tourPattern != RANDOM)
    {
        initCurrOrderArray(camId, tourNo, tourCfgPtr);
    }
    currCamTourInfo[camId].currPositionIndex = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stores the preset position configured by the user for a camera in currCamTourInfo
 *          structure in accordance to the tour pattern of the tour. So, when next camera position is set
 *          by runPtzTour Timer, it will be directly accessed from this array sequentially.
 * @param   camId
 * @param   tourNo
 * @param   tourCfgPtr
 */
static void initCurrOrderArray(UINT8 camId, UINT8 tourNo, PRESET_TOUR_CONFIG_t * tourCfgPtr)
{
    UINT8   presetPosCnt = 0, posNum;
    BOOL    isPresetPositionSet[MAX_ORDER_COUNT];

    //if the tour pattern is Looping or Zigzag
    if((tourCfgPtr->tour[tourNo].tourPattern == LOOPING) || (tourCfgPtr->tour[tourNo].tourPattern == ZIGZAG))
    {
        //find & store the existing preset positions from working copy
        for(posNum = 0; posNum < MAX_ORDER_COUNT; posNum++)
        {
            //if preset position exist
            if(tourCfgPtr->tour[tourNo].ptz[posNum].presetPosition !=  EMPTY_POSITION)
            {
                //store preset position & its view time to activeCamPostion array
                currCamTourInfo[camId].activePositions[presetPosCnt].presetPosition = tourCfgPtr->tour[tourNo].ptz[posNum].presetPosition;
                currCamTourInfo[camId].activePositions[presetPosCnt].viewTime = tourCfgPtr->tour[tourNo].ptz[posNum].viewTime;
                presetPosCnt++;
            }
        }

        //Total active Preset Positions of the tour
        currCamTourInfo[camId].totConfPos = presetPosCnt;
        currCamTourInfo[camId].currZigZagDir = ZIGZAG_NONE;
    }
    else    //RANDOM
    {
        //Find total active Preset Positions of the tour
        for(posNum = 0; posNum < MAX_ORDER_COUNT; posNum++)
        {
            //if preset position exist
            isPresetPositionSet[posNum] = FALSE;
            if(tourCfgPtr->tour[tourNo].ptz[posNum].presetPosition !=  EMPTY_POSITION)
            {
                presetPosCnt++;
            }
        }

        //store total existing preset position to totalCamPositions
        currCamTourInfo[camId].totConfPos = presetPosCnt;

        //find & store the existing preset positions from working copy
        presetPosCnt = 0;
        while(presetPosCnt < currCamTourInfo[camId].totConfPos)
        {
            currCamTourInfo[camId].randomSeed++;
            posNum = generateRandomNo(&currCamTourInfo[camId].randomSeed);

            //if we haven't stored the random no generated earlier
            if (TRUE == isPresetPositionSet[posNum])
            {
                continue;
            }

            //preset exist in the index
            if(tourCfgPtr->tour[tourNo].ptz[posNum].presetPosition == EMPTY_POSITION)
            {
                continue;
            }

            currCamTourInfo[camId].activePositions[presetPosCnt].presetPosition = tourCfgPtr->tour[tourNo].ptz[posNum].presetPosition;
            currCamTourInfo[camId].activePositions[presetPosCnt].viewTime = tourCfgPtr->tour[tourNo].ptz[posNum].viewTime;

            //Set relevant bit to indicate that random number generated for position
            isPresetPositionSet[posNum] = TRUE;
            presetPosCnt++;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates the random number which in range of 1 to MAXIMUM PREST POSITION
 * @param   no
 * @return
 */
static UINT8 generateRandomNo(UINT32PTR no)
{
    //generate random no. between 0 to MAX_PRESET_POSTION
    UINT32 tmpNo = rand_r(no);
    return (UINT8)(tmpNo % MAX_ORDER_COUNT);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function of the timer to run the PTZ tour of the camera number
 *          (input param) as per to the configured preset position and view time of the tour timer
 *          stored in current Cam Tour structure.
 * @param   camId
 */
static void runPtzTour(UINT32 camId)
{
    UINT32 					currReloadInterval = 0;
    UINT8 					curPtzPos, pauseInterval;
    PRESET_TOUR_CONFIG_t	tourCfg;
    CHAR                    detail[MAX_EVENT_DETAIL_SIZE];
    CHAR                    advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    MUTEX_LOCK(currCamTourInfo[camId].tourInfoMutex);
    if(currCamTourInfo[camId].isTourPaused == TRUE)
    {
        DPRINT(PTZ_TOUR, "tour pause: [camera=%d], [position=%u]", camId, currCamTourInfo[camId].currPositionIndex);
        MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
        return;
    }

    if(currCamTourInfo[camId].isSuspended == TRUE)
    {
        //reload tour with Suspend Interval
        reloadPtzTimer(SUSPEND_INTERVAL, camId);

        //reset the isSuspended flag to FALSE
        currCamTourInfo[camId].isSuspended = FALSE;
        MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
        return;
    }

    MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);
    ReadSinglePresetTourConfig(camId, &tourCfg);
    MUTEX_LOCK(currCamTourInfo[camId].tourInfoMutex);

    //if we are at the start of the tour (1st time or after pause time)
    if(currCamTourInfo[camId].currPositionIndex == 0)
    {
        if(tourCfg.tour[currCamTourInfo[camId].currTour].tourPattern == ZIGZAG)
        {
            if(currCamTourInfo[camId].currZigZagDir == ZIGZAG_NONE)
            {
                //we will proceed forward in the ZIGZAG tour
                pauseInterval = 0;
            }
            else
            {
                pauseInterval = tourCfg.tour[currCamTourInfo[camId].currTour].pauseBetweenTour;
            }
            currCamTourInfo[camId].currZigZagDir = ZIGZAG_FWD;
        }
        //if our tour is of Random Pattern
        else
        {
            if(tourCfg.tour[currCamTourInfo[camId].currTour].tourPattern == RANDOM)
            {
                //init current Order Array with randomly generated nos.
                initCurrOrderArray(camId, currCamTourInfo[camId].currTour, &tourCfg);
            }
            pauseInterval = 0;
        }

        //go to the next position
        curPtzPos = currCamTourInfo[camId].currPositionIndex;
        currCamTourInfo[camId].currPositionIndex++;
    }
    //if we are at the end of the tour
    else if(currCamTourInfo[camId].currPositionIndex == (currCamTourInfo[camId].totConfPos - 1))
    {
        curPtzPos = currCamTourInfo[camId].currPositionIndex;
        pauseInterval = tourCfg.tour[currCamTourInfo[camId].currTour].pauseBetweenTour;
        if(tourCfg.tour[currCamTourInfo[camId].currTour].tourPattern == ZIGZAG)
        {
            //we will proceed backwards in the ZIGZAG tour
            currCamTourInfo[camId].currZigZagDir = ZIGZAG_BWD;

            //go to previous index
            currCamTourInfo[camId].currPositionIndex--;
        }
        else
        {
            //set the Position Index to the beginning
            currCamTourInfo[camId].currPositionIndex = 0;
        }
    }
    else    //in between start & end of a tour
    {
        curPtzPos = currCamTourInfo[camId].currPositionIndex;
        pauseInterval = 0;
        if((tourCfg.tour[currCamTourInfo[camId].currTour].tourPattern == LOOPING)
                || (tourCfg.tour[currCamTourInfo[camId].currTour].tourPattern == RANDOM))
        {
            //increment currCamPositionIndex
            currCamTourInfo[camId].currPositionIndex++;
        }
        else
        {
            //if current Ptz Tour Info pattern to Zigzag backward
            if(currCamTourInfo[camId].currZigZagDir == ZIGZAG_BWD)
            {
                currCamTourInfo[camId].currPositionIndex--;
            }
            else //if current Ptz Tour Info pattern to Zigzag Forward
            {
                currCamTourInfo[camId].currPositionIndex++;
            }
        }
    }

    //reload interval = be pause time + view time
    currReloadInterval = ((pauseInterval * TOTAL_SEC_IN_MIN) + currCamTourInfo[camId].activePositions[curPtzPos].viewTime);

    //reload the PTZ timer
    reloadPtzTimer(currReloadInterval, camId);
    MUTEX_UNLOCK(currCamTourInfo[camId].tourInfoMutex);

    if(GotoPtzPosition(camId, currCamTourInfo[camId].activePositions[curPtzPos].presetPosition, NULL, 0,FALSE) == CMD_SUCCESS)
    {
       snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camId));
       snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%d", currCamTourInfo[camId].activePositions[curPtzPos].presetPosition);
       WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_POSITION_CHANGE, detail, advncDetail, EVENT_ALERT);
    }

    DPRINT(PTZ_TOUR, "ptz tour running: [camera=%d], [tour=%d], [position=%d], [period=%d]", camId, currCamTourInfo[camId].currTour,
           currCamTourInfo[camId].activePositions[curPtzPos].presetPosition, currReloadInterval);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the reload interval input to it, if the interval is nonzero than it
 *          reloads the timer  with the interval otherwise it reload the same timer with systimer
 *          minimum interval
 * @param   interval
 * @param   ptzTmrNo
 */
static void reloadPtzTimer(UINT16 interval, UINT8 ptzTmrNo)
{
    //if user has configured a pause time or view time
    ReloadTimer(currCamTourInfo[ptzTmrNo].ptzTourTmrHandle, CONVERT_SEC_TO_TIMER_COUNT((interval ? interval : 100)));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks if the manual tour is running if so it stops the manual tour. It
 *          initializes the Cam Tour Info structure of the camera number (input param) and starts
 *          the schedule tour of the camera.
 * @param   camId
 * @param   presetTour
 * @param   tourType
 * @param   tourCfgPtr
 */
static void startSchPtzTour(UINT8 camId, UINT8 presetTour, TOUR_TYPE_e tourType, PRESET_TOUR_CONFIG_t *tourCfgPtr)
{
    TIMER_INFO_t 	ptzTourTmr;
    CHAR			eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR			eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    if((currCamTourInfo[camId].ptzTourTmrHandle != INVALID_TIMER_HANDLE) && (currCamTourInfo[camId].currTourType == PTZ_TOUR_MANUAL))
    {
        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfgPtr->tour[currCamTourInfo[camId].currTour].name);
        stopPtzTour(camId, eventAdvDetail);
    }

    //initialize the Cam Tour Info structure
    initCurrCamTourInfo(camId, presetTour, tourType, tourCfgPtr);

    //start runPtz with index [camId]
    ptzTourTmr.count = CONVERT_MSEC_TO_TIMER_COUNT(100);
    ptzTourTmr.data = camId;
    ptzTourTmr.funcPtr = runPtzTour;
    StartTimer(ptzTourTmr, &currCamTourInfo[camId].ptzTourTmrHandle);

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camId));
    snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfgPtr->tour[presetTour].name);
    WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_AUTO);
    DPRINT(PTZ_TOUR, "ptz schedule tour started: [camera=%d]", camId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a function which stops the scheduled PTZ Tour of a camera by deleting schedule
 *          PTZ tour timer of the camera ID input.
 * @param   camId
 * @param   advncDetail
 */
static void stopPtzTour(UINT8 camId, CHARPTR advncDetail)
{
    CHAR eventDetail[MAX_EVENT_DETAIL_SIZE];

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camId));
    WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, advncDetail, EVENT_STOP);
    currCamTourInfo[camId].isTourPaused = FALSE;
    DeleteTimer(&currCamTourInfo[camId].ptzTourTmrHandle);
    currCamTourInfo[camId].currTourType = PTZ_TOUR_IDLE;
    DPRINT(PTZ_TOUR, "ptz tour stop: [camera=%d]", camId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns YES if the camera tour is stop/pause sucessfully
 * @param   camIndex
 * @return
 */
BOOL PausePtzTour(UINT8 camIndex)
{
    CHAR                    eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                    eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    PRESET_TOUR_CONFIG_t	tourCfg;

    MUTEX_LOCK(currCamTourInfo[camIndex].tourInfoMutex);
    if((currCamTourInfo[camIndex].isTourPaused != TRUE) && (currCamTourInfo[camIndex].currTourType != PTZ_TOUR_IDLE))
    {
        ReadSinglePresetTourConfig(camIndex, &tourCfg);
        if(currCamTourInfo[camIndex].currTourType == PTZ_TOUR_MANUAL)
        {
            snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
            stopPtzTour(camIndex,eventAdvDetail);
        }
        else
        {
            currCamTourInfo[camIndex].isTourPaused = TRUE;
            snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
            WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_PAUSE);
        }
        MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
        return TRUE;
    }
    MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function resume tour from given position
 * @param   camIndex
 * @return
 */
NET_CMD_STATUS_e ResumePtzTour(UINT8 camIndex)
{
    CHAR					eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR					eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    UINT8 					schNo;
    struct tm 				currTimeStruct = { 0 };
    TIME_HH_MM_t 			currTime;
    TOUR_SCHEDULE_CONFIG_t 	tourSch;
    PRESET_TOUR_CONFIG_t	tourCfg;

    //Get the current time from DateTime in broken Format
    GetLocalTimeInBrokenTm(&currTimeStruct);

    MUTEX_LOCK(currCamTourInfo[camIndex].tourInfoMutex);
    if(currCamTourInfo[camIndex].isTourPaused == FALSE)
    {
        MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
        return CMD_SUCCESS;
    }

    //Read Camera TourConfig for each camera
    ReadSingleTourScheduleConfig(camIndex, &tourSch);
    ReadSinglePresetTourConfig(camIndex, &tourCfg);
    if((currCamTourInfo[camIndex].currTourType == PTZ_TOUR_IDLE) || (currCamTourInfo[camIndex].currTourType == PTZ_TOUR_MANUAL))
    {
        //If camIndex Tour is scheduled for curr. weekday for entire day
        if(tourSch.tour[currTimeStruct.tm_wday].entireDay == TRUE)
        {
            currCamTourInfo[camIndex].isTourPaused  = FALSE;
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
            snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
            WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_RESUME);
        }
        else //scheduled Tour & not entire day tour
        {
            for(schNo = 0; schNo < MAX_TOUR_SCHEDULE; schNo++)
            {
                currTime.hour = currTimeStruct.tm_hour;
                currTime.minute = currTimeStruct.tm_min;

                //If current time between start and end time of the schedule tour for a camera
                if(IsGivenTimeInWindow(currTime,tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.startTime,
                                       tourSch.tour[currTimeStruct.tm_wday]. schedule[schNo].period.endTime) == YES)
                {
                    currCamTourInfo[camIndex].isTourPaused  = FALSE;
                    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
                    snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
                    WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_RESUME);
                }
                else
                {
                    snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
                    stopPtzTour(camIndex, eventAdvDetail);
                }
            }
        }
    }
    else if(currCamTourInfo[camIndex].currTourType == PTZ_TOUR_ENTIRE_DAY)
    {
        if(tourSch.tour[currTimeStruct.tm_wday].entireDay == FALSE)
        {
            snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
            stopPtzTour(camIndex, eventAdvDetail);
        }
        else
        {
            currCamTourInfo[camIndex].isTourPaused  = FALSE;
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
            snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
            WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_RESUME);
        }
    }
    else
    {
        for(schNo = 0; schNo < MAX_TOUR_SCHEDULE; schNo++)	 //scheduled Tour
        {
            currTime.hour = currTimeStruct.tm_hour;
            currTime.minute = currTimeStruct.tm_min;

            if(IsGivenTimeInWindow(currTime,tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.startTime,
                                   tourSch.tour[currTimeStruct.tm_wday].schedule[schNo].period.endTime) == NO)
            {
                //is the running sch. tour the one we should stop
                if(currCamTourInfo[camIndex].currTourType == schNo)
                {
                    snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
                    stopPtzTour(camIndex, eventAdvDetail);
                }
            }
            else
            {
                currCamTourInfo[camIndex].isTourPaused  = FALSE;
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
                snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SCHEDULE_TOUR_ADV_DETAIL, tourCfg.tour[currCamTourInfo[camIndex].currTour].name);
                WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_TOUR, eventDetail, eventAdvDetail, EVENT_RESUME);
            }
        }
    }
    MUTEX_UNLOCK(currCamTourInfo[camIndex].tourInfoMutex);
    return CMD_SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
