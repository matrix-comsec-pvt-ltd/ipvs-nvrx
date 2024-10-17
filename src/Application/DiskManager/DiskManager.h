#if !defined DISKMANAGER_H
#define DISKMANAGER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskManger.h
@brief      File containing the prototype of different functions for disk manager module
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "SysTimer.h"
#include "DateTime.h"
#include "CameraDatabase.h"
#include "DiskManagerComn.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// recovery information field size
#define	RECOVERY_FIELD_SIZE             (sizeof(RECOVERY_INFO_t))

// playback session handle 
#define PLAY_SESSION_ID					UINT8
#define INVALID_PLAY_SESSION_HANDLE		(255)

// maximum allowable session for playback
#define	MAX_PLAYBACK_SESSION			getMaxCameraForCurrentVariant()
#define	MAX_PLAYBACK_DISK_SESSION		(MAX_PLAYBACK_SESSION + 1)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    DM_NONE_EVENT0,
    DM_MANUAL_EVENT,
    DM_ALARM_EVENT,
    DM_MAN_ALRM_EVENT,
    DM_SCHEDULE_EVENT,
    DM_MAN_SCHDL_EVENT,
    DM_ALRM_SCHDL_EVENT,
    DM_MAN_ALRM_SCHDL_EVENT,
    DM_COSEC_EVENT,
    DM_MAN_COSEC_EVENT,
    DM_ALRM_COSEC_EVENT,
    DM_MAN_ALRM_COSEC_EVENT,
    DM_SCHDL_COSEC_EVENT,
    DM_MAN_SCHDL_COSEC_EVENT,
    DM_ALRM_SCHDL_COSEC_EVENT,
    DM_ANY_EVENT

}EVENT_e;

typedef enum
{
    REC_EVENT_MANUAL,
    REC_EVENT_ALARM,
    REC_EVENT_SCHEDULE,
    REC_EVENT_COSEC,
    REC_EVENT_MAX

}RECORDING_EVENTS_e;

typedef enum
{
    PLAY_FORWARD,
    STEP_FORWARD,
    PLAY_REVERSE,
    STEP_REVERSE,

}PLAYBACK_CMD_e;

typedef enum
{
    NW_I_FRAME,
    NW_ANY_FRAME,

}NW_FRAME_TYPE_e;

typedef enum
{
    PLAYBACK_FILE_READ_NORMAL = 0,
    PLAYBACK_FILE_READ_ERROR,
    PLAYBACK_FILE_READ_HDD_STOP,
    PLAYBACK_FILE_READ_OVER,
    MAX_PLAYBACK_FILE_STATE

}PLAYBACK_FILE_READ_e;

//This enum provides file status of backup whether it was success or any error occurs.
typedef enum
{
    BACKUP_SUCCESS,
    BACKUP_DISK_FULL,
    BACKUP_NO_OPERATION_HDD,
    BACKUP_FTP_CONN_FAIL,
    BACKUP_FAIL,
    BACKUP_STOPPED,
    BACKUP_RESP_MAX

}DM_BACKUP_STATUS_e;

typedef enum
{
    ASYNC_ALL_SEARCH,
    NORMAL_SEARCH,
    MONTH_WISE_SEARCH,
    DAY_WISE_SEARCH,
    MAX_SEARCH_TYPE

}PLAY_SEARCH_TYPE_e;

typedef enum
{
    PLAYBACK_PLAY,
    BACKUP_PLAY,
    MAX_PLAY_TYPE

}PLAY_TYPE_e;

typedef enum
{
    PLAY_1X,
    PLAY_2X,
    PLAY_4X,
    PLAY_8X,
    PLAY_16X,
    MAX_PLAY_SPEED

}PLAYBACK_SPEED_e;

typedef struct
{
    UINT8               cameraNo;
    EVENT_TYPE_e        eventType;
    struct tm           startTime;
    struct tm           endTime;

}SYNC_CLIP_DATA_t;

// Gives information of what are the metadata needs to stored a frame into file.
typedef struct
{
    STREAM_TYPE_e		mediaType;
    UINT8		    	codecType;
    UINT16				fps;
    UINT8				eventType;
    RESOLUTION_e		resolution;
    FRAME_TYPE_e 		vop;                    // video object plane
    UINT8				noOfRefFrame;
    UINT8 				channelNo;
    LocalTime_t			localTime;

}METADATA_INFO_t;

// gives information of searching criteria of recorded files.
typedef struct
{
    struct tm 			startTime;              // searching start time
    struct tm 			endTime;                // searching end time
    UINT32				channelNo;              // of which camera searching was requested
    EVENT_e				eventType;              // searcg result should be based on this event only
    UINT16				noOfRecord;             // maximum number of record
    RECORD_ON_DISK_e	searchRecStorageType;   //this Parameter would be for in which storage

}SEARCH_CRITERIA_t; //User wants to search recorded data

