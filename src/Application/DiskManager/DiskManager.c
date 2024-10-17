//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskManger.c
@brief      File containing the defination of different functions for disk manager module
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>
#include <sys/resource.h>

/* Application Includes */
#include "DiskManager.h"
#include "FtpClient.h"
#include "NetworkController.h"
#include "AviWriter.h"
#include "RecordManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	DM_DEVICE_ID				(0x00)
#define I_FRAME_FILE_NAME			"metaData.ifrm"
#define EVENT_FILE_NAME				"metaData.evnt"
#define	TIMEIDX_FILE_NAME			"metaData.tmid"

#define STREAM_FILE_STM_ID_MAX      255
#define STREAM_EXTN					".stm"
#define STREAM_EXTN_OVLP			".ostm"
#define YEAR_MAP_FILE_EXTN			".yrid"
#define BUILD_INDEX_INFO_FILE_NAME	"buildStatus.info"
#define STREAM_FILE_NOT_COMPLETE	"~00_00_00"

// maximum size of FSH for stored stream into harddisk
#define MAX_FSH_SIZE				(sizeof(FSH_INFO_t))

// stream buffer overhead size of 1MB
#define	MAX_STRM_OVERHAD_SIZE       (1 * MEGA_BYTE)

// maximum stream buffer size of 2MB
#define	MAX_STRM_BUFF_SIZE			(2 * MEGA_BYTE)

// stream buffer overhead size of 10KB
#define	MAX_IFRM_OVERHAD_SIZE		(10 * KILO_BYTE)

// maximum buffer size for i frame information of 1KB
#define	MAX_IFRM_BUF_SIZE			(50 * KILO_BYTE)

// Max Stream Info Length
#define MAX_STREAM_FRAME_LENGTH		(2 * MEGA_BYTE)	// 2 times VIDEO_BUF_SIZE

// 512 * 2MB = Total 1GB File size max
#define	MAX_FILE_WRITE_CNT			(512)

// i frame field size
#define IFRAME_FIELD_SIZE		((UINT32)sizeof(IFRM_FIELD_INFO_t))

// event field size
#define EVNT_FIELD_SIZE			(sizeof(EVNT_INFO_t))

// time index field size
#define TIME_INDEX_FIELD_SIZE	(sizeof(TIMEIDX_FIELD_INFO_t))

// end of i frame file indicator
#define	IFRAME_FILE_EOF			(0xAABB)

// end of event file indicator
#define	EVNT_FILE_EOF			(0x0507)

// end of event file indicator
#define	TIMEINDEX_FILE_EOF		(0x0608)

// replace this field with asci value "SF"
#define	STREAM_FILE_SIGN		(0x0102)

// end of stream file indicator
#define	STRM_EOF_INDICATOR		(0x00000305)

// stream file version
#define	STREAM_FILE_VERSION		(0x0503)

// stream file header information size
#define	STREAM_FILE_HDR_SIZE	(sizeof(STRM_FILE_HDR_t))
#define	STREAM_FILE_END_SIZE	(sizeof(STRM_FILE_END_t))

// i frame file sign
#define	IFRAME_FILE_SIGN		(0x0203)

// i frame version number
#define	IFRAME_FILE_VERSION		(0x0503)

// i frame file header
#define	IFRAME_FILE_HDR_SIZE	((UINT32)sizeof(METADATA_FILE_HDR_t))

// event file sign
#define	EVENT_FILE_SIGN			(0x0302)

// event file version
#define	EVENT_FILE_VERSION		(0x0503)

// event file header information size
#define	EVENT_FILE_HDR_SIZE		(sizeof(METADATA_FILE_HDR_t))

// time index file sign
#define	TIMEIDX_FILE_SIGN		(0x0402)

// time index file version
#define	TIMEIDX_FILE_VERSION	(0x0503)

// time index file header information size
#define	TIMEIDX_FILE_HDR_SIZE	(sizeof(METADATA_FILE_HDR_t))

// start code of FSH
#define FSH_START_CODE			(100)

// Invalid minute no. used to stored time index information into file
#define INVALID_MIN_CNT			(-1)

/* Assigned max memory to avoid memory reallocation during frame read */
#define DEFUALT_PLAYBACK_SIZE	(MAX_STREAM_FRAME_LENGTH)

#define AVI_EXTENTION			".avi"
#define AVI_INCOMP_EXTENTION    "_incomplete.avi"
#define MAX_AVI_FILE_SIZE		(2000 * MEGA_BYTE)

#define MAX_METADAT_FILE		(3)

#define BACKUP_STOP_TIMEOUT     (3)
#define FTP_CHECK_CONN_TIMEOUT  (30)

#define BUILD_VERSION           (1)

#define AVI_CNVRT_MSG_QUEUE_MAX	(200)

#define RECOVERY_TIME_OUT		(100)

#define	MAX_BIT_IN_BYTE                 (8)
#define MAX_BIT_IN_LONG                 (64)
#define MAX_RESULT_FOR_HOUR_IN_BYTES    (180)

/* We have scheduled the backup task to limit the backup thread CPU */
#if defined(RK3588_NVRH)
#define BACKUP_TASK_MAX                 2
#else
#define BACKUP_TASK_MAX                 1
#endif

#if defined(RK3568_NVRL)
#define BACKUP_TASK_CHUNK_SLEEP_USEC    (25000)
#else
#define BACKUP_TASK_CHUNK_SLEEP_USEC    (5000)
#endif

/* Define different thread stack size */
#define AVI_CON_THREAD_STACK_SZ         (2*MEGA_BYTE)
#define WRITTER_THREAD_STACK_SZ         (0*MEGA_BYTE)
#define SRCH_RECORD_THREAD_STACK_SZ     (0*MEGA_BYTE)
#define BACKUP_THREAD_STACK_SZ          (2*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    SINGLE_FILE_CONVERT,
    WHOLE_FOLDER_REM,
    MAX_AVI_CONVERT_TYPE

}AVI_CONVERT_TYPE_e;

// Buffering of data
typedef struct
{
    //1. Stream file Name
    //2. Stream file descriptor
    //3. Stream file data
    //4. offset with in the buffer, where the next data going to be written
    //5. indicates stream file should be closed
    CHAR            streamFileName[MAX_FILE_NAME_SIZE];
    INT32           streamFileFd;
    UINT8           streamData[MAX_STRM_BUFF_SIZE + MAX_STRM_OVERHAD_SIZE];
    UINT32          streamOffset;
    BOOL            streamFileClose;

    //1. Iframe File file descriptor
    //2. I Frame data
    //3. offset with in the buffer, where the next data going to be written
    //4. indicates I frame file should be closed
    INT32           iFrmFileFd;
    UINT8           iFrameData[MAX_IFRM_BUF_SIZE + MAX_IFRM_OVERHAD_SIZE];
    UINT32          iFrameOffset;
    BOOL            iFrameFileClose;

    //1. current stream position in stream file
    //2. next stream position in stream file
    //3. current i frame index number
    //4. checking that hour changed or not
    UINT32          prevStreamPos;
    UINT32          nextStreamPos;
    UINT32          nextIFrameIdx;
    struct tm       currTime;

    //1. mutex to protect buffer Parameter
    pthread_mutex_t buffMutex;

}BUFFER_t;

// This structure define field of FSH. The FSH place on top of every media frame
// in stream file.
typedef struct
{
    //1. stream file name
    //2. StreamFile Fd
    //3. number of file one hour folder
    CHAR        filename[MAX_FILE_NAME_SIZE];
    INT32       fileFd;
    UINT8       streamFileIdx;

    // stream file diskId
    UINT8       diskId;

    //1. Indicates current FSH length (FSH field + payload size)
    //2. this indicates current position in file stream
    //3. Indicates previous FSH position for this stream file.
    //4. Indicates next FSH position for this stream file.
    UINT32      curFshLen;
    UINT32      curFshPos;
    UINT32      prevFshPos;
    UINT32      nextFshPos;

    // Local time in sec and ms
    LocalTime_t localTime;

    // last frame written time in milli second
    UINT32      fileStartTime;

    // current recording status
    UINT8       eventStatus;

}STRM_FILE_INFO_t;

typedef struct
{
    UINT16   yearId;
    UINT32   recData[MAX_RECORDING_MODE][MAX_CAMERA][REC_EVENT_MAX];

}RECORD_DATA_t;

// This structure define field of I frame header information.
typedef struct
{
    //1. file descriptor to perform file IO
    //2. i frame index number for current running hour
    //3. Stream position for this I frame.
    INT32   fileFd;
    UINT32  iFrmIdxNo;
    UINT32  streamPos;

}I_FRM_FILE_INFO_t;

// This structure define field of event file information.
typedef struct
{
    //1. file descriptor to perform file IO
    //2. event index number for current hour
    //3. event status
    INT32   fileFd;
    UINT32  evntIdxNo;
    UINT16  eventState;

}EVNT_FILE_INFO_t;

typedef struct
{
    // file descriptor to perform file IO for month file
    INT32   mapFileFd;
    INT8    lastDateIdx;

}YEAR_MAP_FILE_INFO_t;

// This structure define field of time index file information.
typedef struct
{
    //1. file descriptor to perform file IO
    //2. time index number
    //3. for which minute time index file was updated.
    //	 This was initialize in StartRecord session with -1
    INT32   fileFd;
    UINT32  timeIdxNo;
    INT8    lastTimeIdxMin;

}TIME_IDX_FILE_INFO_t;

typedef struct
{
    // By which event recording was started.
    UINT32  eventType : 8;

    // In which file this stream was place.
    UINT32  startFileId : 8;

    // In which file this stream was place.
    UINT32  stopFileId : 8;

    // Indicates stream file is overlap or not
    UINT32  overLapFlg : 8;

    // Event start time.
    UINT32  startTime;

    // Start stream position of this event
    UINT32  startStreamPos;

    // Event end time.
    UINT32  endTime;

    // stop stream position of this event
    UINT32  stopStreamPos;

    // disk id
    UINT32  diskId : 8;

    // reserved
    UINT32  reserved : 24;

}EVNT_INFO_t;

// This structure define field of Event header information.
typedef struct
{
    BOOL        eventPresent;
    BOOL        hourUpdate;

    // Event index number
    UINT32      eventIdx;
    EVNT_INFO_t evntInfo;

}EVENT_INDEX_HEADER_t;

// This structure define session of Disk Manager
typedef struct
{
    //SESSIOIN is active or not
    UINT8 				sessionStatus;

    // first frame second
    BOOL				firstFrameRec;

    // indicates session was overlap with time
    UINT8 				overlapFlg;

    // max 2mb buffer write on disk
    UINT16				streamFileWrCnt;
    time_t				lastHourSec;
    UINT32				sytemTickCnt;
    UINT32				elapsedTickCnt;
    pthread_mutex_t		yearMapFileLock;

    // Recording file info
    STRM_FILE_INFO_t 		streamFileInfo;
    I_FRM_FILE_INFO_t 		iFrameFileInfo;
    EVNT_FILE_INFO_t 		evntFileInfo;
    TIME_IDX_FILE_INFO_t 	timeIdxFileInfo;
    YEAR_MAP_FILE_INFO_t	yearMapFileInfo;

    /* Disk Manager buffer */
    BUFFER_t                mDmBufferMngr;

    /* Schedule event information and copy for RAM */
    EVENT_INDEX_HEADER_t    mScheduleEvent;

    /* Manual event information and copy for RAM */
    EVENT_INDEX_HEADER_t    mManualEvent;

    /* Alarm event information and copy for RAM */
    EVENT_INDEX_HEADER_t    mAlarmEvent;

    /* Cosec event information and copy for RAM */
    EVENT_INDEX_HEADER_t    mCosecEvent;

    /* Store frame in malloc buffer to avoid frame loss when stream file changed (e.g. hour change) */
    METADATA_INFO_t         frameMetaData;
    UINT32                  frameLen;
    UINT8PTR                frameData;

}DM_SESSION_t;

// This structure use while playing video
typedef struct
{
    CHAR        streamFileName[MAX_FILE_NAME_SIZE];
    CHAR        iFrameFile[MAX_FILE_NAME_SIZE];
    UINT8       streamFileIdx;
    INT32       streamFileFd;
    INT32       iFrmFileFd;
    UINT32      fileEndPos;
    UINT32      curStreamPos;
    UINT32      curStreamLen;
    UINT32      prevFshPos;
    UINT32      nextFshPos;
    UINT32      iFrameIdx;
    UINT32      lastIframeF;
    time_t      curTimeSec;

}PLAY_INFO_t;

typedef struct
{
    UINT8PTR    buffPtr;
    UINT32      buffSize;

}PLYBACK_BUFFER_t;

// This structure define session of playback
typedef struct
{
    // Indicates this session running or not
    BOOL 				playStatus;

    //indicates audio present or not
    BOOL 				audioEnbl;

    // at the end of record reached
    BOOL 				recordOver;

    // event starting file Id
    UINT8 				startFileId;

    // event stop file Id
    UINT8 				stopFileId;

    PLAYBACK_SPEED_e	speed;
    CHAR 				playDir[MAX_FILE_NAME_SIZE];

    // event start stream position in start file
    UINT32 				startStreamPos;

    // event stop stream position in stop file
    UINT32 				stopStreamPos;
    time_t				playStartTime;
    time_t				playStopTime;

    PLAY_CNTRL_INFO_t 	playCntrl; // starting time of record playback
    PLAY_INFO_t 		playInfo;
    PLYBACK_BUFFER_t	playBuff;

}PLAYBACK_SESSION_t;

// Used for stored and retrive recovery information
typedef struct
{
    UINT16 				fileSign;
    UINT16 				version;
    UINT32 				nextMetaDataIdx;

}METADATA_FILE_HDR_t;

typedef struct
{
    UINT32 				fileSign : 16;
    UINT32 				version : 16;
    UINT32 				nextStreamPos;
    UINT32 				runFlg : 8;
    UINT32 				backupFlg : 8;
    UINT32 				deviceType : 8;
    UINT32				reserved : 8;

}STRM_FILE_HDR_t;

typedef struct
{
    UINT32 				fileEndIndicator;
    UINT32 				prevFshPos;

}STRM_FILE_END_t;

typedef struct
{
    UINT32 				timeSec;
    UINT32 				curStrmPos;
    UINT32 				strmFileId : 8;
    UINT32 				diskId : 8;
    UINT32				reserved : 16;

}IFRM_FIELD_INFO_t;

typedef struct
{
    UINT32 				min : 8;
    UINT32 				overlapFlg : 8;
    UINT32 				diskId : 8;
    UINT32 				streamFileId : 8;
    UINT32 				fshPos;

}TIMEIDX_FIELD_INFO_t;

typedef struct
{
    pthread_mutex_t		srchMutex;
    VOIDPTR				searchCriteria;
    NW_CMD_REPLY_CB 	callBack;
    INT32 				clientSocket;
    CLIENT_CB_TYPE_e    clientCbType;
    BOOL				searchStatus;
    PLAY_SEARCH_TYPE_e 	searchType;

}SEARCH_THREAD_INFO_t;

typedef union
{
    UINT64		rawData;
    UINT8		convertedData[8];

}CON_UINT64_TO_UINT8_u;

typedef struct
{
    CHAR					fileName[MAX_FILE_NAME_SIZE];
    AVI_CONVERT_TYPE_e 		aviConvertType;

}AVI_CONVERTER_MSG_t;

typedef struct
{
    UINT32					readIdx;
    UINT32					writeIdx;
    AVI_CONVERTER_MSG_t		aviCnvrtMsg[AVI_CNVRT_MSG_QUEUE_MAX];
    pthread_mutex_t 		aviCnvrtCondMutex;
    pthread_cond_t 			aviCnvrtCondSignal;

}AVI_CNVRT_MSG_QUE_t;

typedef struct
{
    pthread_mutex_t writtingRateMutex;
    UINT64          writtenDataSize[MAX_CAMERA];
    UINT64          timeTakenToWriteData[MAX_CAMERA];

}STORAGE_CALC_INFO_t;

typedef struct
{
    UINT8           recordMap[MAX_RESULT_FOR_HOUR_IN_BYTES];
    BOOL            overlapFlag;

}RECORD_MAP_t;

typedef struct
{
    RECORD_MAP_t    eventMap[REC_EVENT_MAX];

}DAY_MAP_t;

typedef struct
{
    DAY_MAP_t       dayMap[MAX_DAYS];

}MONTH_MAP_t;

typedef struct
{
    MONTH_MAP_t     monthMap[MAX_MONTH];

}YEAR_MAP_t;

typedef struct
{
    UINT8	    dayId;
    UINT8       monthId;
    UINT16      yearId;
    DAY_MAP_t   recordsForday[MAX_RECORDING_MODE][MAX_CAMERA];

}RECORD_DATA_DAY_t;

typedef struct
{
    UINT8		buildVersion;
    UINT8		lastBuildCameraId;
    BOOL 		status;
    CHAR 		macAddr[MAX_MAC_ADDRESS_WIDTH];

}BUILD_INFO_t;

typedef struct
{
    UINT8               schedularId;
    DM_BACKUP_TYPE_e    backupType;
    BOOL                cameraBackupStatusF[MAX_CAMERA];
    CHAR                mntPoint[MOUNT_POINT_SIZE];
    CHAR                srcMountPoint[MOUNT_POINT_SIZE];
    CHAR                dstMountPoint[MOUNT_POINT_SIZE];
    struct tm           brokenTime;
    UINT8               mediaNo;
    BACKUP_DEVICE_e     backupDevice;
    pthread_mutex_t		backupTaskMutex;
    BOOL                backupTaskRunStatus;
    DM_BACKUP_STATUS_e  backupStatus;
    UINT32              userData;
    COPY_ABORT_CB       copyAbortCb;

}BACKUP_FILE_XFER_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL isDiskInFault(UINT32 iErrorCode, RECORD_ON_DISK_e iDisk);
//-------------------------------------------------------------------------------------------------
static void closeChnlFiles(UINT8 iChnlNo);
//-------------------------------------------------------------------------------------------------
static BOOL recoverStreamAndIFrameFile(CHAR *pHourFolder, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL checkRecordFolder(UINT8 channelNo, struct tm *curTime);
//-------------------------------------------------------------------------------------------------
static BOOL checkRecordFolderForChnl(UINT8 channelNo, struct tm *curTime);
//-------------------------------------------------------------------------------------------------
static BOOL checkMetaDataFiles(CHARPTR path);
//-------------------------------------------------------------------------------------------------
static BOOL createRecordFolder(UINT8 channelNo, struct tm *curTime, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static void flushWriteBuff(UINT8 channelNo, BOOL streamTerFlg, BOOL iFrmTerFlg, struct tm *curTime, BOOL hourSlotChng);
//-------------------------------------------------------------------------------------------------
static BOOL terMetaDataFiles(UINT8 channelNo, BOOL eventFileTerFlg, BOOL timeIdxtFileTerFlg, BOOL monthDayIndxFileTerFlg, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL enqueFrame(UINT8 channelNo, UINT8PTR streamData, UINT32 streamLen, METADATA_INFO_t *metaData, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static VOIDPTR writterThread(VOIDPTR threadParam);
//-------------------------------------------------------------------------------------------------
static VOIDPTR aviConverter(VOIDPTR threadParam);
//-------------------------------------------------------------------------------------------------
static void closeAllFileOnHddErr(void);
//-------------------------------------------------------------------------------------------------
static BOOL closeStreamFile(BUFFER_t *pFileInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL closeIFrameFile(BUFFER_t *pFileInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL closeEventFile(UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL closeTimeIdxFile(UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL closeMonthDayIdxFiles(UINT8	channelNo);
//-------------------------------------------------------------------------------------------------
static BOOL updateStreamFile(BUFFER_t *fileInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL updateIFrameFile(BUFFER_t *fileInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL updateEventFile(EVENT_INDEX_HEADER_t *eventInfo, EVNT_FILE_INFO_t *eventFileInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL updateEventFileHeader(EVNT_FILE_INFO_t *eventFileInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL updateTimeIdxFile(UINT8 channelNo, struct tm *curTime, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL updateYearMapFile(UINT8 channelNo, UINT8 eventType, struct tm *curTime, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL updateTimeIdxFileHeader(UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL createStreamFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL createIFrameFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL createEventFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL createTimeIdxFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL getAllStreamFileCount(CHARPTR path, UINT32PTR streamFileCount,UINT32PTR aviFileCount);
//-------------------------------------------------------------------------------------------------
static BOOL storeFileRecoveryInfo(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL recoverEventFile(CHARPTR path, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL recoverMonthDayIndxFiles(CHARPTR mntPoint, CHARPTR path, UINT8 channelNo,RECOVERY_INFO_t recveryInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL removeIndexesforGivenHour(CHARPTR mntPoint, UINT8 channelNo, RECOVERY_INFO_t recveryInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL recoverTimeIdxFile(CHARPTR path, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL recoverStreamFile(CHARPTR fileName, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static void clearRecDataForDrive(RECORD_ON_DISK_e disk, UINT8 camCnt);
//-------------------------------------------------------------------------------------------------
static BOOL getLatestHourFolder(CHARPTR mntPoint, UINT8 channelNo, CHARPTR newestFolder, RECOVERY_INFO_t *recoveryInfo, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL getStreamFileName(CHARPTR path, UINT8 fileId, CHARPTR filename, UINT32PTR fileSize);
//-------------------------------------------------------------------------------------------------
static void removeBlankStreamFiles(CHARPTR path);
//-------------------------------------------------------------------------------------------------
static BOOL checkSearchRecordFolder(UINT8 diskCnt, UINT8 channelNo, struct tm *curTime, CHARPTR searchDir, RECORD_ON_DISK_e searchRecStorageType);
//-------------------------------------------------------------------------------------------------
static BOOL checkSearchDateRecordFolder(UINT8 diskCnt, UINT8 channelNo, struct tm *curTime, CHARPTR searchDir, RECORD_ON_DISK_e searchRecStorageType);
//-------------------------------------------------------------------------------------------------
static BOOL checkSearchChnlRecordFolder(UINT8 diskCnt, UINT8 channelNo,CHARPTR searchDir, RECORD_ON_DISK_e searchRecStorageType);
//-------------------------------------------------------------------------------------------------
static BOOL genSrchResult(EVNT_INFO_t *eventData, UINT8 channelNo, UINT8 diskId, SEARCH_DATA_t *searchData,
                          UINT16PTR srchDataLen, RECORD_ON_DISK_e storageDrive);
//-------------------------------------------------------------------------------------------------
static BOOL sortSearchRslt(SEARCH_DATA_t *searchData, SEARCH_RESULT_t *searchRslt, UINT16 srchDataLen, UINT16 maxSortLen, UINT16PTR resltSort);
//-------------------------------------------------------------------------------------------------
static VOIDPTR searchPlayRecord(VOIDPTR threadData);
//-------------------------------------------------------------------------------------------------
static BOOL searchAllEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_RESULT_t *searchData, UINT16PTR srchDataLen,
                           UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL searchAllCamEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_RESULT_t *searchData,
                              UINT16PTR srchDataLen, UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL searchCamAllEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_DATA_t *searchData, UINT16PTR srchDataLen,
                              UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL searchCamEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_DATA_t *searchData, UINT16PTR srchDataLen,
                           UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static void initPlaySession(void);
//-------------------------------------------------------------------------------------------------
static void deInitPlaySession(void);
//-------------------------------------------------------------------------------------------------
static void *backupToMediaThread(void *threadArg);
//-------------------------------------------------------------------------------------------------
static void backupFtpCallBack(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static void getPlayBackDir(PLAY_SESSION_ID playId, CHARPTR playDir);
//-------------------------------------------------------------------------------------------------
static void getPlayBackPosition(PLAY_SESSION_ID playId, UINT8PTR startFile, UINT32PTR startPos);
//-------------------------------------------------------------------------------------------------
static void prepareBackUpFileName(CHARPTR fileName, SEARCH_RESULT_t *record, CHARPTR mountPoint);
//-------------------------------------------------------------------------------------------------
static void appendBackUpEndTime(CHARPTR oldfileName, CHARPTR newfileName, struct tm *endTime);
//-------------------------------------------------------------------------------------------------
static BOOL	normalSearchRecord(SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL	asyncAllSearch(SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL	searchRecordMonth(SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL	searchRecordDay(SEARCH_THREAD_INFO_t *searchThreadInfo);
//-------------------------------------------------------------------------------------------------
static BOOL	isRecordsAvaillableForDay(struct tm *timeData, UINT8 camIndex, RECORD_ON_DISK_e disk, EVENT_e eventCriteria);
//-------------------------------------------------------------------------------------------------
static BOOL writeaviCnvrtMesg(AVI_CONVERTER_MSG_t *aviMsgPtr);
//-------------------------------------------------------------------------------------------------
static void convertStrmToAvi(CHARPTR strmFileName);
//-------------------------------------------------------------------------------------------------
static void setStorageCalculationInfo(UINT64 timeTakeToFullVolume, UINT64 writtingRateInMbps, UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static void initStorageCalculationInfo(void);
//-------------------------------------------------------------------------------------------------
static void testFtpConnection(NET_CMD_STATUS_e status, INT32 clientSocket, BOOL closeConn);
//-------------------------------------------------------------------------------------------------
static BOOL searchSingleChannelPresence(SEARCH_CRITERIA_t *pSeachCriteria);
//-------------------------------------------------------------------------------------------------
static time_t searchStartTimePresence(SEARCH_CRITERIA_t *pSeachCriteria);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR metaFiles[MAX_METADAT_FILE] =
{
    I_FRAME_FILE_NAME,
    EVENT_FILE_NAME,
    TIMEIDX_FILE_NAME,
};

static const UINT32 metaFileHdrSize[MAX_METADAT_FILE] =
{
    IFRAME_FILE_HDR_SIZE,
    EVENT_FILE_HDR_SIZE,
    TIMEIDX_FILE_HDR_SIZE,
};

// Ftp uploading response from ftp client
static FTP_RESPONSE_e		ftpUploadResponse;
static pthread_mutex_t		backupFtpMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t		backupFtpCond = PTHREAD_COND_INITIALIZER;

// Ftp checking test connection
static NET_CMD_STATUS_e		ftpTestUploadResponse;
static pthread_mutex_t		testFtpMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t		testFtpCond = PTHREAD_COND_INITIALIZER;

//Mutex for conditional variables
static pthread_mutex_t 		dmCondMutex = PTHREAD_MUTEX_INITIALIZER;

// This signal used to wake up writter thread to flush buffer data into files.
static pthread_cond_t 		dmCondSignal = PTHREAD_COND_INITIALIZER;

static RECORD_DATA_t		recordInfo[MAX_MONTH];

static RECORD_DATA_DAY_t    dayRecordInfo[MAX_WEEK_DAYS];

// Total number of disk manager session. (per channel one session is allow)
static DM_SESSION_t 		sessionInfo[MAX_CAMERA];

// Total number of play back session.
static PLAYBACK_SESSION_t 	playSession[MAX_CAMERA + 1];
static pthread_mutex_t 		playbckSessionMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t      buildInfoFileLock = PTHREAD_MUTEX_INITIALIZER;

/* this is 64 bit variable which is used for checking which channel has to dump its buffered data into file */
static BOOL 				channelWriterStatus[MAX_CAMERA];
static BOOL                 channelHourSlotChange[MAX_CAMERA];

// thread terminating flag
static BOOL 				dmTerminateFlg;

// disk manager thread id
static pthread_t			dmThreadId;

// recovery status
static BOOL					recoveryStatus;

// mutex lock for recovery
static pthread_mutex_t 		recoveryMutex  = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t      recDataMutex  = PTHREAD_MUTEX_INITIALIZER;

static SEARCH_THREAD_INFO_t	serchThread[MAX_NW_CLIENT];

static UINT32               recordDuration;

static AVI_CNVRT_MSG_QUE_t	aviConvertParam;

static STORAGE_CALC_INFO_t  storageCalculationInfo;

static GENERAL_CONFIG_t     aviRecGenConfig;

static const CHARPTR        storageDriveNameList[MAX_RECORDING_MODE] = {"HDD", "NDD1", "NDD2"};

//-------------------------------------------------------------------------------------------------
static BOOL (*searchFuncPtr[]) (SEARCH_THREAD_INFO_t *searchThreadInfo) =
{
    asyncAllSearch,
    normalSearchRecord,
    searchRecordMonth,
    searchRecordDay
};
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes disk manager, it was recover the entire file in hardisk and
 *          insert proper information in proper file. If event was present in stream file but
 *          corresponding references not present then it will put information of event into event
 *          file. And also creates a thread for flushing stream data into file.
 * @return  SUCCESS
 */
BOOL InitDiskManager(void)
{
    UINT8 chnlId;

    // First read file duration
    ReadGeneralConfig(&aviRecGenConfig);
    recordDuration = (aviRecGenConfig.fileRecordDuration * SEC_IN_ONE_MIN);
    recoveryStatus = STOP;

    for(chnlId = 0; chnlId < MAX_NW_CLIENT; chnlId++)
    {
        serchThread[chnlId].searchStatus = INACTIVE;
        MUTEX_INIT(serchThread[chnlId].srchMutex, NULL);
    }

    // Initialise conditional variable for writer thread
    pthread_cond_init(&dmCondSignal, NULL);
    MUTEX_INIT(recDataMutex, NULL);

    for(chnlId = 0; chnlId < MAX_CAMERA; chnlId++)
    {
        channelWriterStatus[chnlId] = FALSE;
        channelHourSlotChange[chnlId] = FALSE;
        sessionInfo[chnlId].sessionStatus = OFF;
        sessionInfo[chnlId].firstFrameRec = FALSE;
        sessionInfo[chnlId].overlapFlg = 0;
        sessionInfo[chnlId].streamFileWrCnt = 0;
        MUTEX_INIT(sessionInfo[chnlId].mDmBufferMngr.buffMutex, NULL);
        MUTEX_INIT(sessionInfo[chnlId].yearMapFileLock, NULL);
        sessionInfo[chnlId].mDmBufferMngr.streamOffset = 0;
        sessionInfo[chnlId].mDmBufferMngr.iFrameOffset = 0;

        sessionInfo[chnlId].streamFileInfo.fileFd = INVALID_FILE_FD;
        sessionInfo[chnlId].iFrameFileInfo.fileFd = INVALID_FILE_FD;
        sessionInfo[chnlId].evntFileInfo.fileFd = INVALID_FILE_FD;
        sessionInfo[chnlId].timeIdxFileInfo.fileFd = INVALID_FILE_FD;
        sessionInfo[chnlId].yearMapFileInfo.mapFileFd = INVALID_FILE_FD;

        memset(&sessionInfo[chnlId].frameMetaData, 0, sizeof(METADATA_INFO_t));
        sessionInfo[chnlId].frameLen = 0;
        sessionInfo[chnlId].frameData = NULL;
    }

    MUTEX_INIT(buildInfoFileLock, NULL);
    dmTerminateFlg = FALSE;

    //creates a thread for flushing stream data into corresponding files
    if (FAIL == Utils_CreateThread(&dmThreadId, writterThread, NULL, JOINABLE_THREAD, WRITTER_THREAD_STACK_SZ))
    {
        EPRINT(DISK_MANAGER, "fail to create writterThread");
    }
    else if ((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) || (aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT))
    {
        MUTEX_INIT(aviConvertParam.aviCnvrtCondMutex, NULL);
        pthread_cond_init(&aviConvertParam.aviCnvrtCondSignal, NULL);
        aviConvertParam.readIdx = 0;
        aviConvertParam.writeIdx = 0;
        if (FAIL == Utils_CreateThread(NULL, aviConverter, NULL, DETACHED_THREAD, AVI_CON_THREAD_STACK_SZ))
        {
            EPRINT(DISK_MANAGER, "fail to create writterThread");
        }
    }

    // initialize playback session
    initPlaySession();
    initStorageCalculationInfo();
    DPRINT(DISK_MANAGER, "Initialize Disk manager");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initialize disk manager module. It will clear all memory resources used
 *          by disk manager module and also destroy its thread and corresponding conditional variables.
 * @return  SUCCESS
 */
BOOL DeInitDiskManager(void)
{
    /* Deinit all play sessions */
    deInitPlaySession();

    // terminate writterThread
    MUTEX_LOCK(dmCondMutex);
    dmTerminateFlg = TRUE;
    pthread_cond_signal(&dmCondSignal);
    MUTEX_UNLOCK(dmCondMutex);

    pthread_join(dmThreadId, NULL);
    DPRINT(DISK_MANAGER, "Disk Manager De-Initialize successfully");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates a session for disk manager module to stores input  stream into file
 *          to disk. It will also create a session folder in proper directory as well as it also creates
 *          a file to stores a stream data. It will also check that session is already running or not.
 *          It will alloctate memory for pre buffering stream data, I frame data, and event data.
 *          If session is already running then this api return with error code.
 * @param   channelNo
 * @return  SUCCESS/FAIL
 */
BOOL StartRecordSession(UINT8 channelNo)
{
    HDD_CONFIG_t hddConfig;

    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d]", channelNo);
        return FAIL;
    }

    if (TRUE == GetDiskNonFuncStatus(MAX_RECORDING_MODE))
    {
        return FAIL;
    }

    ReadHddConfig(&hddConfig);
    if (STRG_HLT_DISK_NORMAL != GetCameraVolumeHealthStatus(channelNo, hddConfig.recordDisk))
    {
        return FAIL;
    }

    // check that recording for that channel is already running or not.
    DM_SESSION_t *dmSession = &sessionInfo[channelNo];
    if (dmSession->sessionStatus == ON)
    {
        return SUCCESS;
    }

    // Initialize all parameters for event file storage
    memset(&dmSession->mScheduleEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mScheduleEvent.hourUpdate = TRUE;

    memset(&dmSession->mManualEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mManualEvent.hourUpdate = TRUE;

    memset(&dmSession->mAlarmEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mAlarmEvent.hourUpdate = TRUE;

    memset(&dmSession->mCosecEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mCosecEvent.hourUpdate = TRUE;

    dmSession->sessionStatus = ON;
    dmSession->firstFrameRec = FALSE;
    dmSession->overlapFlg = 0;

    memset(&dmSession->streamFileInfo, 0, sizeof(STRM_FILE_INFO_t));
    memset(&dmSession->iFrameFileInfo, 0, sizeof(I_FRM_FILE_INFO_t));
    memset(&dmSession->evntFileInfo, 0, sizeof(EVNT_FILE_INFO_t));
    memset(&dmSession->timeIdxFileInfo, 0, sizeof(TIME_IDX_FILE_INFO_t));
    memset(&dmSession->yearMapFileInfo, 0, sizeof(YEAR_MAP_FILE_INFO_t));

    dmSession->iFrameFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->streamFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->evntFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->timeIdxFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->yearMapFileInfo.mapFileFd = INVALID_FILE_FD;
    dmSession->sytemTickCnt = GetSysTick();
    FREE_MEMORY(dmSession->frameData);
    dmSession->frameLen = 0;
    DPRINT(DISK_MANAGER, "dm session started: [camera=%d]", channelNo);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function closed a disk manager session. It also closed the file which is already
 *          open. It will also removes all resources used by this session.
 * @param   channelNo
 * @return  SUCCESS/FAIL
 */
BOOL StopRecordSession(UINT8 channelNo)
{
    struct tm 		curTime = { 0 };
    UINT32          tErrorCode = INVALID_ERROR_CODE;
    CHAR			tDiskName[MOUNT_POINT_SIZE];
    HDD_CONFIG_t    tHddCnfg = { 0 };

    /* Validate camera index */
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d]", channelNo);
        return FAIL;
    }

    //Get sessionInfo for that camera
    DM_SESSION_t *dmSession = &sessionInfo[channelNo];
    if (dmSession->sessionStatus == OFF)
    {
        return SUCCESS;
    }

    if (dmSession->firstFrameRec == TRUE)
    {
        dmSession->sessionStatus = WAIT;
        DPRINT(DISK_MANAGER, "terminate metadata files: [camera=%d]", channelNo);

        if (aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
        {
            if(terMetaDataFiles(channelNo, TRUE, TRUE, TRUE, &tErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "metadata files close failed: [camera=%d], [err=%s]", channelNo, strerror(tErrorCode));

                tDiskName[0] = '\0';
                if(SUCCESS != GetDiskNameFromMountPoint(dmSession->streamFileInfo.filename, tDiskName))
                {
                    EPRINT(DISK_MANAGER, "fail to get disk name: [camera=%d]", channelNo);
                    return FAIL;
                }

                if(SUCCESS != GetHddConfigFromDiskName(tDiskName, &tHddCnfg))
                {
                    EPRINT(DISK_MANAGER, "fail to get hdd config: [camera=%d]", channelNo);
                    return FAIL;
                }
            }
        }

        time_t totalSec = dmSession->streamFileInfo.localTime.totalSec;
        if (SUCCESS != ConvertLocalTimeInBrokenTm(&totalSec, &curTime))
        {
            EPRINT(DISK_MANAGER, "fail to get converted local time: [camera=%d]", channelNo);
        }

        flushWriteBuff(channelNo, TRUE, TRUE, &curTime, FALSE);
    }

    dmSession->sessionStatus = OFF;
    dmSession->firstFrameRec = FALSE;
    dmSession->lastHourSec = 0;
    dmSession->overlapFlg = 0;
    dmSession->sytemTickCnt = 0;
    dmSession->elapsedTickCnt = 0;
    setStorageCalculationInfo(0,0,channelNo);
    FREE_MEMORY(dmSession->frameData);
    dmSession->frameLen = 0;
    DPRINT(DISK_MANAGER, "dm session stopped: [camera=%d]", channelNo);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function function checks the record folder was present in storage media of given
 *          channel and given time. It will return SUCCESS on folder present otherwise FAIL.
 * @param   channelNo
 * @param   curTime
 * @return  SUCCESS/FAIL
 */
static BOOL checkRecordFolder(UINT8 channelNo, struct tm *curTime)
{
    CHAR	mntPoint[MOUNT_POINT_SIZE]  = "\0";
    CHAR	dateFldr[MAX_FILE_NAME_SIZE];

    if (GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        return FAIL;
    }

    //Check that current system date folder exists
    snprintf(dateFldr, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mntPoint,
             GET_CAMERA_NO(channelNo), curTime->tm_mday, GetMonthName(curTime->tm_mon), curTime->tm_year, curTime->tm_hour);
    if (access(dateFldr, F_OK) != STATUS_OK)
    {
        return FAIL;
    }

    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        if(checkMetaDataFiles(dateFldr) == FAIL)
        {
            RemoveDirectory(dateFldr);
            return FAIL;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function function checks the record folder was present in storage media of given
 *          channel. It will return SUCCESS on folder present otherwise FAIL.
 * @param   channelNo
 * @param   curTime
 * @return
 */
static BOOL checkRecordFolderForChnl(UINT8 channelNo, struct tm *curTime)
{
    CHAR mntPoint[MOUNT_POINT_SIZE] = "\0";
    CHAR filename[MAX_FILE_NAME_SIZE];

    if (GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        return FAIL;
    }

    //Check that current system date folder exists
    snprintf(filename, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR,
             mntPoint, GET_CAMERA_NO(channelNo), curTime->tm_year, YEAR_MAP_FILE_EXTN);
    if (access(filename, F_OK) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "metadata file not present: [camera=%d], [path=%s]", channelNo, filename);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function function checks every metaData file was present or not and also verify its
 *          file size.(size should be minimum atleast header size)
 * @param   path
 * @return  SUCCESS/FAIL
 */
static BOOL checkMetaDataFiles(CHARPTR path)
{
    UINT8           cnt;
    CHAR            filename[MAX_FILE_NAME_SIZE];
    struct stat     tFileSize;
    INT32           tFileFd = INVALID_FILE_FD;

    for (cnt = 0; cnt < MAX_METADAT_FILE; cnt++)
    {
        snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s", path, metaFiles[cnt]);
        if (access(filename, F_OK) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "metadata file not present: [path=%s]", filename);
            return FAIL;
        }

        tFileFd = open(filename, READ_ONLY_MODE);
        if (tFileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open metadata file: [path=%s], [err=%s]", filename, STR_ERR);
            return FAIL;
        }

        if(STATUS_OK != fstat(tFileFd, &tFileSize))
        {
            close(tFileFd);
            EPRINT(DISK_MANAGER, "fail to get fstat of metadata file: [path=%s], [err=%s]", filename, STR_ERR);
            return FAIL;
        }

        if((UINT32)tFileSize.st_size < metaFileHdrSize[cnt])
        {
            close(tFileFd);
            EPRINT(DISK_MANAGER, "metadata file header is corrupted: [path=%s]", filename);
            return FAIL;
        }

        close(tFileFd);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the record folder was present in storage media of given channel and given time.
 * @param   channelNo
 * @param   curTime
 * @param   pErrorCode
 * @return  It will return SUCCESS on folder present otherwise FAIL
 */
static BOOL createRecordFolder(UINT8 channelNo, struct tm *curTime, UINT32PTR pErrorCode)
{
    CHAR	mntPoint[MOUNT_POINT_SIZE] = "\0";
    CHAR	dateFldr[MAX_FILE_NAME_SIZE];

    /* set invalid error code */
    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if (FAIL == GetRecordingPath(channelNo, mntPoint))
    {
        return FAIL;
    }

    snprintf(dateFldr, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mntPoint,
             GET_CAMERA_NO(channelNo), curTime->tm_mday, GetMonthName(curTime->tm_mon), curTime->tm_year, curTime->tm_hour);

    //Create recursive directory
    if(CreateRecursiveDir(dateFldr, pErrorCode) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to create recursive dir: [camera=%d], [path=%s]", channelNo, dateFldr);
    }
    else if(createStreamFile(curTime, channelNo, pErrorCode) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to create stream file: [camera=%d]", channelNo);
    }
    else if(storeFileRecoveryInfo(curTime, channelNo, pErrorCode) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to store recovery info: [camera=%d]", channelNo);
    }
    else if(aviRecGenConfig.recordFormatType == REC_AVI_FORMAT)
    {
        /* No need to create other metadata files for other than native recordings */
        return SUCCESS;
    }
    else if(createEventFile(curTime, channelNo, pErrorCode) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to create event file: [camera=%d]", channelNo);
    }
    else if(createIFrameFile(curTime, channelNo, pErrorCode) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to create i-frame file: [camera=%d]", channelNo);
    }
    else if(createTimeIdxFile(curTime, channelNo, pErrorCode) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to create time-index file: [camera=%d]", channelNo);
    }
    else
    {
        return SUCCESS;
    }

    // Delete whole folder
    RemoveDirectory(dateFldr);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the record folder was present in storage media of given channel and given time.
 * @param   channelNo
 * @param   streamTerFlg
 * @param   iFrmTerFlg
 * @param   curTime
 * @param   hourSlotChng
 */
static void flushWriteBuff(UINT8 channelNo, BOOL streamTerFlg, BOOL iFrmTerFlg, struct tm *curTime, BOOL hourSlotChng)
{
    sessionInfo[channelNo].mDmBufferMngr.streamFileFd = sessionInfo[channelNo].streamFileInfo.fileFd;
    sessionInfo[channelNo].mDmBufferMngr.prevStreamPos = sessionInfo[channelNo].streamFileInfo.prevFshPos;
    sessionInfo[channelNo].mDmBufferMngr.nextStreamPos = sessionInfo[channelNo].streamFileInfo.nextFshPos;

    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        sessionInfo[channelNo].mDmBufferMngr.iFrmFileFd = sessionInfo[channelNo].iFrameFileInfo.fileFd;
        sessionInfo[channelNo].mDmBufferMngr.nextIFrameIdx = sessionInfo[channelNo].iFrameFileInfo.iFrmIdxNo;
    }
    memcpy(&sessionInfo[channelNo].mDmBufferMngr.currTime, curTime, sizeof(struct tm));

    snprintf(sessionInfo[channelNo].mDmBufferMngr.streamFileName, MAX_FILE_NAME_SIZE, "%s", sessionInfo[channelNo].streamFileInfo.filename);

    MUTEX_LOCK(sessionInfo[channelNo].mDmBufferMngr.buffMutex);
    sessionInfo[channelNo].elapsedTickCnt = ElapsedTick(sessionInfo[channelNo].sytemTickCnt);
    sessionInfo[channelNo].sytemTickCnt = GetSysTick();

    if(streamTerFlg == TRUE)
    {
        sessionInfo[channelNo].mDmBufferMngr.streamFileClose = TRUE;
    }

    if(iFrmTerFlg == TRUE)
    {
        sessionInfo[channelNo].mDmBufferMngr.iFrameFileClose = TRUE;
    }

    MUTEX_UNLOCK(sessionInfo[channelNo].mDmBufferMngr.buffMutex);

    MUTEX_LOCK(dmCondMutex);
    channelWriterStatus[channelNo] = TRUE;
    if(hourSlotChng == TRUE)
    {
        channelHourSlotChange[channelNo] = TRUE;
    }
    pthread_cond_signal(&dmCondSignal);
    MUTEX_UNLOCK(dmCondMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function terminates metadata files as per closing state given in input.
 * @param   channelNo
 * @param   eventFileTerFlg
 * @param   timeIdxtFileTerFlg
 * @param   monthDayIndxFileTerFlg
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return
 */
static BOOL terMetaDataFiles(UINT8 channelNo, BOOL eventFileTerFlg, BOOL timeIdxtFileTerFlg, BOOL monthDayIndxFileTerFlg, UINT32PTR pErrorCode)
{
    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    // entry for every event that was present
    if (sessionInfo[channelNo].mScheduleEvent.evntInfo.startTime != 0)
    {
        // make schedule stop event
        sessionInfo[channelNo].mScheduleEvent.evntInfo.eventType = DM_SCHEDULE_EVENT;
        sessionInfo[channelNo].mScheduleEvent.evntInfo.endTime = sessionInfo[channelNo].streamFileInfo.localTime.totalSec;
        sessionInfo[channelNo].mScheduleEvent.evntInfo.stopFileId = sessionInfo[channelNo].streamFileInfo.streamFileIdx;
        sessionInfo[channelNo].mScheduleEvent.evntInfo.stopStreamPos = sessionInfo[channelNo].streamFileInfo.prevFshPos;
        if (updateEventFile(&sessionInfo[channelNo].mScheduleEvent, &sessionInfo[channelNo].evntFileInfo, pErrorCode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to update schedule event: [camera=%d]", channelNo);
            return FAIL;
        }

        memset(&sessionInfo[channelNo].mScheduleEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
        sessionInfo[channelNo].mScheduleEvent.hourUpdate = TRUE;
    }

    if (sessionInfo[channelNo].mManualEvent.evntInfo.startTime != 0)
    {
        // make manual stop event
        sessionInfo[channelNo].mManualEvent.evntInfo.eventType = DM_MANUAL_EVENT;
        sessionInfo[channelNo].mManualEvent.evntInfo.endTime = sessionInfo[channelNo].streamFileInfo.localTime.totalSec;
        sessionInfo[channelNo].mManualEvent.evntInfo.stopFileId = sessionInfo[channelNo].streamFileInfo.streamFileIdx;
        sessionInfo[channelNo].mManualEvent.evntInfo.stopStreamPos = sessionInfo[channelNo].streamFileInfo.prevFshPos;

        if (updateEventFile(&sessionInfo[channelNo].mManualEvent, &sessionInfo[channelNo].evntFileInfo, pErrorCode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to update manual event: [camera=%d]", channelNo);
            return FAIL;
        }

        memset(&sessionInfo[channelNo].mManualEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
        sessionInfo[channelNo].mManualEvent.hourUpdate = TRUE;
    }

    if(sessionInfo[channelNo].mAlarmEvent.evntInfo.startTime != 0)
    {
        // make alarm stop event
        sessionInfo[channelNo].mAlarmEvent.evntInfo.eventType = DM_ALARM_EVENT;
        sessionInfo[channelNo].mAlarmEvent.evntInfo.endTime = sessionInfo[channelNo].streamFileInfo.localTime.totalSec;
        sessionInfo[channelNo].mAlarmEvent.evntInfo.stopFileId = sessionInfo[channelNo].streamFileInfo.streamFileIdx;
        sessionInfo[channelNo].mAlarmEvent.evntInfo.stopStreamPos = sessionInfo[channelNo].streamFileInfo.prevFshPos;

        if (updateEventFile(&sessionInfo[channelNo].mAlarmEvent, &sessionInfo[channelNo].evntFileInfo, pErrorCode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to update alarm event: [camera=%d]", channelNo);
            return FAIL;
        }

        memset(&sessionInfo[channelNo].mAlarmEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
        sessionInfo[channelNo].mAlarmEvent.hourUpdate = TRUE;
    }

    if(sessionInfo[channelNo].mCosecEvent.evntInfo.startTime != 0)
    {
        // make alarm stop event
        sessionInfo[channelNo].mCosecEvent.evntInfo.eventType = DM_COSEC_EVENT;
        sessionInfo[channelNo].mCosecEvent.evntInfo.endTime = sessionInfo[channelNo].streamFileInfo.localTime.totalSec;
        sessionInfo[channelNo].mCosecEvent.evntInfo.stopFileId = sessionInfo[channelNo].streamFileInfo.streamFileIdx;
        sessionInfo[channelNo].mCosecEvent.evntInfo.stopStreamPos = sessionInfo[channelNo].streamFileInfo.prevFshPos;

        if (updateEventFile(&sessionInfo[channelNo].mCosecEvent, &sessionInfo[channelNo].evntFileInfo, pErrorCode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to update cosec event: [camera=%d]", channelNo);
            return FAIL;
        }

        memset(&sessionInfo[channelNo].mCosecEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
        sessionInfo[channelNo].mCosecEvent.hourUpdate = TRUE;
    }

    if(eventFileTerFlg == TRUE)
    {
        if(closeEventFile(channelNo, pErrorCode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to close event file: [camera=%d]", channelNo);
            return FAIL;
        }
    }

    if(timeIdxtFileTerFlg == TRUE)
    {
        if(closeTimeIdxFile(channelNo, pErrorCode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to close time-index file: [camera=%d]", channelNo);
            return FAIL;
        }
    }

    if(monthDayIndxFileTerFlg == TRUE)
    {
        if(closeMonthDayIdxFiles(channelNo) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to close year map file: [camera=%d]", channelNo);
            return FAIL;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates FSH and place it into stream buffer with media frame. It will also
 *          generates I frame information if present and enque into I frame buffer. This function also
 *          updates event file on the based of current frame received. And also update time index file.
 * @param   channelNo
 * @param   streamData
 * @param   streamLen
 * @param   metaData
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL enqueFrame(UINT8 channelNo, UINT8PTR streamData, UINT32 streamLen, METADATA_INFO_t *metaData, UINT32PTR pErrorCode)
{
    UINT32 					streamPayloadSize = sizeof(FSH_INFO_t) + streamLen;
    FSH_INFO_t 				fshData;
    struct tm  				brokenTime;
    IFRM_FIELD_INFO_t 		iFrmFieldInfo;
    I_FRM_FILE_INFO_t 		*iFrmFileInfo = NULL;
    STRM_FILE_INFO_t 		*strmFileInfo;
    EVNT_FILE_INFO_t 		*evntFileInfo = NULL;
    TIME_IDX_FILE_INFO_t 	*timeIdxFileInfo = NULL;

    /* set invalid error code */
    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if((sessionInfo[channelNo].mDmBufferMngr.streamOffset + streamPayloadSize) >= (MAX_STRM_BUFF_SIZE + MAX_STRM_OVERHAD_SIZE))
    {
        EPRINT(DISK_MANAGER, "stream length is more than buf size: [camera=%d], [streamPayloadSize=%d]", channelNo, streamPayloadSize);
        return UNKNOWN;
    }

    strmFileInfo = &sessionInfo[channelNo].streamFileInfo;
    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        iFrmFileInfo = &sessionInfo[channelNo].iFrameFileInfo;
        evntFileInfo = &sessionInfo[channelNo].evntFileInfo;
        timeIdxFileInfo = &sessionInfo[channelNo].timeIdxFileInfo;
    }

    time_t totalSec = metaData->localTime.totalSec;
    ConvertLocalTimeInBrokenTm(&totalSec, &brokenTime);

    /* Make FSH for stream file frame information */
    //set start code
    fshData.startCode = FSH_START_CODE;

    //set current FSH Length
    fshData.fshLen = streamPayloadSize;

    //set previous FSH position
    fshData.prevFshPos = strmFileInfo->prevFshPos;
    strmFileInfo->prevFshPos = strmFileInfo->curFshPos;

    //update current FSH position
    strmFileInfo->curFshPos += streamPayloadSize;

    //Set next FSH position
    strmFileInfo->nextFshPos = strmFileInfo->curFshPos;
    fshData.nextFShPos = strmFileInfo->nextFshPos;

    //set media Type
    fshData.mediaType = metaData->mediaType;

    //set Codec type
    fshData.codecType = metaData->codecType;

    // set resolution
    fshData.resolution = metaData->resolution;

    //set FPS
    fshData.fps = metaData->fps;

    //set VOP
    fshData.vop = metaData->vop;

    //set no of reference frame
    fshData.noOfRefFrame = metaData->noOfRefFrame;

    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        if((metaData->vop == I_FRAME) && (metaData->mediaType == STREAM_TYPE_VIDEO))
        {
            // I frame was detected then updated I frame index number in channel FSH data structure
            iFrmFileInfo->iFrmIdxNo++;

            // add information in I frame add record time
            iFrmFieldInfo.timeSec = metaData->localTime.totalSec;

            // disk Id add
            iFrmFieldInfo.diskId = strmFileInfo->diskId;

            // Add file Id
            iFrmFieldInfo.strmFileId = strmFileInfo->streamFileIdx;

            // set stream position
            iFrmFieldInfo.curStrmPos = strmFileInfo->prevFshPos;

            //Make reserved  byte to 0
            iFrmFieldInfo.reserved = 0;
        }
    }

    // set time in sec  for this frame
    fshData.localTime.totalSec = metaData->localTime.totalSec;
    strmFileInfo->localTime.totalSec = metaData->localTime.totalSec;

    // set time in milli second
    fshData.localTime.mSec = metaData->localTime.mSec;
    strmFileInfo->localTime.mSec = metaData->localTime.mSec;

    // Video frame not present
    if(metaData->mediaType == STREAM_TYPE_AUDIO)
    {
        // Received frame was type of AUDIO so all information related to I frame was invalid
        fshData.diskId = 0;

        // stored I frame index Number
        fshData.iFrmIdxNo = 0;
    }
    else // Video Frame is present
    {
        //This was not needed so remove diskId field
        fshData.diskId = 1;

        // stored I frame index Number
        if(aviRecGenConfig.recordFormatType == REC_AVI_FORMAT)
        {
            fshData.iFrmIdxNo = 0;
        }
        else
        {
            fshData.iFrmIdxNo = iFrmFileInfo->iFrmIdxNo;
        }
    }

    // Make reserved byte to 0
    fshData.cameraNo = GET_CAMERA_NO(channelNo);
    fshData.reserved2 = 0;

    // Set event state in stream file
    fshData.evntType = metaData->eventType;

    // stored event type set if schecule event was present
    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        if(metaData->eventType & DM_SCHEDULE_EVENT)
        {
            if(sessionInfo[channelNo].mScheduleEvent.hourUpdate == TRUE)
            {
                sessionInfo[channelNo].mScheduleEvent.eventPresent = TRUE;
                sessionInfo[channelNo].mScheduleEvent.hourUpdate = FALSE;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.eventType = DM_SCHEDULE_EVENT;
                sessionInfo[channelNo].mScheduleEvent.eventIdx = evntFileInfo->evntIdxNo;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.startTime = metaData->localTime.totalSec;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.diskId = strmFileInfo->diskId;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.startFileId = strmFileInfo->streamFileIdx;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.startStreamPos = strmFileInfo->prevFshPos;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.endTime = 0;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.stopFileId = 0;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.stopStreamPos = 0;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.overLapFlg = sessionInfo[channelNo].overlapFlg;
                sessionInfo[channelNo].mScheduleEvent.evntInfo.reserved = 0;
                if (updateEventFile(&sessionInfo[channelNo].mScheduleEvent, evntFileInfo, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to update schedule event: [camera=%d]", channelNo);
                    return FAIL;
                }

                // increment event index count as we write successful event in event file
                evntFileInfo->evntIdxNo++;
                if(updateEventFileHeader(evntFileInfo, pErrorCode) == FAIL)
                {
                    return FAIL;
                }
            }
        }
        else if(sessionInfo[channelNo].mScheduleEvent.eventPresent == TRUE)
        {
            sessionInfo[channelNo].mScheduleEvent.eventPresent = FALSE;
            sessionInfo[channelNo].mScheduleEvent.hourUpdate = TRUE;
            sessionInfo[channelNo].mScheduleEvent.evntInfo.eventType = DM_SCHEDULE_EVENT;
            sessionInfo[channelNo].mScheduleEvent.evntInfo.endTime = metaData->localTime.totalSec;
            sessionInfo[channelNo].mScheduleEvent.evntInfo.stopFileId = strmFileInfo->streamFileIdx;
            sessionInfo[channelNo].mScheduleEvent.evntInfo.stopStreamPos = strmFileInfo->prevFshPos;

            // generate schedule stop event for this recording.
            if (updateEventFile(&sessionInfo[channelNo].mScheduleEvent, evntFileInfo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update schedule event: [camera=%d]", channelNo);
                return FAIL;
            }

            memset(&sessionInfo[channelNo].mScheduleEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
            sessionInfo[channelNo].mScheduleEvent.hourUpdate = TRUE;
        }

        // stored manual event type
        if(metaData->eventType & DM_MANUAL_EVENT)
        {
            if(sessionInfo[channelNo].mManualEvent.hourUpdate == TRUE)
            {
                sessionInfo[channelNo].mManualEvent.eventPresent = TRUE;
                sessionInfo[channelNo].mManualEvent.hourUpdate = FALSE;
                sessionInfo[channelNo].mManualEvent.evntInfo.eventType = DM_MANUAL_EVENT;
                sessionInfo[channelNo].mManualEvent.eventIdx = evntFileInfo->evntIdxNo;
                sessionInfo[channelNo].mManualEvent.evntInfo.startTime = metaData->localTime.totalSec;
                sessionInfo[channelNo].mManualEvent.evntInfo.diskId = strmFileInfo->diskId;
                sessionInfo[channelNo].mManualEvent.evntInfo.startFileId = strmFileInfo->streamFileIdx;
                sessionInfo[channelNo].mManualEvent.evntInfo.startStreamPos = strmFileInfo->prevFshPos;
                sessionInfo[channelNo].mManualEvent.evntInfo.endTime = 0;
                sessionInfo[channelNo].mManualEvent.evntInfo.stopFileId = 0;
                sessionInfo[channelNo].mManualEvent.evntInfo.stopStreamPos = 0;
                sessionInfo[channelNo].mManualEvent.evntInfo.overLapFlg = sessionInfo[channelNo].overlapFlg;
                sessionInfo[channelNo].mManualEvent.evntInfo.reserved = 0;
                if(updateEventFile(&sessionInfo[channelNo].mManualEvent, evntFileInfo, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to update manual event: [camera=%d]", channelNo);
                    return FAIL;
                }

                // increment event index count as we write successful event in event file
                evntFileInfo->evntIdxNo++;
                if(updateEventFileHeader(evntFileInfo, pErrorCode) == FAIL)
                {
                    return FAIL;
                }
            }
        }
        else if(sessionInfo[channelNo].mManualEvent.eventPresent == TRUE)
        {
            sessionInfo[channelNo].mManualEvent.eventPresent = FALSE;
            sessionInfo[channelNo].mManualEvent.hourUpdate = TRUE;
            sessionInfo[channelNo].mManualEvent.evntInfo.eventType = DM_MANUAL_EVENT;
            sessionInfo[channelNo].mManualEvent.evntInfo.endTime = metaData->localTime.totalSec;
            sessionInfo[channelNo].mManualEvent.evntInfo.stopFileId = strmFileInfo->streamFileIdx;
            sessionInfo[channelNo].mManualEvent.evntInfo.stopStreamPos = strmFileInfo->prevFshPos;
            if(updateEventFile(&sessionInfo[channelNo].mManualEvent, evntFileInfo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update manual event: [camera=%d]", channelNo);
                return FAIL;
            }

            memset(&sessionInfo[channelNo].mManualEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
            sessionInfo[channelNo].mManualEvent.hourUpdate = TRUE;
        }

        // stored alarm event type
        if (metaData->eventType & DM_ALARM_EVENT)
        {
            if (sessionInfo[channelNo].mAlarmEvent.hourUpdate == TRUE)
            {
                sessionInfo[channelNo].mAlarmEvent.eventPresent = TRUE;
                sessionInfo[channelNo].mAlarmEvent.hourUpdate = FALSE;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.eventType = DM_ALARM_EVENT;
                sessionInfo[channelNo].mAlarmEvent.eventIdx = evntFileInfo->evntIdxNo;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.startTime = metaData->localTime.totalSec;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.diskId = strmFileInfo->diskId;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.startFileId = strmFileInfo->streamFileIdx;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.startStreamPos = strmFileInfo->prevFshPos;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.endTime = 0;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.stopFileId = 0;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.stopStreamPos = 0;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.overLapFlg = sessionInfo[channelNo].overlapFlg;
                sessionInfo[channelNo].mAlarmEvent.evntInfo.reserved = 0;
                if(updateEventFile(&sessionInfo[channelNo].mAlarmEvent, evntFileInfo, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to update alarm event: [camera=%d]", channelNo);
                    return FAIL;
                }

                // increment event index count as we write successful event in event file
                evntFileInfo->evntIdxNo++;
                if(updateEventFileHeader(evntFileInfo, pErrorCode) == FAIL)
                {
                    return FAIL;
                }
            }
        }
        else if (sessionInfo[channelNo].mAlarmEvent.eventPresent == TRUE)
        {
            sessionInfo[channelNo].mAlarmEvent.eventPresent = FALSE;
            sessionInfo[channelNo].mAlarmEvent.hourUpdate = TRUE;
            sessionInfo[channelNo].mAlarmEvent.evntInfo.eventType = DM_ALARM_EVENT;
            sessionInfo[channelNo].mAlarmEvent.evntInfo.endTime = metaData->localTime.totalSec;
            sessionInfo[channelNo].mAlarmEvent.evntInfo.stopFileId = strmFileInfo->streamFileIdx;
            sessionInfo[channelNo].mAlarmEvent.evntInfo.stopStreamPos = strmFileInfo->prevFshPos;
            if(updateEventFile(&sessionInfo[channelNo].mAlarmEvent, evntFileInfo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update alarm event: [camera=%d]", channelNo);
                return FAIL;
            }

            memset(&sessionInfo[channelNo].mAlarmEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
            sessionInfo[channelNo].mAlarmEvent.hourUpdate = TRUE;
        }

        // stored alarm event type
        if (metaData->eventType & DM_COSEC_EVENT)
        {
            if (sessionInfo[channelNo].mCosecEvent.hourUpdate == TRUE)
            {
                sessionInfo[channelNo].mCosecEvent.eventPresent = TRUE;
                sessionInfo[channelNo].mCosecEvent.hourUpdate = FALSE;
                sessionInfo[channelNo].mCosecEvent.evntInfo.eventType = DM_COSEC_EVENT;
                sessionInfo[channelNo].mCosecEvent.eventIdx = evntFileInfo->evntIdxNo;
                sessionInfo[channelNo].mCosecEvent.evntInfo.startTime = metaData->localTime.totalSec;
                sessionInfo[channelNo].mCosecEvent.evntInfo.diskId = strmFileInfo->diskId;
                sessionInfo[channelNo].mCosecEvent.evntInfo.startFileId = strmFileInfo->streamFileIdx;
                sessionInfo[channelNo].mCosecEvent.evntInfo.startStreamPos = strmFileInfo->prevFshPos;
                sessionInfo[channelNo].mCosecEvent.evntInfo.endTime = 0;
                sessionInfo[channelNo].mCosecEvent.evntInfo.stopFileId = 0;
                sessionInfo[channelNo].mCosecEvent.evntInfo.stopStreamPos = 0;
                sessionInfo[channelNo].mCosecEvent.evntInfo.overLapFlg = sessionInfo[channelNo].overlapFlg;
                sessionInfo[channelNo].mCosecEvent.evntInfo.reserved = 0;
                if(updateEventFile(&sessionInfo[channelNo].mCosecEvent, evntFileInfo, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to update cosec event: [camera=%d]", channelNo);
                    return FAIL;
                }

                // increment event index count as we write successful event in event file
                evntFileInfo->evntIdxNo++;
                if(updateEventFileHeader(evntFileInfo, pErrorCode) == FAIL)
                {
                    return FAIL;
                }
            }
        }
        else if (sessionInfo[channelNo].mCosecEvent.eventPresent == TRUE)
        {
            sessionInfo[channelNo].mCosecEvent.eventPresent = FALSE;
            sessionInfo[channelNo].mCosecEvent.hourUpdate = TRUE;
            sessionInfo[channelNo].mCosecEvent.evntInfo.eventType = DM_COSEC_EVENT;
            sessionInfo[channelNo].mCosecEvent.evntInfo.endTime = metaData->localTime.totalSec;
            sessionInfo[channelNo].mCosecEvent.evntInfo.stopFileId = strmFileInfo->streamFileIdx;
            sessionInfo[channelNo].mCosecEvent.evntInfo.stopStreamPos = strmFileInfo->prevFshPos;
            if(updateEventFile(&sessionInfo[channelNo].mCosecEvent, evntFileInfo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update cosec event: [camera=%d]", channelNo);
                return FAIL;
            }

            memset(&sessionInfo[channelNo].mCosecEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
            sessionInfo[channelNo].mCosecEvent.hourUpdate = TRUE;
        }

        // check minute was update. if minute was update then we need to update time index file
        if(brokenTime.tm_min != timeIdxFileInfo->lastTimeIdxMin)
        {
            if(updateYearMapFile(metaData->channelNo, metaData->eventType, &brokenTime, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update year map file: [camera=%d]", channelNo);
                return FAIL;
            }

            //update time index file
            if (updateTimeIdxFile(metaData->channelNo, &brokenTime, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update time-index file: [camera=%d]", channelNo);
                return FAIL;
            }

            // Update time index number as we write time index file
            timeIdxFileInfo->timeIdxNo++;
            if(updateTimeIdxFileHeader(channelNo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to update time-index file header: [camera=%d]", channelNo);
                return FAIL;
            }

            timeIdxFileInfo->lastTimeIdxMin = brokenTime.tm_min;
        }
    }

    if(aviRecGenConfig.recordFormatType == REC_AVI_FORMAT)
    {
        fshData.timeIdxNo = 0;
    }
    else
    {
        fshData.timeIdxNo = timeIdxFileInfo->timeIdxNo;
    }

    memcpy((sessionInfo[channelNo].mDmBufferMngr.streamData + sessionInfo[channelNo].mDmBufferMngr.streamOffset), &fshData, sizeof(FSH_INFO_t));

    // set buffer offset position for streaming
    sessionInfo[channelNo].mDmBufferMngr.streamOffset += sizeof(FSH_INFO_t);

    // load payload
    memcpy((sessionInfo[channelNo].mDmBufferMngr.streamData + sessionInfo[channelNo].mDmBufferMngr.streamOffset), streamData, streamLen);
    sessionInfo[channelNo].mDmBufferMngr.streamOffset += streamLen;
    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        if((fshData.vop == I_FRAME) && (fshData.mediaType == STREAM_TYPE_VIDEO))
        {
            // copy i frame data
            memcpy(sessionInfo[channelNo].mDmBufferMngr.iFrameData + sessionInfo[channelNo].mDmBufferMngr.iFrameOffset, &iFrmFieldInfo, sizeof(IFRM_FIELD_INFO_t));
            sessionInfo[channelNo].mDmBufferMngr.iFrameOffset += sizeof(IFRM_FIELD_INFO_t);
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives states of disk manager buffer
 * @param   channelNo
 * @return  IF full then FAIL else SUCCESS.
 */
BOOL GetDmBufferState(UINT8 channelNo)
{
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d]", channelNo);
        return FAIL;
    }

    MUTEX_LOCK(dmCondMutex);
    BOOL dmBuffState = (FALSE == channelWriterStatus[channelNo]) ? SUCCESS : FAIL;
    MUTEX_UNLOCK(dmCondMutex);
    return dmBuffState;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was received the stream data, FSH header is make using metadata information.
 *          And then it will buffered into predefine buffering. After buffering was completed or time
 *          of that file was complete, buffered data is dump into file.
 * @param   streamData - Stream data which is to be written into file.
 * @param   streamLen - Stream Length
 * @param   channelNo
 * @param   metaData - Metadata provides information while making FSH for this stream.
 * @param   pErrorCode - Error code pointer filled with appropiate error code in case of any failure on Disk operation(eg read, write, mkdir etc..)
 * @return  SUCCESS/FAIL
 */
BOOL WriteMediaFrame(UINT8PTR streamData, UINT32 streamLen, UINT8 channelNo, METADATA_INFO_t *metaData, UINT32PTR pErrorCode)
{
    BOOL					retVal = FAIL;
    BOOL					strFileCloseF;
    BOOL					isEnqueFrame = TRUE;
    INT32					readCnt;
    INT32					iFrmFileFd;
    INT32					evntFileFd;
    INT32					timeFileFd;
    INT32					mapFileFd;
    UINT32 					fileDuration;
    CHAR					mntPoint[MOUNT_POINT_SIZE] = "\0";
    CHAR					fileName[MAX_FILE_NAME_SIZE];
    struct 					tm brokenTime = { 0 };
    struct 					tm lastBrokenTime = { 0 };
    METADATA_FILE_HDR_t		iFrmFileInfo;
    METADATA_FILE_HDR_t		evntFileInfo;
    METADATA_FILE_HDR_t		timeIdxFileInfo;
    YEAR_MAP_t				yearRecMap;
    DM_SESSION_t 			*dmSession;
    GENERAL_CONFIG_t		generalCfg;

    /* set invalid error code */
    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d]", channelNo);
        return UNKNOWN;
    }

    if(streamLen > MAX_STREAM_FRAME_LENGTH) // 2 times VIDEO_BUF_SIZE
    {
        EPRINT(DISK_MANAGER, "stream data length is more than buffer size: [camera=%d], [streamLen=%d]", channelNo, streamLen);
    }

    if(metaData == NULL)
    {
        EPRINT(DISK_MANAGER, "metadata is null: [camera=%d]", channelNo);
        return UNKNOWN;
    }

    //Get this session
    dmSession = &sessionInfo[channelNo];
    if (dmSession->sessionStatus != ON)
    {
        return UNKNOWN;
    }

    if(GetDiskNonFuncStatus(MAX_RECORDING_MODE) == TRUE)
    {
        return UNKNOWN;
    }

    // Convert time_t(metadata->timeSec) into broken doen time into brokenTime
    time_t totalSec = metaData->localTime.totalSec;
    if(SUCCESS != ConvertLocalTimeInBrokenTm(&totalSec, &brokenTime))
    {
        EPRINT(DISK_MANAGER, "fail to convert local time: [camera=%d]", channelNo);
    }

    if(dmSession->firstFrameRec == FALSE)
    {
        DPRINT(DISK_MANAGER, "first frame rcvd: [camera=%d]", channelNo);
        // Check record folder
        if(checkRecordFolder(channelNo, &brokenTime) == FAIL)
        {
            if(createRecordFolder(channelNo, &brokenTime, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to create record folder: [camera=%d]", channelNo);
                return FAIL;
            }
        }
        else
        {
            if(storeFileRecoveryInfo(&brokenTime, channelNo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to store the recovery file info: [camera=%d]", channelNo);
                return FAIL;
            }

            // Create stream file
            if(createStreamFile(&brokenTime, channelNo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to create stream file: [camera=%d]", channelNo);
                return FAIL;
            }

            if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
            {
                if(GetRecordingPath(channelNo, mntPoint) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    return FAIL;
                }

                //-------------------------------------------------------------------------------------------------------------
                snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
                         brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, EVENT_FILE_NAME);

                evntFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(evntFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to create event file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    return FAIL;
                }

                // Read its header information of I frame file
                readCnt = Utils_Read(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, pErrorCode);
                if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read from event file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(evntFileFd);
                    return FAIL;
                }

                // set i frame file name and file FD
                if(-1 == lseek(evntFileFd, ((evntFileInfo.nextMetaDataIdx * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE), SEEK_SET))
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to seek event file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(evntFileFd);
                    return FAIL;
                }

                // Sets I frame index number
                dmSession->evntFileInfo.evntIdxNo = evntFileInfo.nextMetaDataIdx;
                dmSession->evntFileInfo.fileFd = evntFileFd;

                //-------------------------------------------------------------------------------------------------------------
                snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
                         brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, I_FRAME_FILE_NAME);

                iFrmFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(iFrmFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to create i-frame file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    return FAIL;
                }

                // Read its header information of I frame file
                readCnt = Utils_Read(iFrmFileFd, &iFrmFileInfo, IFRAME_FILE_HDR_SIZE, pErrorCode);
                if(readCnt != (INT32)IFRAME_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read from i-frame file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(iFrmFileFd);
                    return FAIL;
                }

                // set i frame file name and file FD
                if(-1 == lseek(iFrmFileFd, ((iFrmFileInfo.nextMetaDataIdx * IFRAME_FIELD_SIZE) + IFRAME_FILE_HDR_SIZE), SEEK_SET))
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to seek i-frame file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(iFrmFileFd);
                    return FAIL;
                }

                // Sets I frame index number
                dmSession->iFrameFileInfo.iFrmIdxNo	= iFrmFileInfo.nextMetaDataIdx;
                dmSession->iFrameFileInfo.fileFd = iFrmFileFd;

                //-------------------------------------------------------------------------------------------------------------
                snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
                         brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, TIMEIDX_FILE_NAME);

                // open time index file
                timeFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if (timeFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to create time-index file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    return FAIL;
                }

                // Read its header information of I frame file
                readCnt = Utils_Read(timeFileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, pErrorCode);
                if(readCnt != (INT32)TIMEIDX_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read from time-index file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(timeFileFd);
                    return FAIL;
                }

                if(-1 == lseek(timeFileFd, ((timeIdxFileInfo.nextMetaDataIdx * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE), SEEK_SET))
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to seek time-index file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(timeFileFd);
                    return FAIL;
                }

                // Sets time index number
                dmSession->timeIdxFileInfo.timeIdxNo = timeIdxFileInfo.nextMetaDataIdx;

                // set time index file name and file FD
                dmSession->timeIdxFileInfo.fileFd = timeFileFd;
                dmSession->timeIdxFileInfo.lastTimeIdxMin = INVALID_MIN_CNT;
            }
        }

        //-------------------------------------------------------------------------------------------------------------
        if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
        {
            if(GetRecordingPath(channelNo, mntPoint) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
                return FAIL;
            }

            // make file name
            snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR,
                     mntPoint, GET_CAMERA_NO(channelNo), brokenTime.tm_year, YEAR_MAP_FILE_EXTN);

            if(checkRecordFolderForChnl(channelNo, &brokenTime) == FAIL)
            {
                // Create a file for month file to store day index
                MUTEX_LOCK(dmSession->yearMapFileLock);
                mapFileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
                MUTEX_UNLOCK(dmSession->yearMapFileLock);
                if (mapFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to create year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    return FAIL;
                }

                memset(&yearRecMap, 0, sizeof(YEAR_MAP_t));
                if(Utils_Write(mapFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode) !=sizeof(YEAR_MAP_t))
                {
                    EPRINT(DISK_MANAGER, "fail to write year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                    close(mapFileFd);
                    if (unlink(fileName) == STATUS_OK)
                    {
                        EPRINT(DISK_MANAGER, "year map file removed: [camera=%d], [path=%s]", channelNo, fileName);
                    }
                    else
                    {
                        EPRINT(DISK_MANAGER, "fail to remove year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    }
                    return FAIL;
                }

                CloseFileFd(&dmSession->yearMapFileInfo.mapFileFd);
                dmSession->yearMapFileInfo.mapFileFd = mapFileFd;
                dmSession->yearMapFileInfo.lastDateIdx = 0;
            }
            else
            {
                mapFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(mapFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    return FAIL;
                }

                readCnt = Utils_Read(mapFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode);
                if(readCnt != (INT32)sizeof(YEAR_MAP_t))
                {
                    EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(mapFileFd);
                    return FAIL;
                }

                if(-1 == lseek(mapFileFd, 0, SEEK_SET))
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to seek year map file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                    CloseFileFd(&dmSession->streamFileInfo.fileFd);
                    close(mapFileFd);
                    return FAIL;
                }

                CloseFileFd(&dmSession->yearMapFileInfo.mapFileFd);
                dmSession->yearMapFileInfo.mapFileFd = mapFileFd;
                dmSession->yearMapFileInfo.lastDateIdx = 0;
            }
        }

        brokenTime.tm_min = 0;
        brokenTime.tm_sec = 0;
        ConvertLocalTimeInSec(&brokenTime, &dmSession->lastHourSec);
        dmSession->firstFrameRec = TRUE;
    }

    // File was made based on user selected time duration. If this duration reaches we need to create a new file.
    // So here we are checking that duration of that file was complete or not
    fileDuration = (metaData->localTime.totalSec - dmSession->streamFileInfo.fileStartTime);
    if ((fileDuration >= recordDuration) || ((metaData->localTime.totalSec - dmSession->lastHourSec) >= 3600))
    {
        isEnqueFrame = FALSE;
        if (fileDuration >= recordDuration)
        {
            dmSession->streamFileInfo.fileStartTime = metaData->localTime.totalSec;
        }

        if((metaData->localTime.totalSec - dmSession->lastHourSec) >= 3600)
        {
            ReadGeneralConfig(&generalCfg);
            recordDuration = (generalCfg.fileRecordDuration * SEC_IN_ONE_MIN);
            DPRINT(DISK_MANAGER, "recording hour changed: [camera=%d], [fileRecordDuration=%dmin]", channelNo, generalCfg.fileRecordDuration);

            // Convert time_t(metadata->timeSec) into broken doen time into brokenTime
            time_t totalSec = dmSession->streamFileInfo.localTime.totalSec;
            if(SUCCESS != ConvertLocalTimeInBrokenTm(&totalSec, &lastBrokenTime))
            {
                EPRINT(DISK_MANAGER, "fail to convert local time: [camera=%d]", channelNo);
            }

            flushWriteBuff(channelNo, TRUE, TRUE, &lastBrokenTime, TRUE);
            if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
            {
                // terminate metadata files (EVENT and time index)
                if(terMetaDataFiles(channelNo, TRUE, TRUE, FALSE, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to close metadata files: [camera=%d]", channelNo);
                    return FAIL;
                }
            }

            dmSession->overlapFlg = 0;

            // here we need to check the record folder because it was possible that new hour also
            // contains some recorded data, in case of time rollback by any case
            if(checkRecordFolder(channelNo, &brokenTime) == FAIL)
            {
                // create a record folder
                DPRINT(DISK_MANAGER, "hour folder not present: [camera=%d]", channelNo);
                if(createRecordFolder(channelNo, &brokenTime, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to create record folder: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
                    return FAIL;
                }
            }
            else
            {
                // create stream file
                DPRINT(DISK_MANAGER, "hour folder present: [camera=%d]", channelNo);
                if(createStreamFile(&brokenTime, channelNo, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to create stream file: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
                    return FAIL;
                }

                if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
                {
                    if(GetRecordingPath(channelNo, mntPoint) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
                        return FAIL;
                    }

                    snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
                             brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, EVENT_FILE_NAME);

                    evntFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                    if (evntFileFd == INVALID_FILE_FD)
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        return FAIL;
                    }

                    // Read its header information of I frame file
                    readCnt = Utils_Read(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, pErrorCode);
                    if (readCnt != (INT32)EVENT_FILE_HDR_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        close(evntFileFd);
                        return FAIL;
                    }

                    // set i frame file name and file FD
                    if(-1 == lseek(evntFileFd, ((evntFileInfo.nextMetaDataIdx * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE), SEEK_SET))
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to seek event file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        close(evntFileFd);
                        return FAIL;
                    }

                    // Sets I frame index number
                    dmSession->evntFileInfo.evntIdxNo = evntFileInfo.nextMetaDataIdx;
                    dmSession->evntFileInfo.fileFd = evntFileFd;

                    snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
                             brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, I_FRAME_FILE_NAME);

                    iFrmFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                    if (iFrmFileFd == INVALID_FILE_FD)
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to open i-frame file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        return FAIL;
                    }

                    // Read its header information of I frame file
                    readCnt = Utils_Read(iFrmFileFd, &iFrmFileInfo, IFRAME_FILE_HDR_SIZE, pErrorCode);
                    if (readCnt != (INT32)IFRAME_FILE_HDR_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read i-frame file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        close(iFrmFileFd);
                        return FAIL;
                    }

                    // set i frame file name and file FD
                    if(-1 == lseek(iFrmFileFd, ((iFrmFileInfo.nextMetaDataIdx * IFRAME_FIELD_SIZE) + IFRAME_FILE_HDR_SIZE), SEEK_SET))
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to seek i-frame file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        close(iFrmFileFd);
                        return FAIL;
                    }

                    // Sets I frame index number
                    dmSession->iFrameFileInfo.iFrmIdxNo	= iFrmFileInfo.nextMetaDataIdx;
                    dmSession->iFrameFileInfo.fileFd = iFrmFileFd;

                    snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
                             brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, TIMEIDX_FILE_NAME);

                    // open time index file
                    timeFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                    if(timeFileFd == INVALID_FILE_FD)
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to open time index file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        return FAIL;
                    }

                    // Read its header information of I frame file
                    readCnt = Utils_Read(timeFileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, pErrorCode);

                    if(readCnt != (INT32)TIMEIDX_FILE_HDR_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read time index file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        close(timeFileFd);
                        return FAIL;
                    }

                    if(-1 == lseek(timeFileFd, ((timeIdxFileInfo.nextMetaDataIdx * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE), SEEK_SET))
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to seek time index file: [camera=%d], [err=%s]", channelNo, STR_ERR);
                        CloseFileFd(&dmSession->streamFileInfo.fileFd);
                        close(timeFileFd);
                        return FAIL;
                    }

                    // Sets time index number
                    dmSession->timeIdxFileInfo.timeIdxNo = timeIdxFileInfo.nextMetaDataIdx;

                    // set time index file name and file FD
                    dmSession->timeIdxFileInfo.fileFd = timeFileFd;
                    dmSession->timeIdxFileInfo.lastTimeIdxMin = INVALID_MIN_CNT;
                }
            }

            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &dmSession->lastHourSec);
        }
        else
        {
            DPRINT(DISK_MANAGER, "file duration over: [camera=%d]", channelNo);
            flushWriteBuff(channelNo, TRUE, FALSE, &brokenTime, FALSE);

            // create stream file
            if(createStreamFile(&brokenTime, channelNo, pErrorCode) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to create stream file: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
                return FAIL;
            }
        }
    }

    //-------------------------------------------------------------------------------------------------------------
    if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
    {
        if(brokenTime.tm_mday != dmSession->yearMapFileInfo.lastDateIdx)
        {
            isEnqueFrame = FALSE;
            if(checkRecordFolderForChnl(channelNo, &brokenTime) == FAIL)
            {
                if(GetRecordingPath(channelNo, mntPoint) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
                    return FAIL;
                }

                // make file name
                snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR,
                         mntPoint, GET_CAMERA_NO(channelNo), brokenTime.tm_year, YEAR_MAP_FILE_EXTN);

                // Create a file for minute index
                MUTEX_LOCK(dmSession->yearMapFileLock);
                mapFileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
                MUTEX_UNLOCK(dmSession->yearMapFileLock);
                if (mapFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    return FAIL;
                }

                memset(&yearRecMap, 0, sizeof(YEAR_MAP_t));
                if(Utils_Write(mapFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode) !=sizeof(YEAR_MAP_t))
                {
                    EPRINT(DISK_MANAGER, "fail to write year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
                    close(mapFileFd);
                    if(unlink(fileName) == STATUS_OK)
                    {
                        DPRINT(DISK_MANAGER, "year map file removed: [camera=%d], [path=%s]", channelNo, fileName);
                    }
                    else
                    {
                        EPRINT(DISK_MANAGER, "fail to remove year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
                    }
                    return FAIL;
                }

                CloseFileFd(&dmSession->yearMapFileInfo.mapFileFd);
                dmSession->yearMapFileInfo.mapFileFd = mapFileFd;
            }

            dmSession->yearMapFileInfo.lastDateIdx = brokenTime.tm_mday;
        }
    }

    /* On hour/file change, we need to dump current buffer into disk and need to create new file. frame buffer writter thread
     * will dump the buffer into disk. We cannot dump current frame in previous file because that time is elapsed as well as
     * we cannot dump or queue current frame in new file because frame buffer writter thread is writting previous file.
     * Hence, We will keep current frame in malloc buffer and will queue on next iteration along with next frame */
    if(isEnqueFrame == TRUE)
    {
        /* Do we have a frame in buffer to dump? */
        if (dmSession->frameLen)
        {
            /* First queue previous frame. Expecting buffer is empty */
            retVal = enqueFrame(channelNo, dmSession->frameData, dmSession->frameLen, &dmSession->frameMetaData, pErrorCode);
            if(retVal == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to queue frame in internal buffer: [camera=%d]", channelNo);
                return FAIL;
            }

            /* Reset frame from buffer which was pending */
            DPRINT(DISK_MANAGER, "previous frame written: [camera=%d], [frameLen=%d]", channelNo, dmSession->frameLen);
            dmSession->frameLen = 0;
            FREE_MEMORY(dmSession->frameData);
        }

        /* Queue frame in writter buffer */
        retVal = enqueFrame(channelNo, streamData, streamLen, metaData, pErrorCode);
        if(retVal == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to queue frame in internal buffer: [camera=%d]", channelNo);
            return FAIL;
        }

        /* Check enough space in stream buffer or i-frame buffer */
        if((dmSession->mDmBufferMngr.streamOffset > MAX_STRM_BUFF_SIZE)
                || (dmSession->mDmBufferMngr.iFrameOffset > MAX_IFRM_BUF_SIZE) || (retVal == UNKNOWN))
        {
            strFileCloseF = (dmSession->streamFileWrCnt >= MAX_FILE_WRITE_CNT) ? TRUE : FALSE;
            flushWriteBuff(channelNo, strFileCloseF, FALSE, &brokenTime, FALSE);
            dmSession->streamFileWrCnt++;
            if(strFileCloseF == TRUE)
            {
                if(createStreamFile(&brokenTime, channelNo, pErrorCode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to create stream file: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
                    return FAIL;
                }
            }
        }
    }
    else
    {
        /* We will not keep audio frame but we must have to keep video frame. If we discard current video frame then decoder
         * will not identify proper reference of consecutive frames till next i-frame and Playback will stuck for a moment. */
        if (metaData->mediaType != STREAM_TYPE_VIDEO)
        {
            WPRINT(DISK_MANAGER, "no need to keep audio frame in buffer: [camera=%d], [frameLen=%d]", channelNo, streamLen);
        }
        else
        {
            /* Ideally it should not happen. We cannot keep multiple frames in buffer */
            if (dmSession->frameLen)
            {
                /* If it happens then implement logic to buffer multiple frames */
                EPRINT(DISK_MANAGER, "previous frame discarded from buffer: [camera=%d], [frameLen=%d]", channelNo, dmSession->frameLen);
            }

            /* Free previous frame (if it is) and allocate new memory */
            FREE_MEMORY(dmSession->frameData);
            dmSession->frameData = malloc(streamLen);
            if (dmSession->frameData == NULL)
            {
                EPRINT(DISK_MANAGER, "fail to alloc memory for frame: [camera=%d], [frameLen=%d]", channelNo, streamLen);
                return FAIL;
            }

            /* Store frame data in buffer and queue on next iteration */
            dmSession->frameLen = streamLen;
            dmSession->frameMetaData = *metaData;
            memcpy(dmSession->frameData, streamData, streamLen);
            DPRINT(DISK_MANAGER, "keep frame in buffer for next write: [camera=%d], [frameLen=%d]", channelNo, dmSession->frameLen);
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This search recorded file from input search criteria and creates list of matches search criteria as output.
 * @param   userData - Search criteria
 * @param   searchType - Type of search --> Async, Month wise, Day wise etc.
 * @param   searchRecStorageType - Search storage media
 * @param   sessionIndex -
 * @param   callBack
 * @param   clientSocket
 * @param   clientCbType
 * @return
 */
NET_CMD_STATUS_e SearchRecord(void* userData, PLAY_SEARCH_TYPE_e searchType, RECORD_ON_DISK_e searchRecStorageType,
                              UINT32 sessionIndex, NW_CMD_REPLY_CB callBack, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    // validate input parameter
    if (userData == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    /* MAX_RECORDING_MODE means for searching the recording device is use */
    if(((SEARCH_CRITERIA_t*)userData)->searchRecStorageType == MAX_RECORDING_MODE)
    {
        HDD_CONFIG_t tHddConfig;
        ReadHddConfig(&tHddConfig);
        ((SEARCH_CRITERIA_t*)userData)->searchRecStorageType = tHddConfig.recordDisk;
        searchRecStorageType = tHddConfig.recordDisk;
    }

    if (searchRecStorageType != ALL_DRIVE)
    {
        if (FALSE == IsStorageOperationalForRead(searchRecStorageType))
        {
            /* When we don't use network drive for recording then status of NDD will be set "non functional (not usable)"
             * and due to that search will not allowed for same. Hence considered it as pocessing error. */
            NET_CMD_STATUS_e retVal = GiveMediaStatus(searchRecStorageType);
            EPRINT(DISK_MANAGER, "storage is not connected or faulty: [storageType=%d], [status=%d]", searchRecStorageType, retVal);
            return ((retVal == CMD_SUCCESS) ? CMD_PROCESS_ERROR : retVal);
        }
    }

    MUTEX_LOCK(serchThread[sessionIndex].srchMutex);
    if(serchThread[sessionIndex].searchStatus == ACTIVE)
    {
        MUTEX_UNLOCK(serchThread[sessionIndex].srchMutex);
        EPRINT(DISK_MANAGER, "search record is already in progress: [sessionIndex=%d]", sessionIndex);
        return CMD_REQUEST_IN_PROGRESS;
    }

    serchThread[sessionIndex].searchStatus = ACTIVE;
    MUTEX_UNLOCK(serchThread[sessionIndex].srchMutex);

    if((searchType == NORMAL_SEARCH) || (searchType == ASYNC_ALL_SEARCH))
    {
        serchThread[sessionIndex].searchCriteria = (SEARCH_CRITERIA_t *)malloc(sizeof(SEARCH_CRITERIA_t));
    }
    else
    {
        serchThread[sessionIndex].searchCriteria = (SEARCH_CRITERIA_MNTH_DAY_t *)malloc(sizeof(SEARCH_CRITERIA_MNTH_DAY_t));
    }

    if (serchThread[sessionIndex].searchCriteria == NULL)
    {
        MUTEX_LOCK(serchThread[sessionIndex].srchMutex);
        serchThread[sessionIndex].searchStatus = INACTIVE;
        MUTEX_UNLOCK(serchThread[sessionIndex].srchMutex);
        EPRINT(DISK_MANAGER, "memory allocation failed");
        return CMD_RESOURCE_LIMIT;
    }

    if((searchType == NORMAL_SEARCH) || (searchType == ASYNC_ALL_SEARCH))
    {
        memcpy(serchThread[sessionIndex].searchCriteria, (SEARCH_CRITERIA_t *)userData, sizeof(SEARCH_CRITERIA_t));
    }
    else
    {
        memcpy(serchThread[sessionIndex].searchCriteria, (SEARCH_CRITERIA_MNTH_DAY_t *)userData, sizeof(SEARCH_CRITERIA_MNTH_DAY_t));
    }

    serchThread[sessionIndex].callBack = callBack;
    serchThread[sessionIndex].clientSocket = clientSocket;
    serchThread[sessionIndex].searchType = searchType;
    serchThread[sessionIndex].clientCbType = clientCbType;

    if (FAIL == Utils_CreateThread(NULL, searchPlayRecord, &serchThread[sessionIndex], DETACHED_THREAD, SRCH_RECORD_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(serchThread[sessionIndex].srchMutex);
        serchThread[sessionIndex].searchStatus = INACTIVE;
        MUTEX_UNLOCK(serchThread[sessionIndex].srchMutex);
        EPRINT(DISK_MANAGER, "fail to create search record");
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function search record for asynchronous search result from user
 * @param   searchThreadInfo
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL	asyncAllSearch(SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    UINT8				moreData = FALSE;
    UINT16				srchDataLen = 0, srchRsltLen = 0;
    SEARCH_RESULT_t		*searchData = NULL;
    SEARCH_DATA_t 		*searchRes = NULL;
    CHAR				resString[MAX_REPLY_SZ];
    CHARPTR				resStringPtr;
    UINT32 				sizeWritten, resStringSize;
    UINT16 				recordCounter;
    UINT8				partitionId = 0;
    NET_CMD_STATUS_e 	commandResponse = CMD_PROCESS_ERROR;
    SEARCH_CRITERIA_t	*searchCriteria = (SEARCH_CRITERIA_t *)searchThreadInfo->searchCriteria;

    do
    {
        searchData = (SEARCH_RESULT_t *)malloc((searchCriteria->noOfRecord * sizeof(SEARCH_RESULT_t)));
        if (NULL == searchData)
        {
            EPRINT(DISK_MANAGER, "memory allocation failed");
            break;
        }

        /* Check which type of recorded file needed by user and search as per user config data.
         * Any Camera number and any event type then all event for all camera file was given as search result */
        if (searchCriteria->channelNo == 0)
        {
            if (searchCriteria->eventType == DM_ANY_EVENT)
            {
                // all event of all camera
                if(searchAllEvent(searchCriteria, searchData, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of all events of all cameras");
                    break;
                }
            }
            else
            {
                // all camera's particular event
                if(searchAllCamEvent(searchCriteria, searchData, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of single event of all cameras");
                    break;
                }
            }
        }
        else
        {
            searchRes = (SEARCH_DATA_t *)malloc(sizeof(SEARCH_DATA_t) * (searchCriteria->noOfRecord + 1));
            if (NULL == searchRes)
            {
                EPRINT(DISK_MANAGER, "memory allocation failed");
                break;
            }

            if (searchCriteria->eventType == DM_ANY_EVENT)
            {
                // all event of all camera
                if(searchCamAllEvent(searchCriteria, searchRes, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of all events of single camera");
                    break;
                }
            }
            else
            {
                // all camera's particular event
                if(searchCamEvent(searchCriteria, searchRes, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of single event of single camera");
                    break;
                }
            }

            if (srchDataLen > 0)
            {
                srchRsltLen = srchDataLen;
                srchDataLen = 0;
                sortSearchRslt(searchRes, searchData, srchRsltLen, searchCriteria->noOfRecord, &srchDataLen);
            }
        }

        if (srchDataLen == 0)
        {
            commandResponse = CMD_NO_RECORD_FOUND;
            break;
        }

        if(moreData == TRUE)
        {
            commandResponse = CMD_MORE_DATA;
        }
        else
        {
            commandResponse = CMD_SUCCESS;
        }

        resStringPtr = resString;
        resStringSize = MAX_REPLY_SZ;
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, commandResponse, FSP);
        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;

        for(recordCounter = 0; recordCounter < srchDataLen; recordCounter++)
        {
            sizeWritten = snprintf(resStringPtr, resStringSize,
                                   "%c%d%c%02d%02d%04d%02d%02d%02d%c%02d%02d%04d%02d%02d%02d%c%d%c%d%c%d%c%d%c%d%c%d%c%c",
                                   SOI, (recordCounter + 1), FSP, searchData[recordCounter].startTime.tm_mday,
                                   (searchData[recordCounter].startTime.tm_mon + 1), searchData[recordCounter].startTime.tm_year,
                                   searchData[recordCounter].startTime.tm_hour, searchData[recordCounter].startTime.tm_min,
                                   searchData[recordCounter].startTime.tm_sec, FSP, searchData[recordCounter].endTime.tm_mday,
                                   (searchData[recordCounter].endTime.tm_mon + 1), searchData[recordCounter].endTime.tm_year,
                                   searchData[recordCounter].endTime.tm_hour, searchData[recordCounter].endTime.tm_min,
                                   searchData[recordCounter].endTime.tm_sec, FSP, searchData[recordCounter].channelNo, FSP,
                                   searchData[recordCounter].recordType, FSP, searchData[recordCounter].overlapFlg, FSP,
                                   searchData[recordCounter].diskId, FSP, partitionId, FSP, searchData[recordCounter].recStorageType, FSP,EOI);
            if (sizeWritten >= resStringSize)
            {
                commandResponse = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if (commandResponse == CMD_MAX_BUFFER_LIMIT)
        {
            break;
        }

        if ((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            commandResponse = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[searchThreadInfo->clientCbType](searchThreadInfo->clientSocket, (UINT8PTR)resString,strlen(resString), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[searchThreadInfo->clientCbType](&searchThreadInfo->clientSocket);

    }while(0);

    if ((commandResponse != CMD_SUCCESS) && (commandResponse != CMD_MORE_DATA))
    {
        searchThreadInfo->callBack(commandResponse, searchThreadInfo->clientSocket, TRUE);
    }

    MUTEX_LOCK(searchThreadInfo->srchMutex);
    searchThreadInfo->searchStatus = INACTIVE;
    MUTEX_UNLOCK(searchThreadInfo->srchMutex);
    FREE_MEMORY(searchRes);
    FREE_MEMORY(searchData);
    FREE_MEMORY(searchThreadInfo->searchCriteria);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function search record normally
 * @param   searchThreadInfo
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL	normalSearchRecord(SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    UINT8				moreData = FALSE;
    UINT16				srchDataLen = 0, srchRsltLen = 0;
    SEARCH_RESULT_t		*searchData = NULL;
    SEARCH_DATA_t 		*searchRes = NULL;
    CHAR				resString[MAX_REPLY_SZ];
    CHARPTR				resStringPtr;
    UINT32 				sizeWritten, resStringSize;
    UINT16 				recordCounter;
    UINT8				partitionId = 0;
    NET_CMD_STATUS_e 	commandResponse = CMD_PROCESS_ERROR;
    SEARCH_CRITERIA_t	*searchCriteria = (SEARCH_CRITERIA_t *)searchThreadInfo->searchCriteria;

    do
    {
        searchData = (SEARCH_RESULT_t *)malloc((searchCriteria->noOfRecord * sizeof(SEARCH_RESULT_t)));
        if(searchData == NULL)
        {
            EPRINT(DISK_MANAGER, "memory allocation failed");
            break;
        }

        /* Check which type of recorded file needed by user and search as per user config data.
         * Any Camera number and any event type then all event for all camera file was given as search result */
        if (searchCriteria->channelNo == 0)
        {
            if (searchCriteria->eventType == DM_ANY_EVENT)
            {
                // all event of all camera
                if(searchAllEvent(searchCriteria, searchData, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of all events of all cameras");
                    break;
                }
            }
            else
            {
                // all camera's particular event
                if(searchAllCamEvent(searchCriteria, searchData, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of single event of all cameras");
                    break;
                }
            }
        }
        else
        {
            searchRes = (SEARCH_DATA_t *)malloc(sizeof(SEARCH_DATA_t) * (searchCriteria->noOfRecord + 1));
            if(searchRes == NULL)
            {
                EPRINT(DISK_MANAGER, "memory allocation failed");
                break;
            }

            if (searchCriteria->eventType == DM_ANY_EVENT)
            {
                // all event of particular camera
                if(searchCamAllEvent(searchCriteria, searchRes,	&srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of all events of single camera");
                    break;
                }
            }
            else
            {
                // particular event of particular camera
                if(searchCamEvent(searchCriteria, searchRes, &srchDataLen, &moreData, searchThreadInfo) != SUCCESS)
                {
                    EPRINT(DISK_MANAGER, "error in search of single event of single camera");
                    break;
                }
            }

            if(srchDataLen > 0)
            {
                srchRsltLen = srchDataLen;
                srchDataLen = 0;
                sortSearchRslt(searchRes, searchData, srchRsltLen, searchCriteria->noOfRecord, &srchDataLen);
            }
        }

        if (srchDataLen == 0)
        {
            commandResponse = CMD_NO_RECORD_FOUND;
            break;
        }

        if(moreData == TRUE)
        {
            commandResponse = CMD_MORE_DATA;
        }
        else
        {
            commandResponse = CMD_SUCCESS;
        }

        resStringPtr = resString;
        resStringSize = MAX_REPLY_SZ;
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, commandResponse, FSP);
        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;

        for(recordCounter = 0; recordCounter < srchDataLen; recordCounter++)
        {
            sizeWritten = snprintf(resStringPtr, resStringSize,
                                   "%c%d%c%02d%02d%04d%02d%02d%02d%c%02d%02d%04d%02d%02d%02d%c%d%c%d%c%d%c%d%c%d%c%c",
                                   SOI, (recordCounter + 1), FSP, searchData[recordCounter].startTime.tm_mday,
                                   (searchData[recordCounter].startTime.tm_mon + 1), searchData[recordCounter].startTime.tm_year,
                                   searchData[recordCounter].startTime.tm_hour, searchData[recordCounter].startTime.tm_min,
                                   searchData[recordCounter].startTime.tm_sec, FSP, searchData[recordCounter].endTime.tm_mday,
                                   (searchData[recordCounter].endTime.tm_mon + 1), searchData[recordCounter].endTime.tm_year,
                                   searchData[recordCounter].endTime.tm_hour, searchData[recordCounter].endTime.tm_min,
                                   searchData[recordCounter].endTime.tm_sec, FSP, searchData[recordCounter].channelNo, FSP,
                                   searchData[recordCounter].recordType, FSP, searchData[recordCounter].overlapFlg, FSP,
                                   searchData[recordCounter].diskId, FSP, partitionId, FSP, EOI);
            if (sizeWritten >= resStringSize)
            {
                commandResponse = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if (commandResponse == CMD_MAX_BUFFER_LIMIT)
        {
            break;
        }

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            commandResponse = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[searchThreadInfo->clientCbType](searchThreadInfo->clientSocket, (UINT8PTR)resString, strlen(resString), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[searchThreadInfo->clientCbType](&searchThreadInfo->clientSocket);

    }while(0);

    if((commandResponse != CMD_SUCCESS) && (commandResponse != CMD_MORE_DATA))
    {
        searchThreadInfo->callBack(commandResponse, searchThreadInfo->clientSocket, TRUE);
    }

    MUTEX_LOCK(searchThreadInfo->srchMutex);
    searchThreadInfo->searchStatus = INACTIVE;
    MUTEX_UNLOCK(searchThreadInfo->srchMutex);
    FREE_MEMORY(searchRes);
    FREE_MEMORY(searchData);
    FREE_MEMORY(searchThreadInfo->searchCriteria);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function search record monthwise
 * @param   searchThreadInfo
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL	searchRecordMonth(SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    BOOL                        searchFromActaulMedia = TRUE;
    CHAR                        dirName[MAX_FILE_NAME_SIZE];
    CHAR                        fileName[MAX_FILE_NAME_SIZE];
    UINT8                       camCnt, dayCnt, monthId, byteCnt, evnt, recDriveId;
    UINT32                      diskCnt, totalDiskCnt;
    struct tm                   brokenTime, tempTime;
    YEAR_MAP_t                  yearRecMap;
    INT32                       fileFd = INVALID_FILE_FD;
    INT32                       readCnt;
    UINT32                      resultData = 0;
    CHAR                        resString[MAX_REPLY_SZ];
    UINT32                      sizeWritten;
    RECORD_ON_DISK_e            recDriveIndx;
    HDD_CONFIG_t                hddConfig;
    SEARCH_CRITERIA_MNTH_DAY_t  *searchCriteria = (SEARCH_CRITERIA_MNTH_DAY_t *)searchThreadInfo->searchCriteria;

    if(recordInfo[searchCriteria->timeData.tm_mon].yearId == searchCriteria->timeData.tm_year)
    {
        DPRINT(DISK_MANAGER, "searching from local data");
        searchFromActaulMedia = FALSE;
    }
    else
    {
        DPRINT(DISK_MANAGER, "searching from actual media");
    }

    for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
    {
        if(searchCriteria->searchRecStorageType == ALL_DRIVE)
        {
            if (FALSE == IsStorageOperationalForRead(recDriveId))
            {
                continue;
            }

            recDriveIndx = recDriveId;
        }
        else if(searchCriteria->searchRecStorageType == MAX_RECORDING_MODE)
        {
            ReadHddConfig(&hddConfig);
            recDriveIndx = hddConfig.recordDisk;
        }
        else
        {
            recDriveIndx = searchCriteria->searchRecStorageType;
        }

        if((recDriveIndx == REMOTE_NAS1) || (recDriveIndx == REMOTE_NAS2))
        {
            if (GetCurrentNddStatus(recDriveIndx) == FALSE)
            {
                continue;
            }
        }

        totalDiskCnt = GetTotalMediaNo(recDriveIndx);
        for(camCnt = 0; camCnt < getMaxCameraForCurrentVariant(); camCnt++)
        {
            if (FALSE == searchCriteria->cameraRecordF[camCnt])
            {
                continue;
            }

            brokenTime = searchCriteria->timeData;
            if (FALSE == searchFromActaulMedia)
            {
                monthId = brokenTime.tm_mon;
                tempTime = searchCriteria->timeData;

                for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
                {
                    if(searchCriteria->eventType & (1 << evnt))
                    {
                        if(recordInfo[monthId].yearId == tempTime.tm_year)
                        {
                            resultData |= recordInfo[monthId].recData[recDriveIndx][camCnt][evnt];
                        }
                    }
                }

                /* Check for next camera */
                continue;
            }

            for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
            {
                if(checkSearchChnlRecordFolder(diskCnt, GET_CAMERA_NO(camCnt), dirName, recDriveIndx) == FAIL)
                {
                    continue;
                }

                monthId = brokenTime.tm_mon;
                snprintf(fileName, MAX_FILE_NAME_SIZE,"%s"REC_INDEX_FILE_YEAR, dirName, brokenTime.tm_year, YEAR_MAP_FILE_EXTN);
                if(access(fileName, F_OK) != STATUS_OK)
                {
                    continue;
                }

                fileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(fileFd == INVALID_FILE_FD)
                {
                    DPRINT(DISK_MANAGER, "fail to open year map file: [fileName=%s], [err=%s]", fileName, STR_ERR);
                    continue;
                }

                readCnt = read(fileFd, &yearRecMap, sizeof(YEAR_MAP_t));
                if(readCnt != (INT32)(sizeof(YEAR_MAP_t)))
                {
                    DPRINT(DISK_MANAGER, "fail to read year map file: [fileName=%s], [err=%s]", fileName, STR_ERR);
                    close(fileFd);
                    continue;
                }

                /* Close year map file */
                close(fileFd);
                tempTime = searchCriteria->timeData;
                for(dayCnt = 0; dayCnt < MAX_DAYS; dayCnt++)
                {
                    tempTime.tm_mday = dayCnt + 1;
                    if (FAIL == checkSearchDateRecordFolder(diskCnt, GET_CAMERA_NO(camCnt), &tempTime, dirName, recDriveIndx))
                    {
                        continue;
                    }

                    for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
                    {
                        if (GET_BIT(searchCriteria->eventType, evnt) == 0)
                        {
                            continue;
                        }

                        for(byteCnt = 0; byteCnt < MAX_RESULT_FOR_HOUR_IN_BYTES; byteCnt++)
                        {
                            if (yearRecMap.monthMap[monthId].dayMap[dayCnt].eventMap[evnt].recordMap[byteCnt])
                            {
                                resultData |= (UINT32)(1 << dayCnt);
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (searchCriteria->searchRecStorageType != ALL_DRIVE)
        {
            break;
        }
    }

    sizeWritten = snprintf(resString, MAX_REPLY_SZ, "%c%s%c%02d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
    sizeWritten += snprintf(&resString[sizeWritten], MAX_REPLY_SZ - sizeWritten, "%c1%c", SOI, FSP);
    resString[sizeWritten++] = (UINT8)((resultData >> 24) & 0xFF);
    resString[sizeWritten++] = (UINT8)((resultData >> 16) & 0xFF);
    resString[sizeWritten++] = (UINT8)((resultData >> 8) & 0xFF);
    resString[sizeWritten++] = (UINT8)(resultData & 0xFF);
    sizeWritten += snprintf(&resString[sizeWritten], MAX_REPLY_SZ - sizeWritten, "%c%c%c", FSP, EOI, EOM);

    if (FAIL == sendCmdCb[searchThreadInfo->clientCbType](searchThreadInfo->clientSocket, (UINT8PTR)resString, sizeWritten, MESSAGE_REPLY_TIMEOUT))
    {
        EPRINT(DISK_MANAGER, "fail to send search data: timeout or connection closed");
    }

    closeConnCb[searchThreadInfo->clientCbType](&searchThreadInfo->clientSocket);
    MUTEX_LOCK(searchThreadInfo->srchMutex);
    searchThreadInfo->searchStatus = INACTIVE;
    MUTEX_UNLOCK(searchThreadInfo->srchMutex);
    FREE_MEMORY(searchThreadInfo->searchCriteria);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates the database lacally to optimize the synchronous month search
 *          by reading day indexes for last 12 months
 * @param   index
 * @param   camIndex
 * @return  SUCCESS/FAI
 */
BOOL GenerateLocalRecDatabase(RECORD_ON_DISK_e recDriveIndx, UINT8 camIndex)
{
    CHAR					dirName[MAX_FILE_NAME_SIZE];
    CHAR					fileName[MAX_FILE_NAME_SIZE];
    UINT8					dayCnt, recStorageLimit, loop, byteCnt, evnt;
    UINT8					startCamId, endCamId;
    UINT32					diskCnt, totalDiskCnt;
    struct tm				brokenTime = { 0 };
    struct tm				tempTime = { 0 };
    YEAR_MAP_t				currYearRecMap, prevYearRecMap;
    INT32					fileFd = INVALID_FILE_FD;
    INT32					readCnt;
    LocalTime_t             localTime;

    if (FALSE == IsStorageOperationalForRead(recDriveIndx))
    {
        return FALSE;
    }

    if(SUCCESS != GetLocalTime(&localTime))
    {
        EPRINT(DISK_MANAGER, "fail to get local time: [camera=%d]", camIndex);
        return FALSE;
    }

    time_t totalSec= localTime.totalSec;
    if(SUCCESS != ConvertLocalTimeInBrokenTm(&totalSec, &brokenTime))
    {
        EPRINT(DISK_MANAGER, "fail to get local time in broken time: [camera=%d]", camIndex);
        return FALSE;
    }

    /* verifying the data generation is for any specific camera or for all cameras */
    if (camIndex == INVALID_CAMERA_INDEX)
    {
        /* Data generation for all camera */
        startCamId = 0;
        endCamId = getMaxCameraForCurrentVariant();
    }
    else
    {
        /* Data generation for single camera */
        startCamId = camIndex;
        endCamId = startCamId+1;
    }

    /* Get total volumes for recording drive */
    totalDiskCnt = GetTotalMediaNo(recDriveIndx);

    MUTEX_LOCK(recDataMutex);
    for(camIndex = startCamId; camIndex < endCamId; camIndex++)
    {
        clearRecDataForDrive(recDriveIndx, camIndex);

        for (diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
        {
            checkSearchChnlRecordFolder(diskCnt, GET_CAMERA_NO(camIndex), dirName, recDriveIndx);

            //open current year file
            memset(&currYearRecMap, 0, sizeof(YEAR_MAP_t));
            snprintf(fileName, MAX_FILE_NAME_SIZE,"%s"REC_INDEX_FILE_YEAR, dirName, brokenTime.tm_year, YEAR_MAP_FILE_EXTN);
            if(access(fileName, F_OK) == STATUS_OK)
            {
                fileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(fileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [fileName=%s], [err=%s]", camIndex, fileName, STR_ERR);
                }
                else
                {
                    readCnt = read(fileFd, &currYearRecMap, sizeof(YEAR_MAP_t));
                    if(readCnt != (INT32)(sizeof(YEAR_MAP_t)))
                    {
                        EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [fileName=%s], [err=%s]", camIndex, fileName, STR_ERR);
                        memset(&currYearRecMap, 0, sizeof(YEAR_MAP_t));
                    }
                    close(fileFd);
                }
            }

            //open prev year file in case of completing data of past 12 months
            memset(&prevYearRecMap, 0, sizeof(YEAR_MAP_t));
            snprintf(fileName, MAX_FILE_NAME_SIZE,"%s"REC_INDEX_FILE_YEAR, dirName, brokenTime.tm_year - 1, YEAR_MAP_FILE_EXTN);
            if(access(fileName, F_OK) == STATUS_OK)
            {
                fileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(fileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [fileName=%s], [err=%s]", camIndex, fileName, STR_ERR);
                }
                else
                {
                    readCnt = read(fileFd, &prevYearRecMap, sizeof(YEAR_MAP_t));
                    if(readCnt != (INT32)(sizeof(YEAR_MAP_t)))
                    {
                        EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [fileName=%s], [err=%s]", camIndex, fileName, STR_ERR);
                        memset(&prevYearRecMap, 0, sizeof(YEAR_MAP_t));
                    }
                    close(fileFd);
                }
            }

            //start from current month and decrement upto last 12 months
            tempTime.tm_mon = brokenTime.tm_mon;
            tempTime.tm_year = brokenTime.tm_year;

            INT8 tempDay = brokenTime.tm_mday;
            INT8 tempWday = brokenTime.tm_wday;

            for(loop = SUNDAY; loop < MAX_WEEK_DAYS; loop++)
            {
                /* fill the data for dates availlable for current month */
                if(tempDay > 0)
                {
                    dayRecordInfo[tempWday].dayId = tempDay;
                    dayRecordInfo[tempWday].monthId = brokenTime.tm_mon;
                    dayRecordInfo[tempWday].yearId = brokenTime.tm_year;

                    for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
                    {
                        for(byteCnt = 0; byteCnt < MAX_RESULT_FOR_HOUR_IN_BYTES; byteCnt++)
                        {
                            dayRecordInfo[tempWday].recordsForday[recDriveIndx][camIndex].eventMap[evnt].recordMap[byteCnt]
                                    |= currYearRecMap.monthMap[tempTime.tm_mon].dayMap[tempDay - 1].eventMap[evnt].recordMap[byteCnt];
                        }

                        dayRecordInfo[tempWday].recordsForday[recDriveIndx][camIndex].eventMap[evnt].overlapFlag
                                |= currYearRecMap.monthMap[tempTime.tm_mon].dayMap[tempDay - 1].eventMap[evnt].overlapFlag;
                    }

                    tempDay--;
                }
                else
                {
                    dayRecordInfo[tempWday].yearId = 0;
                }

                tempWday--;
                if (tempWday < 0)
                {
                    /* weekdays are in between 0 to 6 */
                    tempWday = 6;
                }
            }

            for(recStorageLimit = 0; recStorageLimit < MAX_MONTH; recStorageLimit++)
            {
                if((JANUARY <= tempTime.tm_mon) && (tempTime.tm_mon <= DECEMBER ))
                {
                    for(dayCnt = 0; dayCnt < MAX_DAYS; dayCnt++)
                    {
                        tempTime.tm_mday = dayCnt + 1;				/* date starting from index 1 (1 - 31) */
                        if(checkSearchDateRecordFolder(diskCnt, GET_CAMERA_NO(camIndex), &tempTime, dirName, recDriveIndx) == SUCCESS)
                        {
                            for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
                            {
                                for(byteCnt = 0; byteCnt < MAX_RESULT_FOR_HOUR_IN_BYTES; byteCnt++)
                                {
                                    if(currYearRecMap.monthMap[tempTime.tm_mon].dayMap[dayCnt].eventMap[evnt].recordMap[byteCnt])
                                    {
                                        recordInfo[tempTime.tm_mon].recData[recDriveIndx][camIndex][evnt] |= (UINT32)(1 << dayCnt);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                /* put month id and year id to structure */
                recordInfo[tempTime.tm_mon].yearId = tempTime.tm_year;
                tempTime.tm_mon--;

                /* current year has not 12 months data so we will get remaining data from prev year */
                if(tempTime.tm_mon < 0)
                {
                    /* start from prev year data, which already been read */
                    currYearRecMap = prevYearRecMap;
                    tempTime.tm_mon = DECEMBER;
                    tempTime.tm_year--;
                }
            }
        }
    }
    MUTEX_UNLOCK(recDataMutex);

    DPRINT(DISK_MANAGER, "data locally generated: [startCamId=%d], [endCamIdx=%d]", startCamId, endCamId);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function clears the local data stored for given drive for 12 months.
 * @param   disk
 * @param   camCnt
 */
static void clearRecDataForDrive(RECORD_ON_DISK_e disk ,UINT8 camCnt)
{
    UINT8 recStorageLimit, evnt, loop, byteCnt;

    for(recStorageLimit = 0; recStorageLimit < MAX_MONTH; recStorageLimit++)
    {
        for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
        {
            recordInfo[recStorageLimit].recData[disk][camCnt][evnt] = (UINT32)0;
        }
    }

    for(loop = 0; loop < MAX_WEEK_DAYS; loop++)
    {
        for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
        {
            for(byteCnt = 0; byteCnt < MAX_RESULT_FOR_HOUR_IN_BYTES; byteCnt++)
            {
                dayRecordInfo[loop].recordsForday[disk][camCnt].eventMap[evnt].recordMap[byteCnt] = (UINT8)0;
            }
            dayRecordInfo[loop].recordsForday[disk][camCnt].eventMap[evnt].overlapFlag = FALSE;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search records by day
 * @param   searchThreadInfo
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL	searchRecordDay(SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    BOOL                        diskStatus[MAX_RECORDING_MODE], searchFromActualMedia = TRUE;
    BOOL                        overlapIndicator = FALSE, isReadSuccess = FALSE;
    UINT8                       camCnt, eventTypeCnt, diskCnt, monthIdx, dayIdx, byteCnt, recDriveId;
    UINT8                       tempData[MAX_RESULT_FOR_HOUR_IN_BYTES];
    UINT8                       recData[REC_EVENT_MAX][MAX_RESULT_FOR_HOUR_IN_BYTES];
    BOOL                        overlapFlg[REC_EVENT_MAX] = {0,0,0,0};
    INT32                       fileFd = INVALID_FILE_FD;
    INT32                       readCnt;
    UINT8                       resultCnt = 0;
    time_t                      tempTime;
    CHAR                        mountPoint[MOUNT_POINT_SIZE];
    CHAR                        resString[MAX_REPLY_SZ * 2];
    CHAR                        dirName[MAX_FILE_NAME_SIZE];
    YEAR_MAP_t                  yearRecMap;
    CHAR                        fileName[MAX_FILE_NAME_SIZE];
    CHARPTR                     rsltData, resStringPtr = resString;
    UINT32                      sizeWritten, totalDiskCnt, resStringSize = sizeof(resString) - 2; /* Reserved 2 chars for EOM */
    RECORD_ON_DISK_e 			recDriveIndx;
    NET_CMD_STATUS_e			cmdResp = CMD_SUCCESS;
    SEARCH_CRITERIA_MNTH_DAY_t  *searchCriteria = (SEARCH_CRITERIA_MNTH_DAY_t *)searchThreadInfo->searchCriteria;
    HDD_CONFIG_t				hddConfig;

    sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%02d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
    resStringPtr += sizeWritten;
    resStringSize -= sizeWritten;

    if(searchCriteria->searchRecStorageType == ALL_DRIVE)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            diskStatus[recDriveId] = IsStorageOperationalForRead(recDriveId);
        }
    }

    ConvertLocalTimeInSec(&searchCriteria->timeData, &tempTime);
    ConvertLocalTimeInBrokenTm(&tempTime, &searchCriteria->timeData);

    if((dayRecordInfo[searchCriteria->timeData.tm_wday].yearId == searchCriteria->timeData.tm_year)
            && (dayRecordInfo[searchCriteria->timeData.tm_wday].monthId == searchCriteria->timeData.tm_mon)
            && (dayRecordInfo[searchCriteria->timeData.tm_wday].dayId == searchCriteria->timeData.tm_mday))
    {
        DPRINT(DISK_MANAGER, "searching from local data");
        searchFromActualMedia = FALSE;
    }
    else
    {
        DPRINT(DISK_MANAGER, "searching from actual media");
    }

    memset(tempData, 0, sizeof(tempData));
    for(camCnt = 0; camCnt < getMaxCameraForCurrentVariant(); camCnt++)
    {
        if (FALSE == searchCriteria->cameraRecordF[camCnt])
        {
            continue;
        }

        memset(recData, 0, sizeof(recData));
        memset(overlapFlg, FALSE, sizeof(overlapFlg));
        monthIdx = searchCriteria->timeData.tm_mon;
        dayIdx = searchCriteria->timeData.tm_mday - 1;
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            if(searchCriteria->searchRecStorageType == ALL_DRIVE)
            {
                recDriveIndx = recDriveId;
                if(diskStatus[recDriveIndx] != TRUE)
                {
                    continue;
                }
            }
            else if(searchCriteria->searchRecStorageType == MAX_RECORDING_MODE)
            {
                ReadHddConfig(&hddConfig);
                recDriveIndx = hddConfig.recordDisk;
            }
            else
            {
                recDriveIndx = searchCriteria->searchRecStorageType;
            }

            if((recDriveIndx == REMOTE_NAS1) || (recDriveIndx == REMOTE_NAS2))
            {
                if(GetCurrentNddStatus(recDriveIndx) == FALSE )
                {
                    continue;
                }
            }

            totalDiskCnt = GetTotalMediaNo(recDriveIndx);
            if(FALSE == searchFromActualMedia)
            {
                for(eventTypeCnt = REC_EVENT_MANUAL; eventTypeCnt < REC_EVENT_MAX; eventTypeCnt++)
                {
                    if (GET_BIT(searchCriteria->eventType, eventTypeCnt) == 0)
                    {
                        continue;
                    }

                    for(byteCnt = 0; byteCnt < MAX_RESULT_FOR_HOUR_IN_BYTES; byteCnt++)
                    {
                        recData[eventTypeCnt][byteCnt] |= dayRecordInfo[searchCriteria->timeData.tm_wday]
                                .recordsForday[recDriveIndx][camCnt].eventMap[eventTypeCnt].recordMap[byteCnt];
                    }
                    overlapFlg[eventTypeCnt] |= dayRecordInfo[searchCriteria->timeData.tm_wday]
                            .recordsForday[recDriveIndx][camCnt].eventMap[eventTypeCnt].overlapFlag;
                }
                isReadSuccess = TRUE;
            }
            else
            {
                if(isRecordsAvaillableForDay(&searchCriteria->timeData, camCnt, recDriveIndx, searchCriteria->eventType) == FALSE )
                {
                    continue;
                }

                for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
                {
                    if(checkSearchDateRecordFolder(diskCnt, GET_CAMERA_NO(camCnt), &searchCriteria->timeData, dirName, recDriveIndx) == FAIL)
                    {
                        continue;
                    }

                    GetMountPointFromDiskId(diskCnt, mountPoint, recDriveIndx);

                    snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR,
                             mountPoint, GET_CAMERA_NO(camCnt), searchCriteria->timeData.tm_year, YEAR_MAP_FILE_EXTN);
                    if(access(fileName, F_OK) != STATUS_OK)
                    {
                        continue;
                    }

                    fileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                    if(fileFd == INVALID_FILE_FD)
                    {
                        EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [file=%s], [err=%s]", camCnt, fileName, STR_ERR);
                        continue;
                    }

                    readCnt = read(fileFd, &yearRecMap, sizeof(YEAR_MAP_t));
                    if(readCnt != (INT32)(sizeof(YEAR_MAP_t)))
                    {
                        EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [file=%s], [err=%s]", camCnt, fileName, STR_ERR);
                        close(fileFd);
                        continue;
                    }

                    for(eventTypeCnt = REC_EVENT_MANUAL; eventTypeCnt < REC_EVENT_MAX; eventTypeCnt++)
                    {
                        if (GET_BIT(searchCriteria->eventType, eventTypeCnt) == 0)
                        {
                            continue;
                        }

                        for(byteCnt = 0; byteCnt < MAX_RESULT_FOR_HOUR_IN_BYTES; byteCnt++)
                        {
                            recData[eventTypeCnt][byteCnt] |= yearRecMap.monthMap[monthIdx].dayMap[dayIdx].eventMap[eventTypeCnt].recordMap[byteCnt];
                        }
                        overlapFlg[eventTypeCnt] |= yearRecMap.monthMap[monthIdx].dayMap[dayIdx].eventMap[eventTypeCnt].overlapFlag;
                    }

                    isReadSuccess = TRUE;
                    close(fileFd);
                }
            }

            if(searchCriteria->searchRecStorageType != ALL_DRIVE)
            {
                break;
            }
        }

        for(eventTypeCnt = REC_EVENT_MANUAL; eventTypeCnt < REC_EVENT_MAX; eventTypeCnt++)
        {
            if (GET_BIT(searchCriteria->eventType, eventTypeCnt) == 0)
            {
                continue;
            }

            overlapIndicator = FALSE;
            if(isReadSuccess == TRUE)
            {
                rsltData = (CHARPTR)recData[eventTypeCnt];
                if( overlapFlg[eventTypeCnt] != FALSE )
                {
                    overlapIndicator = TRUE;
                }
            }
            else
            {
                rsltData = (CHARPTR)tempData;
            }

            resultCnt++;
            if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%02d%c%d%c",
                                       SOI, resultCnt, FSP, GET_CAMERA_NO(camCnt), FSP, (1 << eventTypeCnt), FSP)) >= resStringSize)
            {
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
            if(resStringSize > MAX_RESULT_FOR_HOUR_IN_BYTES)
            {
                memcpy(resStringPtr, rsltData, MAX_RESULT_FOR_HOUR_IN_BYTES);
                resStringPtr += MAX_RESULT_FOR_HOUR_IN_BYTES;
                resStringSize -= MAX_RESULT_FOR_HOUR_IN_BYTES;
            }

            if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%c",FSP,overlapIndicator,FSP, EOI)) >= resStringSize)
            {
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }
    }

    if (cmdResp == CMD_SUCCESS)
    {
        resStringSize += 2;
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM);
        resStringSize -= sizeWritten;
        if (FAIL == sendCmdCb[searchThreadInfo->clientCbType](searchThreadInfo->clientSocket, (UINT8PTR)resString,
                                                              (sizeof(resString) - resStringSize), MESSAGE_REPLY_TIMEOUT))
        {
            EPRINT(DISK_MANAGER, "fail to send search data: timeout or connection closed");
        }
        closeConnCb[searchThreadInfo->clientCbType](&searchThreadInfo->clientSocket);
    }
    else
    {
        searchThreadInfo->callBack(cmdResp, searchThreadInfo->clientSocket, TRUE);
    }

    MUTEX_LOCK(searchThreadInfo->srchMutex);
    searchThreadInfo->searchStatus = INACTIVE;
    MUTEX_UNLOCK(searchThreadInfo->srchMutex);
    FREE_MEMORY(searchThreadInfo->searchCriteria);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   isRecordsAvaillableForDay
 * @param   timeData
 * @param   camIndex
 * @param   disk
 * @param   eventCriteria
 * @return
 */
static BOOL	isRecordsAvaillableForDay(struct tm* timeData, UINT8 camIndex, RECORD_ON_DISK_e disk, EVENT_e eventCriteria)
{
    RECORDING_EVENTS_e evnt;

    if (recordInfo[timeData->tm_mon].yearId != timeData->tm_year)
    {
        return TRUE;
    }

    for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
    {
        if (GET_BIT(eventCriteria, evnt) == 0)
        {
            continue;
        }

        if (GET_BIT(recordInfo[timeData->tm_mon].recData[disk][camIndex][evnt], (timeData->tm_mday - 1)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function in which it will search record and send proper message to web client.
 * @param   threadData
 * @return
 */
static VOIDPTR searchPlayRecord(VOIDPTR threadData)
{
    SEARCH_THREAD_INFO_t *searchThreadInfo = (SEARCH_THREAD_INFO_t *)threadData;

    THREAD_START("ASYNC_SRCH");
    (*searchFuncPtr[searchThreadInfo->searchType])(searchThreadInfo);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function gives how many disk session can possible.
 * @return
 */
UINT8 GetAvailablPlaySession(void)
{
    UINT8 sessionIdx, availableSession = 0;

    MUTEX_LOCK(playbckSessionMutex);
    // find out any session is free
    for (sessionIdx = 0; sessionIdx < MAX_PLAYBACK_SESSION; sessionIdx++)
    {
        // check that session was running or not
        if(playSession[sessionIdx].playStatus == OFF)
        {
            availableSession++;
        }
    }
    MUTEX_UNLOCK(playbckSessionMutex);
    return availableSession;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates playback session for particular record and it will give playback
 *          session id. This function also checks that maximum session of playback was started, then
 *          it will not allow to create new session of playback.
 * @param   playInfo
 * @param   playType
 * @param   sessionId
 * @return
 */
NET_CMD_STATUS_e OpenPlaySession(PLAY_CNTRL_INFO_t *playInfo, PLAY_TYPE_e playType, PLAY_SESSION_ID *sessionId)
{
    BOOL                    startDetect = NO;
    BOOL                    currentEvent = FALSE;
    UINT8                   sessionIdx = INVALID_PLAY_SESSION_HANDLE, camIndex;
    CHAR                    channelFldr[MAX_FILE_NAME_SIZE];
    CHAR                    eventFile[MAX_FILE_NAME_SIZE];
    INT32                   evntFileFd = INVALID_FILE_FD;
    UINT32                  eventIdxNo = 0;
    UINT32                  readCnt = 0;
    time_t                  evntStartTime;
    time_t                  evntStopTime;
    METADATA_FILE_HDR_t     eventFileInfo;
    EVNT_INFO_t             eventData;
    PLAYBACK_SESSION_t      *playSess;
    NET_CMD_STATUS_e        cmdStatus = CMD_PROCESS_ERROR;

    // validate input parameters
    if((playInfo == NULL) || (sessionId == NULL))
    {
        EPRINT(DISK_MANAGER, "null pointer found");
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == IsStorageOperationalForRead(playInfo->recStorageType))
    {
        return GiveMediaStatus(playInfo->recStorageType);
    }

    do
    {
        if(playType == PLAYBACK_PLAY)
        {
            // find out any session is free
            MUTEX_LOCK(playbckSessionMutex);
            for (sessionIdx = 0; sessionIdx < MAX_PLAYBACK_SESSION; sessionIdx++)
            {
                // check that session was running or not
                if(playSession[sessionIdx].playStatus == OFF)
                {
                    playSession[sessionIdx].playStatus = ON;
                    break;
                }
            }
            MUTEX_UNLOCK(playbckSessionMutex);

            if (sessionIdx >= MAX_PLAYBACK_SESSION)
            {
                /* No free space available for this session  maximum session was running */
                cmdStatus = CMD_MAX_STREAM_LIMIT;
                sessionIdx = INVALID_PLAY_SESSION_HANDLE;
                EPRINT(DISK_MANAGER, "no free play session available: [camera=%d]", playInfo->camNo-1);
                break;
            }
        }
        else
        {
            // for backup type play only one session allow.
            sessionIdx = MAX_PLAYBACK_SESSION;
            MUTEX_LOCK(playbckSessionMutex);
            if(playSession[sessionIdx].playStatus == ON)
            {
                /* Backup session is not free */
                MUTEX_UNLOCK(playbckSessionMutex);
                break;
            }

            playSession[sessionIdx].playStatus = ON;
            MUTEX_UNLOCK(playbckSessionMutex);
        }

        playSess = &playSession[sessionIdx];
        camIndex = (playInfo->camNo - 1);

        // validate that requested record is present. check recorded channel is valid
        if(checkSearchRecordFolder(playInfo->diskId, playInfo->camNo, &playInfo->startTime, channelFldr, playInfo->recStorageType) == FAIL)
        {
            EPRINT(DISK_MANAGER, "camera folder not present: [camera=%d], [path=%s]", camIndex, channelFldr);
            break;
        }

        ConvertLocalTimeInSec(&playInfo->startTime, &evntStartTime);
        ConvertLocalTimeInSec(&playInfo->stopTime, &evntStopTime);
        snprintf(eventFile, MAX_FILE_NAME_SIZE,"%s%s", channelFldr, EVENT_FILE_NAME);

        // open event file
        evntFileFd = open(eventFile, READ_ONLY_MODE, FILE_PERMISSION);
        if(evntFileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", camIndex, eventFile, STR_ERR);
            break;
        }

        // Read event file header. Get next event index number
        readCnt = read(evntFileFd, &eventFileInfo, EVENT_FILE_HDR_SIZE);
        if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read event file header: [camera=%d], [path=%s], [err=%s]", camIndex, eventFile, STR_ERR);
            close(evntFileFd);
            break;
        }

        while(eventIdxNo < eventFileInfo.nextMetaDataIdx)
        {
            // Read event data from current file position
            readCnt = read(evntFileFd, &eventData, EVNT_FIELD_SIZE);
            if (readCnt != EVNT_FIELD_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read event file field: [camera=%d], [path=%s], [err=%s]", camIndex, eventFile, STR_ERR);
                break;
            }

            if((time_t)eventData.endTime == 0)
            {
                eventData.endTime = sessionInfo[camIndex].streamFileInfo.localTime.totalSec;
                currentEvent = TRUE;
            }

            if((playInfo->eventType == DM_MANUAL_EVENT) || (playInfo->eventType == DM_ALARM_EVENT)
                    || (playInfo->eventType == DM_SCHEDULE_EVENT) || (playInfo->eventType == DM_COSEC_EVENT))
            {
                // same event detected as in search criteria
                if (((time_t)eventData.startTime == evntStartTime) && (((time_t)eventData.endTime == evntStopTime) || (currentEvent == TRUE))
                        && (eventData.eventType == playInfo->eventType) && (playInfo->overlapFlg == eventData.overLapFlg))
                {
                    snprintf(playSess->playDir, MAX_FILE_NAME_SIZE, "%s", channelFldr);
                    playSess->startFileId = eventData.startFileId;
                    playSess->startStreamPos = eventData.startStreamPos;
                    if(currentEvent == TRUE)
                    {
                        playSess->stopFileId = sessionInfo[camIndex].streamFileInfo.streamFileIdx;
                        playSess->stopStreamPos = sessionInfo[camIndex].streamFileInfo.curFshPos;
                    }
                    else
                    {
                        playSess->stopFileId = eventData.stopFileId;
                        playSess->stopStreamPos = eventData.stopStreamPos;
                    }

                    memcpy(&playSess->playCntrl, playInfo, sizeof(PLAY_CNTRL_INFO_t));
                    memset(&playSess->playInfo, 0, sizeof(PLAY_INFO_t));

                    playSess->playInfo.streamFileFd = INVALID_FILE_FD;
                    playSess->playInfo.iFrmFileFd = INVALID_FILE_FD;

                    playSess->playStartTime = evntStartTime;
                    playSess->playStopTime = evntStopTime;
                    cmdStatus = CMD_SUCCESS;
                    break;
                }
            }
            else
            {
                if(((time_t)eventData.startTime == evntStartTime) && (startDetect == NO))
                {
                    startDetect = YES;
                    playSess->playStartTime = evntStartTime;
                    snprintf(playSess->playDir, MAX_FILE_NAME_SIZE, "%s", channelFldr);
                    playSess->startFileId = eventData.startFileId;
                    playSess->startStreamPos = eventData.startStreamPos;
                }

                if(((time_t)eventData.endTime == evntStopTime) || (evntStopTime <= (time_t)eventData.endTime))
                {
                    if(currentEvent == TRUE)
                    {
                        playSess->stopFileId = sessionInfo[camIndex].streamFileInfo.streamFileIdx;
                        playSess->stopStreamPos = sessionInfo[camIndex].streamFileInfo.curFshPos;
                    }
                    else
                    {
                        playSess->stopFileId = eventData.stopFileId;
                        playSess->stopStreamPos = eventData.stopStreamPos;
                    }

                    memcpy(&playSess->playCntrl, playInfo, sizeof(PLAY_CNTRL_INFO_t));
                    memset(&playSess->playInfo, 0, sizeof(PLAY_INFO_t));

                    playSess->playInfo.streamFileFd = INVALID_FILE_FD;
                    playSess->playInfo.iFrmFileFd = INVALID_FILE_FD;

                    playSess->playStopTime = evntStopTime;
                    cmdStatus = CMD_SUCCESS;
                    break;
                }
            }
            eventIdxNo++;
        }

        /* Now close event file */
        close(evntFileFd);

        /* Check playback session allocation status */
        if (cmdStatus != CMD_SUCCESS)
        {
            break;
        }

        playSess->playBuff.buffPtr = malloc(DEFUALT_PLAYBACK_SIZE);
        if (NULL == playSess->playBuff.buffPtr)
        {
            EPRINT(DISK_MANAGER, "memory allocation failed: [camera=%d]", camIndex);
            CloseFileFd(&playSess->playInfo.streamFileFd);
            CloseFileFd(&playSess->playInfo.iFrmFileFd);
            playSess->playBuff.buffSize = 0;
            cmdStatus = CMD_PROCESS_ERROR;
            break;
        }

        *sessionId = sessionIdx;
        playSess->playBuff.buffSize = DEFUALT_PLAYBACK_SIZE;
        DPRINT(DISK_MANAGER, "playback session opened: [camera=%d], [session=%d], [startFileId=%d], [stopFileId=%d]",
               camIndex, sessionIdx, playSess->startFileId, playSess->stopFileId);
    } while(0);

    if ((cmdStatus != CMD_SUCCESS) && (sessionIdx != INVALID_PLAY_SESSION_HANDLE))
    {
        MUTEX_LOCK(playbckSessionMutex);
        playSession[sessionIdx].playStatus = OFF;
        MUTEX_UNLOCK(playbckSessionMutex);
    }

    return cmdStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function closed the already created playback session and frees all resources used
 *          by this playback session
 * @param   sessionId
 * @return  SUCCESS/FAIL
 */
BOOL ClosePlaySession(PLAY_SESSION_ID sessionId)
{
    // check session id was valid or not
    if (sessionId >= MAX_PLAYBACK_DISK_SESSION)
    {
        return FAIL;
    }

    // find session id
    MUTEX_LOCK(playbckSessionMutex);
    if(playSession[sessionId].playStatus == OFF)
    {
        MUTEX_UNLOCK(playbckSessionMutex);
        EPRINT(DISK_MANAGER, "playback session is already off: [session=%d]", sessionId);
        return FAIL;
    }

    CloseFileFd(&playSession[sessionId].playInfo.streamFileFd);
    CloseFileFd(&playSession[sessionId].playInfo.iFrmFileFd);
    FREE_MEMORY(playSession[sessionId].playBuff.buffPtr);
    playSession[sessionId].playBuff.buffSize = 0;
    playSession[sessionId].playCntrl.recStorageType = MAX_RECORDING_MODE;
    playSession[sessionId].playStatus = OFF;
    MUTEX_UNLOCK(playbckSessionMutex);

    DPRINT(DISK_MANAGER, "playback session closed: [session=%d]", sessionId);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets file position of a selected record.
 * @param   playPos - play position in selected record
 * @param   mSec - milisecond of second
 * @param   audioEnbl - audio enable or not
 * @param   playCmd
 * @param   speed
 * @param   sessionId - current session id for this playback
 * @return  SUCCESS/FAIL
 */
BOOL SetPlayPosition(struct tm *playPos, UINT16 mSec, UINT8 audioEnbl, PLAYBACK_CMD_e playCmd,
                     PLAYBACK_SPEED_e speed, PLAY_SESSION_ID sessionId)
{
    CHAR                    searchFileName[MAX_FILE_NAME_SIZE] = "";
    INT32                   searchFileFd = INVALID_FILE_FD;
    time_t                  playPosTimeSec = 0;
    PLAYBACK_SESSION_t 		*pPlaySession = NULL;
    PLAY_INFO_t 			*pPlayInfo = NULL;
    FSH_INFO_t 				fshInfo = { 0 };
    UINT32 					lastReadStreamPos = 0;
    UINT64					playPosTimeInMiliSec = 0;
    UINT64					fshTimeInMiliSec = 0;
    UINT8                   *searchFileData = NULL;
    IFRM_FIELD_INFO_t       *iFrameInfo = NULL;
    struct tm               iFrameTime;
    struct stat             searchFileInfo;
    UINT64                  dataCnt;

    /* validate input parameter */
    if (sessionId >= MAX_PLAYBACK_DISK_SESSION)
    {
        EPRINT(DISK_MANAGER, "invld playback session: [session=%d]", sessionId);
        return FAIL;
    }

    if (FALSE == IsStorageOperationalForRead(playSession[sessionId].playCntrl.recStorageType))
    {
        EPRINT(DISK_MANAGER, "disk status not proper to proceed: [session=%d]", sessionId);
        return FAIL;
    }

    /* get session parameters from sessionId */
    pPlaySession = &playSession[sessionId];
    pPlayInfo = &pPlaySession->playInfo;

    MUTEX_LOCK(playbckSessionMutex);
    /* check this session is running */
    if(pPlaySession->playStatus == OFF)
    {
        MUTEX_UNLOCK(playbckSessionMutex);
        EPRINT(DISK_MANAGER, "playback is off for this session: [session=%d]", sessionId);
        return FAIL;
    }
    MUTEX_UNLOCK(playbckSessionMutex);

    pPlaySession->audioEnbl = audioEnbl;
    pPlaySession->speed = speed;

    /* Convert playPos local time into time in second into playPosTimeSec */
    ConvertLocalTimeInSec(playPos, &playPosTimeSec);

    /* Added 1sec offset in play position */
    if ((pPlaySession->playStartTime >= pPlaySession->playStopTime) || ((playPosTimeSec + 1) < pPlaySession->playStartTime) || (playPosTimeSec > pPlaySession->playStopTime))
    {
        EPRINT(DISK_MANAGER, "invld playback position: [session=%d], [position=%ld], [StartTime=%ld], [StopTime%ld]",
               sessionId, playPosTimeSec, pPlaySession->playStartTime, pPlaySession->playStopTime);
        return FAIL;
    }

    /* Earlier for normal search, we were deriving stream file from time index file based on minute match and then traverse (lseek)
     * in stream file for required time in second. It is ok for single camera but for more cameras (16ch sync playback),
     * it takes very much time (more than 1 sec for single camera record search when recording is in network drive) in traversing.
     * Now make it faster, we will derive stream file from I-frame index file based on second match and then traverse (lseek)
     * in stream file for same second. It takes ~100ms for single camera on network drive. */
    /* Preapared iframe index file path using current play directory for camera */
    snprintf(searchFileName, MAX_FILE_NAME_SIZE, "%s%s", pPlaySession->playDir, I_FRAME_FILE_NAME);

    /* Open iframe index file */
    searchFileFd = open(searchFileName, READ_ONLY_MODE, FILE_PERMISSION);
    if (searchFileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open iframe file: [file=%s], [err=%s]", searchFileName, STR_ERR);
        return FAIL;
    }

    /* Get the size of iframe index file */
    searchFileInfo.st_size = 0;
    if ((fstat(searchFileFd, &searchFileInfo) == -1) || (searchFileInfo.st_size <= (IFRAME_FILE_HDR_SIZE + IFRAME_FIELD_SIZE)))
    {
        EPRINT(DISK_MANAGER, "fail to get size of iframe file: [file=%s], [size=%lld], [err=%s]",
               searchFileName, (UINT64)searchFileInfo.st_size, STR_ERR);
        close(searchFileFd);
        return FAIL;
    }

    /* Allocate memory for iframe index file */
    searchFileData = (UINT8 *)malloc(searchFileInfo.st_size);
    if (NULL == searchFileData)
    {
        EPRINT(DISK_MANAGER, "fail to allocate memory for iframe data: [file=%s], [size=%lld], [err=%s]",
               searchFileName, (UINT64)searchFileInfo.st_size, STR_ERR);
        close(searchFileFd);
        return FAIL;
    }

    /* Take iframe index file copy in RAM for faster searching */
    if (read(searchFileFd, searchFileData, searchFileInfo.st_size) != searchFileInfo.st_size)
    {
        EPRINT(DISK_MANAGER, "fail to read iframe file: [file=%s], [err=%s]", searchFileName, STR_ERR);
        close(searchFileFd);
        free(searchFileData);
        return FAIL;
    }

    /* Now no need of iframe index file */
    close(searchFileFd);

    dataCnt = IFRAME_FILE_HDR_SIZE;
    do
    {
        /* Searching in iframe index file data for required second */
        if ((dataCnt + IFRAME_FIELD_SIZE) > (UINT64)searchFileInfo.st_size)
        {
            /* Take some offset to handle boundary cases of hour */
            if ((((time_t)iFrameInfo->timeSec) + 10) >= playPosTimeSec)
            {
                WPRINT(DISK_MANAGER, "offset added to get required record from iframe file");
                break;
            }

            EPRINT(DISK_MANAGER, "required record not found in iframe file: [file=%s]", searchFileName);
            free(searchFileData);
            return FAIL;
        }

        iFrameInfo = (IFRM_FIELD_INFO_t *)(searchFileData + dataCnt);
        dataCnt += IFRAME_FIELD_SIZE;

    }while ((time_t)iFrameInfo->timeSec < playPosTimeSec);

    time_t iFrameTimeSec = iFrameInfo->timeSec;
    ConvertLocalTimeInBrokenTm(&iFrameTimeSec, &iFrameTime);

    /* Store required stream file index */
    pPlayInfo->streamFileIdx = iFrameInfo->strmFileId;

    /* If record required from beginning of an hour, then start record from the first frame rather than i-frame.
     * because when hour changes during sync play or backup, initial records will skip till i-frame and jump will
     * observe. To avoid that, if records required from beginning of hour and it is found in 1st stream file using
     * i-frame file then start record play from initially otherwise start from record from i-frame */
    if ((playPos->tm_min == 0) && (playPos->tm_sec == 0) && (pPlayInfo->streamFileIdx == 1))
    {
        pPlayInfo->curStreamPos = STREAM_FILE_HDR_SIZE;
        DPRINT(DISK_MANAGER, "record found in iframe file but start from beginning: [time=%02d:%02d:%02d]", iFrameTime.tm_hour, iFrameTime.tm_min, iFrameTime.tm_sec);
    }
    else
    {
        pPlayInfo->curStreamPos = iFrameInfo->curStrmPos;
        DPRINT(DISK_MANAGER, "required record found in iframe file: [time=%02d:%02d:%02d]", iFrameTime.tm_hour, iFrameTime.tm_min, iFrameTime.tm_sec);
    }

    free(searchFileData);

    /* close previously open stream file and i frame file */
    CloseFileFd(&pPlayInfo->streamFileFd);
    CloseFileFd(&pPlayInfo->iFrmFileFd);

    /* get information of stream file id, stream position for given position of this record */
    if (getStreamFileName(pPlaySession->playDir, pPlayInfo->streamFileIdx, pPlayInfo->streamFileName, &pPlayInfo->fileEndPos) == FAIL)
    {
        EPRINT(DISK_MANAGER, "stream file not found: [directory=%s], [streamFileIdx=%d]", pPlaySession->playDir, pPlayInfo->streamFileIdx);
        return FAIL;
    }

    // Open pPlaySession->playInfo.streamFileName
    pPlayInfo->streamFileFd = open(pPlayInfo->streamFileName, READ_ONLY_MODE, FILE_PERMISSION);
    if (pPlayInfo->streamFileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open stream file: [file=%s], [err=%s]", pPlayInfo->streamFileName, STR_ERR);
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "file data info: [curStreamPos=%d], [fileEndPos=%d]", pPlayInfo->curStreamPos, pPlayInfo->fileEndPos);

    /* read FSH from the stream file */
    do
    {
        if (pPlayInfo->fileEndPos < (pPlayInfo->curStreamPos + MAX_FSH_SIZE))
        {
            pPlayInfo->streamFileIdx++;
            if(pPlayInfo->streamFileIdx > pPlaySession->stopFileId)
            {
                EPRINT(DISK_MANAGER, "stream file not available in record: [session=%d]", sessionId);
                CloseFileFd(&pPlayInfo->streamFileFd);
                return FAIL;
            }

            /* close previously open stream file and i frame file */
            CloseFileFd(&pPlayInfo->streamFileFd);
            CloseFileFd(&pPlayInfo->iFrmFileFd);

            /* get information of stream file id, stream position for given position of this record */
            if (getStreamFileName(pPlaySession->playDir, pPlayInfo->streamFileIdx, pPlayInfo->streamFileName, &pPlayInfo->fileEndPos) == FAIL)
            {
                EPRINT(DISK_MANAGER, "stream file not found: [directory=%s], [streamFileIdx=%d]", pPlaySession->playDir, pPlayInfo->streamFileIdx);
                return FAIL;
            }

            /* Open pPlaySession->pPlayInfo.streamFileName */
            pPlayInfo->streamFileFd = open(pPlayInfo->streamFileName, READ_ONLY_MODE, FILE_PERMISSION);
            if(pPlayInfo->streamFileFd == INVALID_FILE_FD)
            {
                EPRINT(DISK_MANAGER, "fail to open stream file: [file=%s], [err=%s]", pPlayInfo->streamFileName, STR_ERR);
                return FAIL;
            }

            pPlayInfo->curStreamPos = STREAM_FILE_HDR_SIZE;
        }

        if (lseek(pPlayInfo->streamFileFd, pPlayInfo->curStreamPos, SEEK_SET) != (pPlayInfo->curStreamPos))
        {
            EPRINT(DISK_MANAGER, "fail to set position for stream file: [file=%s], [curStreamPos=%d], [err=%s]",
                   pPlayInfo->streamFileName, pPlayInfo->curStreamPos, STR_ERR);
            CloseFileFd(&pPlayInfo->streamFileFd);
            return FAIL;
        }

        lastReadStreamPos = pPlayInfo->curStreamPos;

        /* Read FSH from current file position into fshBuff */
        if (read(pPlayInfo->streamFileFd, &fshInfo, MAX_FSH_SIZE) != MAX_FSH_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read stream file: [file=%s], [curStreamPos=%d], [err=%s]",
                   pPlayInfo->streamFileName, pPlayInfo->curStreamPos, STR_ERR);
            CloseFileFd(&pPlayInfo->streamFileFd);
            return FAIL;
        }

        if (fshInfo.startCode != FSH_START_CODE)
        {
            EPRINT(DISK_MANAGER, "invalid FSH info: [file=%s]", pPlayInfo->streamFileName);
            CloseFileFd(&pPlayInfo->streamFileFd);
            return FAIL;
        }

        /* get current fsh position and current fsh length */
        pPlayInfo->curStreamPos = fshInfo.nextFShPos;
        pPlayInfo->curStreamLen = fshInfo.fshLen;
        pPlayInfo->nextFshPos = fshInfo.nextFShPos;
        pPlayInfo->prevFshPos = fshInfo.prevFshPos;

        if (fshInfo.mediaType == STREAM_TYPE_VIDEO)
        {
            pPlayInfo->iFrameIdx = fshInfo.iFrmIdxNo;
        }

        playPosTimeInMiliSec = ((UINT64)playPosTimeSec * 1000) + mSec;
        fshTimeInMiliSec = ((UINT64)fshInfo.localTime.totalSec * 1000) + fshInfo.localTime.mSec;

        if (((playCmd == PLAY_REVERSE) || (playCmd == STEP_REVERSE)) && (playPosTimeInMiliSec <= fshTimeInMiliSec))
        {
            pPlayInfo->curStreamPos = pPlayInfo->prevFshPos;
            break;
        }
        else if (((playCmd == PLAY_FORWARD) || (playCmd == STEP_FORWARD)) && (playPosTimeInMiliSec < fshTimeInMiliSec))
        {
            pPlayInfo->curStreamPos = lastReadStreamPos;
            break;
        }

    } while(TRUE);

    /* stored i frame file name */
    snprintf(pPlayInfo->iFrameFile, MAX_FILE_NAME_SIZE, "%s%s", pPlaySession->playDir, I_FRAME_FILE_NAME);

    /* open i frame file and set file descriptor in play back data structure */
    pPlayInfo->iFrmFileFd = open(pPlayInfo->iFrameFile, READ_ONLY_MODE, FILE_PERMISSION);
    if (pPlayInfo->iFrmFileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open iframe file: [file=%s]", pPlayInfo->iFrameFile);
        CloseFileFd(&pPlayInfo->streamFileFd);
        return FAIL;
    }

    /* For reverse playback, we have to move 2 frames back because when playing step reverse only I-frames are played.
     * so that when position is set single frame is read two times to avoid such situation we have to move 2 step back */
    pPlayInfo->lastIframeF = FALSE;
    if (playCmd == STEP_REVERSE)
    {
        if(pPlayInfo->iFrameIdx > 1)
        {
            pPlayInfo->iFrameIdx -= 2;
        }
    }
    else
    {
        if(pPlayInfo->iFrameIdx > 0)
        {
            pPlayInfo->iFrameIdx -= 1;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives playback dir
 * @param   playId
 * @param   playDir
 */
static void getPlayBackDir(PLAY_SESSION_ID playId, CHARPTR playDir)
{
    if (playId < MAX_PLAYBACK_DISK_SESSION)
    {
        snprintf(playDir, MAX_FILE_NAME_SIZE, "%s", playSession[playId].playDir);
    }
    else
    {
        playDir[0] = '\0';
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives current playback file and its position
 * @param   playId
 * @param   startFile
 * @param   startPos
 */
static void getPlayBackPosition(PLAY_SESSION_ID playId, UINT8PTR startFile, UINT32PTR startPos)
{
    if (playId < MAX_PLAYBACK_DISK_SESSION)
    {
        *startFile = playSession[playId].playInfo.streamFileIdx;
        *startPos = playSession[playId].playInfo.curStreamPos;
    }
    else
    {
        *startFile = 0;
        *startPos = 0;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function reads a media frame from current open file at current position and move
 *          to next position.
 * @param   sessionId
 * @param   playCmd
 * @param   frameType
 * @param   fshData
 * @param   frameData
 * @param   frameLen
 * @param   errorCode
 * @param   nextFrameTime
 * @return  SUCCESS/FAIL
 */
BOOL ReadRecordFrame(PLAY_SESSION_ID sessionId, PLAYBACK_CMD_e playCmd, NW_FRAME_TYPE_e frameType, FSH_INFO_t *fshData,
                     UINT8PTR *frameData, UINT32PTR frameLen, PLAYBACK_FILE_READ_e *errorCode, UINT64PTR nextFrameTime)
{
    INT32 					payloadSize;
    UINT32 					initFrameLen = 0;
    UINT32 					iFrmOffset;
    VOIDPTR					tempPtr;
    PLAYBACK_SESSION_t 		*pPlaySession;
    PLAY_INFO_t 			*playInfo;
    FSH_INFO_t 				nextFshInfo;
    IFRM_FIELD_INFO_t 		iFrmField;

    // validate input parameter
    if (sessionId >= MAX_PLAYBACK_DISK_SESSION)
    {
        EPRINT(DISK_MANAGER, "invld playback session: [session=%d]", sessionId);
        return FAIL;
    }

    if (FALSE == IsStorageOperationalForRead(playSession[sessionId].playCntrl.recStorageType))
    {
        EPRINT(DISK_MANAGER, "disk status not proper to proceed: [session=%d]", sessionId);
        *errorCode = PLAYBACK_FILE_READ_HDD_STOP;
        return FAIL;
    }

    // Get session parameters from sessionId
    pPlaySession = &playSession[sessionId];
    playInfo = &pPlaySession->playInfo;

    // in case of file reading we should give a buffer that have sufficient memory
    *frameData = pPlaySession->playBuff.buffPtr;
    initFrameLen = *frameLen;

    /* gives frame as output parameter as per playback commad */
    switch(playCmd)
    {
        case STEP_FORWARD:
        {
            /* Disable audio for step */
            pPlaySession->audioEnbl = DISABLE;
            playInfo->iFrameIdx += 1;
        }

        /* FALLS THROUGH */
        case PLAY_FORWARD:
        {
            /* user need only I frame file */
            if (frameType == NW_I_FRAME)
            {
                /* read i frame information get i frame index number position */
                if (pPlaySession->speed == PLAY_16X)
                {
                    playInfo->iFrameIdx += 1;
                }

                iFrmOffset = ((playInfo->iFrameIdx * IFRAME_FIELD_SIZE) + IFRAME_FILE_HDR_SIZE);
                if (lseek(playInfo->iFrmFileFd, iFrmOffset, SEEK_SET) != iFrmOffset)
                {
                    EPRINT(DISK_MANAGER, "fail to seek i-frame file: [session=%d], [err=%s]", sessionId, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_OVER;
                    return FAIL;
                }

                /* read i frame information and get stream file index number and stream file position */
                if (read(playInfo->iFrmFileFd, &iFrmField, sizeof(IFRM_FIELD_INFO_t)) != IFRAME_FIELD_SIZE)
                {
                    DPRINT(DISK_MANAGER, "i-frame playback over: [session=%d], [iFrmFileFd=%d], [err=%s]", sessionId, playInfo->iFrmFileFd, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_OVER;
                    return FAIL;
                }

                if (iFrmField.strmFileId > pPlaySession->stopFileId)
                {
                    DPRINT(DISK_MANAGER, "playback over: [session=%d], [stopFileId=%d], [strmFileId=%d]",
                           sessionId, pPlaySession->stopFileId, iFrmField.strmFileId);
                    *errorCode = PLAYBACK_FILE_READ_OVER;
                    return FAIL;
                }

                /* stream file was not same */
                if (iFrmField.strmFileId != playInfo->streamFileIdx)
                {
                    /* close privously open stream file */
                    CloseFileFd(&playInfo->streamFileFd);

                    playInfo->streamFileIdx = iFrmField.strmFileId;

                    /* get stream file name */
                    if(getStreamFileName(pPlaySession->playDir, playInfo->streamFileIdx, playInfo->streamFileName, &playInfo->fileEndPos) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to get stream file name: [session=%d], [streamFileIdx=%d]", sessionId, playInfo->streamFileIdx);
                        *errorCode = PLAYBACK_FILE_READ_ERROR;
                        return FAIL;
                    }

                    /* open that stream file */
                    playInfo->streamFileFd = open(playInfo->streamFileName, READ_ONLY_MODE, FILE_PERMISSION);
                    if (playInfo->streamFileFd == INVALID_FILE_FD)
                    {
                        EPRINT(DISK_MANAGER, "fail to open stream file: [session=%d], [path=%s], [err=%s]", sessionId, playInfo->streamFileName, STR_ERR);
                        *errorCode = PLAYBACK_FILE_READ_ERROR;
                        return FAIL;
                    }
                }

                playInfo->curStreamPos = iFrmField.curStrmPos;
                /* set stream file position */
                if(lseek(playInfo->streamFileFd, playInfo->curStreamPos, SEEK_SET) != (playInfo->curStreamPos))
                {
                    EPRINT(DISK_MANAGER, "fail to seek stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }

                /* read FSh from current stream file position */
                if(read(playInfo->streamFileFd, fshData, MAX_FSH_SIZE) != MAX_FSH_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }

                playInfo->prevFshPos = fshData->prevFshPos;
                playInfo->nextFshPos = fshData->nextFShPos;
                playInfo->curStreamPos = fshData->nextFShPos;
                playInfo->curStreamLen = fshData->fshLen;
                playInfo->curTimeSec = fshData->localTime.totalSec;
                playInfo->iFrameIdx = fshData->iFrmIdxNo;
            }
            else
            {
                do
                {
                    /* check current file of record is over or not if current file was over then move to next file of this record */
                    if((playInfo->curStreamPos + MAX_FSH_SIZE) >= playInfo->fileEndPos)
                    {
                        /* current stream file was complete and move to next stream file */
                        playInfo->streamFileIdx++;
                        if((playInfo->streamFileIdx >= pPlaySession->startFileId) && (playInfo->streamFileIdx <= pPlaySession->stopFileId))
                        {
                            /* close previously open stream file */
                            CloseFileFd(&playInfo->streamFileFd);

                            /* get stream file name and file size */
                            if(getStreamFileName(pPlaySession->playDir, playInfo->streamFileIdx, playInfo->streamFileName, &playInfo->fileEndPos) == FAIL)
                            {
                                EPRINT(DISK_MANAGER, "fail to get stream file name: [session=%d], [streamFileIdx=%d]", sessionId, playInfo->streamFileIdx);
                                *errorCode = PLAYBACK_FILE_READ_ERROR;
                                return FAIL;
                            }

                            /* open stream file */
                            playInfo->streamFileFd = open(playInfo->streamFileName, READ_ONLY_MODE, FILE_PERMISSION);
                            if (playInfo->streamFileFd == INVALID_FILE_FD)
                            {
                                EPRINT(DISK_MANAGER, "fail to open stream file: [session=%d], [path=%s], [err=%s]", sessionId, playInfo->streamFileName, STR_ERR);
                                *errorCode = PLAYBACK_FILE_READ_ERROR;
                                return FAIL;
                            }

                            /* set stream file postion to first FSH */
                            playInfo->curStreamPos = STREAM_FILE_HDR_SIZE;
                        }
                        else
                        {
                            /* playback was over and no more frame is available for this record */
                            DPRINT(DISK_MANAGER, "playback over: [session=%d], [streamFileIdx=%d], [startFileId=%d], [stopFileId=%d]",
                                   sessionId, playInfo->streamFileIdx, pPlaySession->startFileId, pPlaySession->stopFileId);
                            *errorCode = PLAYBACK_FILE_READ_OVER;
                            return FAIL;
                        }
                    }

                    /* set stream file position */
                    if(lseek(playInfo->streamFileFd, playInfo->curStreamPos, SEEK_SET) != (playInfo->curStreamPos))
                    {
                        EPRINT(DISK_MANAGER, "fail to seek stream file: [session=%d], [path=%s], [position=%d], [err=%s]",
                               sessionId, playInfo->streamFileName, playInfo->curStreamPos, STR_ERR);
                        *errorCode = PLAYBACK_FILE_READ_ERROR;
                        return FAIL;
                    }

                    /* read FSH and check this media is type of audio then discard this media and move to next FSH position till video is not detected */
                    if(read(playInfo->streamFileFd, fshData, MAX_FSH_SIZE) != MAX_FSH_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                        *errorCode = PLAYBACK_FILE_READ_ERROR;
                        return FAIL;
                    }

                    /* update all information from read FSH in readBuff */
                    playInfo->prevFshPos = fshData->prevFshPos;
                    playInfo->nextFshPos = fshData->nextFShPos;
                    playInfo->curStreamPos = fshData->nextFShPos;
                    playInfo->curStreamLen = fshData->fshLen;
                    playInfo->curTimeSec = fshData->localTime.totalSec;

                    if (fshData->mediaType == STREAM_TYPE_VIDEO)
                    {
                        playInfo->iFrameIdx = fshData->iFrmIdxNo;
                    }

                    /* Audio frame was present then move to next frame in recorded file */
                    if ((fshData->mediaType == STREAM_TYPE_AUDIO) && (pPlaySession->audioEnbl == DISABLE))
                    {
                        /* Skip audio frame */
                        continue;
                    }
                    break;

                } while(TRUE);
            }

            /* calculate actual payload size */
            payloadSize = playInfo->curStreamLen - MAX_FSH_SIZE;
            if ((payloadSize < 0) || (payloadSize > MAX_STREAM_FRAME_LENGTH)) // 2 times VIDEO_BUF_SIZE
            {
                EPRINT(DISK_MANAGER, "frame payload size is more than buffer size: [session=%d], [payloadSize=%d]", sessionId, payloadSize);
                *errorCode = PLAYBACK_FILE_READ_ERROR;
                return FAIL;
            }

            if ((payloadSize > 0) && (pPlaySession->playBuff.buffSize < (payloadSize + initFrameLen)))
            {
                DPRINT(DISK_MANAGER, "playback buffer memory reallocation: [session=%d], [old=%d], [new=%d]",
                       sessionId, pPlaySession->playBuff.buffSize, (payloadSize + initFrameLen));
                /* PARASOFT : No need to validate tainted data */
                tempPtr = realloc(pPlaySession->playBuff.buffPtr,(payloadSize + initFrameLen));
                if(tempPtr == NULL)
                {
                    EPRINT(DISK_MANAGER, "memory allocation failed: [session=%d], [size=%d]", sessionId, (payloadSize + initFrameLen));
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }

                pPlaySession->playBuff.buffPtr = tempPtr;
                pPlaySession->playBuff.buffSize = (payloadSize + initFrameLen);
            }

            if (payloadSize > 0)
            {
                if (read(playInfo->streamFileFd, pPlaySession->playBuff.buffPtr + initFrameLen, payloadSize) != payloadSize)
                {
                    EPRINT(DISK_MANAGER, "fail to read stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }
            }

            if((playInfo->curStreamPos + MAX_FSH_SIZE) < playInfo->fileEndPos)
            {
                if (pread(playInfo->streamFileFd, &nextFshInfo, MAX_FSH_SIZE, playInfo->nextFshPos) != (INT32)MAX_FSH_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to pread stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }
                *nextFrameTime = (UINT64)(((UINT64)nextFshInfo.localTime.totalSec * MSEC_IN_ONE_SEC) + nextFshInfo.localTime.mSec);
            }
            else
            {
                *nextFrameTime = (UINT64)(((UINT64)fshData->localTime.totalSec * MSEC_IN_ONE_SEC) + fshData->localTime.mSec);
            }

            /* give payload size as output parameter */
            *frameData = pPlaySession->playBuff.buffPtr;
            *frameLen = (payloadSize + initFrameLen);

            /* Stop the playback if local time is bigger than playback time */
            if ((time_t)fshData->localTime.totalSec > pPlaySession->playStopTime)
            {
                /* For any error we need to send frame header only */
                WPRINT(DISK_MANAGER, "file is over: [session=%d]", sessionId);
                *frameLen = initFrameLen;
                *errorCode = PLAYBACK_FILE_READ_OVER;
                return FAIL;
            }
            else
            {
                *errorCode = PLAYBACK_FILE_READ_NORMAL;
            }
        }
        break;

        case STEP_REVERSE:
        {
            /* Disable audio for step */
            pPlaySession->audioEnbl = DISABLE;
        }

        /* FALLS THROUGH */
        case PLAY_REVERSE:
        {
            /* Don't skip last i-frame index to play */
            if ((playInfo->lastIframeF == TRUE) && (playInfo->iFrameIdx == 0))
            {
                WPRINT(DISK_MANAGER, "i-frame file read over: [session=%d]", sessionId);
                *errorCode = PLAYBACK_FILE_READ_OVER;
                return FAIL;
            }

            /* read i frame information get i frame index number position */
            if (pPlaySession->speed == PLAY_16X)
            {
                if (playInfo->iFrameIdx > 0)
                {
                    playInfo->iFrameIdx--;
                }
            }

            /* calculate i frame index position from i frame index number */
            iFrmOffset = ((playInfo->iFrameIdx * IFRAME_FIELD_SIZE) + IFRAME_FILE_HDR_SIZE);
            if (lseek(playInfo->iFrmFileFd, iFrmOffset, SEEK_SET) != iFrmOffset)
            {
                EPRINT(DISK_MANAGER, "fail to seek i-frame file: [session=%d], [err=%s]", sessionId, STR_ERR);
                *errorCode = PLAYBACK_FILE_READ_OVER;
                return FAIL;
            }

            /* read i frame information get stream file index number and stream file position */
            if (read(playInfo->iFrmFileFd, &iFrmField, IFRAME_FIELD_SIZE) != IFRAME_FIELD_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read i-frame file: [session=%d], [err=%s]", sessionId, STR_ERR);
                *errorCode = PLAYBACK_FILE_READ_ERROR;
                return FAIL;
            }

            if(iFrmField.strmFileId < pPlaySession->startFileId)
            {
                DPRINT(DISK_MANAGER, "play back over: [session=%d]", sessionId);
                *errorCode = PLAYBACK_FILE_READ_OVER;
                return SUCCESS;
            }

            /* check this I frame is in current working stream file */
            if (playInfo->streamFileIdx != iFrmField.strmFileId)
            {
                /* close privously open stream file */
                CloseFileFd(&playInfo->streamFileFd);

                /* get stream file name */
                playInfo->streamFileIdx = iFrmField.strmFileId;
                if(getStreamFileName(pPlaySession->playDir, iFrmField.strmFileId, playInfo->streamFileName, &playInfo->fileEndPos) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to get stream file name: [session=%d], [streamFileIdx=%d]", sessionId, iFrmField.strmFileId);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }

                /* open that stream file */
                playInfo->streamFileFd = open(playInfo->streamFileName, READ_ONLY_MODE, FILE_PERMISSION);
                if (playInfo->streamFileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open stream file: [session=%d], [path=%s], [err=%s]", sessionId, playInfo->streamFileName, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }
            }

            /* update new stream position in stream file */
            playInfo->curStreamPos = iFrmField.curStrmPos;

            /* go to current stream position in stream file set file position */
            if (lseek(playInfo->streamFileFd, playInfo->curStreamPos, SEEK_SET) != (playInfo->curStreamPos))
            {
                EPRINT(DISK_MANAGER, "fail to seek stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                *errorCode = PLAYBACK_FILE_READ_ERROR;
                return FAIL;
            }

            /* read fsh information from current file position */
            if (read(playInfo->streamFileFd, fshData, MAX_FSH_SIZE) != MAX_FSH_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                *errorCode = PLAYBACK_FILE_READ_ERROR;
                return FAIL;
            }

            /* Validate parameters to avoid buffer overflow */
            if ((fshData->codecType >= MAX_VIDEO_CODEC) || (pPlaySession->speed >= MAX_PLAY_SPEED))
            {
                EPRINT(DISK_MANAGER, "invld video codec or speed: [session=%d], [codec=%d], [speed=%d]", sessionId, fshData->codecType, pPlaySession->speed);
                *errorCode = PLAYBACK_FILE_READ_ERROR;
                return FAIL;
            }

            /* update all information from read FSH in readBuff */
            playInfo->prevFshPos = fshData->prevFshPos;
            playInfo->nextFshPos = fshData->nextFShPos;
            playInfo->curStreamPos = fshData->prevFshPos;
            playInfo->curStreamLen = fshData->fshLen;
            playInfo->curTimeSec = fshData->localTime.totalSec;
            if (playInfo->iFrameIdx > 0)
            {
                playInfo->iFrameIdx--;
            }
            else
            {
                /* Now we have played last i-frame, consider file play over. */
                playInfo->lastIframeF = TRUE;
            }

            /* calculate actual payload size */
            payloadSize = playInfo->curStreamLen - MAX_FSH_SIZE;
            if ((payloadSize > 0)&& (pPlaySession->playBuff.buffSize < (payloadSize + initFrameLen)))
            {
                DPRINT(DISK_MANAGER, "playback buffer memory reallocation: [session=%d], [old=%d], [new=%d]",
                       sessionId, pPlaySession->playBuff.buffSize, (payloadSize + initFrameLen));

                /* PARASOFT : No need to validate tainted data */
                tempPtr = realloc(pPlaySession->playBuff.buffPtr, (payloadSize + initFrameLen));
                if(tempPtr == NULL)
                {
                    EPRINT(DISK_MANAGER, "memory allocation failed: [session=%d], [size=%d]", sessionId, (payloadSize + initFrameLen));
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }

                pPlaySession->playBuff.buffPtr = tempPtr;
                pPlaySession->playBuff.buffSize = (payloadSize + initFrameLen);
            }

            if(payloadSize > 0)
            {
                if (read(playInfo->streamFileFd, pPlaySession->playBuff.buffPtr + initFrameLen, payloadSize) != (INT32)payloadSize)
                {
                    EPRINT(DISK_MANAGER, "fail to read stream file: [session=%d], [err=%s]", sessionId, STR_ERR);
                    *errorCode = PLAYBACK_FILE_READ_ERROR;
                    return FAIL;
                }
            }

            *nextFrameTime = (UINT64)(((UINT64)fshData->localTime.totalSec * MSEC_IN_ONE_SEC) + fshData->localTime.mSec);

            /* give payload size as output parameter */
            *frameData = pPlaySession->playBuff.buffPtr;
            *frameLen = payloadSize + initFrameLen;

            /* Stop the playback if local time is smaller than playback time */
            if((time_t)fshData->localTime.totalSec < pPlaySession->playStartTime)
            {
                /* For any error we need to send frame header only */
                *frameLen = initFrameLen;
                *errorCode = PLAYBACK_FILE_READ_OVER;
                return FAIL;
            }
            else
            {
                *errorCode = PLAYBACK_FILE_READ_NORMAL;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Upon Hard disk error we should close all the file descriptor
 */
static void closeAllFileOnHddErr(void)
{
    UINT8 channelCnt;

    DPRINT(DISK_MANAGER, "stopping all disk manager session");
    for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++ )
    {
        closeChnlFiles(channelCnt);
    }
    DPRINT(DISK_MANAGER, "stopped all disk manager session");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   close all the file descriptor of a particular channel
 * @param   iChnlNo
 */
static void closeChnlFiles(UINT8 iChnlNo)
{
    if (iChnlNo >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    DM_SESSION_t *dmSession = &sessionInfo[iChnlNo];
    CloseFileFd(&dmSession->streamFileInfo.fileFd);
    CloseFileFd(&dmSession->iFrameFileInfo.fileFd);
    CloseFileFd(&dmSession->evntFileInfo.fileFd);
    CloseFileFd(&dmSession->timeIdxFileInfo.fileFd);
    CloseFileFd(&dmSession->yearMapFileInfo.mapFileFd);

    MUTEX_LOCK(dmCondMutex);
    channelWriterStatus[iChnlNo] = FALSE;
    MUTEX_UNLOCK(dmCondMutex);

    // Initialize all parameters for event file storage
    memset(&dmSession->mScheduleEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mScheduleEvent.hourUpdate = TRUE;

    memset(&dmSession->mManualEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mManualEvent.hourUpdate = TRUE;

    memset(&dmSession->mAlarmEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mAlarmEvent.hourUpdate = TRUE;

    memset(&dmSession->mCosecEvent, 0, sizeof(EVENT_INDEX_HEADER_t));
    dmSession->mCosecEvent.hourUpdate = TRUE;

    dmSession->sessionStatus = OFF;
    dmSession->firstFrameRec = FALSE;
    dmSession->overlapFlg = 0;

    memset(&dmSession->streamFileInfo, 0, sizeof(STRM_FILE_INFO_t));
    memset(&dmSession->iFrameFileInfo, 0, sizeof(I_FRM_FILE_INFO_t));
    memset(&dmSession->evntFileInfo, 0, sizeof(EVNT_FILE_INFO_t));
    memset(&dmSession->timeIdxFileInfo, 0, sizeof(TIME_IDX_FILE_INFO_t));

    dmSession->streamFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->iFrameFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->evntFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->timeIdxFileInfo.fileFd = INVALID_FILE_FD;
    dmSession->yearMapFileInfo.mapFileFd = INVALID_FILE_FD;

    /* reset disk manager buffer */
    ResetChannelBuffer(iChnlNo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was thread call back function for disk manager module. This function writes
 *          data into corresponding file of all channel. After written data thread goes into sleep state.
 * @param   threadParam
 * @return  SUCCESS/FAIL
 */
static VOIDPTR writterThread(VOIDPTR threadParam)
{
    BOOL					status = FAIL;
    UINT32                  tErrorCode = INVALID_ERROR_CODE;
    UINT8 					channelCnt, volGrpId;
    UINT8 					writerChannelMax;
    UINT32					wrMemoryCnt[STORAGE_ALLOCATION_GROUP_MAX] = {0};
    BUFFER_t				*writeChannel;
    AVI_CONVERTER_MSG_t		aviMsg;
    HDD_CONFIG_t			hddConfig;
    UINT8                   storageUpdateCnt;

    THREAD_START("DM WRITER");

    setpriority(PRIO_PROCESS, PRIO_PROCESS, -1);
    writerChannelMax = getMaxCameraForCurrentVariant();

    /* For 4, 8, and 16 channel NVR, it is 16; 32 channel NVR, it is 32, 64 and more channel NVR, it is 64 */
    storageUpdateCnt = (writerChannelMax < 16) ? 16 : MIN(64, (writerChannelMax/16)*16);

    while(TRUE)
    {
        MUTEX_LOCK(dmCondMutex);
        for(channelCnt = 0; channelCnt < writerChannelMax; channelCnt++)
        {
            if (TRUE == channelWriterStatus[channelCnt])
            {
                break;
            }
        }

        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        if ((dmTerminateFlg == TRUE) && (channelCnt >= writerChannelMax))
        {
            MUTEX_UNLOCK(dmCondMutex);
            break;
        }

        // Check any buffer is ready for writing data into files
        if (channelCnt >= writerChannelMax)
        {
            // no buffered data is available for write thread goes into sleep state
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            pthread_cond_wait(&dmCondSignal, &dmCondMutex);
        }
        MUTEX_UNLOCK(dmCondMutex);

        ReadHddConfig(&hddConfig);
        for(channelCnt = 0; channelCnt < writerChannelMax; channelCnt++)
        {
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            MUTEX_LOCK(dmCondMutex);
            if (FALSE == channelWriterStatus[channelCnt])
            {
                MUTEX_UNLOCK(dmCondMutex);
                continue;
            }
            MUTEX_UNLOCK(dmCondMutex);

            volGrpId = 0;
            writeChannel = &sessionInfo[channelCnt].mDmBufferMngr;
            setStorageCalculationInfo(sessionInfo[channelCnt].elapsedTickCnt, writeChannel->streamOffset, channelCnt);
            if (hddConfig.recordDisk == LOCAL_HARD_DISK)
            {
                volGrpId = GetCameraStorageAllocationGroup(channelCnt, hddConfig.mode);
            }

            if (volGrpId < STORAGE_ALLOCATION_GROUP_MAX)
            {
                if (wrMemoryCnt[volGrpId] >= storageUpdateCnt)
                {
                    /* Here we need to check whether the disk has sufficient size to dump the 2 MB buffered data */
                    wrMemoryCnt[volGrpId] = 0;
                    if (UpdateCameraStorage(channelCnt) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to update the storage: [camera=%d]", channelCnt);
                    }
                }
                wrMemoryCnt[volGrpId]++;
            }
            else
            {
                EPRINT(DISK_MANAGER, "invld camera volume group: [camera=%d]", channelCnt);
            }

            // update stream file data
            status = updateStreamFile(writeChannel, &tErrorCode);
            if (FAIL == status)
            {
                EPRINT(DISK_MANAGER, "fail to update stream file: [camera=%d], [file=%s]", channelCnt, writeChannel->streamFileName);
                HandleDiskError(channelCnt, tErrorCode);
                break;
            }

            // update I frame file data
            if((aviRecGenConfig.recordFormatType != REC_AVI_FORMAT) && (status = updateIFrameFile(writeChannel, &tErrorCode)) == FAIL)
            {
                HandleDiskError(channelCnt, tErrorCode);
                break;
            }

            // close stream file
            MUTEX_LOCK(writeChannel->buffMutex);
            if(writeChannel->streamFileClose == TRUE)
            {
                MUTEX_UNLOCK(writeChannel->buffMutex);

                status = closeStreamFile(writeChannel, &tErrorCode);
                if(FAIL == status)
                {
                    HandleDiskError(channelCnt, tErrorCode);
                    break;
                }

                MUTEX_LOCK(writeChannel->buffMutex);
                writeChannel->streamFileClose = FALSE;
                MUTEX_UNLOCK(writeChannel->buffMutex);

                if((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) || (aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT))
                {
                    snprintf(aviMsg.fileName, MAX_FILE_NAME_SIZE, "%s", writeChannel->streamFileName);
                    MUTEX_LOCK(dmCondMutex);
                    aviMsg.aviConvertType = (TRUE == channelHourSlotChange[channelCnt]) ? WHOLE_FOLDER_REM : SINGLE_FILE_CONVERT;
                    channelHourSlotChange[channelCnt] = FALSE;
                    MUTEX_UNLOCK(dmCondMutex);
                    writeaviCnvrtMesg( &aviMsg);
                }
            }
            else
            {
                MUTEX_UNLOCK(writeChannel->buffMutex);
            }

            /** Close I-frame file */
            MUTEX_LOCK(writeChannel->buffMutex);
            if(writeChannel->iFrameFileClose == TRUE)
            {
                MUTEX_UNLOCK(writeChannel->buffMutex);
                if(aviRecGenConfig.recordFormatType != REC_AVI_FORMAT)
                {
                    status = closeIFrameFile(writeChannel, &tErrorCode);
                    if(FAIL == status)
                    {
                        EPRINT(DISK_MANAGER, "fail to close i-frame file: [camera=%d], [err=%s]", channelCnt, strerror(tErrorCode));
                        HandleDiskError(channelCnt, tErrorCode);
                        break;
                    }
                }
                MUTEX_LOCK(writeChannel->buffMutex);
                writeChannel->iFrameFileClose = FALSE;
                MUTEX_UNLOCK(writeChannel->buffMutex);
            }
            else
            {
                MUTEX_UNLOCK(writeChannel->buffMutex);
            }

            if (status == SUCCESS)
            {
                MUTEX_LOCK(dmCondMutex);
                channelWriterStatus[channelCnt] = FALSE;
                MUTEX_UNLOCK(dmCondMutex);
            }
        }
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Below api handel the disk error, based on the error code below api either restart the
 *          recording of a particular channel or perform disk fault activity if drive is in fault
 * @param   channelNo - Channel for which error occured
 * @param   errorCode - Error code, this is set in case of any failure in disk operation(read,write,mkdir etc) with the particular channel.
 */
void HandleDiskError(UINT8 channelNo, UINT32 errorCode)
{
    CHAR            searchPath[MOUNT_POINT_SIZE] = {0};
    CHAR            diskName[MOUNT_POINT_SIZE] = {0};
    HDD_CONFIG_t    hddConfig;

    do
    {
        /* Take drive config from the stream file path first, if it is not available then take from recording path */
        if (sessionInfo[channelNo].streamFileInfo.filename[0] != '\0')
        {
            snprintf(searchPath, sizeof(searchPath), "%s", sessionInfo[channelNo].streamFileInfo.filename);
        }
        else
        {
            if (FAIL == GetRecordingPath(channelNo, searchPath))
            {
                EPRINT(DISK_MANAGER, "fail to get recording path config: [camera=%d], [filename=%s]", channelNo, sessionInfo[channelNo].streamFileInfo.filename);
                break;
            }
        }

        /* Get disk name from mount path */
        if (SUCCESS != GetDiskNameFromMountPoint(searchPath, diskName))
        {
            EPRINT(DISK_MANAGER, "fail to get hdd config: [camera=%d], [searchPath=%s]", channelNo, searchPath);
            break;
        }

        /* Get disk config from disk name */
        if (SUCCESS != GetHddConfigFromDiskName(diskName, &hddConfig))
        {
            EPRINT(DISK_MANAGER, "fail to get hdd config: [camera=%d], [diskName=%s]", channelNo, diskName);
            break;
        }

        /* Check disk is in fault or it is a particular channel error */
        if (FALSE == isDiskInFault(errorCode, hddConfig.recordDisk))
        {
            break;
        }

        /* Close all opened file */
        closeAllFileOnHddErr();

        /* If it is local disk recording then check for other volume. If available then switch recording else reboot the system */
        if(hddConfig.recordDisk == LOCAL_HARD_DISK)
        {
            CHAR advanceDetails[MAX_EVENT_ADVANCE_DETAIL_SIZE];
            if (SUCCESS == SetDiskVolumeFault(channelNo, hddConfig.mode, ((errorCode == EROFS) ? DM_DISK_VOL_READ_ONLY : DM_DISK_VOL_FAULT), advanceDetails))
            {
                /* Update to storage about failure */
                UpdateCameraStorage(channelNo);
                return;
            }
        }

        /* Stop all the activity on the disk and take particular action based on the disk type */
        HandleFileIOError(channelNo, &hddConfig);
        return;

    }while(0);

    /* close the channel files */
    closeChnlFiles(channelNo);

    /* Particular channel error restart the recording */
    RestartRecSession(channelNo, errorCode);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was closed the current working stream file and sets proper stream header
 *          into file. It also sets end of file indicator and previous FSH length after end of file.
 *          And sets its file name based on last frame time.
 * @param   pFileInfo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL closeStreamFile(BUFFER_t *pFileInfo, UINT32PTR pErrorCode)
{
    STRM_FILE_HDR_t strmFileHdr;
    STRM_FILE_END_t strmFileEnd;
    CHARPTR         charSearch;
    CHAR            tempName[MAX_FILE_NAME_SIZE];
    CHAR            reName[MAX_FILE_NAME_SIZE];

    // validate input parameter
    if (pFileInfo == NULL)
    {
        EPRINT(DISK_MANAGER, "null pointer found");
        return FAIL;
    }

    // stored stream file sign
    strmFileHdr.fileSign = STREAM_FILE_SIGN;

    // version of file
    strmFileHdr.version = STREAM_FILE_VERSION;

    // set next stream position
    strmFileHdr.nextStreamPos = pFileInfo->nextStreamPos;

    // set run flag here to 1
    strmFileHdr.runFlg = FALSE;

    // set backup flag. file was newly created so set this flag to 0 to indicates no back up is taken
    strmFileHdr.backupFlg = 0;

    //Device type
    strmFileHdr.deviceType = DM_DEVICE_ID;

    // Make reserve byte 0
    strmFileHdr.reserved = 0;

    if(Utils_pWrite(pFileInfo->streamFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, 0, pErrorCode) != STREAM_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write in stream file: [file=%s], [err=%s]", pFileInfo->streamFileName, strerror(*pErrorCode));
        return FAIL;
    }

    // set end of file indicator
    strmFileEnd.fileEndIndicator = STRM_EOF_INDICATOR;

    // set next stream position
    strmFileEnd.prevFshPos = pFileInfo->prevStreamPos;

    // write data into file
    if(Utils_pWrite(pFileInfo->streamFileFd, &strmFileEnd, STREAM_FILE_END_SIZE, pFileInfo->nextStreamPos, pErrorCode) != STREAM_FILE_END_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write in stream file: [file=%s], [err=%s]", pFileInfo->streamFileName, strerror(*pErrorCode));
        return FAIL;
    }

    CloseFileFd(&pFileInfo->streamFileFd);

    // file name has uniqueness in its name that was start time and date and stop time and date .
    // start time was added when file was create so here we need to stop time
    charSearch = strchr(pFileInfo->streamFileName, '~');
    if (charSearch == NULL)
    {
        return SUCCESS;
    }

    memcpy(tempName, pFileInfo->streamFileName, (strlen(pFileInfo->streamFileName) - strlen(charSearch)));
    tempName[(strlen(pFileInfo->streamFileName) - strlen(charSearch))]	= '\0';
    charSearch = strchr(pFileInfo->streamFileName, '.');
    if(charSearch == NULL)
    {
        return SUCCESS;
    }

    snprintf(reName, MAX_FILE_NAME_SIZE, "%s~"REC_FOLDER_TIME_FORMAT"%s",
             tempName, pFileInfo->currTime.tm_hour, pFileInfo->currTime.tm_min, pFileInfo->currTime.tm_sec, charSearch);
    if(rename(pFileInfo->streamFileName, reName) !=  STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to rename stream file: [oldName=%s], [newName=%s], [err=%s]", pFileInfo->streamFileName, reName, STR_ERR);
        return FAIL;
    }

    if((aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT) || (aviRecGenConfig.recordFormatType == REC_AVI_FORMAT))
    {
        snprintf(pFileInfo->streamFileName, MAX_FILE_NAME_SIZE, "%s", reName);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was closed the current working I frame and sets proper stream header into
 *          file. It also sets end of file indicator.
 * @param   pFileInfo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL closeIFrameFile(BUFFER_t *pFileInfo, UINT32PTR pErrorCode)
{
    BOOL    retVal = FAIL;
    UINT8   bufIdx = 0;
    CHAR    iFrmBuf[2];

    iFrmBuf[bufIdx++] = (UINT8)IFRAME_FILE_EOF;
    iFrmBuf[bufIdx++] = (IFRAME_FILE_EOF >> 8);

    // Set the end of file indicator of I frame file
    if (Utils_Write(pFileInfo->iFrmFileFd, iFrmBuf, bufIdx, pErrorCode) == bufIdx)
    {
        retVal = SUCCESS;
    }

    CloseFileFd(&pFileInfo->iFrmFileFd);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was closed the current working event file and sets proper stream header
 *          into file and also sets end of event file indicator.
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return
 */
static BOOL closeEventFile(UINT8 channelNo, UINT32PTR pErrorCode)
{
    BOOL    retVal = FAIL;
    CHAR    evntBuf[2];
    UINT8   bufIdx = 0;
    UINT32  fileOffset;

    fileOffset = ((sessionInfo[channelNo].evntFileInfo.evntIdxNo * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE);
    evntBuf[bufIdx++] = (UINT8)EVNT_FILE_EOF;
    evntBuf[bufIdx++] = EVNT_FILE_EOF >> 8;

    // Set the end of file indicator of event file
    if(Utils_pWrite(sessionInfo[channelNo].evntFileInfo.fileFd, evntBuf, bufIdx, fileOffset, pErrorCode) == bufIdx)
    {
        retVal = SUCCESS;
    }

    CloseFileFd(&sessionInfo[channelNo].evntFileInfo.fileFd);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was closed the current working time index file and sets end of time index
 *          file indicator at end of time index file
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL closeTimeIdxFile(UINT8	channelNo, UINT32PTR pErrorCode)
{
    BOOL    retVal = FAIL;
    UINT8   bufIdx = 0;
    CHAR    timeIdxBuf[2];

    timeIdxBuf[bufIdx++] = (UINT8)TIMEINDEX_FILE_EOF;
    timeIdxBuf[bufIdx++] = TIMEINDEX_FILE_EOF >> 8;

    if (Utils_Write(sessionInfo[channelNo].timeIdxFileInfo.fileFd, timeIdxBuf, bufIdx, pErrorCode) == bufIdx)
    {
        retVal = SUCCESS;
    }

    CloseFileFd(&sessionInfo[channelNo].timeIdxFileInfo.fileFd);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was closed the current working Month and day index files
 * @param   channelNo
 * @return  SUCCESS/FAIL
 */
static BOOL closeMonthDayIdxFiles(UINT8	channelNo)
{
    if (sessionInfo[channelNo].yearMapFileInfo.mapFileFd == INVALID_FILE_FD)
    {
        return FAIL;
    }

    CloseFileFd(&sessionInfo[channelNo].yearMapFileInfo.mapFileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was writes buffer into stream file and updates next stream position in file header
 * @param   fileInfo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return
 */
static BOOL updateStreamFile(BUFFER_t *fileInfo, UINT32PTR pErrorCode)
{
    STRM_FILE_HDR_t strmFileHdr;

    // create stream file headers
    // stored stream file sign
    strmFileHdr.fileSign = STREAM_FILE_SIGN;

    // version of file
    strmFileHdr.version = STREAM_FILE_VERSION;

    // set next stream position
    strmFileHdr.nextStreamPos = fileInfo->nextStreamPos;

    // set run flag here to 1
    strmFileHdr.runFlg = TRUE;

    // file was newly created so set this flag to 0 to indicates no back up is taken
    strmFileHdr.backupFlg = 0;

    //Device Type
    strmFileHdr.deviceType = DM_DEVICE_ID;

    // Make reserved byte 0
    strmFileHdr.reserved = 0;

    if (Utils_Write(fileInfo->streamFileFd, fileInfo->streamData, fileInfo->streamOffset, pErrorCode) != (ssize_t)fileInfo->streamOffset)
    {
        return FAIL;
    }

    // set buffer position to starting position
    fileInfo->streamOffset = 0;

    // write file next stream position
    if (Utils_pWrite(fileInfo->streamFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, 0, pErrorCode) != STREAM_FILE_HDR_SIZE)
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was writes buffer into I frame and updates next I frame index in I frame file header
 * @param   fileInfo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL updateIFrameFile(BUFFER_t *fileInfo, UINT32PTR pErrorCode)
{
    METADATA_FILE_HDR_t iFrmHdr;

    if (Utils_Write(fileInfo->iFrmFileFd, fileInfo->iFrameData, fileInfo->iFrameOffset, pErrorCode) != (ssize_t)fileInfo->iFrameOffset)
    {
        EPRINT(DISK_MANAGER, "fail to write i-frame metadata in file: [err=%s]", strerror(*pErrorCode));
        return FAIL;
    }

    //set master buffer data to 0
    memset(fileInfo->iFrameData, 0, MAX_IFRM_BUF_SIZE);

    // set buffer position to starting position
    fileInfo->iFrameOffset = 0;
    iFrmHdr.fileSign = IFRAME_FILE_SIGN;
    iFrmHdr.version = IFRAME_FILE_VERSION;
    iFrmHdr.nextMetaDataIdx = fileInfo->nextIFrameIdx;

    // write file next i frame position
    if (Utils_pWrite(fileInfo->iFrmFileFd, &iFrmHdr, IFRAME_FILE_HDR_SIZE, 0, pErrorCode) != IFRAME_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write i-frame header in file: [err=%s]", strerror(*pErrorCode));
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was generates event i frame and write it into event file. It was generate all event stream
 * @param   eventInfo
 * @param   eventFileInfo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL updateEventFile(EVENT_INDEX_HEADER_t *eventInfo, EVNT_FILE_INFO_t *eventFileInfo, UINT32PTR pErrorCode)
{
    if (-1 == lseek(eventFileInfo->fileFd, ((eventInfo->eventIdx * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE), SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek event file: [err=%s]", STR_ERR);
        return FAIL;
    }

    if (Utils_Write(eventFileInfo->fileFd, &eventInfo->evntInfo, EVNT_FIELD_SIZE, pErrorCode) != EVNT_FIELD_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write event file: [err=%s]", strerror(*pErrorCode));
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was generates event i frame and write it into event file. It was generate all event stream.
 * @param   eventFileInfo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL updateEventFileHeader(EVNT_FILE_INFO_t *eventFileInfo, UINT32PTR pErrorCode)
{
    METADATA_FILE_HDR_t evntFileHdr;

    evntFileHdr.fileSign = EVENT_FILE_SIGN;
    evntFileHdr.version = EVENT_FILE_VERSION;
    evntFileHdr.nextMetaDataIdx = eventFileInfo->evntIdxNo;

    if (Utils_pWrite(eventFileInfo->fileFd, &evntFileHdr, EVENT_FILE_HDR_SIZE, 0, pErrorCode) != EVENT_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write event file header: [err=%s]", strerror(*pErrorCode));
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was update time index file. It will put information of current minute,
 *          current stream file and current stream position of current file
 * @param   channelNo
 * @param   curTime
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL updateTimeIdxFile(UINT8 channelNo, struct tm *curTime, UINT32PTR pErrorCode)
{
    TIME_IDX_FILE_INFO_t    *timeIdxFileInfo = &sessionInfo[channelNo].timeIdxFileInfo;
    TIMEIDX_FIELD_INFO_t 	timeIdxField;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if (-1 == lseek(timeIdxFileInfo->fileFd, ((timeIdxFileInfo->timeIdxNo * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE), SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek time-index file: [camera=%d], [err=%s]", channelNo, STR_ERR);
        return FAIL;
    }

    timeIdxField.min = curTime->tm_min;
    timeIdxField.overlapFlg = sessionInfo[channelNo].overlapFlg;
    timeIdxField.diskId = sessionInfo[channelNo].streamFileInfo.diskId;
    timeIdxField.streamFileId = sessionInfo[channelNo].streamFileInfo.streamFileIdx;
    timeIdxField.fshPos = sessionInfo[channelNo].streamFileInfo.prevFshPos;

    if (Utils_Write(timeIdxFileInfo->fileFd, &timeIdxField, TIME_INDEX_FIELD_SIZE, pErrorCode) != TIME_INDEX_FIELD_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write time-index file: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was update month and day index file. It will put information of current minute
 * @param   channelNo
 * @param   eventType
 * @param   curTime
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL updateYearMapFile(UINT8 channelNo, UINT8 eventType, struct tm *curTime, UINT32PTR pErrorCode)
{
    YEAR_MAP_FILE_INFO_t    *yearMapFileinfo = &sessionInfo[channelNo].yearMapFileInfo;
    YEAR_MAP_t				yearRecMap;
    UINT8					dayIndx, monthIndx, curByte, curBit, evntType;
    UINT32					totalMin;
    HDD_CONFIG_t	        hddConfig;

    if (-1 == lseek(yearMapFileinfo->mapFileFd, 0, SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek year map file: [camera=%d], [err=%s]", channelNo, STR_ERR);
        return FAIL;
    }

    if (Utils_Read(yearMapFileinfo->mapFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode) != sizeof(YEAR_MAP_t))
    {
        EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
        return FAIL;
    }

    if ((dayRecordInfo[curTime->tm_wday].yearId != curTime->tm_year)
            || (dayRecordInfo[curTime->tm_wday].monthId != curTime->tm_mon) || (dayRecordInfo[curTime->tm_wday].dayId != curTime->tm_mday))
    {
        RECORD_DATA_DAY_t tempData;

        memset(&tempData, 0, sizeof(RECORD_DATA_DAY_t));
        dayRecordInfo[curTime->tm_wday] = tempData;
        dayRecordInfo[curTime->tm_wday].dayId = curTime->tm_mday;
        dayRecordInfo[curTime->tm_wday].monthId = curTime->tm_mon;
        dayRecordInfo[curTime->tm_wday].yearId = curTime->tm_year;
    }

    if(recordInfo[curTime->tm_mon].yearId != curTime->tm_year)
    {
        RECORD_DATA_t tempData;

        /* As year is rolled over, overwitre the oldest month's data */
        memset(&tempData, 0, sizeof(RECORD_DATA_t));
        recordInfo[curTime->tm_mon] = tempData;

        /* Make data for old month default and insert current year identity */
        recordInfo[curTime->tm_mon].yearId = curTime->tm_year;
    }

    // find minute position for day
    totalMin = (curTime->tm_hour * 60) + curTime->tm_min;
    curByte = (totalMin / MAX_BIT_IN_BYTE);
    curBit = (totalMin % MAX_BIT_IN_BYTE);
    dayIndx = (UINT8)(curTime->tm_mday - 1);
    monthIndx = (UINT8)curTime->tm_mon;
    ReadHddConfig(&hddConfig);

    for(evntType = REC_EVENT_MANUAL; evntType < REC_EVENT_MAX; evntType++)
    {
        if (GET_BIT(eventType, evntType) == 0)
        {
            continue;
        }

        //set index of current day if not set
        yearRecMap.monthMap[monthIndx].dayMap[dayIndx].eventMap[evntType].recordMap[curByte] |= (UINT8)(1 << curBit);

        //if overlap flag is not set for day then only update it.
        if(yearRecMap.monthMap[monthIndx].dayMap[dayIndx].eventMap[evntType].overlapFlag == FALSE)
        {
            yearRecMap.monthMap[monthIndx].dayMap[dayIndx].eventMap[evntType].overlapFlag = sessionInfo[channelNo].overlapFlg;
        }

        dayRecordInfo[curTime->tm_wday].recordsForday[hddConfig.recordDisk][channelNo].eventMap[evntType].recordMap[curByte] |= (UINT8)(1 << curBit);
        if(dayRecordInfo[curTime->tm_wday].recordsForday[hddConfig.recordDisk][channelNo].eventMap[evntType].overlapFlag == FALSE)
        {
            dayRecordInfo[curTime->tm_wday].recordsForday[hddConfig.recordDisk][channelNo].eventMap[evntType].overlapFlag = sessionInfo[channelNo].overlapFlg;
        }

        recordInfo[curTime->tm_mon].recData[hddConfig.recordDisk][channelNo][evntType] |= (UINT32)(1 << (dayIndx));
    }

    if( -1 == lseek(yearMapFileinfo->mapFileFd, 0, SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek year map file: [camera=%d], [err=%s]", channelNo, STR_ERR);
        return FAIL;
    }

    if (Utils_Write(yearMapFileinfo->mapFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode) != sizeof(YEAR_MAP_t))
    {
        EPRINT(DISK_MANAGER, "fail to write year map file: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was update time index file. It will put information of current minute,
 *          current stream file and current stream position of current file
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL updateTimeIdxFileHeader(UINT8 channelNo, UINT32PTR pErrorCode)
{
    TIME_IDX_FILE_INFO_t    *timeIdxFileInfo = &sessionInfo[channelNo].timeIdxFileInfo;
    METADATA_FILE_HDR_t     timeIdxFileHdr;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    // stored stream file sign
    timeIdxFileHdr.fileSign = TIMEIDX_FILE_SIGN;

    // version of file
    timeIdxFileHdr.version = TIMEIDX_FILE_VERSION;

    //next event index number
    timeIdxFileHdr.nextMetaDataIdx = timeIdxFileInfo->timeIdxNo;

    if(Utils_pWrite(timeIdxFileInfo->fileFd, &timeIdxFileHdr, TIMEIDX_FILE_HDR_SIZE, 0, pErrorCode) != TIMEIDX_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write time-index file header: [camera=%d], [err=%s]", channelNo, strerror(*pErrorCode));
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates a stream file and write stream file header into file with RUN flag
 *          set to 1. It means, this file is currently working file.
 * @param   currTime
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL createStreamFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode)
{
    UINT8               overlapNo = 0;
    UINT32              streamFileNo;
    CHAR                mntPoint[MAX_FILE_NAME_SIZE] = "\0";
    CHAR                filePath[MAX_FILE_NAME_SIZE];
    CHAR                fileName[MAX_FILE_NAME_SIZE];
    CHAR                evntFileName[MAX_FILE_NAME_SIZE];
    INT32               evntFileFd;
    INT32               fileFd;
    UINT32              lastEventPos;
    UINT32              curEventPos;
    time_t              curTimeSec;
    time_t              timeInSec;
    METADATA_FILE_HDR_t evntFileInfo;
    EVNT_INFO_t         eventFieldInfo;
    STRM_FILE_HDR_t     strmFileHdr;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if (GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
        return FAIL;
    }

    snprintf(filePath, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mntPoint,
             GET_CAMERA_NO(channelNo), currTime->tm_mday, GetMonthName(currTime->tm_mon), currTime->tm_year, currTime->tm_hour);
    snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s", filePath, EVENT_FILE_NAME);

    if ((aviRecGenConfig.recordFormatType != REC_AVI_FORMAT) && ((sessionInfo[channelNo].overlapFlg == 0) && (access(evntFileName, F_OK) == STATUS_OK)))
    {
        // event file was present. previously event file present means here time was some how change and overlapping condition occurred.
        evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
        if (evntFileFd == INVALID_FILE_FD)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", channelNo, evntFileName, STR_ERR);
            return FAIL;
        }

        if(Utils_Read(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, pErrorCode) != EVENT_FILE_HDR_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to write event file: [camera=%d], [path=%s], [err=%s]", channelNo, evntFileName, strerror(*pErrorCode));
            close(evntFileFd);
            return FAIL;
        }

        // Get next event entry position in event file in eventIdxNo and convert current local time into time in second
        ConvertLocalTimeInSec(currTime, &curTimeSec);

        // Sets next event index number
        sessionInfo[channelNo].evntFileInfo.evntIdxNo = evntFileInfo.nextMetaDataIdx;
        lastEventPos = ((evntFileInfo.nextMetaDataIdx * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE);

        // Set current event entry (curEventPos) to first event entry in file
        curEventPos = EVENT_FILE_HDR_SIZE;

        /* PARASOFT : No need to validate tainted data */
        while((curEventPos < lastEventPos) && (evntFileInfo.nextMetaDataIdx > 0))
        {
            if(-1 == lseek(evntFileFd, curEventPos, SEEK_SET))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek event file: [camera=%d], [path=%s], [err=%s]", channelNo, evntFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }

            if (Utils_Read(evntFileFd, &eventFieldInfo, EVNT_FIELD_SIZE, pErrorCode) != EVNT_FIELD_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", channelNo, evntFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }

            // Get end of time event in endEventTime and also get overlap count in overlapCnt
            // check current recording time falls into this event
            if((time_t)eventFieldInfo.endTime > curTimeSec)
            {
                // this recording was overlapping. set overlapping flag in current disk manager session
                overlapNo++;
            }

            // Update current event position in curEventPos
            curEventPos += EVNT_FIELD_SIZE;
        }

        // set overlapping condition
        sessionInfo[channelNo].overlapFlg = overlapNo;
        close(evntFileFd);
    }
    else
    {
        if(sessionInfo[channelNo].overlapFlg == 0)
        {
            sessionInfo[channelNo].overlapFlg = overlapNo;
        }
    }

    if (getAllStreamFileCount(filePath, &streamFileNo, NULL) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get all stream file count: [camera=%d], [path=%s]", channelNo, filePath);
        return FAIL;
    }

    /* Ideally stream file count should not be greater than 255. We support stream file id upto ".stm255" */
    if (streamFileNo >= STREAM_FILE_STM_ID_MAX)
    {
        EPRINT(DISK_MANAGER, "invld stream file count: [camera=%d], [path=%s], [fileCnt=%d]", channelNo, filePath, streamFileNo);
        return FAIL;
    }

    /* Increment stream file number by one */
    streamFileNo++;

    // Make a file with proper file name with time stamp and give extension of file name as per overlap flag
    snprintf(fileName, MAX_FILE_NAME_SIZE, "%s"REC_FOLDER_TIME_FORMAT"~"REC_FOLDER_TIME_FORMAT"%s%d",
             filePath, currTime->tm_hour, currTime->tm_min, currTime->tm_sec,
             0, 0, 0, (sessionInfo[channelNo].overlapFlg == 0) ? STREAM_EXTN : STREAM_EXTN_OVLP, streamFileNo);

    // Create a file for stream file
    fileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
    if(fileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open stream file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
        return FAIL;
    }

    // create stream file headers and stored stream file sign
    strmFileHdr.fileSign = STREAM_FILE_SIGN;

    // version of file
    strmFileHdr.version = STREAM_FILE_VERSION;

    // set run flag here to 1
    strmFileHdr.runFlg = TRUE;

    // file was newly created so set this flag to 0 to indicates no back up is taken
    strmFileHdr.backupFlg = 0;

    // set next stream position
    strmFileHdr.nextStreamPos = STREAM_FILE_HDR_SIZE;

    //Device type
    strmFileHdr.deviceType = DM_DEVICE_ID;

    // Make reserved byte 0
    strmFileHdr.reserved = 0;

    if (Utils_Write(fileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, pErrorCode) != STREAM_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write stream file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
        close(fileFd);
        unlink(fileName);
        return FAIL;
    }

    // set stream file info element
    sessionInfo[channelNo].streamFileInfo.streamFileIdx = streamFileNo;
    sessionInfo[channelNo].streamFileInfo.fileFd = fileFd;
    sessionInfo[channelNo].streamFileInfo.prevFshPos = STREAM_FILE_HDR_SIZE;
    sessionInfo[channelNo].streamFileInfo.nextFshPos = 0;
    sessionInfo[channelNo].streamFileInfo.curFshPos = STREAM_FILE_HDR_SIZE;
    sessionInfo[channelNo].streamFileInfo.curFshLen = 0;
    sessionInfo[channelNo].streamFileWrCnt = 0;
    snprintf(sessionInfo[channelNo].streamFileInfo.filename, MAX_FILE_NAME_SIZE, "%s", fileName);
    ConvertLocalTimeInSec(currTime, &timeInSec);
    sessionInfo[channelNo].streamFileInfo.fileStartTime = timeInSec;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was create a I frame file and write a I frame file header into file
 * @param   currTime
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL createIFrameFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode)
{
    CHAR                mntPoint[MAX_FILE_NAME_SIZE]	= "\0";
    CHAR                filename[MAX_FILE_NAME_SIZE];
    INT32               fileFd;
    METADATA_FILE_HDR_t iFrmInfo;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if(GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
        return FAIL;
    }

    // make file name
    snprintf(filename, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
             currTime->tm_mday, GetMonthName(currTime->tm_mon), currTime->tm_year, currTime->tm_hour, I_FRAME_FILE_NAME);

    // Create a file for i frame file
    fileFd = open(filename, CREATE_RDWR_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open i-frame file: [camera=%d], [path=%s], [err=%s]", channelNo, filename, STR_ERR);
        return FAIL;
    }

    // stored i frame file sign
    iFrmInfo.fileSign = IFRAME_FILE_SIGN;

    // version of file
    iFrmInfo.version = IFRAME_FILE_VERSION;

    //next I frame  index number
    iFrmInfo.nextMetaDataIdx = 0;

    // write i frame file header
    if(Utils_Write(fileFd, &iFrmInfo, IFRAME_FILE_HDR_SIZE, pErrorCode) != IFRAME_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write i-frame file: [camera=%d], [path=%s], [err=%s]", channelNo, filename, strerror(*pErrorCode));
        close(fileFd);
        unlink(filename);
        return FAIL;
    }

    /* Initialize I frame index number. Assume that no I frame was received on starting so reference to
     * this P frame was not found. In case of first frame was I frame then one increment to this field
     * make it 0 because of fshInfo[channelNo].iFrameIdx was only 32 bit value. */
    sessionInfo[channelNo].iFrameFileInfo.fileFd = fileFd;
    sessionInfo[channelNo].iFrameFileInfo.iFrmIdxNo = iFrmInfo.nextMetaDataIdx;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was create a event file and write event file header into file
 * @param   currTime
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return
 */
static BOOL createEventFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode)
{
    CHAR                mntPoint[MAX_FILE_NAME_SIZE] = "\0";
    CHAR                filename[MAX_FILE_NAME_SIZE];
    INT32               fileFd;
    METADATA_FILE_HDR_t evntFileInfo;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if(GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
        return FAIL;
    }

    // make file name
    snprintf(filename, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
             currTime->tm_mday, GetMonthName(currTime->tm_mon), currTime->tm_year, currTime->tm_hour, EVENT_FILE_NAME);

    // Create a file for eventfile
    fileFd = open(filename, CREATE_RDWR_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", channelNo, filename, STR_ERR);
        return FAIL;
    }

    // stored stream file sign
    evntFileInfo.fileSign = EVENT_FILE_SIGN;

    // version of file
    evntFileInfo.version = EVENT_FILE_VERSION;

    //next event index number
    evntFileInfo.nextMetaDataIdx = 0;

    if(Utils_Write(fileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, pErrorCode) != EVENT_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write event file: [camera=%d], [path=%s], [err=%s]", channelNo, filename, strerror(*pErrorCode));
        close(fileFd);
        unlink(filename);
        return FAIL;
    }

    //Set event index number for this event file
    sessionInfo[channelNo].evntFileInfo.fileFd = fileFd;
    sessionInfo[channelNo].evntFileInfo.evntIdxNo = evntFileInfo.nextMetaDataIdx;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was create a time index file and write time index file header into file
 * @param   currTime
 * @param   channelNo
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL createTimeIdxFile(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode)
{
    CHAR                mntPoint[MAX_FILE_NAME_SIZE] = "\0";
    CHAR                filename[MAX_FILE_NAME_SIZE];
    INT32               fileFd;
    METADATA_FILE_HDR_t timeIdxFileInfo;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if(GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
        return FAIL;
    }

    snprintf(filename, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT"%s", mntPoint, GET_CAMERA_NO(channelNo),
             currTime->tm_mday, GetMonthName(currTime->tm_mon), currTime->tm_year, currTime->tm_hour, TIMEIDX_FILE_NAME);

    // Create a file for time index file
    fileFd = open(filename, CREATE_RDWR_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open time-index file: [camera=%d], [path=%s], [err=%s]", channelNo, filename, STR_ERR);
        return FAIL;
    }

    // stored stream file sign
    timeIdxFileInfo.fileSign = TIMEIDX_FILE_SIGN;

    // version of file
    timeIdxFileInfo.version = TIMEIDX_FILE_VERSION;

    //next event index number
    timeIdxFileInfo.nextMetaDataIdx = 0;

    if(Utils_Write(fileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, pErrorCode) != TIMEIDX_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write time-index file: [camera=%d], [path=%s], [err=%s]", channelNo, filename, strerror(*pErrorCode));
        close(fileFd);
        unlink(filename);
        return FAIL;
    }

    //Set event index number for this event file
    sessionInfo[channelNo].timeIdxFileInfo.fileFd = fileFd;
    sessionInfo[channelNo].timeIdxFileInfo.timeIdxNo = timeIdxFileInfo.nextMetaDataIdx;
    sessionInfo[channelNo].timeIdxFileInfo.lastTimeIdxMin = INVALID_MIN_CNT;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will search the stream file in given date and hour folder and returns
 *          number of file available in that folder
 * @param   path
 * @param   streamFileCount
 * @param   aviFileCount
 * @return  SUCCESS/FAIL
 */
static BOOL getAllStreamFileCount(CHARPTR path, UINT32PTR streamFileCount, UINT32PTR aviFileCount)
{
    UINT32          strmFileCnt = 0;
    UINT32          aviFileCnt  = 0;
    CHARPTR         extension;
    DIR             *dir;
    struct dirent   *entry;

    if (streamFileCount != NULL)
    {
        *streamFileCount = 0;
    }

    if (aviFileCount != NULL)
    {
        *aviFileCount = 0;
    }

    dir = opendir(path);
    if (dir == NULL)
    {
        return FAIL;
    }

    // read every file of current directory
    while((entry = readdir(dir)) != NULL)
    {
        // check this file was stream file
        if(((extension = strstr(entry->d_name, STREAM_EXTN)) != NULL) || ((extension = strstr(entry->d_name, STREAM_EXTN_OVLP)) != NULL))
        {
            // update stream file count
            strmFileCnt++;
        }
        else if(((extension = strstr(entry->d_name, AVI_EXTENTION)) != NULL) || ((extension = strstr(entry->d_name, AVI_INCOMP_EXTENTION)) != NULL))
        {
            aviFileCnt++;
        }
    }

    if (closedir(dir) != STATUS_OK)
    {
        return FAIL;
    }

    if (streamFileCount != NULL)
    {
        *streamFileCount = strmFileCnt;
    }
    if (aviFileCount != NULL)
    {
        *aviFileCount = aviFileCnt;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stores information that from where recovery should be performed. Recovery
 *          information stored whenever new session was created or date or hour was changed. This
 *          function creates a single file for each channel for each hard disk and stores information
 *          of last modified date and last modified hour
 * @param   currTime
 * @param   channelNo
 * @param   pErrorCode - Error code, set when any I/O operation on disk fails
 * @return  SUCCESS/FAIL
 */
static BOOL storeFileRecoveryInfo(struct tm *currTime, UINT8 channelNo, UINT32PTR pErrorCode)
{
    CHAR            mntPoint[MAX_FILE_NAME_SIZE] = "\0";
    CHAR            recoveryFldr[MAX_FILE_NAME_SIZE];
    CHAR            channelFile[MAX_FILE_NAME_SIZE];
    INT32           fileFd;
    RECOVERY_INFO_t recveryInfo;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    if(GetRecordingPath(channelNo, mntPoint) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get mount path: [camera=%d]", channelNo);
        return FAIL;
    }

    // One separate partition is for system information which is not viewable by user either on SMB.
    // In that partition create a RECOVERY INFO folder if not exists
    // Check that RECOVERY INFO folder was present in separate partition of hard disk
    snprintf(recoveryFldr, MAX_FILE_NAME_SIZE, RECOVERY_FOLDER, mntPoint);
    if (access(recoveryFldr, F_OK) != STATUS_OK)
    {
        // create recovery informaion folder
        if (Utils_Mkdir(recoveryFldr, FOLDER_PERMISSION, pErrorCode) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "recovery folder not created: [camera=%d], [path=%s], [err=%s]", channelNo, recoveryFldr, strerror(*pErrorCode));
            return FAIL;
        }

        DPRINT(DISK_MANAGER, "recovery folder created: [camera=%d], [path=%s]", channelNo, recoveryFldr);
    }

    // check that channel information present for this channel
    snprintf(channelFile, MAX_FILE_NAME_SIZE, RECOVERY_CHANNEL_FILE, recoveryFldr, GET_CAMERA_NO(channelNo));
    if (access(channelFile, F_OK) != STATUS_OK)
    {
        // create recovery file for this channel
        fileFd = open(channelFile, CREATE_RDWR_MODE, FILE_PERMISSION);
        if (fileFd == INVALID_FILE_FD)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to create recovery file: [camera=%d], [path=%s], [err=%s]", channelNo, channelFile, STR_ERR);
            return FAIL;
        }

        DPRINT(DISK_MANAGER, "recovery file created: [camera=%d], [path=%s]", channelNo, channelFile);
    }
    else
    {
        // open recovery file for this channel
        fileFd = open(channelFile, WRITE_ONLY_MODE, FILE_PERMISSION);
        if (fileFd == INVALID_FILE_FD)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to open recovery file: [camera=%d], [path=%s], [err=%s]", channelNo, channelFile, STR_ERR);
            return FAIL;
        }
    }

    // update current date and hour in recovery file
    // this fields are used at recovery time.
    recveryInfo.date = currTime->tm_mday;
    recveryInfo.mon = currTime->tm_mon;
    recveryInfo.year = currTime->tm_year;
    recveryInfo.hour = currTime->tm_hour;
    recveryInfo.reserved = 0;

    // write recovery information into file
    if (Utils_Write(fileFd, &recveryInfo, sizeof(RECOVERY_INFO_t), pErrorCode ) != sizeof(RECOVERY_INFO_t))
    {
        EPRINT(DISK_MANAGER, "faile to write recovery file: [camera=%d], [path=%s], [err=%s]", channelNo, channelFile, strerror(*pErrorCode));
        close(fileFd);
        return FAIL;
    }

    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was check that event information was properly organized or not. If file was
 *          not proper then removes some un useful information from file
 * @param   path
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL recoverEventFile(CHARPTR path, UINT32PTR pErrorCode)
{
    UINT8               bufIdx;
    CHAR                evntBuf[2];
    CHAR                evntFileName[MAX_FILE_NAME_SIZE];
    CHAR                strmFileName[MAX_FILE_NAME_SIZE];
    INT32               evntFileFd = INVALID_FILE_FD;
    INT32               strmFileFd = INVALID_FILE_FD;
    UINT32              fileOffset;
    UINT32              eventNo;
    UINT32              strmFileNo;
    UINT32              strmFileSize;
    struct stat         stateInfo;
    METADATA_FILE_HDR_t evntFileInfo;
    EVNT_INFO_t         recoverEvent;
    STRM_FILE_HDR_t     strmFileHdr;
    STRM_FILE_END_t     strmEndInfo;
    FSH_INFO_t          fshInfo;
    UINT32              curSysTick = GetSysTick();

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    // make file name for event file
    snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s", path, EVENT_FILE_NAME);

    //Open event file
    evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(evntFileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
        return FAIL;
    }

    // Get size of event file into fileSize
    if (stat(evntFileName, &stateInfo) != STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to stat event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
        close(evntFileFd);
        return FAIL;
    }

    // Read the file header of event file
    if(Utils_Read(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, pErrorCode) != EVENT_FILE_HDR_SIZE)
    {
        close(evntFileFd);
        EPRINT(DISK_MANAGER, "fail to read event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
        return FAIL;
    }

    // Each event information takes 19 bytes. Calculate offset position in to file for nextEventIdx
    fileOffset = ((evntFileInfo.nextMetaDataIdx * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE);
    if((INT32)fileOffset < stateInfo.st_size)
    {
        /* When last event index was written into file but updation of next event position was not possible. So correct this condition here */
        evntFileInfo.nextMetaDataIdx += ((stateInfo.st_size - fileOffset) / EVNT_FIELD_SIZE);

        // Update nextEventIdx in current event file @ file header @ 5th byte
        if(Utils_pWrite(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, 0, pErrorCode) != EVENT_FILE_HDR_SIZE)
        {
            close(evntFileFd);
            EPRINT(DISK_MANAGER, "fail to write event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
            return FAIL;
        }

        // After nextEventIdx, if nay data was present that was garbage or incomplete data so remove it from file
        fileOffset = ((evntFileInfo.nextMetaDataIdx * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE);
        if ((stateInfo.st_size - fileOffset) > 0)
        {
            // go to next event index position in file stream
            if(-1 == lseek(evntFileFd, fileOffset, SEEK_SET))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }

            //remove no. of bytes from nextIFrameIdx in current event file
            if(-1 == ftruncate(evntFileFd, lseek(evntFileFd, 0, SEEK_CUR)))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to truncate event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }
        }
    }

    //go to end of file in event file
    if(-1 == lseek(evntFileFd, 0, SEEK_END))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
        close(evntFileFd);
        return FAIL;
    }

    bufIdx = 0;
    evntBuf[bufIdx++] = EVNT_FILE_EOF >> 8;
    evntBuf[bufIdx++] = (UINT8) EVNT_FILE_EOF;

    // Write end of file indicator of eventfile
    if (Utils_Write(evntFileFd, evntBuf, bufIdx, pErrorCode) != bufIdx)
    {
        close(evntFileFd);
        EPRINT(DISK_MANAGER, "fail to write event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
        return FAIL;
    }

    // set file start position. Move current file position to starting of file
    if (-1 == lseek(evntFileFd, 0, SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
        close(evntFileFd);
        return FAIL;
    }

    // First event start from 9th bytes in file. Set currEventPos to 9th byte in event file
    for(eventNo = 0; eventNo < evntFileInfo.nextMetaDataIdx; eventNo++)
    {
        if(ElapsedTick(curSysTick) >= CONVERT_SEC_TO_TIMER_COUNT(RECOVERY_TIME_OUT))
        {
            close(evntFileFd);
            EPRINT(DISK_MANAGER, "recovery timeout occurred");
            return FAIL;
        }

        fileOffset = ((eventNo * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE);
        if(-1 == lseek(evntFileFd, fileOffset, SEEK_SET))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to seek event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
            close(evntFileFd);
            return FAIL;
        }

        // Read its first event index information @ 6th byte of 19 bytes from file
        if(Utils_Read(evntFileFd, &recoverEvent, EVNT_FIELD_SIZE, pErrorCode) != EVNT_FIELD_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
            close(evntFileFd);
            return FAIL;
        }

        // Check that start of event present but end of event not present then we need to add stop event at last frame of stream file.
        if ((recoverEvent.startTime == 0) || (recoverEvent.endTime != 0))
        {
            continue;
        }

        DPRINT(DISK_MANAGER, "start and stop time are not same: [eventNo=%d]", eventNo);

        // Get total number of stream file in current folder
        if(getAllStreamFileCount(path, &strmFileNo, NULL) == FAIL)
        {
            close(evntFileFd);
            EPRINT(DISK_MANAGER, "fail to get all stream file count: [path=%s]", path);
            return FAIL;
        }

        DPRINT(DISK_MANAGER, "stream file info: [strmFileCnt=%d], [startFileId=%d]", strmFileNo, recoverEvent.startFileId);

        if(strmFileNo >= recoverEvent.startFileId)
        {
            //Find last stream file and get its proper name in streamFiles
            if(getStreamFileName(path, strmFileNo, strmFileName, &strmFileSize) == FAIL)
            {
                close(evntFileFd);
                EPRINT(DISK_MANAGER, "fail to get stream file name: [path=%s], [strmFileId=%d]", path, strmFileNo);
                return FAIL;
            }

            // Open steamFile
            strmFileFd = open(strmFileName, READ_WRITE_MODE, FILE_PERMISSION);
            if(strmFileFd == INVALID_FILE_FD)
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to open stream file: [path=%s], [err=%s]", strmFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }

            // Read stream file header
            DPRINT(DISK_MANAGER, "stream file verification: [path=%s]", strmFileName);
            if(Utils_Read(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, pErrorCode) != STREAM_FILE_HDR_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", strmFileName, strerror(*pErrorCode));
                close(evntFileFd);
                close(strmFileFd);
                return FAIL;
            }

            // Get next stream position. Go to nextStreamPos in file
            if(-1 == lseek(strmFileFd, strmFileHdr.nextStreamPos, SEEK_SET))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [position=%d], [err=%s]", strmFileName, strmFileHdr.nextStreamPos, STR_ERR);
                close(evntFileFd);
                close(strmFileFd);
                return FAIL;
            }

            // Read 6 bytes from current file position
            if(Utils_Read(strmFileFd, &strmEndInfo, STREAM_FILE_END_SIZE, pErrorCode) != STREAM_FILE_END_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", strmFileName, strerror(*pErrorCode));
                close(evntFileFd);
                close(strmFileFd);
                return FAIL;
            }

            //Check end of stream file indicator present or not.
            if (strmEndInfo.fileEndIndicator != STRM_EOF_INDICATOR)
            {
                EPRINT(DISK_MANAGER, "invld stream file end indicator: [path=%s], [fileEndIndicator=0x%x]", strmFileName, strmEndInfo.fileEndIndicator);
                close(evntFileFd);
                close(strmFileFd);
                return FAIL;
            }

            // In EOF, EOF + previous FSH length is present. so we get previous FSH length from last 4 bytes. Go to prevFshLen in file
            if (-1 == lseek(strmFileFd, strmEndInfo.prevFshPos, SEEK_SET))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [position=%d], [err=%s]", strmFileName, strmEndInfo.prevFshPos, STR_ERR);
                close(evntFileFd);
                close(strmFileFd);
                return FAIL;
            }

            if ((strmEndInfo.prevFshPos + MAX_FSH_SIZE) < strmFileSize)
            {
                // read FSH from current position
                if(Utils_Read(strmFileFd, &fshInfo, MAX_FSH_SIZE, pErrorCode) != MAX_FSH_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", strmFileName, strerror(*pErrorCode));
                    close(evntFileFd);
                    close(strmFileFd);
                    return FAIL;
                }

                // check that valid FSH is present
                if(fshInfo.startCode == FSH_START_CODE)
                {
                    // generate stop condition for that event because stop was missing
                    recoverEvent.endTime = fshInfo.localTime.totalSec;
                    recoverEvent.stopFileId = strmFileNo;
                    recoverEvent.stopStreamPos = strmEndInfo.prevFshPos;

                    // update this event entry in event file
                    if (-1 == lseek(evntFileFd, ((eventNo * EVNT_FIELD_SIZE) + EVENT_FILE_HDR_SIZE), SEEK_SET))
                    {
                        SET_ERROR_NUMBER(pErrorCode, errno);
                        EPRINT(DISK_MANAGER, "fail to seek event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
                        close(evntFileFd);
                        close(strmFileFd);
                        return FAIL;
                    }

                    if(Utils_Write(evntFileFd, &recoverEvent, EVNT_FIELD_SIZE, pErrorCode) != EVNT_FIELD_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to write event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
                        close(evntFileFd);
                        close(strmFileFd);
                        return FAIL;
                    }
                }
            }

            // Close the stream file
            CloseFileFd(&strmFileFd);
        }
        else
        {
            // corresponding stream file for that event was not present so we need to remove all event entry from current event
            if(-1 == ftruncate(evntFileFd, lseek(evntFileFd, fileOffset, SEEK_SET)))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to truncate event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }

            evntFileInfo.nextMetaDataIdx = eventNo;
            if(Utils_pWrite(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE,  0, pErrorCode) != EVENT_FILE_HDR_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to write event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
                close(evntFileFd);
                return FAIL;
            }

            //go to end of file in event file
            if(-1 == lseek(evntFileFd, 0, SEEK_END))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek event file: [path=%s], [err=%s]", evntFileName, STR_ERR);
                close(evntFileFd);
                return FAIL;
            }

            bufIdx = 0;
            evntBuf[bufIdx++] = EVNT_FILE_EOF >> 8;
            evntBuf[bufIdx++] = (UINT8) EVNT_FILE_EOF;

            // Write end of file indicator of event file
            if(Utils_Write(evntFileFd, evntBuf, bufIdx, pErrorCode) != bufIdx)
            {
                EPRINT(DISK_MANAGER, "fail to write event file: [path=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
                close(evntFileFd);
                return FAIL;
            }
        }
    }

    // closed event file
    CloseFileFd(&evntFileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks that index of minutes are updated according to recovered event file.
 *          If file was not proper then removes some un useful information from file
 * @param   mntPoint
 * @param   path
 * @param   channelNo
 * @param   recveryInfo
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL recoverMonthDayIndxFiles(CHARPTR mntPoint, CHARPTR path, UINT8 channelNo, RECOVERY_INFO_t recveryInfo, UINT32PTR pErrorCode)
{
    UINT8 					minCnt, evnt;
    UINT8					curBit = 0;
    UINT8					curByte, bitRemCnt = 0;
    CHAR					fileName[MAX_FILE_NAME_SIZE];
    CHAR 					evntFileName[MAX_FILE_NAME_SIZE];
    INT32 					evntFileFd = INVALID_FILE_FD;
    INT32 					yrFileFd = INVALID_FILE_FD;
    INT32					readCnt;
    UINT32 				    totalBits = 0;
    UINT32 					curEventIdx = 0;
    UINT64					minRecData[REC_EVENT_MAX];
    BOOL					ovlpFlag[REC_EVENT_MAX] = {0,0,0,0};
    time_t                  timeInSec;
    METADATA_FILE_HDR_t 	evntFileInfo;
    EVNT_INFO_t 			eventInfo;
    YEAR_MAP_t				yearRecMap;
    CON_UINT64_TO_UINT8_u	converter;
    struct stat             fileInfo;
    struct tm 				startTm = { 0 };
    struct tm 				endTm = { 0 };

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);
    memset(minRecData, 0, sizeof(minRecData));
    snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s", path, EVENT_FILE_NAME);
    if (access(evntFileName, F_OK) != STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        return FAIL;
    }

    snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR, mntPoint, GET_CAMERA_NO(channelNo), recveryInfo.year, YEAR_MAP_FILE_EXTN);
    if(access(fileName, F_OK) != STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "metadata file not present: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
        return FAIL;
    }

    yrFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(yrFileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open metadata file: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
        return FAIL;
    }

    readCnt = Utils_Read(yrFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode);
    if(readCnt != (INT32)sizeof(YEAR_MAP_t))
    {
        EPRINT(DISK_MANAGER, "fail to read year map: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));

        /* Read file info for size */
        if (fstat(yrFileFd, &fileInfo) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to stat year map: [path=%s], [err=%s]", fileName, STR_ERR);
            close(yrFileFd);
        }
        else
        {
            /* Close the file and remove if it is zero */
            close(yrFileFd);
            if (fileInfo.st_size == 0)
            {
                unlink(fileName);
                WPRINT(DISK_MANAGER, "blank year map file removed: [camera=%d], [path=%s]", channelNo, fileName);
            }
        }
        return FAIL;
    }

    if(-1 == lseek(yrFileFd, 0, SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        close(yrFileFd);
        EPRINT(DISK_MANAGER, "year map file position not set: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
        return FAIL;
    }

    // Open event file
    evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(evntFileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        close(yrFileFd);
        EPRINT(DISK_MANAGER, "fail to open event file: [file=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
        return FAIL;
    }

    // Read event file header. Get next event index number
    readCnt = Utils_Read(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE, pErrorCode);
    if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
    {
        close(yrFileFd);
        close(evntFileFd);
        EPRINT(DISK_MANAGER, "fail to read event file: [file=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
        return FAIL;
    }

    // loop till end of file not received. here we check curEventPos +2 is greater than or equal to file
    // size because of end of file indicator takes 2 byte value
    curEventIdx = 0;
    while(curEventIdx < evntFileInfo.nextMetaDataIdx)
    {
        // Read event index information from current file position
        if(Utils_Read(evntFileFd, &eventInfo, EVNT_FIELD_SIZE, pErrorCode) != EVNT_FIELD_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read event file: [file=%s], [err=%s]", evntFileName, strerror(*pErrorCode));
            break;
        }

        timeInSec = eventInfo.startTime;
        ConvertLocalTimeInBrokenTm(&timeInSec, &startTm);
        timeInSec = eventInfo.endTime;
        ConvertLocalTimeInBrokenTm(&timeInSec, &endTm);

        for(minCnt = startTm.tm_min; minCnt <= endTm.tm_min; minCnt++)
        {
            for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
            {
                if (FALSE == GET_BIT(eventInfo.eventType, evnt))
                {
                    continue;
                }

                SET_BIT(minRecData[evnt], minCnt);
                if (ovlpFlag[evnt] == FALSE)
                {
                    ovlpFlag[evnt] = eventInfo.overLapFlg;
                }
            }
        }

        //set current event position
        curEventIdx++;
    }

    /* Close the event file */
    CloseFileFd(&evntFileFd);

    if(startTm.tm_hour >= HOUR_IN_ONE_DAY)
    {
        WPRINT(DISK_MANAGER, "invld hour found: [camera=%d], [hour=%d]", channelNo, startTm.tm_hour);
        CloseFileFd(&yrFileFd);
        return SUCCESS;
    }

    // Find current position for given hour in minutwise storage stucture
    totalBits = (startTm.tm_hour * MIN_IN_ONE_HOUR);
    bitRemCnt = (MAX_BIT_IN_BYTE - curBit);

    for(evnt = 0; evnt < REC_EVENT_MAX; evnt++)
    {
        curByte = (totalBits / MAX_BIT_IN_BYTE);
        curBit = (totalBits % MAX_BIT_IN_BYTE);
        bitRemCnt = (MAX_BIT_IN_BYTE - curBit);
        if(bitRemCnt >= MAX_BIT_IN_BYTE)
        {
            // convert 64 bit data into chunks of 8 bit
            converter.rawData = minRecData[evnt];
            memcpy(yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap + curByte, converter.convertedData, sizeof(UINT64));
            curByte += (MIN_IN_ONE_HOUR/MAX_BIT_IN_BYTE);
            curBit += (MAX_BIT_IN_LONG - MIN_IN_ONE_HOUR);
        }
        else
        {
            yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap[curByte] |= ((minRecData[evnt] & 0xff) << bitRemCnt);
            curBit += bitRemCnt;
            if(curBit >= MAX_BIT_IN_BYTE)
            {
                curBit = 0;
                curByte++;
            }

            converter.rawData = (minRecData[evnt] >> bitRemCnt);
            memcpy(yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap + curByte, converter.convertedData, sizeof(UINT64));

            curByte += ((MIN_IN_ONE_HOUR - bitRemCnt) / MAX_BIT_IN_BYTE);
            curBit += ((MIN_IN_ONE_HOUR - bitRemCnt) % MAX_BIT_IN_BYTE);;
        }

        if(yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].overlapFlag == FALSE)
        {
            yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].overlapFlag = ovlpFlag[evnt];
        }
    }

    if (Utils_Write(yrFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode) != sizeof(YEAR_MAP_t))
    {
        EPRINT(DISK_MANAGER, "fail to write metadata file: [err=%s]", strerror(*pErrorCode));
    }

    CloseFileFd(&yrFileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will remove indexes for oldest folder when it would be deleted.
 * @param   mntPoint
 * @param   channelNo
 * @param   pRecveryInfo
 * @param   deleteRetentionFiles
 * @return
 */
BOOL RemoveIndexesForFolder(CHARPTR mntPoint, UINT8 channelNo, const RECOVERY_INFO_t *pRecveryInfo, BOOL deleteRetentionFiles)
{
    UINT32 tErrorCode = INVALID_ERROR_CODE;

    if (FALSE == deleteRetentionFiles) /* Delete old files and overwrite */
    {
        if (removeIndexesforGivenHour(mntPoint, channelNo, *pRecveryInfo, &tErrorCode) != SUCCESS)
        {
            EPRINT(DISK_MANAGER, "fail to remove indexes: [camera=%d], [type=%s], [mntPoint=%s]", channelNo, "OLD_OVERWRITE", mntPoint);
            return FAIL;
        }
    }
    else /* Delete retention files */
    {
        UINT8           hrCnt;
        RECOVERY_INFO_t recveryInfo;

        recveryInfo.date = pRecveryInfo->date;
        recveryInfo.hour = pRecveryInfo->hour;
        recveryInfo.mon = pRecveryInfo->mon;
        recveryInfo.year = pRecveryInfo->year;
        recveryInfo.reserved = 0;

        for (hrCnt = 0; hrCnt < HOUR_IN_ONE_DAY ; hrCnt++)
        {
            recveryInfo.hour = hrCnt;
            if (removeIndexesforGivenHour(mntPoint, channelNo, recveryInfo, &tErrorCode) != SUCCESS)
            {
                EPRINT(DISK_MANAGER, "fail to remove indexes: [camera=%d], [type=%s], [mntPoint=%s]", channelNo, "RETENTION", mntPoint);
                return FAIL;
            }
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes indexes from file for deleted hour folder and update it.
 * @param   mntPoint
 * @param   channelNo
 * @param   recveryInfo
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL removeIndexesforGivenHour(CHARPTR mntPoint, UINT8 channelNo, RECOVERY_INFO_t recveryInfo, UINT32PTR pErrorCode)
{
    UINT8 					evnt, curBit, curByte, bitRemCnt = 0;
    CHAR					fileName[MAX_FILE_NAME_SIZE];
    INT32 					yrFileFd = INVALID_FILE_FD;
    INT32					readCnt;
    UINT32 					totalBits = 0;
    YEAR_MAP_t				yearRecMap;
    struct stat             fileInfo;
    CON_UINT64_TO_UINT8_u	converter;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);
    if (recveryInfo.hour >= HOUR_IN_ONE_DAY)
    {
        WPRINT(DISK_MANAGER, "invld hour found: [camera=%d], [hour=%d]", channelNo, recveryInfo.hour);
        return SUCCESS;
    }

    snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR, mntPoint, GET_CAMERA_NO(channelNo), recveryInfo.year, YEAR_MAP_FILE_EXTN);
    if(access(fileName, F_OK) != STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "year map metadata file not present: [camera=%d], [path=%s]", channelNo, fileName);
        return FAIL;
    }

    yrFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(yrFileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
        return FAIL;
    }

    readCnt = Utils_Read(yrFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode);
    if(readCnt != (INT32)sizeof(YEAR_MAP_t))
    {
        EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));

        /* Read file info for size */
        if (fstat(yrFileFd, &fileInfo) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to stat year map: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
            close(yrFileFd);
        }
        else
        {
            /* Close the file and remove if it is zero */
            close(yrFileFd);
            if (fileInfo.st_size == 0)
            {
                unlink(fileName);
                WPRINT(DISK_MANAGER, "blank year map file removed: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
            }
        }
        return FAIL;
    }

    if(-1 == lseek(yrFileFd, 0, SEEK_SET))
    {
        EPRINT(DISK_MANAGER, "fail to seek year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, STR_ERR);
        SET_ERROR_NUMBER(pErrorCode, errno);
        close(yrFileFd);
        return FAIL;
    }

    totalBits = (recveryInfo.hour * MIN_IN_ONE_HOUR);
    for(evnt = 0; evnt < REC_EVENT_MAX; evnt++)
    {
        curByte = (totalBits / MAX_BIT_IN_BYTE);
        curBit = (totalBits % MAX_BIT_IN_BYTE);
        bitRemCnt = (MAX_BIT_IN_BYTE - curBit);
        if(bitRemCnt >= MAX_BIT_IN_BYTE)
        {
            // convert 64 bit data into chunks of 8 bit
            converter.rawData = 0;
            memcpy(yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap + curByte, converter.convertedData, sizeof(UINT64) -1);
            curByte += (sizeof(UINT64) -1);
            yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap[curByte] &= 0xf0;
            curBit += (MAX_BIT_IN_LONG - MIN_IN_ONE_HOUR);
        }
        else
        {
            yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap[curByte] &= 0x0f;
            curBit += bitRemCnt;
            if(curBit >= MAX_BIT_IN_BYTE)
            {
                curBit = 0;
                curByte++;
            }

            converter.rawData = 0;
            memcpy(yearRecMap.monthMap[recveryInfo.mon].dayMap[recveryInfo.date - 1].eventMap[evnt].recordMap + curByte, converter.convertedData, sizeof(UINT64) - 1);
            curByte += ((MIN_IN_ONE_HOUR - bitRemCnt) / MAX_BIT_IN_BYTE);
            curBit += ((MIN_IN_ONE_HOUR - bitRemCnt) % MAX_BIT_IN_BYTE);
        }
    }

    if (Utils_Write(yrFileFd, &yearRecMap, sizeof(YEAR_MAP_t), pErrorCode) != sizeof(YEAR_MAP_t))
    {
        EPRINT(DISK_MANAGER, "fail to write year map file: [camera=%d], [path=%s], [err=%s]", channelNo, fileName, strerror(*pErrorCode));
    }

    close(yrFileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread would build indexes from previous database
 * @param   mntPoint
 * @return  SUCCESS/FAIL
 */
BOOL BuildRecIndexFromPrevStoredData(CHARPTR mntPoint)
{
    UINT8 					minCnt, camId, hourCnt, evnt;
    UINT8					curBit, curByte, bitRemCnt = 0;
    CHAR					fileName[MAX_FILE_NAME_SIZE];
    CHAR					dirName[MAX_FILE_NAME_SIZE];
    CHAR 					macAddrs[MAX_MAC_ADDRESS_WIDTH] = {0};
    CHAR 					evntFileName[MAX_FILE_NAME_SIZE];
    CHAR					month[10];
    INT32 					evntFileFd = INVALID_FILE_FD;
    INT32 					yrMapFileFd = INVALID_FILE_FD;
    INT32					buildInfoFileFd = INVALID_FILE_FD;
    INT32					readCnt;
    INT32				    date = 0, year = 0;
    UINT32 					totalBits = 0;
    UINT32 					curEventIdx = 0;
    UINT64					minRecData[REC_EVENT_MAX];
    BOOL					ovlpFlag[REC_EVENT_MAX];
    BOOL					eventPresent = FALSE;
    BOOL                    retVal = SUCCESS;
    METADATA_FILE_HDR_t 	evntFileInfo;
    EVNT_INFO_t 			eventInfo;
    YEAR_MAP_t				yearRecMap;
    BUILD_INFO_t			buildInfo;
    CON_UINT64_TO_UINT8_u	converter;
    struct tm 				startTm, endTm;
    time_t                  timeInSec;
    DIR                     *dir = NULL;
    struct dirent           *entry;

    snprintf(fileName, MAX_FILE_NAME_SIZE, "%s%s", mntPoint, BUILD_INDEX_INFO_FILE_NAME);
    memset(&buildInfo, 0, sizeof(BUILD_INFO_t));
    GetMacAddrPrepare(LAN1_PORT, macAddrs);

    MUTEX_LOCK(buildInfoFileLock);
    if(access(fileName, F_OK) != STATUS_OK)
    {
        snprintf(buildInfo.macAddr, MAX_MAC_ADDRESS_WIDTH, "%s", macAddrs);
        buildInfo.buildVersion = BUILD_VERSION;
        buildInfoFileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
        if (buildInfoFileFd < 0)
        {
            EPRINT(DISK_MANAGER, "fail to create build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            MUTEX_UNLOCK(buildInfoFileLock);
            return FAIL;
        }

        if (write(buildInfoFileFd, &buildInfo, sizeof(BUILD_INFO_t)) != sizeof(BUILD_INFO_t))
        {
            EPRINT(DISK_MANAGER, "fail to write build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(buildInfoFileFd);
            if(unlink(fileName) == STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "build info file removed: [path=%s]", fileName);
            }
            else
            {
                EPRINT(DISK_MANAGER, "fail to remove build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            }
            MUTEX_UNLOCK(buildInfoFileLock);
            return FAIL;
        }
    }
    else
    {
        buildInfoFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
        if (buildInfoFileFd < 0)
        {
            EPRINT(DISK_MANAGER, "fail to open build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            MUTEX_UNLOCK(buildInfoFileLock);
            return FAIL;
        }

        if(lseek(buildInfoFileFd, 0, SEEK_SET) < STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to seek build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(buildInfoFileFd);
            MUTEX_UNLOCK(buildInfoFileLock);
            return FAIL;
        }

        if(read(buildInfoFileFd, &buildInfo, sizeof(BUILD_INFO_t)) != sizeof(BUILD_INFO_t))
        {
            EPRINT(DISK_MANAGER, "fail to read build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(buildInfoFileFd);
            if(unlink(fileName) == STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "build info file removed: [path=%s]", fileName);
            }
            else
            {
                EPRINT(DISK_MANAGER, "fail to remove build info file: [path=%s], [err=%s]", fileName, STR_ERR);
            }
            MUTEX_UNLOCK(buildInfoFileLock);
            return FAIL;
        }
    }
    MUTEX_UNLOCK(buildInfoFileLock);

    if (buildInfo.status == SUCCESS)
    {
        close(buildInfoFileFd);
        return SUCCESS;
    }

    DPRINT(DISK_MANAGER, "index building started: [mntPoint=%s]", mntPoint);
    /* PARASOFT : No need to validate tainted data */
    for(camId = buildInfo.lastBuildCameraId; camId < getMaxCameraForCurrentVariant(); camId++)
    {
        snprintf(dirName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL, mntPoint, GET_CAMERA_NO(camId));
        if(access(dirName, F_OK) != STATUS_OK)
        {
            continue;
        }

        /* PARASOFT : No need to validate file path */
        dir = opendir(dirName);
        if(dir == NULL)
        {
            EPRINT(DISK_MANAGER, "fail to open recording dir: [camera=%d], [path=%s], [err=%s]", camId, dirName, STR_ERR);
            continue;
        }

        while((entry = readdir(dir)) != NULL)
        {
            if((strstr(entry->d_name, YEAR_MAP_FILE_EXTN)) ||  (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
            {
                continue;
            }

            /* PARASOFT : No need to validate tainted data */
            sscanf(entry->d_name, "%d_%03s_%d", &date, month, &year);
            snprintf(fileName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_INDEX_FILE_YEAR, mntPoint, GET_CAMERA_NO(camId), year, YEAR_MAP_FILE_EXTN);
            MUTEX_LOCK(sessionInfo[camId].yearMapFileLock);
            /* PARASOFT : No need to validate file path */
            if((aviRecGenConfig.recordFormatType != REC_AVI_FORMAT) && (access(fileName, F_OK) != STATUS_OK))
            {
                /* PARASOFT : No need to validate file path */
                yrMapFileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
                if(yrMapFileFd == INVALID_FILE_FD)
                {
                    MUTEX_UNLOCK(sessionInfo[camId].yearMapFileLock);
                    EPRINT(DISK_MANAGER, "fail to create year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                    retVal = FAIL;
                    break;
                }

                memset(&yearRecMap, 0, sizeof(YEAR_MAP_t));
                if(write(yrMapFileFd, &yearRecMap, sizeof(YEAR_MAP_t)) != sizeof(YEAR_MAP_t))
                {
                    EPRINT(DISK_MANAGER, "fail to write year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                    CloseFileFd(&yrMapFileFd);
                    if(unlink(fileName) == STATUS_OK)
                    {
                        EPRINT(DISK_MANAGER, "year map file removed: [camera=%d], [path=%s]", camId, fileName);
                    }
                    else
                    {
                        EPRINT(DISK_MANAGER, "fail to remove year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                    }
                    MUTEX_UNLOCK(sessionInfo[camId].yearMapFileLock);
                    retVal = FAIL;
                    break;
                }
            }
            else
            {
                /* PARASOFT : No need to validate file path */
                yrMapFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(yrMapFileFd == INVALID_FILE_FD)
                {
                    MUTEX_UNLOCK(sessionInfo[camId].yearMapFileLock);
                    EPRINT(DISK_MANAGER, "fail to open year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                    retVal = FAIL;
                    break;
                }
            }

            if(lseek(yrMapFileFd, 0, SEEK_SET) < STATUS_OK)
            {
                MUTEX_UNLOCK(sessionInfo[camId].yearMapFileLock);
                EPRINT(DISK_MANAGER, "fail to seek year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                retVal = FAIL;
                break;
            }

            if(read(yrMapFileFd, &yearRecMap, sizeof(YEAR_MAP_t)) != (INT32)sizeof(YEAR_MAP_t))
            {
                MUTEX_UNLOCK(sessionInfo[camId].yearMapFileLock);
                EPRINT(DISK_MANAGER, "fail to read year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                retVal = FAIL;
                break;
            }
            MUTEX_UNLOCK(sessionInfo[camId].yearMapFileLock);

            memset(ovlpFlag, 0, sizeof(ovlpFlag));
            for(hourCnt = 0; hourCnt < HOUR_IN_ONE_DAY; hourCnt++)
            {
                snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s/%02d/%s", dirName, entry->d_name, hourCnt, EVENT_FILE_NAME);
                if(access(evntFileName, F_OK) != STATUS_OK)
                {
                    continue;
                }

                // Open event file
                evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(evntFileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", camId, evntFileName, STR_ERR);
                    retVal = FAIL;
                    break;
                }

                // Read event file header and Get next event index number
                readCnt = read(evntFileFd, &evntFileInfo, EVENT_FILE_HDR_SIZE);
                if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read event file header: [camera=%d], [path=%s], [err=%s]", camId, evntFileName, STR_ERR);
                    retVal = FAIL;
                    break;
                }

                memset(minRecData, 0, sizeof(minRecData));
                eventPresent = FALSE;
                curEventIdx = 0;

                // loop till end of file not received. here we check curEventPos +2 is greater than or equal to file size
                // because of end of file indicator takes 2 byte value
                while(curEventIdx < evntFileInfo.nextMetaDataIdx)
                {
                    // Read event index information from current file position
                    if(read(evntFileFd, &eventInfo, EVNT_FIELD_SIZE) != EVNT_FIELD_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", camId, evntFileName, STR_ERR);
                        break;
                    }

                    timeInSec = eventInfo.startTime;
                    ConvertLocalTimeInBrokenTm(&timeInSec, &startTm);
                    timeInSec = eventInfo.endTime;
                    ConvertLocalTimeInBrokenTm(&timeInSec, &endTm);
                    curEventIdx++;

                    if (eventInfo.endTime == 0)
                    {
                        continue;
                    }

                    /* PARASOFT : No need to validate tainted data */
                    for(minCnt = startTm.tm_min; minCnt <= endTm.tm_min; minCnt++)
                    {
                        for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
                        {
                            if (FALSE == GET_BIT(eventInfo.eventType, evnt))
                            {
                                continue;
                            }

                            eventPresent = TRUE;
                            SET_BIT(minRecData[evnt], minCnt);
                            if(ovlpFlag[evnt] == FALSE)
                            {
                                ovlpFlag[evnt] = eventInfo.overLapFlg;
                            }
                        }
                    }
                }

                CloseFileFd(&evntFileFd);
                if (FALSE == eventPresent)
                {
                    continue;
                }

                // Find current position for given hour in minutwise storage stucture
                totalBits = (hourCnt * MIN_IN_ONE_HOUR);
                bitRemCnt = (MAX_BIT_IN_BYTE - curBit);

                if((startTm.tm_mon < JANUARY) || (startTm.tm_mon >= MAX_MONTH) || ((startTm.tm_mday - 1) < 0) || ((startTm.tm_mday - 1) >= MAX_DAYS))
                {
                    retVal = FAIL;
                    break;
                }

                for(evnt = REC_EVENT_MANUAL; evnt < REC_EVENT_MAX; evnt++)
                {
                    curByte = (totalBits / MAX_BIT_IN_BYTE);
                    curBit = (totalBits % MAX_BIT_IN_BYTE);
                    bitRemCnt = (MAX_BIT_IN_BYTE - curBit);

                    if(bitRemCnt >= MAX_BIT_IN_BYTE)
                    {
                        // convert 64 bit data into chunks of 8 bit
                        converter.rawData = minRecData[evnt];
                        memcpy(yearRecMap.monthMap[startTm.tm_mon].dayMap[startTm.tm_mday - 1].eventMap[evnt].recordMap + curByte, converter.convertedData, sizeof(UINT64));
                        curByte += (MIN_IN_ONE_HOUR/MAX_BIT_IN_BYTE);
                        curBit += (MAX_BIT_IN_LONG - MIN_IN_ONE_HOUR);
                    }
                    else
                    {
                        yearRecMap.monthMap[startTm.tm_mon].dayMap[startTm.tm_mday - 1].eventMap[evnt].recordMap[curByte] |= ((minRecData[evnt] & 0xff) << bitRemCnt);
                        curBit += bitRemCnt;
                        if(curBit >= MAX_BIT_IN_BYTE)
                        {
                            curBit = 0;
                            curByte++;
                        }

                        converter.rawData = (minRecData[evnt] >> bitRemCnt);
                        memcpy(yearRecMap.monthMap[startTm.tm_mon].dayMap[startTm.tm_mday - 1].eventMap[evnt].recordMap + curByte, converter.convertedData, sizeof(UINT64));
                        curByte += ((MIN_IN_ONE_HOUR - bitRemCnt) / MAX_BIT_IN_BYTE);
                        curBit += ((MIN_IN_ONE_HOUR - bitRemCnt) % MAX_BIT_IN_BYTE);;
                    }

                    if(yearRecMap.monthMap[startTm.tm_mon].dayMap[startTm.tm_mday - 1].eventMap[evnt].overlapFlag == FALSE)
                    {
                        yearRecMap.monthMap[startTm.tm_mon].dayMap[startTm.tm_mday - 1].eventMap[evnt].overlapFlag = ovlpFlag[evnt];
                    }
                }
            }

            /* If error found than do not process further */
            if (retVal == FAIL)
            {
                break;
            }

            if(lseek(yrMapFileFd, 0, SEEK_SET) < STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "fail to seek year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                retVal = FAIL;
                break;
            }

            if (write(yrMapFileFd, &yearRecMap, sizeof(YEAR_MAP_t)) != sizeof(YEAR_MAP_t))
            {
                EPRINT(DISK_MANAGER, "fail to write year map file: [camera=%d], [path=%s], [err=%s]", camId, fileName, STR_ERR);
                retVal = FAIL;
                break;
            }

            CloseFileFd(&yrMapFileFd);
        }

        /* Need to free all resources */
        closedir(dir);
        if (retVal == FAIL)
        {
            break;
        }

        buildInfo.lastBuildCameraId = camId;
        buildInfo.status = IN_PROGRESS;
        if(lseek(buildInfoFileFd, 0, SEEK_SET) < STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to seek build info file: [camera=%d], [err=%s]", camId, STR_ERR);
            retVal = FAIL;
            break;
        }

        if (write(buildInfoFileFd, &buildInfo, sizeof(BUILD_INFO_t)) != sizeof(BUILD_INFO_t))
        {
            EPRINT(DISK_MANAGER, "fail to write build info file: [camera=%d], [err=%s]", camId, STR_ERR);
            retVal = FAIL;
            break;
        }

        DPRINT(DISK_MANAGER, "index building completed: [camera=%d]", camId);
    }

    if (retVal == SUCCESS)
    {
        snprintf(buildInfo.macAddr, MAX_MAC_ADDRESS_WIDTH, "%s", macAddrs);
        buildInfo.buildVersion = BUILD_VERSION;
        buildInfo.lastBuildCameraId = getMaxCameraForCurrentVariant();
        buildInfo.status = SUCCESS;

        if (lseek(buildInfoFileFd, 0, SEEK_SET) < STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to seek build info file: [err=%s]", STR_ERR);
            retVal = FAIL;
        }
        else if (write(buildInfoFileFd, &buildInfo, sizeof(BUILD_INFO_t)) != sizeof(BUILD_INFO_t))
        {
            EPRINT(DISK_MANAGER, "fail to write build info file: [err=%s]", STR_ERR);
            retVal = FAIL;
        }
        else
        {
            DPRINT(DISK_MANAGER, "recording index building completed: [mntPoint=%s]", mntPoint);
        }
    }

    CloseFileFd(&evntFileFd);
    CloseFileFd(&yrMapFileFd);
    CloseFileFd(&buildInfoFileFd);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was check that time index information was properly organized or not.
 *          If file was not proper then removes some un useful information from file
 * @param   path
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL recoverTimeIdxFile(CHARPTR path, UINT32PTR pErrorCode)
{
    UINT8 					bufIdx;
    CHAR 					timeIdxBuf[2];
    CHAR 					timeFileName[MAX_FILE_NAME_SIZE];
    CHAR 					strmFileName[MAX_FILE_NAME_SIZE];
    INT32 					timeFileFd;
    UINT32 					fileOffset;
    UINT32 					strmFileSize;
    UINT32					strmFileNo;
    struct stat 			stateInfo;
    METADATA_FILE_HDR_t 	timeIdxFileInfo;
    TIMEIDX_FIELD_INFO_t 	timeIdxField;
    UINT32					curSysTick = GetSysTick();

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    // make file name for event file
    snprintf(timeFileName, MAX_FILE_NAME_SIZE, "%s%s", path, TIMEIDX_FILE_NAME);

    //Open time index  file
    timeFileFd = open(timeFileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(timeFileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open time-index file: [path=%s]", timeFileName);
        return FAIL;
    }

    // 	Get size of time index into fileSize
    if(stat(timeFileName, &stateInfo) < STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to stat time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
        close(timeFileFd);
        return FAIL;
    }

    // Read the file header of time index file. Get the next time index number in nextTimeIdxNo
    if(Utils_Read(timeFileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, pErrorCode) != TIMEIDX_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to read time-index file: [path=%s], [err=%s]", timeFileName, strerror(*pErrorCode));
        close(timeFileFd);
        return FAIL;
    }

    // each time index information takes 6 bytes and Calculate offset position in to file for nextTimeIdxNo
    fileOffset = ((timeIdxFileInfo.nextMetaDataIdx * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE);
    if((INT32)fileOffset < stateInfo.st_size)
    {
        // when last time index information was written into file but updation of next time index position was not possible.
        // So correct this condition here
        timeIdxFileInfo.nextMetaDataIdx += ((stateInfo.st_size - fileOffset) / TIME_INDEX_FIELD_SIZE);

        //Update nextTimeIdxNo in current event file @ file header @ 6th byte
        if(Utils_pWrite(timeFileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, 0, pErrorCode) != TIMEIDX_FILE_HDR_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to write time-index file: [path=%s], [err=%s]", timeFileName, strerror(*pErrorCode));
            close(timeFileFd);
            return FAIL;
        }

        // After nextTimeIdxNo, if any data was present that was garbage or incomplete data so remove it from file
        fileOffset = ((timeIdxFileInfo.nextMetaDataIdx * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE);
        if((stateInfo.st_size - fileOffset) > 0)
        {
            if(-1 == lseek(timeFileFd, fileOffset, SEEK_SET))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
                close(timeFileFd);
                return FAIL;
            }

            //remove no. of bytes from nextIFrameIdx in current event file
            if(-1 == ftruncate(timeFileFd, lseek(timeFileFd, 0, SEEK_CUR)))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to truncate time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
                close(timeFileFd);
                return FAIL;
            }
        }
    }

    if(-1 == lseek(timeFileFd, fileOffset, SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
        close(timeFileFd);
        return FAIL;
    }

    bufIdx = 0;
    timeIdxBuf[bufIdx++] = (UINT8)TIMEINDEX_FILE_EOF;
    timeIdxBuf[bufIdx++] = TIMEINDEX_FILE_EOF >> 8;

    // 	Write end of file indicator of time index file
    if(Utils_Write(timeFileFd, timeIdxBuf, bufIdx, pErrorCode) != bufIdx)
    {
        EPRINT(DISK_MANAGER, "fail to write time-index file: [path=%s], [err=%s]", timeFileName, strerror(*pErrorCode));
        close(timeFileFd);
        return FAIL;
    }

    // Read the file header of time index
    if(pread(timeFileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, lseek(timeFileFd, 0, SEEK_SET)) != TIMEIDX_FILE_HDR_SIZE)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to read time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
        close(timeFileFd);
        return FAIL;
    }

    if(getAllStreamFileCount(path, &strmFileNo, NULL) == FAIL)
    {
        close(timeFileFd);
        EPRINT(DISK_MANAGER, "fail to get all stream file count: [path=%s]", path);
        return FAIL;
    }

    while(timeIdxFileInfo.nextMetaDataIdx > 0)
    {
        if(ElapsedTick(curSysTick) >= CONVERT_SEC_TO_TIMER_COUNT(RECOVERY_TIME_OUT))
        {
            close(timeFileFd);
            EPRINT(DISK_MANAGER, "recovery timeout occurred");
            return FAIL;
        }

        // get last time index information position in file
        fileOffset = (((timeIdxFileInfo.nextMetaDataIdx - 1) * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE);
        if(-1 == lseek(timeFileFd, fileOffset, SEEK_SET))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to seek time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
            close(timeFileFd);
            return FAIL;
        }

        // Read time index information in readBuff and Get streamFileId, streamFilePos from readBuff (as per time index information)
        if(Utils_Read(timeFileFd, &timeIdxField, TIME_INDEX_FIELD_SIZE, pErrorCode) != TIME_INDEX_FIELD_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read time-index file: [path=%s], [err=%s]", timeFileName, strerror(*pErrorCode));
            close(timeFileFd);
            return FAIL;
        }

        // get current FSH position for last time index information in time index  file
        if(timeIdxField.streamFileId <= strmFileNo)
        {
            // Get stream file name from streamFileId in streamFile and Get stream file size in streamFileSize
            if(getStreamFileName(path, strmFileNo, strmFileName, &strmFileSize) == FAIL)
            {
                close(timeFileFd);
                EPRINT(DISK_MANAGER, "fail to get stream file name: [path=%s], [strmFileId=%d]", path, strmFileNo);
                return FAIL;
            }

            // Check that this streamFilePos is valid or not
            if(strmFileSize >= timeIdxField.fshPos)
            {
                // stream file position was properly set as per time index says
                break;
            }
        }

        // Go to next time index index number
        if(-1 == ftruncate(timeFileFd, lseek(timeFileFd, fileOffset, SEEK_SET)))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to truncate time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
            close(timeFileFd);
            return FAIL;
        }
        timeIdxFileInfo.nextMetaDataIdx--;
    }

    // get last time index information position in file
    fileOffset = ((timeIdxFileInfo.nextMetaDataIdx * TIME_INDEX_FIELD_SIZE) + TIMEIDX_FILE_HDR_SIZE);

    // Go to next time index  index number
    if(-1 == lseek(timeFileFd, fileOffset, SEEK_SET))
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to seek time-index file: [path=%s], [err=%s]", timeFileName, STR_ERR);
        close(timeFileFd);
        return FAIL;
    }

    // Update nextIFrameIdx in current time index  file @ file header @ 5th byte
    if (Utils_pWrite(timeFileFd, &timeIdxFileInfo, TIMEIDX_FILE_HDR_SIZE, 0, pErrorCode) != TIMEIDX_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write time-index file: [path=%s], [err=%s]", timeFileName, strerror(*pErrorCode));
        close(timeFileFd);
        return FAIL;
    }

    bufIdx = 0;
    timeIdxBuf[bufIdx++] = (UINT8)TIMEINDEX_FILE_EOF;
    timeIdxBuf[bufIdx++] = TIMEINDEX_FILE_EOF >> 8;

    // Write end of file indicator of time index  file
    if(Utils_Write(timeFileFd, timeIdxBuf, bufIdx, pErrorCode) != bufIdx)
    {
        EPRINT(DISK_MANAGER, "fail to write time-index file: [path=%s], [err=%s]", timeFileName, strerror(*pErrorCode));
        close(timeFileFd);
        return FAIL;
    }

    close(timeFileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was updates next stream position in stream file header in case of mis
 *          matching of index and removes garbage data from file
 * @param   fileName
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL recoverStreamFile(CHARPTR fileName, UINT32PTR pErrorCode)
{
    INT32 				strmFileFd;
    UINT32 				curStrmPos;
    CHARPTR 			charSearch;
    CHAR 				tempName[MAX_FILE_NAME_SIZE];
    CHAR 				reName[MAX_FILE_NAME_SIZE];
    struct tm 			localTime;
    struct stat 		stateInfo;
    STRM_FILE_HDR_t 	strmFileHdr;
    STRM_FILE_END_t 	strmFileEnd;
    FSH_INFO_t 			fshData = { 0 };
    UINT32				curSysTick = GetSysTick();

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    // open stream file
    strmFileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(strmFileFd == INVALID_FILE_FD)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to open stream file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
    }

    //Get file size
    if(stat(fileName, &stateInfo) < STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to stat stream file: [path=%s], [err=%s]", fileName, STR_ERR);
        close(strmFileFd);
        return FAIL;
    }

    if (stateInfo.st_size < (INT32)MAX_FSH_SIZE)
    {
        // file was not proper so remove this file from database
        if (remove(fileName) != STATUS_OK)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to remove stream file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(strmFileFd);
            return FAIL;
        }

        close(strmFileFd);
        return SUCCESS;
    }


    // Read stream file header from starting of file
    if(Utils_Read(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, pErrorCode) != STREAM_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", fileName, strerror(*pErrorCode));
        close(strmFileFd);
        return FAIL;
    }

    // Checks RUN flag status. IF RUN flag is 1 then recovery of stream file started otherwise stream file has not any problem
    if (FALSE == strmFileHdr.runFlg)
    {
        close(strmFileFd);
        return SUCCESS;
    }

    curStrmPos = STREAM_FILE_HDR_SIZE;
    /* PARASOFT : No need to validate tainted data */
    while(stateInfo.st_size >= (INT32)(curStrmPos + MAX_FSH_SIZE))
    {
        if(ElapsedTick(curSysTick) >= CONVERT_SEC_TO_TIMER_COUNT(RECOVERY_TIME_OUT))
        {
            close(strmFileFd);
            EPRINT(DISK_MANAGER, "recovery timeout occurred");
            return FAIL;
        }

        // Go to currStreamPos in file
        if(-1 == lseek(strmFileFd, curStrmPos, SEEK_SET))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(strmFileFd);
            return FAIL;
        }

        // read fsh data from file
        if(Utils_Read(strmFileFd, &fshData, MAX_FSH_SIZE, pErrorCode) != MAX_FSH_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read stream file fsh: [path=%s], [err=%s]", fileName, strerror(*pErrorCode));
            close(strmFileFd);
            return FAIL;
        }

        //Check valid FSH present
        if(fshData.startCode != FSH_START_CODE)
        {
            if(STATUS_OK != ftruncate(strmFileFd, curStrmPos))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to truncate stream file: [path=%s], [err=%s]", fileName, STR_ERR);
                close(strmFileFd);
                return FAIL;
            }

            EPRINT(DISK_MANAGER, "valid fsh not found: [path=%s]", fileName);
            close(strmFileFd);
            return UNKNOWN;
        }

        // check that stream file size was more than current FSH + payload size
        if(stateInfo.st_size >= (INT32)((curStrmPos + fshData.fshLen + MAX_FSH_SIZE)))
        {
            // update current stream position
            curStrmPos = fshData.nextFShPos;
            continue;
        }

        // set currrent stream position to previous FSH position
        curStrmPos = fshData.prevFshPos;
        if(-1 == lseek(strmFileFd, curStrmPos, SEEK_SET))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(strmFileFd);
            return FAIL;
        }

        if(Utils_Read(strmFileFd, &fshData, MAX_FSH_SIZE, pErrorCode) != MAX_FSH_SIZE)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to read stream file fsh: [path=%s], [err=%s]", fileName, strerror(*pErrorCode));
            close(strmFileFd);
            return FAIL;
        }

        /* Truncate stream file */
        curStrmPos = fshData.nextFShPos;
        if(-1 == lseek(strmFileFd, curStrmPos, SEEK_SET))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(strmFileFd);
            return FAIL;
        }

        DPRINT(DISK_MANAGER, "current stream file status: [path=%s], [position=%d]", fileName, curStrmPos);
        if(STATUS_OK != ftruncate(strmFileFd, lseek(strmFileFd, 0, SEEK_CUR)))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to truncate stream file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(strmFileFd);
            return FAIL;
        }
        break;
    }

    // Write currStreamPos @ next stream position in stream file header
    strmFileHdr.nextStreamPos = curStrmPos;
    strmFileHdr.runFlg = FALSE;

    DPRINT(DISK_MANAGER, "write stream header: [path=%s], [position=%d]", fileName, curStrmPos);
    if(Utils_pWrite(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, 0, pErrorCode) != STREAM_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to read stream file fsh: [path=%s], [err=%s]", fileName, strerror(*pErrorCode));
        close(strmFileFd);
        return FAIL;
    }

    //	Write valid end of file @ currStreamPos
    strmFileEnd.fileEndIndicator = STRM_EOF_INDICATOR;
    strmFileEnd.prevFshPos = (fshData.nextFShPos - fshData.fshLen);
    if(Utils_pWrite(strmFileFd, &strmFileEnd, STREAM_FILE_END_SIZE, lseek(strmFileFd, 0, SEEK_END), pErrorCode) != STREAM_FILE_END_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to write stream file fsh: [path=%s], [err=%s]", fileName, strerror(*pErrorCode));
        close(strmFileFd);
        return FAIL;
    }

    /* Close the stream file */
    close(strmFileFd);

    /* Update the stream file name */
    charSearch = strchr(fileName, '~');
    if (charSearch == NULL)
    {
        return SUCCESS;
    }

    /* Get file start time */
    snprintf(tempName, MIN(MAX_FILE_NAME_SIZE, (strlen(fileName) - strlen(charSearch) + 1)), "%s", fileName);
    charSearch = strchr(fileName, '.');
    if(charSearch == NULL)
    {
        return SUCCESS;
    }

    time_t totalSec = fshData.localTime.totalSec;
    ConvertLocalTimeInBrokenTm(&totalSec, &localTime);
    snprintf(reName, MAX_FILE_NAME_SIZE, "%s~"REC_FOLDER_TIME_FORMAT"%s", tempName, localTime.tm_hour, localTime.tm_min, localTime.tm_sec, charSearch);
    if(rename(fileName, reName) != STATUS_OK)
    {
        SET_ERROR_NUMBER(pErrorCode, errno);
        EPRINT(DISK_MANAGER, "fail to rename stream file: [old=%s], [new=%s], [err=%s]", fileName, reName, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function find out newest folder with 2 directory depth from given base directory and channelNo
 * @param   mntPoint
 * @param   channelNo
 * @param   newestFolder
 * @param   recoveryInfo
 * @param   pErrorCode
 * @return  SUCCESS/FAIL
 */
static BOOL getLatestHourFolder(CHARPTR mntPoint, UINT8 channelNo, CHARPTR newestFolder, RECOVERY_INFO_t *recoveryInfo, UINT32PTR pErrorCode)
{
    UINT8           subDir;
    CHAR            folderName[MAX_FILE_NAME_SIZE];
    CHAR            channelFolder[MAX_FILE_NAME_SIZE];
    CHAR            newChannelFolder[MAX_FILE_NAME_SIZE];
    CHAR            month[10];
    CHARPTR         tmpPtr = NULL;
    INT32           channelCnt = 0, date = 0, year = 0, hour = 0;
    DIR 			*dir;
    struct dirent 	*entry;
    struct stat		channelStat;
    struct stat		newChannelStat;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);
    snprintf(folderName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL, mntPoint, channelNo);
    snprintf(newChannelFolder, MAX_FILE_NAME_SIZE, "%s", folderName);

    for(subDir = 0; subDir < 2; subDir++)
    {
        memset(&newChannelStat, 0, sizeof(newChannelStat));
        snprintf(channelFolder, MAX_FILE_NAME_SIZE, "%s", newChannelFolder);

        dir = opendir(channelFolder);
        if(dir == NULL)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to open dir: [path=%s], [err=%s]", channelFolder, STR_ERR);
            newestFolder[0] = '\0';
            return FAIL;
        }

        while((entry = readdir(dir)) != NULL)
        {
            if(strstr(entry->d_name, YEAR_MAP_FILE_EXTN))
            {
                continue;
            }

            // check this file was stream file
            snprintf(folderName, MAX_FILE_NAME_SIZE, "%s%s/", channelFolder, entry->d_name);
            if (stat(folderName, &channelStat) != STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "fail to stat channel dir: [path=%s], [err=%s]", folderName, STR_ERR);
                SET_ERROR_NUMBER(pErrorCode, errno);
                closedir(dir);
                newestFolder[0] = '\0';
                return FAIL;
            }

            if((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
            {
                continue;
            }

            if((channelStat.st_mtime > newChannelStat.st_mtime))
            {
                snprintf(newChannelFolder, MAX_FILE_NAME_SIZE, "%s", folderName);
                memcpy(&newChannelStat, &channelStat, sizeof(newChannelStat));
            }
        }

        DPRINT(DISK_MANAGER, "newest folder found: [path=%s]", newChannelFolder);
        closedir(dir);
    }

    snprintf(newestFolder, MAX_FILE_NAME_SIZE, "%s", newChannelFolder);
    tmpPtr = strstr(newChannelFolder, CHANNEL_NAME_REC);
    if (NULL == tmpPtr)
    {
        return FAIL;
    }

    sscanf(tmpPtr, CHANNEL_NAME_REC"%02d/%d_%03s_%d/%d/",&channelCnt, &date, month, &year,&hour);
    recoveryInfo->date = date;
    recoveryInfo->mon = GetMonthNo(month);
    recoveryInfo->year = year;
    recoveryInfo->hour = hour;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was read a recovery file from hardisk for each channel and starts recovery
 *          of corrupted files. This function was read last modified date and hour value and performs
 *          recovery for that folder only.
 * @param   mntPoint
 * @param   iDiskId
 * @return  SUCCESS/FAIL
 */
BOOL StartRecovery(CHARPTR 	mntPoint, RECORD_ON_DISK_e iDiskId)
{
    BOOL				retVal = FAIL;
    BOOL				strmRecStatus;
    BOOL				remFolder = FALSE;
    BOOL				latestFolderFlg;
    UINT8 				channelCnt;
    INT32 				recFileFd = INVALID_FILE_FD;
    UINT32				streamFileNo;
    UINT32				aviFileNo;
    CHAR 				recoveryFldr[MAX_FILE_NAME_SIZE];
    CHAR 				recvrFileName[MAX_FILE_NAME_SIZE];
    CHAR 				folderName[MAX_FILE_NAME_SIZE];
    CHAR 				strmFileName[MAX_FILE_NAME_SIZE];
    RECOVERY_INFO_t 	recveryInfo;
    UINT8				folderRecCnt = 0;
    time_t				localTime;
    struct tm			brokenTime;
    CHARPTR 			extension;
    DIR 				*dir;
    struct dirent 		*entry;
    UINT32              tErrorCode = INVALID_ERROR_CODE;
    GENERAL_CONFIG_t	generalCfg;
    CHAR                ifrmFileName[MAX_FILE_NAME_SIZE];

    ReadGeneralConfig(&generalCfg);
    MUTEX_LOCK(recoveryMutex);
    recoveryStatus = START;
    MUTEX_UNLOCK(recoveryMutex);

    do
    {
        // check that recovery folder was present in specific partition
        snprintf(recoveryFldr, MAX_FILE_NAME_SIZE, RECOVERY_FOLDER, mntPoint);
        if (access(recoveryFldr, F_OK) != STATUS_OK)
        {
            if (isDiskInFault(errno, iDiskId))
            {
                break;
            }

            retVal = SUCCESS;
            break;
        }

        // recovery status check because at shutdown no need to recover file further.
        for (channelCnt = 0; (channelCnt < getMaxCameraForCurrentVariant() && (recoveryStatus == START)); channelCnt++)
        {
            latestFolderFlg = FALSE;

            // check channel file was present
            snprintf(recvrFileName, MAX_FILE_NAME_SIZE, RECOVERY_CHANNEL_FILE, recoveryFldr, GET_CAMERA_NO(channelCnt));
            if (access(recvrFileName, F_OK) != STATUS_OK)
            {
                if (isDiskInFault(errno, iDiskId))
                {
                    break;
                }

                /* Check next channel */
                continue;
            }

            // Open file for channelCnt
            recFileFd = open(recvrFileName, READ_WRITE_MODE, FILE_PERMISSION);
            if (recFileFd == INVALID_FILE_FD)
            {
                EPRINT(DISK_MANAGER, "fail to open recovery file: [file=%s], [err=%s]", recvrFileName, STR_ERR);
                if (isDiskInFault(errno, iDiskId))
                {
                    break;
                }

                /* Check next channel */
                continue;
            }

            /* Read the data from file. Read last modified date and hour from that file */
            if (Utils_Read(recFileFd, &recveryInfo, RECOVERY_FIELD_SIZE, &tErrorCode) != RECOVERY_FIELD_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read recovery file: [file=%s], [err=%s]", recvrFileName, strerror(tErrorCode));
                latestFolderFlg = TRUE;
                if (isDiskInFault(tErrorCode, iDiskId))
                {
                    close(recFileFd);
                    break;
                }
            }
            else
            {
                /* Get recovery date and hour. create folder name from where recovery is to be started */
                snprintf(folderName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mntPoint,
                         GET_CAMERA_NO(channelCnt), recveryInfo.date, GetMonthName(recveryInfo.mon), recveryInfo.year, recveryInfo.hour);
                if(access(folderName, F_OK) != STATUS_OK)
                {
                    if (isDiskInFault(errno, iDiskId))
                    {
                        close(recFileFd);
                        break;
                    }
                    latestFolderFlg = TRUE;
                }
            }

            // closed recovery file
            close(recFileFd);

            // check recovery information ok !
            if(latestFolderFlg == TRUE)
            {
                if(getLatestHourFolder(mntPoint, GET_CAMERA_NO(channelCnt), folderName, &recveryInfo, &tErrorCode) == FAIL)
                {
                    if(isDiskInFault(tErrorCode, iDiskId))
                    {
                        break;
                    }

                    // no folder was found.
                    DPRINT(DISK_MANAGER, "latest hour folder not found: [camera=%d]", channelCnt);
                    continue;
                }

                DPRINT(DISK_MANAGER, "latest hour folder found: [path=%s]", folderName);
            }

            folderRecCnt = 0;
            while(folderRecCnt < 2)
            {
                folderRecCnt++;

                // create folder name from where recovery is to be started
                snprintf(folderName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mntPoint,
                         GET_CAMERA_NO(channelCnt), recveryInfo.date, GetMonthName(recveryInfo.mon), recveryInfo.year, recveryInfo.hour);
                if(access(folderName, F_OK) == STATUS_OK)
                {
                    /* if i-frame file is present then recover the stream file using i-frame file */
                    snprintf(ifrmFileName, MAX_FILE_NAME_SIZE, "%s%s", folderName, I_FRAME_FILE_NAME);
                    if(STATUS_OK == access(ifrmFileName, F_OK))
                    {
                        /* recover stream file and i-frame file */
                        if(FALSE == recoverStreamAndIFrameFile(folderName, &tErrorCode))
                        {
                            EPRINT(DISK_MANAGER, "fail to recover stream file and i-frame file: [path=%s]", folderName);
                            if (isDiskInFault(tErrorCode, iDiskId))
                            {
                                break;
                            }

                            remFolder = TRUE;
                        }
                    }
                    else
                    {
                        /* this case happens when recording is done in avi format then there is no i-frame file only stream file is present */
                        dir = opendir(folderName);
                        if (dir == NULL)
                        {
                            EPRINT(DISK_MANAGER, "fail to open dir: [path=%s]", folderName);
                            if (isDiskInFault(errno, iDiskId))
                            {
                                break;
                            }

                            // Remove whole directory.
                            if (FAIL == RemoveDirectory(folderName))
                            {
                                continue;
                            }

                            if (FAIL == removeIndexesforGivenHour(mntPoint, channelCnt, recveryInfo, &tErrorCode))
                            {
                                if (isDiskInFault(tErrorCode, iDiskId))
                                {
                                    break;
                                }
                            }
                            continue;
                        }

                        // read every file of current directory
                        while ((entry = readdir(dir)) != NULL)
                        {
                            // check this file was stream file
                            if (((extension = strstr(entry->d_name, STREAM_EXTN)) != NULL) || ((extension = strstr(entry->d_name, STREAM_EXTN_OVLP)) != NULL))
                            {
                                // make file name for stream file
                                snprintf(strmFileName, MAX_FILE_NAME_SIZE, "%s%s", folderName, entry->d_name);

                                // recover stream file
                                strmRecStatus = recoverStreamFile(strmFileName, &tErrorCode);
                                if ((strmRecStatus == FAIL)
                                        || ((strmRecStatus == UNKNOWN) && (recoverStreamFile(strmFileName, &tErrorCode) != SUCCESS)))
                                {
                                    remFolder = TRUE;
                                    break;
                                }
                            }
                        }

                        // closed current open directory
                        closedir(dir);

                        if (isDiskInFault(tErrorCode, iDiskId))
                        {
                            break;
                        }
                    }

                    getAllStreamFileCount(folderName, &streamFileNo, &aviFileNo);
                    if ((aviFileNo == 0) && (streamFileNo == 0))
                    {
                        EPRINT(DISK_MANAGER, "recovery folder is empty: [path=%s]", folderName);
                        remFolder = TRUE;
                    }

                    if (remFolder == TRUE)
                    {
                        // Remove whole directory.
                        remFolder = FALSE;
                        if ((RemoveDirectory(folderName) == SUCCESS) && (generalCfg.recordFormatType != REC_AVI_FORMAT))
                        {
                            if(FAIL == removeIndexesforGivenHour(mntPoint, channelCnt, recveryInfo, &tErrorCode))
                            {
                                if(isDiskInFault(tErrorCode, iDiskId))
                                {
                                    break;
                                }
                            }
                        }

                        EPRINT(DISK_MANAGER, "remove whole folder due to issue in stream file recovery: [path=%s]", folderName);
                        continue;
                    }

                    if (generalCfg.recordFormatType != REC_AVI_FORMAT)
                    {
                        if (recoverEventFile(folderName, &tErrorCode) == FAIL)
                        {
                            if(isDiskInFault(tErrorCode, iDiskId))
                            {
                                break;
                            }

                            // Remove whole directory.
                            if(RemoveDirectory(folderName) == SUCCESS)
                            {
                                if(FAIL == removeIndexesforGivenHour(mntPoint, channelCnt, recveryInfo, &tErrorCode))
                                {
                                    if(isDiskInFault(tErrorCode, iDiskId))
                                    {
                                        break;
                                    }
                                }
                            }

                            EPRINT(DISK_MANAGER, "remove whole folder due to issue in event file recovery: [path=%s]", folderName);
                            continue;
                        }

                        if(FAIL == recoverMonthDayIndxFiles(mntPoint, folderName, channelCnt,recveryInfo,&tErrorCode))
                        {
                            if(isDiskInFault(tErrorCode, iDiskId))
                            {
                                break;
                            }
                        }

                        if(recoverTimeIdxFile(folderName, &tErrorCode) == FAIL)
                        {
                            if(isDiskInFault(tErrorCode, iDiskId))
                            {
                                break;
                            }

                            // Remove whole directory.
                            if(RemoveDirectory(folderName) == SUCCESS)
                            {
                                if(FAIL == removeIndexesforGivenHour(mntPoint, channelCnt, recveryInfo, &tErrorCode))
                                {
                                    if(isDiskInFault(tErrorCode, iDiskId))
                                    {
                                        break;
                                    }
                                }
                            }

                            EPRINT(DISK_MANAGER, "remove whole folder due to issue in time index file recovery: [path=%s]", folderName);
                            continue;
                        }

                        getAllStreamFileCount(folderName, &streamFileNo,&aviFileNo);
                        if((aviFileNo == 0) && (streamFileNo == 0))
                        {
                            // Remove whole directory.
                            if(RemoveDirectory(folderName) == SUCCESS)
                            {
                                if(FAIL == removeIndexesforGivenHour(mntPoint, channelCnt, recveryInfo, &tErrorCode))
                                {
                                    if(isDiskInFault(tErrorCode, iDiskId))
                                    {
                                        break;
                                    }
                                }
                            }

                            EPRINT(DISK_MANAGER, "folder removed due to no stream file: [path=%s]", folderName);
                        }
                    }
                }
                else
                {
                    if (isDiskInFault(errno, iDiskId))
                    {
                        break;
                    }
                }

                brokenTime.tm_year = recveryInfo.year;
                brokenTime.tm_mon = recveryInfo.mon;
                brokenTime.tm_mday = recveryInfo.date;
                brokenTime.tm_hour = recveryInfo.hour;
                brokenTime.tm_min = 0;
                brokenTime.tm_sec = 0;
                RESET_EXTRA_BORKEN_TIME_VAR(brokenTime);
                ConvertLocalTimeInSec(&brokenTime, &localTime);
                localTime += SEC_IN_ONE_HOUR;
                ConvertLocalTimeInBrokenTm(&localTime, &brokenTime);
                recveryInfo.year = brokenTime.tm_year;
                recveryInfo.mon = brokenTime.tm_mon;
                recveryInfo.date = brokenTime.tm_mday;
                recveryInfo.hour = brokenTime.tm_hour;
            }

            if (isDiskInFault(tErrorCode, iDiskId))
            {
                break;
            }
        }

        retVal = SUCCESS;
    }
    while(0);

    DPRINT(DISK_MANAGER, "end recording recovery: [path=%s]", mntPoint);
    MUTEX_LOCK(recoveryMutex);
    recoveryStatus = STOP;
    MUTEX_UNLOCK(recoveryMutex);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread was read last modified date and hour value and performs recovery for that
 *          folder only. and also convert into .avi file
 * @param   aviInfo
 * @return  SUCCESS/FAIL
 */
VOIDPTR StmToAviConvertor(VOIDPTR aviInfo)
{
    BOOL				latestFolderFlg;
    UINT8 				channelCnt;
    CHAR 				foldername[MAX_FILE_NAME_SIZE];
    CHAR 				strmFilename[MAX_FILE_NAME_SIZE];
    RECOVERY_INFO_t 	recveryInfo;
    AVI_CONVERT_t       *aviConvrtInfo = (AVI_CONVERT_t *)aviInfo;
    UINT8				folderRecCnt = 0;
    time_t				localTime;
    struct tm			brokenTime;
    DIR 				*dir;
    struct dirent 		*entry;
    INT32				aviFileFd;
    struct stat         stateInfoAviFile;
    CHAR 				mountPoint[MOUNT_POINT_SIZE];
    CHAR				aviFileName[MAX_FILE_NAME_SIZE];
    CHARPTR				charSearch;
    UINT16              outLen;
    UINT32              tErrorCode = INVALID_ERROR_CODE;

    THREAD_START("STM_TO_AVI_CONV");
    snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", aviConvrtInfo->aviRecMountPoint);
    DPRINT(DISK_MANAGER, "stm to avi convertor started: [mountPoint=%s]", mountPoint);

    for (channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
    {
        latestFolderFlg = FALSE;
        recveryInfo = aviConvrtInfo->recoveryInfoForAvi[channelCnt];

        // get recovery date and hour. create folder name from where recovery is to be started
        snprintf(foldername, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mountPoint,
                 GET_CAMERA_NO(channelCnt), recveryInfo.date, GetMonthName(recveryInfo.mon), recveryInfo.year, recveryInfo.hour);
        if (access(foldername, F_OK) == STATUS_OK)
        {
            DPRINT(DISK_MANAGER, "recovery folder found: [camera=%d], [path=%s]", channelCnt, foldername);
        }
        else
        {
            latestFolderFlg = TRUE;
        }

        // check recovery information ok !
        if(latestFolderFlg == TRUE)
        {
            if(getLatestHourFolder(mountPoint, GET_CAMERA_NO(channelCnt), foldername, &recveryInfo, NULL) == FAIL)
            {
                EPRINT(DISK_MANAGER, "latest hour folder not found: [camera=%d], [path=%s]", channelCnt, foldername);
                continue;
            }

            DPRINT(DISK_MANAGER, "latest hour folder: [camera=%d], [path=%s]", channelCnt, foldername);
        }

        folderRecCnt = 0;

        while(folderRecCnt < 2)
        {
            folderRecCnt++;
            // create folder name from where recovery is to be started
            snprintf(foldername, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mountPoint,
                     GET_CAMERA_NO(channelCnt), recveryInfo.date, GetMonthName(recveryInfo.mon), recveryInfo.year, recveryInfo.hour);
            if(access(foldername, F_OK) == STATUS_OK)
            {
                // recover for stream file
                DPRINT(DISK_MANAGER, "recovery folder: [camera=%d], [path=%s]", channelCnt, foldername);
                dir = opendir(foldername);
                if (dir == NULL)
                {
                    // Remove whole directory.
                    EPRINT(DISK_MANAGER, "fail to open dir: [camera=%d], [path=%s], [err=%s]", channelCnt, foldername, STR_ERR);
                    if (SUCCESS == RemoveDirectory(foldername))
                    {
                        removeIndexesforGivenHour(mountPoint, channelCnt, recveryInfo, &tErrorCode);
                    }
                    continue;
                }

                // read every file of current directory
                while((entry = readdir(dir)) != NULL)
                {
                    // check this file was stream file
                    if ((strstr(entry->d_name, STREAM_EXTN) == NULL) && (strstr(entry->d_name, STREAM_EXTN_OVLP) == NULL))
                    {
                        /* This is not a stream file */
                        continue;
                    }

                    /* Stream file should not be incomplete */
                    if (strstr(entry->d_name, STREAM_FILE_NOT_COMPLETE) != NULL)
                    {
                        /* This is incomplete stream file */
                        continue;
                    }

                    // make file name for stream file
                    snprintf(strmFilename, MAX_FILE_NAME_SIZE, "%s%s", foldername, entry->d_name);
                    if ((charSearch = strrchr(strmFilename, STM_FILE_SEP)) == NULL)
                    {
                        EPRINT(DISK_MANAGER, "file seperator not found: [camera=%d], [file=%s]", channelCnt, strmFilename);
                        continue;
                    }

                    /* Remove incomplete avi files */
                    outLen = ((strlen(strmFilename) - strlen(charSearch)) + 1);
                    snprintf(aviFileName, outLen > MAX_FILE_NAME_SIZE ? MAX_FILE_NAME_SIZE : outLen, "%s", strmFilename);
                    snprintf(&aviFileName[strlen(aviFileName)], MAX_FILE_NAME_SIZE - (strlen(aviFileName)), "_incomplete.avi");
                    if(access(aviFileName, F_OK) == STATUS_OK)
                    {
                        if(unlink(aviFileName) == STATUS_OK)
                        {
                            DPRINT(DISK_MANAGER, "incomplete avi file removed: [camera=%d], [file=%s]", channelCnt, aviFileName);
                        }
                        else
                        {
                            EPRINT(DISK_MANAGER, "fail to remove incomplete avi file: [camera=%d], [file=%s], [err=%s]", channelCnt, aviFileName, STR_ERR);
                        }
                    }

                    snprintf(aviFileName, outLen > MAX_FILE_NAME_SIZE ? MAX_FILE_NAME_SIZE : outLen, "%s", strmFilename);
                    snprintf(&aviFileName[strlen(aviFileName)], MAX_FILE_NAME_SIZE - (strlen(aviFileName)), ".avi");
                    aviFileFd = open(aviFileName, READ_ONLY_MODE, FILE_PERMISSION);
                    if(aviFileFd == INVALID_FILE_FD)
                    {
                        DPRINT(DISK_MANAGER, "avi file not present, require to convert: [camera=%d], [file=%s]", channelCnt, aviFileName);
                        close(aviFileFd);
                        convertStrmToAvi(strmFilename);
                        continue;
                    }

                    //Get file size
                    if(stat(aviFileName, &stateInfoAviFile) != STATUS_OK)
                    {
                        EPRINT(DISK_MANAGER, "fail to stat avi file, require to convert: [camera=%d], [file=%s], [err=%s]", channelCnt, aviFileName, STR_ERR);
                        close(aviFileFd);
                        convertStrmToAvi(strmFilename);
                        continue;
                    }

                    DPRINT(DISK_MANAGER, "avi file present and not require to convert: [camera=%d], [file=%s]", channelCnt, aviFileName);
                    close(aviFileFd);
                }

                // closed current open directory
                closedir(dir);
            }
            else
            {
                DPRINT(DISK_MANAGER, "recovery folder not present: [camera=%d], [path=%s]", channelCnt, foldername);
            }

            brokenTime.tm_year = recveryInfo.year;
            brokenTime.tm_mon = recveryInfo.mon;
            brokenTime.tm_mday = recveryInfo.date;
            brokenTime.tm_hour = recveryInfo.hour;
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            RESET_EXTRA_BORKEN_TIME_VAR(brokenTime);
            ConvertLocalTimeInSec(&brokenTime, &localTime);
            localTime += SEC_IN_ONE_HOUR;
            ConvertLocalTimeInBrokenTm(&localTime, &brokenTime);
            recveryInfo.year = brokenTime.tm_year;
            recveryInfo.mon = brokenTime.tm_mon;
            recveryInfo.date = brokenTime.tm_mday;
            recveryInfo.hour = brokenTime.tm_hour;
        }
    }

    DPRINT(DISK_MANAGER, "stm to avi convertor ended: [mountPoint=%s]", mountPoint);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was search stream file and gives file name and file size as output parameters.
 * @param   path - directory path from where file should search
 * @param   fileId
 * @param   filename
 * @param   fileSize
 * @return  SUCCESS/FAIL
 */
static BOOL getStreamFileName(CHARPTR path, UINT8 fileId, CHARPTR filename, UINT32PTR fileSize)
{
    DIR             *dir;
    struct dirent   *entry;
    CHAR            fileExten[10];
    CHARPTR         fileCmp;
    struct stat     tFileInfo;
    INT32           tFileFd;

    /* Reset input vars */
    filename[0] = '\0';
    *fileSize = 0;

    dir = opendir(path);
    if (dir == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to open dir: [path=%s], [err=%s]", path, STR_ERR);
        return FAIL;
    }

    /* Prepared stream file extention */
    snprintf(fileExten, sizeof(fileExten), "stm%d", fileId);

    // read every file of current directory
    while((entry = readdir(dir)) != NULL)
    {
        // check this file was stream file
        if ((fileCmp = strstr(entry->d_name, fileExten)) == NULL)
        {
            continue;
        }

        if (strcmp(fileCmp, fileExten) != STATUS_OK)
        {
            continue;
        }

        snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s", path, entry->d_name);
        tFileFd = open(filename, READ_ONLY_MODE);
        if (tFileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open stream file: [path=%s], [err=%s]", filename, STR_ERR);
            break;
        }

        if (fstat(tFileFd, &tFileInfo) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to stat stream file: [path=%s], [err=%s]", filename, STR_ERR);
            close(tFileFd);
            break;
        }

        *fileSize = tFileInfo.st_size;
        close(tFileFd);
        closedir(dir);
        return SUCCESS;
    }

    closedir(dir);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove all zero byte stream file from folder
 * @param   path
 * @note    Workaround cleanup for FIT
 */
static void removeBlankStreamFiles(CHARPTR path)
{
    DIR             *dir;
    struct dirent   *entry;
    INT32           fileFd;
    struct stat     fileInfo;
    CHAR            streamFileName[MAX_FILE_NAME_SIZE];

    dir = opendir(path);
    if (dir == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to open dir: [path=%s], [err=%s]", path, STR_ERR);
        return;
    }

    /* Read every file of current directory */
    while((entry = readdir(dir)) != NULL)
    {
        /* Check this file is stream file */
        if (strstr(entry->d_name, "stm") == NULL)
        {
            continue;
        }

        /* Prepare the stream file with relative path and open it */
        snprintf(streamFileName, MAX_FILE_NAME_SIZE, "%s%s", path, entry->d_name);
        fileFd = open(streamFileName, READ_ONLY_MODE);
        if (fileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open stream file: [path=%s], [err=%s]", streamFileName, STR_ERR);
            continue;
        }

        /* Read file info for size */
        if (fstat(fileFd, &fileInfo) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to stat stream file: [path=%s], [err=%s]", streamFileName, STR_ERR);
            close(fileFd);
            continue;
        }

        /* Close the file and remove if it is zero */
        close(fileFd);
        if (fileInfo.st_size == 0)
        {
            unlink(streamFileName);
        }
    }

    closedir(dir);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was search recorded file based on all event of all camera number.
 * @param   pSeachCriteria
 * @param   searchData - search result fill into this buffer
 * @param   srchDataLen - search result data length
 * @param   moreData - Indictaes that more search result was present or not as per number of record given in search criteria
 * @param   searchThreadInfo
 * @return  SUCCESS/FAIL
 */
static BOOL searchAllEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_RESULT_t *searchData, UINT16PTR srchDataLen,
                           UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    UINT8               channelCnt;
    time_t              stopTimeSec = 0;
    time_t              startTimeSec = 0;
    time_t              tempTimeSec = 0;
    SEARCH_DATA_t       *result = NULL;
    SEARCH_DATA_t       *tmpPtr;
    SEARCH_DATA_t       *srchInto = NULL;
    UINT16              resultLen, srchLen;
    SEARCH_CRITERIA_t   searchCmd = *pSeachCriteria;

    *srchDataLen = 0;
    srchInto = malloc(pSeachCriteria->noOfRecord * sizeof(SEARCH_DATA_t));
    if(srchInto == NULL)
    {
        return FAIL;
    }

    startTimeSec = searchStartTimePresence(&searchCmd);
    ConvertLocalTimeInSec(&searchCmd.endTime, &stopTimeSec);
    while(startTimeSec < stopTimeSec)
    {
        if((startTimeSec + SEC_IN_ONE_HOUR) < stopTimeSec)
        {
            ConvertLocalTimeInBrokenTm(&startTimeSec, &searchCmd.startTime);
            tempTimeSec = startTimeSec + (SEC_IN_ONE_MIN * (MIN_IN_ONE_HOUR - searchCmd.startTime.tm_min));
            ConvertLocalTimeInBrokenTm(&tempTimeSec, &searchCmd.endTime);
            startTimeSec = tempTimeSec;
        }
        else
        {
            ConvertLocalTimeInBrokenTm(&startTimeSec, &searchCmd.startTime);
            ConvertLocalTimeInBrokenTm(&stopTimeSec, &searchCmd.endTime);
            startTimeSec += SEC_IN_ONE_HOUR;
        }

        resultLen = 0;
        for (channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
        {
            searchCmd.channelNo = channelCnt + 1;
            srchLen = 0;
            if (searchCamAllEvent(&searchCmd, srchInto, &srchLen, moreData, searchThreadInfo) == REFUSE)
            {
                FREE_MEMORY(result);
                FREE_MEMORY(srchInto);
                *srchDataLen = 0;
                return REFUSE;
            }

            if (srchLen == 0)
            {
                continue;
            }

            tmpPtr = realloc(result, (resultLen + srchLen) * sizeof(SEARCH_DATA_t));
            if (tmpPtr == NULL)
            {
                FREE_MEMORY(srchInto);
                FREE_MEMORY(result);
                EPRINT(DISK_MANAGER, "memory rallocation failed: [camera=%d]", channelCnt);
                return FAIL;
            }

            result = tmpPtr;
            memcpy(result + resultLen, srchInto, (srchLen * sizeof(SEARCH_DATA_t)));
            resultLen += srchLen;
        }

        sortSearchRslt(result, searchData, resultLen, pSeachCriteria->noOfRecord, srchDataLen);
        FREE_MEMORY(result);

        if(*srchDataLen >= pSeachCriteria->noOfRecord)
        {
            *moreData = TRUE;
            break;
        }
    }

    FREE_MEMORY(srchInto);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sort event search result
 * @param   searchData
 * @param   searchRslt
 * @param   srchDataLen
 * @param   maxSortLen
 * @param   resltSort
 * @return  SUCCESS/FAIL
 */
static BOOL sortSearchRslt(SEARCH_DATA_t *searchData, SEARCH_RESULT_t *searchRslt, UINT16 srchDataLen, UINT16 maxSortLen, UINT16PTR resltSort)
{
    UINT16  searchCnt, index, lowestId = 0;
    time_t  startTime, defRefTime;

    /* Set max default value of time_t variable */
    memset(&defRefTime, 0xFF, sizeof(defRefTime));
    for (searchCnt = 0; searchCnt < srchDataLen; searchCnt++)
    {
        if(*resltSort >= maxSortLen)
        {
            break;
        }

        /* Init with default on starting of sort operation */
        startTime = defRefTime;
        for(index = 0; index < srchDataLen; index++)
        {
            if (searchData[index].startTime == defRefTime)
            {
                continue;
            }

            if ((startTime == defRefTime) || (startTime > searchData[index].startTime))
            {
                startTime = searchData[index].startTime;
                lowestId = index;
            }
        }

        ConvertLocalTimeInBrokenTm(&searchData[lowestId].startTime, &searchRslt[*resltSort].startTime);
        ConvertLocalTimeInBrokenTm(&searchData[lowestId].stopTime, &searchRslt[*resltSort].endTime);
        searchRslt[*resltSort].channelNo = searchData[lowestId].camNo;
        searchRslt[*resltSort].diskId = searchData[lowestId].diskId;
        searchRslt[*resltSort].recordType = searchData[lowestId].eventType;
        searchRslt[*resltSort].overlapFlg = searchData[lowestId].overlapFlg;
        searchRslt[*resltSort].recStorageType = searchData[lowestId].recStorageType;
        searchData[lowestId].startTime = defRefTime;
        (*resltSort)++;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkSearchRecordFolder
 * @param   diskCnt
 * @param   channelNo
 * @param   curTime
 * @param   searchDir
 * @param   searchRecStorageType
 * @return  SUCCESS/FAIL
 */
static BOOL checkSearchRecordFolder(UINT8 diskCnt, UINT8 channelNo, struct tm *curTime, CHARPTR searchDir, RECORD_ON_DISK_e	searchRecStorageType)
{
    CHAR mountPoint[MOUNT_POINT_SIZE];

    if (FAIL == GetMountPointFromDiskId(diskCnt, mountPoint, searchRecStorageType))
    {
        return FAIL;
    }

    // check for in in-active disk
    snprintf(searchDir, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mountPoint, channelNo,
             curTime->tm_mday, GetMonthName(curTime->tm_mon), curTime->tm_year, curTime->tm_hour);
    if (access(searchDir, F_OK) != STATUS_OK)
    {
        searchDir[0] = '\0';
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkSearchDateRecordFolder
 * @param   diskCnt
 * @param   channelNo
 * @param   curTime
 * @param   searchDir
 * @param   searchRecStorageType
 * @return  SUCCESS/FAIL
 */
static BOOL checkSearchDateRecordFolder(UINT8 diskCnt, UINT8 channelNo, struct tm *curTime, CHARPTR	searchDir, RECORD_ON_DISK_e searchRecStorageType)
{
    CHAR mountPoint[MOUNT_POINT_SIZE];

    if (FAIL == GetMountPointFromDiskId(diskCnt, mountPoint, searchRecStorageType))
    {
        return FAIL;
    }

    // check for in in-active disk
    snprintf(searchDir, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_FORMAT_SEP, mountPoint, channelNo,
             curTime->tm_mday, GetMonthName(curTime->tm_mon), curTime->tm_year);
    if(access(searchDir, F_OK) != STATUS_OK)
    {
        searchDir[0] = '\0';
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkSearchChnlRecordFolder
 * @param   diskCnt
 * @param   channelNo
 * @param   searchDir
 * @param   searchRecStorageType
 * @return  SUCCESS/FAIL
 */
static BOOL checkSearchChnlRecordFolder(UINT8 diskCnt, UINT8 channelNo, CHARPTR	searchDir,RECORD_ON_DISK_e searchRecStorageType)
{
    CHAR mountPoint[MOUNT_POINT_SIZE];

    if (FAIL == GetMountPointFromDiskId(diskCnt, mountPoint, searchRecStorageType))
    {
        return FAIL;
    }

    // check for in in-active disk
    snprintf(searchDir, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL, mountPoint, channelNo);
    if(access(searchDir, F_OK) != STATUS_OK)
    {
        searchDir[0] = '\0';
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was search recorded file based on particular event of particular camera number
 * @param   pSeachCriteria
 * @param   searchData - search result fill into this buffer
 * @param   srchDataLen - search result data length
 * @param   moreData - Indictaes that more search result was present or not as per number of record given in search criteria
 * @param   searchThreadInfo
 * @return  SUCCESS/FAIL
 */
static BOOL searchCamEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_DATA_t *searchData,
                           UINT16PTR srchDataLen, UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    BOOL                    diskStatus[MAX_RECORDING_MODE];
    UINT8                   recDriveId;
    CHAR                    searchDir[MAX_FILE_NAME_SIZE];
    CHAR                    evntFileName[MAX_FILE_NAME_SIZE];
    UINT16                  eventCnt = 0;
    INT32                   evntFileFd = INVALID_FILE_FD;
    INT32                   readCnt;
    UINT32                  curEventIdx = 0;
    UINT8                   diskCnt, totalDiskCnt;
    time_t                  startTimeSec = 0;
    time_t                  stopTimeSec = 0;
    time_t                  timeSec = 0;
    struct tm               evntStartTime;
    METADATA_FILE_HDR_t		eventFileInfo;
    EVNT_INFO_t 			eventInfo;
    RECORD_ON_DISK_e        recDriveIndx;
    SEARCH_CRITERIA_t       searchForChannelDate = *pSeachCriteria;

    // convert event search criteria start time into time second because of comparing exact start time of event
    ConvertLocalTimeInSec(&pSeachCriteria->startTime, &startTimeSec);
    ConvertLocalTimeInSec(&pSeachCriteria->endTime, &stopTimeSec);

    evntStartTime = pSeachCriteria->startTime;
    evntStartTime.tm_min = 0;
    evntStartTime.tm_sec = 0;

    ConvertLocalTimeInSec(&evntStartTime, &timeSec);
    if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            diskStatus[recDriveId] = IsStorageOperationalForRead(recDriveId);
        }
    }

    if (FAIL == searchSingleChannelPresence(&searchForChannelDate))
    {
        return SUCCESS;
    }

    while(stopTimeSec > timeSec)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
            {
                recDriveIndx = recDriveId;
                if(FALSE == diskStatus[recDriveIndx])
                {
                    continue;
                }
            }
            else
            {
                recDriveIndx = pSeachCriteria->searchRecStorageType;
            }

            if(recDriveId != 0)
            {
                if(GetCurrentNddStatus(recDriveId) == FALSE)
                {
                    continue;
                }
            }

            totalDiskCnt = GetTotalMediaNo(recDriveIndx);
            /* PARASOFT : No need to validate tainted data */
            for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
            {
                if(checkSearchRecordFolder(diskCnt, pSeachCriteria->channelNo, &evntStartTime, searchDir, recDriveIndx) == FAIL)
                {
                    continue;
                }

                // Open event file
                snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s", searchDir, EVENT_FILE_NAME);
                evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(evntFileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                    continue;
                }

                // Read event file header. Get next event index number
                readCnt = read(evntFileFd, &eventFileInfo, EVENT_FILE_HDR_SIZE);
                if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                    close(evntFileFd);
                    continue;
                }

                // loop till end of file not received. here we check curEventPos +2 is greater than or equal to file
                // size because of end of file indicator takes 2 byte value
                curEventIdx  = 0;
                while(curEventIdx < eventFileInfo.nextMetaDataIdx)
                {
                    // Read event index information from current file position
                    if (read(evntFileFd, &eventInfo, EVNT_FIELD_SIZE) != EVNT_FIELD_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                        break;
                    }

                    // Event end time 0 means that record was currently running
                    if(eventInfo.endTime == 0)
                    {
                        eventInfo.endTime = sessionInfo[pSeachCriteria->channelNo-1].streamFileInfo.localTime.totalSec;
                    }

                    // Check second time of this event matches of start time search criteria
                    if (!(((time_t)eventInfo.endTime < startTimeSec) ||
                          (((time_t)eventInfo.startTime > stopTimeSec))) && (eventInfo.eventType == pSeachCriteria->eventType))
                    {
                        // update event detect counter
                        eventCnt = *srchDataLen + 1;

                        // And check that this count more than no of record than break from this loop
                        if(eventCnt > pSeachCriteria->noOfRecord)
                        {
                            *moreData = TRUE;
                            close(evntFileFd);
                            return SUCCESS;
                        }

                        // this event falls into search criteria, so we need to give it as output parameter
                        if (genSrchResult(&eventInfo, pSeachCriteria->channelNo, diskCnt, searchData, srchDataLen, recDriveIndx) == FAIL)
                        {
                            close(evntFileFd);
                            EPRINT(DISK_MANAGER, "search result not generated: [camera=%d], [path=%s]", pSeachCriteria->channelNo-1, evntFileName);
                            return FAIL;
                        }
                    }

                    //set current event position
                    curEventIdx++;
                }

                close(evntFileFd);
            }

            if(pSeachCriteria->searchRecStorageType != ALL_DRIVE)
            {
                break;
            }
        }

        // Directory of that date and hour was not present means recording was not started in this time zone.
        // So we need to move to another hour of that date to search record.
        // Add one minute into this second variable to increments one hour in time
        timeSec += SEC_IN_ONE_HOUR;
        ConvertLocalTimeInBrokenTm(&timeSec, &evntStartTime);

        if ((searchThreadInfo != NULL) && (searchThreadInfo->clientCbType == CLIENT_CB_TYPE_NATIVE)
                && (CheckSocketFdState(searchThreadInfo->clientSocket) == FALSE))
        {
            *srchDataLen = 0;
            return REFUSE;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was search recorded file based on particular event of all camera number
 * @param   pSeachCriteria
 * @param   searchData - search result fill into this buffer
 * @param   srchDataLen - search result data length
 * @param   moreData - Indictaes that more search result was present or not as per number of record given in search criteria
 * @param   searchThreadInfo
 * @return  SUCCESS/FAIL
 */
static BOOL searchAllCamEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_RESULT_t *searchData,
                              UINT16PTR srchDataLen, UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    UINT8               channelCnt;
    time_t              stopTimeSec = 0;
    time_t              startTimeSec = 0;
    time_t              tempTimeSec = 0;
    SEARCH_DATA_t       *result = NULL;
    SEARCH_DATA_t       *tmpPtr;
    SEARCH_DATA_t       *srchInto = NULL;
    UINT16              resultLen = 0, srchLen = 0;
    SEARCH_CRITERIA_t   searchCmd = *pSeachCriteria;

    *srchDataLen = 0;
    srchInto = malloc(pSeachCriteria->noOfRecord * sizeof(SEARCH_DATA_t));
    if(srchInto == NULL)
    {
        return FAIL;
    }

    startTimeSec = searchStartTimePresence(&searchCmd);
    ConvertLocalTimeInSec(&searchCmd.endTime, &stopTimeSec);
    while(startTimeSec < stopTimeSec)
    {
        if((startTimeSec + SEC_IN_ONE_HOUR) < stopTimeSec)
        {
            ConvertLocalTimeInBrokenTm(&startTimeSec, &searchCmd.startTime);
            tempTimeSec = startTimeSec + (SEC_IN_ONE_MIN * (MIN_IN_ONE_HOUR - searchCmd.startTime.tm_min));
            ConvertLocalTimeInBrokenTm(&tempTimeSec, &searchCmd.endTime);
            startTimeSec = tempTimeSec;
        }
        else
        {
            ConvertLocalTimeInBrokenTm(&startTimeSec, &searchCmd.startTime);
            ConvertLocalTimeInBrokenTm(&stopTimeSec, &searchCmd.endTime);
            startTimeSec += SEC_IN_ONE_HOUR;
        }

        for (channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
        {
            searchCmd.channelNo = channelCnt + 1;
            srchLen = 0;
            if(searchCamEvent(&searchCmd, srchInto, &srchLen, moreData, searchThreadInfo) == REFUSE)
            {
                FREE_MEMORY(result);
                FREE_MEMORY(srchInto);
                *srchDataLen = 0;
                return REFUSE;
            }

            if (srchLen == 0)
            {
                continue;
            }

            tmpPtr = realloc(result, (resultLen + srchLen) * sizeof(SEARCH_DATA_t));
            if(tmpPtr == NULL)
            {
                FREE_MEMORY(result);
                FREE_MEMORY(srchInto);
                EPRINT(DISK_MANAGER, "memory reallocation failed: [camera=%d]", channelCnt);
                return FAIL;
            }

            result = tmpPtr;
            memcpy(result + resultLen, srchInto, (srchLen * sizeof(SEARCH_DATA_t)));
            resultLen += srchLen;
        }

        sortSearchRslt(result, searchData, resultLen, pSeachCriteria->noOfRecord, srchDataLen);
        resultLen = 0;
        FREE_MEMORY(result);

        if (*srchDataLen >= pSeachCriteria->noOfRecord)
        {
            *moreData = TRUE;
            break;
        }
    }

    FREE_MEMORY(srchInto);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was search recorded file based on all event of particular camera number
 * @param   pSeachCriteria
 * @param   searchData - search result fill into this buffer
 * @param   srchDataLen - search result data length
 * @param   moreData - Indictaes that more search result was present or not as per number of record given in search criteria
 * @param   searchThreadInfo
 * @return  SUCCESS/FAIL
 */
static BOOL searchCamAllEvent(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_DATA_t *searchData,
                              UINT16PTR srchDataLen, UINT8PTR moreData, SEARCH_THREAD_INFO_t *searchThreadInfo)
{
    BOOL                    diskStatus[MAX_RECORDING_MODE];
    UINT8                   index, recDriveId;
    UINT8                   serchEvntNo;
    CHAR                    searchDir[MAX_FILE_NAME_SIZE];
    CHAR                    evntFileName[MAX_FILE_NAME_SIZE];
    UINT16                  eventCnt = 0;
    INT32                   evntFileFd = INVALID_FILE_FD;
    INT32                   readCnt;
    UINT32                  curEventIdx = 0;
    RECORD_ON_DISK_e        recDriveIndx;
    time_t                  startTimeSec = 0;
    time_t                  stopTimeSec = 0;
    time_t                  timeSec = 0;
    UINT32                  diskCnt, totalDiskCnt;
    struct tm               evntStartTime;
    METADATA_FILE_HDR_t		eventFileInfo;
    EVNT_INFO_t 			eventInfo;
    EVNT_INFO_t				*tempSearchData;
    EVNT_INFO_t				*freeSearchData;
    SEARCH_CRITERIA_t 		searchForChannelDate = *pSeachCriteria;

    // convert event search criteria start time into time second because of comparing exact start time of event
    ConvertLocalTimeInSec(&pSeachCriteria->startTime, &startTimeSec);
    ConvertLocalTimeInSec(&pSeachCriteria->endTime, &stopTimeSec);

    evntStartTime = pSeachCriteria->startTime;
    evntStartTime.tm_min = 0;
    evntStartTime.tm_sec = 0;

    ConvertLocalTimeInSec(&evntStartTime, &timeSec);
    if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            diskStatus[recDriveId] = IsStorageOperationalForRead(recDriveId);
        }
    }

    if (FAIL == searchSingleChannelPresence(&searchForChannelDate))
    {
        return SUCCESS;
    }

    while(stopTimeSec > timeSec)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
            {
                recDriveIndx = recDriveId;
                if(FALSE == diskStatus[recDriveIndx])
                {
                    continue;
                }
            }
            else
            {
                recDriveIndx = pSeachCriteria->searchRecStorageType;
                recDriveId = pSeachCriteria->searchRecStorageType;	// Assign the drive id for proper Current sts validation
            }

            // if not local drive need to check status of network drive
            if(recDriveId != 0)
            {
                if(GetCurrentNddStatus(recDriveId) == FALSE)
                {
                    if(ALL_DRIVE == pSeachCriteria->searchRecStorageType)
                    {
                        continue;
                    }
                    else // In case of single storage search, do not continue
                    {
                        break;
                    }
                }
            }

            totalDiskCnt = GetTotalMediaNo(recDriveIndx);
            for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
            {
                if(checkSearchRecordFolder(diskCnt, pSeachCriteria->channelNo, &evntStartTime, searchDir, recDriveIndx) == FAIL)
                {
                    continue;
                }

                // Open event file
                snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s", searchDir, EVENT_FILE_NAME);
                evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(evntFileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                    continue;
                }

                // Read event file header. Get next event index number
                readCnt = read(evntFileFd, &eventFileInfo, EVENT_FILE_HDR_SIZE);
                if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                    close(evntFileFd);
                    continue;
                }

                tempSearchData = (EVNT_INFO_t *)malloc( sizeof(EVNT_INFO_t));
                if(tempSearchData == NULL)
                {
                    close(evntFileFd);
                    EPRINT(DISK_MANAGER, "memory allocation failed: [camera=%d]", pSeachCriteria->channelNo-1);
                    return FAIL;
                }

                index = 0;
                curEventIdx  = 0;
                memset(tempSearchData, 0, sizeof(EVNT_INFO_t));

                // loop till end of file not received. here we check curEventPos +2 is greater than or equal to file
                // size because of end of file indicator takes 2 byte value
                while(curEventIdx < eventFileInfo.nextMetaDataIdx)
                {
                    // Read event index information from current file position
                    if(read(evntFileFd, &eventInfo, EVNT_FIELD_SIZE) != EVNT_FIELD_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                        break;
                    }

                    // Event end time 0 means that record was currently running
                    if(eventInfo.endTime == 0)
                    {
                        eventInfo.endTime = sessionInfo[pSeachCriteria->channelNo-1].streamFileInfo.localTime.totalSec;
                    }

                    // firt we need to make all continues event and check the event which is falls in search criteria
                    if(tempSearchData[index].startTime == 0)
                    {
                        memcpy(&tempSearchData[index], &eventInfo, EVNT_FIELD_SIZE);
                    }
                    else if((eventInfo.startTime <= tempSearchData[index].endTime) && (eventInfo.startTime >= tempSearchData[index].startTime)
                            && (eventInfo.overLapFlg == tempSearchData[index].overLapFlg) && (eventInfo.eventType != tempSearchData[index].eventType))
                    {
                        tempSearchData[index].eventType |= eventInfo.eventType;
                        if(tempSearchData[index].endTime < eventInfo.endTime)
                        {
                            tempSearchData[index].endTime = eventInfo.endTime;
                        }
                    }
                    else
                    {
                        freeSearchData = realloc(tempSearchData, ((index + 2) * sizeof(EVNT_INFO_t)));
                        if(freeSearchData != NULL)
                        {
                            tempSearchData = freeSearchData;
                            index++;
                            memcpy(&tempSearchData[index], &eventInfo, EVNT_FIELD_SIZE);
                        }
                        else
                        {
                            EPRINT(DISK_MANAGER, "memory re-allocation failed: [camera=%d]", pSeachCriteria->channelNo-1);
                            break;
                        }
                    }

                    //set current event position
                    curEventIdx++;
                }

                /* Close file */
                close(evntFileFd);

                for(serchEvntNo = 0; serchEvntNo <= index; serchEvntNo++)
                {
                    if (tempSearchData[serchEvntNo].startTime == 0)
                    {
                        continue;
                    }

                    // Check second time of this event matches of start time search criteria
                    if (((time_t)tempSearchData[serchEvntNo].endTime < startTimeSec) || ((time_t)tempSearchData[serchEvntNo].startTime > stopTimeSec))
                    {
                        continue;
                    }

                    // update event detect counter
                    eventCnt = *srchDataLen + 1;

                    // And check that this count more than no of record than break from this loop
                    if(eventCnt > pSeachCriteria->noOfRecord)
                    {
                        *moreData = TRUE;
                        free(tempSearchData);
                        return SUCCESS;
                    }

                    // this event falls into search criteria, so we need to give it as output parameter
                    if(genSrchResult(&tempSearchData[serchEvntNo], pSeachCriteria->channelNo, diskCnt, searchData, srchDataLen, recDriveIndx) == FAIL)
                    {
                        free(tempSearchData);
                        EPRINT(DISK_MANAGER, "search result not generated: [camera=%d]", pSeachCriteria->channelNo-1);
                        return FAIL;
                    }
                }

                free(tempSearchData);
            }

            if(pSeachCriteria->searchRecStorageType != ALL_DRIVE)
            {
                break;
            }
        }

        // Directory of that date and hour was not present means recording was not started in this time zone.
        // So we need to move to another hour of that date to search record.
        // Add one minute into this second variable to increments one hour in time
        timeSec += SEC_IN_ONE_HOUR;
        ConvertLocalTimeInBrokenTm(&timeSec, &evntStartTime);

        if ((searchThreadInfo != NULL) && (searchThreadInfo->clientCbType == CLIENT_CB_TYPE_NATIVE)
                && (CheckSocketFdState(searchThreadInfo->clientSocket) == FALSE))
        {
            *srchDataLen = 0;
            return REFUSE;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was search recorded file based on all event of particular camera number
 * @param   pSeachCriteria
 * @param   searchData - search result fill into this buffer
 * @param   srchDataLen - search result data length
 * @param   moreData - Indictaes that more search result was present or not as per number of record given in search criteria
 * @param   overlapIndicator
 * @return  SUCCESS/FAIL
 */
BOOL SearchCamAllEventForSync(SEARCH_CRITERIA_t *pSeachCriteria, SEARCH_DATA_t *searchData,
                              UINT16PTR srchDataLen, UINT8PTR moreData, UINT8PTR overlapIndicator)
{
    BOOL                    diskStatus[MAX_RECORDING_MODE];
    UINT32                  index, recDriveId;
    UINT32                  serchEvntNo;
    CHAR                    searchDir[MAX_FILE_NAME_SIZE];
    CHAR                    evntFileName[MAX_FILE_NAME_SIZE];
    UINT16                  eventCnt = 0;
    INT32                   evntFileFd = INVALID_FILE_FD;
    INT32                   readCnt;
    UINT32                  curEventIdx = 0;
    time_t                  startTimeSec = 0;
    time_t                  stopTimeSec = 0;
    time_t                  timeSec = 0;
    UINT32                  diskCnt, totalDiskCnt;
    struct tm               evntStartTime;
    RECORD_ON_DISK_e 		recDriveIndx;
    METADATA_FILE_HDR_t		eventFileInfo;
    EVNT_INFO_t 			eventInfo;
    EVNT_INFO_t				*tempSearchData;
    EVNT_INFO_t				*freeSearchData;

    if ((pSeachCriteria == NULL) || (searchData == NULL) || (srchDataLen == NULL) || (moreData == NULL))
    {
        EPRINT(DISK_MANAGER, "null pointer found");
        return FAIL;
    }

    // convert event search criteria start time into time second because of comparing exact start time of event
    ConvertLocalTimeInSec(&pSeachCriteria->startTime, &startTimeSec);
    ConvertLocalTimeInSec(&pSeachCriteria->endTime, &stopTimeSec);
    *srchDataLen = 0;
    *overlapIndicator = FALSE;

    evntStartTime = pSeachCriteria->startTime;
    evntStartTime.tm_min = 0;
    evntStartTime.tm_sec = 0;
    ConvertLocalTimeInSec(&evntStartTime, &timeSec);

    /* get health status of recording drive */
    if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            diskStatus[recDriveId] = IsStorageOperationalForRead(recDriveId);
        }
    }

    // Compare two string till result not get equal to or less than 0
    while(stopTimeSec > timeSec)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
            {
                recDriveIndx = recDriveId;
                if(diskStatus[recDriveIndx] != TRUE)
                {
                    continue;
                }
            }
            else
            {
                recDriveIndx = pSeachCriteria->searchRecStorageType;
            }

            totalDiskCnt = GetTotalMediaNo(recDriveIndx);
            for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
            {
                if(checkSearchRecordFolder(diskCnt, pSeachCriteria->channelNo, &evntStartTime, searchDir, recDriveIndx) == FAIL)
                {
                    continue;
                }

                // Open event file
                snprintf(evntFileName, MAX_FILE_NAME_SIZE, "%s%s", searchDir, EVENT_FILE_NAME);
                evntFileFd = open(evntFileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(evntFileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                    continue;
                }

                // Read event file header. Get next event index number
                readCnt = read(evntFileFd, &eventFileInfo, EVENT_FILE_HDR_SIZE);
                if(readCnt != (INT32)EVENT_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                    close(evntFileFd);
                    continue;
                }

                tempSearchData = (EVNT_INFO_t *)malloc( sizeof(EVNT_INFO_t));
                if(tempSearchData == NULL)
                {
                    close(evntFileFd);
                    EPRINT(DISK_MANAGER, "memory allocation failed: [camera=%d]", pSeachCriteria->channelNo-1);
                    return FAIL;
                }

                index = 0;
                curEventIdx  = 0;
                memset(tempSearchData, 0, sizeof(EVNT_INFO_t));

                // loop till end of file not received. here we check curEventPos +2 is greater than or equal to file
                // size because of end of file indicator takes 2 byte value
                while(curEventIdx < eventFileInfo.nextMetaDataIdx)
                {
                    // Read event index information from current file position
                    if(read(evntFileFd, &eventInfo, EVNT_FIELD_SIZE) != EVNT_FIELD_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read event file: [camera=%d], [path=%s], [err=%s]", pSeachCriteria->channelNo-1, evntFileName, STR_ERR);
                        break;
                    }

                    // Event end time 0 means that record was currently running
                    if(eventInfo.endTime == 0)
                    {
                        eventInfo.endTime = sessionInfo[pSeachCriteria->channelNo-1].streamFileInfo.localTime.totalSec;
                    }

                    // firt we need to make all continues event and check the event which is falls in search criteria
                    if ((tempSearchData[index].startTime == 0) && ((pSeachCriteria->eventType & eventInfo.eventType) == eventInfo.eventType))
                    {
                        memcpy(&tempSearchData[index], &eventInfo, EVNT_FIELD_SIZE);
                    }
                    else if((eventInfo.startTime <= tempSearchData[index].endTime) && (eventInfo.startTime >= tempSearchData[index].startTime)
                            && (eventInfo.overLapFlg == 0) && (eventInfo.eventType != tempSearchData[index].eventType)
                            && ((pSeachCriteria->eventType & eventInfo.eventType) == eventInfo.eventType))
                    {
                        tempSearchData[index].eventType |= eventInfo.eventType;
                        if(tempSearchData[index].endTime < eventInfo.endTime)
                        {
                            tempSearchData[index].endTime = eventInfo.endTime;
                        }
                    }
                    else if((pSeachCriteria->eventType & eventInfo.eventType) == eventInfo.eventType)
                    {
                        freeSearchData = realloc( tempSearchData,((index + 2) * sizeof(EVNT_INFO_t)));
                        if(freeSearchData != NULL)
                        {
                            tempSearchData = freeSearchData;
                            index++;
                            memcpy(&tempSearchData[index], &eventInfo, EVNT_FIELD_SIZE);

                            if(eventInfo.overLapFlg != FALSE)
                            {
                                *overlapIndicator = TRUE;
                            }
                        }
                        else
                        {
                            EPRINT(DISK_MANAGER, "memory re-allocation failed: [camera=%d]", pSeachCriteria->channelNo-1);
                            break;
                        }
                    }

                    //set current event position
                    curEventIdx++;
                }

                /* Close file */
                close(evntFileFd);

                for(serchEvntNo = 0; serchEvntNo <= index; serchEvntNo++)
                {
                    if (tempSearchData[serchEvntNo].startTime == 0)
                    {
                        continue;
                    }

                    // Check second time of this event matches of start time search criteria
                    if (((time_t)tempSearchData[serchEvntNo].endTime < startTimeSec) || ((time_t)tempSearchData[serchEvntNo].startTime > stopTimeSec))
                    {
                        continue;
                    }

                    // update event detect counter
                    eventCnt = *srchDataLen + 1;

                    // And check that this count more than no of record than break from this loop
                    if(eventCnt > pSeachCriteria->noOfRecord)
                    {
                        *moreData = TRUE;
                        free(tempSearchData);
                        return SUCCESS;
                    }

                    // this event falls into search criteria, so we need to give it as output parameter
                    if(genSrchResult(&tempSearchData[serchEvntNo], pSeachCriteria->channelNo, diskCnt, searchData, srchDataLen, recDriveIndx) == FAIL)
                    {
                        free(tempSearchData);
                        EPRINT(DISK_MANAGER, "search result not generated: [camera=%d]", pSeachCriteria->channelNo-1);
                        return FAIL;
                    }
                }

                free(tempSearchData);
            }

            if(pSeachCriteria->searchRecStorageType != ALL_DRIVE)
            {
                break;
            }
        }

        // Directory of that date and hour was not present means recording was not started in this time zone.
        // So we need to move to another hour of that date to search record.
        // Add one minute into this second variable to increments one hour in time
        timeSec += SEC_IN_ONE_HOUR;
        ConvertLocalTimeInBrokenTm(&timeSec, &evntStartTime);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was generate search result stream as command response nedded when search request was added
 * @param   eventData - generate time for start and stop of event
 * @param   channelNo - for which channel result was generated
 * @param   diskId
 * @param   searchData - search result fill into this buffer
 * @param   srchDataLen - search result data length
 * @param   storageDrive
 * @return  SUCCESS/FAIL
 */
static BOOL genSrchResult(EVNT_INFO_t *eventData, UINT8 channelNo, UINT8 diskId, SEARCH_DATA_t *searchData,
                          UINT16PTR srchDataLen, RECORD_ON_DISK_e storageDrive)
{
    UINT16      dataLen = *srchDataLen;
    struct tm   startTm, endTm;

    // Set search result
    if(eventData->endTime != 0)
    {
        searchData[dataLen].stopTime = eventData->endTime;
    }
    else if(eventData->endTime > eventData->startTime)
    {
        time_t totalSec = sessionInfo[channelNo-1].streamFileInfo.localTime.totalSec;
        ConvertLocalTimeInBrokenTm(&totalSec, &startTm);
        ConvertLocalTimeInBrokenTm(&searchData[dataLen].startTime, &endTm);
        if(startTm.tm_hour != endTm.tm_hour)
        {
            return SUCCESS;
        }

        searchData[dataLen].stopTime = sessionInfo[channelNo-1].streamFileInfo.localTime.totalSec;
    }
    else
    {
        return SUCCESS;
    }

    searchData[dataLen].startTime = eventData->startTime;
    searchData[dataLen].camNo = channelNo;
    searchData[dataLen].eventType = eventData->eventType;
    searchData[dataLen].overlapFlg = eventData->overLapFlg;
    searchData[dataLen].diskId = diskId;
    searchData[dataLen].recStorageType = storageDrive;
    dataLen++ ;
    *srchDataLen = dataLen;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize all play session to free state.
 */
static void initPlaySession(void)
{
    UINT8 sessionIdx;

    // Set all sesion to intial state
    for (sessionIdx = 0; sessionIdx < MAX_PLAYBACK_DISK_SESSION; sessionIdx++)
    {
        playSession[sessionIdx].playStatus = OFF;
        playSession[sessionIdx].playBuff.buffPtr = NULL;
        playSession[sessionIdx].playBuff.buffSize = 0;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function deinitialize all play session to free state
 */
static void deInitPlaySession(void)
{
    UINT8 sessionIdx;

    // Set all sesion to intial state
    for (sessionIdx = 0; sessionIdx < MAX_PLAYBACK_DISK_SESSION; sessionIdx++)
    {
        if(playSession[sessionIdx].playStatus == ON)
        {
            playSession[sessionIdx].playStatus = OFF;
            CloseFileFd(&playSession[sessionIdx].playInfo.streamFileFd);
            CloseFileFd(&playSession[sessionIdx].playInfo.iFrmFileFd);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function give recovery status
 * @return  recovery status
 */
BOOL GetRecoveryStatus(void)
{
    MUTEX_LOCK(recoveryMutex);
    BOOL status = recoveryStatus;
    MUTEX_UNLOCK(recoveryMutex);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set recovery status
 * @param   status
 */
void SetRecoveryStatus(BOOL status)
{
    MUTEX_LOCK(recoveryMutex);
    recoveryStatus = status;
    MUTEX_UNLOCK(recoveryMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function give recording status of each channel.
 * @return
 */
void ChannelWriterExitWait(void)
{
    UINT8 channelNo;
    UINT8 retryCnt = 0;
    UINT8 writerChannelMax = getMaxCameraForCurrentVariant();

    while(TRUE)
    {
        /* Check all channels are completed or not */
        MUTEX_LOCK(dmCondMutex);
        for (channelNo = 0; channelNo < writerChannelMax; channelNo++)
        {
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            if (TRUE == channelWriterStatus[channelNo])
            {
                break;
            }
        }
        MUTEX_UNLOCK(dmCondMutex);

        /* Is channel writer still busy? */
        if (channelNo >= writerChannelMax)
        {
            DPRINT(DISK_MANAGER, "writter thread completed: [retryCnt=%d]", retryCnt);
            break;
        }

        /* Wait for some time to complete */
        retryCnt++;
        if (retryCnt > 30)
        {
            DPRINT(DISK_MANAGER, "writter thread still not completed, so giving up: [channelNo=%d], [retryCnt=%d]", channelNo, retryCnt);
            break;
        }

        /* Give some time and check again and repeat till timeout */
        DPRINT(DISK_MANAGER, "wait for writter thread to complete: [channelNo=%d], [retryCnt=%d]", channelNo, retryCnt);
        sleep(1);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function give AVI file writter status
 * @return
 */
BOOL GetAviWritterStatus(void)
{
    BOOL status = SUCCESS;

    MUTEX_LOCK(aviConvertParam.aviCnvrtCondMutex);
    if (aviConvertParam.readIdx != aviConvertParam.writeIdx)
    {
        status = FAIL;
    }
    MUTEX_UNLOCK(aviConvertParam.aviCnvrtCondMutex);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function resets all buffer of each channel.
 * @param   channelNo
 */
void ResetChannelBuffer(UINT8 channelNo)
{
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    DM_SESSION_t *dmSession = &sessionInfo[channelNo];
    dmSession->mDmBufferMngr.streamFileName[0] = '\0';
    dmSession->mDmBufferMngr.streamFileFd = INVALID_FILE_FD;
    dmSession->mDmBufferMngr.streamData[0] = '\0';
    dmSession->mDmBufferMngr.streamOffset = 0;
    dmSession->mDmBufferMngr.streamFileClose = FALSE;

    dmSession->mDmBufferMngr.iFrmFileFd = INVALID_FILE_FD;
    dmSession->mDmBufferMngr.iFrameData[0] = '\0';
    dmSession->mDmBufferMngr.iFrameOffset = 0;
    dmSession->mDmBufferMngr.iFrameFileClose = FALSE;

    dmSession->mDmBufferMngr.prevStreamPos = 0;
    dmSession->mDmBufferMngr.nextStreamPos = 0;
    dmSession->mDmBufferMngr.nextIFrameIdx = 0;
    memset(&dmSession->mDmBufferMngr.currTime, 0, sizeof(struct tm));
    FREE_MEMORY(dmSession->frameData);
    dmSession->frameLen = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was gives back up of native format of NVR system for whole hour as given
 *          in input to given destination drive. This function call was blocking so external module
 *          should take care while calling this function.
 * @param   cameraMask
 * @param   hourTime - hour time with date for which backup should be taken
 * @param   backupType
 * @param   backupDevice
 * @param   callback
 * @param   userData
 * @return  DM_BACKUP_STATUS_e
 */
DM_BACKUP_STATUS_e BackupToMedia(CAMERA_BIT_MASK_t cameraMask, time_t hourTime, DM_BACKUP_TYPE_e backupType,
                                 BACKUP_DEVICE_e backupDevice, BKUP_ABORT_CB callback, UINT32 userData)
{
    BOOL                    status;
    UINT8                   mediaNo;
    UINT8                   channelNo;
    DM_BACKUP_STATUS_e      backupStatus = BACKUP_SUCCESS;
    CHAR                    hourFolder[MAX_FILE_NAME_SIZE];
    CHAR                    srcMountPoint[MOUNT_POINT_SIZE];
    CHAR                    dstMountPoint[MOUNT_POINT_SIZE] = "\0";
    CHAR                    mntPoint[MOUNT_POINT_SIZE] = "\0";
    UINT64                  folderSize;
    UINT8                   diskCnt, totalDiskCnt;
    struct tm               brokenTime = { 0 };
    DISK_SIZE_t             diskSize;
    BOOL                    cameraBackupStatusF[MAX_CAMERA];
    UINT32                  cameraBackupDiskId[MAX_CAMERA];
    UINT8                   bkpTaskId, bkpTaskCnt = 0, tempTaskId;
    BACKUP_FILE_XFER_INFO_t fileXferInfo[BACKUP_TASK_MAX];

    /* Is any recording storage available for read? */
    if (FALSE == IsStorageOperationalForRead(MAX_RECORDING_MODE))
    {
        EPRINT(DISK_MANAGER, "backup not possible due to storage media fault");
        return BACKUP_NO_OPERATION_HDD;
    }

    /* Set backup media mount path */
    if(backupDevice == BACKUP_ON_USB)
    {
        /* Set usb as backup media */
        mediaNo = backupType + MANUAL_BACKUP_DISK;
        status = GetMountPointForBackupDevice(mediaNo, dstMountPoint);
        snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", dstMountPoint);
    }
    else
    {
        /* Set nas as backup media */
        mediaNo = backupDevice - BACKUP_ON_NAS1;
        status = GetMountPointFromNetworkDiskId(mediaNo, mntPoint);
        if (status == SUCCESS)
        {
            status = GetPathOfNetworkDrive(mediaNo, MATRIX_BACKUP_DIR, dstMountPoint);
        }
    }

    /* Destination media is not available */
    if(status == FAIL)
    {
        EPRINT(DISK_MANAGER, "proper mount point not present for backup device");
        return BACKUP_FAIL;
    }

    /* Convert local time in broken time */
    if(SUCCESS != ConvertLocalTimeInBrokenTm(&hourTime, &brokenTime))
    {
        EPRINT(DISK_MANAGER, "fail to get converted local time");
    }

    /* Get total disk of recording media */
    totalDiskCnt = GetTotalMediaNo(MAX_RECORDING_MODE);

    /* Check all camera to take backup */
    for (channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        /* Assume camera backup and disk are not available */
        cameraBackupStatusF[channelNo] = FALSE;
        cameraBackupDiskId[channelNo] = 0;

        /* Is camera backup enabled in configuration? */
        if (FALSE == GET_CAMERA_MASK_BIT(cameraMask, channelNo))
        {
            continue;
        }

        /* Check camera recording in all disks */
        for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
        {
            /* Is disk available for backup? */
            if(GetMountPointFromDiskId(diskCnt, srcMountPoint, MAX_RECORDING_MODE) == FAIL)
            {
                continue;
            }

            /* Prepare hour folder path to check whether folder is available or not on the disk */
            snprintf(hourFolder, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, srcMountPoint,
                     GET_CAMERA_NO(channelNo), brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour);
            if (access(hourFolder, F_OK) != STATUS_OK)
            {
                DPRINT(DISK_MANAGER, "hour folder not present: [camera=%d], [path=%s]", channelNo, hourFolder);
                continue;
            }

            /* Get size of that hour folder */
            if(GetFolderSize(hourFolder, &folderSize) == FAIL)
            {
                EPRINT(DISK_MANAGER, "hour folder size not found: [camera=%d], [path=%s]", channelNo, hourFolder);
                return BACKUP_FAIL;
            }

            /* Get the size of backup media */
            if(GetSizeOfMountFs(mntPoint, &diskSize) == FAIL)
            {
                EPRINT(DISK_MANAGER, "disk size not proper: [camera=%d], [mntPoint=%s]", channelNo, mntPoint);
                return BACKUP_FAIL;
            }

            /* Check sufficient size was available to hold the complete hour folder */
            if((folderSize / KILO_BYTE) > ((UINT64)diskSize.freeSize * KILO_BYTE))
            {
                EPRINT(DISK_MANAGER, "backup disk has not sufficient size: [camera=%d]", channelNo);
                return BACKUP_DISK_FULL;
            }

            /* Set Disk Id for backup. Recording can be in multiple disks */
            cameraBackupStatusF[channelNo] = TRUE;
            SET_BIT(cameraBackupDiskId[channelNo], diskCnt);
        }
    }

    /* Init backup task information */
    for (bkpTaskId = 0; bkpTaskId < BACKUP_TASK_MAX; bkpTaskId++)
    {
        /* Init dynamic information for task schedular */
        memset(fileXferInfo[bkpTaskId].cameraBackupStatusF, FALSE, sizeof(fileXferInfo[bkpTaskId].cameraBackupStatusF));
        RESET_STR_BUFF(fileXferInfo[bkpTaskId].srcMountPoint);
        MUTEX_INIT(fileXferInfo[bkpTaskId].backupTaskMutex, NULL);
        fileXferInfo[bkpTaskId].backupTaskRunStatus = FALSE;
        fileXferInfo[bkpTaskId].backupStatus = BACKUP_SUCCESS;

        /* Set fixed information for task schedular */
        fileXferInfo[bkpTaskId].schedularId = bkpTaskId;
        fileXferInfo[bkpTaskId].backupType = backupType;
        snprintf(fileXferInfo[bkpTaskId].mntPoint, MAX_FILE_NAME_SIZE, "%s", mntPoint);
        snprintf(fileXferInfo[bkpTaskId].dstMountPoint, MAX_FILE_NAME_SIZE, "%s", dstMountPoint);
        fileXferInfo[bkpTaskId].brokenTime = brokenTime;
        fileXferInfo[bkpTaskId].mediaNo = mediaNo;
        fileXferInfo[bkpTaskId].backupDevice = backupDevice;
        fileXferInfo[bkpTaskId].userData = userData;
        fileXferInfo[bkpTaskId].copyAbortCb = (COPY_ABORT_CB)callback;
    }

    /* Start backing up camera recording disk wise */
    bkpTaskId = bkpTaskCnt = 0;
    for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
    {
        /* Is disk mounted? */
        if (GetMountPointFromDiskId(diskCnt, srcMountPoint, MAX_RECORDING_MODE) == FAIL)
        {
            continue;
        }

        /* Get backup of current hour from current disk of all cameras */
        status = FALSE;
        for (channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
        {
            /* Assume no backup needed for current camera */
            fileXferInfo[bkpTaskId].cameraBackupStatusF[channelNo] = FALSE;

            /* Is camera backup needed and recorded in current disk? */
            if ((cameraBackupStatusF[channelNo] == FALSE) || (FALSE == GET_BIT(cameraBackupDiskId[channelNo], diskCnt)))
            {
                continue;
            }

            /* We are going to take backup, mark it */
            status = TRUE;
            fileXferInfo[bkpTaskId].cameraBackupStatusF[channelNo] = TRUE;

            /* Is we have taken backup of camera from all disks? */
            RESET_BIT(cameraBackupDiskId[channelNo], diskCnt);
            if (cameraBackupDiskId[channelNo] == 0)
            {
                cameraBackupStatusF[channelNo] = FALSE;
            }
        }

        /* No backup available in current disk */
        if (status == FALSE)
        {
            continue;
        }

        /* Set source media mount path */
        snprintf(fileXferInfo[bkpTaskId].srcMountPoint, MAX_FILE_NAME_SIZE, "%s", srcMountPoint);

        /* Start camera hour backup thread */
        MUTEX_LOCK(fileXferInfo[bkpTaskId].backupTaskMutex);
        fileXferInfo[bkpTaskId].backupTaskRunStatus = TRUE;
        fileXferInfo[bkpTaskId].backupStatus = BACKUP_SUCCESS;
        MUTEX_UNLOCK(fileXferInfo[bkpTaskId].backupTaskMutex);

        /* Creates a thread for backing up the recording data of same disk */
        if (FAIL == Utils_CreateThread(NULL, backupToMediaThread, &fileXferInfo[bkpTaskId], DETACHED_THREAD, BACKUP_THREAD_STACK_SZ))
        {
            EPRINT(DISK_MANAGER, "fail to create writterThread");
            MUTEX_LOCK(fileXferInfo[bkpTaskId].backupTaskMutex);
            fileXferInfo[bkpTaskId].backupTaskRunStatus = FALSE;
            fileXferInfo[bkpTaskId].backupStatus = BACKUP_FAIL;
            MUTEX_UNLOCK(fileXferInfo[bkpTaskId].backupTaskMutex);
            backupStatus = BACKUP_FAIL;
            break;
        }

        /* We have started a new schedular for backup */
        backupStatus = BACKUP_SUCCESS;
        bkpTaskCnt++;
        bkpTaskId = bkpTaskCnt;

        /* Have we started maximum schedulars? */
        if (bkpTaskId < BACKUP_TASK_MAX)
        {
            continue;
        }

        /* Check all schedulars status */
        while(TRUE)
        {
            for (tempTaskId = 0; tempTaskId < BACKUP_TASK_MAX; tempTaskId++)
            {
                /* Is any schedular exited? */
                MUTEX_LOCK(fileXferInfo[tempTaskId].backupTaskMutex);
                if (FALSE == fileXferInfo[tempTaskId].backupTaskRunStatus)
                {
                    /* We will start reassignment from 1st if all task schedular available */
                    if (bkpTaskId >= BACKUP_TASK_MAX)
                    {
                        /* Start this schedular with new backup info */
                        bkpTaskId = tempTaskId;
                    }

                    /* Is last task executed successfully? */
                    if (fileXferInfo[tempTaskId].backupStatus != BACKUP_SUCCESS)
                    {
                        backupStatus = fileXferInfo[tempTaskId].backupStatus;
                    }
                }
                MUTEX_UNLOCK(fileXferInfo[tempTaskId].backupTaskMutex);
            }

            /* Is error found in last execution? */
            if (backupStatus != BACKUP_SUCCESS)
            {
                break;
            }

            /* Is free task schedular available? */
            if (bkpTaskId < BACKUP_TASK_MAX)
            {
                bkpTaskCnt--;
                break;
            }

            /* Wait to free task schedular */
            sleep(1);
        }

        /* Is error found in last execution? */
        if (backupStatus != BACKUP_SUCCESS)
        {
            break;
        }
    }

    /* Before exiting, check status of all schedular */
    while(TRUE)
    {
        bkpTaskCnt = 0;
        for (bkpTaskId = 0; bkpTaskId < BACKUP_TASK_MAX; bkpTaskId++)
        {
            /* Is schedular exited? */
            MUTEX_LOCK(fileXferInfo[bkpTaskId].backupTaskMutex);
            if (FALSE == fileXferInfo[bkpTaskId].backupTaskRunStatus)
            {
                /* Get last status of backup schedular */
                bkpTaskCnt++;
                if ((backupStatus == BACKUP_SUCCESS) && (fileXferInfo[bkpTaskId].backupStatus != BACKUP_SUCCESS))
                {
                    backupStatus = fileXferInfo[bkpTaskId].backupStatus;
                }
            }
            MUTEX_UNLOCK(fileXferInfo[bkpTaskId].backupTaskMutex);
        }

        /* Are all schedulars exited? */
        if (bkpTaskCnt >= BACKUP_TASK_MAX)
        {
            break;
        }

        /* Wait to free task schedular */
        sleep(1);
    }

    /* Free local mutex memory */
    for (bkpTaskId = 0; bkpTaskId < BACKUP_TASK_MAX; bkpTaskId++)
    {
        pthread_mutex_destroy(&fileXferInfo[bkpTaskId].backupTaskMutex);
    }

    /* Return backup status */
    return backupStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Backup to media thread task handler
 * @param   threadArg - Pointer to backup file transfer info
 */
static void *backupToMediaThread(void *threadArg)
{
    UINT8                   channelNo;
    BOOL                    copyMataData = NO;
    UINT8                   metaDataCnt;
    DM_BACKUP_STATUS_e      backupStatus = BACKUP_SUCCESS;
    CHAR                    hourFolder[MAX_FILE_NAME_SIZE];
    CHAR                    destFolder[MAX_FILE_NAME_SIZE];
    CHAR                    destFile[MAX_FILE_NAME_SIZE];
    CHAR                    strmFileName[MAX_FILE_NAME_SIZE];
    CHAR                    diskName[MOUNT_POINT_SIZE];
    CHARPTR                 tmpStrmFileName;
    UINT32                  strmFileCnt, streamFileId;
    INT32                   strmFileFd;
    UINT32                  strmFileSize;
    UINT64                  folderSize;
    DISK_SIZE_t             diskSize;
    STRM_FILE_HDR_t         strmFileHdr;
    DIR                     *dir;
    struct dirent           *entry;
    BACKUP_FILE_XFER_INFO_t *pXferInfo = (BACKUP_FILE_XFER_INFO_t*)threadArg;

    /* Set thread name */
    THREAD_START_INDEX("BACKUP_TASK", pXferInfo->schedularId);

    /* Check backup of all cameras */
    for (channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        /* Is camera backup available in current disk? */
        if (pXferInfo->cameraBackupStatusF[channelNo] == FALSE)
        {
            continue;
        }

        /* Prepare hour folder path to check whether folder is available or not on the disk */
        snprintf(hourFolder, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, pXferInfo->srcMountPoint,
                 GET_CAMERA_NO(channelNo), pXferInfo->brokenTime.tm_mday, GetMonthName(pXferInfo->brokenTime.tm_mon),
                 pXferInfo->brokenTime.tm_year, pXferInfo->brokenTime.tm_hour);
        if (access(hourFolder, F_OK) != STATUS_OK)
        {
            DPRINT(DISK_MANAGER, "hour folder not present: [camera=%d], [path=%s]", channelNo, hourFolder);
            continue;
        }

        /* Get size of that hour folder */
        if(GetFolderSize(hourFolder, &folderSize) == FAIL)
        {
            EPRINT(DISK_MANAGER, "hour folder size not found: [camera=%d], [path=%s]", channelNo, hourFolder);
            backupStatus = BACKUP_FAIL;
            break;
        }

        /* Get the size of backup media */
        if(GetSizeOfMountFs(pXferInfo->mntPoint, &diskSize) == FAIL)
        {
            EPRINT(DISK_MANAGER, "disk size not proper: [camera=%d], [mntPoint=%s]", channelNo, pXferInfo->mntPoint);
            backupStatus = BACKUP_FAIL;
            break;
        }

        /* Check sufficient size was available to hold the complete hour folder */
        if((folderSize / KILO_BYTE) > ((UINT64)diskSize.freeSize * KILO_BYTE))
        {
            EPRINT(DISK_MANAGER, "backup disk has not sufficient size: [camera=%d]", channelNo);
            backupStatus = BACKUP_DISK_FULL;
            break;
        }

        /* Get disk name from mount path */
        if(GetDiskNameFromMountPoint(pXferInfo->srcMountPoint, diskName) == FAIL)
        {
            backupStatus = BACKUP_FAIL;
            break;
        }

        /* Prepare backup destination path */
        snprintf(destFolder, MAX_FILE_NAME_SIZE, BACKUP_REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, pXferInfo->dstMountPoint, diskName,
                 GET_CAMERA_NO(channelNo), pXferInfo->brokenTime.tm_mday, GetMonthName(pXferInfo->brokenTime.tm_mon),
                 pXferInfo->brokenTime.tm_year, pXferInfo->brokenTime.tm_hour);

        /* Take action based on recording format type */
        if((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) || (aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT))
        {
            /* Hour folder was not present in backup disk. Create same hour folder in backup disk */
            if(access(destFolder, F_OK) != STATUS_OK)
            {
                if (CreateRecursiveDir(destFolder, NULL) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to create backup folder: [camera=%d], [path=%s]", channelNo, destFolder);
                    backupStatus = BACKUP_FAIL;
                    break;
                }
            }

            /* Open hour folder */
            dir = opendir(hourFolder);
            if (dir == NULL)
            {
                break;
            }

            /* Read all files and folder of hour */
            while((entry = readdir(dir)) != NULL)
            {
                /* These are system file and that can not removed */
                if((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
                {
                    continue;
                }

                /* Prepare source stream file name */
                snprintf(strmFileName, MAX_FILE_NAME_SIZE, "%s%s", hourFolder, entry->d_name);

                /* Prepare destination stream file name */
                snprintf(destFile, MAX_FILE_NAME_SIZE, "%s%s", destFolder, entry->d_name);

                /* Copy file from source to destination */
                if(CopyFile(strmFileName, destFile, BACKUP_TASK_CHUNK_SLEEP_USEC, pXferInfo->copyAbortCb, pXferInfo->userData) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to copy stream file in backup media: [camera=%d], [path=%s]", channelNo, strmFileName);
                    closedir(dir);
                    backupStatus = BACKUP_FAIL;
                    break;
                }
            }

            /* Close the folder */
            closedir(dir);

            /* Is last copy task failed? */
            if (backupStatus != BACKUP_SUCCESS)
            {
                break;
            }
        }
        else
        {
            /* Get stream file count in hour folder */
            if(getAllStreamFileCount(hourFolder, &strmFileCnt, NULL) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to get stream file count: [camera=%d]", channelNo);
                backupStatus = BACKUP_FAIL;
                break;
            }

            /* Assume, metadata copy is not required */
            copyMataData = NO;

            /* Copy all stream file */
            for(streamFileId = 1; streamFileId <= strmFileCnt; streamFileId++)
            {
                /* Get stream file name */
                if(getStreamFileName(hourFolder, streamFileId, strmFileName, &strmFileSize) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "stream file not found: [camera=%d], [path=%s]", channelNo, hourFolder);
                    backupStatus = BACKUP_FAIL;
                    break;
                }

                /* Open the stream file */
                strmFileFd = open(strmFileName, READ_WRITE_MODE, FILE_PERMISSION);
                if(strmFileFd == INVALID_FILE_FD)
                {
                    EPRINT(DISK_MANAGER, "fail to open stream file: [camera=%d], [path=%s], [err=%s]", channelNo, strmFileName, STR_ERR);
                    backupStatus = BACKUP_FAIL;
                    break;
                }

                /* Read the stream file header */
                if(read(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE) != STREAM_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to read stream file: [camera=%d], [path=%s], [err=%s]", channelNo, strmFileName, STR_ERR);
                    close(strmFileFd);
                    backupStatus = BACKUP_FAIL;
                    break;
                }

                /* Is file backup already done for scheduled backup? */
                if ((pXferInfo->backupType == DM_SCHEDULE_BACKUP) && (strmFileHdr.backupFlg >> DM_SCHEDULE_BACKUP) != FALSE)
                {
                    DPRINT(DISK_MANAGER, "stream file already copied: [camera=%d], [path=%s]", channelNo, strmFileName);
                    close(strmFileFd);
                    continue;
                }

                /* hour folder was not present in backup disk. Create same hour folder in backup disk */
                if(access(destFolder, F_OK) != STATUS_OK)
                {
                    if (CreateRecursiveDir(destFolder, NULL) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to create backup folder: [camera=%d], [path=%s]", channelNo, destFolder);
                        close(strmFileFd);
                        backupStatus = BACKUP_FAIL;
                        break;
                    }
                }

                /* Make destination file name and remove '/' by incrementing one address */
                tmpStrmFileName = strrchr(strmFileName, '/');
                snprintf(destFile, MAX_FILE_NAME_SIZE, "%s%s", destFolder, (tmpStrmFileName + 1));

                /* Copy file from source to destination */
                if(CopyFile(strmFileName, destFile, BACKUP_TASK_CHUNK_SLEEP_USEC, pXferInfo->copyAbortCb, pXferInfo->userData) == FAIL)
                {
                    close(strmFileFd);
                    EPRINT(DISK_MANAGER, "fail to copy stream file in backup media: [camera=%d], [path=%s]", channelNo, strmFileName);
                    backupStatus = BACKUP_FAIL;
                    break;
                }

                /* Set the backup taken of this stream file with backupType */
                strmFileHdr.backupFlg |= (1 << pXferInfo->backupType);

                /* Write the stream file header with backup flag */
                if(pwrite(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, 0) != (INT32)STREAM_FILE_HDR_SIZE)
                {
                    EPRINT(DISK_MANAGER, "fail to write stream file header: [camera=%d], [path=%s], [err=%s]", channelNo, strmFileName, STR_ERR);
                    close(strmFileFd);
                    backupStatus = BACKUP_FAIL;
                    break;
                }

                /* Copy metadata */
                copyMataData = YES;
                close(strmFileFd);
            }

            /* Is last copy task failed? */
            if (backupStatus != BACKUP_SUCCESS)
            {
                break;
            }

            /* Need to copy metadata files? */
            if(copyMataData == YES)
            {
                /* Copy all metadata files */
                for(metaDataCnt = 0; metaDataCnt < MAX_METADAT_FILE; metaDataCnt++)
                {
                    /* Prepare source and destination metadata file names */
                    snprintf(strmFileName, MAX_FILE_NAME_SIZE, "%s%s", hourFolder, metaFiles[metaDataCnt]);
                    snprintf(destFile, MAX_FILE_NAME_SIZE, "%s%s", destFolder, metaFiles[metaDataCnt]);

                    /* Copy file from source to destination */
                    if(CopyFile(strmFileName, destFile, 2000, pXferInfo->copyAbortCb, pXferInfo->userData) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to copy meta files in backup media: [camera=%d], [path=%s]", channelNo, destFile);
                        backupStatus = BACKUP_FAIL;
                        break;
                    }
                }

                /* Is last copy task failed? */
                if (backupStatus != BACKUP_SUCCESS)
                {
                    break;
                }
            }

            /* Update backup media disk size */
            if(pXferInfo->backupDevice == BACKUP_ON_USB)
            {
                UpdateUsbDiskSize(pXferInfo->mediaNo);
            }
            else
            {
                UpdateNddDiskSize(pXferInfo->mediaNo);
            }
        }
    }

    /* Free backup task schedular */
    MUTEX_LOCK(pXferInfo->backupTaskMutex);
    pXferInfo->backupTaskRunStatus = FALSE;
    pXferInfo->backupStatus = backupStatus;
    MUTEX_UNLOCK(pXferInfo->backupTaskMutex);

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was gives back up of native format of NVR system for whole hour as given
 *          in input to given destination FTP server. This function call was blocking so external
 *          module should take care while calling this function.
 * @param   cameraMask
 * @param   hourTime - hour time with date for which backup should be taken
 * @param   backupType
 * @param   ftpServer
 * @param   callback
 * @param   userData
 * @return  DM_BACKUP_STATUS_e
 */
DM_BACKUP_STATUS_e BackupToFtp(CAMERA_BIT_MASK_t cameraMask, time_t	hourTime, DM_BACKUP_TYPE_e backupType,
                               FTP_SERVER_e ftpServer, BKUP_ABORT_CB callback, UINT32 userData)
{
    UINT8                   channelNo;
    BOOL                    copyMataData = NO;
    DM_BACKUP_STATUS_e      backupStatus = BACKUP_FAIL;
    CHAR                    hourFolder[MAX_FILE_NAME_SIZE];
    CHAR                    strmFileName[MAX_FILE_NAME_SIZE];
    CHAR                    srcMountPoint[MOUNT_POINT_SIZE];
    CHAR                    remoteFileName[MAX_FILE_NAME_SIZE];
    UINT32                  ftpData = 0;
    UINT32                  strmFileCnt, streamFileId;
    INT32                   strmFileFd;
    UINT32                  strmFileSize;
    UINT8                   diskCnt, totalDiskCnt;
    INT32                   timeWaitRet;
    FTP_HANDLE              ftpHandle;
    STRM_FILE_HDR_t         strmFileHdr;
    FTP_FILE_INFO_t         ftpFileInfo;
    struct tm               brokenTime = { 0 };
    struct timespec 		ts;
    DIR                     *dir;
    struct dirent           *entry;

    if (FALSE == IsStorageOperationalForRead(MAX_RECORDING_MODE))
    {
        EPRINT(DISK_MANAGER, "backup not possible due to storage media fault");
        return BACKUP_NO_OPERATION_HDD;
    }

    totalDiskCnt = GetTotalMediaNo(MAX_RECORDING_MODE);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        /* Check if backup is enabled or not for this channel */
        if (FALSE == GET_CAMERA_MASK_BIT(cameraMask, channelNo))
        {
            continue;
        }

        for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
        {
            if(GetMountPointFromDiskId(diskCnt, srcMountPoint,MAX_RECORDING_MODE) == FAIL)
            {
                continue;
            }

            // Compose hour folder name which is input
            if(SUCCESS != ConvertLocalTimeInBrokenTm(&hourTime, &brokenTime))
            {
                EPRINT(DISK_MANAGER, "fail to get converted local time: [camera=%d]", channelNo);
            }

            backupStatus = BACKUP_SUCCESS;
            snprintf(hourFolder, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, srcMountPoint,
                     GET_CAMERA_NO(channelNo), brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour);
            if(access(hourFolder, F_OK) != STATUS_OK)
            {
                DPRINT(DISK_MANAGER, "hour folder not present: [camera=%d], [path=%s]", channelNo, hourFolder);
                continue;
            }

            if((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) || (aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT))
            {
                dir = opendir(hourFolder);
                if (dir == NULL)
                {
                    continue;
                }

                while((entry = readdir(dir)) != NULL)
                {
                    // These are system file and that can not removed
                    if((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
                    {
                        continue;
                    }

                    // check this file was stream file
                    snprintf(strmFileName, MAX_FILE_NAME_SIZE, "%s%s", hourFolder, entry->d_name);

                    // copy local file name which is to be upload
                    snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s", strmFileName);

                    if(ParseRemoteStrmFileName(strmFileName, remoteFileName) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to parse remote stream file name: [camera=%d], [path=%s]", channelNo, strmFileName);
                        closedir(dir);
                        return BACKUP_FAIL;
                    }

                    snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "%s", remoteFileName);
                    ftpFileInfo.ftpServer = ftpServer;

                    // Request to upload file to FTP client
                    if(StartFtpUpload(&ftpFileInfo, (VOIDPTR)&backupFtpCallBack, ftpData, &ftpHandle) != CMD_SUCCESS)
                    {
                        EPRINT(DISK_MANAGER, "fail to upload file on ftp: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        closedir(dir);
                        return BACKUP_FAIL;
                    }

                    DPRINT(DISK_MANAGER, "ftp file upload success: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);

                    do
                    {
                        clock_gettime(CLOCK_REALTIME, &ts);
                        ts.tv_sec += BACKUP_STOP_TIMEOUT;

                        /* File was uploaded or not that was indicate by user callback so here we need to check already uploading
                         * was in process then no need to upload another file. */
                        MUTEX_LOCK(backupFtpMutex);
                        timeWaitRet = pthread_cond_timedwait(&backupFtpCond, &backupFtpMutex, &ts);
                        MUTEX_UNLOCK(backupFtpMutex);
                        if(timeWaitRet == STATUS_OK)
                        {
                            break;
                        }

                        if (callback == NULL)
                        {
                            continue;
                        }

                        if(callback(userData) == TRUE)
                        {
                            StopFtpTransfer(ftpHandle);
                            WPRINT(DISK_MANAGER, "user has aborted file upload on ftp: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        }

                    } while(TRUE);

                    if(ftpUploadResponse != FTP_SUCCESS)
                    {
                        EPRINT(DISK_MANAGER, "ftp connection error in file upload: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        closedir(dir);
                        return BACKUP_FTP_CONN_FAIL;
                    }
                }

                closedir(dir);
            }
            else
            {
                if(getAllStreamFileCount(hourFolder, &strmFileCnt, NULL) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to get stream file count: [camera=%d]", channelNo);
                    return BACKUP_FAIL;
                }

                copyMataData = NO;
                for(streamFileId = 1; streamFileId <= strmFileCnt; streamFileId++)
                {
                    if(getStreamFileName(hourFolder, streamFileId, strmFileName, &strmFileSize) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "stream file not found: [camera=%d], [path=%s]", channelNo, hourFolder);
                        return BACKUP_FAIL;
                    }

                    // here we need to check for configuration changed if config was changed then we need to the uploading data on FTP
                    // Open the stream file
                    strmFileFd = open(strmFileName, READ_WRITE_MODE, FILE_PERMISSION);
                    if(strmFileFd == INVALID_FILE_FD)
                    {
                        EPRINT(DISK_MANAGER, "fail to open stream file: [camera=%d], [path=%s], [err=%s]", channelNo, strmFileName, STR_ERR);
                        return BACKUP_FAIL;
                    }

                    // Read the stream file header
                    if(read(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE) != STREAM_FILE_HDR_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to read stream file: [camera=%d], [path=%s], [err=%s]", channelNo, strmFileName, STR_ERR);
                        return BACKUP_FAIL;
                    }

                    if((backupType == DM_SCHEDULE_BACKUP) && (strmFileHdr.backupFlg >> DM_SCHEDULE_BACKUP) != FALSE)
                    {
                        DPRINT(DISK_MANAGER, "stream file already copied: [camera=%d], [path=%s]", channelNo, strmFileName);
                        close(strmFileFd);
                        continue;
                    }

                    // copy local file name which is to be upload
                    snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s", strmFileName);
                    if(ParseRemoteStrmFileName(strmFileName, remoteFileName) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to parse remote stream file name: [camera=%d], [path=%s]", channelNo, strmFileName);
                        return BACKUP_FAIL;
                    }

                    snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "%s", remoteFileName);
                    ftpFileInfo.ftpServer = ftpServer;

                    // Request to upload file to FTP client
                    if(StartFtpUpload(&ftpFileInfo, (VOIDPTR)&backupFtpCallBack, 0, &ftpHandle) != CMD_SUCCESS)
                    {
                        close(strmFileFd);
                        EPRINT(DISK_MANAGER, "fail to upload file on ftp: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        return BACKUP_FAIL;
                    }

                    DPRINT(DISK_MANAGER, "ftp file upload success: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);

                    do
                    {
                        clock_gettime(CLOCK_REALTIME, &ts);
                        ts.tv_sec += BACKUP_STOP_TIMEOUT;

                        /* File was uploaded or not that was indicate by user callback so here we need to check already uploading
                         * was in process then no need to upload another file. */
                        MUTEX_LOCK(backupFtpMutex);
                        timeWaitRet = pthread_cond_timedwait(&backupFtpCond, &backupFtpMutex, &ts);
                        MUTEX_UNLOCK(backupFtpMutex);
                        if(timeWaitRet == STATUS_OK)
                        {
                            break;
                        }

                        if (callback == NULL)
                        {
                            continue;
                        }

                        if(callback(userData) == TRUE)
                        {
                            StopFtpTransfer(ftpHandle);
                            WPRINT(DISK_MANAGER, "user has aborted file upload on ftp: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        }

                    } while(TRUE);

                    if(ftpUploadResponse != FTP_SUCCESS)
                    {
                        close(strmFileFd);
                        EPRINT(DISK_MANAGER, "ftp connection error in file upload: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        return BACKUP_FTP_CONN_FAIL;
                    }

                    //Set the backup taken of this stream file with backupType
                    strmFileHdr.backupFlg |= (1 << backupType);

                    // Write the stream file header with backup flag
                    if(pwrite(strmFileFd, &strmFileHdr, STREAM_FILE_HDR_SIZE, 0) != STREAM_FILE_HDR_SIZE)
                    {
                        EPRINT(DISK_MANAGER, "fail to write stream file header: [camera=%d], [path=%s], [err=%s]", channelNo, strmFileName, STR_ERR);
                        close(strmFileFd);
                        return BACKUP_FAIL;
                    }

                    /* Copy metadata */
                    copyMataData = YES;
                    close(strmFileFd);
                }

                if (copyMataData == NO)
                {
                    continue;
                }

                for(streamFileId = 0; streamFileId < MAX_METADAT_FILE; streamFileId++)
                {
                    // copy local file name which is to be upload
                    snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s%s", hourFolder, metaFiles[streamFileId]);
                    if(ParseRemoteStrmFileName(ftpFileInfo.localFileName, remoteFileName) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to parse remote stream file name: [camera=%d], [path=%s]", channelNo, ftpFileInfo.localFileName);
                        return BACKUP_FAIL;
                    }

                    // generate remote file name
                    snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "%s", remoteFileName);
                    ftpFileInfo.ftpServer = ftpServer;

                    // Request to upload file to FTP client
                    if(StartFtpUpload(&ftpFileInfo, (VOIDPTR)&backupFtpCallBack, 0, &ftpHandle) != CMD_SUCCESS)
                    {
                        EPRINT(DISK_MANAGER, "fail to upload meta files in backup media: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        return BACKUP_FAIL;
                    }

                    DPRINT(DISK_MANAGER, "meta files uploaded successfully: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);

                    do
                    {
                        clock_gettime(CLOCK_REALTIME, &ts);
                        ts.tv_sec += BACKUP_STOP_TIMEOUT;

                        /* File was uploaded or not that was indicate by user callback so here we need to check already uploading
                         * was in process then no need to upload another file. */
                        MUTEX_LOCK(backupFtpMutex);
                        timeWaitRet = pthread_cond_timedwait(&backupFtpCond, &backupFtpMutex, &ts);
                        MUTEX_UNLOCK(backupFtpMutex);
                        if(timeWaitRet == STATUS_OK)
                        {
                            break;
                        }

                        if (callback == NULL)
                        {
                            continue;
                        }

                        if(callback(userData) == TRUE)
                        {
                            StopFtpTransfer(ftpHandle);
                            WPRINT(DISK_MANAGER, "user has aborted file upload on ftp: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        }

                    } while(TRUE);

                    if(ftpUploadResponse != FTP_SUCCESS)
                    {
                        EPRINT(DISK_MANAGER, "ftp connection error in file upload: [camera=%d], [path=%s]", channelNo, ftpFileInfo.remoteFile);
                        return BACKUP_FTP_CONN_FAIL;
                    }
                }
            }
        }
    }

    return backupStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was call back of ftp upload process. Ii will gives status of uploading file.
 * @param   ftpHandle
 * @param   ftpResponse
 * @param   userData
 */
static void backupFtpCallBack(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData)
{
    ftpUploadResponse = ftpResponse;
    MUTEX_LOCK(backupFtpMutex);
    pthread_cond_signal(&backupFtpCond);
    MUTEX_UNLOCK(backupFtpMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total size of given record
 * @param   startTime
 * @param   endTime
 * @param   backupCameraMask
 * @param   totalSize
 */
void GetSizeOfDuration(time_t startTime, time_t endTime, CAMERA_BIT_MASK_t backupCameraMask, UINT64PTR totalSize)
{
    CHAR		mountPoint[MOUNT_POINT_SIZE];
    CHAR		folderName[MAX_FILE_NAME_SIZE];
    UINT8		channelNo;
    UINT8		totalDisk;
    UINT8		diskCnt;
    UINT64		folderSize;
    time_t		timeInst;
    struct tm 	brokenTime = {0};

    *totalSize = 0;
    totalDisk = GetTotalMediaNo(MAX_RECORDING_MODE);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        if (GET_CAMERA_MASK_BIT(backupCameraMask, channelNo) == 0)
        {
            continue;
        }

        timeInst = startTime;
        while(timeInst < endTime)
        {
            if(GetBackupTermFlag() == TRUE)
            {
                break;
            }

            if(SUCCESS != ConvertLocalTimeInBrokenTm(&timeInst, &brokenTime))
            {
                EPRINT(DISK_MANAGER, "fail to get converted local time: [camera=%d]", channelNo);
            }

            for(diskCnt = 0; diskCnt < totalDisk; diskCnt++)
            {
                if(GetMountPointFromDiskId(diskCnt, mountPoint, MAX_RECORDING_MODE) == FAIL)
                {
                    continue;
                }

                snprintf(folderName, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mountPoint,
                         GET_CAMERA_NO(channelNo), brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour);
                if(access(folderName, F_OK) != STATUS_OK)
                {
                    continue;
                }

                DPRINT(DISK_MANAGER, "folder found: [camera=%d], [path=%s]", channelNo, folderName);
                if(GetFolderSize(folderName, &folderSize) == SUCCESS)
                {
                    *totalSize += folderSize;
                }
            }

            timeInst += SEC_IN_ONE_HOUR;
        }
    }

    DPRINT(DISK_MANAGER, "backup: [totalSize=%lld]", *totalSize);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total size of given record.
 * @param   serchData
 * @param   recStorageDrive
 * @param   noOfRecord
 * @param   totalSize
 * @return
 */
BOOL GetSizeOfRecord(SEARCH_RESULT_t *serchData, RECORD_ON_DISK_e recStorageDrive, UINT16 noOfRecord, UINT64PTR totalSize)
{
    UINT16          cnt, resCnt;
    UINT64          recSize = 0;
    CHAR            playDir[MAX_FILE_NAME_SIZE];
    CHAR            fileName[MAX_FILE_NAME_SIZE];
    PLAY_SESSION_ID playHandle;
    UINT8           startFile = 0, endFile = 0;
    UINT32          startPos = 0, endPos = 0;
    UINT32          fileSize;

    if((serchData == NULL) || (totalSize == NULL) || (noOfRecord == 0))
    {
        EPRINT(DISK_MANAGER, "invld input params");
        return FAIL;
    }

    for(cnt = 0; cnt < noOfRecord; cnt++)
    {
        if(OpenBackupSession(&serchData[cnt], recStorageDrive, &playHandle) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to open backup session");
            return FAIL;
        }

        getPlayBackDir(playHandle, playDir);
        if (SetPlayPosition(&serchData[cnt].startTime, 0, ENABLE, PLAY_FORWARD, PLAY_1X, playHandle) == FAIL)
        {
            ClosePlaySession(playHandle);
            EPRINT(DISK_MANAGER, "fail to set backup play position: [path=%s]", playDir);
            return FAIL;
        }

        getPlayBackPosition(playHandle, &startFile, &startPos);
        DPRINT(DISK_MANAGER, "backup start: [backupDir=%s], [startFile=%d], [startPos=%d]", playDir, startFile, startPos);

        if (SetPlayPosition(&serchData[cnt].endTime, 0, ENABLE, PLAY_FORWARD, PLAY_1X, playHandle) == FAIL)
        {
            ClosePlaySession(playHandle);
            EPRINT(DISK_MANAGER, "fail to set backup play position: [path=%s]", playDir);
            return FAIL;
        }

        getPlayBackPosition(playHandle, &endFile, &endPos);
        DPRINT(DISK_MANAGER, "backup end: [backupDir=%s], [endFile=%d], [endPos=%d]", playDir, endFile, endPos);

        if(startFile == endFile)
        {
            recSize += endPos - startPos;
        }
        else
        {
            for(resCnt = startFile; resCnt <= endFile; resCnt++)
            {
                if(getStreamFileName(playDir, resCnt, fileName, &fileSize) == FAIL)
                {
                    ClosePlaySession(playHandle);
                    EPRINT(DISK_MANAGER, "fail to get backup file size: [path=%s], [file=%s]", playDir, fileName);
                    return FAIL;
                }

                DPRINT(DISK_MANAGER, "backup info: [path=%s], [fileCnt=%d], [fileSize=%d]", playDir, resCnt, fileSize);

                if(resCnt == startFile)
                {
                    recSize += fileSize - startPos;
                }
                else if (resCnt == endFile)
                {
                    recSize += fileSize - (fileSize - endPos);
                }
                else
                {
                    recSize += fileSize;
                }
            }
        }

        ClosePlaySession(playHandle);
        *totalSize = recSize;
        DPRINT(DISK_MANAGER, "backup info: [totalSize=%llu]", recSize);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total size of given record.
 * @param   clipData
 * @param   recStorageDrive
 * @param   noOfRecord
 * @param   totalSize
 * @return
 */
BOOL GetSizeOfMultipleRecord(SYNC_CLIP_DATA_t *clipData, RECORD_ON_DISK_e recStorageDrive, UINT16 noOfRecord, UINT64PTR totalSize)
{
    BOOL                retVal = SUCCESS;
    BOOL                status;
    UINT16              cnt, resCnt;
    UINT64              recSize = 0;
    CHAR				playDir[MAX_FILE_NAME_SIZE];
    CHAR				fileName[MAX_FILE_NAME_SIZE];
    PLAY_SESSION_ID		playHandle;
    UINT8				startFile = 0, endFile = 0;
    UINT32				startPos = 0, endPos = 0;
    UINT32				fileSize;
    UINT32              recCnt = 0;
    UINT8               moreData = FALSE;
    UINT8               overlapIndicator;
    UINT16              searchResltLen = 0;
    UINT32              searchDataLen = 100;
    SEARCH_DATA_t       *searchData =  NULL;
    SEARCH_CRITERIA_t   searchCriteria;
    SEARCH_RESULT_t     searchRslt;
    time_t              localTime;

    if ((clipData == NULL) || (totalSize == NULL) || (noOfRecord == 0))
    {
        EPRINT(DISK_MANAGER, "invld input params");
        return FAIL;
    }

    for(cnt = 0; cnt < noOfRecord; cnt++)
    {
        searchCriteria.channelNo = clipData[cnt].cameraNo;
        memcpy(&searchCriteria.endTime, &clipData[cnt].endTime, sizeof(struct tm));
        memcpy(&searchCriteria.startTime, &clipData[cnt].startTime, sizeof(struct tm));

        searchCriteria.eventType = (EVENT_e)clipData[cnt].eventType;
        searchCriteria.searchRecStorageType = recStorageDrive;

        do
        {
            searchCriteria.noOfRecord = searchDataLen;
            if (searchData == NULL)
            {
                searchData = (SEARCH_DATA_t *)malloc(searchDataLen * (sizeof(SEARCH_DATA_t)));
                if (searchData == NULL)
                {
                    retVal = FAIL;
                    break;
                }
            }

            status = SearchCamAllEventForSync(&searchCriteria, searchData, &searchResltLen, &moreData, &overlapIndicator);
            if (status == FAIL || (searchResltLen == 0))
            {
                /* no record found or error while searching records */
                retVal = FAIL;
                break;
            }

            if (moreData == FALSE)
            {
                break;
            }

            /* more data available */
            searchDataLen += 50;
            free(searchData);
            searchData = NULL;
            moreData = FALSE;

        } while(TRUE);

        if (retVal == FAIL)
        {
            break;
        }

        for(recCnt = 0; recCnt < searchResltLen; recCnt++)
        {
            searchRslt.channelNo = searchData[recCnt].camNo;
            searchRslt.diskId = searchData[recCnt].diskId;
            searchRslt.recordType = searchData[recCnt].eventType;
            searchRslt.overlapFlg = searchData[recCnt].overlapFlg;
            searchRslt.recStorageType = searchData[recCnt].recStorageType;
            ConvertLocalTimeInBrokenTm(&searchData[recCnt].startTime, &searchRslt.startTime);
            ConvertLocalTimeInBrokenTm(&searchData[recCnt].stopTime, &searchRslt.endTime);

            if(OpenBackupSession(&searchRslt, recStorageDrive, &playHandle) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to open backup session");
                retVal = FAIL;
                break;
            }

            getPlayBackDir(playHandle, playDir);
            if(recCnt == 0)
            {
                ConvertLocalTimeInSec(&searchCriteria.startTime, &localTime);
                if(localTime > searchData[recCnt].startTime)
                {
                    memcpy(&searchRslt.startTime, &searchCriteria.startTime, sizeof(struct tm));
                }

                ConvertLocalTimeInSec(&searchCriteria.endTime, &localTime);
                if(localTime < searchData[recCnt].stopTime)
                {
                    memcpy(&searchRslt.endTime, &searchCriteria.endTime, sizeof(struct tm));
                }
            }

            if (SetPlayPosition(&searchRslt.startTime, 0, ENABLE, PLAY_FORWARD, PLAY_1X, playHandle) == FAIL)
            {
                ClosePlaySession(playHandle);
                EPRINT(DISK_MANAGER, "fail to set backup play position: [path=%s]", playDir);
                retVal = FAIL;
                break;
            }

            getPlayBackPosition(playHandle, &startFile, &startPos);
            DPRINT(DISK_MANAGER, "backup start: [backupDir=%s], [startFile=%d], [startPos=%d]", playDir, startFile, startPos);

            if (SetPlayPosition(&searchRslt.endTime, 0, ENABLE, PLAY_FORWARD, PLAY_1X, playHandle) == FAIL)
            {
                ClosePlaySession(playHandle);
                EPRINT(DISK_MANAGER, "fail to set backup play position: [path=%s]", playDir);
                retVal = FAIL;
                break;
            }

            getPlayBackPosition(playHandle, &endFile, &endPos);
            DPRINT(DISK_MANAGER, "backup end: [backupDir=%s], [endFile=%d], [endPos=%d]", playDir, endFile, endPos);

            if(startFile == endFile)
            {
                recSize += endPos - startPos;
            }
            else
            {
                for(resCnt = startFile; resCnt <= endFile; resCnt++)
                {
                    if(getStreamFileName(playDir, resCnt, fileName, &fileSize) == FAIL)
                    {
                        ClosePlaySession(playHandle);
                        retVal = FAIL;
                        EPRINT(DISK_MANAGER, "fail to get backup file size: [path=%s], [file=%s]", playDir, fileName);
                        break;
                    }

                    DPRINT(DISK_MANAGER, "backup info: [path=%s], [fileCnt=%d], [fileSize=%d]", playDir, resCnt, fileSize);

                    if(resCnt == startFile)
                    {
                        recSize += fileSize - startPos;
                    }
                    else if (resCnt == endFile)
                    {
                        recSize += fileSize - (fileSize - endPos);
                    }
                    else
                    {
                        recSize += fileSize;
                    }
                }
            }

            ClosePlaySession(playHandle);
            DPRINT(DISK_MANAGER, "backup info: [totalSize=%llu]", recSize);

            if (retVal == FAIL)
            {
                break;
            }
        }

        if (retVal == FAIL)
        {
            break;
        }
    }

    DPRINT(DISK_MANAGER, "backup info final: [totalSize=%llu]", recSize);
    *totalSize = recSize;
    FREE_MEMORY(searchData);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives session of back to take as per record search.
 * @param   serchData
 * @param   recStorageDrive
 * @param   playId
 * @return
 */
BOOL OpenBackupSession(SEARCH_RESULT_t *serchData, RECORD_ON_DISK_e recStorageDrive, PLAY_SESSION_ID *playId)
{
    UINT8				moreData = 0;
    UINT16				srchDataLen = 0, resCnt;
    SEARCH_CRITERIA_t	searchCriteria;
    SEARCH_DATA_t		searchRes[10];
    PLAY_CNTRL_INFO_t	playCtrl;
    time_t	 			startTime;
    time_t	 			endTime;

    searchCriteria.eventType = DM_ANY_EVENT;
    searchCriteria.noOfRecord = 10;
    searchCriteria.channelNo = serchData->channelNo;
    searchCriteria.startTime = serchData->startTime;
    searchCriteria.endTime = serchData->endTime;
    searchCriteria.searchRecStorageType = serchData->recStorageType;

    ConvertLocalTimeInSec(&serchData->startTime, &startTime);
    ConvertLocalTimeInSec(&serchData->endTime, &endTime);

    DPRINT(DISK_MANAGER, "backup session time: [camera=%d], [start=%02d/%02d/%04d %02d:%02d:%02d], [end=%02d/%02d/%04d %02d:%02d:%02d]",
           searchCriteria.channelNo-1, searchCriteria.startTime.tm_mday, searchCriteria.startTime.tm_mon+1, searchCriteria.startTime.tm_year,
           searchCriteria.startTime.tm_hour, searchCriteria.startTime.tm_min, searchCriteria.startTime.tm_sec,
           searchCriteria.endTime.tm_mday, searchCriteria.endTime.tm_mon+1, searchCriteria.endTime.tm_year,
           searchCriteria.endTime.tm_hour, searchCriteria.endTime.tm_min, searchCriteria.endTime.tm_sec);

    // all event of all camera
    searchCamAllEvent(&searchCriteria, searchRes, &srchDataLen, &moreData, NULL);
    if (srchDataLen == 0)
    {
        DPRINT(DISK_MANAGER, "search result not found: [camera=%d]", searchCriteria.channelNo-1);
        return FAIL;
    }

    for(resCnt = 0; resCnt < srchDataLen; resCnt++)
    {
        DPRINT(DISK_MANAGER, "search result time: [camera=%d], [start=%ld:%ld], [end=%ld:%ld], [overlap=%d:%d], [diskId=%d:%d]",
               searchCriteria.channelNo-1, searchRes[resCnt].startTime, startTime, searchRes[resCnt].stopTime, endTime,
               searchRes[resCnt].overlapFlg, serchData->overlapFlg, searchRes[resCnt].diskId, serchData->diskId);

        if((searchRes[resCnt].startTime <= startTime) && (searchRes[resCnt].stopTime >= endTime)
                && (searchRes[resCnt].overlapFlg == serchData->overlapFlg) && (searchRes[resCnt].diskId == serchData->diskId))
        {
            break;
        }
    }

    if (resCnt >= srchDataLen)
    {
        return FAIL;
    }

    playCtrl.camNo = searchRes[resCnt].camNo;
    playCtrl.diskId = searchRes[resCnt].diskId;
    playCtrl.eventType = searchRes[resCnt].eventType;
    playCtrl.overlapFlg = searchRes[resCnt].overlapFlg;
    playCtrl.recStorageType = searchRes[resCnt].recStorageType;

    ConvertLocalTimeInBrokenTm(&searchRes[resCnt].startTime,&playCtrl.startTime);
    ConvertLocalTimeInBrokenTm(&searchRes[resCnt].stopTime,&playCtrl.stopTime);

    if(OpenPlaySession(&playCtrl, BACKUP_PLAY, playId) != CMD_SUCCESS)
    {
        EPRINT(DISK_MANAGER, "fail to open backup session: [camera=%d]", searchCriteria.channelNo-1);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function closes session of backup
 * @param   playId
 * @return  SUCCESS/FAIL
 */
BOOL CloseBackupSession(PLAY_SESSION_ID playId)
{
    return ClosePlaySession(playId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns Playback stream count.
 * @return
 */
UINT8 GetPlayBackStreamCnt(void)
{
    UINT8 	sessionIdx;
    UINT8	totalPlaybackSession = 0;

    MUTEX_LOCK(playbckSessionMutex);
    for (sessionIdx = 0; sessionIdx < MAX_PLAYBACK_SESSION; sessionIdx++)
    {
        // check that session was running or not
        if(playSession[sessionIdx].playStatus == ON)
        {
            totalPlaybackSession++;
        }
    }
    MUTEX_UNLOCK(playbckSessionMutex);
    return totalPlaybackSession;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns Playback stream count.
 * @return
 */
UINT8 GetRecordingStreamCnt(void)
{
    UINT8 channelCnt;
    UINT8 recordingStreamCount = 0;

    for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
    {
        if(sessionInfo[channelCnt].sessionStatus == ON)
        {
            recordingStreamCount++;
        }
    }
    return recordingStreamCount;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function takes back-up of give record.
 * @param   record
 * @param   recStorageDrive
 * @param   callback
 * @param   privData
 * @param   stepSize
 * @param   backupLocation
 * @return
 */
DM_BACKUP_STATUS_e BackUpRecordToManualDrive(SEARCH_RESULT_t *record, RECORD_ON_DISK_e recStorageDrive, BKUP_RCD_EVENT_CB callback,
                                             VOIDPTR privData, UINT64 stepSize, MB_LOCATION_e backupLocation)
{
    PLAY_SESSION_ID 		playId;
    DM_BACKUP_STATUS_e 		retVal = BACKUP_FAIL;
    FSH_INFO_t				fshData;
    PLAYBACK_FILE_READ_e	playbackReadError = PLAYBACK_FILE_READ_NORMAL;
    UINT8PTR				frameData;
    UINT32					frameLen;
    AVI_HANDLE				aviHandle;
    UINT8					eventCnt;
    struct tm 				tmpBrokenTime;
    time_t					tmpTime;
    CHAR					backUpFileName[MAX_FILE_NAME_SIZE];
    CHAR					tmpBackUpFileName[MAX_FILE_NAME_SIZE];
    CHAR					mountPoint[MOUNT_POINT_SIZE];
    BOOL					brakeAviFile;
    UINT64					processedSize;
    UINT64					fileSizeLimit;
    time_t					recordEndTime;
    UINT16					chunkCnt;
    UINT64					nextFrameTime = 0;
    MANUAL_BACKUP_CONFIG_t  manualBackupCfg;

    ReadManualBackupConfig(&manualBackupCfg);
    if((backupLocation < MB_TO_USB_DEVICE) || (backupLocation >= MAX_MB_LOCATION))
    {
        backupLocation = manualBackupCfg.backupLocation;
    }

    if (FAIL == OpenBackupSession(record, recStorageDrive, &playId))
    {
        return BACKUP_FAIL;
    }

    if (SetPlayPosition(&record->startTime, 0, ENABLE, PLAY_FORWARD, PLAY_1X, playId) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to set backup play position");
        CloseBackupSession(playId);
        return BACKUP_FAIL;
    }

    if (backupLocation == MB_TO_USB_DEVICE)
    {
        if (GetMountPointForBackupDevice(MANUAL_BACKUP_DISK, mountPoint) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to get mount path for manual backup on usb");
            CloseBackupSession(playId);
            return BACKUP_FAIL;
        }
    }
    else
    {
        UINT8 diskId = backupLocation - MB_TO_NETWORK_DRIVE_1;
        if (GetMountPointFromNetworkDiskIdForBackUp(diskId, mountPoint) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to get mount path for manual backup on ndd: [ndd=%d]", diskId);
            CloseBackupSession(playId);
            return BACKUP_FAIL;
        }
    }

    // Sort- of Initialisation
    ConvertLocalTimeInSec(&record->endTime, &recordEndTime);
    prepareBackUpFileName(backUpFileName, record, mountPoint);

    fileSizeLimit = MAX_AVI_FILE_SIZE;
    eventCnt = 0;
    chunkCnt = 0;
    processedSize = (STREAM_FILE_HDR_SIZE);

    // Here kind of FSM is used
    // (OPEN)-->(WRITE)-->(WRITE) --> read over -->(CLOSE)
    // (OPEN)-->(WRITE) -- size > 2 GB --> (CLOSE) --> do needful --> (OPEN) -->(WRITE)--> (CLOSE)
    do
    {
        aviHandle = AviOpen(backUpFileName);
        if (aviHandle == INVALID_AVI_HANDLE)
        {
            retVal = BACKUP_FAIL;
            EPRINT(DISK_MANAGER, "fail to start backup of records");
            break;
        }

        // If file-size exceeds limit set this flag as true
        brakeAviFile = FALSE;
        retVal = BACKUP_SUCCESS;

        do
        {
            frameLen = 0;
            if (ReadRecordFrame(playId, PLAY_FORWARD, NW_ANY_FRAME, &fshData, &frameData, &frameLen, &playbackReadError, &nextFrameTime) == FAIL)
            {
                if (playbackReadError != PLAYBACK_FILE_READ_OVER)
                {
                    retVal = BACKUP_FAIL;
                    EPRINT(DISK_MANAGER, "fail to read frame: [playbackReadError=%d]", playbackReadError);
                }
                break;
            }

            if (AviWrite(aviHandle, &fshData, frameData, frameLen) == FAIL)
            {
                retVal = BACKUP_FAIL;
                break;
            }

            // data size exceeds step size call event callback
            processedSize += (frameLen + MAX_FSH_SIZE);
            if ((processedSize - (eventCnt * stepSize)) > stepSize)
            {
                // Check if we need to stop back up
                eventCnt++;
                if (callback(privData, processedSize, FALSE) == FALSE)
                {
                    retVal = BACKUP_STOPPED;
                }
            }
            else
            {
                if (callback(privData, processedSize, TRUE) == FALSE)
                {
                    retVal = BACKUP_STOPPED;
                }
            }

            if ((time_t)fshData.localTime.totalSec > recordEndTime)
            {
                DPRINT(DISK_MANAGER, "record over for backup");
                break;
            }

            // put manual sleep at every KILO_BYTE chunk
            if ((processedSize - (chunkCnt * KILO_BYTE)) > KILO_BYTE)
            {
                usleep(5);
                chunkCnt++;
            }

            // AVI file has limitation of 2 GB. here 1 MB margin is taken If file size exceeds this we brake record in to files
            if (processedSize > fileSizeLimit)
            {
                // Set limit to max so that next time condition evaluates to false
                fileSizeLimit += MAX_AVI_FILE_SIZE;
                brakeAviFile = TRUE;
                break;
            }

        } while (retVal == BACKUP_SUCCESS);

        AviClose(aviHandle);

        if (brakeAviFile == TRUE)
        {
            DPRINT(DISK_MANAGER, "breaking avi file in two parts");
            tmpBrokenTime = record->endTime;

            // As time stored in UINT32 First store it in time_t then pass to function for being on safer side
            tmpTime = fshData.localTime.totalSec;
            ConvertLocalTimeInBrokenTm(&tmpTime, &record->endTime);
            prepareBackUpFileName(tmpBackUpFileName, record, mountPoint);
            rename(backUpFileName, tmpBackUpFileName);

            record->startTime = record->endTime;
            record->endTime = tmpBrokenTime;
            prepareBackUpFileName(backUpFileName, record, mountPoint);
        }

        // Rename file if back up stopped in-between
        if (retVal == BACKUP_STOPPED)
        {
            // As time stored in UINT32 First store it in time_t then pass to function for being on safer side
            tmpTime = (time_t)fshData.localTime.totalSec;
            ConvertLocalTimeInBrokenTm(&tmpTime, &record->endTime);
            prepareBackUpFileName(tmpBackUpFileName, record, mountPoint);
            rename(backUpFileName, tmpBackUpFileName);
        }

        // Check If the File is to be broken into two parts
    } while (brakeAviFile == TRUE);

    CloseBackupSession(playId);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function takes back-up of give record.
 * @param   record
 * @param   recStorageDrive
 * @param   callback
 * @param   privData
 * @param   stepSize
 * @return
 */
DM_BACKUP_STATUS_e BackUpSyncRecordToManualDrive(SYNC_CLIP_DATA_t *record, RECORD_ON_DISK_e recStorageDrive,
                                                 BKUP_RCD_EVENT_CB callback, VOIDPTR privData, UINT64 stepSize)
{
    PLAY_SESSION_ID 		playId;
    DM_BACKUP_STATUS_e 		retVal;
    FSH_INFO_t				fshData;
    PLAYBACK_FILE_READ_e	playbackReadError = PLAYBACK_FILE_READ_NORMAL;
    UINT8PTR				frameData;
    UINT32					frameLen;
    AVI_HANDLE				aviHandle = INVALID_AVI_HANDLE;
    UINT8					eventCnt;
    struct tm 				tmpBrokenTime;
    time_t					tmpTime;
    CHAR					backUpFileName[MAX_FILE_NAME_SIZE];
    CHAR					tmpBackUpFileName[MAX_FILE_NAME_SIZE];
    CHAR					mountPoint[MOUNT_POINT_SIZE];
    BOOL					brakeAviFile;
    UINT64					processedSize;
    UINT64					fileSizeLimit;
    time_t					recordEndTime;
    UINT16					chunkCnt;
    UINT64					nextFrameTime = 0;
    time_t					localTime;
    BOOL					status;
    UINT16					recCnt = 0;
    UINT8					moreData = FALSE;
    UINT8					overlapIndicator;
    UINT16					searchResltLen = 0;
    UINT32					searchDataLen = 100;
    SEARCH_DATA_t			*searchData =  NULL;
    SEARCH_CRITERIA_t		searchCriteria;
    SEARCH_RESULT_t			searchRslt, tmpSrchRslt;
    UINT8 					refIndex,compareIndex;
    SEARCH_DATA_t			tempSrchData;
    MANUAL_BACKUP_CONFIG_t  manualBackupCfg;

    searchCriteria.channelNo = record->cameraNo;
    memcpy(&searchCriteria.endTime, &record->endTime, sizeof(struct tm));
    memcpy(&searchCriteria.startTime, &record->startTime, sizeof(struct tm));
    searchCriteria.eventType = (EVENT_e)record->eventType;
    searchCriteria.searchRecStorageType = recStorageDrive;

    do
    {
        searchCriteria.noOfRecord = searchDataLen;
        if(searchData == NULL)
        {
            searchData = (SEARCH_DATA_t *)malloc(searchDataLen * (sizeof(SEARCH_DATA_t)));
        }

        if (searchData == NULL)
        {
            EPRINT(DISK_MANAGER, "fail to alloc memory: [camera=%d]", searchCriteria.channelNo-1);
            return BACKUP_FAIL;
        }

        // Check Only One event is present for playback
        status = SearchCamAllEventForSync(&searchCriteria, searchData, &searchResltLen, &moreData, &overlapIndicator);
        if (status == FAIL)
        {
            break;
        }

        if (moreData == FALSE)
        {
            break;
        }

        searchDataLen += 50;
        free(searchData);
        searchData = NULL;
        moreData = FALSE;

    } while(TRUE);

    // Initialisation
    retVal = BACKUP_FAIL;
    fileSizeLimit = MAX_AVI_FILE_SIZE;
    eventCnt = 0;
    chunkCnt = 0;
    processedSize = STREAM_FILE_HDR_SIZE;

    ReadManualBackupConfig(&manualBackupCfg);

    if (manualBackupCfg.backupLocation == MB_TO_USB_DEVICE)
    {
        if (GetMountPointForBackupDevice(MANUAL_BACKUP_DISK, mountPoint) == FAIL)
        {
            free(searchData);
            return BACKUP_FAIL;
        }
    }
    else
    {
        if (GetMountPointFromNetworkDiskIdForBackUp((manualBackupCfg.backupLocation - MB_TO_NETWORK_DRIVE_1), mountPoint) == FAIL)
        {
            free(searchData);
            return BACKUP_FAIL;
        }
    }

    if(searchResltLen == 0)
    {
        free(searchData);
        return BACKUP_FAIL;
    }

    for(recCnt = 0; recCnt < (searchResltLen-1); recCnt++)
    {
        refIndex = recCnt;
        for(compareIndex = (refIndex + 1); compareIndex < searchResltLen; compareIndex++)
        {
            if(searchData[refIndex].recStorageType > searchData[compareIndex].recStorageType)
            {
                refIndex = compareIndex;
            }
            else if((searchData[refIndex].recStorageType == searchData[compareIndex].recStorageType) &&
                    (searchData[refIndex].overlapFlg > searchData[compareIndex].overlapFlg))
            {
                refIndex = compareIndex;
            }
            else if((searchData[refIndex].recStorageType == searchData[compareIndex].recStorageType) &&
                    (searchData[refIndex].overlapFlg == searchData[compareIndex].overlapFlg) &&
                    (searchData[refIndex].startTime > searchData[compareIndex].startTime))
            {
                refIndex = compareIndex;
            }
        }

        if(recCnt != refIndex)
        {
            memcpy(&tempSrchData, &searchData[recCnt], sizeof(SEARCH_DATA_t));
            memcpy(&searchData[recCnt], &searchData[refIndex], sizeof(SEARCH_DATA_t));
            memcpy(&searchData[refIndex], &tempSrchData, sizeof(SEARCH_DATA_t));
        }
    }

    for(recCnt = 0; recCnt < searchResltLen; recCnt++)
    {
        searchRslt.channelNo = searchData[recCnt].camNo;
        searchRslt.diskId = searchData[recCnt].diskId;
        searchRslt.recordType = searchData[recCnt].eventType;
        searchRslt.overlapFlg = searchData[recCnt].overlapFlg;
        searchRslt.recStorageType = searchData[recCnt].recStorageType;
        ConvertLocalTimeInBrokenTm(&searchData[recCnt].startTime, &searchRslt.startTime);
        ConvertLocalTimeInBrokenTm(&searchData[recCnt].stopTime, &searchRslt.endTime);

        if(recCnt == 0)
        {
            memcpy(&tmpSrchRslt, &searchRslt, sizeof(SEARCH_RESULT_t));
            ConvertLocalTimeInSec(&searchCriteria.startTime, &localTime);
            if(localTime > searchData[recCnt].startTime)
            {
                memcpy(&searchRslt.startTime, &searchCriteria.startTime, sizeof(struct tm));
            }

            ConvertLocalTimeInSec(&searchCriteria.endTime, &localTime);
            if(localTime < searchData[recCnt].stopTime)
            {
                memcpy(&searchRslt.endTime, &searchCriteria.endTime, sizeof(struct tm));
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&searchData[recCnt].stopTime, &searchRslt.endTime);
            }

            prepareBackUpFileName(backUpFileName, &searchRslt, mountPoint);
            aviHandle = AviOpen(backUpFileName);
            if (aviHandle == INVALID_AVI_HANDLE)
            {
                retVal = BACKUP_FAIL;
                EPRINT(DISK_MANAGER, "fail to start backup of records: [camera=%d]", searchCriteria.channelNo-1);
                break;
            }
        }
        else if(((searchData[recCnt].startTime - searchData[recCnt -1].stopTime) > 5)
                 || (searchData[recCnt].overlapFlg != searchData[recCnt -1].overlapFlg)
                 || (searchData[recCnt].recStorageType != searchData[recCnt -1].recStorageType))
        {
            DPRINT(DISK_MANAGER, "breaking avi file in two parts: [camera=%d]", searchCriteria.channelNo-1);
            AviClose(aviHandle);
            aviHandle = INVALID_AVI_HANDLE;

            tmpSrchRslt.channelNo = searchData[recCnt - 1].camNo;
            tmpSrchRslt.diskId = searchData[recCnt - 1].diskId;
            tmpSrchRslt.recordType = searchData[recCnt - 1].eventType;
            tmpSrchRslt.overlapFlg = searchData[recCnt - 1].overlapFlg;
            ConvertLocalTimeInBrokenTm(&searchData[recCnt - 1].startTime, &tmpSrchRslt.startTime);

            ConvertLocalTimeInSec(&searchCriteria.startTime, &localTime);
            if(localTime > searchData[recCnt].startTime)
            {
                memcpy(&searchRslt.startTime, &searchCriteria.startTime, sizeof(struct tm));
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&searchData[recCnt].startTime, &searchRslt.startTime);
            }

            ConvertLocalTimeInSec(&searchCriteria.endTime, &localTime);
            if(localTime < searchData[recCnt].stopTime)
            {
                memcpy(&searchRslt.endTime, &searchCriteria.endTime, sizeof(struct tm));
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&searchData[recCnt].stopTime, &searchRslt.endTime);
            }

            if(localTime < searchData[recCnt - 1].stopTime)
            {
                memcpy(&tmpSrchRslt.endTime, &searchCriteria.endTime, sizeof(struct tm));
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&searchData[recCnt - 1].stopTime, &tmpSrchRslt.endTime);
            }

            appendBackUpEndTime(backUpFileName, tmpBackUpFileName, &tmpSrchRslt.endTime);
            rename(backUpFileName, tmpBackUpFileName);
            prepareBackUpFileName(backUpFileName, &searchRslt, mountPoint);

            aviHandle = AviOpen(backUpFileName);
            if (aviHandle == INVALID_AVI_HANDLE)
            {
                retVal = BACKUP_FAIL;
                EPRINT(DISK_MANAGER, "fail to start backup of records: [camera=%d]", searchCriteria.channelNo-1);
                break;
            }
        }

        if (OpenBackupSession(&searchRslt, recStorageDrive, &playId) == FAIL)
        {
            break;
        }

        if(recCnt == 0)
        {
            ConvertLocalTimeInSec(&searchCriteria.startTime, &localTime);
            if(localTime > searchData[recCnt].startTime)
            {
                memcpy(&searchRslt.startTime, &searchCriteria.startTime, sizeof(struct tm));
            }

            ConvertLocalTimeInSec(&searchCriteria.endTime, &localTime);
            if(localTime < searchData[recCnt].stopTime)
            {
                memcpy(&searchRslt.endTime, &searchCriteria.endTime, sizeof(struct tm));
            }
        }
        else
        {
            ConvertLocalTimeInSec(&searchCriteria.startTime, &localTime);
            if(localTime > searchData[recCnt].startTime)
            {
                memcpy(&searchRslt.startTime, &searchCriteria.startTime, sizeof(struct tm));
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&searchData[recCnt].startTime, &searchRslt.startTime);
            }

            ConvertLocalTimeInSec(&searchCriteria.endTime, &localTime);
            if(localTime < searchData[recCnt].stopTime)
            {
                memcpy(&searchRslt.endTime, &searchCriteria.endTime, sizeof(struct tm));
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&searchData[recCnt].stopTime, &searchRslt.endTime);
            }
        }

        if (SetPlayPosition(&searchRslt.startTime, 0, ENABLE, PLAY_FORWARD, PLAY_1X, playId) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to set backup play position: [camera=%d]", searchCriteria.channelNo-1);
            break;
        }

        // Sort- of Initialisation
        ConvertLocalTimeInSec(&searchRslt.endTime, &recordEndTime);

        // If file-size exceeds limit set this flag as true
        brakeAviFile = FALSE;
        retVal = BACKUP_SUCCESS;

        do
        {
            frameLen = 0;
            if (ReadRecordFrame(playId, PLAY_FORWARD, NW_ANY_FRAME, &fshData, &frameData, &frameLen, &playbackReadError, &nextFrameTime) == FAIL)
            {
                if (playbackReadError != PLAYBACK_FILE_READ_OVER)
                {
                    retVal = BACKUP_FAIL;
                    EPRINT(DISK_MANAGER, "fail to read frame: [camera=%d], [playbackReadError=%d]", searchCriteria.channelNo-1, playbackReadError);
                }
                break;
            }

            if (AviWrite(aviHandle, &fshData, frameData, frameLen) == FAIL)
            {
                retVal = BACKUP_FAIL;
                break;
            }

            // data size exceeds step size call event callback
            processedSize += (frameLen + MAX_FSH_SIZE);
            if ((processedSize - (eventCnt * stepSize)) > stepSize)
            {
                // Check if we need to stop back up
                if (callback(privData, processedSize, FALSE) == FALSE)
                {
                    EPRINT(DISK_MANAGER, "backup percentage callback failed: [camera=%d]", searchCriteria.channelNo-1);
                    retVal = BACKUP_STOPPED;
                }
                eventCnt++;
            }
            else
            {
                if (callback(privData, processedSize, TRUE) == FALSE)
                {
                    EPRINT(DISK_MANAGER, "backup percentage callback failed: [camera=%d]", searchCriteria.channelNo-1);
                    retVal = BACKUP_STOPPED;
                }
            }

            if (fshData.localTime.totalSec > (UINT32)recordEndTime)
            {
                DPRINT(DISK_MANAGER, "backup record over: [camera=%d]", searchCriteria.channelNo-1);
                break;
            }

            // put manual sleep at every KILO_BYTE chunk
            if ((processedSize - (chunkCnt * KILO_BYTE)) > KILO_BYTE)
            {
                usleep(5);
                chunkCnt++;
            }

            // AVI file has limitation of 2 GB. here 1 MB margin is taken. If file size exceeds this we brake record into files
            if (processedSize > fileSizeLimit)
            {
                // Set limit to max so that next time condition evaluates to false
                fileSizeLimit += MAX_AVI_FILE_SIZE;
                brakeAviFile = TRUE;
            }

            if (brakeAviFile == TRUE)
            {
                brakeAviFile = FALSE;
                DPRINT(DISK_MANAGER, "breaking avi file in two parts: [camera=%d]", searchCriteria.channelNo-1);
                AviClose(aviHandle);
                aviHandle = INVALID_AVI_HANDLE;
                tmpBrokenTime = record->endTime;

                // As time stored in UINT32 First store it in time_t then pass to function for being on safer side
                tmpTime = (time_t)fshData.localTime.totalSec;
                ConvertLocalTimeInBrokenTm(&tmpTime, &searchRslt.endTime);
                appendBackUpEndTime(backUpFileName, tmpBackUpFileName, &searchRslt.endTime);
                rename(backUpFileName, tmpBackUpFileName);
                searchRslt.startTime = searchRslt.endTime;
                searchRslt.endTime = tmpBrokenTime;

                prepareBackUpFileName(backUpFileName, &searchRslt, mountPoint);
                aviHandle = AviOpen(backUpFileName);
                if(aviHandle == INVALID_AVI_HANDLE)
                {
                    retVal = BACKUP_FAIL;
                    EPRINT(DISK_MANAGER, "fail to start backup of records: [camera=%d]", searchCriteria.channelNo-1);
                    break;
                }
            }

        } while (retVal == BACKUP_SUCCESS);

        CloseBackupSession(playId);

        // Rename file if back up stopped in-between
        if (retVal != BACKUP_SUCCESS)
        {
            break;
        }
    }

    if(aviHandle != INVALID_AVI_HANDLE)
    {
        AviClose(aviHandle);

        // As time stored in UINT32 First store it in time_t then pass to function for being on safer side
        tmpTime = (time_t)fshData.localTime.totalSec;
        ConvertLocalTimeInBrokenTm(&tmpTime, &searchRslt.endTime);
        appendBackUpEndTime(backUpFileName, tmpBackUpFileName, &searchRslt.endTime);
        if(strcmp(backUpFileName, tmpBackUpFileName) != STATUS_OK)
        {
            rename(backUpFileName, tmpBackUpFileName);
        }
    }

    free(searchData);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives backup file name.
 * @param   fileName
 * @param   record
 * @param   mountPoint
 */
static void prepareBackUpFileName(CHARPTR fileName, SEARCH_RESULT_t *record,
                                  CHARPTR mountPoint)
{
    snprintf(fileName, MAX_FILE_NAME_SIZE, "%s" "Camera%02d~" REC_FOLDER_DATE_FORMAT "~"
             REC_FOLDER_TIME_FORMAT "~" REC_FOLDER_TIME_FORMAT "~" "%s~" "%1d%1d%s",
             mountPoint, record->channelNo, record->startTime.tm_mday,
             GetMonthName(record->startTime.tm_mon), record->startTime.tm_year,
             record->startTime.tm_hour, record->startTime.tm_min, record->startTime.tm_sec,
             record->endTime.tm_hour, record->endTime.tm_min, record->endTime.tm_sec,
             storageDriveNameList[record->recStorageType],
            record->diskId, record->overlapFlg, AVI_EXTENTION);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives backup file name.
 * @param   oldfileName
 * @param   newfileName
 * @param   endTime
 */
static void appendBackUpEndTime(CHARPTR oldfileName, CHARPTR newfileName, struct tm *endTime)
{
    CHARPTR searchPtr;
    UINT16  outLen;

    snprintf(newfileName, MAX_FILE_NAME_SIZE, "%s", oldfileName);
    searchPtr = strchr(newfileName, '~');
    if (searchPtr == NULL)
    {
        return;
    }

    searchPtr++;
    searchPtr = strchr(searchPtr, '~');
    if (searchPtr == NULL)
    {
        return;
    }

    searchPtr++;
    searchPtr = strchr(searchPtr, '~');
    if (searchPtr == NULL)
    {
        return;
    }

    searchPtr++;
    outLen = snprintf(searchPtr, MAX_FILE_NAME_SIZE, REC_FOLDER_TIME_FORMAT, endTime->tm_hour, endTime->tm_min, endTime->tm_sec);
    searchPtr[outLen] = '~';
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is avi convertor thread
 * @param   threadParam
 * @return
 */
static VOIDPTR aviConverter(VOIDPTR threadParam)
{
    UINT32              readIdx = 0;
    AVI_CONVERTER_MSG_t aviMessage;
    BOOL                updateReadIndex = FALSE;
    struct stat         fileSize;
    double              totalTime;
    clock_t             start, end;

    THREAD_START("AVI_CONVERT");
    memset(&aviMessage, 0, sizeof(AVI_CONVERTER_MSG_t));
    aviMessage.aviConvertType = MAX_AVI_CONVERT_TYPE;

    while(TRUE)
    {
        MUTEX_LOCK(aviConvertParam.aviCnvrtCondMutex);
        if(aviConvertParam.readIdx == aviConvertParam.writeIdx)
        {
            pthread_cond_wait(&aviConvertParam.aviCnvrtCondSignal, &aviConvertParam.aviCnvrtCondMutex);
            MUTEX_UNLOCK(aviConvertParam.aviCnvrtCondMutex);
        }
        else
        {
            readIdx = aviConvertParam.readIdx + 1;
            if(readIdx >= AVI_CNVRT_MSG_QUEUE_MAX)
            {
                readIdx = 0;
            }

            memcpy(&aviMessage, &aviConvertParam.aviCnvrtMsg[readIdx], sizeof(AVI_CONVERTER_MSG_t));
            MUTEX_UNLOCK(aviConvertParam.aviCnvrtCondMutex);
            updateReadIndex = TRUE;
        }

        if(aviMessage.aviConvertType != MAX_AVI_CONVERT_TYPE)
        {
            DPRINT(DISK_MANAGER, "convert avi file: [file=%s]", aviMessage.fileName);
            start = clock();
            convertStrmToAvi(aviMessage.fileName);
            end = clock();
            totalTime = ((double)(end - start))/CLOCKS_PER_SEC;

            if(stat(aviMessage.fileName, &fileSize) ==  STATUS_OK)
            {
                DPRINT(DISK_MANAGER, "avi file converted: [file=%s], [size=%lld], [time_taken=%fsec]", aviMessage.fileName, (UINT64)fileSize.st_size, totalTime);
            }
            else
            {
                EPRINT(DISK_MANAGER, "fail to stat avi file: [file=%s], [time_taken=%fsec], [err=%s]", aviMessage.fileName, totalTime, STR_ERR);
            }

            if((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) && (access(aviMessage.fileName,F_OK)) == STATUS_OK)
            {
                if(unlink(aviMessage.fileName) == STATUS_OK)
                {
                    DPRINT(DISK_MANAGER, "avi file removed: [file=%s]", aviMessage.fileName);
                }
                else
                {
                    EPRINT(DISK_MANAGER, "fail to remove avi file: [file=%s], [err=%s]", aviMessage.fileName, STR_ERR);
                }
            }
        }

        if(updateReadIndex == TRUE)
        {
            MUTEX_LOCK(aviConvertParam.aviCnvrtCondMutex);
            aviConvertParam.readIdx = readIdx;  //read index should be updated after conversion
            MUTEX_UNLOCK(aviConvertParam.aviCnvrtCondMutex);
            updateReadIndex = FALSE;
        }

        memset(&aviMessage, 0, sizeof(AVI_CONVERTER_MSG_t));
        aviMessage.aviConvertType = MAX_AVI_CONVERT_TYPE;
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   writeaviCnvrtMesg
 * @param   aviMsgPtr
 * @return
 */
static BOOL writeaviCnvrtMesg(AVI_CONVERTER_MSG_t *aviMsgPtr)
{
    UINT32 writeIdx;

    MUTEX_LOCK(aviConvertParam.aviCnvrtCondMutex);
    writeIdx = aviConvertParam.writeIdx + 1;
    if(writeIdx >= AVI_CNVRT_MSG_QUEUE_MAX)
    {
        writeIdx = 0;
    }

    if (writeIdx == aviConvertParam.readIdx)
    {
        MUTEX_UNLOCK(aviConvertParam.aviCnvrtCondMutex);
        EPRINT(DISK_MANAGER, "avi msg write failed: [file=%s]", aviMsgPtr->fileName);
        return FAIL;
    }

    memcpy(&aviConvertParam.aviCnvrtMsg[writeIdx], aviMsgPtr, sizeof(AVI_CONVERTER_MSG_t));
    aviConvertParam.writeIdx = writeIdx;
    pthread_cond_signal(&aviConvertParam.aviCnvrtCondSignal);
    MUTEX_UNLOCK(aviConvertParam.aviCnvrtCondMutex);
    DPRINT(DISK_MANAGER, "avi msg write success: [file=%s]", aviMsgPtr->fileName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   convertStrmToAvi
 * @param   strmFileName
 */
static void convertStrmToAvi(CHARPTR strmFileName)
{
    CHAR            aviFileName[MAX_FILE_NAME_SIZE] = {0};
    CHAR            newAviFileName[MAX_FILE_NAME_SIZE];
    CHARPTR         tmpFileName;
    UINT32          curStreamPos = 0;
    INT32           payloadSize = 0;
    INT32           streamFileFd = INVALID_FILE_FD;
    AVI_HANDLE      aviHandle = NULL;
    UINT16          outLen = 0;
    FSH_INFO_t      fshInfo;
    STRM_FILE_HDR_t streamFileHdr;
    UINT8           frameBuffer[1000000];

    //Open the Stream File
    streamFileFd = open(strmFileName, READ_ONLY_MODE, FILE_PERMISSION);
    if (streamFileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open stream file: [path=%s], [err=%s]", strmFileName, STR_ERR);
        return;
    }

    //Parse the aviFileName
    tmpFileName = strrchr(strmFileName, STM_FILE_SEP);
    if (tmpFileName == NULL)
    {
        close(streamFileFd);
        EPRINT(DISK_MANAGER, "file seperator not found: [path=%s]", strmFileName);
        return;
    }

    outLen = (strlen(strmFileName) - strlen(tmpFileName)) + 1 ;
    snprintf(aviFileName, outLen > MAX_FILE_NAME_SIZE ? MAX_FILE_NAME_SIZE : outLen, "%s", strmFileName);
    snprintf(&aviFileName[strlen(aviFileName)], MAX_FILE_NAME_SIZE - strlen(aviFileName), "_incomplete.avi");

    //Opening The Avi file and Initialize the Params
    aviHandle = AviOpen(aviFileName);
    if (aviHandle == INVALID_AVI_HANDLE)
    {
        close(streamFileFd);
        EPRINT(DISK_MANAGER, "fail to open avi file: [path=%s], [avi=%s]", strmFileName, aviFileName);
        return;
    }

    DPRINT(DISK_MANAGER, "stream file ready for conversion: [path=%s], [avi=%s]", strmFileName, aviFileName);
    curStreamPos = 0;
    if (read(streamFileFd, &streamFileHdr, STREAM_FILE_HDR_SIZE) != STREAM_FILE_HDR_SIZE)
    {
        EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", strmFileName, STR_ERR);
    }
    else
    {
        DPRINT(DISK_MANAGER, "stream file position: [end=%d]", streamFileHdr.nextStreamPos);
        curStreamPos += STREAM_FILE_HDR_SIZE;

        do
        {
            if (read(streamFileFd, &fshInfo, MAX_FSH_SIZE) != MAX_FSH_SIZE)
            {
                EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", strmFileName, STR_ERR);
                break;
            }

            curStreamPos += MAX_FSH_SIZE;
            if (fshInfo.startCode != FSH_START_CODE)
            {
                continue;
            }

            /* Validate media type and codec type */
            if (fshInfo.mediaType == STREAM_TYPE_VIDEO)
            {
                if (fshInfo.codecType >= MAX_VIDEO_CODEC)
                {
                    /* Invalid video codec */
                    continue;
                }
            }
            else if (fshInfo.mediaType == STREAM_TYPE_AUDIO)
            {
                if (fshInfo.codecType >= MAX_AUDIO_CODEC)
                {
                    /* Invalid audio codec */
                    continue;
                }
            }
            else
            {
                /* Invalid media type */
                continue;
            }

            payloadSize = (fshInfo.fshLen - MAX_FSH_SIZE);
            if ((payloadSize <= 0) || (payloadSize > (INT32)sizeof(frameBuffer)))
            {
                EPRINT(DISK_MANAGER, "invld frame payload size: [path=%s], [payloadSize=%d]", strmFileName, payloadSize);
                break;
            }

            if(read(streamFileFd, frameBuffer, payloadSize) != (INT32)payloadSize)
            {
                EPRINT(DISK_MANAGER, "fail to read stream file: [path=%s], [err=%s]", strmFileName, STR_ERR);
                break;
            }

            // write into avi file
            curStreamPos += payloadSize;
            if (AviWrite(aviHandle, &fshInfo, frameBuffer, payloadSize) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to write avi file: [path=%s], [avi=%s], [err=%s]", strmFileName, aviFileName, STR_ERR);
            }

            /* PARASOFT : No need to validate tainted data */
        }while ((curStreamPos + MAX_FSH_SIZE) <= streamFileHdr.nextStreamPos);
    }

    AviClose(aviHandle);
    snprintf(newAviFileName, outLen > MAX_FILE_NAME_SIZE ? MAX_FILE_NAME_SIZE : outLen, "%s", strmFileName);
    snprintf(&newAviFileName[strlen(newAviFileName)], MAX_FILE_NAME_SIZE - strlen(newAviFileName), ".avi");
    if(rename(aviFileName, newAviFileName) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "fail to rename avi file: [old=%s], [new=%s], [err=%s]", aviFileName, newAviFileName, STR_ERR);
    }

    close(streamFileFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives storage calculation related information.
 * @param   timeTakeToFullVolume
 * @param   writtingRateInMbps
 * @param   channelNo
 */
static void setStorageCalculationInfo(UINT64 timeTakeToFullVolume, UINT64 writtingRateInMbps, UINT8 channelNo)
{
    MUTEX_LOCK(storageCalculationInfo.writtingRateMutex);
    storageCalculationInfo.writtenDataSize[channelNo] = writtingRateInMbps;
    storageCalculationInfo.timeTakenToWriteData[channelNo] = timeTakeToFullVolume;
    MUTEX_UNLOCK(storageCalculationInfo.writtingRateMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives storage calculation related information.
 */
static void initStorageCalculationInfo(void)
{
    UINT8 camIndex;

    MUTEX_INIT(storageCalculationInfo.writtingRateMutex, NULL);
    MUTEX_LOCK(storageCalculationInfo.writtingRateMutex);
    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant() ; camIndex++)
    {
        storageCalculationInfo.writtenDataSize[camIndex] = 0;
        storageCalculationInfo.timeTakenToWriteData[camIndex] = 0;
    }
    MUTEX_UNLOCK(storageCalculationInfo.writtingRateMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives storage calculation related information
 * @param   timeTakeToFullVolume
 * @param   writtingRateInMbps
 */
void GetStorageCalculationInfo(UINT64PTR timeTakeToFullVolume, UINT64PTR writtingRateInMbps)
{
    UINT8 			camIndex;
    UINT64			cameraWrittingRate = 0;
    UINT64			timeTakenToFull = 0;
    HDD_CONFIG_t	hddConfig;
    DISK_SIZE_t		diskSize = {0};

    *timeTakeToFullVolume = 0;
    *writtingRateInMbps = 0;

    ReadHddConfig(&hddConfig);

    if(hddConfig.recordDisk == LOCAL_HARD_DISK)
    {
        GetVolumeSize(hddConfig.mode, &diskSize);
    }
    else
    {
        GetNasDriveSize((hddConfig.recordDisk - REMOTE_NAS1), &diskSize);
    }

    MUTEX_LOCK(storageCalculationInfo.writtingRateMutex);

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        if(storageCalculationInfo.timeTakenToWriteData[camIndex] == 0)
        {
            continue;
        }

        //calculate writing rate
        cameraWrittingRate += ((storageCalculationInfo.writtenDataSize[camIndex] * MSEC_IN_ONE_SEC)
                               / (storageCalculationInfo.timeTakenToWriteData[camIndex] * TIMER_RESOLUTION_MINIMUM_MSEC));
    }

    if(cameraWrittingRate != 0)
    {
        timeTakenToFull = (((UINT64)diskSize.totalSize * MEGA_BYTE) / cameraWrittingRate);
    }

    if((cameraWrittingRate != 0) && (timeTakenToFull != 0))
    {
        *writtingRateInMbps = cameraWrittingRate;
        *timeTakeToFullVolume = timeTakenToFull;
    }

    MUTEX_UNLOCK(storageCalculationInfo.writtingRateMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was gives status of Soucre is Availble or not
 * @param   backupType
 * @param   backupLocation
 * @return
 */
BOOL CheckDestinationStatus(DM_BACKUP_TYPE_e backupType, SB_LOCATION_e backupLocation)
{
    BOOL				status = FAIL;
    UINT8				mediaNo;
    CHAR				mntPoint[MOUNT_POINT_SIZE];
    CHAR				dstMountPoint[MOUNT_POINT_SIZE] = "\0";
    FTP_UPLOAD_CONFIG_t ftpConfig;
    struct timespec     ts;

    switch(backupLocation)
    {
        case SB_TO_USB_DEVICE:
        {
            mediaNo = (backupType + MANUAL_BACKUP_DISK);
            status = GetMountPointForBackupDevice(mediaNo, dstMountPoint);
            snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", dstMountPoint);
        }
        break;

        case SB_TO_NETWORK_DRIVE_1:
        case SB_TO_NETWORK_DRIVE_2:
        {
            mediaNo = ((backupLocation == SB_TO_NETWORK_DRIVE_1) ? BACKUP_ON_NAS1 : BACKUP_ON_NAS2) - BACKUP_ON_NAS1;  // NAS1 = 0, NAS2 = 1;
            status = GetMountPointFromNetworkDiskId(mediaNo, mntPoint);
            if (status == SUCCESS)
            {
                status = GetPathOfNetworkDrive(mediaNo, MATRIX_BACKUP_DIR, dstMountPoint);
            }
        }
        break;

        case SB_TO_FTP_SERVER_1:
        case SB_TO_FTP_SERVER_2:
        {
            ReadSingleFtpUploadConfig((backupLocation == SB_TO_FTP_SERVER_1) ? FTP_SERVER1 : FTP_SERVER2, &ftpConfig);
            if (ftpConfig.ftp == DISABLE)
            {
                break;
            }

            if (TestFtpConn(&ftpConfig, testFtpConnection, INVALID_CONNECTION) != CMD_SUCCESS)
            {
                break;
            }

            ftpTestUploadResponse = CMD_PROCESS_ERROR;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += FTP_CHECK_CONN_TIMEOUT;

            /* File was uploaded or not that was indicate by user callback so here we need to check already
             * uploading was in process then no need to upload another file. */
            MUTEX_LOCK(testFtpMutex);
            pthread_cond_timedwait(&testFtpCond, &testFtpMutex, &ts);
            MUTEX_UNLOCK(testFtpMutex);

            if (ftpTestUploadResponse == CMD_SUCCESS)
            {
                status = SUCCESS;
            }
        }
        break;

        default:
        {
            break;
        }
    }

    if (status == FAIL)
    {
        EPRINT(DISK_MANAGER, "proper mount point not present for backup: [backupLocation=%d]", backupLocation);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function recieves response using callback mechanism.
 * @param   status
 * @param   clientSocket
 * @param   closeConn
 */
static void testFtpConnection(NET_CMD_STATUS_e status, INT32 clientSocket, BOOL closeConn)
{
    ftpTestUploadResponse = status;
    MUTEX_LOCK(testFtpMutex);
    pthread_cond_signal(&testFtpCond);
    MUTEX_UNLOCK(testFtpMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   searchSingleChannelPresence
 * @param   pSeachCriteria
 * @return  SUCCESS/FAIL
 */
static BOOL searchSingleChannelPresence(SEARCH_CRITERIA_t *pSeachCriteria)
{
    BOOL                diskStatus[MAX_RECORDING_MODE];
    UINT8               recDriveId;
    UINT32              diskCnt, totalDiskCnt;
    CHAR                searchDir[MAX_FILE_NAME_SIZE];
    RECORD_ON_DISK_e    recDriveIndx;
    CHAR                mountPoint[MOUNT_POINT_SIZE];

    if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            diskStatus[recDriveId] = IsStorageOperationalForRead(recDriveId);
        }
    }

    for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
    {
        if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
        {
            recDriveIndx = recDriveId;
            if(diskStatus[recDriveIndx] == FALSE)
            {
                continue;
            }
        }
        else
        {
            recDriveIndx = pSeachCriteria->searchRecStorageType;
        }

        if(recDriveId != 0)
        {
            if(GetCurrentNddStatus(recDriveId) == FALSE)
            {
                continue;
            }
        }

        totalDiskCnt = GetTotalMediaNo(recDriveIndx);
        for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
        {
            if (GetMountPointFromDiskId(diskCnt, mountPoint, recDriveIndx) == FAIL)
            {
                continue;
            }

            // check for in in-active disk
            snprintf(searchDir, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL, mountPoint, pSeachCriteria->channelNo);
            if(access(searchDir, F_OK) != STATUS_OK)
            {
                continue;
            }

            /* Camera record found */
            return SUCCESS;
        }
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   searchStartTimePresence
 * @param   pSeachCriteria
 * @return
 */
static time_t searchStartTimePresence(SEARCH_CRITERIA_t *pSeachCriteria)
{
    BOOL                dateValue = FAIL;
    BOOL                hourValue = FAIL;
    UINT8               channelCnt;
    BOOL                diskStatus[MAX_RECORDING_MODE];
    UINT8               recDriveId;
    UINT32              diskCnt, totalDiskCnt;
    CHAR                searchDir[MAX_FILE_NAME_SIZE];
    time_t              startTimeSec = 0;
    time_t              stopTimeSec = 0;
    struct tm           evntStartTime;
    RECORD_ON_DISK_e    recDriveIndx;
    CHAR                mountPoint[MOUNT_POINT_SIZE];

    // convert event search criteria start time into time second because of comparing exact start time of event
    ConvertLocalTimeInSec(&pSeachCriteria->startTime, &startTimeSec);
    ConvertLocalTimeInSec(&pSeachCriteria->endTime, &stopTimeSec);
    evntStartTime = pSeachCriteria->startTime;

    if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
    {
        for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
        {
            diskStatus[recDriveId] = IsStorageOperationalForRead(recDriveId);
        }
    }

    while(startTimeSec < stopTimeSec)
    {
        for(channelCnt = 1; channelCnt <= getMaxCameraForCurrentVariant(); channelCnt++)
        {
            for(recDriveId = 0; recDriveId < MAX_RECORDING_MODE; recDriveId++)
            {
                if(pSeachCriteria->searchRecStorageType == ALL_DRIVE)
                {
                    recDriveIndx = recDriveId;
                    if (diskStatus[recDriveIndx] == FALSE)
                    {
                        continue;
                    }
                }
                else
                {
                    recDriveIndx = pSeachCriteria->searchRecStorageType;
                }

                if(recDriveId != 0)
                {
                    if(GetCurrentNddStatus(recDriveId) == FALSE)
                    {
                        continue;
                    }
                }

                totalDiskCnt = GetTotalMediaNo(recDriveIndx);
                for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
                {
                    if(GetMountPointFromDiskId(diskCnt, mountPoint, recDriveIndx) == SUCCESS)
                    {
                        // check for in in-active disk
                        snprintf(searchDir, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_FORMAT_SEP, mountPoint,
                                 channelCnt, evntStartTime.tm_mday, GetMonthName(evntStartTime.tm_mon), evntStartTime.tm_year);
                        if(access(searchDir, F_OK) != STATUS_OK)
                        {
                            searchDir[0] = '\0';
                        }
                        else
                        {
                            dateValue = SUCCESS;
                            searchDir[0] = '\0';
                            snprintf(searchDir, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mountPoint, channelCnt,
                                     evntStartTime.tm_mday, GetMonthName(evntStartTime.tm_mon), evntStartTime.tm_year, evntStartTime.tm_hour);
                            if(access(searchDir, F_OK) != STATUS_OK)
                            {
                                searchDir[0] = '\0';
                            }
                            else
                            {
                                hourValue = SUCCESS;
                                searchDir[0] = '\0';
                            }
                        }
                    }

                    if((dateValue == SUCCESS) || (hourValue == SUCCESS))
                    {
                        break;
                    }
                }

                if((dateValue == SUCCESS) || (hourValue == SUCCESS))
                {
                    break;
                }
            }

            if((dateValue == SUCCESS) || (hourValue == SUCCESS))
            {
                break;
            }
        }

        if((dateValue == SUCCESS) || (hourValue == SUCCESS))
        {
            break;
        }

        if(dateValue == FAIL)
        {
            ConvertLocalTimeInBrokenTm(&startTimeSec, &pSeachCriteria->startTime);
            startTimeSec = startTimeSec + (SEC_IN_ONE_MIN * (MIN_IN_ONE_HOUR - pSeachCriteria->startTime.tm_min));
            ConvertLocalTimeInBrokenTm(&startTimeSec, &evntStartTime);
        }
        else if(hourValue == FAIL)
        {
            startTimeSec += SEC_IN_ONE_HOUR;
            ConvertLocalTimeInBrokenTm(&startTimeSec, &evntStartTime);
        }
    }

    return startTimeSec;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on the error code and the disk passed below api take the decision either that disk is in fault or not.
 * @param   iErrorCode - Error code which occur while doing operation(read,write,mkdir etc) on the mounted disk.
 * @param   iDisk - Disk on which error has occured
 * @return  True if whole disk is considered in fault else false is return
 */
static BOOL isDiskInFault(UINT32 iErrorCode, RECORD_ON_DISK_e iDisk)
{
    BOOL tDiskFault = FALSE;

    /* validate parameters */
    if((iDisk >= MAX_RECORDING_MODE) || (INVALID_ERROR_CODE == iErrorCode))
    {
        return FALSE;
    }

    switch (iErrorCode)
    {
        /* Considered disk fault in below cases */
        case EIO:
        case EHOSTDOWN:
        case ENXIO:
        /* Access rights specific cases */
        case EROFS:
        {
            tDiskFault = TRUE;
            EPRINT(DISK_MANAGER, "disk is in fault: [disk=%d], [err=%s]", iDisk, strerror(iErrorCode));
        }
        break;

        /* Network drive specific cases */
        case ENETDOWN:
        case ENETUNREACH:
        case ECONNRESET:
        case EPIPE:
        case ETIMEDOUT:
        /* Access rights specific cases */
        case EACCES:
        case EPERM:
        {
            if((REMOTE_NAS1 == iDisk) || (REMOTE_NAS2 == iDisk))
            {
                tDiskFault = TRUE;
                EPRINT(DISK_MANAGER, "ndd is in fault: [ndd=%d], [err=%s]", iDisk, strerror(iErrorCode));
            }
        }
        break;

        case ENOSPC:
        {
            EPRINT(DISK_MANAGER, "there is no space left on the disk: [disk=%d]", iDisk);
        }
        break;

        /* consider all other cases as channel error */
        default:
        {
            /* Nothing to do */
        }
        break;
    }

    return tDiskFault;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function resets all buffer of each channel.
 */
void ResetAllChannelBuffer()
{
    UINT8 channelNo;

    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        ResetChannelBuffer(channelNo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This api recovers the i-frame file and stream file and terminates the files if not properly terminated.
 * @param   pHourFolder - Hour folder in which recovery needs to be done.
 * @param   pErrorCode  - In case of any I/O operation error code is filled in this variable.
 * @return  TRUE on success else FALSE is return
 */
static BOOL recoverStreamAndIFrameFile(CHAR *pHourFolder, UINT32PTR pErrorCode)
{
    CHAR                tIFrameFile[MAX_FILE_NAME_SIZE] = "";
    INT32               tIFrameFileFd   = INVALID_FILE_FD;
    struct stat         tIFrameFileInfo = { 0 };
    UINT32              tIFrameFileSize = 0;
    CHAR                *tpIFrameFileData = NULL;
    CHAR                *tpIFrameFileDataTemp = NULL;
    METADATA_FILE_HDR_t *tpIFrameFileHeader = NULL;
    IFRM_FIELD_INFO_t	*tpIFrameFieldInfo = NULL;
    CHAR                tStreamFileName[MAX_FILE_NAME_SIZE] = "";
    CHAR                tNewStreamFileName[MAX_FILE_NAME_SIZE] = "";
    UINT32              tStreamFileSize = 0;
    INT32               tStreamFileFd = INVALID_FILE_FD;
    STRM_FILE_END_t 	tStreamFileEndIndicator = { 0 };
    FSH_INFO_t          tFshData = { 0 };
    STRM_FILE_HDR_t     tStreamFileHdr = { 0 };
    struct tm           tStreamFileEndTime = { 0 };
    CHAR                *tpNewFileName = NULL;
    INT16               tPrevStmFileId = -1;
    UINT32				strmFileCnt, tStrmFileId;
    BOOL                tRetVal = FALSE;

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);
    if(NULL == pHourFolder)
    {
        EPRINT(DISK_MANAGER, "invld hour folder for recovery");
        return FALSE;
    }

    do
    {
        // Get total number of stream file in current folder
        if(getAllStreamFileCount(pHourFolder, &strmFileCnt, NULL) == FAIL)
        {
            EPRINT(DISK_MANAGER, "stream file not found: [path=%s]", pHourFolder);
        }

        /* We allow stream file id upto 255 (.stm1 to .stm255) */
        if (strmFileCnt > STREAM_FILE_STM_ID_MAX)
        {
            /* Remove all 0 byte stream files. There was issue in older firmware in which if hard-disk gets full (no space left on device)
             * due to any reason, in that case stream file was getting created without data (0 byte). Due to that, thousands of file was getting created. */
            EPRINT(DISK_MANAGER, "stream file count discovered more than allowed: [path=%s], [strmFileCnt=%d]", pHourFolder, strmFileCnt);
            removeBlankStreamFiles(pHourFolder);
            if (getAllStreamFileCount(pHourFolder, &strmFileCnt, NULL) == FAIL)
            {
                EPRINT(DISK_MANAGER, "stream file not found: [path=%s]", pHourFolder);
            }
        }

        /* Truncate file count to 255 if found more than that */
        DPRINT(DISK_MANAGER, "stream file count info: [path=%s], [strmFileCnt=%d]", pHourFolder, strmFileCnt);
        if (strmFileCnt > STREAM_FILE_STM_ID_MAX)
        {
            strmFileCnt = STREAM_FILE_STM_ID_MAX;
            WPRINT(DISK_MANAGER, "stream file count truncated: [path=%s], [strmFileCnt=%d]", pHourFolder, strmFileCnt);
        }

         /* prepare I-frame file name */
        snprintf(tIFrameFile, MAX_FILE_NAME_SIZE, "%s%s", pHourFolder, I_FRAME_FILE_NAME);

        /* open i-frame file */
        tIFrameFileFd = open(tIFrameFile, READ_WRITE_MODE);
        if(INVALID_FILE_FD == tIFrameFileFd)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to open the i-frame file: [path=%s], [err=%s]", tIFrameFile, STR_ERR);
            return FALSE;
        }

        /* get size of I-frame file */
        if(STATUS_OK != fstat(tIFrameFileFd, &tIFrameFileInfo))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to stat i-frame file: [path=%s], [err=%s]", tIFrameFile, STR_ERR);
            break;
        }

        tIFrameFileSize = tIFrameFileInfo.st_size;

        /* validate the size of i-frame file */
        if(tIFrameFileSize < (IFRAME_FILE_HDR_SIZE + IFRAME_FIELD_SIZE))
        {
            EPRINT(DISK_MANAGER, "invld i-frame file size: [path=%s], [size=%d]", tIFrameFile, tIFrameFileSize);
            break;
        }

        /* allocate memory for the I-frame data */
        tpIFrameFileData = malloc(tIFrameFileSize);
        if(NULL == tpIFrameFileData)
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to allocate memory for the i-frame file data: [path=%s], [err=%s]", tIFrameFile, STR_ERR);
            break;
        }

        if((ssize_t)tIFrameFileSize != Utils_Read(tIFrameFileFd, tpIFrameFileData, tIFrameFileSize, pErrorCode))
        {
            EPRINT(DISK_MANAGER, "fail to read i-frame file: [path=%s], [err=%s]", tIFrameFile, strerror(*pErrorCode));
            break;
        }

        /* read i-frame header */
        tpIFrameFileHeader = (METADATA_FILE_HDR_t *)tpIFrameFileData;

        /* check total data size according to header and actual data present in the file should be in sync */
        if(tpIFrameFileHeader->nextMetaDataIdx > ((UINT32)(tIFrameFileSize - IFRAME_FILE_HDR_SIZE) / IFRAME_FIELD_SIZE))
        {
            DPRINT(DISK_MANAGER, "updating i-frame index in header: [path=%s], [prev=%d], [new=%d], [file_size=%d], [header_size=%d], [field_size=%d]",
                   tIFrameFile, tpIFrameFileHeader->nextMetaDataIdx, ((tIFrameFileSize - IFRAME_FILE_HDR_SIZE) / IFRAME_FIELD_SIZE),
                   tIFrameFileSize, IFRAME_FILE_HDR_SIZE, IFRAME_FIELD_SIZE);
            tpIFrameFileHeader->nextMetaDataIdx = ((UINT32)(tIFrameFileSize - IFRAME_FILE_HDR_SIZE) / IFRAME_FIELD_SIZE);
        }

        /* validate the i-frame data is present in the stream file or not. if not remove the entry from the i-frame file */
        while(tpIFrameFileHeader->nextMetaDataIdx)
        {
            /* read last i-frame data */
            tpIFrameFieldInfo = (IFRM_FIELD_INFO_t *)(tpIFrameFileData +
                                                      ((tpIFrameFileHeader->nextMetaDataIdx - 1) * IFRAME_FIELD_SIZE) + IFRAME_FILE_HDR_SIZE);

            /* while traversing i-frame data if stream id changes then close previous stream file and open new one */
            if(tPrevStmFileId != tpIFrameFieldInfo->strmFileId)
            {
                CloseFileFd(&tStreamFileFd);

                /* get stream file name from stream file id */
                if(FAIL == getStreamFileName(pHourFolder, tpIFrameFieldInfo->strmFileId, tStreamFileName, &tStreamFileSize))
                {
                    /* this should not be treated as an error, in this case only discard the i-frame file */
                    tpIFrameFileHeader->nextMetaDataIdx -= 1;
                    EPRINT(DISK_MANAGER, "fail to get stream file name: [path=%s], [strmFileId=%d]", pHourFolder, tpIFrameFieldInfo->strmFileId);
                    continue;
                }

                /* open stream file and check i-frame is present or not */
                tStreamFileFd = open(tStreamFileName, READ_WRITE_MODE);
                if(tStreamFileFd == INVALID_FILE_FD)
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to open the stream file: [path=%s], [err=%s]", tStreamFileName, STR_ERR);
                    break;
                }

                /* update the prev stream file id */
                tPrevStmFileId = tpIFrameFieldInfo->strmFileId;
                DPRINT(DISK_MANAGER, "recover stream file: [path=%s] [size=%d]", tStreamFileName, tStreamFileSize);
            }

            /* check stream file have FSH of that i-frame or not */
            if(tStreamFileSize <= (tpIFrameFieldInfo->curStrmPos + MAX_FSH_SIZE))
            {
                tpIFrameFileHeader->nextMetaDataIdx -= 1;
                continue;
            }

            /* seek to the i-frame in stream file */
            if(-1 == lseek(tStreamFileFd, tpIFrameFieldInfo->curStrmPos, SEEK_SET))
            {
                SET_ERROR_NUMBER(pErrorCode, errno);
                EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [err=%s]", tStreamFileName, STR_ERR);
                break;
            }

            /* read the fsh from the stream file */
            if(MAX_FSH_SIZE != Utils_Read(tStreamFileFd, &tFshData, MAX_FSH_SIZE, pErrorCode))
            {
                EPRINT(DISK_MANAGER, "fail to read stream file: [file=%s], [err=%s]", tStreamFileName, STR_ERR);
                break;
            }

            /* invalid FSH found or frame data is not present so remove i-frame entry from i-frame file data */
            if((tFshData.startCode != FSH_START_CODE) || (tStreamFileSize < (tpIFrameFieldInfo->curStrmPos + tFshData.fshLen)))
            {
                tpIFrameFileHeader->nextMetaDataIdx -= 1;
                continue;
            }

            /* update stream file size */
            tIFrameFileSize = (tpIFrameFileHeader->nextMetaDataIdx * IFRAME_FIELD_SIZE) + IFRAME_FILE_HDR_SIZE;
            tRetVal = TRUE;
            break;
        }

        /* no i-frame is found in the stream file */
        if((FALSE == tRetVal) || (0 == tpIFrameFileHeader->nextMetaDataIdx))
        {
            EPRINT(DISK_MANAGER, "no i-frame found: [path=%s]", tIFrameFile);
            break;
        }

        tRetVal = FALSE;

        /* truncate the file and discard all frames */
        if(-1 == ftruncate(tStreamFileFd, tFshData.nextFShPos))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to truncate stream file: [path=%s], [err=%s]", tStreamFileName, STR_ERR);
            break;
        }

        /* set pointer at the beginning to update header */
        if (-1 == lseek(tStreamFileFd, 0, SEEK_SET))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to seek stream file: [path=%s], [err=%s]", tStreamFileName, STR_ERR);
            break;
        }

        /* read header from the stream file */
        if(STREAM_FILE_HDR_SIZE != Utils_Read(tStreamFileFd, &tStreamFileHdr, STREAM_FILE_HDR_SIZE, pErrorCode))
        {
            EPRINT(DISK_MANAGER, "fail to read the header from the stream file: [path=%s], [err=%s]", tStreamFileName, strerror(*pErrorCode));
            break;
        }

        /* update the stream file header */
        tStreamFileHdr.nextStreamPos = tFshData.nextFShPos;
        tStreamFileHdr.runFlg = FALSE;
        if(STREAM_FILE_HDR_SIZE != Utils_pWrite(tStreamFileFd, &tStreamFileHdr, STREAM_FILE_HDR_SIZE, 0, pErrorCode))
        {
            EPRINT(DISK_MANAGER, "fail to write stream file header: [path=%s], [err=%s]", tStreamFileName, strerror(*pErrorCode));
            break;
        }

        /* add end of file indicator in the stream file */
        tStreamFileEndIndicator.fileEndIndicator = STRM_EOF_INDICATOR;
        tStreamFileEndIndicator.prevFshPos = tpIFrameFieldInfo->curStrmPos;
        if(STREAM_FILE_END_SIZE != Utils_pWrite(tStreamFileFd, &tStreamFileEndIndicator, STREAM_FILE_END_SIZE, lseek(tStreamFileFd, 0, SEEK_END), pErrorCode ))
        {
            EPRINT(DISK_MANAGER, "fail to write stream file eof: [path=%s], [err=%s]", tStreamFileName, strerror(*pErrorCode));
            break;
        }

        /* if stream file is not terminated properly then terminate the file properly */
        if(NULL != strstr(tStreamFileName, STREAM_FILE_NOT_COMPLETE))
        {
            /* get time from the last fsh data */
            time_t totalSec = tFshData.localTime.totalSec;
            ConvertLocalTimeInBrokenTm(&totalSec, &tStreamFileEndTime);

            /* copy stream file name */
            snprintf(tNewStreamFileName, MAX_FILE_NAME_SIZE, "%s", tStreamFileName);

            tpNewFileName = strchr(tNewStreamFileName, '~');
            if(tpNewFileName)
            {
                tpNewFileName++;
                /* change stream file name */
                snprintf(tpNewFileName, MAX_FILE_NAME_SIZE, REC_FOLDER_TIME_FORMAT, tStreamFileEndTime.tm_hour, tStreamFileEndTime.tm_min, tStreamFileEndTime.tm_sec);

                /* add '.' extension is already present in new stream file name */
                tNewStreamFileName[strlen(tNewStreamFileName)] = '.';
                if(STATUS_OK != rename(tStreamFileName, tNewStreamFileName))
                {
                    SET_ERROR_NUMBER(pErrorCode, errno);
                    EPRINT(DISK_MANAGER, "fail to rename stream file: [old=%s], [new=%s], [err=%s]", tStreamFileName, tNewStreamFileName, STR_ERR);
                    break;
                }
            }
        }

        /* Store last stream file id before reallocating memory */
        tPrevStmFileId = tpIFrameFieldInfo->strmFileId;

        /* To write eof indicator(of 2 bytes) check enough space is present in the buffer or not, If not realloc the memory,
         * this check is needed if in case we have removed the i-file data from the buffer then we already have enough bytes
         * in the memory so in that case there is no need for reallocating the memory */
        if((tIFrameFileSize + 2) > (UINT32)tIFrameFileInfo.st_size)
        {
            tpIFrameFileDataTemp = realloc(tpIFrameFileData, (tIFrameFileSize + 2));
            if(NULL == tpIFrameFileDataTemp)
            {
                EPRINT(DISK_MANAGER, "fail to reallocate memory: [path=%s]", tStreamFileName);
                break;
            }
            tpIFrameFileData = tpIFrameFileDataTemp;
        }

        /* add i-frame eof indicator in the i-frame buffer */
        tpIFrameFileData[tIFrameFileSize++] = (UINT8)IFRAME_FILE_EOF;
        tpIFrameFileData[tIFrameFileSize++] = (IFRAME_FILE_EOF >> 8);

        /* write the updated data(header + i-frame data) in the i-frame file */
        if((ssize_t)tIFrameFileSize != Utils_Write(tIFrameFileFd, tpIFrameFileData, tIFrameFileSize, pErrorCode))
        {
            EPRINT(DISK_MANAGER, "fail to update i-frame data: [path=%s], [err=%s]", tIFrameFile, strerror(*pErrorCode));
            break;
        }

        /* truncate extra bytes from the i-frame meta data file */
        if(-1 == ftruncate(tIFrameFileFd, tIFrameFileSize))
        {
            SET_ERROR_NUMBER(pErrorCode, errno);
            EPRINT(DISK_MANAGER, "fail to truncate i-frame file: [path=%s], [err=%s]", tIFrameFile, STR_ERR);
            break;
        }

        /* remove unrecovered stream files */
        for(tStrmFileId = (tPrevStmFileId + 1); tStrmFileId <= strmFileCnt; tStrmFileId++)
        {
            /* get stream file name from stream file id */
            if(FAIL == getStreamFileName(pHourFolder, tStrmFileId, tStreamFileName, &tStreamFileSize))
            {
                EPRINT(DISK_MANAGER, "fail to find the stream file: [path=%s], [strmFileId=%d]", pHourFolder, tStrmFileId);
                continue;
            }

            DPRINT(DISK_MANAGER, "remove unrecovered stream file: [path=%s], [size=%d]", tStreamFileName, tStreamFileSize);
            unlink(tStreamFileName);
        }

        tRetVal = TRUE;

    }while(0);

    CloseFileFd(&tStreamFileFd);
    CloseFileFd(&tIFrameFileFd);
    FREE_MEMORY(tpIFrameFileData);
    return tRetVal;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
