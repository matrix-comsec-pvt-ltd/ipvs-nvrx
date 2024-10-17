//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file       SnapshotSchd.c
@brief      The image uploader uploads images to given FTP server and/or send via email. It will
            send or upload images periodically. This period was defined in configuration module.
            And also between this time any other action tells that image uploading should start
            then this module immediately sends images. If image should be send by email then this
            module gives email to SMTP client with attachment as IMAGE. The images for uploading to
            FTP server are added into queue and those images which are into queue are uploaded on
            ftp server at every one second (when one second timer was expire). Whenever there was
            connection error from lib curl then uploading was stopped and started after predefined time.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Utils.h"
#include "CameraInterface.h"
#include "SmtpClient.h"
#include "FtpClient.h"
#include "SnapshotSchd.h"
#include "NetworkController.h"
#include "DebugLog.h"
#include "CameraDatabase.h"
#include "Queue.h"
#include "NwDriveManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define CAMERA_NAME					"Camera%02d"
#define IMAGE_UPL_DATE_FORMAT		"%02d_%s_%04d%c%02d%c"
#define IMAGE_UPL_FILE_NAME			"%02d_%02d_%02d.jpg"
#define DIR_SEPERATOR				'/'

#define IMAGE_FAIL_TIME				(15)
#define MAX_FILE_NAME				(50)
#define MAX_URL_SIZE				(250)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	SNAPSHT_UPLOAD_STOP,
	SNAPSHT_UPLOAD_START,
	SNAPSHT_UPLOAD_FAIL,
	MAX_SNAPSHT_UPLOAD_STATE

}SNAPSHT_UPLOAD_STATE_e;

typedef struct
{
	// file name which is to be upload
    CHAR            fileName[MAX_FILE_NAME];
    CHAR            relativeUrl[MAX_URL_SIZE];
    FTP_HANDLE      ftpHandle;
    NAS_HANDLE      nasHandle;
}IMAGE_FILE_INFO_t;