typedef struct
{
    struct tm 			timeData;                   // searching start time
    BOOL				cameraRecordF[MAX_CAMERA];  // of which camera searching was requested
    EVENT_e				eventType;                  // searcg result should be based on this event only
    RECORD_ON_DISK_e	searchRecStorageType;       //this Parameter would be for in which storage

}SEARCH_CRITERIA_MNTH_DAY_t; //User wants to search recorded data monthwise

typedef struct
{
    struct tm			startTime;              // start time of playback record
    struct tm 			stopTime;               // stop time of playback record
    UINT8				camNo;                  // camera number
    UINT8				overlapFlg;             // indicates that record was overlap
    UINT8				diskId;
    EVENT_e				eventType;
    RECORD_ON_DISK_e	recStorageType;

}PLAY_CNTRL_INFO_t;

typedef struct
{
    UINT32				startCode : 16;
    UINT32				fps : 16;
    UINT32				fshLen;
    UINT32				prevFshPos;
    UINT32				nextFShPos;
    UINT32				iFrmIdxNo;
    UINT32				timeIdxNo;
    UINT32				mediaType : 8;
    UINT32				codecType : 8;
    UINT32				resolution : 8;
    UINT32				vop : 8;
    UINT32				evntType : 8;
    UINT32				diskId : 8;
    UINT32				noOfRefFrame : 8;
    UINT32				cameraNo : 8;
    LocalTime_t			localTime;
    UINT32				reserved2 : 16;

}FSH_INFO_t;

typedef struct
{
    struct tm 			startTime;
    struct tm 			endTime;
    UINT8				channelNo;
    UINT8				recordType;
    UINT8				overlapFlg;
    UINT8				diskId;
    RECORD_ON_DISK_e	recStorageType;

}SEARCH_RESULT_t;

typedef struct
{
    time_t				startTime;              // start time of playback record
    time_t 				stopTime;               // stop time of playback record
    UINT8				camNo;                  // camera number
    UINT8				overlapFlg;             // indicates that record was overlap
    UINT8				diskId;                 // indicates that record was overlap
    EVENT_e				eventType;
    RECORD_ON_DISK_e	recStorageType;

}SEARCH_DATA_t;

typedef struct
{
    UINT32 				date : 8;
    UINT32 				mon : 8;
    UINT32 				year : 16;
    UINT32 				hour : 8;
    UINT32				reserved : 24;

}RECOVERY_INFO_t;

typedef struct
{
    RECOVERY_INFO_t 	recoveryInfoForAvi[MAX_CAMERA];
    CHAR				aviRecMountPoint[MOUNT_POINT_SIZE];

}AVI_CONVERT_t;

