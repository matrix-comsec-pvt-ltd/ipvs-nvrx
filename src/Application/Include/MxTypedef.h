#if !defined MXTYPEDEF_H
#define MXTYPEDEF_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxTypedef.h
@brief      This file declares all project specific data type, defines  and constants that are used
            in the entire project. All source file (.c) should include this file.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

/* Application Includes */
#include "MxCmsStatusCode.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Matrix folder subdirectory paths */
#if defined(OEM_JCI)
#define MATRIX_DIR_PATH         "/nvr"
#else
#define MATRIX_DIR_PATH         "/matrix"
#endif

#define BIN_DIR_PATH            MATRIX_DIR_PATH "/bin"
#define CONFIG_DIR_PATH         MATRIX_DIR_PATH "/config"
#define EVENT_DIR_PATH          MATRIX_DIR_PATH "/event"
#define KERNEL_DIR_PATH         MATRIX_DIR_PATH "/kernel"
#define LANGUAGES_DIR_PATH      MATRIX_DIR_PATH "/languages"
#define LIB_DIR_PATH            MATRIX_DIR_PATH "/lib"
#define LOG_DIR_PATH            MATRIX_DIR_PATH "/log"
#define MEDIA_DIR_PATH          MATRIX_DIR_PATH "/media"
#define SCRIPTS_DIR_PATH        MATRIX_DIR_PATH "/scripts"
#define WEB_DIR_PATH            MATRIX_DIR_PATH "/web"
#define ZIP_DIR_PATH            MATRIX_DIR_PATH "/zip"

/* Socket related common options for socket creation */
#define TCP_SOCK_OPTIONS        (SOCK_STREAM | SOCK_CLOEXEC)
#define TCP_NB_SOCK_OPTIONS     (SOCK_NONBLOCK | TCP_SOCK_OPTIONS)
#define UDP_SOCK_OPTIONS        (SOCK_DGRAM | SOCK_CLOEXEC)
#define UDP_NB_SOCK_OPTIONS     (SOCK_NONBLOCK | UDP_SOCK_OPTIONS)

/* File related common options for file operations */
#define	READ_ONLY_MODE          (O_RDONLY | O_CLOEXEC)
#define	READ_SYNC_MODE          (O_SYNC | READ_ONLY_MODE)

#define	WRITE_ONLY_MODE         (O_WRONLY | O_CLOEXEC)
#define	WRITE_SYNC_MODE         (O_SYNC | WRITE_ONLY_MODE)

#define	READ_WRITE_MODE         (O_RDWR | O_CLOEXEC)
#define	READ_WRITE_SYNC_MODE    (O_SYNC | READ_WRITE_MODE)

#define	CREATE_READ_MODE        (O_CREAT | READ_ONLY_MODE)
#define	CREATE_READ_SYNC_MODE   (O_SYNC | CREATE_READ_MODE)

#define	CREATE_WRITE_MODE       (O_CREAT | WRITE_ONLY_MODE)
#define	CREATE_WRITE_SYNC_MODE  (O_SYNC | CREATE_WRITE_MODE)

#define	CREATE_RDWR_MODE        (O_CREAT | READ_WRITE_MODE)
#define	CREATE_RDWR_SYNC_MODE   (O_SYNC | CREATE_RDWR_MODE)

/* File or folder related common permissions for file or folder operations */
#define USR_RD_WR_EX            (S_IRWXU)
#define USR_RD                  (S_IRUSR)
#define USR_WR                  (S_IWUSR)
#define USR_EX                  (S_IXUSR)
#define USR_RD_WR               (S_IRUSR | S_IWUSR)
#define USR_RD_EX               (S_IRUSR | S_IXUSR)

#define GRP_RD_WR_EX            (S_IRWXG)
#define GRP_RD                  (S_IRGRP)
#define GRP_WR                  (S_IWGRP)
#define GRP_EX                  (S_IXGRP)
#define GRP_RD_WR               (S_IRGRP | S_IWGRP)
#define GRP_RD_EX               (S_IRGRP | S_IXGRP)