// Image upload information
typedef struct
{
    BOOL                    startImgUpldStatus;
    TIMER_HANDLE            imgTimeHandle;
    UINT32                  imgUpldSysTk;
    UINT32                  imgUploadRateCnt;
    pthread_mutex_t         imgUpldInfMutx;
    SNAPSHT_UPLOAD_STATE_e  imgUpldState;
    CHAR                    fileName[MAX_FILE_NAME];

}SNAPSHOT_SCHEDULE_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR cosecMonthName[MAX_MONTH] =
{
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

// Connection timer handle
static SNAPSHOT_SCHEDULE_INFO_t		snapShotUploadPara[MAX_CAMERA];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void getImageTimer(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL startSnapshotUpload(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static BOOL stopSanpshotUpload(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static void snapshotFtpCb(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static void snapshotNasCb(NAS_HANDLE nasHandle, NAS_RESPONSE_e nasResponse, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static BOOL checkTimeForSnapShot(struct tm*	brokenTime,SNAPSHOT_SCHEDULE_CONFIG_t* snapShotSchdCnfg);
//-------------------------------------------------------------------------------------------------
static BOOL snapshotScheduler(UINT32 count);
//-------------------------------------------------------------------------------------------------
static void imageCallBack(UINT8 channelNo, NET_CMD_STATUS_e status, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static BOOL writeToImageFile(UINT8 channelNo, CHARPTR fileName, CHARPTR relativeUrl, CHARPTR imageBuff, UINT32 imageSize);
//-------------------------------------------------------------------------------------------------
static void procedToNextImage(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static BOOL getSnapshotLocationStr(UINT8 channelNo, CHARPTR pSnapshotLocationStr, UINT16 buffLen);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes image uploader module and starts one second timer for
 *          uploading image or sending image via email.
 */
void InitSnapshotSchdUpload(void)
{
    UINT8               channelCnt;
    ONE_MIN_NOTIFY_t    oneMinFun;

	oneMinFun.funcPtr = snapshotScheduler;
	oneMinFun.userData = 0;
	RegisterOnMinFun(&oneMinFun);

	for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
	{
		//default image upload parameter
		snapShotUploadPara[channelCnt].startImgUpldStatus = OFF;
		snapShotUploadPara[channelCnt].imgTimeHandle = INVALID_TIMER_HANDLE;
		snapShotUploadPara[channelCnt].imgUpldSysTk = 0;
		snapShotUploadPara[channelCnt].imgUpldState = MAX_SNAPSHT_UPLOAD_STATE;
        MUTEX_INIT(snapShotUploadPara[channelCnt].imgUpldInfMutx, NULL);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initialize image upload module. It will clear all memory resources used
 *          by image upload module and also destroy the mutex which is used for data sharing.
 */
void DeinitSnapshotSchdUpload(void)
{
    UINT8 channelCnt;

	// Set snapShotUploadPara to default value.
	for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
	{
		//make all image upload parameter default
        MUTEX_LOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
		snapShotUploadPara[channelCnt].startImgUpldStatus = OFF;
		snapShotUploadPara[channelCnt].imgUpldSysTk    = 0;
        MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
		//delete particular channel getImage timer if it is running
		DeleteTimer(&snapShotUploadPara[channelCnt].imgTimeHandle);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was callback of timer which is periodically run for every one hour.
 *          It will checks whether its time to start snapshot schedule up as per configuration.
 * @param count
 * @return
 */
static BOOL snapshotScheduler(UINT32 count)
{
	UINT8						channelCnt;
	struct tm					brokenTime = { 0 };
    SNAPSHOT_CONFIG_t			snapShotCnfg;
    SNAPSHOT_SCHEDULE_CONFIG_t  snapShotSchdCnfg;
	CHAR						eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                        advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

	for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
	{
        ReadSingleSnapshotConfig(channelCnt, &snapShotCnfg);
        ReadSingleSnapshotScheduleConfig(channelCnt, &snapShotSchdCnfg);

		// check current time
		GetLocalTimeInBrokenTm(&brokenTime);

		// check snapshot Cnfg enble
        if(snapShotCnfg.snapShotEnable == DISABLE)
		{
            MUTEX_LOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
            if(snapShotUploadPara[channelCnt].imgTimeHandle == INVALID_TIMER_HANDLE)
            {
                MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
                continue;
            }
            MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);

            if(snapShotUploadPara[channelCnt].imgUpldState != SNAPSHT_UPLOAD_STOP)
            {
                snapShotUploadPara[channelCnt].imgUpldState = SNAPSHT_UPLOAD_STOP;
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelCnt));
                WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, NULL, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_STOP);
            }

            stopSanpshotUpload(channelCnt);
            DeleteTimer(&snapShotUploadPara[channelCnt].imgTimeHandle);
            continue;
        }

        if(checkTimeForSnapShot(&brokenTime, &snapShotSchdCnfg) == TRUE)
        {
            MUTEX_LOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
            if(snapShotUploadPara[channelCnt].imgTimeHandle != INVALID_TIMER_HANDLE)
            {
                MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
                continue;
            }
            MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);

            if(snapShotUploadPara[channelCnt].imgUpldState != SNAPSHT_UPLOAD_START)
            {
                snapShotUploadPara[channelCnt].imgUpldState = SNAPSHT_UPLOAD_START;
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelCnt));
                getSnapshotLocationStr(channelCnt, advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE);
                WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, advncDetail, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_START);
            }
            startSnapshotUpload(channelCnt);
        }
        else
        {
            MUTEX_LOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
            if(snapShotUploadPara[channelCnt].imgTimeHandle == INVALID_TIMER_HANDLE)
            {
                MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);
                continue;
            }
            MUTEX_UNLOCK(snapShotUploadPara[channelCnt].imgUpldInfMutx);

            if(snapShotUploadPara[channelCnt].imgUpldState != SNAPSHT_UPLOAD_STOP)
            {
                snapShotUploadPara[channelCnt].imgUpldState = SNAPSHT_UPLOAD_STOP;
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelCnt));
                WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, NULL, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_STOP);
            }

            stopSanpshotUpload(channelCnt);
            DeleteTimer(&snapShotUploadPara[channelCnt].imgTimeHandle);
        }
	}

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkTimeForSnapShot
 * @param   brokenTime
 * @param   snapShotSchdCnfg
 * @return
 */
static BOOL checkTimeForSnapShot(struct tm*	brokenTime,SNAPSHOT_SCHEDULE_CONFIG_t* snapShotSchdCnfg)
{
	UINT8			actionCntrlCnt;
	TIME_HH_MM_t	timeWindow;

	// is action should take of today
    if ((snapShotSchdCnfg->copyToWeekDays & (1 << brokenTime->tm_wday)) == 0)
	{
        return FALSE;
    }

    // check snapshot Cnfg for entireday
    if(snapShotSchdCnfg->dailySanpShotPeriod.recordEntireDay == TRUE)
    {
        return TRUE;
    }

    timeWindow.hour = brokenTime->tm_hour;
    timeWindow.minute = brokenTime->tm_min;

    // entire day was not enable so checking each 6 schedule event time
    for(actionCntrlCnt = 0; actionCntrlCnt < MAX_EVENT_SCHEDULE; actionCntrlCnt++)
    {
        if(IsGivenTimeInWindow(timeWindow, snapShotSchdCnfg->dailySanpShotPeriod.period[actionCntrlCnt].startTime,
                               snapShotSchdCnfg->dailySanpShotPeriod.period[actionCntrlCnt].endTime) == YES)
        {
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initiates image uploading for particular channel. It updates entry in
 *          snapShotUploadPara for specified channel and initiates image retrieval request from
 *          Camera Interface module.
 * @param   channelNo
 * @return
 */
static BOOL startSnapshotUpload(UINT8 channelNo)
{
    TIMER_INFO_t        imageTimerInfo;
    CHAR                eventDetail[MAX_EVENT_DETAIL_SIZE];
    UINT16              nextTime;
    INT32               recvImgTimeDiffCnt;
    INT32               upldRateCnt;
    LocalTime_t         localTime = { 0 };
    SNAPSHOT_CONFIG_t   snpshtUpldCfg;

	// validate channel number
    if (channelNo >= getMaxCameraForCurrentVariant())
	{
        return FAIL;
    }

    ReadSingleSnapshotConfig(channelNo, &snpshtUpldCfg);
    MUTEX_LOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);

    //If image upload not started then start it
    if(snapShotUploadPara[channelNo].startImgUpldStatus != OFF)
    {
        MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
        DPRINT(SNAPSHOT_SCHEDULE, "image upload already started: [camera=%d]", channelNo);
        return SUCCESS;
    }

    snapShotUploadPara[channelNo].startImgUpldStatus = ON;
    MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);

    if(GetImage(channelNo, NULL, imageCallBack, CLIENT_CB_TYPE_NATIVE) != CMD_SUCCESS)
    {
        upldRateCnt = (SEC_IN_ONE_MIN / snpshtUpldCfg.snapShotuploadImageRate);
        upldRateCnt = CONVERT_SEC_TO_TIMER_COUNT(upldRateCnt);
        recvImgTimeDiffCnt = ElapsedTick(upldRateCnt + GetSysTick());

        //if upload rate is greater then received image
        if(upldRateCnt > recvImgTimeDiffCnt)
        {
            nextTime = (upldRateCnt - recvImgTimeDiffCnt);
        }
        else
        {
            GetLocalTime(&localTime);
            nextTime = (10 - (localTime.mSec / TIMER_RESOLUTION_MINIMUM_MSEC));
        }

        if(snapShotUploadPara[channelNo].imgTimeHandle == INVALID_TIMER_HANDLE)
        {
            imageTimerInfo.count   = (nextTime + 1);
            imageTimerInfo.data    = (UINT32)channelNo;
            imageTimerInfo.funcPtr = &getImageTimer;
            StartTimer(imageTimerInfo, &snapShotUploadPara[channelNo].imgTimeHandle);
        }

        if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_FAIL)
        {
            snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_FAIL;
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
            WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, NULL, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_FAIL);
        }

        EPRINT(SNAPSHOT_SCHEDULE, "image request failed: [camera=%d]", channelNo);
        return FAIL;
    }

    snapShotUploadPara[channelNo].imgUpldSysTk = GetSysTick();
    DPRINT(SNAPSHOT_SCHEDULE, "get image ok: [camera=%d]", channelNo);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops image uploading for particular camera.
 * @param   channelNo
 * @return
 */
static BOOL stopSanpshotUpload(UINT8 channelNo)
{
    if (channelNo >= getMaxCameraForCurrentVariant())
	{
        return FAIL;
    }

    MUTEX_LOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
    if(snapShotUploadPara[channelNo].startImgUpldStatus == ON)
    {
        snapShotUploadPara[channelNo].startImgUpldStatus = OFF;
        DPRINT(SNAPSHOT_SCHEDULE, "image upload stopped: [camera=%d]", channelNo);
    }
    MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   this function is used to write image file in to our specified file interface ,and returns
 *          its local file name and its upload relative url.
 * @param   channelNo
 * @param   fileName
 * @param   relativeUrl
 * @param   imageBuff
 * @param   imageSize
 * @return
 */
static BOOL writeToImageFile(UINT8 channelNo, CHARPTR fileName, CHARPTR relativeUrl, CHARPTR imageBuff, UINT32 imageSize)
{
	INT32		fileFd;
	INT32 		writeCnt;
	struct tm 	brokenTime = { 0 };

    if ((imageBuff == NULL) || (imageSize == 0))
	{
        EPRINT(SNAPSHOT_SCHEDULE, "image size is not proper: [camera=%d], [imageSize=%d]", channelNo, imageSize);
        return FAIL;
    }

    GetLocalTimeInBrokenTm(&brokenTime);
    // Image File Name
    snprintf(fileName, MAX_FILE_NAME, "/tmp/"CAMERA_NAME"-"IMAGE_UPL_FILE_NAME,
             GET_CAMERA_NO(channelNo), brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec);

    if (access(fileName, F_OK) == STATUS_OK)
    {
        EPRINT(SNAPSHOT_SCHEDULE, "image file already present, skipped: [camera=%d]", channelNo);
        return FAIL;
    }

    // make local copy only. Make relative URL for remote file
    snprintf(relativeUrl, MAX_URL_SIZE, CAMERA_NAME"%c"IMAGE_UPL_DATE_FORMAT IMAGE_UPL_FILE_NAME, GET_CAMERA_NO(channelNo), DIR_SEPERATOR,
             brokenTime.tm_mday, cosecMonthName[brokenTime.tm_mon], brokenTime.tm_year, DIR_SEPERATOR, brokenTime.tm_hour, DIR_SEPERATOR,
            brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec);

    //write image to file
    fileFd = open(fileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(fileFd == INVALID_FILE_FD)
    {
        EPRINT(SNAPSHOT_SCHEDULE, "failed to open image file: [camera=%d], [err=%s]", channelNo, STR_ERR);
        return FAIL;
    }

    writeCnt = write(fileFd, imageBuff, imageSize);
    close(fileFd);

    if(writeCnt < (INT32)imageSize)
    {
        unlink(fileName);
        EPRINT(SNAPSHOT_SCHEDULE, "image file is not proper: [camera=%d]", channelNo);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function invoked for particular channel by camera interface. In this
 *          function we received the image buffer and write it to a file. Then we add it into image upload queue.
 * @param   channelNo
 * @param   status
 * @param   imageBuff
 * @param   imageSize
 * @param   clientCbType
 */
static void imageCallBack(UINT8 channelNo, NET_CMD_STATUS_e status, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType)
{
	IMAGE_FILE_INFO_t  		imageFile;
	FTP_HANDLE				ftpHandle;
	NAS_HANDLE				nasHandle;
	NAS_FILE_INFO_t			nasFileInfo;
	FTP_FILE_INFO_t    		ftpFileInfo;
	CHAR					eventDetail[MAX_EVENT_DETAIL_SIZE];
	BOOL					procedNextImage = FALSE;
	SNAPSHOT_CONFIG_t 		snpshtUpldCfg;

	do
	{
        if (status != CMD_SUCCESS)
		{
            EPRINT(SNAPSHOT_SCHEDULE, "image request failed: [camera=%d], [status=%d]", channelNo, status);
			procedNextImage = TRUE;
			break;
		}

		//Now write image to file name specified in fileName
        status = CMD_SNAPSHOT_FAILED;
        if(writeToImageFile(channelNo, imageFile.fileName, imageFile.relativeUrl, imageBuff, imageSize) == FAIL)
		{
			if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_FAIL)
			{
				snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_FAIL;
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
                WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, NULL, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_FAIL);
                EPRINT(SNAPSHOT_SCHEDULE, "schedule image upload failed: [camera=%d]", channelNo);
			}

            DPRINT(SNAPSHOT_SCHEDULE, "image write failed: [camera=%d]", channelNo);
			procedNextImage = TRUE;
			break;
		}

        ReadSingleSnapshotConfig(channelNo, &snpshtUpldCfg);
		switch(snpshtUpldCfg.snapShotuploadLocation)
		{
            case UPLOAD_TO_FTP_SERVER1:
            case UPLOAD_TO_FTP_SERVER2:
                snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s", imageFile.fileName);
                snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "%s", imageFile.relativeUrl);
                snprintf(snapShotUploadPara[channelNo].fileName, MAX_FILE_NAME, "%s", imageFile.fileName);
                ftpFileInfo.ftpServer = (snpshtUpldCfg.snapShotuploadLocation - UPLOAD_TO_FTP_SERVER1);
                status = StartFtpUpload(&ftpFileInfo, snapshotFtpCb, channelNo, &ftpHandle);
                break;

            case UPLOAD_TO_EMAIL_SERVER:
                status = ProcessEmail(&snpshtUpldCfg.snapShotuploadToEmail, imageFile.fileName);
                snprintf(snapShotUploadPara[channelNo].fileName, MAX_FILE_NAME, "%s", imageFile.fileName);
                procedNextImage = TRUE;
                break;

            case UPLOAD_TO_NW_DRIVE1:
            case UPLOAD_TO_NW_DRIVE2:
                nasFileInfo.nasServer = (snpshtUpldCfg.snapShotuploadLocation - UPLOAD_TO_NW_DRIVE1);
                nasFileInfo.camIndex = channelNo;
                snprintf(nasFileInfo.localFileName, MAX_FILE_NAME_SIZE, "%s", imageFile.fileName);
                snprintf(nasFileInfo.remoteFile, MAX_FILE_NAME_SIZE, "%s", imageFile.relativeUrl);
                snprintf(snapShotUploadPara[channelNo].fileName, MAX_FILE_NAME, "%s", imageFile.fileName);
                status = StartNasUpload(&nasFileInfo, snapshotNasCb, channelNo, &nasHandle);
                break;

            default:
                EPRINT(SNAPSHOT_SCHEDULE, "invld image upload config: [location=%d]", snpshtUpldCfg.snapShotuploadLocation);
                break;
		}

		if(status != CMD_SUCCESS)
		{
			unlink(snapShotUploadPara[channelNo].fileName);
			if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_FAIL)
			{
                CHAR advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

				snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_FAIL;
                advncDetail[0] = '\0';
                if (status == CMD_SERVER_DISABLED)
                {
                    if (SUCCESS == getSnapshotLocationStr(channelNo, eventDetail, MAX_EVENT_DETAIL_SIZE))
                    {
                        snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | config disabled", eventDetail);
                    }
                }
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
                WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, advncDetail, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_FAIL);
                EPRINT(SNAPSHOT_SCHEDULE, "schedule image upload failed: [camera=%d]", channelNo);
			}

			procedNextImage = TRUE;
		}
	}
	while(0);

	if(procedNextImage == TRUE)
	{
		procedToNextImage(channelNo);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was request to FTP with fixed text file for checking connectivity with FTP server
 * @param   channelNo
 */
static void getImageTimer(UINT32 channelNo)
{
    CHAR            eventDetail[MAX_EVENT_DETAIL_SIZE];
    TIMER_INFO_t    imageTimerInfo;

	DeleteTimer(&snapShotUploadPara[channelNo].imgTimeHandle);
    MUTEX_LOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
    if(snapShotUploadPara[channelNo].startImgUpldStatus == ON)
	{
        MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
		snapShotUploadPara[channelNo].imgUpldSysTk = GetSysTick();
        if(GetImage(channelNo, NULL, imageCallBack, CLIENT_CB_TYPE_NATIVE) == CMD_SUCCESS)
		{
            DPRINT(SNAPSHOT_SCHEDULE, "get image ok: [camera=%d]", channelNo);
		}
		else
		{
			if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_FAIL)
			{
				snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_FAIL;
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
                WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, NULL, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_FAIL);
			}

			imageTimerInfo.count   = CONVERT_SEC_TO_TIMER_COUNT(IMAGE_FAIL_TIME);
			imageTimerInfo.data    = (UINT32)channelNo;
			imageTimerInfo.funcPtr = &getImageTimer;
			StartTimer(imageTimerInfo, &snapShotUploadPara[channelNo].imgTimeHandle);
            EPRINT(SNAPSHOT_SCHEDULE, "image request failed: [camera=%d]", channelNo);
		}
	}
	else
	{
        MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
        DPRINT(SNAPSHOT_SCHEDULE, "image upload stopped: [camera=%d]", channelNo);
		if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_STOP)
		{
            snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_STOP;
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
            WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, NULL, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_STOP);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function for FTP Module, based on response of FTP module this
 *          function reply to the client requested for snapshot
 * @param   ftpHandle
 * @param   ftpResponse
 * @param   userData
 */
static void snapshotFtpCb(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData)
{
    UINT8   channelNo = (UINT8)userData;
    CHAR    eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR    advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

	unlink(snapShotUploadPara[channelNo].fileName);
	if(ftpResponse != FTP_SUCCESS)
	{
		if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_FAIL)
		{
            EPRINT(SNAPSHOT_SCHEDULE, "snapshot ftp callback status: [camera=%d], [ftpResponse=%d]", channelNo, ftpResponse);
			snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_FAIL;
            getSnapshotLocationStr(channelNo, eventDetail, MAX_EVENT_DETAIL_SIZE);
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s", eventDetail, GetFtpStatusStr(ftpResponse));
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
            WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, advncDetail, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_FAIL);
		}
	}
	else
	{
		if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_START)
		{
			snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_START;
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
            getSnapshotLocationStr(channelNo, advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE);
            WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, advncDetail, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_START);
		}
	}

	procedToNextImage(channelNo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function for FTP Module, based on response of FTP module this
 *          function reply to the client requested for snapshot
 * @param   nasHandle
 * @param   nasResponse
 * @param   userData
 */
static void snapshotNasCb(NAS_HANDLE nasHandle, NAS_RESPONSE_e nasResponse, UINT16 userData)
{
    UINT8   channelNo = (UINT8)userData;
    CHAR    eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR    advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

	unlink(snapShotUploadPara[channelNo].fileName);
	if(nasResponse != NAS_SUCCESS)
	{
		if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_FAIL)
		{
            EPRINT(SNAPSHOT_SCHEDULE, "snapshot nas callback status: [camera=%d], [nasResponse=%d]", channelNo, nasResponse);
			snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_FAIL;
            getSnapshotLocationStr(channelNo, eventDetail, MAX_EVENT_DETAIL_SIZE);
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s", eventDetail, GetNddStatusStr(nasResponse));
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
            WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, advncDetail, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_FAIL);
		}
	}
	else
	{
		if(snapShotUploadPara[channelNo].imgUpldState != SNAPSHT_UPLOAD_START)
		{
			snapShotUploadPara[channelNo].imgUpldState = SNAPSHT_UPLOAD_START;
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
            getSnapshotLocationStr(channelNo, advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE);
            WriteEvent(LOG_CAMERA_EVENT, LOG_SNAPSHOT_SCHEDULE, eventDetail, advncDetail, (LOG_EVENT_STATE_e)SNAPSHT_UPLOAD_START);
		}
	}

	procedToNextImage(channelNo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   procedToNextImage
 * @param   channelNo
 */
static void procedToNextImage(UINT8 channelNo)
{
	UINT16 			   		nextTime;
	INT32 			   		recvImgTimeDiffCnt;
	INT32 			   		upldRateCnt;
	LocalTime_t				localTime = { 0 };
	TIMER_INFO_t  	   		imageTimerInfo;
	SNAPSHOT_CONFIG_t 		snpshtUpldCfg;

	//Now call get image function according to snapSht upload configuration rate
    MUTEX_LOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
    if(snapShotUploadPara[channelNo].startImgUpldStatus == OFF)
	{
        MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);
        DeleteTimer(&snapShotUploadPara[channelNo].imgTimeHandle);
        DPRINT(SNAPSHOT_SCHEDULE, "image upload stopped: [camera=%d]", channelNo);
        return;
    }
    MUTEX_UNLOCK(snapShotUploadPara[channelNo].imgUpldInfMutx);

    ReadSingleSnapshotConfig(channelNo, &snpshtUpldCfg);
    recvImgTimeDiffCnt = ElapsedTick(snapShotUploadPara[channelNo].imgUpldSysTk);
    upldRateCnt = (SEC_IN_ONE_MIN / snpshtUpldCfg.snapShotuploadImageRate);
    upldRateCnt = CONVERT_SEC_TO_TIMER_COUNT(upldRateCnt);

    //if upload rate is greater then received image
    if(upldRateCnt > recvImgTimeDiffCnt)
    {
        nextTime = (upldRateCnt - recvImgTimeDiffCnt);
    }
    else
    {
        if(SUCCESS != GetLocalTime(&localTime))
        {
            EPRINT(SNAPSHOT_SCHEDULE, "failed to get local time: [camera=%d]", channelNo);
        }
        nextTime = (10 - (localTime.mSec / TIMER_RESOLUTION_MINIMUM_MSEC));
    }

    if(snapShotUploadPara[channelNo].imgTimeHandle != INVALID_TIMER_HANDLE)
    {
        EPRINT(SNAPSHOT_SCHEDULE, "get image timer running: [camera=%d]", channelNo);
        return;
    }

    imageTimerInfo.count   = (nextTime + 1);
    imageTimerInfo.data    = (UINT32)channelNo;
    imageTimerInfo.funcPtr = &getImageTimer;
    StartTimer(imageTimerInfo, &snapShotUploadPara[channelNo].imgTimeHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get snapshot location prefix for event's advance details
 * @param   channelNo
 * @param   pSnapshotLocationStr
 * @param   buffLen
 * @return  SUCCESS/FAIL
 */
static BOOL getSnapshotLocationStr(UINT8 channelNo, CHARPTR pSnapshotLocationStr, UINT16 buffLen)
{
    SNAPSHOT_CONFIG_t snpshtUpldCfg;

    ReadSingleSnapshotConfig(channelNo, &snpshtUpldCfg);
    switch(snpshtUpldCfg.snapShotuploadLocation)
    {
        case UPLOAD_TO_FTP_SERVER1:
        case UPLOAD_TO_FTP_SERVER2:
        {
            snprintf(pSnapshotLocationStr, buffLen, "FTP-%d", (snpshtUpldCfg.snapShotuploadLocation - UPLOAD_TO_FTP_SERVER1)+1);
        }
        return SUCCESS;

        case UPLOAD_TO_EMAIL_SERVER:
        {
            snprintf(pSnapshotLocationStr, buffLen, "Email");
        }
        return SUCCESS;

        case UPLOAD_TO_NW_DRIVE1:
        case UPLOAD_TO_NW_DRIVE2:
        {
            snprintf(pSnapshotLocationStr, buffLen, "NDD-%d", (snpshtUpldCfg.snapShotuploadLocation - UPLOAD_TO_NW_DRIVE1)+1);
        }
        return SUCCESS;

        default:
        {
            pSnapshotLocationStr[0] = '\0';
        }
        return FAIL;
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
