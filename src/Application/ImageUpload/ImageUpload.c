//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		ImageUpload.c
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
#include "ImageUpload.h"
#include "NetworkController.h"
#include "DebugLog.h"
#include "EventLogger.h"
#include "CameraDatabase.h"
#include "Queue.h"
#include "NwDriveManager.h"
#include "DoorCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define CAMERA_NAME                     "Camera%02d"
#define IMAGE_UPL_DATE_FORMAT           "%02d_%s_%04d%c%02d%c"
#define IMAGE_UPL_FILE_NAME             "%02d_%02d_%02d.jpg"
#define COSEC_IMAGE_UPL_FILE_NAME       "%02d%02d%02d_cam%02d.jpg"
#define COSEC_IMAGE_UPL_FTP_DOORINFO	"%02d_%05d_%02d"
#define COSEC_IMAGE_UPL_EMAIL_DOORINFO	"%02d_%05d_%02d_%02d"
#define DIR_SEPERATOR                   '/'
#define COSEC_FOLDER                    "COSEC%c"
#define COSEC_EMAIL_SUBJECT             "SATATYA Alert"

#define IMAGE_FAIL_TIME                 (15)
#define MAX_FILE_NAME                   (50)
#define MAX_URL_SIZE                    (250)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	// file name which is to be upload
    CHAR                fileName[MAX_FILE_NAME];
    CHAR                relativeUrl[MAX_URL_SIZE];
    FTP_HANDLE          ftpHandle;
    NAS_HANDLE          nasHandle;
}IMAGE_FILE_INFO_t;

// Image upload information
typedef struct
{
    UINT8               startImgUpldCnt;
    TIMER_HANDLE        imgTimeHandle;
    UINT32              imgUpldSysTk;
    UINT32              imgUploadRateCnt;
    pthread_mutex_t     imgUpldInfMutx;
}IMAGE_UPLOAD_INFO_t;

typedef struct
{
    BOOL                reqStatus;
    CHAR                fileName[MAX_FILE_NAME];
    NW_CMD_REPLY_CB     callback;
    INT32               connFd;
    pthread_mutex_t     reqStatusLock;
}GET_SNAPSHOT_INFO_t;