#define OTH_RD_WR_EX            (S_IRWXO)
#define OTH_RD                  (S_IROTH)
#define OTH_WR                  (S_IWOTH)
#define OTH_EX                  (S_IXOTH)
#define OTH_RD_WR               (S_IROTH | S_IWOTH)
#define OTH_RD_EX               (S_IROTH | S_IXOTH)

#define USR_RWE_GRP_RWE_OTH_RWE (USR_RD_WR_EX | GRP_RD_WR_EX | OTH_RD_WR_EX)
#define USR_RWE_GRP_RW_OTH_RW   (USR_RD_WR_EX | GRP_RD_WR | OTH_RD_WR)
#define USR_RWE_GRP_RE_OTH_RE   (USR_RD_WR_EX | GRP_RD_EX | OTH_RD_EX)
#define USR_RW_GRP_RW_OTH_RW    (USR_RD_WR | GRP_RD_WR | OTH_RD_WR)
#define USR_RW_GRP_R_OTH_R      (USR_RD_WR | GRP_RD | OTH_RD)
#define USR_R_GRP_R_OTH_R       (USR_RD | GRP_RD | OTH_RD)

/* Memory calculation related macros */
#define KILO_BYTE 				(1024)
#define MEGA_BYTE 				(1048576)       /* (KILO_BYTE * KILO_BYTE) */
#define GIGA_BYTE				(1073741824)    /* (MEGA_BYTE * KILO_BYTE) */

/* Nano seconds in 1 millisec */
#define NANO_SEC_PER_MILLI_SEC  1000000LL

/* Nano seconds in 1 sec */
#define NANO_SEC_PER_SEC        1000000000LL

/* Micro seconds in 1 sec */
#define MICRO_SEC_PER_SEC       1000000

/* Milli seconds in 1 sec */
#define MILLI_SEC_PER_SEC       1000

/* Micro seconds in 1 ms */
#define MICRO_SEC_PER_MS        1000

/* Max ethernet MTU size */
#define NETWORK_MTU_SIZE_MAX    1500

// total second in one minute
#define	MSEC_IN_ONE_SEC			(1000)
#define	SEC_IN_ONE_MIN			(60)
#define	MIN_IN_ONE_HOUR			(60)
#define SEC_IN_ONE_HOUR			(3600)  /* (MIN_IN_ONE_HOUR * SEC_IN_ONE_MIN) */
#define HOUR_IN_ONE_DAY			(24)
#define SEC_IN_ONE_DAY          (86400) /* (days * HOUR_IN_ONE_DAY * SEC_IN_ONE_HOUR) */
#define MAX_DAYS				(31)

#define MAX_VOLUME              (MAX_RAID_VOLUME_GRP * MAX_RAID_VOLUME)

#define IPV4_ADDR_LEN_MAX       (INET_ADDRSTRLEN)
#define IPV6_ADDR_LEN_MAX       (INET6_ADDRSTRLEN)
#define IPV6_NW_ADDR_U32_SIZE   (4)
#define INTERFACE_NAME_LEN_MAX  (IF_NAMESIZE)
#define MAX_CAMERA_URI_WIDTH	(500)
#define INVALID_CAMERA_INDEX    255
#define INVALID_VALUE			255
#define DOMAIN_NAME_SIZE_MAX	256

#define NILL                    (-1)
#define INVALID_FILE_FD         (-1)
#define INVALID_CONNECTION      (-1)
#define STATUS_OK               (0)

#define MAX_FW_FILE_NAME_LEN        (50)
#define MAX_RESTORE_FILE_NAME_LEN   (50)
#define MAX_LANGUAGE_FILE_NAME_LEN  (201)
#define TEMP_DIR_PATH				"/tmp/"
#define RAMFS_DIR_PATH				"/mnt/ramfs/"
#define SAMPLE_CSV_FILE_NAME		"English.csv"
#define SAMPLE_LANGUAGE_FILE		LANGUAGES_DIR_PATH "/" SAMPLE_CSV_FILE_NAME
#define LANGUAGE_UPDATE_NOTIFY		LOG_DIR_PATH "/updateNotify.txt"
#define MAX_LANGUAGE				6
#define MAX_LABEL_STRING_SIZE		1024