//-------------------------------------------------------------------------------------------------
typedef BOOL (*BKUP_RCD_EVENT_CB)(void* privData, UINT64 processedData, BOOL checkStopFlag);
//-------------------------------------------------------------------------------------------------
typedef BOOL (*BKUP_ABORT_CB)(UINT32 userData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitDiskManager(void);
//-------------------------------------------------------------------------------------------------
BOOL DeInitDiskManager(void);
//-------------------------------------------------------------------------------------------------
BOOL BuildRecIndexFromPrevStoredData(CHARPTR mntPoint);
//-------------------------------------------------------------------------------------------------
BOOL GenerateLocalRecDatabase(RECORD_ON_DISK_e recDriveIndx, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
BOOL StartRecordSession(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
BOOL StopRecordSession(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
BOOL GetDmBufferState(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
BOOL WriteMediaFrame(UINT8PTR streamData, UINT32 streamLen, UINT8 channelNo, METADATA_INFO_t *metaData, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SearchRecord(void* userData, PLAY_SEARCH_TYPE_e searchType, RECORD_ON_DISK_e searchRecStorageType,
                              UINT32 sessionIndex, NW_CMD_REPLY_CB callBack, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
UINT8 GetAvailablPlaySession(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e OpenPlaySession(PLAY_CNTRL_INFO_t * playInfo, PLAY_TYPE_e playType, PLAY_SESSION_ID * sessionId);
//-------------------------------------------------------------------------------------------------
BOOL ClosePlaySession(PLAY_SESSION_ID sessionId);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIndexesForFolder(CHARPTR mntPoint, UINT8 channelNo, const RECOVERY_INFO_t *pRecveryInfo, BOOL deleteRetentionFiles);
//-------------------------------------------------------------------------------------------------
BOOL SetPlayPosition(struct tm *playPos, UINT16 mSec, UINT8 audioEnbl, PLAYBACK_CMD_e playCmd, PLAYBACK_SPEED_e speed, PLAY_SESSION_ID sessionId);
//-------------------------------------------------------------------------------------------------
BOOL ReadRecordFrame(PLAY_SESSION_ID sessionId, PLAYBACK_CMD_e playCmd, NW_FRAME_TYPE_e frameType, FSH_INFO_t *fshData,
                     UINT8PTR *frameData, UINT32PTR frameLen, PLAYBACK_FILE_READ_e *errorCode, UINT64PTR nextFrameTime);
//-------------------------------------------------------------------------------------------------
BOOL GetRecoveryStatus(void);
//-------------------------------------------------------------------------------------------------
void SetRecoveryStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
BOOL StartRecovery(CHARPTR 	mntPoint, RECORD_ON_DISK_e iDiskId);
//-------------------------------------------------------------------------------------------------
void ChannelWriterExitWait(void);
//-------------------------------------------------------------------------------------------------
BOOL GetAviWritterStatus(void);
//-------------------------------------------------------------------------------------------------
void ResetChannelBuffer(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
void ResetAllChannelBuffer(void);
//-------------------------------------------------------------------------------------------------
DM_BACKUP_STATUS_e BackupToMedia(CAMERA_BIT_MASK_t cameraMask, time_t hourTime, DM_BACKUP_TYPE_e backupType,
                                 BACKUP_DEVICE_e backupDevice, BKUP_ABORT_CB callback, UINT32 userData);
//-------------------------------------------------------------------------------------------------
DM_BACKUP_STATUS_e BackupToFtp(CAMERA_BIT_MASK_t cameraMask, time_t	hourTime, DM_BACKUP_TYPE_e backupType,
                               FTP_SERVER_e ftpServer, BKUP_ABORT_CB callback, UINT32 userData);
//-------------------------------------------------------------------------------------------------
DM_BACKUP_STATUS_e BackUpSyncRecordToManualDrive(SYNC_CLIP_DATA_t * record, RECORD_ON_DISK_e recStorageDrive,
                                                 BKUP_RCD_EVENT_CB callback, void*	privData, UINT64 stepSize);
//-------------------------------------------------------------------------------------------------
BOOL GetSizeOfRecord(SEARCH_RESULT_t * serchData, RECORD_ON_DISK_e recStorageDrive, UINT16 noOfRecord, UINT64PTR totalSize);
//-------------------------------------------------------------------------------------------------
BOOL OpenBackupSession(SEARCH_RESULT_t * serchData, RECORD_ON_DISK_e recStorageDrive, PLAY_SESSION_ID * playId );
//-------------------------------------------------------------------------------------------------
BOOL CloseBackupSession(PLAY_SESSION_ID playId);
//-------------------------------------------------------------------------------------------------
void GetSizeOfDuration(time_t startTime, time_t endTime, CAMERA_BIT_MASK_t backupCameraMask, UINT64PTR totalSize);
//-------------------------------------------------------------------------------------------------
DM_BACKUP_STATUS_e BackUpRecordToManualDrive(SEARCH_RESULT_t * record, RECORD_ON_DISK_e recStorageDrive,
                                             BKUP_RCD_EVENT_CB callback, void *privData, UINT64 steps, MB_LOCATION_e backupLocation);
//-------------------------------------------------------------------------------------------------
BOOL SearchCamAllEventForSync(SEARCH_CRITERIA_t * pSeachCriteria, SEARCH_DATA_t * searchData,
                              UINT16PTR srchDataLen, UINT8PTR moreData, UINT8PTR overlapIndicator);
//-------------------------------------------------------------------------------------------------
BOOL GetSizeOfMultipleRecord(SYNC_CLIP_DATA_t *clipData, RECORD_ON_DISK_e recStorageDrive, UINT16 noOfRecord, UINT64PTR totalSize);
//-------------------------------------------------------------------------------------------------
UINT8 GetPlayBackStreamCnt(void);
//-------------------------------------------------------------------------------------------------
UINT8 GetRecordingStreamCnt(void);
//-------------------------------------------------------------------------------------------------
void GetStorageCalculationInfo(UINT64PTR timeTakeToFullVolume, UINT64PTR writtingRateInMbps);
//-------------------------------------------------------------------------------------------------
BOOL CheckDestinationStatus(DM_BACKUP_TYPE_e backupType, SB_LOCATION_e backupLocation);
//-------------------------------------------------------------------------------------------------
VOIDPTR StmToAviConvertor(VOIDPTR mntPoint);
//-------------------------------------------------------------------------------------------------
void HandleDiskError(UINT8 channelNo, UINT32 errorCode);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif	// end of file DISKMANAGER_H