typedef struct
{
    BOOL                reqStatus;
    CHAR                fileName[MAX_COSEC_CMD][MAX_FILE_NAME];
    pthread_mutex_t     cosecReqStatusLock;
    COSEC_INTEGRATION_t cosecInfo[MAX_COSEC_CMD];
}GET_COSEC_SNAPSHOT_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void getImageTimer(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void imageCallBack(UINT8 channelNo, NET_CMD_STATUS_e status, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static void snapshotFtpCb(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static void snapshotNasCb(NAS_HANDLE nasHandle, NAS_RESPONSE_e nasResponse, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static void snapshotCiCb(UINT8 channelNo, NET_CMD_STATUS_e commandResponse, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static BOOL writeToImageFile(UINT8 channelNo, CHARPTR fileName, CHARPTR relativeUrl, CHARPTR imageBuff, UINT32 imageSize, const UINT16 relativeUrlLen);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e uploadImageToLoc(UINT8 channelNo, IMAGE_UPLOAD_CONFIG_t * imgUpldCfg, CHARPTR fileName, CHARPTR relativeUrl);
//-------------------------------------------------------------------------------------------------
static void cosecSnapshotCiCb(UINT8 channelNo, NET_CMD_STATUS_e commandResponse, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static BOOL createCosecNotifyMsg(UINT8 channelNo, COSEC_CMD_NAME_e doorCommand, CHARPTR msg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
// Connection timer handle
static IMAGE_UPLOAD_INFO_t 			imageUploadPara[MAX_CAMERA];
static GET_SNAPSHOT_INFO_t 			snpshotInfo[MAX_CAMERA];
static GET_COSEC_SNAPSHOT_INFO_t 	cosecSnapShotInfo[MAX_CAMERA];
static EMAIL_PARAMETER_t			cosecEmailParam;

static const CHARPTR cosecEmailStr[COSEC_DOOR_EVENT_MAX] =
{
	"Access allowed to %s (User ID: %s) on Door - %s on %02d_%s_%04d at %02d:%02d.",
	"Access denied to %s (User ID: %s) on Door - %s on %02d_%s_%04d at %02d:%02d.",
	"Access denied on Door - %s on %02d_%s_%04d at %02d:%02d, as the user could not be identified.",
	"Status of Aux In-1 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Duress Alarm activated by %s (User ID: %s) on Door - %s on %02d_%s_%04d at %02d:%02d.",
	"Dead Man timer on Door - %s expired for %s (User ID: %s) on %02d_%s_%04d at %02d:%02d.",
	"Panic Alarm detected on Door - %s on %02d_%s_%04d at %02d:%02d.",
	"Door - %s detected abnormal on %02d_%s_%04d at %02d:%02d.",
	"Door - %s was forced opened on %02d_%s_%04d at %02d:%02d.",
	"Tamper Alarm detected on Door - %s at on %02d_%s_%04d at %02d:%02d.",
	"Intercom Panic Alarm detected on door- %s by Extension Number-(User ID: %s) on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-2 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-3 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-4 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-5 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-6 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-7 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
	"Status of Aux In-8 on Door - %s has changed on %02d_%s_%04d at %02d:%02d.",
};

static const CHARPTR cosecMonthName[MAX_MONTH] =
{
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes image uploader module and starts one second timer for uploading
 *          image or sending image via email.
 */
void InitImageUpload(void)
{
    UINT8 channelCnt;

    //Initialize image and snapshot parameter for all channel
	for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
	{
		//default image upload parameter
		imageUploadPara[channelCnt].startImgUpldCnt = 0;
		imageUploadPara[channelCnt].imgTimeHandle = INVALID_TIMER_HANDLE;
		imageUploadPara[channelCnt].imgUpldSysTk = 0;
        MUTEX_INIT(imageUploadPara[channelCnt].imgUpldInfMutx, NULL);

        snpshotInfo[channelCnt].reqStatus = FREE;
        MUTEX_INIT(snpshotInfo[channelCnt].reqStatusLock, NULL);
        cosecSnapShotInfo[channelCnt].reqStatus = FREE;
        MUTEX_INIT(cosecSnapShotInfo[channelCnt].cosecReqStatusLock, NULL);
        snpshotInfo[channelCnt].callback = NULL;
        snpshotInfo[channelCnt].connFd 	 = INVALID_CONNECTION;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initialize image upload module. It will clear all memory resources used
 *          by image upload module and also destroy the mutex which is used for data sharing.
 */
void DeinitImageUpload(void)
{
    UINT8 channelCnt;

	// Set imageUploadPara to default value.
	for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
	{
		//make all image upload parameter default
        MUTEX_LOCK(imageUploadPara[channelCnt].imgUpldInfMutx);
		imageUploadPara[channelCnt].startImgUpldCnt = 0;
        imageUploadPara[channelCnt].imgUpldSysTk = 0;
        MUTEX_UNLOCK(imageUploadPara[channelCnt].imgUpldInfMutx);
		//delete particular channel getImage timer if it is running
		DeleteTimer(&imageUploadPara[channelCnt].imgTimeHandle);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initiates image uploading for particular channel. It updates entry in
 *          imageUploadPara for specified channel and initiates image retrieval request from Camera
 *          Interface module.
 * @param   channelNo
 * @return  SUCCESS/FAIL
 */
BOOL StartImageUpload(UINT8 channelNo)
{
	IMAGE_UPLOAD_CONFIG_t	imgUpldCfg;
	TIMER_INFO_t  	   		imageTimerInfo;

	// validate channel number
    if (channelNo >= getMaxCameraForCurrentVariant())
	{
        return FAIL;
    }

    MUTEX_LOCK(imageUploadPara[channelNo].imgUpldInfMutx);
    imageUploadPara[channelNo].startImgUpldCnt++;

    // If image upload not started then start it
    if (imageUploadPara[channelNo].startImgUpldCnt != 1)
    {
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
        DPRINT(IMAGE_UPLOAD, "image upload already on: [camera=%d]", channelNo);
        return SUCCESS;
    }

    MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
    ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);

    if (GetImage(channelNo, imgUpldCfg.resolution, imageCallBack, CLIENT_CB_TYPE_NATIVE) != CMD_SUCCESS)
    {
        if (imageUploadPara[channelNo].imgTimeHandle == INVALID_TIMER_HANDLE)
        {
            imageTimerInfo.count   = CONVERT_SEC_TO_TIMER_COUNT(IMAGE_FAIL_TIME);
            imageTimerInfo.data    = (UINT32)channelNo;
            imageTimerInfo.funcPtr = &getImageTimer;
            StartTimer(imageTimerInfo, &imageUploadPara[channelNo].imgTimeHandle);
            EPRINT(IMAGE_UPLOAD, "image request fail, timer started: [camera=%d]", channelNo);
        }
        else
        {
            EPRINT(IMAGE_UPLOAD, "get image timer running: [camera=%d]", channelNo);
        }
        return FAIL;
    }

    imageUploadPara[channelNo].imgUpldSysTk = GetSysTick();
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   StopImageUpload
 * @param   channelNo
 * @return  This function stops image uploading for particular camera.
 */
BOOL StopImageUpload(UINT8 channelNo)
{
    if (channelNo >= getMaxCameraForCurrentVariant())
	{
        return FAIL;
    }

    MUTEX_LOCK(imageUploadPara[channelNo].imgUpldInfMutx);
    if(imageUploadPara[channelNo].startImgUpldCnt == 0)
    {
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
        return FAIL;
    }

    imageUploadPara[channelNo].startImgUpldCnt--;
    if(imageUploadPara[channelNo].startImgUpldCnt == 0)
    {
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
        DPRINT(IMAGE_UPLOAD, "image upload stopped: [camera=%d]", channelNo);
    }
    else
    {
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   this function is used to write image file in to our specified file interface, and returns
 *          its local file name and its upload relative url.
 * @param   channelNo
 * @param   fileName
 * @param   relativeUrl
 * @param   imageBuff
 * @param   imageSize
 * @param   relativeUrlLen
 * @return
 */
static BOOL writeToImageFile(UINT8 channelNo, CHARPTR fileName, CHARPTR relativeUrl, CHARPTR imageBuff, UINT32 imageSize, const UINT16 relativeUrlLen)
{
	INT32		fileFd;
	INT32 		writeCnt;
    struct tm   brokenTime;

    if ((imageBuff == NULL) || (imageSize == 0))
	{
        EPRINT(IMAGE_UPLOAD, "image size is not proper: [camera=%d], [imageSize=%d]", channelNo, imageSize);
        return FAIL;
    }

    if (SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
    {
        EPRINT(IMAGE_UPLOAD, "failed to get local time in broken: [camera=%d]", channelNo);
        return FAIL;
    }

    // Image File Name
    snprintf(fileName, MAX_FILE_NAME, "/tmp/"CAMERA_NAME"-"IMAGE_UPL_FILE_NAME,
             GET_CAMERA_NO(channelNo), brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec);

    if (access(fileName, F_OK) == STATUS_OK)
    {
        EPRINT(IMAGE_UPLOAD, "image file already present, skipped: [camera=%d]", channelNo);
        return FAIL;
    }

    // make local copy only. Make relative URL for remote file
    snprintf(relativeUrl, relativeUrlLen, CAMERA_NAME"%c"IMAGE_UPL_DATE_FORMAT IMAGE_UPL_FILE_NAME, GET_CAMERA_NO(channelNo),
             DIR_SEPERATOR, brokenTime.tm_mday, cosecMonthName[brokenTime.tm_mon], brokenTime.tm_year, DIR_SEPERATOR,
            brokenTime.tm_hour, DIR_SEPERATOR, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec);

    //write image to file
    fileFd = open(fileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(fileFd == INVALID_FILE_FD)
    {
        EPRINT(IMAGE_UPLOAD, "failed to open image file: [camera=%d], [err=%s]", channelNo, STR_ERR);
        return FAIL;
    }

    writeCnt = write(fileFd, imageBuff, imageSize);
    close(fileFd);

    if(writeCnt < (INT32)imageSize)
    {
        unlink(fileName);
        EPRINT(IMAGE_UPLOAD, "image file is not proper: [camera=%d]", channelNo);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function invoked for particular channel by camera interface. In this
 *          function we received the image buffer and write it to a file. Then we add it into image
 *          upload queue.
 * @param   channelNo
 * @param   cmdStat
 * @param   imageBuff
 * @param   imageSize
 * @param   clientCbType
 */
static void imageCallBack(UINT8 channelNo, NET_CMD_STATUS_e status, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType)
{
	UINT16 			   		nextTime;
	INT32 			   		recvImgTimeDiffCnt;
	INT32 			   		upldRateCnt;
	TIMER_INFO_t  	   		imageTimerInfo;
	IMAGE_FILE_INFO_t  		imageFile;
	IMAGE_UPLOAD_CONFIG_t 	imgUpldCfg;
	LocalTime_t				localTime = { 0 };
	NAS_FILE_INFO_t			nasFileInfo;
	FTP_FILE_INFO_t    		ftpFileInfo;

	ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);

	do
	{
        if (status != CMD_SUCCESS)
		{
            EPRINT(IMAGE_UPLOAD, "image request failed: [camera=%d], [status=%d]", channelNo, status);
			break;
		}

		//Now write image to file name specified in fileName
        status = CMD_SNAPSHOT_FAILED;
        if(writeToImageFile(channelNo, imageFile.fileName, imageFile.relativeUrl, imageBuff, imageSize, sizeof(imageFile.relativeUrl)) == FAIL)
		{
			break;
		}

		switch(imgUpldCfg.uploadLocation)
		{
            case UPLOAD_TO_FTP_SERVER1:
            case UPLOAD_TO_FTP_SERVER2:
                snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s", imageFile.fileName);
                snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "%s", imageFile.relativeUrl);
                ftpFileInfo.ftpServer = (imgUpldCfg.uploadLocation - UPLOAD_TO_FTP_SERVER1);
                status = FtpImageUpload(channelNo, &ftpFileInfo);
                break;

            case UPLOAD_TO_EMAIL_SERVER:
                status = ProcessEmail(&imgUpldCfg.uploadToEmail, imageFile.fileName);
                break;

            case UPLOAD_TO_NW_DRIVE1:
            case UPLOAD_TO_NW_DRIVE2:
                nasFileInfo.camIndex = channelNo;
                nasFileInfo.nasServer = (imgUpldCfg.uploadLocation - UPLOAD_TO_NW_DRIVE1);
                snprintf(nasFileInfo.localFileName, MAX_FILE_NAME_SIZE, "%s", imageFile.fileName);
                snprintf(nasFileInfo.remoteFile, MAX_FILE_NAME_SIZE, "%s", imageFile.relativeUrl);
                status = NasImageUpload(&nasFileInfo);
                break;

            default:
                EPRINT(IMAGE_UPLOAD, "invld image upload config: [camera=%d]", channelNo);
                break;
		}

		if(status != CMD_SUCCESS)
		{
            EPRINT(IMAGE_UPLOAD, "image request failed: [camera=%d], [status=%d]", channelNo, status);
			unlink(imageFile.fileName);
		}

		if(status == CMD_SERVER_DISABLED)
		{
			status = CMD_SNAPSHOT_SERVER_DISABLED;
		}
		else if(status == CMD_PROCESS_ERROR)
		{
			status = CMD_SNAPSHOT_FAILED;
		}
	}
	while(0);

	//Now call get image function according to image upload configuration rate
    MUTEX_LOCK(imageUploadPara[channelNo].imgUpldInfMutx);

    if (imageUploadPara[channelNo].startImgUpldCnt > 0)
	{
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
		recvImgTimeDiffCnt = ElapsedTick(imageUploadPara[channelNo].imgUpldSysTk);
		upldRateCnt = (SEC_IN_ONE_MIN / imgUpldCfg.uploadImageRate);
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
                EPRINT(IMAGE_UPLOAD, "failed to get local time: [camera=%d]", channelNo);
			}
			nextTime = (10 - (localTime.mSec / TIMER_RESOLUTION_MINIMUM_MSEC));
		}

		if(imageUploadPara[channelNo].imgTimeHandle == INVALID_TIMER_HANDLE)
		{
            imageTimerInfo.count = (nextTime + 1);
            imageTimerInfo.data = (UINT32)channelNo;
            imageTimerInfo.funcPtr = &getImageTimer;
			StartTimer(imageTimerInfo, &imageUploadPara[channelNo].imgTimeHandle);
		}
		else
		{
            EPRINT(IMAGE_UPLOAD, "get image timer running: [camera=%d]", channelNo);
		}
	}
	else
	{
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
		DeleteTimer(&imageUploadPara[channelNo].imgTimeHandle);
        DPRINT(IMAGE_UPLOAD, "image upload stopped: [camera=%d]", channelNo);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was request to FTP with fixed text file for checking connectivity with FTP server
 * @param   channelNo
 */
static void getImageTimer(UINT32 channelNo)
{
    IMAGE_UPLOAD_CONFIG_t   imgUpldCfg;
    TIMER_INFO_t            imageTimerInfo;

	DeleteTimer(&imageUploadPara[channelNo].imgTimeHandle);
    MUTEX_LOCK(imageUploadPara[channelNo].imgUpldInfMutx);

	if(imageUploadPara[channelNo].startImgUpldCnt > 0)
	{
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
		ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);
		imageUploadPara[channelNo].imgUpldSysTk = GetSysTick();

        if(GetImage(channelNo, imgUpldCfg.resolution,imageCallBack, CLIENT_CB_TYPE_NATIVE) != CMD_SUCCESS)
		{
			imageTimerInfo.count   = CONVERT_SEC_TO_TIMER_COUNT(IMAGE_FAIL_TIME);
			imageTimerInfo.data    = (UINT32)channelNo;
			imageTimerInfo.funcPtr = &getImageTimer;
			StartTimer(imageTimerInfo, &imageUploadPara[channelNo].imgTimeHandle);
            EPRINT(IMAGE_UPLOAD, "image request failed: [camera=%d]", channelNo);
		}
	}
	else
	{
        MUTEX_UNLOCK(imageUploadPara[channelNo].imgUpldInfMutx);
        DPRINT(IMAGE_UPLOAD, "image upload stopped: [camera=%d]", channelNo);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   this function is used to upload image at specified location
 * @param   channelNo
 * @param   imgUpldCfg
 * @param   fileName
 * @param   relativeUrl
 * @return
 */
static NET_CMD_STATUS_e uploadImageToLoc(UINT8 channelNo, IMAGE_UPLOAD_CONFIG_t * imgUpldCfg, CHARPTR fileName, CHARPTR relativeUrl)
{
    NET_CMD_STATUS_e    status = CMD_SNAPSHOT_FAILED;
    FTP_HANDLE          ftpHandle;
    NAS_HANDLE          nasHandle;
    NAS_FILE_INFO_t     nasFileInfo;
    FTP_FILE_INFO_t     ftpFileInfo;

	switch(imgUpldCfg->uploadLocation)
	{
        case UPLOAD_TO_FTP_SERVER1:
        case UPLOAD_TO_FTP_SERVER2:
            snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s", fileName);
            snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "%s", relativeUrl);
            ftpFileInfo.ftpServer = (imgUpldCfg->uploadLocation == UPLOAD_TO_FTP_SERVER1) ? FTP_SERVER1 : FTP_SERVER2;
            status = StartFtpUpload(&ftpFileInfo, snapshotFtpCb, channelNo, &ftpHandle);
            break;

        case UPLOAD_TO_EMAIL_SERVER :
            status = ProcessEmail(&imgUpldCfg->uploadToEmail, fileName);
            break;

        case UPLOAD_TO_NW_DRIVE1:
        case UPLOAD_TO_NW_DRIVE2:
            nasFileInfo.nasServer = (imgUpldCfg->uploadLocation == UPLOAD_TO_NW_DRIVE1) ? NAS_SERVER_1 : NAS_SERVER_2;
            nasFileInfo.camIndex = channelNo;
            snprintf(nasFileInfo.localFileName, MAX_FILE_NAME_SIZE, "%s", fileName);
            snprintf(nasFileInfo.remoteFile, MAX_FILE_NAME_SIZE, "%s", relativeUrl);
            status = StartNasUpload(&nasFileInfo, snapshotNasCb, channelNo, &nasHandle);
            break;

        default :
            EPRINT(IMAGE_UPLOAD, "invld image upload config: [camera=%d]", channelNo);
            break;
	}

	if(status != CMD_SUCCESS)
	{
        EPRINT(IMAGE_UPLOAD, "image request failed: [camera=%d], [status=%d]", channelNo, status);
		unlink(fileName);
	}

	if(status == CMD_SERVER_DISABLED)
	{
		status = CMD_SNAPSHOT_SERVER_DISABLED;
	}
	else if(status == CMD_PROCESS_ERROR)
	{
		status = CMD_SNAPSHOT_FAILED;
	}

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get snapshot for given camera from Camera Interface and give this image
 *          to Ftp Module to upload to configured Ftp Server
 * @param   channelNo
 * @param   callback
 * @param   connFd
 * @return
 */
NET_CMD_STATUS_e UploadSnapshot(UINT8 channelNo, NW_CMD_REPLY_CB callback, INT32 connFd)
{
    IMAGE_UPLOAD_CONFIG_t imgUpldCfg;

    if (channelNo >= getMaxCameraForCurrentVariant())
	{
        return CMD_SNAPSHOT_FAILED;
    }

    MUTEX_LOCK(snpshotInfo[channelNo].reqStatusLock);

    if(snpshotInfo[channelNo].reqStatus != FREE)
    {
        MUTEX_UNLOCK(snpshotInfo[channelNo].reqStatusLock);
        EPRINT(IMAGE_UPLOAD, "snapshot request in progress: [camera=%d]", channelNo);
        return CMD_REQUEST_IN_PROGRESS;
    }

    snpshotInfo[channelNo].reqStatus = BUSY;
    MUTEX_UNLOCK(snpshotInfo[channelNo].reqStatusLock);

    //Read Image Upload configurations to retrieve image resolution value
    ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);

    if (GetImage(channelNo, imgUpldCfg.resolution, snapshotCiCb, CLIENT_CB_TYPE_NATIVE) != CMD_SUCCESS)
    {
        MUTEX_LOCK(snpshotInfo[channelNo].reqStatusLock);
        snpshotInfo[channelNo].reqStatus = FREE;
        MUTEX_UNLOCK(snpshotInfo[channelNo].reqStatusLock);
        return CMD_SNAPSHOT_FAILED;
    }

    snpshotInfo[channelNo].callback = callback;
    snpshotInfo[channelNo].connFd = connFd;
    DPRINT(IMAGE_UPLOAD, "snapshot started: [camera=%d]", channelNo);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get snapshot for given camera from Camera Interface and give this image
 *          to Ftp Module to upload to configured Ftp Server on Cosec event
 * @param   channelNo
 * @param   data
 * @return
 */
BOOL UploadCosecSnapshot(UINT8 channelNo, VOIDPTR data)
{
	IMAGE_UPLOAD_CONFIG_t  	imgUpldCfg;
    COSEC_INTEGRATION_t  	*cosecInfo = (COSEC_INTEGRATION_t *)data;

    if (channelNo >= getMaxCameraForCurrentVariant())
	{
        return FAIL;
    }

    MUTEX_LOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
    if (cosecSnapShotInfo[channelNo].reqStatus == FREE)
    {
        cosecSnapShotInfo[channelNo].reqStatus = BUSY;
        MUTEX_UNLOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);

        memcpy(&cosecSnapShotInfo[channelNo].cosecInfo[cosecInfo->doorCommand], cosecInfo, sizeof(COSEC_INTEGRATION_t));
        DPRINT(IMAGE_UPLOAD, "start cosec snapshot upload: [camera=%d], [doorCommand=%d]", channelNo, cosecInfo->doorCommand);
        cosecSnapShotInfo[channelNo].cosecInfo[cosecInfo->doorCommand].enable = ENABLE;
        ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);

        if(GetImage(channelNo, imgUpldCfg.resolution, cosecSnapshotCiCb, CLIENT_CB_TYPE_NATIVE) == CMD_SUCCESS)
        {
            DPRINT(IMAGE_UPLOAD, "snapshot started: [camera=%d]", channelNo);
        }
        else
        {
            MUTEX_LOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
            cosecSnapShotInfo[channelNo].reqStatus = FREE;
            MUTEX_UNLOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
        }

        return SUCCESS;
    }

    if ((cosecInfo->doorCommand == COSEC_IMAGE_UPLD) || (cosecInfo->doorCommand == COSEC_EMAIL_NOTF))
    {
        if(cosecSnapShotInfo[channelNo].cosecInfo[cosecInfo->doorCommand].enable == ENABLE)
        {
            memcpy(&cosecSnapShotInfo[channelNo].cosecInfo[cosecInfo->doorCommand], cosecInfo, sizeof(COSEC_INTEGRATION_t));
            cosecSnapShotInfo[channelNo].cosecInfo[cosecInfo->doorCommand].enable = ENABLE;
            DPRINT(IMAGE_UPLOAD, "cosec imgae upload request: [camera=%d], [destination=%s]",
                   channelNo, (cosecInfo->doorCommand == COSEC_IMAGE_UPLD) ? "ftp/nas" : "email");
        }
        else
        {
            DPRINT(IMAGE_UPLOAD, "snapshot upload is disabled: [camera=%d], [destination=%s]",
                   channelNo, (cosecInfo->doorCommand == COSEC_IMAGE_UPLD) ? "ftp/nas" : "email");
        }
    }

    MUTEX_UNLOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
    EPRINT(IMAGE_UPLOAD, "snapshot request in progress: [camera=%d], [command=%d]", channelNo, cosecInfo->doorCommand);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function for camera interface, this function creates jpeg file for
 *          given image and give that file to FTP module to upload it to configured server
 * @param   channelNo
 * @param   commandResponse
 * @param   imageBuff
 * @param   imageSize
 * @param   clientCbType
 */
static void snapshotCiCb(UINT8 channelNo, NET_CMD_STATUS_e commandResponse, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType)
{
	BOOL					removeFile = FALSE;
	CHAR					relativeUrl[FTP_REMOTE_PATH_LEN];
	IMAGE_UPLOAD_CONFIG_t	imgUpldCfg;

	if(commandResponse == CMD_SUCCESS)
    {
        if(writeToImageFile(channelNo, snpshotInfo[channelNo].fileName, relativeUrl, imageBuff, imageSize, sizeof(relativeUrl)) == SUCCESS)
		{
			ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);

            commandResponse = uploadImageToLoc(channelNo, &imgUpldCfg, snpshotInfo[channelNo].fileName, relativeUrl);
			if(commandResponse == CMD_SUCCESS)
			{
				if(imgUpldCfg.uploadLocation != UPLOAD_TO_EMAIL_SERVER)
				{
                    return;
				}
			}
			else
			{
				removeFile = TRUE;
			}
		}
		else
		{
            EPRINT(IMAGE_UPLOAD, "snapshot image callback failed: [camera=%d]", channelNo);
			commandResponse = CMD_SNAPSHOT_FAILED;
		}
	}
	else
	{
		commandResponse = CMD_SNAPSHOT_FAILED;
        EPRINT(IMAGE_UPLOAD, "get snapshot failed: [camera=%d]", channelNo);
	}

    if(snpshotInfo[channelNo].callback != NULL)
    {
        snpshotInfo[channelNo].callback(commandResponse, snpshotInfo[channelNo].connFd, TRUE);
        snpshotInfo[channelNo].callback = NULL;
        snpshotInfo[channelNo].connFd	= INVALID_CONNECTION;
    }

    if(removeFile == TRUE)
    {
        unlink(snpshotInfo[channelNo].fileName);
    }

    MUTEX_LOCK(snpshotInfo[channelNo].reqStatusLock);
    snpshotInfo[channelNo].reqStatus = FREE;
    MUTEX_UNLOCK(snpshotInfo[channelNo].reqStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function for camera interface, this function creates jpeg file for
 *          given image and give that file to FTP module to upload it to configured server
 * @param   channelNo
 * @param   commandResponse
 * @param   imageBuff
 * @param   imageSize
 * @param   clientCbType
 */
static void cosecSnapshotCiCb(UINT8 channelNo, NET_CMD_STATUS_e commandResponse, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType)
{
	CHAR					relativeUrl[FTP_REMOTE_PATH_LEN];
	IMAGE_UPLOAD_CONFIG_t	imgUpldCfg;
	struct tm 				brokenTime = { 0 };
	CHAR 					macAddress[MAX_MAC_ADDRESS_WIDTH];
	INT32					fileFd, writeCnt;
	COSEC_CMD_NAME_e		doorCommand;

	if(commandResponse == CMD_SUCCESS)
    {
		for(doorCommand = COSEC_IMAGE_UPLD; doorCommand <= COSEC_EMAIL_NOTF; doorCommand ++)
		{
            MUTEX_LOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
            if(cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].enable == DISABLE)
			{
                MUTEX_UNLOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
                EPRINT(IMAGE_UPLOAD, "door command disable: [camera=%d], [doorCommand=%d]", channelNo, doorCommand);
                continue;
            }

            cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].enable = DISABLE;
            MUTEX_UNLOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);

            time_t doorCmdTime = cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorCmdData;
            if (SUCCESS != ConvertLocalTimeInBrokenTm(&doorCmdTime, &brokenTime))
            {
                EPRINT(IMAGE_UPLOAD, "failed to get local time in broken: [camera=%d]", channelNo);
            }

            if(doorCommand == COSEC_IMAGE_UPLD)
            {
                // Image File Name
                snprintf(cosecSnapShotInfo[channelNo].fileName[doorCommand], MAX_FILE_NAME, "/tmp/"COSEC_IMAGE_UPL_FTP_DOORINFO"_"COSEC_IMAGE_UPL_FILE_NAME,
                         cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorType, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorMid,
                         cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorDid,
                         brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec, GET_CAMERA_NO(channelNo));
            }
            else
            {
                // Image File Name
                snprintf(cosecSnapShotInfo[channelNo].fileName[doorCommand], MAX_FILE_NAME, "/tmp/"COSEC_IMAGE_UPL_EMAIL_DOORINFO"_"COSEC_IMAGE_UPL_FILE_NAME,
                         cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorType, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorMid,
                         cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorDid, doorCommand,
                         brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec, GET_CAMERA_NO(channelNo));
            }

            if(access(cosecSnapShotInfo[channelNo].fileName[doorCommand], F_OK) == STATUS_OK)
            {
                EPRINT(IMAGE_UPLOAD, "image file already present, skipped: [camera=%d]", channelNo);
                continue;
            }

            // make local copy only. Make relative URL for remote file
            GetMacAddrPrepare(LAN1_PORT, macAddress);

            if(doorCommand == COSEC_IMAGE_UPLD)
            {
                // Image File Name
                snprintf(relativeUrl, FTP_REMOTE_PATH_LEN, "%c"COSEC_FOLDER"%02d_%s_%04d%c"COSEC_IMAGE_UPL_FTP_DOORINFO"_"COSEC_IMAGE_UPL_FILE_NAME,
                            DIR_SEPERATOR, DIR_SEPERATOR, brokenTime.tm_mday, cosecMonthName[brokenTime.tm_mon],
                            brokenTime.tm_year, DIR_SEPERATOR, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorType,
                            cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorMid, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorDid,
                            brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec, GET_CAMERA_NO(channelNo));
            }
            else
            {
                // Image File Name
                snprintf(relativeUrl, FTP_REMOTE_PATH_LEN, "%c"COSEC_FOLDER"%02d_%s_%04d%c"COSEC_IMAGE_UPL_EMAIL_DOORINFO"_"COSEC_IMAGE_UPL_FILE_NAME,
                            DIR_SEPERATOR, DIR_SEPERATOR, brokenTime.tm_mday, cosecMonthName[brokenTime.tm_mon],
                            brokenTime.tm_year, DIR_SEPERATOR, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorType,
                            cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorMid, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorDid,
                            doorCommand, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec, GET_CAMERA_NO(channelNo));
            }

            //write image to file
            fileFd = open(cosecSnapShotInfo[channelNo].fileName[doorCommand], CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
            if(fileFd == INVALID_FILE_FD)
            {
                EPRINT(IMAGE_UPLOAD, "failed to open image file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                continue;
            }

            writeCnt = write(fileFd, imageBuff, imageSize);
            close(fileFd);

            if(writeCnt < (INT32)imageSize)
            {
                //unlink(fileName);
                EPRINT(IMAGE_UPLOAD, "image file is not proper: [camera=%d]", channelNo);
                continue;
            }

            ReadSingleImageUploadConfig(channelNo, &imgUpldCfg);
            if(cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorCommand != COSEC_EMAIL_NOTF)
            {
                uploadImageToLoc(channelNo, &imgUpldCfg, cosecSnapShotInfo[channelNo].fileName[doorCommand], relativeUrl);
                continue;
            }

            snprintf(cosecEmailParam.emailAddress, MAX_EMAIL_ADDRESS_WIDTH, "%s", cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].emailId);
            snprintf(cosecEmailParam.subject, MAX_EMAIL_SUBJECT_WIDTH, COSEC_EMAIL_SUBJECT);

            if(cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode < COSEC_DOOR_EVENT_MAX)
            {
                createCosecNotifyMsg(channelNo, doorCommand, cosecEmailParam.message);
                DPRINT(IMAGE_UPLOAD, "cosec msg: [camera=%d], [msg=%s]", channelNo, cosecEmailParam.message);
                ProcessEmail(&cosecEmailParam, cosecSnapShotInfo[channelNo].fileName[doorCommand]);
            }
            else
            {
                //unlink(fileName);
                EPRINT(IMAGE_UPLOAD, "wrong event code from door: [camera=%d]", channelNo);
            }
		}
	}
	else
	{
        EPRINT(IMAGE_UPLOAD, "get snapshot failed: [camera=%d]", channelNo);
	}

    MUTEX_LOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
	cosecSnapShotInfo[channelNo].reqStatus = FREE;
    MUTEX_UNLOCK(cosecSnapShotInfo[channelNo].cosecReqStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function for FTP Module, based on response of FTP module this function
 *          reply to the client requested for snapshot
 * @param   ftpHandle
 * @param   ftpResponse
 * @param   userData
 */
static void snapshotFtpCb(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData)
{
    UINT8               channelNo = (UINT8)userData;
    NET_CMD_STATUS_e    cmdResp;

    DPRINT(IMAGE_UPLOAD, "snapshot ftp callback: [camera=%d], [ftpResponse=%d]", channelNo, ftpResponse);
	if(ftpResponse == FTP_SUCCESS)
	{
		cmdResp = CMD_SUCCESS;
	}
	else if(ftpResponse == FTP_CONNECTION_ERROR)
	{
		cmdResp = CMD_SNAPSHOT_CONNECTION_ERROR;
	}
	else if(ftpResponse == FTP_WRITE_PROTECTED)
	{
		cmdResp = CMD_SNAPSHOT_NO_WRITE_PERMISSION;
	}
	else
	{
		cmdResp = CMD_SNAPSHOT_FAILED;
	}

	if(snpshotInfo[channelNo].callback != NULL)
	{
        snpshotInfo[channelNo].callback(cmdResp, snpshotInfo[channelNo].connFd, TRUE);
		snpshotInfo[channelNo].callback = NULL;
		snpshotInfo[channelNo].connFd = INVALID_CONNECTION;
	}

	unlink(snpshotInfo[channelNo].fileName);
	unlink(cosecSnapShotInfo[channelNo].fileName[COSEC_IMAGE_UPLD]);

    MUTEX_LOCK(snpshotInfo[channelNo].reqStatusLock);
	snpshotInfo[channelNo].reqStatus = FREE;
    MUTEX_UNLOCK(snpshotInfo[channelNo].reqStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback function for FTP Module, based on response of FTP module this function
 *          reply to the client requested for snapshot
 * @param   nasHandle
 * @param   nasResponse
 * @param   userData
 */
static void snapshotNasCb(NAS_HANDLE nasHandle, NAS_RESPONSE_e nasResponse, UINT16 userData)
{
    UINT8               channelNo = (UINT8)userData;
    NET_CMD_STATUS_e    cmdResp;

    DPRINT(IMAGE_UPLOAD, "snapshot nas callback: [camera=%d], [nasResponse=%d]", channelNo, nasResponse);
	if(nasResponse == NAS_SUCCESS)
	{
		cmdResp = CMD_SUCCESS;
	}
	else if(nasResponse == NAS_CONNECTION_ERROR)
	{
		cmdResp = CMD_SNAPSHOT_CONNECTION_ERROR;
	}
	else
	{
		cmdResp = CMD_SNAPSHOT_FAILED;
	}

	if(snpshotInfo[channelNo].callback != NULL)
	{
        snpshotInfo[channelNo].callback(cmdResp, snpshotInfo[channelNo].connFd, TRUE);
		snpshotInfo[channelNo].callback = NULL;
		snpshotInfo[channelNo].connFd = INVALID_CONNECTION;
	}

	unlink(snpshotInfo[channelNo].fileName);

    MUTEX_LOCK(snpshotInfo[channelNo].reqStatusLock);
	snpshotInfo[channelNo].reqStatus = FREE;
    MUTEX_UNLOCK(snpshotInfo[channelNo].reqStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function cosec door creates email message
 * @param   channelNo
 * @param   doorCommand
 * @param   msg
 * @return
 */
static BOOL createCosecNotifyMsg(UINT8 channelNo, COSEC_CMD_NAME_e doorCommand, CHARPTR msg)
{
    time_t      doorCmdTime;
    struct tm   brokenTime;

    doorCmdTime = cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorCmdData;
    if (SUCCESS != ConvertLocalTimeInBrokenTm(&doorCmdTime, &brokenTime))
	{
        EPRINT(IMAGE_UPLOAD, "failed to get local time in broken: [camera=%d]", channelNo);
		return FAIL;
	}

    switch(cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode)
    {
        case COSEC_DOOR_EVENT_USER_ALLOWED:
        case COSEC_DOOR_EVENT_USER_DENIED:
        case COSEC_DOOR_EVENT_DURESS_DETECTED:
        {
            snprintf(msg, MAX_EMAIL_MESSAGE_WIDTH, cosecEmailStr[cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode],
                    cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].userName, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].userId,
                    cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorName, brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon),
                    brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min);
        }
        break;

        case COSEC_DOOR_EVENT_USER_NOT_IDENTIFIED:
        case COSEC_DOOR_EVENT_AUX_IN1_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_PANIC_ALARM:
        case COSEC_DOOR_EVENT_DOOR_ABNORMAL:
        case COSEC_DOOR_EVENT_DOOR_FORCE_OPENED:
        case COSEC_DOOR_EVENT_TAMPER_ALARM:
        case COSEC_DOOR_EVENT_AUX_IN2_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_AUX_IN3_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_AUX_IN4_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_AUX_IN5_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_AUX_IN6_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_AUX_IN7_STATUS_CHANGED:
        case COSEC_DOOR_EVENT_AUX_IN8_STATUS_CHANGED:
        {
            snprintf(msg, MAX_EMAIL_MESSAGE_WIDTH, cosecEmailStr[cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode],
                    cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorName, brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon),
                    brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min);
        }
        break;

        case COSEC_DOOR_EVENT_DEAD_MAN_TIMER_EXPIRED:
        {
            snprintf(msg, MAX_EMAIL_MESSAGE_WIDTH, cosecEmailStr[cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode],
                    cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorName, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].userName,
                    cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].userId, brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon),
                    brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min);
        }
        break;

        case COSEC_DOOR_EVENT_INTERCOM_PANIC:
        {
            snprintf(msg, MAX_EMAIL_MESSAGE_WIDTH, cosecEmailStr[cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode],
                    cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].doorName, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].userId,
                    brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min);
        }
        break;

        default:
        {
            EPRINT(IMAGE_UPLOAD, "unhandled cosec door event: [camera=%d], [eventCode=%d]", channelNo, cosecSnapShotInfo[channelNo].cosecInfo[doorCommand].eventCode);
        }
        return FAIL;
    }

	return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