/* Macros for camera bit conversion */
#define CAMERA_MASK_MAX                     2
#define CAMERA_BIT_WISE_MAX                 64
#define GET_CAMERA_MASK_IDX(camera)         ((camera)/CAMERA_BIT_WISE_MAX)
#define GET_CAMERA_BIT_IDX(camera)          ((camera)%CAMERA_BIT_WISE_MAX)
#define GET_CAMERA_MASK_BIT(mask, camera)   GET_BIT(mask.bitMask[GET_CAMERA_MASK_IDX(camera)], GET_CAMERA_BIT_IDX(camera))
#define SET_CAMERA_MASK_BIT(mask, camera)   SET_BIT(mask.bitMask[GET_CAMERA_MASK_IDX(camera)], GET_CAMERA_BIT_IDX(camera))
#define CLR_CAMERA_MASK_BIT(mask, camera)   RESET_BIT(mask.bitMask[GET_CAMERA_MASK_IDX(camera)], GET_CAMERA_BIT_IDX(camera))
#define IS_ALL_CAMERA_MASK_BIT_CLR(mask)    ((mask.bitMask[0] == 0) && (mask.bitMask[1] == 0))

/* Skip the first 'n' characters of the given string and return a pointer to the resulting substring */
#define SKIP_STRING_PREFIX(str, n)  (str + n)

/* Get video stream type */
#define GET_VIDEO_STREAM_TYPE(cam)  ((cam < getMaxCameraForCurrentVariant()) ? MAIN_STREAM : SUB_STREAM)

/* Case to string conversion using token pasting with skipping common prefix of enum */
#define	CASE_TO_STR(x, skip)        case x: return (#x+skip);

/* Initialization of mutex */
#define MUTEX_INIT(x, attr)         pthread_mutex_init(&x, attr)

/* Define to acquire mutex lock */
#define MUTEX_LOCK(x)               pthread_mutex_lock(&x)

/* Define to release mutex lock */
#define MUTEX_UNLOCK(x)             pthread_mutex_unlock(&x)

/* Free memory after checking null */
#define FREE_MEMORY(x)              if (x != NULL) {free(x); x = NULL;}

/* Reset string buffer with null */
#define RESET_STR_BUFF(x)           x[0] = '\0'

/* Get value in bool (true or false) */
#define GET_BOOL_VALUE(x)           ((x) ? TRUE : FALSE)

#define SET_ALL_BIT(shift)          ((1ULL << (shift)) - 1)
#define RESET_BIT(value, shift)     ((value) &= ~(1ULL << (shift)))
#define SET_BIT(value, shift)       ((value) |= (1ULL << (shift)))
#define GET_BIT(value, shift)       (((value) >> (shift)) & 1)

/* Print linux error in string */
#define STR_ERR                     strerror(errno)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef unsigned char       BOOL;

typedef char                CHAR;

typedef signed char         INT8;
typedef unsigned char       UINT8;

typedef signed short        INT16;
typedef unsigned short      UINT16;

typedef signed int          INT32;
typedef unsigned int        UINT32;

typedef signed long long    INT64;
typedef unsigned long long  UINT64;

typedef INT8*               INT8PTR;
typedef CHAR*               CHARPTR;
typedef UINT8*              UINT8PTR;

typedef INT16*              INT16PTR;
typedef UINT16*             UINT16PTR;

typedef INT32*              INT32PTR;
typedef UINT32*             UINT32PTR;

typedef INT64*              INT64PTR;
typedef UINT64*             UINT64PTR;

typedef void*               VOIDPTR;

typedef enum
{
	MAIN_STREAM,
	SUB_STREAM,
	MAX_STREAM

}VIDEO_TYPE_e;

enum
{
	CLEAR = 0,
	SET,

	DISABLE = 0,
	ENABLE,

	OFF = 0,
	ON,
	WAIT,

	FALSE = 0,
	TRUE,

	NO = 0,
	YES,

	FREE = 0,
	BUSY,
	PENDING,

	INACTIVE = 0,
	ACTIVE,
	INTERRUPTED,

	FAIL = 0,
	SUCCESS,
	REFUSE,
    IN_PROGRESS,
    TIMEOUT,

	STOP = 0,
	START,
	RESTART,

	REMOVED = 0,
	ADDED,
	UNKNOWN,
};

typedef enum
{
	JANUARY = 0,				// January
	FEBRUARY,					// February
	MARCH,						// March
	APRIL,						// April
	MAY,						// May
	JUNE,						// June
	JULY,						// July
	AUGUST,						// August
	SEPTEMBER,					// September
	OCTOBER,					// October
	NOVEMBER,					// November
	DECEMBER,					// December
	MAX_MONTH					// Max number of months
}MONTH_e;

typedef enum
{
	SUNDAY = 0,					// Sunday
	MONDAY,						// Monday
	TUESDAY,					// Tuesday
	WEDNESDAY,					// Wednesday
	THURSDAY,					// Thursday
	FRIDAY,						// Friday
	SATURDAY,					// Saturday
	MAX_WEEK_DAYS				// Max number of week-days
}WEEK_DAY_e;

typedef enum
{
	FIRST_WEEK = 0,				// First week of the month
	SECOND_WEEK,				// Second week of the month
	THIRD_WEEK,					// Third week of the month
	FOURTH_WEEK,				// Fourth week of the month
	LAST_WEEK,					// Last week of the month
	MAX_WEEK					// max number of weeks
}WEEK_e;

typedef enum
{
	STREAM_TYPE_VIDEO,
	STREAM_TYPE_AUDIO,
	MAX_STREAM_TYPE
}STREAM_TYPE_e;

typedef enum
{
	I_FRAME,
	P_FRAME,
	B_FRAME,
	FRAME_TYPE_SEI,
	VPS_FRAME,
	SPS_FRAME,
	PPS_FRAME,
	MAX_FRAME_TYPE
}FRAME_TYPE_e;

typedef enum
{
    VIDEO_CODEC_NONE    = 0,
    VIDEO_MJPG          = 1,    // Motion JPEG encoding
    VIDEO_H264          = 2,    // H264 encoding
    VIDEO_MPEG4         = 3,    // MPEG4 encoding
    VIDEO_CODEC_UNUSED  = 4,    // Unused encoding (Was allocated to mobotix)
    VIDEO_H265          = 5,
	MAX_VIDEO_CODEC,

    AUDIO_CODEC_NONE    = 0,
    AUDIO_G711_ULAW     = 1,
    AUDIO_G726_8        = 2,
    AUDIO_G726_16       = 3,
    AUDIO_G726_24       = 4,
    AUDIO_G726_32       = 5,
    AUDIO_G726_40       = 6,
    AUDIO_AAC           = 7,
    AUDIO_PCM_L         = 8,
    AUDIO_PCM_B         = 9,
    AUDIO_G711_ALAW     = 10,
	MAX_AUDIO_CODEC,

    MAX_CODEC_TYPE      = 0xFF

}STREAM_CODEC_TYPE_e;

typedef enum
{
    MATRIX_IP_CAM_RESOLUTION_8MP,
    MATRIX_IP_CAM_RESOLUTION_6MP,
    MATRIX_IP_CAM_RESOLUTION_5MP,
	MATRIX_IP_CAM_RESOLUTION_4MP,
	MATRIX_IP_CAM_RESOLUTION_3MP,
	MATRIX_IP_CAM_RESOLUTION_1080P,
	MATRIX_IP_CAM_RESOLUTION_720P,
	MATRIX_IP_CAM_RESOLUTION_D1,
	MATRIX_IP_CAM_RESOLUTION_2CIF,
    MATRIX_IP_CAM_RESOLUTION_VGA,
	MATRIX_IP_CAM_RESOLUTION_CIF,
	MATRIX_IP_CAM_RESOLUTION_QCIF,
	MATRIX_IP_CAM_MAX_RESOLUTION

}MATRIX_IP_CAM_RESOLUTION_e;

typedef struct
{
	UINT8 		hour;			// hour count
	UINT8 		minute;			// minute count
}TIME_HH_MM_t;

typedef struct
{
	WEEK_DAY_e 	weekDay;		// specific week day of week
	WEEK_e 		week;			// specific week of the month
	MONTH_e 	month;			// specific month of the year
}DAY_WD_WK_MM_t;

typedef enum
{
#if defined(OEM_JCI)
    HRIN_NVR_NONE = 0,
    HRIN_1208_18_SR,
    HRIN_2808_18_SR,
    HRIN_4808_18_SR,
    HRIN_6408_18_SR,
#else
    NVR_NONE = 0,
    HVR0408S,
    HVR0408P,
    HVR0824S,
    HVR0824P,
    HVR1624S,
    HVR1624P,
    NVR08S,
    NVR16S,
    NVR16H,
    NVR24P,
    NVR64S,
    NVR64P,
    NVR0801X,
    NVR1601X,
    NVR1602X,
    NVR3202X,
    NVR3204X,
    NVR6404X,
    NVR6408X,
    NVR9608X,
    NVR0801XP2,
    NVR1601XP2,
    NVR1602XP2,
    NVR3202XP2,
    NVR3204XP2,
    NVR6404XP2,
    NVR6408XP2,
    NVR0801XSP2,
    NVR1601XSP2,
    NVR0401XSP2,
    NVR9608XP2,
#endif
    NVR_VARIANT_MAX
}NVR_VARIANT_e;

typedef struct
{
    UINT32  softwareVersion;
    UINT32  softwareRevision;
    UINT32  commVersion;
    UINT32  commRevision;
    UINT32  responseTime;
    UINT32  KLVTime;
    UINT32  maxCameras;
    UINT32  maxAnalogCameras;
    UINT32  maxIpCameras;
    UINT32  configuredAnalogCameras;
    UINT32  configuredIpCameras;
    UINT32  maxSensorInput;
    UINT32  maxAlarmOutput;
    UINT32  audioIn;
    UINT32  audioOut;
    UINT32  noOfHdd;
    UINT32  noOfNdd;
    UINT32  noOfLanPort;
    UINT32  noOfVGA;
    UINT32  HDMI1;
    UINT32  HDMI2;
    UINT32  CVBSMain;
    UINT32  CVBSSpot;
    UINT32  CVBSSpotAnalog;
    UINT32  anlogPTZSupport;
    UINT32  USBPort;
    UINT32  maxMainAnalogResolution;
    UINT32  maxSubAnalogResolution;
    UINT32  videoStandard;
    UINT32  maxMainEncodingCap;
    UINT32  maxSubEncodingCap;
    UINT32  diskCheckingReq;
    UINT32  productVariant;
    UINT32  productSubRevision;

}NVR_DEVICE_INFO_t;

typedef struct
{
    CHAR    ip[IPV4_ADDR_LEN_MAX];
    UINT16  port;

}IP_ADDR_INFO_t;

typedef union
{
    struct sockaddr     sockAddr;
    struct sockaddr_in  sockAddr4;
    struct sockaddr_in6 sockAddr6;

}SOCK_ADDR_INFO_u;

typedef union
{
    in_addr_t       ip4;
    struct in6_addr ip6;

}IP_NW_ADDR_u;

typedef enum
{
    IPV6_ADDR_GLOBAL,
    IPV6_ADDR_LINKLOCAL,
    MAX_IPV6_ADDR_TYPE,

}IPV6_ADDR_TYPE_e;

typedef struct
{
    UINT64 bitMask[CAMERA_MASK_MAX];

}CAMERA_BIT_MASK_t;

/** Following client callback type is defined to differenciate between existing callback/sendtosocket APIs and new introducuded APIs for P2P */
typedef enum
{
    CLIENT_CB_TYPE_NATIVE, /* Native TCP (Dedicated socket for every request/response) */
    CLIENT_CB_TYPE_P2P,    /* Peer to Peer (Every request/response only on single socket) */
    CLIENT_CB_TYPE_MAX

}CLIENT_CB_TYPE_e;

//-------------------------------------------------------------------------------------------------
typedef void (*NW_CMD_REPLY_CB)(NET_CMD_STATUS_e status, INT32 clientSocket, BOOL closeConnF);
//-------------------------------------------------------------------------------------------------
typedef BOOL (*SEND_TO_SOCKET_CB)(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
typedef INT32 (*SEND_TO_CLIENT_CB)(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
typedef void (*CLOSE_SOCKET_CB)(INT32 *pConnFd);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif	// MXSYSTYPEDEF_H_
