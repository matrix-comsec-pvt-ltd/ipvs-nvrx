//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkCommand.c
@brief      This file contains functions to process the commands and send response back to client.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>
#include <stdint.h>

/* Application Includes */
#include "TimeZone.h"
#include "Config.h"
#include "Utils.h"
#include "NetworkCommand.h"
#include "NetworkConfig.h"
#include "NetworkManager.h"
#include "NetworkFileTransfer.h"
#include "LiveMediaStreamer.h"
#include "PlaybackMediaStreamer.h"
#include "SyncPlaybackMediaStreamer.h"
#include "InstantPlaybackMediaStreamer.h"
#include "BackupManager.h"
#include "PtzTour.h"
#include "SystemUpgrade.h"
#include "CameraInterface.h"
#include "CameraEvent.h"
#include "RecordManager.h"
#include "ImageUpload.h"
#include "FtpClient.h"
#include "SmtpClient.h"
#include "TcpNotification.h"
#include "SmsNotify.h"
#include "DdnsClient.h"
#include "DiskController.h"
#include "DebugLog.h"
#include "CameraDatabase.h"
#include "RestoreConfig.h"
#include "MobileBroadBand.h"
#include "SMSonHttp.h"
#include "MatrixMacClient.h"
#include "DoorCommand.h"
#include "CameraSearch.h"
#include "DiskUtility.h"
#include "ViewerUserSession.h"
#include "AdvanceCameraSearch.h"
#include "AutoConfigCamera.h"
#include "ClientMediaStreamer.h"
#include "CameraInitiation.h"
#include "P2pInterface.h"
#include "FirmwareManagement.h"
#include "DhcpServer.h"
#include "FcmPushNotification.h"
#include "DeviceInitiation.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_RECORD_SEARCH		500
#define MAX_EVENT_SEARCH		2000
#define EVENT_SEARCH_RESP_MAX   (262144) /* 256KB: 2000 events * 126 bytes + extra bytes */
#define MAX_SYNC_CLIP_NUMBER	(10)
#define MAX_STRING_LENGTH		32768

#define NETWORK_DISCOVERY_STR   "Network Discovery"
#define IPV6_DISABLED_STR       "IPv6 Not Available"

#define CONVERT_CMD_TIME_TO_BROKEN_TIME(cmdTime, brokenTime)        \
    brokenTime.tm_sec = (cmdTime % 100); cmdTime /= 100;            \
    brokenTime.tm_min = (cmdTime % 100); cmdTime /= 100;            \
    brokenTime.tm_hour = (cmdTime % 100); cmdTime /= 100;           \
    brokenTime.tm_year = (cmdTime % 10000); cmdTime /= 10000;       \
    brokenTime.tm_mon = ((cmdTime % 100) - 1); cmdTime /= 100;      \
    brokenTime.tm_mday = (cmdTime % 100); cmdTime /= 100;           \
    RESET_EXTRA_BORKEN_TIME_VAR(brokenTime);

#define CONVERT_CMD_MNTH_YEAR_TO_BROKEN_TIME(cmdTime, brokenTime)   \
    brokenTime.tm_year = (cmdTime % 10000); cmdTime /= 10000;       \
    brokenTime.tm_mon = ((cmdTime % 100) - 1); cmdTime /= 100;      \
    brokenTime.tm_mday = 0;                                         \
    brokenTime.tm_hour = 0;                                         \
    brokenTime.tm_min = 0;                                          \
    brokenTime.tm_sec = 0;                                          \
    RESET_EXTRA_BORKEN_TIME_VAR(brokenTime);

#define CONVERT_CMD_DATE_TO_BROKEN_TIME(cmdTime, brokenTime)        \
    brokenTime.tm_year = (cmdTime % 10000); cmdTime /= 10000;       \
    brokenTime.tm_mon = ((cmdTime % 100) - 1); cmdTime /= 100;      \
    brokenTime.tm_mday = (cmdTime % 100); cmdTime /= 100;           \
    brokenTime.tm_hour = 0;                                         \
    brokenTime.tm_min = 0;                                          \
    brokenTime.tm_sec = 0;                                          \
    RESET_EXTRA_BORKEN_TIME_VAR(brokenTime);

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    CMD_SRT_LV_STRM = 0,
    CMD_STP_LV_STRM,
    CMD_INC_LV_AUD,
    CMD_EXC_LV_AUD,
    CMD_CNG_LV_STRM,
    CMD_SRT_MAN_REC,
    CMD_STP_MAN_REC,
    CMD_SETPANTILT,
    CMD_SETZOOM,
    CMD_SETFOCUS,
    CMD_SETIRIS,
    CMD_CALLPRESET,
    CMD_SRT_MAN_PTZ_TOUR,
    CMD_STP_MAN_PTZ_TOUR,
    CMD_SEARCH_RCD_DATA,
    CMD_SEARCH_RCD_ALL_DATA,
    CMD_GET_PLY_STRM_ID,
    CMD_PLY_RCD_STRM,
    CMD_PAUSE_RCD_STRM,
    CMD_RSM_PLY_RCD_STRM,
    CMD_STEP_RCD_STRM,
    CMD_STP_RCD_STRM,
    CMD_CLR_PLY_STRM_ID,
    CMD_SEARCH_STR_EVT,
    CMD_ONLINE_USR,
    CMD_BLK_USR,
    CMD_VIEW_BLK_USR,
    CMD_UNBLK_USR,
    CMD_PHYSICAL_DISK_STS,
    CMD_LOGICAL_VOLUME_STS,
    CMD_HDDFORMAT,
    CMD_CNCL_RAID_PRCS,
    CMD_SRT_MAN_TRG,
    CMD_STP_MAN_TRG,
    CMD_GET_DATE_TIME,
    CMD_SET_DATE_TIME,
    CMD_RSTR_CFG,
    CMD_UPDT_FRM,
    CMD_LOGOUT,
    CMD_SHUTDOWN,
    CMD_RESTART,
    CMD_FAC_DEF_CFG,
    CMD_UPDT_DDNS,
    CMD_TST_MAIL,
    CMD_TST_FTP_CON,
    CMD_TST_TCP_CON,
    CMD_TST_SMS,
    CMD_USB_DISK_STS,
    CMD_USBFORMAT,
    CMD_USBUNPLUG,
    CMD_STP_BCKUP,
    CMD_ALRM_OUT,
    CMD_GET_MAC,
    CMD_REC_RGT_STS,
    CMD_BCK_RGT_STS,
    CMD_HEALTH_STS,
    CMD_BUZ_CTRL,
    CMD_SNP_SHT,
    CMD_BKUP_RCD,
    CMD_CNCT_STS,
    CMD_CHK_BAL,
    CMD_CNG_PWD,
    CMD_CNG_USER,
    CMD_BRND_NAME,
    CMD_MDL_NAME,
    CMD_ENCDR_SUP,
    CMD_RES_SUP,
    CMD_FR_SUP,
    CMD_QLT_SUP,
    CMD_OTHR_SUP,
    CMD_UPDT_HOST_NM,
    CMD_CHK_DID,
    CMD_CHK_PRIV,
    CMD_ADVANCE_STS,
    CMD_ADVANCE_STS_CAMERA,
    CMD_COSEC_VIDEO_POPUP,
    CMD_AUTO_SEARCH,
    CMD_ADD_CAMERAS,
    CMD_CNCL_SEARCH,
    CMD_TST_CAM,
    CMD_PLYBCK_SRCH_MNTH,
    CMD_PLYBCK_SRCH_DAY,
    CMD_PLYBCK_RCD,
    CMD_PAUSE_RCD,
    CMD_RSM_RCD,
    CMD_STP_RCD,
    CMD_CLR_RCD,
    CMD_SYNC_PLYBCK_RCD,
    CMD_SYNC_CLP_RCD,
    CMD_GET_MOB_NO,
    CMD_GET_USER_DETAIL,
    CMD_GET_BUILD_DATE,
    CMD_GET_CAMERA_INFO,
    CMD_STRT_INSTANT_PLY,
    CMD_SEEK_INSTANT_PLY,
    CMD_STOP_INSTANT_PLY,
    CMD_PAUSE_INSTANT_PLY,
    CMD_RSM_INSTANT_PLY,
    CMD_GET_MAX_SUPP_PROF,
    CMD_GET_PROF_PARA,
    CMD_GET_BITRATE,
    CMD_GET_MOTION_WINDOW,
    CMD_CHANGE_IP_CAM_ADDRS,
    CMD_ADV_CAM_SEARCH,
    CMD_RESUME_PTZ_TOUR,
    CMD_SET_MOTION_WINDOW,
    CMD_GENERATE_FAILURE_REPORT,
    CMD_PTZ_TOUR_STATUS,
    CMD_MAX_ADD_CAM,
    CMD_GET_PRIVACY_MASK_WINDOW,
    CMD_SET_PRIVACY_MASK_WINDOW,
    CMD_TST_ND_CON,
    CMD_AUTO_CONFIGURE,
    CMD_GET_ACQ_LIST,
    CMD_CPU_LOAD,
    CMD_GET_USR_RIGHTS,
    CMD_GET_REG_CFG,
    CMD_SND_LAYOUT_FILE,
    CMD_RCV_LAYOUT_FILE,
    CMD_AUTO_CFG_STATUS_RPRT,
    CMD_STRT_CLNT_AUDIO,
    CMD_STOP_CLNT_AUDIO,
    CMD_SND_AUDIO,              /* Used by device client to send audio */
    CMD_STP_AUDIO,              /* Used by device client to stop audio */
    CMD_GET_CAM_INITIATED_LIST,
    CMD_ADD_CAM_INITIATED,
    CMD_RJCT_CAM_INITIATED,
    CMD_MAN_BACKUP,
    CMD_GET_LANGUAGE,           /* Get all system's languages */
    CMD_GET_USER_LANGUAGE,      /* Get user's configured language */
    CMD_SET_USER_LANGUAGE,      /* Set language for user */
    CMD_GET_STATUS,             /* Get P2P & Device-Initiated SAMAS connection status */
    CMD_SEARCH_FIRMWARE,        /* Check for new firmware on FTP server */
    CMD_START_UPGRADE,          /* Update firmware with latest version */
    CMD_GET_DHCP_LEASE,         /* DHCP server lease status info */
    CMD_GET_CAPABILITY,         /* Get camera capability info */
    CMD_ENABLE_PUSH,            /* enable/disable push notification */
    CMD_GET_PUSH_STATUS,        /* get push notification flag value */
    CMD_GET_PUSH_DEV_LIST,      /* get push notification device status list */
    CMD_DEL_PUSH_DEV,           /* delete push notification device from list */
    CMD_TEST_EMAIL_ID,          /* Send test email for password reset functionality */
    CMD_GET_PWD_RST_INFO,       /* Get password reset Configuration of user */
    CMD_SET_PWD_RST_INFO,       /* Set password reset Configuration of user */
    CMD_GET_MAN_BKP_LOC,        /* Get manual backup location */
    CMD_VALIDATE_USER_CRED,     /* Validate user's credentials (e.g. Username, Password, etc.) */
    MAX_NET_COMMAND

}NET_COMMAND_e;

typedef enum
{
    SRT_LV_STRM_CAMERA = 0,
    SRT_LV_STRM_TYPE,
    SRT_LV_STRM_FRAME_TYPE,
    SRT_LV_STRM_FRAME_TYPE_MJEPG,
    SRT_LV_STRM_FPS_MJPEG,
    MAX_SRT_LV_STRM_ARG,

    STP_LV_STRM_CAMERA = 0,
    STP_LV_STRM_STREAM_TYPE,
    MAX_STP_LV_STRM_ARG,

    INC_LV_AUDIO_CAMERA = 0,
    INC_LV_AUDIO_STREAM_TYPE,
    MAX_INC_LV_AUDIO_ARG,

    EXC_LV_AUDIO_CAMERA = 0,
    EXC_LV_AUDIO_STREAM_TYPE,
    MAX_EXC_LV_AUDIO_ARG,

    CHNG_LV_STRM_CAMERA = 0,
    CHNG_LV_STRM_TYPE,
    CHNG_LV_STRM_FRAME_TYPE,
    CHNG_LV_STRM_FRAME_TYPE_MJEPG,
    CHNG_LV_STRM_FPS_MJPEG,
    MAX_CHNG_LV_STRM_ARG,

    SRT_MAN_REC_CAMERA = 0,
    MAX_SRT_MAN_REC_ARG,

    STP_MAN_REC_CAMERA = 0,
    MAX_STP_MAN_REC_ARG,

    SETPANTILT_CAMERA = 0,
    SETPANTILT_PAN,
    SETPANTILT_TILT,
    SETPANTILT_SPEED,
    MAX_SETPANTILT_ARG,

    SETZOOM_CAMERA = 0,
    SETZOOM_ZOOM,
    SETZOOM_SPEED,
    MAX_SETZOOM_ARG,

    SETFOCUS_CAMERA = 0,
    SETFOCUS_FOCUS,
    SETFOCUS_SPEED,
    MAX_SETFOCUS_ARG,

    SETIRIS_CAMERA = 0,
    SETIRIS_IRIS,
    MAX_SETIRIS_ARG,

    CALLPRESET_CAMERA = 0,
    CALLPRESET_PRESET_NUM,
    MAX_CALLPRESET_ARG,

    SRT_MAN_PTZ_TOUR_CAMERA = 0,
    MAX_SRT_MAN_PTZ_TOUR_ARG,

    STP_MAN_PTZ_TOUR_CAMERA = 0,
    MAX_STP_MAN_PTZ_TOUR_ARG,

    SEARCH_RCD_DATA_CAMERA = 0,
    SEARCH_RCD_DATA_TYPE,
    SEARCH_RCD_DATA_START_TIME,
    SEARCH_RCD_DATA_STOP_TIME,
    SEARCH_RCD_DATA_NUM_OF_DATA,
    SEARCH_RCD_DATA_STORAGE_TYPE,
    MAX_SEARCH_RCD_DATA_ARG,

    GET_PLY_STRM_ID_START_TIME = 0,
    GET_PLY_STRM_ID_STOP_TIME,
    GET_PLY_STRM_ID_CAMERA,
    GET_PLY_STRM_ID_TYPE,
    GET_PLY_STRM_ID_OVERLAP,
    GET_PLY_STRM_ID_DISK_ID,
    GET_PLY_STRM_ID_PARTITION_ID,
    GET_PLY_STRM_ID_STORAGE_TYPE,
    MAX_GET_PLY_STRM_ID_ARG,

    PLY_RCD_STRM_PLAYBACK_ID = 0,
    PLY_RCD_STRM_DIRECTION,
    PLY_RCD_STRM_PLAY_TIME,
    PLY_RCD_STRM_AUDIO,
    PLY_RCD_STRM_SPEED,
    MAX_PLY_RCD_STRM_ARG,

    PAUSE_RCD_STRM_PLAYBACK_ID = 0,
    MAX_PAUSE_RCD_STRM_ARG,

    RSM_PLY_RCD_STRM_PLAYBACK_ID = 0,
    MAX_RSM_PLY_RCD_STRM_ARG,

    STEP_RCD_STRM_PLAYBACK_ID = 0,
    STEP_RCD_STRM_STEP,
    STEP_RCD_STRM_TIME,
    STEP_RCD_STRM_FRAME_TYPE,
    MAX_STEP_RCD_STRM_ARG,

    STP_RCD_STRM_PLAYBACK_ID = 0,
    MAX_STP_RCD_STRM_ARG,

    CLR_PLY_STRM_ID_PLAYBACK_ID = 0,
    MAX_CLR_PLY_STRM_ID_ARG,

    SEARCH_STR_EVT_START_TIME = 0,
    SEARCH_STR_EVT_END_TIME,
    SEARCH_STR_EVT_TYPE,
    SEARCH_STR_EVT_NUM_OF_REC,
    MAX_SEARCH_STR_EVT_ARG,

    MAX_ONLINE_USR_ARG = 0,

    MAX_PHYSICAL_DISK_STS_ARG = 0,

    MAX_LOGICAL_VOLUME_STS_ARG = 0,

    HDDFORMAT_VOL_NUM = 0,
    MAX_HDDFORMAT_ARG,

    STOP_RAID_VOL_NO = 0,
    MAX_STOP_RAID_ARG,

    MAX_SRT_MAN_TRG_ARG = 0,

    MAX_STP_MAN_TRG_ARG = 0,

    MAX_GET_DATE_TIME_ARG = 0,

    SET_DATE_TIME_CURRENT_TIME = 0,
    MAX_SET_DATE_TIME_ARG,

    MAX_IMPRT_CFG_ARG = 0,

    MAX_EXPRT_CFG_ARG = 0,

    MAX_UPDT_FRM_ARG = 0,

    MAX_LOGOUT_ARG = 0,

    MAX_SHUTDOWN_ARG = 0,

    MAX_RESTART_ARG = 0,

    FAC_DEF_CFG_NETWORK = 0,
    FAC_DEF_CFG_USERACC,
    FAC_DEF_CFG_OTHER,
    MAX_FAC_DEF_CFG_ARG,

    MAX_UPDT_DDNS_ARG = 0,

    TST_MAIL_MAIL_ADDR = 0,
    MAX_TST_MAIL_ARG,

    MAX_TST_TCP_CON_ARG = 0,

    TST_SMS_MOBILE_NUM = 0,
    MAX_TST_SMSARG,

    MAX_USB_DISK_STS_ARG = 0,

    USBFORMAT_PORT = 0,
    MAX_USBFORMAT_ARG,

    USBUNPLUG_PORT = 0,
    MAX_USBUNPLUG_ARG,

    BACKUP_TYPE = 0,
    MAX_BACKUP_TYPE_ARG,

    ALARM_OUT_ALARM_NUM = 0,
    ALARM_OUT_ALARM_STS,
    MAX_ALARM_OUT_ARG,

    GET_MAC_LAN_NUM = 0,
    MAX_GET_MAC_ARG,

    MAX_REC_RGT_STS_ARG = 0,

    MAX_BCK_RGT_STS_ARG = 0,

    MAX_HEALTH_STS_ARG = 0,

    BKUP_RCD_START_TIME = 0,
    BKUP_RCD_STOP_TIME,
    BKUP_RCD_CAMERA,
    BKUP_RCD_OVERLAP,
    BKUP_RCD_DISK_ID,
    BKUP_RCD_FORMAT,
    BKUP_RCD_PARTITION_ID,
    BKUP_RCD_FROM_STORAGE_TYPE,
    BKUP_RCD_DESTINATION,
    MAX_BKUP_RCD_ARG,

    ENCDR_SUP_CAM = 0,
    ENCDR_SUP_STREAM,
    ENCDR_SUP_PROFILE,
    MAX_ENCDR_SUP,

    RES_SUP_CAM = 0,
    RES_SUP_ENCDR_NAME,
    RES_SUP_STREAM,
    RES_SUP_PROFILE,
    MAX_RES_SUP,

    FR_SUP_CAM = 0,
    FR_SUP_ENCDR,
    FR_SUP_RESOLUTION,
    FR_SUP_STREAM,
    FR_SUP_PROFILE,
    MAX_FR_SUP,

    QLT_SUP_CAM = 0,
    QLT_SUP_ENCDR,
    QLT_SUP_STREAM,
    QLT_SUP_PROFILE,
    MAX_QLT_SUP,

    PLAY_SEARCH_MNTH_RCD_MNTH_YEAR = 0,
    PLAY_SEARCH_MNTH_RCD_EVENT_TYPE,
    PLAY_SEARCH_MNTH_RCD_CAMERA_MASK1,
    PLAY_SEARCH_MNTH_RCD_STORAGE_TYPE,
    PLAY_SEARCH_MNTH_RCD_CAMERA_MASK2,
    MAX_PLAY_SEARCH_MNTH_RCD_ARG,

    PLAY_SEARCH_DAY_RCD_DATE = 0,
    PLAY_SEARCH_DAY_RCD_EVENT_TYPE,
    PLAY_SEARCH_DAY_RCD_CAMERA_MASK1,
    PLAY_SEARCH_DAY_RCD_STORAGE_TYPE,
    PLAY_SEARCH_DAY_RCD_CAMERA_MASK2,
    MAX_PLAY_SEARCH_DAY_RCD_ARG,

    SYNC_PLAY_STREAM_CAMERA_MASK1 = 0,
    SYNC_PLAY_STREAM_DIRECTION,
    SYNC_PLAY_STREAM_PLAY_TIME,
    SYNC_PLAY_STREAM_SPEED,
    SYNC_PLAY_STREAM_EVENT,
    SYNC_PLAY_STREAM_RCD_STORAGE_TYPE,
    SYNC_PLAY_STREAM_CAMERA_MASK2,
    MAX_SYNC_PLAY_STREAM_ARG,

    MAX_SYNC_PLAY_STRM_PAUSE_ARG = 0,

    MAX_SYNC_PLAY_STRM_RESUME_ARG = 0,

    MAX_SYNC_PLAY_STRM_STOP_ARG = 0,

    SYNC_PLAY_STRM_STEP_DIRECTION = 0,
    SYNC_PLAY_STRM_STEP_TIME,
    SYNC_PLAY_STRM_STEP_FRM_TYPE,
    MAX_SYNC_PLAY_STRM_STEP_ARG,

    MAX_SYNC_PLAY_STRM_CLEAR_ARG = 0,

    SYNC_PLAY_SEEK_SPEED_CAMERA_MASK1 = 0,
    SYNC_PLAY_SEEK_SPEED_DIRECTION,
    SYNC_PLAY_SEEK_SPEED_TIME,
    SYNC_PLAY_SEEK_SPEED_SPEED,
    SYNC_PLAY_SEEK_SPEED_EVENT_TYPE,
    SYNC_PLAY_SEEK_SPEED_AUDIO_MASK1,
    SYNC_PLAY_SEEK_SPEED_FRM_SYNC_NO,
    SYNC_PLAY_SEEK_SPEED_CAMERA_MASK2,
    SYNC_PLAY_SEEK_SPEED_AUDIO_MASK2,
    MAX_SYNC_PLAY_SEEK_SPEED_ARG,

    SYNC_CLP_CAMERA_NO = 0,
    SYNC_CLP_START_TIME,
    SYNC_CLP_END_TIME,
    SYNC_CLP_RECORD_STORAGE_TYPE,
    MAX_SYNC_CPL_ARG,

    GET_MOB_NUM_CAMERA_NO = 0,
    MAX_GET_MOB_NUM_ARG,

    MAX_DISK_CHECKING_ARG = 0,

    MAX_BUILD_DATE_ARG = 0,

    MAX_GET_CAM_INFO_ARG = 0,

    INSTANT_PLAY_STRT_CAMERA_NO = 0,
    MAX_INSTANT_PLAY_STRT_ARG,

    INSTANT_PLAY_SEEK_PLAYBACK_ID = 0,
    INSTANT_PLAY_SEEK_PLAYBACK_TIME,
    INSTANT_PLAY_SEEK_PLAYBACK_AUDIO,
    INSTANT_PLAY_SEEK_PLAYBACK_DIRECTION,
    INSTANT_PLAY_SEEK_FRAME_NO,
    MAX_INSTANT_PLAY_SEEK_ARG,

    INSTANT_PLAY_STOP_PLAYBACK_ID = 0,
    MAX_INSTANT_PLAY_STOP_ARG,

    INSTANT_PLAY_PAUSE_PLAYBACK_ID = 0,
    MAX_INSTANT_PLAY_PAUSE_ARG,

    INSTANT_PLAY_RSM_PLAYBACK_ID = 0,
    MAX_INSTANT_PLAY_RSM_ARG,

    PROFILE_SUP_CAM = 0,
    MAX_PROFILE_SUP,

    PROFILE_PARAM_CAM = 0,
    PROFILE_PARAM_STREAM,
    PROFILE_PARAM_PROF_INDEX,
    MAX_PROFILE_PARAM,

    BITRATE_SUP_CAM = 0,
    BITRATE_SUP_ENCDR,
    BITRATE_SUP_STREAM,
    BITRATE_SUP_PROFILE,
    MAX_BITRATE_SUP,

    GET_MOTION_WIN_CAM = 0,
    MAX_GET_MOTION_WIN,

    CURRENT_CAM_INDEX = 0,
    CHANGE_CAM_IP,
    CHANGE_SUB_NET,
    CHANGE_GATE_WAY,
    MAX_CHANGE_IP_ADDR,

    ADV_SEARCH_START_IP = 0,
    ADV_SEARCH_END_IP,
    ADV_SEARCH_CAM_BRAND,
    ADV_SEARCH_HTTP_PORT,
    ADV_SEARCH_USERNAME,
    ADV_SEARCH_PASSWORD,
    MAX_ADV_SEARCH,

    SET_MOTION_WIN_CAM = 0,
    SET_MOTION_WIN_METHOD,
    MAX_SET_MOTION_WIN_COMMON,

    MAX_PTZ_RESUME = 0,

    MAX_GEN_FAILURE_REPORT = 0,
    MAX_PTZ_TOUR_STATUS = 0,
    MAX_GET_MAX_ADD_CAM = 0,

    GET_PRIVACY_MASK_CAM = 0,
    MAX_GET_PRIVACY_MASK,

    SET_PRIVACY_MASK_CAM = 0,
    SET_PRIVACY_MASK_MAX_SUPPORTED_WIN,
    MAX_SET_PRIVACY_MASK_COMMON,

    /* startx, starty, width, height for 1 privacy mask. total 8 privacy mask = 32 points */
    MAX_SET_PRIVACY_MASK_POINT = 32,

    TEST_NDD_CONN_IP_ADDR = 0,
    TEST_NDD_CONN_FILE_SYS_TYPE,
    TEST_NDD_CONN_USERNAME,
    TEST_NDD_CONN_PASSWOR,
    TEST_NDD_CONN_UPLOAD_PATH,
    TEST_NDD_CONN_NDD_NO,
    MAX_TEST_NDD_CONN,

    MAX_GET_ACQ_LIST = 0,

    MAX_SND_LAYOUT_FILE = 0,
    MAX_RCV_LAYOUT_FILE = 0

}COMMAND_ARG_e;

typedef enum
{
    FAC_DFLT_ALL = 0,
    FAC_DFLT_EXCPT_LAN,
    MAX_FAC_DFLT
}FAC_DFLT_SETTING_e;

typedef enum
{
    TEST_IMAGE_UPLOAD_SERVER = 0,
    TEST_BACKUP_SERVER,
    MAX_TEST_FTP_SERVER
}TEST_FTP_SERVER_e;

typedef enum
{
    HS_MOTION_DET           = 0,
    HS_VIEW_TAMPER          = 1,
    HS_CAM_CONNECTION       = 2,
    HS_CAM_SENSOR_1         = 3,
    HS_CAM_SENSOR_2         = 4,
    HS_CAM_SENSOR_3         = 5,
    HS_CAM_ALARM_1          = 6,
    HS_CAM_ALARM_2          = 7,
    HS_CAM_ALARM_3          = 8,
    HS_MAN_RECORDING        = 9,
    HS_ALM_RECORDING        = 10,
    HS_SCH_RECORDING        = 11,
    HS_PRESET_TOUR          = 12,
    HS_SYS_SENSOR           = 13,
    HS_SYS_ALARM            = 14,
    HS_HDD_STATUS           = 15,
    HS_SCH_BACKUP           = 16,
    HS_MAN_TRIGGER          = 17,
    HS_MAINS_STATUS         = 18,
    HS_BUZ_STATUS           = 19,
    HS_USB_STATUS           = 20,
    HS_COSEC_RECORDING      = 21,
    HS_CAM_STREAM           = 22,
    HS_LINE_DETECTION       = 23,
    HS_OBJECT_INTRUTION     = 24,
    HS_AUDIO_EXCEPTION      = 25,
    HS_MISSING_OBJECT       = 26,
    HS_SUSPICIOUS_OBJECT    = 27,
    HS_LOITERING            = 28,
    HS_OBJECT_COUNTING      = 29,
    HS_ADA_RECORDING        = 30,
    HS_MAX_STATUS

}HEALTH_STATUS_REPLY_e;

typedef enum
{
    TOUR_ACTIVE = 0,
    TOUR_PAUSE

}PTZ_TOUR_STATE_e;

typedef enum
{
    CAPABILITY_CMD_ID_TEXT_OVERLAY = 0,
    CAPABILITY_CMD_ID_IMAGING_CAPABILITY,
    CAPABILITY_CMD_ID_ONVIF_MEDIA2_SUPPORT,
    CAPABILITY_CMD_ID_MAX

}CAPABILITY_CMD_ID_e;

typedef enum
{
    STATUS_CMD_ID_P2P,
    STATUS_CMD_ID_SAMAS,
    STATUS_CMD_ID_MAX

}STATUS_CMD_ID_e;


typedef struct
{
    BOOL		(*funcPtr)(UINT8  majorIdx, UINT8  mainorIdx);
    UINT8		majorIndex;
    UINT8		minorIndex;
}HEALTH_STATUS_REPLY_t;

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
const CHARPTR headerReq[MAX_HEADER_REQ] =
{
    "REQ_LOG",
    "ACK_LOG",
    "REQ_POL",
    "RSP_POL",
    "GET_CFG",
    "SET_CFG",
    "DEF_CFG",
    "RPL_CFG",
    "SET_CMD",
    "RPL_CMD",
    "REQ_EVT",
    "RCV_EVT",
    "REQ_FTS",
    "RPL_FTS",
    "ACK_FTS",
    "SET_FTS",
    "STP_FTS",
    "RPL_STP",
    "DOR_CMD",
    "PWD_RST",
    "RPL_PWD",

    /* These onwords our message structure */
    "DEV_DETECT",
    "CGI_REQ_GETPORT",
    "WS_SYSTEM_LOGS",
    "WS_PCAP_TRACE",
    "GET_MAC_SERVER_CFG",
    "WS_SYSTEM_INFO",
    "DHCP_SERVER_NOTIFY",
    "DDNS_CLIENT_NOTIFY",
};

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
/* 2d array of reply message to use it as thread indepedent for p2p and native commands */
static CHAR replyMsg[CLIENT_CB_TYPE_MAX][MAX_REPLY_SZ];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void sendClientCmdResp(NET_CMD_STATUS_e status, INT32 connFd, BOOL closeConnF, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static void nativeClientCmdResp(NET_CMD_STATUS_e status, INT32 connFd, BOOL closeConnF);
//-------------------------------------------------------------------------------------------------
static void p2pClientCmdResp(NET_CMD_STATUS_e status, INT32 connFd, BOOL closeConnF);
//-------------------------------------------------------------------------------------------------
static INT32 sendP2pFrameData(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
static BOOL StartLiveStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopLiveStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL IncludeLiveAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ExcludeLiveAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ChangeLiveStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartManualRecordingCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopManualRecordingCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetPanTiltCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetZoomCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetFocusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetIrisCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL CallPtzPresetCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartManualPtzTourCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopManualPtzTourCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SearchRecordDataCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SearchRecordAllDataCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetAsyncPlayStreamIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AsyncPlayRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AsyncPauseRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AsyncResumeRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AsyncStepRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AsyncStopRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ClearAsyncPlayStreamIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SearchEventLogCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetOnlineUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL BlockUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ViewBlockedUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL UnBlockUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetPhysicalDiskStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetLogicalDiskStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL HarddiskFormatCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopRaidCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartManualTriggerCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopManualTriggerCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetDateTimeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetDateTimeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL RestoreConfigCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL UpdateFirmwareCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SystemLogoutCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SystemShutdownCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SystemRestartCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL FactoryDefaultCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL UpdateDdnsCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL TestMailCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL TestFtpConnCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL TestTcpConnCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL TestSmsConnCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetUsbStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL UsbFormatCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL UsbUnplugCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopBackupCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetAlarmOutputCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetMacAddressCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL RecordRightsStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL BackupRightsStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetHelthStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL MuteBuzzerCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSnapshotCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL BackUpRecordCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetModemStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSmsBalanceCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ChangeUserPasswordCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ChangeUserSessionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetBrandNameCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetModelNameCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSupportedProfileCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetProfileParamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSupportedBitRateCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSupportedEncoderCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSupportedResolutionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSupportedFrameRateCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSupportedVideoQualityCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetOtherSupportedFeaturesCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL	UpdateMatrixDnsHostNameCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL	CheckDeviceIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL CheckUserCameraPrivilegeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetAdvanceStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetAdvanceCameraStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCosecVideoPopUpDetailCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AutoSearchCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AddCamerasCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL CancelCameraSearchCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void ciTestCamCb(UINT8 channelNo, NET_CMD_STATUS_e cmdStat, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static BOOL TestCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL PlaySearchMonthWiseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL PlaySearchDayWiseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncPlayStartCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncPlayPauseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncPlayResumeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncPlayStopCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncPlaySetPositionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncPlayClearCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SyncRecordClipCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCameraMobileNumberCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCameraUserDetailCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetSystemBuildDateCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCameraInfoCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SeekInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopInstantPlayback(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL PauseInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ResumeInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ChangeCameraIpAddressCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AdvanceCameraSearchCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL PtzTourResumeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetMotionWindowCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetMotionWindowCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GenerateFailureReportCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetPtzTourStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetConfiguredCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetPrivacyMaskCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetPrivacyMaskCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL TestNddConnectionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AutoConfigureCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetAcquiredCameraListCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCpuLoadCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetUserRightsCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetRegionalSettingsCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SendLayoutFileCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ReceiveLayoutFileCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AutoConfigStatusReportCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartTxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopTxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartRxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StopRxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCameraInitiatedListCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL AddCameraInitiatedCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL RejectCameraInitiatedCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ManualBackupCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetLanguageCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetUserLanguageCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetUserLanguageCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SearchFirmwareCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL StartUpgradeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetDhcpLeaseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetCapabilityCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL EnablePushNotificationCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetPushNotificationStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetPushNotificationDevListCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL DeletePushNotificationDevCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL TestEmailIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetPasswordResetInfoCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL SetPasswordResetInfoCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL GetManualBackupLocationCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL ValidateUserCredentialCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR netCommandStr[MAX_NET_COMMAND] =
{
    "SRT_LV_STRM",
    "STP_LV_STRM",
    "INC_LV_AUD",
    "EXC_LV_AUD",
    "CNG_LV_STRM",
    "SRT_MAN_REC",
    "STP_MAN_REC",
    "SETPANTILT",
    "SETZOOM",
    "SETFOCUS",
    "SETIRIS",
    "CALLPRESET",
    "SRT_MAN_PTZ_TOUR",
    "STP_MAN_PTZ_TOUR",
    "SEARCH_RCD_DATA",
    "SEARCH_RCD_ALL_DATA",
    "GET_PLY_STRM_ID",
    "PLY_RCD_STRM",
    "PAUSE_RCD_STRM",
    "RSM_PLY_RCD_STRM",
    "STEP_RCD_STRM",
    "STP_RCD_STRM",
    "CLR_PLY_STRM_ID",
    "SEARCH_STR_EVT",
    "ONLINE_USR",
    "BLK_USR",
    "VIEW_BLK_USR",
    "UNBLK_USR",
    "PHYSICAL_DISK_STS",
    "LOGICAL_VOLUME_STS",
    "HDDFORMAT",
    "CNCL_RAID_PRCS",
    "SRT_MAN_TRG",
    "STP_MAN_TRG",
    "GET_DATE_TIME",
    "SET_DATE_TIME",
    "RSTR_CFG",
    "UPDT_FRM",
    "LOGOUT",
    "SHUTDOWN",
    "RESTART",
    "FAC_DEF_CFG",
    "UPDT_DDNS",
    "TST_MAIL",
    "TST_FTP_CON",
    "TST_TCP_CON",
    "TST_SMS",
    "USB_DISK_STS",
    "USBFORMAT",
    "USBUNPLUG",
    "STP_BCKUP",
    "ALRM_OUT",
    "GET_MAC",
    "REC_RGT_STS",
    "BCK_RGT_STS",
    "HEALTH_STS",
    "BUZ_CTRL",
    "SNP_SHT",
    "BKUP_RCD",
    "CNCT_STS",
    "CHK_BAL",
    "CNG_PWD",
    "CNG_USER",
    "BRND_NAME",
    "MDL_NAME",
    "ENCDR_SUP",
    "RES_SUP",
    "FR_SUP",
    "QLT_SUP",
    "OTHR_SUP",
    "UPDT_HOST_NM",
    "CHK_DID",
    "CHK_PRIV",
    "ADVANCE_STS",
    "ADVANCE_STS_CAMERA",
    "COSEC_VIDEO_POPUP",
    "AUTO_SEARCH",
    "ADD_CAMERAS",
    "CNCL_SEARCH",
    "TST_CAM",
    "PLYBCK_SRCH_MNTH",
    "PLYBCK_SRCH_DAY",
    "PLYBCK_RCD",
    "PAUSE_RCD",
    "RSM_RCD",
    "STP_RCD",
    "CLR_RCD",
    "SYNC_PLYBCK_RCD",
    "SYNC_CLP_RCD",
    "GET_MOB_NO",
    "GET_USER_DETAIL",
    "GET_BUILD_DATE",
    "GET_CAMERA_INFO",
    "STRT_INSTANT_PLY",
    "SEEK_INSTANT_PLY",
    "STOP_INSTANT_PLY",
    "PAUSE_INSTANT_PLY",
    "RSM_INSTANT_PLY",
    "GET_MAX_SUPP_PROF",
    "GET_PROF_PARA",
    "GET_BITRATE",
    "GET_MOTION_WINDOW",
    "CHANGE_IP_CAM_ADDRS",
    "ADV_CAM_SEARCH",
    "RESUME_PTZ_TOUR",
    "SET_MOTION_WINDOW",
    "GENERATE_FAILURE_REPORT",
    "PTZ_TOUR_STATUS",
    "MAX_ADD_CAM",
    "GET_PRIVACY_MASK_WINDOW",
    "SET_PRIVACY_MASK_WINDOW",
    "TST_ND_CON",
    "AUTO_CONFIGURE",
    "GET_ACQ_LIST",
    "CPU_LOAD",
    "GET_USR_RIGHTS",
    "GET_REG_CFG",
    "SND_LAYOUT_FILE",
    "RCV_LAYOUT_FILE",
    "AUTO_CFG_STATUS_RPRT",
    "STRT_CLNT_AUDIO",
    "STOP_CLNT_AUDIO",
    "SND_AUDIO",
    "STP_AUDIO",
    "GET_CAM_INITIATED_LIST",
    "ADD_CAM_INITIATED",
    "RJCT_CAM_INITIATED",
    "MAN_BACKUP",
    "GET_LANGUAGE",
    "GET_USER_LANGUAGE",
    "SET_USER_LANGUAGE",
    "GET_STATUS",
    "SEARCH_FIRMWARE",
    "START_UPGRADE",
    "GET_DHCP_LEASE",
    "GET_CAPABILITY",
    "ENABLE_PUSH",
    "GET_PUSH_STATUS",
    "GET_PUSH_DEV_LIST",
    "DEL_PUSH_DEV",
    "TEST_EMAIL_ID",
    "GET_PWD_RST_INFO",
    "SET_PWD_RST_INFO",
    "GET_MAN_BKP_LOC",
    "VALIDATE_USER_CRED",
};

static BOOL (*cmsCommandFuncPtr[MAX_NET_COMMAND])(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex) =
{
    StartLiveStreamCmd,             /* CMD_SRT_LV_STRM */
    StopLiveStreamCmd,              /* CMD_STP_LV_STRM */
    IncludeLiveAudioCmd,            /* CMD_INC_LV_AUD */
    ExcludeLiveAudioCmd,            /* CMD_EXC_LV_AUD */
    ChangeLiveStreamCmd,            /* CMD_CNG_LV_STRM */
    StartManualRecordingCmd,        /* CMD_SRT_MAN_REC */
    StopManualRecordingCmd,         /* CMD_STP_MAN_REC */
    SetPanTiltCmd,                  /* CMD_SETPANTILT */
    SetZoomCmd,                     /* CMD_SETZOOM */
    SetFocusCmd,                    /* CMD_SETFOCUS */
    SetIrisCmd,                     /* CMD_SETIRIS */
    CallPtzPresetCmd,               /* CMD_CALLPRESET */
    StartManualPtzTourCmd,          /* CMD_SRT_MAN_PTZ_TOUR */
    StopManualPtzTourCmd,           /* CMD_STP_MAN_PTZ_TOUR */
    SearchRecordDataCmd,            /* CMD_SEARCH_RCD_DATA */
    SearchRecordAllDataCmd,         /* CMD_SEARCH_RCD_ALL_DATA */
    GetAsyncPlayStreamIdCmd,        /* CMD_GET_PLY_STRM_ID */
    AsyncPlayRecordStreamCmd,       /* CMD_PLY_RCD_STRM */
    AsyncPauseRecordStreamCmd,      /* CMD_PAUSE_RCD_STRM */
    AsyncResumeRecordStreamCmd,     /* CMD_RSM_PLY_RCD_STRM */
    AsyncStepRecordStreamCmd,       /* CMD_STEP_RCD_STRM */
    AsyncStopRecordStreamCmd,       /* CMD_STP_RCD_STRM */
    ClearAsyncPlayStreamIdCmd,      /* CMD_CLR_PLY_STRM_ID */
    SearchEventLogCmd,              /* CMD_SEARCH_STR_EVT */
    GetOnlineUserCmd,               /* CMD_ONLINE_USR */
    BlockUserCmd,                   /* CMD_BLK_USR */
    ViewBlockedUserCmd,             /* CMD_VIEW_BLK_USR */
    UnBlockUserCmd,                 /* CMD_UNBLK_USR */
    GetPhysicalDiskStatusCmd,       /* CMD_PHYSICAL_DISK_STS */
    GetLogicalDiskStatusCmd,        /* CMD_LOGICAL_VOLUME_STS */
    HarddiskFormatCmd,              /* CMD_HDDFORMAT */
    StopRaidCmd,                    /* CMD_CNCL_RAID_PRCS */
    StartManualTriggerCmd,          /* CMD_SRT_MAN_TRG */
    StopManualTriggerCmd,           /* CMD_STP_MAN_TRG */
    GetDateTimeCmd,                 /* CMD_GET_DATE_TIME */
    SetDateTimeCmd,                 /* CMD_SET_DATE_TIME */
    RestoreConfigCmd,               /* CMD_RSTR_CFG */
    UpdateFirmwareCmd,              /* CMD_UPDT_FRM */
    SystemLogoutCmd,                /* CMD_LOGOUT */
    SystemShutdownCmd,              /* CMD_SHUTDOWN */
    SystemRestartCmd,               /* CMD_RESTART */
    FactoryDefaultCmd,              /* CMD_FAC_DEF_CFG */
    UpdateDdnsCmd,                  /* CMD_UPDT_DDNS */
    TestMailCmd,                    /* CMD_TST_MAIL */
    TestFtpConnCmd,                 /* CMD_TST_FTP_CON */
    TestTcpConnCmd,                 /* CMD_TST_TCP_CON */
    TestSmsConnCmd,                 /* CMD_TST_SMS */
    GetUsbStatusCmd,                /* CMD_USB_DISK_STS */
    UsbFormatCmd,                   /* CMD_USBFORMAT */
    UsbUnplugCmd,                   /* CMD_USBUNPLUG */
    StopBackupCmd,                  /* CMD_STP_BCKUP */
    SetAlarmOutputCmd,              /* CMD_ALRM_OUT */
    GetMacAddressCmd,               /* CMD_GET_MAC */
    RecordRightsStatusCmd,          /* CMD_REC_RGT_STS */
    BackupRightsStatusCmd,          /* CMD_BCK_RGT_STS */
    GetHelthStatusCmd,              /* CMD_HEALTH_STS */
    MuteBuzzerCmd,                  /* CMD_BUZ_CTRL */
    GetSnapshotCmd,                 /* CMD_SNP_SHT */
    BackUpRecordCmd,                /* CMD_BKUP_RCD */
    GetModemStatusCmd,              /* CMD_CNCT_STS */
    GetSmsBalanceCmd,               /* CMD_CHK_BAL */
    ChangeUserPasswordCmd,          /* CMD_CNG_PWD */
    ChangeUserSessionCmd,           /* CMD_CNG_USER */
    GetBrandNameCmd,                /* CMD_BRND_NAME */
    GetModelNameCmd,                /* CMD_MDL_NAME */
    GetSupportedEncoderCmd,         /* CMD_ENCDR_SUP */
    GetSupportedResolutionCmd,      /* CMD_RES_SUP */
    GetSupportedFrameRateCmd,       /* CMD_FR_SUP */
    GetSupportedVideoQualityCmd,    /* CMD_QLT_SUP */
    GetOtherSupportedFeaturesCmd,   /* CMD_OTHR_SUP */
    UpdateMatrixDnsHostNameCmd,     /* CMD_UPDT_HOST_NM */
    CheckDeviceIdCmd,               /* CMD_CHK_DID */
    CheckUserCameraPrivilegeCmd,    /* CMD_CHK_PRIV */
    GetAdvanceStatusCmd,            /* CMD_ADVANCE_STS */
    GetAdvanceCameraStatusCmd,      /* CMD_ADVANCE_STS_CAMERA */
    GetCosecVideoPopUpDetailCmd,    /* CMD_COSEC_VIDEO_POPUP */
    AutoSearchCameraCmd,            /* CMD_AUTO_SEARCH */
    AddCamerasCmd,                  /* CMD_ADD_CAMERAS */
    CancelCameraSearchCmd,          /* CMD_CNCL_SEARCH */
    TestCameraCmd,                  /* CMD_TST_CAM */
    PlaySearchMonthWiseCmd,         /* CMD_PLYBCK_SRCH_MNTH */
    PlaySearchDayWiseCmd,           /* CMD_PLYBCK_SRCH_DAY */
    SyncPlayStartCmd,               /* CMD_PLYBCK_RCD */
    SyncPlayPauseCmd,               /* CMD_PAUSE_RCD */
    SyncPlayResumeCmd,              /* CMD_RSM_RCD */
    SyncPlayStopCmd,                /* CMD_STP_RCD */
    SyncPlayClearCmd,               /* CMD_CLR_RCD */
    SyncPlaySetPositionCmd,         /* CMD_SYNC_PLYBCK_RCD */
    SyncRecordClipCmd,              /* CMD_SYNC_CLP_RCD */
    GetCameraMobileNumberCmd,       /* CMD_GET_MOB_NO */
    GetCameraUserDetailCmd,         /* CMD_GET_USER_DETAIL */
    GetSystemBuildDateCmd,          /* CMD_GET_BUILD_DATE */
    GetCameraInfoCmd,               /* CMD_GET_CAMERA_INFO */
    StartInstantPlaybackCmd,        /* CMD_STRT_INSTANT_PLY */
    SeekInstantPlaybackCmd,         /* CMD_SEEK_INSTANT_PLY */
    StopInstantPlayback,            /* CMD_STOP_INSTANT_PLY */
    PauseInstantPlaybackCmd,        /* CMD_PAUSE_INSTANT_PLY */
    ResumeInstantPlaybackCmd,       /* CMD_RSM_INSTANT_PLY */
    GetSupportedProfileCmd,         /* CMD_GET_MAX_SUPP_PROF */
    GetProfileParamCmd,             /* CMD_GET_PROF_PARA */
    GetSupportedBitRateCmd,         /* CMD_GET_BITRATE */
    GetMotionWindowCmd,             /* CMD_GET_MOTION_WINDOW */
    ChangeCameraIpAddressCmd,       /* CMD_CHANGE_IP_CAM_ADDRS */
    AdvanceCameraSearchCmd,         /* CMD_ADV_CAM_SEARCH */
    PtzTourResumeCmd,               /* CMD_RESUME_PTZ_TOUR */
    SetMotionWindowCmd,             /* CMD_SET_MOTION_WINDOW */
    GenerateFailureReportCmd,       /* CMD_GENERATE_FAILURE_REPORT */
    GetPtzTourStatusCmd,            /* CMD_PTZ_TOUR_STATUS */
    GetConfiguredCameraCmd,         /* CMD_MAX_ADD_CAM */
    GetPrivacyMaskCmd,              /* CMD_GET_PRIVACY_MASK_WINDOW */
    SetPrivacyMaskCmd,              /* CMD_SET_PRIVACY_MASK_WINDOW */
    TestNddConnectionCmd,           /* CMD_TST_ND_CON */
    AutoConfigureCameraCmd,         /* CMD_AUTO_CONFIGURE */
    GetAcquiredCameraListCmd,       /* CMD_GET_ACQ_LIST */
    GetCpuLoadCmd,                  /* CMD_CPU_LOAD */
    GetUserRightsCmd,                /* CMD_GET_USR_RIGHTS */
    GetRegionalSettingsCmd,         /* CMD_GET_REG_CFG */
    SendLayoutFileCmd,              /* CMD_SND_LAYOUT_FILE */
    ReceiveLayoutFileCmd,           /* CMD_RCV_LAYOUT_FILE */
    AutoConfigStatusReportCmd,      /* CMD_AUTO_CFG_STATUS_RPRT */
    StartTxClientAudioCmd,          /* CMD_STRT_CLNT_AUDIO */
    StopTxClientAudioCmd,           /* CMD_STOP_CLNT_AUDIO */
    StartRxClientAudioCmd,          /* CMD_SND_AUDIO */
    StopRxClientAudioCmd,           /* CMD_STP_AUDIO */
    GetCameraInitiatedListCmd,      /* CMD_GET_CAM_INITIATED_LIST */
    AddCameraInitiatedCmd,          /* CMD_ADD_CAM_INITIATED */
    RejectCameraInitiatedCmd,       /* CMD_RJCT_CAM_INITIATED */
    ManualBackupCmd,                /* CMD_MAN_BACKUP */
    GetLanguageCmd,                 /* CMD_GET_LANGUAGE */
    GetUserLanguageCmd,             /* CMD_GET_USER_LANGUAGE */
    SetUserLanguageCmd,             /* CMD_SET_USER_LANGUAGE */
    GetStatusCmd,                   /* CMD_GET_STATUS */
    SearchFirmwareCmd,              /* CMD_SEARCH_FIRMWARE */
    StartUpgradeCmd,                /* CMD_START_UPGRADE */
    GetDhcpLeaseCmd,                /* CMD_GET_DHCP_LEASE */
    GetCapabilityCmd,               /* CMD_GET_CAPABILITY */
    EnablePushNotificationCmd,      /* CMD_ENABLE_PUSH */
    GetPushNotificationStatusCmd,   /* CMD_GET_PUSH_STATUS */
    GetPushNotificationDevListCmd,  /* CMD_GET_PUSH_DEV_LIST */
    DeletePushNotificationDevCmd,   /* CMD_DEL_PUSH_DEV */
    TestEmailIdCmd,                 /* CMD_TEST_EMAIL_ID */
    GetPasswordResetInfoCmd,        /* CMD_GET_PWD_RST_INFO */
    SetPasswordResetInfoCmd,        /* CMD_SET_PWD_RST_INFO */
    GetManualBackupLocationCmd,     /* CMD_GET_MAN_BKP_LOC */
    ValidateUserCredentialCmd,      /* CMD_VALIDATE_USER_CRED */
};

// hsReply variable should modify only at init time, not any other place
static HEALTH_STATUS_REPLY_t	hsReply[HS_MAX_STATUS] =
{
    {GetCamEventMotionHealthSts,MAX_CAMERA,     MOTION_DETECT},     //Motion
    {GetCamEventStatus,         MAX_CAMERA, 	VIEW_TEMPERING},	//View Tamper
    {GetCamEventStatus,         MAX_CAMERA, 	CONNECTION_FAILURE},//Connection Status
    {GetCamEventStatus,         MAX_CAMERA, 	CAMERA_SENSOR_1},	//Camera sensor 1
    {GetCamEventStatus,         MAX_CAMERA, 	CAMERA_SENSOR_2},	//Camera sensor 2
    {GetCamEventStatus,         MAX_CAMERA, 	CAMERA_SENSOR_3},	//Camera sensor 3
    {GetCamAlarmState,          MAX_CAMERA, 	0},					//Camera Alarm 1
    {GetCamAlarmState,          MAX_CAMERA, 	1},					//Camera Alarm 2
    {GetCamAlarmState,          MAX_CAMERA, 	2},					//Camera Alarm 3
    {GetRecordStatus,           MAX_CAMERA, 	MANUAL_RECORD},		//Manual Rrecording
    {GetRecordStatus,           MAX_CAMERA, 	ALARM_RECORD},		//Alarm Recording
    {GetRecordStatus,           MAX_CAMERA, 	SCHEDULE_RECORD},	//Schedule Recording
    {GetCamTourStatus,          MAX_CAMERA, 	0},					//Preset Tour
    {GetSensorStatus,           MAX_SENSOR, 	0},					//System Sensor
    {GetAlarmStatus,            MAX_ALARM, 		0},					//System Alarm
    {GetMediaHealthStatus,      1, 				0},					//Hdd Status
    {GetBackupStatus,           1, 				DM_SCHEDULE_BACKUP},//Schedule Backup
    {GetManTriggerStatus,       1, 				0},					//Manual Trigger
    {NULL,                      1, 				0},					//Mains Status (NA)
    {GetAllBuzzerStatus,        1,				0},					//Buzzer status
    {UpdateUsbHealthStatus,     DM_MAX_BACKUP, 	0},					//Usb disk
    {GetRecordStatus,           MAX_CAMERA, 	COSEC_RECORD},		//Cosec Recording
    {GetCameraStreamStatus,     MAX_CAMERA,	    0},					//Camera Status
    {GetCamEventStatus,         MAX_CAMERA,	   	LINE_CROSS},		//TripWire
    {GetCamEventStatus,         MAX_CAMERA,	    OBJECT_INTRUSION},	//Object Intrusion
    {GetCamEventStatus,         MAX_CAMERA,	    AUDIO_EXCEPTION},	//Audio Exception
    {GetCamEventStatus,         MAX_CAMERA,	    MISSING_OBJECT},	//Missing_Object
    {GetCamEventStatus,         MAX_CAMERA,	    SUSPICIOUS_OBJECT},	//Suspicious object
    {GetCamEventStatus,         MAX_CAMERA,	    LOITERING},			//loitering
    {GetCamEventStatus,         MAX_CAMERA,	    OBJECT_COUNTING},
    {GetAdaptiveRecordStatus,   MAX_CAMERA,     0},                 //Adaptive Recording
};

static INT32 testCamConnFd[MAX_CAMERA] =
{
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
#if defined(HI3536_NVRH) || defined(RK3588_NVRH)
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
#endif
#if defined(RK3588_NVRH)
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
    INVALID_CONNECTION,	INVALID_CONNECTION,	INVALID_CONNECTION, INVALID_CONNECTION,
#endif
};

// This pointer is constant and used in evry health status comparision
static const HEALTH_STATUS_REPLY_t *const healthStatusReply = (HEALTH_STATUS_REPLY_t*)hsReply;

//-------------------------------------------------------------------------------------------------
/* Prepare command response string and send on socket as per client type */
const NW_CMD_REPLY_CB clientCmdRespCb[CLIENT_CB_TYPE_MAX] =
{
    nativeClientCmdResp,    /* Native TCP */
    p2pClientCmdResp        /* P2P */
};

//-------------------------------------------------------------------------------------------------
/* Send command response string data on socket based on client type */
const SEND_TO_SOCKET_CB sendCmdCb[CLIENT_CB_TYPE_MAX] =
{
    SendToSocket,           /* Native TCP */
    P2pCmdSendCallback      /* P2P */
};

//-------------------------------------------------------------------------------------------------
/* Send frame data on socket based on client type */
const SEND_TO_SOCKET_CB sendDataCb[CLIENT_CB_TYPE_MAX] =
{
    SendToSocket,           /* Native TCP */
    P2pDataSendCallback     /* P2P */
};

//-------------------------------------------------------------------------------------------------
/* Send client data on socket based on client type */
const SEND_TO_CLIENT_CB sendClientDataCb[CLIENT_CB_TYPE_MAX] =
{
    SendToClient,           /* Native TCP */
    sendP2pFrameData        /* P2P */
};

//-------------------------------------------------------------------------------------------------
/* Close socket based on client type */
const CLOSE_SOCKET_CB closeConnCb[CLIENT_CB_TYPE_MAX] =
{
    CloseSocket,            /* Native TCP */
    P2pCloseConnCallback    /* P2P */
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of Network Command Module
 * @note    This value should be modify only one time after bootup. hsReply must not modify in any
 *          other places. Runtime Detection and healthStatusReply pointer will used in everyplace
 *          instead of hsReply
 */
void InitNetworkCommand(void)
{
    HEALTH_STATUS_REPLY_e hsIdx;

    for (hsIdx = 0 ; hsIdx < HS_MAX_STATUS; hsIdx++)
    {
        if (((hsIdx >= HS_MOTION_DET) && (hsIdx <= HS_PRESET_TOUR)) || ((hsIdx >= HS_COSEC_RECORDING) && (hsIdx <= HS_ADA_RECORDING)))
        {
            hsReply[hsIdx].majorIndex = getMaxCameraForCurrentVariant();
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief       SET_CMD request from client will served from here depending on parsed command value.
 * @param[in]   pCmdStr
 * @param[in]   sessionIndex
 * @param[in]   clientSocket
 * @param[in]   commandCallback:
 * @return
 */
BOOL ProcessSetCommand(CHARPTR pCmdStr, UINT8 sessionIndex, INT32 clientSocket, CLIENT_CB_TYPE_e callbackType)
{
    NET_COMMAND_e	cmdType;
    CHAR			cmdTypeStr[MAX_COMMAND_ARG_LEN];

    /* If parsing of command value failed */
    do
    {
        if (FAIL == ParseStr(&pCmdStr, FSP, cmdTypeStr, MAX_COMMAND_ARG_LEN))
        {
            cmdTypeStr[0] = '\0';
            break;
        }

        /* Parse command string, If it is valid then call related function */
        cmdType = ConvertStringToIndex(cmdTypeStr, netCommandStr, MAX_NET_COMMAND);
        if (cmdType >= MAX_NET_COMMAND)
        {
            break;
        }

        if (cmdType != CMD_CPU_LOAD)
        {
            if (callbackType == CLIENT_CB_TYPE_P2P)
            {
                DPRINT(NETWORK_MANAGER, "[cmd=%s], [p2pId=0x%x], [sessionIdx=%d]", netCommandStr[cmdType], clientSocket, sessionIndex);
            }
            else
            {
                DPRINT(NETWORK_MANAGER, "[cmd=%s], [fd=%d], [sessionIdx=%d]", netCommandStr[cmdType], clientSocket, sessionIndex);
            }
        }

        /* Call function as per command */
        (*cmsCommandFuncPtr[cmdType])(&pCmdStr, callbackType, clientSocket, sessionIndex);
        return SUCCESS;

    }while(0);



    clientCmdRespCb[callbackType](CMD_INVALID_MESSAGE, clientSocket, TRUE);
    EPRINT(NETWORK_MANAGER, "invld network command: [cmdType=%s], [msgStr=%s]", cmdTypeStr, pCmdStr);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function for command response for native type client
 * @param   status
 * @param   connFd
 * @param   closeConnF
 */
static void nativeClientCmdResp(NET_CMD_STATUS_e status, INT32 connFd, BOOL closeConnF)
{
    sendClientCmdResp(status, connFd, closeConnF, CLIENT_CB_TYPE_NATIVE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function for command response for p2p type client
 * @param   status
 * @param   connFd
 * @param   closeConnF
 */
static void p2pClientCmdResp(NET_CMD_STATUS_e status, INT32 connFd, BOOL closeConnF)
{
    sendClientCmdResp(status, connFd, closeConnF, CLIENT_CB_TYPE_P2P);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send command reply to client
 * @param   status
 * @param   connFd
 * @param   closeConnF
 * @param   clientCbType
 */
static void sendClientCmdResp(NET_CMD_STATUS_e status, INT32 connFd, BOOL closeConnF, CLIENT_CB_TYPE_e clientCbType)
{
    UINT16  outLen;
    CHAR    respMsg[256];

    outLen = snprintf(respMsg, sizeof(respMsg), "%c%s%c%03d%c%c", SOM, headerReq[RPL_CMD], FSP, status, FSP, EOM);

    if (FAIL == sendCmdCb[clientCbType](connFd, (UINT8PTR)respMsg, outLen, MESSAGE_REPLY_TIMEOUT))
    {
        closeConnCb[clientCbType](&connFd);
    }

    if (closeConnF == TRUE)
    {
        closeConnCb[clientCbType](&connFd);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send frame data on p2p socket. It is a wrapper function only.
 * @param   connFd
 * @param   pSendBuff
 * @param   buffLen
 * @param   timeout
 * @return  Returns -1 on failure else number data bytes sent on socket
 */
static INT32 sendP2pFrameData(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeout)
{
    /* Is data send failed? */
    if (FAIL == P2pDataSendCallback(connFd, pSendBuff, buffLen, SEND_PACKET_TIMEOUT))
    {
        /* Failed to send data on p2p socket */
        return -1;
    }

    /* Return sent bytes on socket */
    return buffLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start live stream of camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StartLiveStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SRT_LV_STRM_ARG];
    VIDEO_TYPE_e			streamType;
    UINT8					cameraIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					status;
    UINT8					frameType = 0;
    UINT8					frameTypeMPJEG = 0;
    UINT8					fpsMPJEG = 0;
    CAMERA_CONFIG_t		    cameraCfg;

    memset(tempData, INVALID_CAMERA_INDEX, sizeof(tempData));
    status = ParseStringGetVal(pCmdStr, tempData, MAX_SRT_LV_STRM_ARG, FSP);

    do
    {
        if ((tempData[SRT_LV_STRM_CAMERA] == INVALID_CAMERA_INDEX) && (tempData[SRT_LV_STRM_TYPE] == INVALID_CAMERA_INDEX))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SRT_LV_STRM_CAMERA]);
        if (cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.monitor == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        ReadSingleCameraConfig(cameraIndex, &cameraCfg);
        if (cameraCfg.camera == DISABLE)
        {
            cmdResp = CMD_CHANNEL_DISABLED;
            break;
        }

        if (GetCamEventStatus(cameraIndex, CONNECTION_FAILURE)== INACTIVE)
        {
            cmdResp = CMD_CAM_DISCONNECTED; // To stop stream retry from UI
            break;
        }

        if (status == SUCCESS)
        {
            frameType = (UINT8)tempData[SRT_LV_STRM_FRAME_TYPE];
            frameTypeMPJEG = (UINT8)tempData[SRT_LV_STRM_FRAME_TYPE_MJEPG];
            fpsMPJEG = (UINT8)tempData[SRT_LV_STRM_FPS_MJPEG];
            if (fpsMPJEG == 0)
            {
                frameTypeMPJEG = 0;
            }
        }

        streamType = (VIDEO_TYPE_e)tempData[SRT_LV_STRM_TYPE];
        cmdResp = AddLiveMediaStream(cameraIndex, streamType, sessionIndex, clientSocket, clientCbType, frameType, frameTypeMPJEG, fpsMPJEG);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop live stream of camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopLiveStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT64              tempData[MAX_STP_LV_STRM_ARG];
    UINT8               cameraIndex;
    VIDEO_TYPE_e        streamType = MAX_STREAM;

    tempData[STP_LV_STRM_CAMERA] = INVALID_CAMERA_INDEX;
    tempData[STP_LV_STRM_STREAM_TYPE] = MAX_STREAM;

    if (SUCCESS == ParseStringGetVal(pCmdStr, tempData, MAX_STP_LV_STRM_ARG, FSP))
    {
        streamType = (VIDEO_TYPE_e)tempData[STP_LV_STRM_STREAM_TYPE];
    }

    cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[STP_LV_STRM_CAMERA]);
    if(cameraIndex < getMaxCameraForCurrentVariant())
    {
        if(streamType != MAX_STREAM)
        {
            RemoveLiveMediaStream(cameraIndex, streamType, sessionIndex);
        }
        else
        {
            for(streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
            {
                RemoveLiveMediaStream(cameraIndex, streamType, sessionIndex);
            }
        }

        /* Disable Two way audio if enabled when camera config changed to disabled */
        TerminateClientMediaStreamer(cameraIndex, sessionIndex);
    }
    else
    {
        cmdResp = CMD_INVALID_FIELD_VALUE;
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Include audio in live stream of camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL IncludeLiveAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_INC_LV_AUDIO_ARG];
    UINT8					cameraIndex;
    VIDEO_TYPE_e			streamType;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    tempData[INC_LV_AUDIO_CAMERA] = INVALID_CAMERA_INDEX;
    tempData[INC_LV_AUDIO_STREAM_TYPE] = MAX_STREAM;

    if (SUCCESS == ParseStringGetVal(pCmdStr, tempData, MAX_INC_LV_AUDIO_ARG, FSP))
    {
        streamType = (VIDEO_TYPE_e)tempData[INC_LV_AUDIO_STREAM_TYPE];
    }
    else
    {
        streamType = MAIN_STREAM;
    }

    do
    {
        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[INC_LV_AUDIO_CAMERA]);
        if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (streamType >= MAX_STREAM))
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.audio == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        STREAM_CONFIG_t streamConfig;
        if(TRUE != ReadSingleStreamConfig(cameraIndex, &streamConfig))
        {
            EPRINT(NETWORK_MANAGER, "failed to read camera config: [camera=%d]", cameraIndex);
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        // check audio enable flag as per the stream type
        if (MAIN_STREAM == streamType)
        {
            if (FALSE == streamConfig.enableAudio)
            {
                DPRINT(NETWORK_MANAGER, "main stream audio disabled in config: [camera=%d]", cameraIndex);
                cmdResp = CMD_AUDIO_DISABLED;
            }
        }
        else
        {
            if (FALSE == streamConfig.enableAudioSub)
            {
                DPRINT(NETWORK_MANAGER, "sub stream audio disabled in config: [camera=%d]", cameraIndex);
                cmdResp = CMD_AUDIO_DISABLED;
            }
        }

        /* Enable audio if user have rights else disable it (If already started) */
        if (ChangeLiveMediaAudioState(cameraIndex, streamType, sessionIndex, (cmdResp == CMD_SUCCESS) ? ENABLE : DISABLE) == FAIL)
        {
            EPRINT(NETWORK_MANAGER, "fail to enable audio: [camera=%d]", cameraIndex);
            cmdResp = CMD_PROCESS_ERROR;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Exclude audio from live stream of camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ExcludeLiveAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_EXC_LV_AUDIO_ARG];
    UINT8					cameraIndex;
    VIDEO_TYPE_e			streamType = MAX_STREAM;

    tempData[EXC_LV_AUDIO_CAMERA] = INVALID_CAMERA_INDEX;
    tempData[EXC_LV_AUDIO_STREAM_TYPE] = MAX_STREAM;

    if (SUCCESS == ParseStringGetVal(pCmdStr, tempData, MAX_EXC_LV_AUDIO_ARG, FSP))
    {
        streamType = (VIDEO_TYPE_e)tempData[EXC_LV_AUDIO_STREAM_TYPE];
    }

    cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[EXC_LV_AUDIO_CAMERA]);
    if(cameraIndex < getMaxCameraForCurrentVariant())
    {
        if(streamType != MAX_STREAM)
        {
            ChangeLiveMediaAudioState(cameraIndex, streamType, sessionIndex, DISABLE);
        }
        else
        {
            for(streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
            {
                ChangeLiveMediaAudioState(cameraIndex, streamType, sessionIndex, DISABLE);
            }
        }
    }
    else
    {
        cmdResp = CMD_INVALID_FIELD_VALUE;
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Change strem type of live stream of camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ChangeLiveStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_CHNG_LV_STRM_ARG];
    UINT8					cameraIndex;
    VIDEO_TYPE_e			streamType;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					status;
    UINT8					frameType = 0;
    UINT8					frameTypeMPJEG = 0;
    UINT8					fpsMPJEG = 0;

    memset(tempData, INVALID_CAMERA_INDEX, sizeof(tempData));
    status = ParseStringGetVal(pCmdStr, tempData,	MAX_CHNG_LV_STRM_ARG, FSP);

    do
    {
        if((tempData[CHNG_LV_STRM_CAMERA] == INVALID_CAMERA_INDEX) && (tempData[CHNG_LV_STRM_TYPE] == INVALID_CAMERA_INDEX))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[CHNG_LV_STRM_CAMERA]);
        if (cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.monitor == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (status == SUCCESS)
        {
            frameType = (UINT8)tempData[CHNG_LV_STRM_FRAME_TYPE];
            frameTypeMPJEG = (UINT8)tempData[CHNG_LV_STRM_FRAME_TYPE_MJEPG];
            fpsMPJEG = (UINT8)tempData[CHNG_LV_STRM_FPS_MJPEG];
            if (fpsMPJEG == 0)
            {
                frameTypeMPJEG = 0;
            }
        }
        else
        {
            DPRINT(NETWORK_MANAGER, "no change in stream param in change live stream cmd: [camera=%d]", cameraIndex);
        }

        streamType = (VIDEO_TYPE_e)tempData[CHNG_LV_STRM_TYPE];
        if (ChangeLiveMediaStream(cameraIndex, streamType, sessionIndex, clientSocket, clientCbType, frameType,frameTypeMPJEG,fpsMPJEG) == FAIL)
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute start manual record cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StartManualRecordingCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SRT_MAN_REC_ARG];
    UINT8					cameraIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    if(ParseStringGetVal(pCmdStr, tempData, MAX_SRT_MAN_REC_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SRT_MAN_REC_CAMERA]);
        if ((userAccountConfig.userGroup == VIEWER) || (userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.monitor == DISABLE))
        {
            cmdResp = CMD_NO_PRIVILEGE;
        }
        else
        {
            cmdResp = StartRecord(cameraIndex, MANUAL_RECORD, DEFAULT_REC_DURATION, userAccountConfig.username);
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute stop manual record cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopManualRecordingCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_STP_MAN_REC_ARG];
    UINT8					cameraIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    if(ParseStringGetVal(pCmdStr, tempData, MAX_SRT_MAN_REC_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SRT_MAN_REC_CAMERA]);
        if ((userAccountConfig.userGroup == VIEWER) || (userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.monitor == DISABLE))
        {
            cmdResp = CMD_NO_PRIVILEGE;
        }
        else
        {
            cmdResp = StopRecord(cameraIndex, MANUAL_RECORD, userAccountConfig.username);
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set PTZ cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetPanTiltCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SETPANTILT_ARG];
    UINT8					cameraIndex;
    PTZ_OPTION_e			pan, tilt;
    UINT8 					speed;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    PRESET_TOUR_CONFIG_t	presetTourConfig;
    PTZ_TOUR_HLT_STATUS_e	tourStatus;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_SETPANTILT_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SETPANTILT_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        /* Check tour status */
        tourStatus = GetCamTourStatus(cameraIndex, 0);
        ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);
        if ((tourStatus == PTZ_HLT_SCH_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_SCHEDULE_TOUR_ON;
            break;
        }

        if ((tourStatus == PTZ_HLT_MAN_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_MANUAL_TOUR_ON;
            break;
        }

        pan = (PTZ_OPTION_e)(tempData[SETPANTILT_PAN]);
        tilt = (PTZ_OPTION_e)(tempData[SETPANTILT_TILT]);
        speed = (UINT8)tempData[SETPANTILT_SPEED];
        cmdResp = SetPtzPosition(cameraIndex, pan, tilt, MAX_PTZ_ZOOM_OPTION, speed, sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set zoom cmd
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetZoomCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SETZOOM_ARG];
    UINT8					cameraIndex;
    PTZ_OPTION_e			zoom;
    UINT8                   speed;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    PRESET_TOUR_CONFIG_t	presetTourConfig;
    PTZ_TOUR_HLT_STATUS_e	tourStatus;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, SETZOOM_SPEED, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[SETZOOM_SPEED], 1, FSP) == FAIL)
        {
            speed = 1;
        }
        else
        {
            speed = (UINT8)tempData[SETZOOM_SPEED];
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SETZOOM_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex]. privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        /* Check tour status */
        tourStatus = GetCamTourStatus(cameraIndex, 0);
        ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);
        if ((tourStatus == PTZ_HLT_SCH_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_SCHEDULE_TOUR_ON;
            break;
        }

        if ((tourStatus == PTZ_HLT_MAN_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_MANUAL_TOUR_ON;
            break;
        }

        zoom = (PTZ_OPTION_e)tempData[SETZOOM_ZOOM];
        cmdResp = SetPtzPosition(cameraIndex, MAX_PTZ_PAN_OPTION, MAX_PTZ_TILT_OPTION, zoom, speed, sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set focus cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetFocusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SETFOCUS_ARG];
    UINT8					cameraIndex;
    CAMERA_FOCUS_e			focus;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    PRESET_TOUR_CONFIG_t	presetTourConfig;
    PTZ_TOUR_HLT_STATUS_e	tourStatus;
    UINT8                   speed;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, SETFOCUS_SPEED, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[SETFOCUS_SPEED], 1, FSP) == FAIL)
        {
            speed = 1;
        }
        else
        {
            speed = (UINT8)tempData[SETFOCUS_SPEED];
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SETFOCUS_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        /* Check tour status */
        tourStatus = GetCamTourStatus(cameraIndex, 0);
        ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);
        if ((tourStatus == PTZ_HLT_SCH_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_SCHEDULE_TOUR_ON;
            break;
        }

        if ((tourStatus == PTZ_HLT_MAN_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_MANUAL_TOUR_ON;
            break;
        }

        focus = (CAMERA_FOCUS_e)tempData[SETFOCUS_FOCUS];
        cmdResp = SetFocus(cameraIndex, focus, speed, sessionIndex, clientCmdRespCb[clientCbType],clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set iris cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetIrisCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SETIRIS_ARG];
    UINT8					cameraIndex;
    CAMERA_IRIS_e			iris;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    PRESET_TOUR_CONFIG_t	presetTourConfig;
    PTZ_TOUR_HLT_STATUS_e	tourStatus;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_SETIRIS_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SETIRIS_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        /* Check tour status */
        tourStatus = GetCamTourStatus(cameraIndex, 0);
        ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);
        if ((tourStatus == PTZ_HLT_SCH_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_SCHEDULE_TOUR_ON;
            break;
        }

        if ((tourStatus == PTZ_HLT_MAN_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_MANUAL_TOUR_ON;
            break;
        }

        iris = (CAMERA_IRIS_e)tempData[SETIRIS_IRIS];
        cmdResp = SetIris(cameraIndex, iris, sessionIndex, clientCmdRespCb[clientCbType],clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set PTZ index cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL CallPtzPresetCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_CALLPRESET_ARG];
    UINT8					cameraIndex;
    UINT8					ptzIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    PRESET_TOUR_CONFIG_t	presetTourConfig;
    PTZ_TOUR_HLT_STATUS_e	tourStatus;
    CHAR                    detail[MAX_EVENT_DETAIL_SIZE];
    CHAR                    advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_CALLPRESET_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[CALLPRESET_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        /* Check tour status */
        tourStatus = GetCamTourStatus(cameraIndex, 0);
        ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);
        if ((tourStatus == PTZ_HLT_SCH_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_SCHEDULE_TOUR_ON;
            break;
        }

        if ((tourStatus == PTZ_HLT_MAN_TOUR) && (presetTourConfig.activeTourOverride == DISABLE))
        {
            cmdResp = CMD_MANUAL_TOUR_ON;
            break;
        }

        ptzIndex = (UINT8)(tempData[CALLPRESET_PRESET_NUM]);
        cmdResp = GotoPtzPosition(cameraIndex, ptzIndex, clientCmdRespCb[clientCbType], clientSocket,TRUE);
        if(cmdResp == CMD_SUCCESS)
        {
           snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
           snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%d", ptzIndex );
           WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_POSITION_CHANGE, detail, advncDetail, EVENT_ALERT);
           return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute start manual PTZ tour cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StartManualPtzTourCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SRT_MAN_PTZ_TOUR_ARG];
    UINT8					cameraIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_SRT_MAN_PTZ_TOUR_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SRT_MAN_PTZ_TOUR_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        cmdResp = StartManualPtzTour(cameraIndex, userAccountConfig.username);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute stop manual PTZ tour cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopManualPtzTourCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_STP_MAN_PTZ_TOUR_ARG];
    UINT8					cameraIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_STP_MAN_PTZ_TOUR_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[STP_MAN_PTZ_TOUR_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        cmdResp = StopManualPtzTour(cameraIndex, userAccountConfig.username);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search recording (Normal search)
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SearchRecordDataCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    BOOL                retVal = FAIL;
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT64              tempData[MAX_SEARCH_RCD_DATA_ARG];
    SEARCH_CRITERIA_t   searchCriteria;

    tempData[SEARCH_RCD_DATA_CAMERA] = INVALID_CAMERA_INDEX;
    tempData[SEARCH_RCD_DATA_TYPE] = DM_NONE_EVENT0;
    tempData[SEARCH_RCD_DATA_START_TIME] = 0;
    tempData[SEARCH_RCD_DATA_STOP_TIME] = 0;
    tempData[SEARCH_RCD_DATA_NUM_OF_DATA] = MAX_RECORD_SEARCH;
    tempData[SEARCH_RCD_DATA_STORAGE_TYPE] = MAX_RECORDING_MODE;

    do
    {
        retVal = ParseStringGetVal(pCmdStr, tempData, MAX_SEARCH_RCD_DATA_ARG, FSP);
        if (tempData[SEARCH_RCD_DATA_CAMERA] == INVALID_CAMERA_INDEX)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        searchCriteria.channelNo = (UINT8)(tempData[SEARCH_RCD_DATA_CAMERA]);
        if (searchCriteria.channelNo > getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        searchCriteria.eventType = (EVENT_e)tempData[SEARCH_RCD_DATA_TYPE];
        tempData[SEARCH_RCD_DATA_START_TIME] *= 100;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SEARCH_RCD_DATA_START_TIME], searchCriteria.startTime);
        tempData[SEARCH_RCD_DATA_STOP_TIME] *= 100;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SEARCH_RCD_DATA_STOP_TIME], searchCriteria.endTime);
        searchCriteria.noOfRecord = ((UINT16)tempData[SEARCH_RCD_DATA_NUM_OF_DATA] < MAX_RECORD_SEARCH) ?
                    (UINT16)tempData[SEARCH_RCD_DATA_NUM_OF_DATA] : MAX_RECORD_SEARCH;
        searchCriteria.searchRecStorageType = (retVal == FAIL) ?
                    MAX_RECORDING_MODE : (RECORD_ON_DISK_e)tempData[SEARCH_RCD_DATA_STORAGE_TYPE];
        cmdResp = SearchRecord(&searchCriteria, NORMAL_SEARCH, searchCriteria.searchRecStorageType, sessionIndex,
                               clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search recording (Async search)
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SearchRecordAllDataCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT64              tempData[MAX_SEARCH_RCD_DATA_ARG - 1];
    SEARCH_CRITERIA_t   searchCriteria;

    tempData[SEARCH_RCD_DATA_CAMERA] = MAX_CAMERA;
    tempData[SEARCH_RCD_DATA_TYPE] = DM_ANY_EVENT;
    tempData[SEARCH_RCD_DATA_START_TIME] = 0;
    tempData[SEARCH_RCD_DATA_STOP_TIME] = 0;
    tempData[SEARCH_RCD_DATA_NUM_OF_DATA] = MAX_RECORD_SEARCH;

    ParseStringGetVal(pCmdStr, tempData, (MAX_SEARCH_RCD_DATA_ARG - 1), FSP);
    searchCriteria.channelNo = (UINT8)(tempData[SEARCH_RCD_DATA_CAMERA]);
    if (searchCriteria.channelNo > getMaxCameraForCurrentVariant())
    {
        cmdResp = CMD_INVALID_FIELD_VALUE;
    }
    else
    {
        searchCriteria.eventType = (EVENT_e)tempData[SEARCH_RCD_DATA_TYPE];
        tempData[SEARCH_RCD_DATA_START_TIME] *= 100;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SEARCH_RCD_DATA_START_TIME], searchCriteria.startTime);
        tempData[SEARCH_RCD_DATA_STOP_TIME] *= 100;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SEARCH_RCD_DATA_STOP_TIME], searchCriteria.endTime);
        searchCriteria.noOfRecord = ((UINT16)tempData[SEARCH_RCD_DATA_NUM_OF_DATA] < MAX_RECORD_SEARCH) ?
                    (UINT16)tempData[SEARCH_RCD_DATA_NUM_OF_DATA] : MAX_RECORD_SEARCH;
        searchCriteria.searchRecStorageType = ALL_DRIVE;
        cmdResp = SearchRecord(&searchCriteria, ASYNC_ALL_SEARCH, searchCriteria.searchRecStorageType,
                               sessionIndex, clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get async record playback stream ID
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetAsyncPlayStreamIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_GET_PLY_STRM_ID_ARG];
    UINT8					cameraIndex;
    UINT8 					overlap;
    UINT8 					diskId;
    EVENT_e 				eventType;
    struct tm 				startTime, stopTime;
    PLAYBACK_ID 			playbackId;
    UINT32 					sizeWritten;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    RECORD_ON_DISK_e		recStorageType = MAX_RECORDING_MODE;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, GET_PLY_STRM_ID_STORAGE_TYPE, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[GET_PLY_STRM_ID_CAMERA]);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.playback == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        eventType = (EVENT_e)tempData[GET_PLY_STRM_ID_TYPE];
        overlap = (UINT8)tempData[GET_PLY_STRM_ID_OVERLAP];
        diskId = (UINT8)tempData[GET_PLY_STRM_ID_DISK_ID];
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[GET_PLY_STRM_ID_START_TIME], startTime);
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[GET_PLY_STRM_ID_STOP_TIME], stopTime);

        //this change is for Mobile client
        tempData[0] = 0;
        if(ParseStringGetVal(pCmdStr, tempData, 1, FSP) == FAIL)
        {
            recStorageType = MAX_RECORDING_MODE;
        }
        else
        {
            recStorageType = (RECORD_ON_DISK_e)tempData[0];
        }

        cmdResp = AddPlaybackStream(clientCbType, startTime, stopTime, cameraIndex, sessionIndex,
                                    eventType, overlap, diskId, recStorageType, &playbackId);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        /* NOTE: %02d : parsing on CMS side is byte size based and not format based */
        sizeWritten = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%02d%c%c%d%c%d%c%c%c",
                               SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, playbackId, FSP, EOI, EOM);
        if (SUCCESS != sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType], sizeWritten, MESSAGE_REPLY_TIMEOUT))
        {
            /* If failed to send data then remove playback stream from session */
            RemovePlaybackStream(playbackId, sessionIndex, NULL, INVALID_CONNECTION);
        }

        /* Close current connection */
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Play async record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AsyncPlayRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_PROCESS_ERROR;
    UINT64 					tempData[MAX_PLY_RCD_STRM_ARG];
    PLAYBACK_ID 			playbackId;
    PLAYBACK_CMD_e 			direction;
    PLAYBACK_SPEED_e		speed;
    BOOL					audio;
    struct tm 				startTime = { 0 };

    if(ParseStringGetVal(pCmdStr, tempData, MAX_PLY_RCD_STRM_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (PLAYBACK_ID)tempData[PLY_RCD_STRM_PLAYBACK_ID];
        direction = (tempData[PLY_RCD_STRM_DIRECTION] == 0) ? PLAY_FORWARD : PLAY_REVERSE;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[PLY_RCD_STRM_PLAY_TIME], startTime);
        audio = (BOOL)tempData[PLY_RCD_STRM_AUDIO];
        speed = (PLAYBACK_SPEED_e)tempData[PLY_RCD_STRM_SPEED];
        if (StartPlaybackStream(playbackId, sessionIndex, direction, startTime, audio, speed, clientCmdRespCb[clientCbType], clientSocket) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pause async record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AsyncPauseRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_PROCESS_ERROR;
    UINT64              tempData[MAX_PAUSE_RCD_STRM_ARG];
    PLAYBACK_ID         playbackId;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_PAUSE_RCD_STRM_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (PLAYBACK_ID)tempData[PAUSE_RCD_STRM_PLAYBACK_ID];
        if (PausePlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Resume async record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AsyncResumeRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_PROCESS_ERROR;
    UINT64              tempData[MAX_RSM_PLY_RCD_STRM_ARG];
    PLAYBACK_ID         playbackId;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_RSM_PLY_RCD_STRM_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (PLAYBACK_ID)tempData[RSM_PLY_RCD_STRM_PLAYBACK_ID];
        if (ResumePlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Play async record in steps (Play I-frames only)
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AsyncStepRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_PROCESS_ERROR;
    UINT64              tempData[MAX_STEP_RCD_STRM_ARG];
    PLAYBACK_ID         playbackId;
    PLAYBACK_CMD_e      direction;
    NW_FRAME_TYPE_e     frameType;
    UINT16              mSec;
    struct tm           startTime = { 0 };

    if (ParseStringGetVal(pCmdStr, tempData, MAX_STEP_RCD_STRM_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (PLAYBACK_ID)tempData[STEP_RCD_STRM_PLAYBACK_ID];
        direction = (tempData[STEP_RCD_STRM_STEP] == 0) ? STEP_FORWARD : STEP_REVERSE;
        frameType = (NW_FRAME_TYPE_e)tempData[STEP_RCD_STRM_FRAME_TYPE];
        mSec = tempData[STEP_RCD_STRM_TIME] % 1000;
        tempData[STEP_RCD_STRM_TIME] /= 1000;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[STEP_RCD_STRM_TIME], startTime);
        if (StepPlaybackStream(playbackId, sessionIndex, direction, frameType, startTime, mSec, clientCmdRespCb[clientCbType], clientSocket) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop async record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AsyncStopRecordStreamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_PROCESS_ERROR;
    UINT64              tempData[MAX_STP_RCD_STRM_ARG];
    PLAYBACK_ID         playbackId;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_STP_RCD_STRM_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (PLAYBACK_ID)tempData[STP_RCD_STRM_PLAYBACK_ID];
        if (StopPlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear async record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ClearAsyncPlayStreamIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_PROCESS_ERROR;
    UINT64              tempData[MAX_CLR_PLY_STRM_ID_ARG];
    PLAYBACK_ID         playbackId;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_CLR_PLY_STRM_ID_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (PLAYBACK_ID)tempData[CLR_PLY_STRM_ID_PLAYBACK_ID];
        if (RemovePlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute search event cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SearchEventLogCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SEARCH_STR_EVT_ARG];
    UINT64 					eventTypeMask = UINT64_MAX;
    EVENT_SEARCH_RESULT_e 	eventSearchResult = 0;
    LOG_EVENT_TYPE_e 		eventLogType;
    UINT16					noOfEvent, eventCounter = 0;
    struct tm 				eventStartTime, eventEndTime;
    EVENT_LOG_t 			*eventLog = NULL;
    UINT32 					counter, sizeWritten, resStringSize;
    CHARPTR 				pEventRespMsg = NULL, resStringPtr;

    do
    {
        /* Exclude event type mask from parsing to maintain backward compatibity. It is added later on. */
        if(ParseStringGetVal(pCmdStr, tempData, MAX_SEARCH_STR_EVT_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Get event search type. If it is all then check mask */
        eventLogType = (LOG_EVENT_TYPE_e)tempData[SEARCH_STR_EVT_TYPE];
        if (eventLogType == LOG_ANY_EVENT)
        {
            /* Check event type filter mask available or not */
            if (ParseStringGetVal(pCmdStr, &eventTypeMask, 1, FSP) == FAIL)
            {
                /* Event type filter mask not available. Set all bits 1 */
                eventTypeMask = UINT64_MAX;
            }
        }

        /* We will provide max 2000 events in one response */
        noOfEvent = (UINT16)tempData[SEARCH_STR_EVT_NUM_OF_REC];
        if(noOfEvent > MAX_EVENT_SEARCH)
        {
            noOfEvent = MAX_EVENT_SEARCH;
        }

        /* Alloc memory for event search local copy */
        eventLog = (EVENT_LOG_t *)malloc((noOfEvent * sizeof(EVENT_LOG_t)));
        if (eventLog == NULL)
        {
            EPRINT(NETWORK_MANAGER, "fail to alloc memory");
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        /* Get event search start and stop time */
        tempData[SEARCH_STR_EVT_START_TIME] *= 100;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SEARCH_STR_EVT_START_TIME], eventStartTime);
        tempData[SEARCH_STR_EVT_END_TIME] *= 100;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SEARCH_STR_EVT_END_TIME], eventEndTime);

        /* Start event search session */
        if(StartEventSearch(sessionIndex, eventLogType, &eventStartTime, &eventEndTime) == FAIL)
        {
            cmdResp = CMD_RESOURCE_LIMIT;
            break;
        }

        while (eventCounter < noOfEvent)
        {
            /* Read event one by one */
            eventSearchResult = ReadEvent(sessionIndex, &eventLog[eventCounter]);
            if (eventSearchResult != EVENT_FOUND)
            {
                /* No more event available */
                break;
            }

            /* Check current event type is available or not in filter mask */
            if (GET_BIT(eventTypeMask, (eventLog[eventCounter].eventType - 1)) == 0)
            {
                /* Event type is not available mask. Skip it */
                continue;
            }

            /* Update event counter */
            eventCounter++;
        }

        EndEventSearch(sessionIndex);
        if(eventSearchResult == EVENT_SEARCH_FAIL)
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if((eventCounter == 0) && (eventSearchResult == NO_MORE_EVENT))
        {
            cmdResp = CMD_NO_EVENT_FOUND;
            break;
        }

        /* Alloc memory for event response message */
        pEventRespMsg = (CHARPTR)malloc(EVENT_SEARCH_RESP_MAX);
        if (pEventRespMsg == NULL)
        {
            EPRINT(NETWORK_MANAGER, "fail to alloc memory");
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        if(eventCounter >= noOfEvent)
        {
            cmdResp = CMD_MORE_EVENTS;
        }

        resStringPtr = pEventRespMsg;
        resStringSize = EVENT_SEARCH_RESP_MAX;
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;

        for(counter = 0; counter < eventCounter; counter++)
        {
            sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%02d%02d%04d%02d%02d%02d%c%d%c%d%c%s%c%d%c%s%c%c",
                                   SOI, (counter + 1), FSP, eventLog[counter].eventTime.tm_mday, (eventLog[counter].eventTime.tm_mon + 1),
                                   eventLog[counter].eventTime.tm_year, eventLog[counter].eventTime.tm_hour, eventLog[counter].eventTime.tm_min,
                                   eventLog[counter].eventTime.tm_sec, FSP, eventLog[counter].eventType, FSP, eventLog[counter].eventSubtype,
                                   FSP, eventLog[counter].detail, FSP, eventLog[counter].eventState, FSP, eventLog[counter].advncDetail, FSP, EOI);
            if (sizeWritten >= resStringSize)
            {
                EPRINT(NETWORK_MANAGER, "max buffer limit reached: [eventCnt=%d]", counter);
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if (cmdResp == CMD_MAX_BUFFER_LIMIT)
        {
            break;
        }

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            EPRINT(NETWORK_MANAGER, "max buffer limit reached");
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)pEventRespMsg, strlen(pEventRespMsg), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        FREE_MEMORY(eventLog);
        FREE_MEMORY(pEventRespMsg);
        return SUCCESS;

    }while(0);

    FREE_MEMORY(eventLog);
    FREE_MEMORY(pEventRespMsg);
    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute get online user cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetOnlineUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8					counter, count = 1;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
    resStringPtr += sizeWritten;
    resStringSize -= sizeWritten;

    // Skip first user index (It is local client)
    for(counter = 1; counter < MAX_NW_CLIENT; counter++)
    {
        if(IsUserSessionActive(counter) == FALSE)
        {
            continue;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(counter), &userAccountConfig);
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%s%c%d%c%s%c%c",  SOI, count, FSP,
                               userAccountConfig.username, FSP, userAccountConfig.userGroup, FSP, GetUserIpAddr(counter), FSP, EOI);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        count++;
    }

    if(cmdResp == CMD_SUCCESS)
    {
        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
        }
        else
        {
            sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
            closeConnCb[clientCbType](&clientSocket);
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API blocks authenticates blocking request and after successful validation it blocks
 *          requested user.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 * @note    User will be allowed after reboot the device
 */
static BOOL BlockUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					userIndex;
    UINT8					tempSessnIndex;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if ((userAccountConfig.userGroup != ADMIN) || (strcmp(userAccountConfig.username, ADMIN_USER_NAME) != STATUS_OK))
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if (ParseStr(pCmdStr, FSP, userName, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else if (strcmp(userName, ADMIN_USER_NAME) == STATUS_OK)
    {
        cmdResp = CMD_ADMIN_CANT_BLOCKED;
    }
    else
    {
        for(userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if (strcmp(userAccountConfig.username, userName) != STATUS_OK)
            {
                continue;
            }

            // Block particular User
            SetUserBlockStatus(userIndex, TRUE);

            // Log - Out User if already loged-in
            for(tempSessnIndex = 0; tempSessnIndex < MAX_NW_CLIENT; tempSessnIndex++)
            {
                if(GetUserAccountIndex(tempSessnIndex) == userIndex)
                {
                    UserLogout(tempSessnIndex);
                }
            }
            break;
        }

        if(userIndex >= MAX_USER_ACCOUNT)
        {
            cmdResp = CMD_PROCESS_ERROR;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the list of blocked users.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ViewBlockedUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					userIndex;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    const UINT32            respStringSize = MAX_REPLY_SZ;
    UINT8					writeIndex = 1;
    UINT32                  outLen;

    /* Prepare resp msg header */
    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    for(userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
    {
        if (GetUserBlockStatus(userIndex) == FALSE)
        {
            continue;
        }

        // (1&user name&user group)(2&user name&user group)
        ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%d%c%c",
                           SOI, writeIndex++, FSP, userAccountConfig.username, FSP, userAccountConfig.userGroup, FSP, EOI);
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
            break;
        }
    }

    // Append EOM
    outLen += snprintf(respStringPtr + outLen,  respStringSize - outLen, "%c", EOM);
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to unblock the user.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL UnBlockUserCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					userIndex;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if ((userAccountConfig.userGroup != ADMIN) || (strcmp(userAccountConfig.username, ADMIN_USER_NAME) != STATUS_OK))
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if (ParseStr(pCmdStr, FSP, userName, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        for(userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if (strcmp(userAccountConfig.username, userName) == STATUS_OK)
            {
                // Un-Block particular User
                SetUserBlockStatus(userIndex, FALSE);
                break;
            }
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute get physical disk status cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetPhysicalDiskStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8 					diskIndex;
    PHYSICAL_DISK_INFO_t 	physicalDiskInfo;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];

    do
    {
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        for(diskIndex = 0; diskIndex < getMaxHardDiskForCurrentVariant(); diskIndex++)
        {
            if(GetPhysicalDiskPara(diskIndex, &physicalDiskInfo) == FAIL)
            {
                cmdResp = CMD_PROCESS_ERROR;
                break;
            }

            sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%s%c%s%c%d%c%c", SOI,  (diskIndex + 1), FSP,
                                   physicalDiskInfo.serialNo, FSP, physicalDiskInfo.capacity, FSP, physicalDiskInfo.status, FSP, EOI);
            if(sizeWritten >= resStringSize)
            {
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute get logical disk cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetLogicalDiskStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    BOOL					volStatus;
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8 					diskIndex;
    DISK_VOLUME_INFO_t 		diskVolumeInfo;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];
    UINT8					totalVolume, driveType;
    HDD_CONFIG_t		    hddConfig;

    do
    {
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;

        ReadHddConfig(&hddConfig);
        totalVolume = GetTotalDiskNumber(hddConfig.mode);

        for(diskIndex = 0; diskIndex < (totalVolume + MAX_NW_DRIVE); diskIndex++)
        {
            if (diskIndex < totalVolume)
            {
                /* 0: for single local disk or raid mode */
                driveType = 0;
                volStatus = GetDiskVolumeInfo(diskIndex, &diskVolumeInfo);
            }
            else
            {
                /* 1: for network disk */
                driveType = 1;
                volStatus = GetNddVolumeInfo(diskIndex - totalVolume, &diskVolumeInfo);
            }

            if (volStatus == FAIL)
            {
                cmdResp = CMD_PROCESS_ERROR;
                break;
            }

            if (volStatus != SUCCESS)
            {
                continue;
            }

            sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%s%c%s%c%s%c%d%c%s%c%d%c%c", SOI, (diskIndex + 1), FSP,
                                   diskVolumeInfo.volumeName, FSP, diskVolumeInfo.totalSize, FSP, diskVolumeInfo.freeSize, FSP,
                                   diskVolumeInfo.status, FSP, diskVolumeInfo.percentageFormat, FSP, driveType, FSP, EOI);
            if(sizeWritten >= resStringSize)
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

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute harddisk format cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL HarddiskFormatCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_HDDFORMAT_ARG];
    UINT8					diskIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_HDDFORMAT_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
        }
        else
        {
            diskIndex = (UINT8)(tempData[HDDFORMAT_VOL_NUM] - 1);
            cmdResp = FormatMedia(diskIndex, userAccountConfig.username);
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API stops RAID building process.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopRaidCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_STOP_RAID_ARG];
    UINT8					raidVolNo;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_STOP_RAID_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
        }
        else
        {
            raidVolNo = (UINT8)(tempData[STOP_RAID_VOL_NO] - 1);
            cmdResp = StopRaid(raidVolNo);
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute start manual trigger cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StartManualTriggerCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        cmdResp = ManualTrigger(ON);
        if(cmdResp == CMD_SUCCESS)
        {
            WriteEvent(LOG_USER_EVENT, LOG_MANUAL_TRIGGER, userAccountConfig.username, NULL, EVENT_START);
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute stop manual trigger cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopManualTriggerCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        cmdResp = ManualTrigger(OFF);
        if(cmdResp == CMD_SUCCESS)
        {
            WriteEvent(LOG_USER_EVENT, LOG_MANUAL_TRIGGER, userAccountConfig.username, NULL, EVENT_STOP);
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute get date time cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetDateTimeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_MAX_BUFFER_LIMIT;
    struct tm 				getSetTime;
    UINT32 					sizeWritten;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];

    /* Sometimes it gets failed, Hence added retry */
    if ((GetLocalTimeInBrokenTm(&getSetTime) == FAIL) && (GetLocalTimeInBrokenTm(&getSetTime) == FAIL))
    {
        cmdResp = CMD_PROCESS_ERROR;
    }
    else
    {
        sizeWritten = snprintf(resStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c""%c%d%c%02d%02d%04d%02d%02d%02d%c%c%c",
                               SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, getSetTime.tm_mday, (getSetTime.tm_mon + 1),
                               getSetTime.tm_year, getSetTime.tm_hour, getSetTime.tm_min, getSetTime.tm_sec, FSP, EOI, EOM);
        if(sizeWritten < MAX_REPLY_SZ)
        {
            sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], sizeWritten, MESSAGE_REPLY_TIMEOUT);
            closeConnCb[clientCbType](&clientSocket);
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute restore configuration cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL RestoreConfigCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    CHAR 					fileName[MAX_RESTORE_FILE_NAME_LEN];
    UINT64					fieldVal;
    NET_CMD_STATUS_e		requestStatus = CMD_SUCCESS;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup != ADMIN)
    {
        requestStatus = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, &fieldVal, 1, FSP)== FAIL)
    {
        requestStatus = CMD_INVALID_SYNTAX;
    }
    else if(fieldVal != 1)
    {
        requestStatus = CMD_INVALID_FIELD_VALUE;
    }
    else if(ParseStr(pCmdStr, FSP, fileName, MAX_RESTORE_FILE_NAME_LEN) == FAIL)
    {
        requestStatus = CMD_INVALID_SYNTAX;
    }
    else
    {
        requestStatus = StartRestoreConfig(fileName);
    }

    clientCmdRespCb[clientCbType](requestStatus, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set date time cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetDateTimeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SET_DATE_TIME_ARG];
    struct tm 				getSetTime;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    DPRINT(NETWORK_MANAGER, "set date-time cmd: [session=%d], [ip=%s], [user=%s]",
           sessionIndex, GetUserIpAddr(sessionIndex), userAccountConfig.username);
    if(userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, tempData, MAX_SET_DATE_TIME_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SET_DATE_TIME_CURRENT_TIME], getSetTime);
        if ((getSetTime.tm_year < 2000) || (SetNewDateTime(&getSetTime, userAccountConfig.username) == FAIL))
        {
            cmdResp = CMD_PROCESS_ERROR;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute update firmware cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL UpdateFirmwareCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    CHAR 					fileName[MAX_FW_FILE_NAME_LEN];
    UINT64					fieldVal;
    NET_CMD_STATUS_e		requestStatus = CMD_SUCCESS;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup != ADMIN)
    {
        requestStatus = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, &fieldVal, 1, FSP)== FAIL)
    {
        requestStatus = CMD_INVALID_SYNTAX;
    }
    else if(fieldVal != 0)
    {
        requestStatus = CMD_INVALID_FIELD_VALUE;
    }
    else if(ParseStr(pCmdStr, FSP, fileName, MAX_FW_FILE_NAME_LEN) == FAIL)
    {
        requestStatus = CMD_INVALID_SYNTAX;
    }
    else
    {
        requestStatus = StartSysUpgrade(fileName);
    }

    clientCmdRespCb[clientCbType](requestStatus, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute system logout cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SystemLogoutCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UserLogout(sessionIndex);
    clientCmdRespCb[clientCbType](CMD_SUCCESS, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute system shutdown cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SystemShutdownCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8					userIndex;
    LOG_EVENT_STATE_e		eventState;
    CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR					passWord[MAX_USER_ACCOUNT_PASSWORD_WIDTH];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        if(ParseStr(pCmdStr, FSP, userName, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if(ParseStr(pCmdStr, FSP, passWord, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        for(userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if(strcmp(userAccountConfig.username, userName) == STATUS_OK)
            {
                break;
            }
        }

        if(userIndex >= MAX_USER_ACCOUNT)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if(strcmp(userAccountConfig.password, passWord) != STATUS_OK)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        eventState = (sessionIndex == USER_LOCAL) ? EVENT_LOCAL : EVENT_REMOTE;
        if (PrepForPowerAction(SHUTDOWN_DEVICE, eventState, userName) == FAIL)
        {
            cmdResp = CMD_PROCESS_ERROR;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute system restart cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SystemRestartCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8					userIndex;
    LOG_EVENT_STATE_e		eventState;
    CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR					passWord[MAX_USER_ACCOUNT_PASSWORD_WIDTH];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        if(ParseStr(pCmdStr, FSP, userName, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if(ParseStr(pCmdStr, FSP, passWord, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        for(userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if(strcmp(userAccountConfig.username, userName) == STATUS_OK)
            {
                break;
            }
        }

        if(userIndex >= MAX_USER_ACCOUNT)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if(strcmp(userAccountConfig.password, passWord) != STATUS_OK)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        eventState = (sessionIndex == USER_LOCAL) ? EVENT_LOCAL : EVENT_REMOTE;
        if (PrepForPowerAction(REBOOT_DEVICE, eventState, userName) == FAIL)
        {
            cmdResp = CMD_PROCESS_ERROR;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute factory default cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL FactoryDefaultCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_FAC_DEF_CFG_ARG];
    UINT8					configId;
    UINT8					userIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR					passWord[MAX_USER_ACCOUNT_PASSWORD_WIDTH];
    CHAR					detail[MAX_EVENT_DETAIL_SIZE];
    NET_CMD_STATUS_e 		factDfltResponse[MAX_FAC_DEF_CFG_ARG];
    UINT32					factDfltCnt = 0;
    LOG_EVENT_STATE_e       eventState;

    do
    {
        if (ParseStr(pCmdStr, FSP, userName, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if (ParseStr(pCmdStr, FSP, passWord, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if (ParseStringGetVal(pCmdStr, tempData, MAX_FAC_DEF_CFG_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        for (userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if (strcmp(userAccountConfig.username, userName) == STATUS_OK)
            {
                break;
            }
        }

        if (userIndex >= MAX_USER_ACCOUNT)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if (strcmp(userAccountConfig.password, passWord) != STATUS_OK)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if (userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        for (configId = 0; configId < MAX_CONFIG_ID; configId++)
        {
            if ((configId == LAN1_CONFIG_ID) || (configId == LAN2_CONFIG_ID))
            {
                if (((BOOL)tempData[FAC_DEF_CFG_NETWORK]) == DISABLE)
                {
                    continue;
                }

                factDfltCnt = FAC_DEF_CFG_NETWORK;
            }
            else if (configId == USER_ACCOUNT_CONFIG_ID)
            {
                if (((BOOL)tempData[FAC_DEF_CFG_USERACC]) == DISABLE)
                {
                    continue;
                }

                factDfltCnt = FAC_DEF_CFG_USERACC;

                // Delete all users, rquire only Default user admin, local, viewer and operator
                for (userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
                {
                    ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
                    if (userAccountConfig.userStatus == ENABLE)
                    {
                        /* Delete devices which are associated with this username */
                        FcmPushDeleteSystemUserNotify(userAccountConfig.username);

                        /* Remove admin group user from linux system user list */
                        if (userAccountConfig.userGroup == ADMIN)
                        {
                            DeleteLinuxUser(userAccountConfig.username, GetUserAccountIndex(sessionIndex));
                        }
                    }

                    /* Delete client's layout file */
                    DeleteClientDispLayoutFile(userIndex);

                    /* Default the user's password recovery config */
                    DfltSinglePasswordRecoveryConfig(userIndex);
                }

                // Default Password Reset Info
                RemovePassExpiryFile();

                // reset viewer user sesion info
                ResetAllDefaultUser(TRUE);
            }
            else
            {
                if (((BOOL)tempData[FAC_DEF_CFG_OTHER]) == DISABLE)
                {
                    continue;
                }

                factDfltCnt = FAC_DEF_CFG_OTHER;
            }

            if (RemoveConfigFile(configId) == FAIL)
            {
                cmdResp = CMD_PROCESS_ERROR;
                factDfltResponse[factDfltCnt] = CMD_PROCESS_ERROR;
                break;
            }

            factDfltResponse[factDfltCnt] = CMD_SUCCESS;
        }

        for (configId = 0; configId < MAX_LOCAL_CONFIG_ID; configId++)
        {
            if (((BOOL)tempData[FAC_DEF_CFG_OTHER]) == DISABLE)
            {
                break;
            }

            if (RemoveLocalConfigFile(configId) == FAIL)
            {
                EPRINT(NETWORK_MANAGER, "fail to remove local config file");
                break;
            }
        }

        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        for (configId = 0; configId <= factDfltCnt; configId++)
        {
            if((BOOL)tempData[configId] == DISABLE)
            {
                continue;
            }

            eventState = (factDfltResponse[factDfltCnt] == CMD_SUCCESS) ? EVENT_DEFAULT_COMPLETE : EVENT_DEFAULT_FAIL;
            snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", (configId + 1));
            WriteEvent(LOG_USER_EVENT, LOG_SYS_CONFIGURATION, detail, userName, EVENT_DEFAULT);
            WriteEvent(LOG_USER_EVENT, LOG_SYS_CONFIGURATION, detail, userName, eventState);
        }

        PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, userName);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute update ddns cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL UpdateDdnsCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        cmdResp = UpdateDdnsRegIp(clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute test mail cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL TestMailCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_INVALID_SYNTAX;
    UINT64 					tempData;
    CHAR 					recvEmailId[MAX_SMTP_EMAILID_WIDTH];
    SMTP_CONFIG_t			smtpRecvData;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup == VIEWER)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (ParseStr(pCmdStr, FSP, recvEmailId, MAX_SMTP_EMAILID_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smtpRecvData.server, MAX_SMTP_SERVER_NAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smtpRecvData.serverPort = (UINT16)tempData;
        if (ParseStr(pCmdStr, FSP, smtpRecvData.username, MAX_SMTP_USERNAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smtpRecvData.password, MAX_SMTP_PASSWORD_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smtpRecvData.senderAddress, MAX_SMTP_EMAILID_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smtpRecvData.encryptionType = (SMTP_ENCRYPTION_TYPE_e)tempData;
        cmdResp = SendTestEmail(recvEmailId, &smtpRecvData, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute testm ftp connecton cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL TestFtpConnCmd(CHARPTR *pCmdStr,CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_INVALID_SYNTAX;
    UINT64 					tempData;
    FTP_UPLOAD_CONFIG_t		ftpRecvData;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup == VIEWER)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (ParseStr(pCmdStr, FSP, ftpRecvData.server, MAX_FTP_SERVER_NAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        ftpRecvData.serverPort = (UINT16)tempData;
        if (ParseStr(pCmdStr, FSP, ftpRecvData.username, MAX_FTP_USERNAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, ftpRecvData.password, MAX_FTP_PASSWORD_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, ftpRecvData.uploadPath, MAX_FTP_UPLOAD_PATH_WIDTH) == FAIL)
        {
            break;
        }

        cmdResp = TestFtpConn(&ftpRecvData, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute test tcp connection cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL TestTcpConnCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_INVALID_SYNTAX;
    UINT64 					tempData;
    TCP_NOTIFY_CONFIG_t		tcpData;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup == VIEWER)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (ParseStr(pCmdStr, FSP, tcpData.server, MAX_TCP_SERVER_NAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        tcpData.serverPort = (UINT16)tempData;
        cmdResp = SendTestTcpNotify(&tcpData, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute test sms cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL TestSmsConnCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_INVALID_SYNTAX;
    UINT64 					tempData;
    CHAR 					recvNumber[MAX_MOBILE_NUMBER_WIDTH];
    SMS_CONFIG_t			smsData;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup == VIEWER)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (ParseStr(pCmdStr, FSP, recvNumber, MAX_MOBILE_NUMBER_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smsData.mode = (SMS_MODE_e)tempData;
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smsData.smsServer = (SMS_SERVER_e)tempData;
        if (ParseStr(pCmdStr, FSP, smsData.userName, MAX_SMS_ACC_USERNAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smsData.password, MAX_SMS_ACC_PASSWORD_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smsData.sendorId, MAX_SMS_ACC_SENDOR_ID_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smsData.sendAsFlash = (BOOL)tempData;
        cmdResp = TestSmsNotification(&smsData, recvNumber, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute get usb disk cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetUsbStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT8               diskIndex;
    USB_DISK_INFO_t     usbDiskInfo;
    UINT32              sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR             resStringPtr = &replyMsg[clientCbType][0];

    do
    {
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        for(diskIndex = 0; diskIndex < DM_MAX_BACKUP; diskIndex++)
        {
            if(GetBackupDiskPara(diskIndex, &usbDiskInfo) == FAIL)
            {
                cmdResp = CMD_PROCESS_ERROR;
                break;
            }

            sizeWritten = snprintf(resStringPtr, resStringSize, "%c%d%c%s%c%s%c%s%c%d%c%s%c%d%c%c",
                                   SOI, (diskIndex + 1), FSP, usbDiskInfo.diskType, FSP, usbDiskInfo.totalSize, FSP,  usbDiskInfo.freeSize,
                                   FSP, usbDiskInfo.status, FSP, usbDiskInfo.percentageFormat, FSP, usbDiskInfo.backupStatus, FSP, EOI);
            if (sizeWritten >= resStringSize)
            {
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c", EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute usb disk format.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL UsbFormatCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_USBFORMAT_ARG];
    DM_BACKUP_TYPE_e		backupDevice;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, tempData, MAX_USBFORMAT_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        backupDevice = (DM_BACKUP_TYPE_e)(tempData[USBFORMAT_PORT]);
        cmdResp = FormatBackupMedia(backupDevice);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute usb disk unplug cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL UsbUnplugCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_USBUNPLUG_ARG];
    DM_BACKUP_TYPE_e		backupDevice;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, tempData, MAX_USBUNPLUG_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        backupDevice = (DM_BACKUP_TYPE_e)(tempData[USBUNPLUG_PORT]);
        cmdResp = RemoveBackupMedia(backupDevice, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute stop backup cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopBackupCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_BACKUP_TYPE_ARG];
    DM_BACKUP_TYPE_e		backupDevice;

    if(ParseStringGetVal(pCmdStr, tempData, MAX_BACKUP_TYPE_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        backupDevice = (DM_BACKUP_TYPE_e)(tempData[BACKUP_TYPE]);
        SetBackupStopReq(backupDevice, TRUE); // store the user req
        if(backupDevice == DM_MANUAL_BACKUP)
        {
            StopManualBackup();
        }
        else if(backupDevice == DM_SCHEDULE_BACKUP)
        {
            StopScheduleBackup();
        }
        else
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute set alarm output cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetAlarmOutputCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_ALARM_OUT_ARG];
    UINT8 					alarmIndex;
    BOOL 					alarmAction;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, tempData, MAX_ALARM_OUT_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        alarmIndex = (UINT8)(tempData[ALARM_OUT_ALARM_NUM] - 1);
        alarmAction = (BOOL)(tempData[ALARM_OUT_ALARM_STS]);
        cmdResp = SetAlarmOut(alarmIndex, alarmAction);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute get mac address cmd.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetMacAddressCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT64              tempData[MAX_GET_MAC_ARG];
    LAN_CONFIG_ID_e     lanPort;
    UINT32              sizeWritten;
    CHAR                macAddrsStr[MAX_MAC_ADDRESS_WIDTH];

    if(ParseStringGetVal(pCmdStr, tempData, MAX_GET_MAC_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        lanPort = (UINT8)(tempData[GET_MAC_LAN_NUM] - 1);
        GetMacAddr(lanPort, macAddrsStr);
        sizeWritten = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%s%c%c%c",
                SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, macAddrsStr, FSP, EOI, EOM);
        if(sizeWritten >= MAX_REPLY_SZ)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
        }
        else
        {
            sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], sizeWritten, MESSAGE_REPLY_TIMEOUT);
            closeConnCb[clientCbType](&clientSocket);
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the recording rights for user.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL RecordRightsStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    BOOL 					recordRight = ENABLE;
    UINT32                  sizeWritten;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        recordRight = DISABLE;
    }

    sizeWritten = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d%c%c%c",
            SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1,  FSP, recordRight, FSP, EOI, EOM);
    if (sizeWritten >= MAX_REPLY_SZ)
    {
        cmdResp = CMD_MAX_BUFFER_LIMIT;
    }
    else
    {
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], sizeWritten, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the backup rights for user.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL BackupRightsStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    BOOL 					backupRights = ENABLE;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        backupRights = DISABLE;
    }

    sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c%c%d%c%d%c%c%c",
                           SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1,  FSP, backupRights, FSP, EOI, EOM);
    if (sizeWritten >= resStringSize)
    {
        cmdResp = CMD_MAX_BUFFER_LIMIT;
    }
    else
    {
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], sizeWritten, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API fills helth status.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetHelthStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    BOOL 					status;
    HEALTH_STATUS_REPLY_e  	helthStatus;
    UINT8					majorIdx;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];

    do
    {
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP);
        if (sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        for(helthStatus = 0; helthStatus < HS_MAX_STATUS; helthStatus++)
        {
            for(majorIdx = 0; majorIdx < (healthStatusReply + helthStatus)->majorIndex; majorIdx++)
            {
                status = 0;
                if ((healthStatusReply + helthStatus)->funcPtr != NULL)
                {
                    status = (healthStatusReply + helthStatus)->funcPtr(majorIdx, (healthStatusReply + helthStatus)->minorIndex);
                }

                sizeWritten = snprintf(resStringPtr, resStringSize, "%d", status);
                if(sizeWritten >= resStringSize)
                {
                    cmdResp = CMD_MAX_BUFFER_LIMIT;
                    break;
                }

                resStringPtr += sizeWritten;
                resStringSize -= sizeWritten;
            }

            if(cmdResp != CMD_SUCCESS)
            {
                break;
            }

            sizeWritten = snprintf(resStringPtr, resStringSize, "%c", FSP);
            if(sizeWritten >= resStringSize)
            {
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c%c", EOI, EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to mute the buzzer
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL MuteBuzzerCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        MuteBuzzer();
    }
    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute snapshot command.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSnapshotCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8 					camIndex;
    UINT64 					tempData;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == VIEWER)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if(ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        camIndex = (UINT8)GET_CAMERA_INDEX(tempData);
        cmdResp = UploadSnapshot(camIndex, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute backup record command.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL BackUpRecordCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_INVALID_SYNTAX;
    UINT8        			cameraIndex;
    UINT64 					tempData[MAX_BKUP_RCD_ARG];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    SEARCH_RESULT_t			records[MAX_MANUAL_BACK_RCD];
    RECORD_ON_DISK_e		recStorageDrive = MAX_RECORDING_MODE;
    UINT8					count = 0;
    MB_LOCATION_e           backupLocation=MAX_MB_LOCATION;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    memset(tempData, 0, MAX_BKUP_RCD_ARG * sizeof(UINT64));

    do
    {
        // Parse until faces error
        if (ParseStringGetVal(pCmdStr, tempData, BKUP_RCD_FROM_STORAGE_TYPE, FSP) == FAIL)
        {
            // Assume there isn't any more record
            break;
        }

        records[count].channelNo = (UINT8)(tempData[BKUP_RCD_CAMERA]);
        cameraIndex = (UINT8)GET_CAMERA_INDEX(records[count].channelNo);
        if(userAccountConfig.userPrivilege[cameraIndex]. privilegeBitField.playback == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        records[count].overlapFlg = (UINT8)(tempData[BKUP_RCD_OVERLAP]);
        records[count].diskId = (UINT8)(tempData[BKUP_RCD_DISK_ID]);

        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[BKUP_RCD_START_TIME], records[count].startTime);
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[BKUP_RCD_STOP_TIME], records[count].endTime);

        //change for mobile client
        tempData[0] = 0;
        if(ParseStringGetVal(pCmdStr, tempData, 1, FSP) == FAIL)
        {
            recStorageDrive = MAX_RECORDING_MODE;
            records[count].recStorageType = recStorageDrive;
        }
        else
        {
            recStorageDrive = (RECORD_ON_DISK_e)tempData[0];
            records[count].recStorageType = recStorageDrive;
        }

        tempData[BKUP_RCD_DESTINATION] = 0;
        if(ParseStringGetVal(pCmdStr, &tempData[BKUP_RCD_DESTINATION], 1, FSP) == FAIL)
        {
            backupLocation = MAX_MB_LOCATION;
        }
        else
        {
            backupLocation = (MB_LOCATION_e)(tempData[BKUP_RCD_DESTINATION]);
        }

        cmdResp = CMD_SUCCESS;
        count++;

    } while (count < MAX_MANUAL_BACK_RCD);

    // check if any valid record found
    if (cmdResp == CMD_SUCCESS)
    {
        cmdResp = ManualBackUpOfRecords(records, recStorageDrive, count, (BACKUP_FORMAT_e)tempData[BKUP_RCD_FORMAT],
                                        NwCallbackFuncForBkupRcd, clientSocket, userAccountConfig.username, backupLocation, clientCbType);
    }

    if (cmdResp == CMD_SUCCESS)
    {
        DPRINT(NETWORK_MANAGER, "backup started: [time=%02d:02%d:02%d]", records[0].startTime.tm_hour, records[0].startTime.tm_min, records[0].startTime.tm_sec);
    }
    else
    {
        clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
        EPRINT(NETWORK_MANAGER, "backup record not found: [sessionIndex=%d] [cmdResp=%d]", sessionIndex, cmdResp);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gets Modem status
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetModemStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT32 					sizeWritten;
    LAN_CONFIG_t            networkInfo = {0};
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];

    do
    {
        GetNetworkParamInfo(NETWORK_PORT_USB_MODEM, &networkInfo);
        sizeWritten = snprintf(resStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%02d%c%s%c%s%c%s%c%s%c%s%c%s%c%d%c%s%c%s%c%s%c%c%c",
                               SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, GetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM), FSP,
                               networkInfo.ipv4.lan.ipAddress, FSP, networkInfo.ipv4.lan.subnetMask, FSP, networkInfo.ipv4.lan.gateway, FSP,
                               networkInfo.ipv4.dns.primaryAddress, FSP, networkInfo.ipv4.dns.secondaryAddress, FSP,
                               networkInfo.ipv6.lan.ipAddress, FSP, networkInfo.ipv6.lan.prefixLen, FSP, networkInfo.ipv6.lan.gateway, FSP,
                               networkInfo.ipv6.dns.primaryAddress, FSP, networkInfo.ipv6.dns.secondaryAddress, FSP, EOI, EOM);
        if(sizeWritten >= MAX_REPLY_SZ)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], sizeWritten, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives SMS balance of configured HTTP SMS Service.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSmsBalanceCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_INVALID_SYNTAX;
    UINT64              tempData;
    SMS_CONFIG_t        smsData;

    do
    {
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smsData.mode = (SMS_MODE_e)tempData;
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smsData.smsServer = (SMS_SERVER_e)tempData;
        if (ParseStr(pCmdStr, FSP, smsData.userName, MAX_SMS_ACC_USERNAME_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smsData.password, MAX_SMS_ACC_PASSWORD_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStr(pCmdStr, FSP, smsData.sendorId, MAX_SMS_ACC_SENDOR_ID_WIDTH) == FAIL)
        {
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            break;
        }

        smsData.sendAsFlash = (BOOL)tempData;
        cmdResp = CheckHttpSMSBalance(&smsData, clientCbType, clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API changes password of requested user.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ChangeUserPasswordCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    LOGIN_POLICY_CONFIG_t	loginPolicyCfg;
    UINT8					minCharReq = 0;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    UINT32                  outLen;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(ParseStr(pCmdStr, FSP, userAccountConfig.password, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        ReadLoginPolicyConfig(&loginPolicyCfg);
        cmdResp = CheckPasswordPolicy(&loginPolicyCfg, &userAccountConfig, &minCharReq);
        if(cmdResp == CMD_SUCCESS)
        {
            WriteSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
            CreateNewPasswordExpiryInfo(GetUserAccountIndex(sessionIndex), &userAccountConfig, FALSE);
        }
    }

    //Validate Buffer size
    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    // minimum characters required, validate buffer size for add last two character including NULL
    outLen += snprintf((respStringPtr + outLen), respStringSize - outLen, "%c%d%c%d%c%c", SOI, 1, FSP, minCharReq, FSP, EOI);
    if(outLen > respStringSize - 2)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize - 2;
    }

    // Append EOM
    outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API changes password of requested user.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ChangeUserSessionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e            cmdResp = CMD_INVALID_CREDENTIAL;
    UINT8                       userIndex, camIndex, index;
    CHAR                        userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR                        passWord[MAX_USER_ACCOUNT_PASSWORD_WIDTH];
    USER_ACCOUNT_CONFIG_t       userAccountConfig;
    USER_ACCOUNT_CONFIG_t       currUserAccountConfig;
    LOG_EVENT_SUBTYPE_e         evSubType = LOG_LOGIN_REQUEST;
    LOG_EVENT_STATE_e           eventState = EVENT_FAIL;
    CHAR                        eventDetail[MAX_EVENT_DETAIL_SIZE] = "0.0.0.0";
    UINT8                       viewerUsrCfgIndex = 0;
    USER_SESSION_FILE_INFO_t    viewerUserSessInfo;
    UINT32                      tempSysTick;
    VIDEO_TYPE_e                streamType;
    CHARPTR                     respStringPtr = &replyMsg[clientCbType][0];
    UINT32                      outLen;
    const UINT32                respStringSize = MAX_REPLY_SZ;
    UINT8                       indexId = 0;
    BOOL                        passResetInOneDay = FALSE;
    UINT32                      passLockDuration = 0;
    UINT8                       remainingLoginAttempts = 0;

    do
    {
        userAccountConfig.userGroup = VIEWER;
        if(ParseStr(pCmdStr, FSP, userName, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
        {
            break;
        }

        if(ParseStr(pCmdStr, FSP, passWord, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
        {
            break;
        }

        for(userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            if(SUCCESS != ReadSingleUserAccountConfig(userIndex, &userAccountConfig))
            {
                // if not success then continue
                continue;
            }

            if(strcmp(userAccountConfig.username, userName) == STATUS_OK)
            {
                break;
            }
        }

        if(userIndex >= MAX_USER_ACCOUNT)
        {
            break;
        }

        if((GetUserAccountIndex(sessionIndex) == userIndex) && (GetUserAccountIndex(sessionIndex) == USER_LOCAL))
        {
            EPRINT(NETWORK_MANAGER, "same user attempting for relogin");
            break;
        }

        if(CheckAutoLockTmrForUser(INVALID_ATTEMPT_TYPE_LOGIN, userIndex, &passLockDuration) == TRUE)
        {
            EPRINT(NETWORK_MANAGER, "auto timer started so user block");
            cmdResp = CMD_USER_ACCOUNT_LOCK;
            break;
        }

        if(strcmp(userAccountConfig.password, passWord) == STATUS_OK)
        {
            if(userAccountConfig.userStatus == DISABLE)
            {
                cmdResp = CMD_USER_DISABLED;
                break;
            }

            if(GetUserBlockStatus(userIndex) == TRUE)
            {
                cmdResp = CMD_USER_BLOCKED;
                break;
            }

            if(userAccountConfig.multiLogin == DISABLE)
            {
                for(index = 0; index < MAX_NW_CLIENT; index++)
                {
                    // Deny User if another user is Logged in with same User Name
                    if((GetUserAccountIndex(index) == userIndex) && (GetUserMultiLoginFlag(sessionIndex) != TRUE))
                    {
                        break;
                    }
                }

                if(index < MAX_NW_CLIENT)
                {
                    cmdResp = CMD_MULTILOGIN;
                    break;
                }
            }

            evSubType = LOG_USER_SESSION;
            if(strcmp(userAccountConfig.username, LOCAL_USER_NAME) == STATUS_OK)
            {
                // Read current user para
                ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &currUserAccountConfig);
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%s", currUserAccountConfig.username);
                eventState = EVENT_LOGOUT;
            }
            else
            {
                snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%s", userAccountConfig.username);
                eventState = EVENT_LOGIN;
            }

            SetUserAccountIndex(sessionIndex, userIndex);
            for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
            {
                if(userAccountConfig.userPrivilege[camIndex].privilegeBitField.monitor == DISABLE)
                {
                    for(streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
                    {
                        RemoveLiveMediaStream(camIndex, streamType, sessionIndex);
                    }
                }

                if(userAccountConfig.userPrivilege[camIndex].privilegeBitField.audio == DISABLE)
                {
                    for(streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
                    {
                        ChangeLiveMediaAudioState(camIndex, streamType, sessionIndex, DISABLE);
                    }
                }

                if(userAccountConfig.userPrivilege[camIndex].privilegeBitField.playback == DISABLE)
                {
                    RemovePlayBackForSessCamId(sessionIndex, camIndex);
                    RemoveInstantPlayBackForSessCamId(sessionIndex, camIndex);
                }
            }

            cmdResp = CMD_SUCCESS;
            if(eventState == EVENT_LOGIN)
            {
                if(CheckForPasswordResetReq(userIndex, &userAccountConfig) == TRUE)
                {
                    cmdResp = CMD_RESET_PASSWORD;
                    if(userAccountConfig.multiLogin == DISABLE)
                    {
                        SetUserMultiLoginFlag(sessionIndex,TRUE);
                    }
                    WPRINT(NETWORK_MANAGER, "user has not changed its password after creating account");
                    break;
                }

                // check for time has expired or not ...
                if(IsPasswordResetTimeExpire(userIndex, &userAccountConfig, &passResetInOneDay) == YES)
                {
                    cmdResp = CMD_PASSWORD_EXPIRE;
                    WPRINT(NETWORK_MANAGER, "password reset required as per password policy");
                    break;
                }

                // After successful login we should remove all invalid attempts.
                RemoveInvalidPassEntry(userIndex);
            }
            SetUserMultiLoginFlag(sessionIndex, FALSE);
        }
        else
        {
            if(UpdateInvalidPassEntry(INVALID_ATTEMPT_TYPE_LOGIN, userIndex, &passLockDuration, &remainingLoginAttempts) == TRUE)
            {
                cmdResp = CMD_USER_ACCOUNT_LOCK;
            }
        }

        if ((cmdResp == CMD_SUCCESS) || (cmdResp == CMD_RESET_PASSWORD) || (cmdResp == CMD_PASSWORD_EXPIRE))
        {
            if ((userAccountConfig.userGroup != VIEWER) || (strcmp(userAccountConfig.username, LOCAL_USER_NAME) == STATUS_OK))
            {
                SetViewerUserCfgIndex(sessionIndex, MAX_USER_ACCOUNT);
                break;
            }

            // check for user availabele in cfg
            viewerUsrCfgIndex = MAX_USER_ACCOUNT;
            if(IfUserAvailableInCfg(&viewerUsrCfgIndex, userAccountConfig.username, &viewerUserSessInfo) == TRUE)
            {
                DPRINT(NETWORK_MANAGER, "user available in cfg: [viewerUsrCfgIndex=%d], [totalSessionTime=%d], [totalElapsedSessionTime=%d]",
                       viewerUsrCfgIndex, viewerUserSessInfo.totalSessionTime, viewerUserSessInfo.totalElapsedSessionTime);

                // if user found then check remaining time
                if(viewerUserSessInfo.totalSessionTime <= viewerUserSessInfo.totalElapsedSessionTime)
                {
                    // give message session expire
                    cmdResp = CMD_LOGIN_SESSION_DURATION_OVER;
                    break;
                }

                cmdResp = CMD_SUCCESS;
                index = MAX_NW_CLIENT;
                SetViewerUserCfgIndex(sessionIndex, viewerUsrCfgIndex);
                if(userAccountConfig.multiLogin == ENABLE)
                {
                    for(index = USER_LOCAL; index < MAX_NW_CLIENT; index++)
                    {
                        // Checked user already logged in or not with same User Name
                        if(GetUserAccountIndex(index) == sessionIndex)
                        {
                            tempSysTick = GetViewerUserSysTk(index);
                            break;
                        }
                    }
                }

                if(index != MAX_NW_CLIENT)
                {
                    // session already on, copy sys tick
                    DPRINT(NETWORK_MANAGER, "same viewer user session running: [index=%d]", index);
                    SetViewerUserSysTk(sessionIndex, tempSysTick);
                    SetViewSessMultiLoginFlag(sessionIndex, TRUE);
                }
                else
                {
                    SetViewerUserSysTk(sessionIndex, GetSysTick());
                }
            }
            else
            {
                // else user not found then find free index in cfg
                if(FindFreeIndexInCfg(&viewerUsrCfgIndex) == TRUE)
                {
                    // fill total Acess time from user Account cfg
                    DPRINT(NETWORK_MANAGER, "viewer user not found in cfg, find free index [viewerUsrCfgIndex=%d]", viewerUsrCfgIndex);
                    snprintf(viewerUserSessInfo.username, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userAccountConfig.username);
                    viewerUserSessInfo.totalRemainingSessionTime = userAccountConfig.loginSessionDuration;
                    viewerUserSessInfo.totalSessionTime = userAccountConfig.loginSessionDuration;
                    viewerUserSessInfo.totalElapsedSessionTime = 0;
                    UpdateUserSessionData(viewerUsrCfgIndex, &viewerUserSessInfo);
                    SetViewerUserCfgIndex(sessionIndex, viewerUsrCfgIndex);
                    SetViewerUserSysTk(sessionIndex, GetSysTick());
                    cmdResp = CMD_SUCCESS;
                }
            }
        }

    }while(0);

    // send the cmdResp response
    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    //Validate Buffer size for add last two character including NULL
    outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%d%c%03d%c%d%c%d%c%c",
                       SOI, (indexId + 1), FSP, userAccountConfig.userGroup, FSP, passLockDuration, FSP, passResetInOneDay, FSP, remainingLoginAttempts, FSP, EOI);
    if(outLen > respStringSize - 2)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize - 2;
    }

    // Append EOM
    outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);

    if(cmdResp == CMD_SUCCESS)
    {
        WriteEvent(LOG_USER_EVENT, evSubType, eventDetail, LOCAL_CLIENT_EVT_STR, eventState);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported brand names.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetBrandNameCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    CHARPTR					cameraBrandStr[MAX_CAMERA_BRAND];
    UINT8					indexId;
    UINT8 					noOfBrand = 0;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    UINT32                  outLen;
    NET_CMD_STATUS_e		cmdResp;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        cmdResp = GetCameraBrand(cameraBrandStr, &noOfBrand);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        for(indexId = 0; indexId < noOfBrand; indexId++)
        {
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c",
                               SOI, (indexId + 1), FSP, cameraBrandStr[indexId], FSP, EOI);
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
                break;
            }
        }

        // Append EOM
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Model names for given brand.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetModelNameCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp;
    UINT8 					indexId;
    UINT8					noOfModels = 0;
    CHAR					brandNameStr[MAX_BRAND_NAME_LEN];
    CHARPTR					camModelStr[CAMERA_BRAND_MODEL_MAX];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    UINT32                  outLen;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        // Parse Brand Name
        if(ParseStr(pCmdStr, FSP, brandNameStr, MAX_BRAND_NAME_LEN) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetCameraModels(brandNameStr, camModelStr, &noOfModels);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        for(indexId = 0; indexId< noOfModels; indexId++)
        {
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c", SOI, (indexId+1), FSP, camModelStr[indexId], FSP, EOI);
            //Validate buffer size for add last two character
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
                break;
            }
        }

        // Append EOM
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Encoders for given camera.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSupportedProfileCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8 					indexId = 1;
    UINT8					noOfProfileSupported = 0;
    UINT64 					tempData[MAX_PROFILE_SUP];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT32					outLen;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, tempData, MAX_PROFILE_SUP, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetMaxSupportedProfile ((UINT8)GET_CAMERA_INDEX(tempData[PROFILE_SUP_CAM]), &noOfProfileSupported);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%d%c%c", SOI, indexId, FSP, noOfProfileSupported, FSP, EOI);
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Encoders for given camera.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetProfileParamCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp;
    UINT64 					tempData[MAX_PROFILE_PARAM];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8 					cameraIndex;
    CAMERA_TYPE_e 			cameraType;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, tempData, MAX_PROFILE_PARAM, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[PROFILE_PARAM_CAM]);
        cameraType = CameraType(cameraIndex);
        if ((CameraType(cameraIndex) != IP_CAMERA) && (CameraType(cameraIndex) != AUTO_ADDED_CAMERA))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if ((cameraType == IP_CAMERA) && (GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == FAIL))
        {
            cmdResp = CMD_CAM_DISCONNECTED;
            break;
        }

        cmdResp = GetProfileParam(cameraIndex, (UINT8)tempData[PROFILE_PARAM_PROF_INDEX], (VIDEO_TYPE_e)tempData[PROFILE_PARAM_STREAM],
                                  clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Encoders for given camera.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSupportedEncoderCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8 					indexId, noOfCodecs = 0;
    UINT64 					tempData[MAX_ENCDR_SUP];
    CHARPTR					camEncoderStr[MAX_VIDEO_CODEC];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT32					outLen;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, tempData, MAX_ENCDR_SUP, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetSupportedCodec((UINT8)GET_CAMERA_INDEX(tempData[ENCDR_SUP_CAM]),
                                    (VIDEO_TYPE_e)tempData[ENCDR_SUP_STREAM], (UINT8)tempData[ENCDR_SUP_PROFILE], camEncoderStr, &noOfCodecs);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        for(indexId = 0; indexId < noOfCodecs; indexId++)
        {
            // (1&MJPEG&)(2&MPEG4&)
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c",
                               SOI, (indexId + 1), FSP, camEncoderStr[indexId], FSP, EOI);
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
            }
        }

        // Append EOM
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Resolution for given camera and Encoder Name.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSupportedResolutionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8 					indexId, noOfReslns = 0;
    UINT64 					tempData[MAX_RES_SUP];
    CHAR					camEncoderStr[MAX_ENCODER_NAME_LEN];
    CHARPTR					camResoltn[MAX_RESOLUTION];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT32					outLen;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if ((ParseStringGetVal(pCmdStr, &tempData[RES_SUP_CAM], 1, FSP) == FAIL)
            || (ParseStr(pCmdStr, FSP, camEncoderStr, MAX_ENCODER_NAME_LEN) == FAIL)
            || (ParseStringGetVal(pCmdStr, &tempData[RES_SUP_STREAM], 1, FSP) == FAIL)
            || (ParseStringGetVal(pCmdStr, &tempData[RES_SUP_PROFILE], 1, FSP) == FAIL))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetSupportedResolution((UINT8)GET_CAMERA_INDEX(tempData[RES_SUP_CAM]), camEncoderStr,
                                         (VIDEO_TYPE_e)tempData[RES_SUP_STREAM], (UINT8)tempData[RES_SUP_PROFILE], camResoltn, &noOfReslns);
        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        for(indexId = 0; indexId < noOfReslns; indexId++)
        {
            //(1&160x120&)(2&176x144&)
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c",
                               SOI, (indexId + 1), FSP, camResoltn[indexId], FSP, EOI);
            //Validate buffer size for add last two character
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
                break;
            }
        }

        // Append EOM
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Frame Rate for given camera, Encoder, Resolution.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSupportedFrameRateCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64                  camFps;
    UINT64 					tempData[MAX_FR_SUP];
    CHAR					camEncoderStr[MAX_ENCODER_NAME_LEN];
    CHAR 					camResln[MAX_RESOLUTION_NAME_LEN];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    const UINT32            respStringSize = MAX_REPLY_SZ;
    UINT32					outLen;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[FR_SUP_CAM], 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        // Parse Encoder Name
        if ((ParseStr(pCmdStr, FSP, camEncoderStr, MAX_ENCODER_NAME_LEN) == FAIL)
            || (ParseStr(pCmdStr, FSP, camResln, MAX_RESOLUTION_NAME_LEN) == FAIL)
            || (ParseStringGetVal(pCmdStr, &tempData[FR_SUP_STREAM], 1, FSP) == FAIL)
            || ((ParseStringGetVal(pCmdStr, &tempData[FR_SUP_PROFILE], 1, FSP) == FAIL)))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetSupportedFramerate((UINT8)GET_CAMERA_INDEX(tempData[FR_SUP_CAM]), camEncoderStr, camResln,
                                        (VIDEO_TYPE_e)tempData[FR_SUP_STREAM], (UINT8)tempData[FR_SUP_PROFILE], &camFps);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0& (1&1073741823&)}
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c%c%d%c%llu%c%c%c",
                          SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, camFps, FSP, EOI, EOM);
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Video Quality for given camera and Video Codec.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSupportedBitRateCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_BITRATE_SUP];
    CHAR					camEncoderStr[MAX_ENCODER_NAME_LEN];
    CHARPTR					bitRateSupStr[MAX_BITRATE_VALUE];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					maxSupportedBitrate = 0;
    BITRATE_VALUE_e         minBitrateIndex, maxBitrateIndex;
    UINT32					outLen;
    UINT8 					indexId;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if ((ParseStringGetVal(pCmdStr, &tempData[BITRATE_SUP_CAM], 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, camEncoderStr, MAX_ENCODER_NAME_LEN) == FAIL)
                || (ParseStringGetVal(pCmdStr, &tempData[BITRATE_SUP_STREAM], 1, FSP) == FAIL)
                || (ParseStringGetVal(pCmdStr, &tempData[BITRATE_SUP_PROFILE], 1, FSP) == FAIL))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetSupportedBitRate((UINT8)GET_CAMERA_INDEX(tempData[BITRATE_SUP_CAM]), camEncoderStr, (VIDEO_TYPE_e)tempData[BITRATE_SUP_STREAM],
                                      (UINT8)tempData[BITRATE_SUP_PROFILE], bitRateSupStr, &maxSupportedBitrate, &minBitrateIndex, &maxBitrateIndex);
        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        for(indexId = 0; indexId < maxSupportedBitrate; indexId++)
        {
            //(1&160x120&)(2&176x144&)
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c",
                               SOI, (indexId + 1), FSP, bitRateSupStr[indexId], FSP, EOI);
            //Validate Buffer size for add last two character including NULL
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
            }
        }

        // Append EOM
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives supported Video Quality for given camera and Video Codec.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSupportedVideoQualityCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8					maxVideoQlty;
    UINT64 					tempData[MAX_QLT_SUP];
    CHAR					camEncoderStr[MAX_ENCODER_NAME_LEN];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if ((ParseStringGetVal(pCmdStr, &tempData[QLT_SUP_CAM], 1, FSP) == FAIL)
            || (ParseStr(pCmdStr, FSP, camEncoderStr, MAX_ENCODER_NAME_LEN) == FAIL)
            || (ParseStringGetVal(pCmdStr, &tempData[QLT_SUP_STREAM], 1, FSP) == FAIL)
            || (ParseStringGetVal(pCmdStr, &tempData[QLT_SUP_PROFILE], 1, FSP) == FAIL))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetSupportedQuality((UINT8)GET_CAMERA_INDEX(tempData[QLT_SUP_CAM]), camEncoderStr,
                                      (VIDEO_TYPE_e)tempData[QLT_SUP_STREAM], (UINT8)tempData[QLT_SUP_PROFILE], &maxVideoQlty);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0& (1&100&)}
        snprintf(respStringPtr, respStringSize, "%c%s%c%d%c%c%d%c%d%c%c%c",
                 SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, maxVideoQlty, FSP, EOI, EOM);
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API get the motion window params
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetMotionWindowCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp;
    UINT64 					tempData[MAX_GET_MOTION_WIN];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					camIndex;
    CAMERA_TYPE_e           cameraType;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[GET_MOTION_WIN_CAM], 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        camIndex = (UINT8)GET_CAMERA_INDEX(tempData[GET_MOTION_WIN_CAM]);
        cameraType = CameraType(camIndex);
        if ((cameraType != IP_CAMERA) && (cameraType != AUTO_ADDED_CAMERA))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if(GetCamEventStatus(camIndex, CONNECTION_FAILURE) == FAIL)
        {
            cmdResp = CMD_CAM_DISCONNECTED;
            break;
        }

        cmdResp = GetMotionWindowConfig(camIndex, clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API set the motion window params
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetMotionWindowCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    UINT64                      tempData[MAX_SET_MOTION_WIN_COMMON];
    USER_ACCOUNT_CONFIG_t       userAccountConfig;
    UINT8                       blockByte;
    UINT8                       cameraIndex;
    MOTION_BLOCK_METHOD_PARAM_t motionBlockParam;
    CHARPTR                     motionBlockData;
    CHAR                        rawAsciiBytes[3];
    UINT32                      rawHexVal;
    UINT64                      tmpData;
    CAMERA_TYPE_e               cameraType;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[SET_MOTION_WIN_CAM], MAX_SET_MOTION_WIN_COMMON, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SET_MOTION_WIN_CAM]);
        cameraType = CameraType(cameraIndex);
        if((cameraType != IP_CAMERA) && (cameraType != AUTO_ADDED_CAMERA))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if(GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == FAIL)
        {
            cmdResp = CMD_CAM_DISCONNECTED;
            break;
        }

        if (MOTION_AREA_METHOD_BLOCK != (MOTION_AREA_METHOD_e)tempData[SET_MOTION_WIN_METHOD])
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tmpData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }
        motionBlockParam.sensitivity = tmpData;

        if(ParseStringGetVal(pCmdStr, &tmpData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }
        motionBlockParam.noMotionSupportF = tmpData;

        if(ParseStringGetVal(pCmdStr, &tmpData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }
        motionBlockParam.isNoMotionEvent = GET_BOOL_VALUE(tmpData);

        if(ParseStringGetVal(pCmdStr, &tmpData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }
        motionBlockParam.noMotionDuration = tmpData;

        motionBlockData = *pCmdStr;
        for(blockByte = 0; blockByte < MOTION_AREA_BLOCK_BYTES; blockByte++)
        {
            memcpy(rawAsciiBytes, &motionBlockData[blockByte*2], 2);
            rawAsciiBytes[2] = '\0';
            sscanf(rawAsciiBytes, "%x", &rawHexVal);
            motionBlockParam.blockBitString[blockByte] = (UINT8)rawHexVal;
        }

        cmdResp = SetMotionWindowConfig(cameraIndex, &motionBlockParam, clientCmdRespCb[clientCbType], clientSocket, clientCbType, ENABLE);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API get the privacy mask window params
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetPrivacyMaskCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_GET_PRIVACY_MASK];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					cameraIndex;
    CAMERA_TYPE_e           cameraType;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[GET_PRIVACY_MASK_CAM], 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[GET_PRIVACY_MASK_CAM]);
        cameraType = CameraType(cameraIndex);
        if((cameraType != IP_CAMERA) && (cameraType != AUTO_ADDED_CAMERA))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if(GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == FAIL)
        {
            cmdResp = CMD_CAM_DISCONNECTED;
            break;
        }

        cmdResp = GetPrivacyMaskWindowConfig(cameraIndex, clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API set the privacy mask window params
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SetPrivacyMaskCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp;
    UINT64 					tempData[MAX_SET_PRIVACY_MASK_COMMON];
    UINT64					tempPointData[MAX_SET_PRIVACY_MASK_POINT];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT8					cameraIndex;
    UINT8					argCnt = 0;
    UINT8 					winCnt = 0;
    PRIVACY_MASK_CONFIG_t	privacyArea[MAX_PRIVACY_MASKS];
    UINT8					maxSupportedPrivacyMaskWindow;
    CAMERA_TYPE_e           cameraType;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData[SET_PRIVACY_MASK_CAM], MAX_SET_PRIVACY_MASK_COMMON, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[SET_PRIVACY_MASK_CAM]);
        cameraType = CameraType(cameraIndex);
        if((cameraType != IP_CAMERA) && (cameraType != AUTO_ADDED_CAMERA))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if(GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == FAIL)
        {
            cmdResp = CMD_CAM_DISCONNECTED;
            break;
        }

        maxSupportedPrivacyMaskWindow = (UINT8)(tempData[SET_PRIVACY_MASK_MAX_SUPPORTED_WIN]);
        if(ParseStringGetVal(pCmdStr, &tempPointData[0], (maxSupportedPrivacyMaskWindow * 4), FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        for(argCnt = 0; argCnt < (maxSupportedPrivacyMaskWindow * 4); argCnt++)
        {
            privacyArea[winCnt].startXPoint = (UINT32)tempPointData[argCnt++];
            privacyArea[winCnt].startYPoint = (UINT32)tempPointData[argCnt++];
            privacyArea[winCnt].width = (UINT32)tempPointData[argCnt++];
            privacyArea[winCnt].height = (UINT32)tempPointData[argCnt];
            winCnt++;
        }

        cmdResp = SetPrivacyMaskConfig(cameraIndex, privacyArea, clientCmdRespCb[clientCbType], clientSocket, ENABLE, clientCbType);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives information whether camera supports Motion Detection, Temper detection,
 *          Audio Support, PTZ Support, Number of Digital Input and  Output
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetOtherSupportedFeaturesCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 			cmdResp = CMD_SUCCESS;
    UINT8						camIndex;
    UINT64 						tempData;
    CHARPTR						respStringPtr = &replyMsg[clientCbType][0];
    CAMERA_CAPABILTY_INFO_t 	camCapability;
    const UINT32                respStringSize = MAX_REPLY_SZ;

    if(ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        // call respective API of Camera Database
        camIndex = (UINT8)GET_CAMERA_INDEX(tempData);
        cmdResp = GetSupportedCapability(camIndex, &camCapability);
        if(cmdResp == CMD_SUCCESS)
        {
            // (MOTION&VIEW_TEMPER&PTZ&AUDIO&SENSOR_INPUT&ALARM_OUTPUT&)
            // {RPL_CMD&0&(1&1&0&0&1&1&0&)}
            snprintf(respStringPtr, respStringSize, "%c%s%c%d%c%c%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c"
                                                    "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%d%c" "%c%c",
                    SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, camCapability.motionDetectionSupport, FSP,
                    camCapability.viewTamperSupport, FSP, camCapability.ptzSupport, FSP, camCapability.audioSupport, FSP,
                    camCapability.maxSensorInput, FSP, camCapability.maxAlarmOutput,FSP, camCapability.motionWinConfigSupport,FSP,
                    camCapability.privacymaskWinConfigSupport, FSP, camCapability.lineCrossingSupport, FSP,
                    camCapability.objectIntrusionSupport, FSP, camCapability.audioExceptionSupport, FSP,
                    camCapability.missingObjectSupport, FSP, camCapability.suspiciousObjectSupport, FSP,
                    camCapability.loiteringSupport, FSP, camCapability.objectCounting, FSP,
                    camCapability.noMotionDetectionSupport, FSP, EOI, EOM);

            sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
            closeConnCb[clientCbType](&clientSocket);
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API update hostname in matrix dns server
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL	UpdateMatrixDnsHostNameCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e			cmdResp = CMD_SUCCESS;
    MATRIX_DNS_SERVER_CONFIG_t	dnsCfg;
    UINT64						port;
    USER_ACCOUNT_CONFIG_t 		userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if (userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else if ((ParseStr(pCmdStr, FSP, dnsCfg.hostName, MAX_MATRIX_DNS_SERVER_HOST_NAME_LEN) == FAIL)
             || (ParseStringGetVal(pCmdStr, &port, 1, FSP) == FAIL))
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        dnsCfg.forwardedPort = port;
        cmdResp = UpdateMacClientHostName(dnsCfg, clientCmdRespCb[clientCbType], clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Compare Device id and return whether it is match or not
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL CheckDeviceIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 			cmdResp = CMD_SUCCESS;
    UINT8						deviceId;
    UINT64 						tempData;
    GENERAL_CONFIG_t			generalCfg;

    if(ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        deviceId = (UINT8)tempData;
        ReadGeneralConfig(&generalCfg);
        if(generalCfg.deviceId != deviceId)
        {
            cmdResp = CMD_PROCESS_ERROR;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API get the user privilege
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL CheckUserCameraPrivilegeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT64 						tempData;
    UINT8						camIndex;
    USER_ACCOUNT_CONFIG_t		userAccountConfig;
    STREAM_CONFIG_t				streamCfg;
    const UINT32                respStringSize = MAX_REPLY_SZ;

    if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == SUCCESS)
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        camIndex = (UINT8)GET_CAMERA_INDEX(tempData);
        ReadSingleStreamConfig(camIndex, &streamCfg);

        snprintf(&replyMsg[clientCbType][0], respStringSize, "%c%s%c%d%c%c%d%c%d%c%d%c%d%c%c%c", SOM,
                headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP,
                userAccountConfig.userPrivilege[camIndex].privilegeBitField.monitor, FSP,
                userAccountConfig.userPrivilege[camIndex].privilegeBitField.audio, FSP,
                streamCfg.enableAudio, FSP, EOI, EOM);

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
    }
    else
    {
        clientCmdRespCb[clientCbType](CMD_INVALID_SYNTAX, clientSocket, TRUE);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API get the advance status of the system
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetAdvanceStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_INTERFACE_LOAD_PARAM_t	netIfLoad;
    LAN_CONFIG_t				networkInfo = {0};
    CHARPTR						pRespStr = &replyMsg[clientCbType][0];
    UINT8						portType, majorIdx;
    UINT32						outLen;
    UINT64 						timeTakeToFullVolume = 0;
    UINT64	 					recordWriteRateInMbps = 0, recordRatePerHourInMbps = 0, recordRatePerDayInMbps = 0;
    const UINT32                respStrLen = MAX_REPLY_SZ;

    outLen = snprintf(pRespStr, respStrLen, "%c%s%c%d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP);
    if(outLen > respStrLen)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStrLen;
    }

    /* Add LAN Port parameters info */
    for (portType = NETWORK_PORT_LAN1; portType < NETWORK_PORT_MAX; portType++)
    {
        netIfLoad.txLoad = netIfLoad.rxLoad = 0;
        if (TRUE == IsNetworkPortLinkUp(portType))
        {
            GetNetworkInterfaceLoad(portType, &netIfLoad);
        }

        if (FAIL == GetNetworkParamInfo(portType, &networkInfo))
        {
            memset(&networkInfo, 0, sizeof(networkInfo));
        }

        /* Check if Ip address is under discovery */
        if ((networkInfo.ipv4.ipAssignMode != IPV4_ASSIGN_STATIC) && (networkInfo.ipv4.lan.ipAddress[0] == '\0'))
        {
            outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%d%c%s%c%d%c%d%c",
                               GetNetworkPortLinkStatus(portType), FSP, NETWORK_DISCOVERY_STR, FSP,
                               netIfLoad.txLoad, FSP, netIfLoad.rxLoad, FSP);
        }
        else
        {
            outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%d%c%s%c%d%c%d%c",
                               GetNetworkPortLinkStatus(portType), FSP, networkInfo.ipv4.lan.ipAddress, FSP,
                               netIfLoad.txLoad, FSP, netIfLoad.rxLoad, FSP);
        }

        if(outLen > respStrLen)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStrLen;
            break;
        }
    }

    /* Add Live Stream & Play Back Stream Count */
    outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%d%c%d%c", GetLiveStreamCnt(), FSP, GetPlayBackStreamCnt(), FSP);
    if(outLen > respStrLen)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStrLen;
    }

    /* Add CPU Usage Info & System uptime */
    outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%d%c%lld%c", GetCurrCpuUsage(), FSP, GetDeviceUpTime(), FSP);
    if(outLen > respStrLen)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStrLen;
    }

    /* Add Camera recording information */
    for(majorIdx = 0; majorIdx < (healthStatusReply + HS_CAM_CONNECTION)->majorIndex; majorIdx++)
    {
        if ((healthStatusReply + HS_CAM_CONNECTION)->funcPtr == NULL)
        {
            continue;
        }

        if (ACTIVE != (healthStatusReply + HS_CAM_CONNECTION)->funcPtr(majorIdx, (healthStatusReply + HS_CAM_CONNECTION)->minorIndex))
        {
            continue;
        }

        if (ACTIVE == (healthStatusReply + HS_SCH_RECORDING)->funcPtr(majorIdx, (healthStatusReply + HS_SCH_RECORDING)->minorIndex))
        {
            GetStorageCalculationInfo(&timeTakeToFullVolume, &recordWriteRateInMbps);
            break;
        }

        if (ACTIVE == (healthStatusReply + HS_ALM_RECORDING)->funcPtr(majorIdx, (healthStatusReply + HS_ALM_RECORDING)->minorIndex))
        {
            GetStorageCalculationInfo(&timeTakeToFullVolume, &recordWriteRateInMbps);
            break;
        }

        if (ACTIVE == (healthStatusReply + HS_MAN_RECORDING)->funcPtr(majorIdx, (healthStatusReply + HS_MAN_RECORDING)->minorIndex))
        {
            GetStorageCalculationInfo(&timeTakeToFullVolume, &recordWriteRateInMbps);
            break;
        }

        if (ACTIVE == (healthStatusReply + HS_COSEC_RECORDING)->funcPtr(majorIdx, (healthStatusReply + HS_COSEC_RECORDING)->minorIndex))
        {
            GetStorageCalculationInfo(&timeTakeToFullVolume, &recordWriteRateInMbps);
            break;
        }
    }

    if (recordWriteRateInMbps)
    {
        recordRatePerHourInMbps = (recordWriteRateInMbps * 3600) / (1024 * 1024);
        recordRatePerDayInMbps = recordRatePerHourInMbps * 24;
    }

    outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%d%c%lld%c%lld%c%lld%c", GetRecordingStreamCnt(), FSP, timeTakeToFullVolume,
                       FSP, recordRatePerHourInMbps, FSP, recordRatePerDayInMbps, FSP);
    if(outLen > respStrLen)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStrLen;
    }

    /* Add CPU temperature and internet connectivity status */
    outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%d%c%d%c", GetCpuTemperature(), FSP, getInternetConnStatus(), FSP);
    if(outLen > respStrLen)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStrLen;
    }

    /* Add IPv6 Address info */
    for (portType = NETWORK_PORT_LAN1; portType < NETWORK_PORT_MAX; portType++)
    {
        GetNetworkParamInfo(portType, &networkInfo);

        /* Check if Ip address is under discovery */
        if (networkInfo.ipAddrMode == IP_ADDR_MODE_IPV4)
        {
            outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%s%c", IPV6_DISABLED_STR, FSP);
        }
        else if ((networkInfo.ipv6.ipAssignMode != IPV6_ASSIGN_STATIC) && (networkInfo.ipv6.lan.ipAddress[0] == '\0'))
        {
            outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%s%c", NETWORK_DISCOVERY_STR, FSP);
        }
        else
        {
            outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%s%c", networkInfo.ipv6.lan.ipAddress, FSP);
        }

        if(outLen > respStrLen)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStrLen;
            break;
        }
    }

    outLen += snprintf(pRespStr + outLen, respStrLen - outLen, "%c%c", EOI, EOM);
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API get the advance status of cameras
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetAdvanceCameraStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8           fps[MAX_STREAM][MAX_CAMERA_CONFIG];
    UINT8           gop[MAX_STREAM][MAX_CAMERA_CONFIG];
    CHARPTR         respStringPtr = &replyMsg[clientCbType][0];
    UINT8           cameraIndex;
    UINT32          outLen;
    VIDEO_TYPE_e    streamType;
    UINT64          isSubStreamDataNeeded = FALSE;
    UINT64          isClientSupportMaxCameraConfig = FALSE;
    const UINT32    respStringSize = MAX_REPLY_SZ;

    memset(&fps, 0, sizeof(fps));
    memset(&gop, 0, sizeof(gop));

    /* Latest client will send this field. If client is old then parsing will get failed else we will get the value */
    /* Earlier, Substream support was not there. If parsing success then we have to provide the substream data also */
    if (ParseStringGetVal(pCmdStr, &isSubStreamDataNeeded, 1, FSP) == FAIL)
    {
        isSubStreamDataNeeded = FALSE;
    }
    /* Earlier, Only 64 cameras were supported, if client is new then we can provide the max 96 camera data */
    else if (ParseStringGetVal(pCmdStr, &isClientSupportMaxCameraConfig, 1, FSP) == FAIL)
    {
        isClientSupportMaxCameraConfig = FALSE;
    }

    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP);

    for (streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
    {
        if (streamType == SUB_STREAM)
        {
            if (FALSE == isSubStreamDataNeeded)
            {
                break;
            }

            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%d%c", (BOOL)isSubStreamDataNeeded, FSP);
        }

        for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            GetCameraFpsGop(cameraIndex, streamType, &fps[streamType][cameraIndex], &gop[streamType][cameraIndex]);
        }

        for (cameraIndex = 0; cameraIndex < CAMERA_CONFIG_MAX_V1; cameraIndex++)
        {
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%d%c", fps[streamType][cameraIndex], FSP);
        }

        for (cameraIndex = 0; cameraIndex < CAMERA_CONFIG_MAX_V1; cameraIndex++)
        {
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%d%c", gop[streamType][cameraIndex], FSP);
        }
    }

    if ((TRUE == isSubStreamDataNeeded) && (TRUE == isClientSupportMaxCameraConfig))
    {
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%d%c", (BOOL)isClientSupportMaxCameraConfig, FSP);

        for (streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
        {
            for (cameraIndex = CAMERA_CONFIG_MAX_V1; cameraIndex < MAX_CAMERA_CONFIG; cameraIndex++)
            {
                outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%d%c", fps[streamType][cameraIndex], FSP);
            }

            for (cameraIndex = CAMERA_CONFIG_MAX_V1; cameraIndex < MAX_CAMERA_CONFIG; cameraIndex++)
            {
                outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%d%c", gop[streamType][cameraIndex], FSP);
            }
        }
    }

    outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%c", EOI, EOM);
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API get the cosec video popup details
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetCosecVideoPopUpDetailCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    CHARPTR         respStringPtr = &replyMsg[clientCbType][0];
    time_t          dateTimeSec = cosecInfoVideoPopup.dateTimeSec;
    struct tm       brokenTime = { 0 };
    const UINT32    respStringSize = MAX_REPLY_SZ;

    if(SUCCESS != ConvertLocalTimeInBrokenTm(&dateTimeSec, &brokenTime))
    {
        EPRINT(NETWORK_MANAGER, "fail to convert local time in broken");
    }

    snprintf(respStringPtr, respStringSize, "%c%s%c%d%c%c%d%c%s%c%s%c%d%c%s%c%d%c%d-%s-%d%c%02d:%02d:%02d%c%c%c",
             SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, cosecInfoVideoPopup.userId, FSP, cosecInfoVideoPopup.userName,
             FSP, cosecInfoVideoPopup.eventCode, FSP, cosecInfoVideoPopup.doorName, FSP, cosecInfoVideoPopup.doorDid,
             FSP, brokenTime.tm_mday, GetMonthName(brokenTime.tm_mon), brokenTime.tm_year,
             FSP, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec, FSP, EOI, EOM);
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the auto search camera command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AutoSearchCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;
    UINT64					tempData = 0;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if ((userAccountConfig.userGroup != ADMIN) && (strcmp(userAccountConfig.username, LOCAL_USER_NAME)))
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if((IsCamSearchActiveForClient(sessionIndex) == ACTIVE) || (IsAdvCamSearchActiveForClient(sessionIndex) == ACTIVE))
        {
            cmdResp = CMD_REQUEST_IN_PROGRESS;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if(tempData == MX_CAM_SEARCH_AUTO_CONFIG_ON_BOOT)
        {
            SetAutoConfigReportFlag(AUTO_CONFIG_REPORT_START, sessionIndex);
        }
        else
        {
            SetAutoConfigReportFlag(MAX_AUTO_CONFIG_STATE, sessionIndex);
        }

        cmdResp = StartCameraSearch(sessionIndex);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the add cameras command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AddCamerasCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;
    CAMERA_CONFIG_t			tempCameraCfg;
    IP_CAMERA_CONFIG_t		tempIpCameraCfg;
    IP_CAMERA_CONFIG_t		ipCameraCfg[MAX_CAMERA];
    UINT8					loop;
    UINT64					httpPort;
    UINT64					onvifPort;
    UINT64					camIndex;
    UINT64					onvifSupportFlag;
    UINT8					freeCamIndex;
    UINT32					outLen;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if (userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        freeCamIndex = camIndex = getMaxCameraForCurrentVariant();
        memcpy(&tempCameraCfg, &DfltCameraCfg, sizeof(CAMERA_CONFIG_t));
        memcpy(&tempIpCameraCfg, &DfltIpCameraCfg, sizeof(IP_CAMERA_CONFIG_t));

        if ((ParseStringGetVal(pCmdStr, &camIndex, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, tempCameraCfg.name, MAX_CAMERA_NAME_WIDTH) == FAIL)
                || (ParseStr(pCmdStr, FSP, tempIpCameraCfg.cameraAddress, MAX_CAMERA_ADDRESS_WIDTH) == FAIL)
                || (ParseStringGetVal(pCmdStr, &httpPort, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, tempIpCameraCfg.brand, MAX_BRAND_NAME_LEN) == FAIL)
                || (ParseStr(pCmdStr, FSP, tempIpCameraCfg.model, MAX_MODEL_NAME_LEN) == FAIL)
                || (ParseStr(pCmdStr, FSP, tempIpCameraCfg.username, MAX_CAMERA_USERNAME_WIDTH) == FAIL)
                || (ParseStr(pCmdStr, FSP, tempIpCameraCfg.password, MAX_CAMERA_PASSWORD_WIDTH) == FAIL)
                || (ParseStringGetVal(pCmdStr, &onvifSupportFlag, 1, FSP) == FAIL)
                || (ParseStringGetVal(pCmdStr, &onvifPort, 1, FSP) == FAIL))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        ReadIpCameraConfig(ipCameraCfg);

        if(camIndex == 0)
        {
            for(loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
            {
                if (ipCameraCfg[loop].cameraAddress[0] != '\0')
                {
                    continue;
                }

                /* Find free Camera Index */
                freeCamIndex = loop;
                break;
            }
        }
        else
        {
            /* Replace config to given Index */
            freeCamIndex = GET_CAMERA_INDEX(camIndex);
            for(loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
            {
                if (strcmp(ipCameraCfg[loop].cameraAddress, tempIpCameraCfg.cameraAddress) != STATUS_OK)
                {
                    continue;
                }

                if (ipCameraCfg[loop].onvifSupportF != onvifSupportFlag)
                {
                    continue;
                }

                if (TRUE == onvifSupportFlag)
                {
                    if (ipCameraCfg[loop].onvifPort != onvifPort)
                    {
                        continue;
                    }
                }
                else
                {
                    if (ipCameraCfg[loop].httpPort != httpPort)
                    {
                        continue;
                    }
                }

                /* Default camera config if found on different index */
                if (loop != freeCamIndex)
                {
                    DfltSingleCameraConfig(loop);
                    DfltSingleIpCameraConfig(loop);
                    DfltSingleStreamConfig(loop);
                    DfltSinglePresetTourConfig(loop);
                    DfltSingleTourScheduleConfig(loop);

                    /* Default image setting param and config */
                    DefaultCameraImageCapability(loop);
                }
                break;
            }
        }

        if (freeCamIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_MAX_IP_CAMERA_CONFIGURED;
            break;
        }

        /* changes for camera Addition Feature if already configured max cam or change index of current camera */
        tempCameraCfg.camera = ENABLE;
        tempIpCameraCfg.httpPort = (UINT16)httpPort;
        tempIpCameraCfg.onvifPort = (UINT16)onvifPort;
        tempIpCameraCfg.onvifSupportF = (BOOL)onvifSupportFlag;

        DfltSingleCameraConfig(freeCamIndex);
        DfltSingleIpCameraConfig(freeCamIndex);

        if((WriteSingleCameraConfig(freeCamIndex, &tempCameraCfg) != CMD_SUCCESS)
                || (WriteSingleIpCameraConfig(freeCamIndex, &tempIpCameraCfg) != CMD_SUCCESS))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        /* Make initial Buffer */
        outLen = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d%c%c%c",
                SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, GET_CAMERA_NO(freeCamIndex), FSP, EOI, EOM);
        if(outLen > MAX_REPLY_SZ)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);

        /* Adding entry in Event Log */
        WriteConfigChangeEvent(sessionIndex, TBL_CAMERA_CFG, EVENT_CHANGE);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the cancel camera search command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL CancelCameraSearchCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    if(IsCamSearchActiveForClient(sessionIndex) == ACTIVE)
    {
        if(GetAutoConfigReportFlag(sessionIndex) == AUTO_CONFIG_REPORT_START)
        {
            SetAutoConfigReportFlag(AUTO_CONFIG_REPORT_ABOUT_TO_CLEAR,sessionIndex);
        }
        StopCameraSearch(sessionIndex);
    }
    else if(IsAdvCamSearchActiveForClient(sessionIndex) == ACTIVE)
    {
        StopAdvanceCameraSearch(sessionIndex);
    }

    clientCmdRespCb[clientCbType](CMD_SUCCESS, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback API of test camera command
 * @param   channelNo
 * @param   cmdStat
 * @param   imageBuff
 * @param   imageSize
 * @param   clientCbType
 */
static void ciTestCamCb(UINT8 channelNo, NET_CMD_STATUS_e cmdStat, CHARPTR imageBuff, UINT32 imageSize, CLIENT_CB_TYPE_e clientCbType)
{
    CHAR	respString[128];
    UINT16	outLen;

    if (testCamConnFd[channelNo] == INVALID_CONNECTION)
    {
        return;
    }

    // Make initial Buffer and Validate buffer size for add last four character including NULL
    outLen = snprintf(respString, sizeof(respString), "%c%s%c%03d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdStat, FSP, SOI, 1, FSP);
    if(outLen > sizeof(respString) - 5)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = sizeof(respString) - 5;
    }

    // Append Image Size
    respString[outLen++] = (imageSize & 0xFF);
    respString[outLen++] = ((imageSize >> 8) & 0xFF);
    respString[outLen++] = ((imageSize >> 16) & 0xFF);
    respString[outLen++] = ((imageSize >> 24) & 0xFF);
    respString[outLen++] = FSP;

    if (sendCmdCb[clientCbType](testCamConnFd[channelNo], (UINT8PTR)respString, outLen, MESSAGE_REPLY_TIMEOUT) == SUCCESS)
    {
        if (cmdStat == CMD_SUCCESS)
        {
            if (sendCmdCb[clientCbType](testCamConnFd[channelNo], (UINT8PTR)imageBuff, imageSize, MESSAGE_REPLY_TIMEOUT) == SUCCESS)
            {
                respString[0] = FSP;
                respString[1] = EOI;
                respString[2] = EOM;
                sendCmdCb[clientCbType](testCamConnFd[channelNo], (UINT8PTR)respString, 3, MESSAGE_REPLY_TIMEOUT);
            }
        }
    }

    closeConnCb[clientCbType](&testCamConnFd[channelNo]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the test camera command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL TestCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT64              tempData;
    UINT8               camIndex;
    UINT32              outLen;
    const UINT32        respStringSize = MAX_REPLY_SZ - 6;

    do
    {
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        camIndex = (UINT8)GET_CAMERA_INDEX(tempData);
        if (testCamConnFd[camIndex] != INVALID_CONNECTION)
        {
            cmdResp = CMD_REQUEST_IN_PROGRESS;
            break;
        }

        testCamConnFd[camIndex] = clientSocket;
        cmdResp = GetImage(camIndex, NULL, ciTestCamCb, clientCbType);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

        testCamConnFd[camIndex] = INVALID_CONNECTION;

    }while(0);

    // Make initial Buffer
    outLen = snprintf(&replyMsg[clientCbType][0], respStringSize, "%c%s%c%03d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    // Append 4 Byte Image Size
    replyMsg[clientCbType][outLen++] = 0;
    replyMsg[clientCbType][outLen++] = 0;
    replyMsg[clientCbType][outLen++] = 0;
    replyMsg[clientCbType][outLen++] = 0;
    replyMsg[clientCbType][outLen++] = FSP;
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search recording (Monthwise search)
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL PlaySearchMonthWiseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8                       cameraIndex;
    NET_CMD_STATUS_e			cmdResp;
    CAMERA_BIT_MASK_t           cameraMask;
    UINT64 						tempData[MAX_PLAY_SEARCH_MNTH_RCD_ARG] = {0};
    SEARCH_CRITERIA_MNTH_DAY_t	searchCriteria;

    memset(&cameraMask, 0, sizeof(cameraMask));
    memset(&searchCriteria, 0, sizeof(searchCriteria));

    do
    {
        if (FAIL == ParseStringGetVal(pCmdStr, tempData, PLAY_SEARCH_MNTH_RCD_STORAGE_TYPE, FSP))
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            EPRINT(NETWORK_MANAGER, "fail to parse play month wise search msg");
            break;
        }

        if (FAIL == ParseStringGetVal(pCmdStr, &tempData[PLAY_SEARCH_MNTH_RCD_STORAGE_TYPE], 1, FSP))
        {
            tempData[PLAY_SEARCH_MNTH_RCD_STORAGE_TYPE] = MAX_RECORDING_MODE;
        }

        if (FAIL == ParseStringGetVal(pCmdStr, &tempData[PLAY_SEARCH_MNTH_RCD_CAMERA_MASK2], 1, FSP))
        {
            tempData[PLAY_SEARCH_MNTH_RCD_CAMERA_MASK2] = 0;
        }

        searchCriteria.eventType = (EVENT_e)tempData[PLAY_SEARCH_MNTH_RCD_EVENT_TYPE];
        if (searchCriteria.eventType > DM_ANY_EVENT)
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        CONVERT_CMD_MNTH_YEAR_TO_BROKEN_TIME(tempData[PLAY_SEARCH_MNTH_RCD_MNTH_YEAR], searchCriteria.timeData);
        cameraMask.bitMask[0] = tempData[PLAY_SEARCH_MNTH_RCD_CAMERA_MASK1];
        cameraMask.bitMask[1] = tempData[PLAY_SEARCH_MNTH_RCD_CAMERA_MASK2];
        searchCriteria.searchRecStorageType = (RECORD_ON_DISK_e)tempData[PLAY_SEARCH_MNTH_RCD_STORAGE_TYPE];

        DPRINT(NETWORK_MANAGER, "search month record: [cameraMask1=0x%llx & cameraMask2=0x%llx], [eventType=0x%x], [storage=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
               cameraMask.bitMask[0], cameraMask.bitMask[1], searchCriteria.eventType, searchCriteria.searchRecStorageType,
               searchCriteria.timeData.tm_mday, searchCriteria.timeData.tm_mon + 1, searchCriteria.timeData.tm_year,
               searchCriteria.timeData.tm_hour, searchCriteria.timeData.tm_min, searchCriteria.timeData.tm_sec);

        for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            if (GET_CAMERA_MASK_BIT(cameraMask, cameraIndex))
            {
                searchCriteria.cameraRecordF[cameraIndex] = TRUE;
            }
        }

        cmdResp = SearchRecord(&searchCriteria, MONTH_WISE_SEARCH, searchCriteria.searchRecStorageType, sessionIndex,
                               clientCmdRespCb[clientCbType], clientSocket, clientCbType);

        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search recording (Daywise search)
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL PlaySearchDayWiseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8                       cameraIndex;
    NET_CMD_STATUS_e			cmdResp;
    CAMERA_BIT_MASK_t           cameraMask;
    UINT64                      tempData[MAX_PLAY_SEARCH_DAY_RCD_ARG] = {0};
    SEARCH_CRITERIA_MNTH_DAY_t	searchCriteria;

    memset(&cameraMask, 0, sizeof(cameraMask));
    memset(&searchCriteria, 0, sizeof(searchCriteria));

    do
    {
        if (FAIL == ParseStringGetVal(pCmdStr, tempData, PLAY_SEARCH_DAY_RCD_STORAGE_TYPE, FSP))
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            EPRINT(NETWORK_MANAGER, "fail to parse play day wise search msg");
            break;
        }

        if (FAIL == ParseStringGetVal(pCmdStr, &tempData[PLAY_SEARCH_DAY_RCD_STORAGE_TYPE], 1, FSP))
        {
            tempData[PLAY_SEARCH_DAY_RCD_STORAGE_TYPE] = MAX_RECORDING_MODE;
        }

        if (FAIL == ParseStringGetVal(pCmdStr, &tempData[PLAY_SEARCH_DAY_RCD_CAMERA_MASK2], 1, FSP))
        {
            tempData[PLAY_SEARCH_DAY_RCD_CAMERA_MASK2] = 0;
        }

        searchCriteria.eventType = (EVENT_e)tempData[PLAY_SEARCH_DAY_RCD_EVENT_TYPE];
        if (searchCriteria.eventType > DM_ANY_EVENT)
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        CONVERT_CMD_DATE_TO_BROKEN_TIME(tempData[PLAY_SEARCH_DAY_RCD_DATE], searchCriteria.timeData);
        cameraMask.bitMask[0] = tempData[PLAY_SEARCH_DAY_RCD_CAMERA_MASK1];
        cameraMask.bitMask[1] = tempData[PLAY_SEARCH_DAY_RCD_CAMERA_MASK2];
        searchCriteria.searchRecStorageType = (RECORD_ON_DISK_e)tempData[PLAY_SEARCH_DAY_RCD_STORAGE_TYPE];

        DPRINT(NETWORK_MANAGER, "search day record: [cameraMask1=%llx & cameraMask2=%llx], [eventType=%d], [storage=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
               cameraMask.bitMask[0], cameraMask.bitMask[1], searchCriteria.eventType, searchCriteria.searchRecStorageType,
               searchCriteria.timeData.tm_mday, searchCriteria.timeData.tm_mon + 1, searchCriteria.timeData.tm_year,
               searchCriteria.timeData.tm_hour, searchCriteria.timeData.tm_min, searchCriteria.timeData.tm_sec);

        for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            if (GET_CAMERA_MASK_BIT(cameraMask, cameraIndex))
            {
                searchCriteria.cameraRecordF[cameraIndex] = TRUE;
            }
        }

        cmdResp = SearchRecord(&searchCriteria, DAY_WISE_SEARCH, searchCriteria.searchRecStorageType, sessionIndex,
                               clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start records playback in sync
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncPlayStartCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SYNC_PLAY_STREAM_ARG] = {0};
    UINT32					camIndex;
    CAMERA_BIT_MASK_t       cameraMask;
    struct tm				startTime;
    UINT8					userIndex;
    SYNC_PLAY_PARAMS_t      syncPlayParam;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    memset(&cameraMask, 0, sizeof(cameraMask));
    memset(&syncPlayParam, 0, sizeof(syncPlayParam));

    do
    {
        if (FAIL == ParseStringGetVal(pCmdStr, tempData, SYNC_PLAY_STREAM_RCD_STORAGE_TYPE, FSP))
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            EPRINT(NETWORK_MANAGER, "fail to parse sync play msg");
            break;
        }

        if (FAIL == ParseStringGetVal(pCmdStr, &tempData[SYNC_PLAY_STREAM_RCD_STORAGE_TYPE], 1, FSP))
        {
            tempData[SYNC_PLAY_STREAM_RCD_STORAGE_TYPE] = MAX_RECORDING_MODE;
        }

        if (FAIL == ParseStringGetVal(pCmdStr, &tempData[SYNC_PLAY_STREAM_CAMERA_MASK2], 1, FSP))
        {
            tempData[SYNC_PLAY_STREAM_CAMERA_MASK2] = 0;
        }

        cameraMask.bitMask[0] = tempData[SYNC_PLAY_STREAM_CAMERA_MASK1];
        cameraMask.bitMask[1] = tempData[SYNC_PLAY_STREAM_CAMERA_MASK2];
        syncPlayParam.recStorageDrive = (RECORD_ON_DISK_e)tempData[SYNC_PLAY_STREAM_RCD_STORAGE_TYPE];
        userIndex = GetUserAccountIndex(sessionIndex);
        ReadSingleUserAccountConfig(userIndex, &userAccountConfig);

        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            if (FALSE == GET_CAMERA_MASK_BIT(cameraMask, camIndex))
            {
                continue;
            }

            if (ENABLE == userAccountConfig.userPrivilege[camIndex].privilegeBitField.playback)
            {
                break;
            }
        }

        if (camIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            if (TRUE == GET_CAMERA_MASK_BIT(cameraMask, camIndex))
            {
                syncPlayParam.cameraRecordF[camIndex] = TRUE;
                syncPlayParam.totalCamera++;
            }
        }

        syncPlayParam.direction = ((PLAYBACK_CMD_e)tempData[SYNC_PLAY_STREAM_DIRECTION] == 0) ? PLAY_FORWARD : PLAY_REVERSE;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SYNC_PLAY_STREAM_PLAY_TIME], startTime);
        syncPlayParam.speed = (UINT8)tempData[SYNC_PLAY_STREAM_SPEED];
        syncPlayParam.eventType = (EVENT_e)tempData[SYNC_PLAY_STREAM_EVENT];
        cmdResp = AddSyncPlaybackStream(&syncPlayParam, &startTime, clientCbType, sessionIndex, userIndex, clientCmdRespCb[clientCbType], clientSocket);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, (cmdResp == CMD_SUCCESS) ? FALSE : TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pause sync records playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncPlayPauseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e cmdResp = PauseSyncPlaybackStream(sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
    if (cmdResp != CMD_SUCCESS)
    {
        clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Resume sync records playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncPlayResumeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e cmdResp = ResumeSyncPlaybackStream(sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
    if (cmdResp != CMD_SUCCESS)
    {
        clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop sync records playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncPlayStopCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e cmdResp = StopSyncPlaybackStream(sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
    if (cmdResp != CMD_SUCCESS)
    {
        clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear sync records playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncPlayClearCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e cmdResp = RemoveSyncPlaybackStream(sessionIndex, NULL, INVALID_CONNECTION, DISK_ACT_NORMAL);
    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set sync records playback position
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncPlaySetPositionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SYNC_PLAY_SEEK_SPEED_ARG] = {0};
    UINT32					camIndex;
    CAMERA_BIT_MASK_t       cameraMask, cameraMaskAudio;
    struct tm				startTime;
    SYNC_PLAY_PARAMS_t      syncPlayParam;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    memset(&cameraMask, 0, sizeof(cameraMask));
    memset(&cameraMaskAudio, 0, sizeof(cameraMaskAudio));
    memset(&syncPlayParam, 0, sizeof(syncPlayParam));

    do
    {
        if (ParseStringGetVal(pCmdStr, tempData, SYNC_PLAY_SEEK_SPEED_CAMERA_MASK2, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            EPRINT(NETWORK_MANAGER, "fail to parse set position msg");
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData[SYNC_PLAY_SEEK_SPEED_CAMERA_MASK2], 1, FSP) == FAIL)
        {
            tempData[SYNC_PLAY_SEEK_SPEED_CAMERA_MASK2] = 0;
        }

        if (ParseStringGetVal(pCmdStr, &tempData[SYNC_PLAY_SEEK_SPEED_AUDIO_MASK2], 1, FSP) == FAIL)
        {
            tempData[SYNC_PLAY_SEEK_SPEED_AUDIO_MASK2] = 0;
        }

        cameraMask.bitMask[0] = tempData[SYNC_PLAY_SEEK_SPEED_CAMERA_MASK1];
        cameraMask.bitMask[1] = tempData[SYNC_PLAY_SEEK_SPEED_CAMERA_MASK2];
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

        for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            if (FALSE == GET_CAMERA_MASK_BIT(cameraMask, camIndex))
            {
                continue;
            }

            if (ENABLE == userAccountConfig.userPrivilege[camIndex].privilegeBitField.playback)
            {
                break;
            }
        }

        if (camIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        cameraMaskAudio.bitMask[0] = tempData[SYNC_PLAY_SEEK_SPEED_AUDIO_MASK1];
        cameraMaskAudio.bitMask[1] = tempData[SYNC_PLAY_SEEK_SPEED_AUDIO_MASK2];

        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            if (TRUE == GET_CAMERA_MASK_BIT(cameraMask, camIndex))
            {
                syncPlayParam.cameraRecordF[camIndex] = TRUE;
                syncPlayParam.totalCamera++;
            }

            if (TRUE == GET_CAMERA_MASK_BIT(cameraMaskAudio, camIndex))
            {
                syncPlayParam.cameraAudioF[camIndex] = TRUE;
            }
        }

        syncPlayParam.direction = ((PLAYBACK_CMD_e)tempData[SYNC_PLAY_SEEK_SPEED_DIRECTION] == 0) ? PLAY_FORWARD : PLAY_REVERSE;
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SYNC_PLAY_SEEK_SPEED_TIME], startTime);
        syncPlayParam.speed = (UINT8)tempData[SYNC_PLAY_SEEK_SPEED_SPEED];
        syncPlayParam.eventType = (UINT8)tempData[SYNC_PLAY_SEEK_SPEED_EVENT_TYPE];
        syncPlayParam.frmSyncNum = (UINT8)tempData[SYNC_PLAY_SEEK_SPEED_FRM_SYNC_NO];
        cmdResp = SetSyncPlaybackPosition(&syncPlayParam, &startTime, sessionIndex, NULL, INVALID_CONNECTION);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the download sync playback record clip command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SyncRecordClipCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8					clipCnt = 0;
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_SYNC_CPL_ARG];
    UINT64					tmpEventData;
    EVENT_TYPE_e			eventType;
    SYNC_CLIP_DATA_t		clipData[MAX_SYNC_CLIP_NUMBER];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    RECORD_ON_DISK_e		recStorageDrive = MAX_RECORDING_MODE;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if (userAccountConfig.userGroup == VIEWER)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tmpEventData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        eventType = (EVENT_TYPE_e)tmpEventData;
        for(clipCnt = 0; clipCnt < MAX_SYNC_CLIP_NUMBER; clipCnt++)
        {
            if(ParseStringGetVal(pCmdStr, tempData, SYNC_CLP_RECORD_STORAGE_TYPE, FSP) == FAIL)
            {
                cmdResp = CMD_INVALID_SYNTAX;
                break;
            }

            clipData[clipCnt].cameraNo = (UINT8)tempData[SYNC_CLP_CAMERA_NO];
            CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SYNC_CLP_START_TIME], clipData[clipCnt].startTime);
            CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[SYNC_CLP_END_TIME], clipData[clipCnt].endTime);
            clipData[clipCnt].eventType = eventType;

            /* Change for mobile client */
            if (ParseStringGetVal(pCmdStr, tempData, 1, FSP) == FAIL)
            {
                tempData[0] = MAX_RECORDING_MODE;
            }

            recStorageDrive = (RECORD_ON_DISK_e)tempData[0];
        }

        /* check if any valid record found */
        if (clipCnt == 0)
        {
            break;
        }

        cmdResp = ClipBackUpOfRecords(clipData, recStorageDrive, clipCnt, NwCallbackFuncForBkupRcd,
                                      clientSocket, userAccountConfig.username, clientCbType);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get mobile number for camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetCameraMobileNumberCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8					camIndex = 0;
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_GET_MOB_NUM_ARG];
    CAMERA_CONFIG_t			camConfig;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    UINT32					outLen;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        if(ParseStringGetVal(pCmdStr, tempData, MAX_GET_MOB_NUM_ARG, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        camIndex = (UINT8)GET_CAMERA_INDEX(tempData[GET_MOB_NUM_CAMERA_NO]);
        if(camIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleCameraConfig(camIndex, &camConfig);
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c%c%d%c%s%c%c%c",
                          SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, camConfig.mobileNum, FSP, EOI, EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get user details for camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetCameraUserDetailCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_PROCESS_ERROR;
    CHAR					brandNameStr[MAX_BRAND_NAME_LEN];
    CHAR					username[MAX_CAMERADETAIL_USERNAME_WIDTH];
    CHAR					password[MAX_CAMERADETAIL_PASSWORD_WIDTH];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT32					outLen;
    const UINT32            respStringSize = MAX_REPLY_SZ;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        // Parse Brand Name
        if(ParseStr(pCmdStr, FSP, brandNameStr, MAX_BRAND_NAME_LEN) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cmdResp = GetCameraUsernamePassword(brandNameStr, username, password);
        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        // {RPL_CMD&0&
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c", SOI, 1, FSP, username, FSP, EOI);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%c", SOI, 2, FSP, password, FSP, EOI);
        //Validate buffer size for add last two character including NULL
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
        }

        // Append EOM
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the system build-date
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetSystemBuildDateCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    UINT32					outLen;

    outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%s%c%c%c",
                      SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, GetBuildDateTimeStr(), FSP, EOI, EOM);

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the camera information
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetCameraInfoCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    CAMERA_CONFIG_t 		cameraCfg;
    IP_CAMERA_CONFIG_t		ipCameraCfg;
    UINT8				    camIndex;
    UINT8				    index = 1;
    UINT32					outLen;
    CHAR 					ipAddress[MAX_CAMERA_ADDRESS_WIDTH];
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    const UINT32            respStringSize = MAX_REPLY_SZ;

    // Read User Config For Rights
    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if(userAccountConfig.userGroup == ADMIN)
    {
        //Validate buffer size for add last two character including NULL
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c", SOM,headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
        }

        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            ReadSingleIpCameraConfig(camIndex, &ipCameraCfg);
            ReadSingleCameraConfig(camIndex, &cameraCfg);
            snprintf(ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", ipCameraCfg.cameraAddress);
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%2d%c%d%c%s%c%s%c%d%c%c",
                               SOI, index++, FSP,(camIndex + 1), FSP,ipAddress, FSP,cameraCfg.name, FSP, cameraCfg.camera, FSP,EOI);

            //Validate buffer size for add last two character including NULL
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
                break;
            }
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
    }
    else
    {
        clientCmdRespCb[clientCbType](CMD_NO_PRIVILEGE, clientSocket, TRUE);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start instant record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StartInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_INSTANT_PLAY_STRT_ARG];
    UINT8					cameraIndex;
    struct tm 				startTime;
    INSTANT_PLAYBACK_ID 	playbackId;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        if (FAIL == ParseStringGetVal(pCmdStr, tempData, MAX_INSTANT_PLAY_STRT_ARG, FSP))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = (UINT8)GET_CAMERA_INDEX(tempData[INSTANT_PLAY_STRT_CAMERA_NO]);
        if (cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex].privilegeBitField.playback == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (GetLocalTimeInBrokenTm(&startTime) == FAIL)
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        cmdResp = AddInstantPlaybackStream(clientCbType, startTime, cameraIndex, sessionIndex, clientCmdRespCb[clientCbType], clientSocket, &playbackId);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set play position for instant record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SeekInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_INSTANT_PLAY_SEEK_ARG];
    INSTANT_PLAYBACK_ID 	playbackId;
    struct tm 				startTime;
    PLAYBACK_CMD_e 			direction;
    UINT32 					audioChannel;
    UINT8					frmSyncNum;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_INSTANT_PLAY_SEEK_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (INSTANT_PLAYBACK_ID)tempData[INSTANT_PLAY_SEEK_PLAYBACK_ID];
        direction = ((PLAYBACK_CMD_e)tempData[INSTANT_PLAY_SEEK_PLAYBACK_DIRECTION] == 0) ? PLAY_FORWARD : PLAY_REVERSE;
        audioChannel = tempData[INSTANT_PLAY_SEEK_PLAYBACK_AUDIO];
        frmSyncNum = (UINT8)tempData[INSTANT_PLAY_SEEK_FRAME_NO];
        CONVERT_CMD_TIME_TO_BROKEN_TIME(tempData[INSTANT_PLAY_SEEK_PLAYBACK_TIME], startTime);
        cmdResp = SetInstantPlaybackPosition(playbackId, sessionIndex, clientCmdRespCb[clientCbType],
                                             clientSocket, startTime, direction, audioChannel, frmSyncNum);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop instant record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StopInstantPlayback(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_INSTANT_PLAY_STOP_ARG];
    INSTANT_PLAYBACK_ID 	playbackId;

    if (FAIL == ParseStringGetVal(pCmdStr, tempData, MAX_INSTANT_PLAY_STOP_ARG, FSP))
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (INSTANT_PLAYBACK_ID)tempData[INSTANT_PLAY_STOP_PLAYBACK_ID];
        cmdResp = StopInstantPlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pause instant record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL PauseInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_INSTANT_PLAY_PAUSE_ARG];
    INSTANT_PLAYBACK_ID 	playbackId;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_INSTANT_PLAY_PAUSE_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (INSTANT_PLAYBACK_ID)tempData[INSTANT_PLAY_PAUSE_PLAYBACK_ID];
        cmdResp = PauseInstantPlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Resume instant record playback
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ResumeInstantPlaybackCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_INSTANT_PLAY_RSM_ARG];
    INSTANT_PLAYBACK_ID 	playbackId;

    if (ParseStringGetVal(pCmdStr, tempData, MAX_INSTANT_PLAY_RSM_ARG, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        playbackId = (INSTANT_PLAYBACK_ID)tempData[INSTANT_PLAY_RSM_PLAYBACK_ID];
        cmdResp = ResumeInstantPlaybackStream(playbackId, sessionIndex, clientCmdRespCb[clientCbType], clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to change camera ip address
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ChangeCameraIpAddressCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    UINT64                      cameraNumber;
    USER_ACCOUNT_CONFIG_t       userAccountConfig;
    IP_ADDR_PARAM_t             networkParam;
    UINT8                       camIndex;
    UINT64                      prefixLen;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if((ParseStringGetVal(pCmdStr, &cameraNumber, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, networkParam.ipAddr, sizeof(networkParam.ipAddr)) == FAIL)
                || (ParseStringGetVal(pCmdStr, &prefixLen, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, networkParam.gateway, sizeof(networkParam.gateway)) == FAIL))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        camIndex = (UINT8)GET_CAMERA_INDEX(cameraNumber);
        if(CameraType(camIndex) != IP_CAMERA)
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        if(GetCamEventStatus(camIndex, CONNECTION_FAILURE) == FAIL)
        {
            cmdResp = CMD_CAM_DISCONNECTED;
            break;
        }

        networkParam.ipAddrType = ((NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(networkParam.ipAddr)) ? IP_ADDR_TYPE_IPV4 : IP_ADDR_TYPE_IPV6);
        networkParam.prefixLen = prefixLen;
        cmdResp = ChangeCamIpAddr(camIndex, &networkParam, clientCmdRespCb[clientCbType], clientSocket, clientCbType);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the advance camera search command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AdvanceCameraSearchCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t       userAccountConfig;
    UINT64                      tempData;
    ADV_IP_CAM_SEARCH_INPARAM_t param;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if (userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if((IsCamSearchActiveForClient(sessionIndex) == ACTIVE) || (IsAdvCamSearchActiveForClient(sessionIndex) == ACTIVE))
        {
            cmdResp = CMD_REQUEST_IN_PROGRESS;
            break;
        }

        if((ParseStr(pCmdStr, FSP, param.startRangeIpAddr, sizeof(param.startRangeIpAddr)) == FAIL)
                || (ParseStr(pCmdStr, FSP, param.endRangeIpAddr, sizeof(param.endRangeIpAddr)) == FAIL)
                || (ParseStr(pCmdStr, FSP, param.brand, sizeof(param.brand)) == FAIL)
                || (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, param.camUsername, sizeof(param.camUsername)) == FAIL)
                || (ParseStr(pCmdStr, FSP, param.camPassword, sizeof(param.camPassword)) == FAIL))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        param.httpPort = (UINT16)tempData;
        SetAutoConfigReportFlag(MAX_AUTO_CONFIG_STATE,sessionIndex);
        cmdResp = StartAdvanceCameraSearch(sessionIndex, &param);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to resume PTZ tour
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL PtzTourResumeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;
    UINT64 					cameraIndex;

    do
    {
        if(ParseStringGetVal(pCmdStr, &cameraIndex, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        cameraIndex = GET_CAMERA_INDEX(cameraIndex);
        if(cameraIndex >= getMaxCameraForCurrentVariant())
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userPrivilege[cameraIndex]. privilegeBitField.ptzControl == DISABLE)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        cmdResp = ResumePtzTour(cameraIndex);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to generate camera search failure report
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GenerateFailureReportCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if (userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        cmdResp = GenerateFailureReport(clientSocket, clientCbType, sessionIndex);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get PTZ tour status
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetPtzTourStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT64          cameraNumber;
    BOOL            tourStatus;
    UINT32          outLen = 0;
    CHARPTR         respStringPtr = &replyMsg[clientCbType][0];
    const UINT32    respStringSize = MAX_REPLY_SZ;

    if(ParseStringGetVal(pCmdStr, &cameraNumber, 1, FSP) == SUCCESS)
    {
        tourStatus = GetCamTourStatus(GET_CAMERA_INDEX(cameraNumber), 0);
        tourStatus = (tourStatus != PTZ_HLT_PAUS_TOUR) ? TOUR_ACTIVE : TOUR_PAUSE ;
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%2d%c%c", SOI,1, FSP, tourStatus, FSP, EOI);
        //Validate buffer size for add last two character including NULL
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
    }
    else
    {
        clientCmdRespCb[clientCbType](CMD_PROCESS_ERROR, clientSocket, TRUE);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get total configured cameras
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetConfiguredCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8 					loop = 0;
    UINT8					resultCnt = 0;
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t		ipCameraCfg[MAX_CAMERA];
    UINT32					outLen = 0;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    const UINT32            respStringSize = MAX_REPLY_SZ;

    ReadIpCameraConfig(ipCameraCfg);
    for(loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
    {
        if(ipCameraCfg[loop].cameraAddress[0] != '\0')
        {
            resultCnt++;
        }
    }

    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c%c%d%c%d%c%c%c",
                      SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, resultCnt, FSP, EOI, EOM);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the test NDD connection command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL TestNddConnectionCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT64 					tempData[MAX_TEST_NDD_CONN];
    UINT8					nddIdx = 0;
    NETWORK_DRIVE_CONFIG_t	nddConfig;

    if((ParseStr(pCmdStr, FSP, nddConfig.ipAddr, sizeof(nddConfig.ipAddr)) == FAIL) ||
        (ParseStringGetVal(pCmdStr, &tempData[TEST_NDD_CONN_FILE_SYS_TYPE], 1, FSP) == FAIL) ||
        (ParseStr(pCmdStr, FSP, nddConfig.userName, sizeof(nddConfig.userName)) == FAIL) ||
        (ParseStr(pCmdStr, FSP, nddConfig.password, sizeof(nddConfig.password)) == FAIL) ||
        (ParseStr(pCmdStr, FSP, nddConfig.dfltFolder, sizeof(nddConfig.dfltFolder)) == FAIL) ||
        (ParseStringGetVal(pCmdStr, &tempData[TEST_NDD_CONN_NDD_NO], 1, FSP) == FAIL))
    {
        cmdResp = CMD_PROCESS_ERROR;
    }
    else
    {
        nddConfig.fileSys = (FILE_SYS_TYPE_e)tempData[TEST_NDD_CONN_FILE_SYS_TYPE];
        nddIdx = (UINT8)tempData[TEST_NDD_CONN_NDD_NO];
        cmdResp = NddTestConnection(nddIdx, &nddConfig, clientCmdRespCb[clientCbType], clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the auto configure camera command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AutoConfigureCameraCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT8					index = 0;
    UINT8 					freeIndex = 0;
    UINT64					tempHttpPort, tempOnvifSupp, tempOnvifPort, tmpStatus;
    AUTO_CONFIG_REQ_PARAM_t cameraList[MAX_CONFIGURE_CAMERA_LIST];

    while(**pCmdStr != EOM)
    {
        if((ParseStringGetVal(pCmdStr, &tmpStatus, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].camName, sizeof(cameraList[index].camName)) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].ipAddr, sizeof(cameraList[index].ipAddr)) == FAIL)
                || (ParseStringGetVal(pCmdStr, &tempHttpPort, 1, FSP) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].brand, sizeof(cameraList[index].brand)) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].model, sizeof(cameraList[index].model)) == FAIL)
                || (ParseStringGetVal(pCmdStr, &tempOnvifSupp, 1, FSP) == FAIL)
                || (ParseStringGetVal(pCmdStr, &tempOnvifPort, 1, FSP) == FAIL))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        cameraList[index].camStatus = (CAM_STATUS_e)tmpStatus;
        cameraList[index].httpPort = (UINT16)tempHttpPort;
        cameraList[index].onvifPort = (UINT16)tempOnvifPort;
        cameraList[index].onvifSupport = (BOOL)tempOnvifSupp;
        index++;
        if(index >= MAX_CONFIGURE_CAMERA_LIST)
        {
            break;
        }
    }

    if(index == 0)
    {
        cmdResp = CMD_PROCESS_ERROR;
    }
    else if(CheckIfConfiguredLessMax(&freeIndex) == FALSE)
    {
        cmdResp = CMD_MAX_IP_CAMERA_CONFIGURED;
    }
    else
    {
        cmdResp = StartAutoConfigProcess(cameraList, index, sessionIndex);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get list of camera found in search
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetAcquiredCameraListCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_PROCESS_ERROR;
    IP_CAM_SEARCH_RESULT_t	searchResult[MAX_CAM_SEARCH_IN_ONE_SHOT];
    UINT8					resultCnt = 0, index = 0;
    UINT32					outLen = 0;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    const UINT32            respStringSize = MAX_REPLY_SZ;
    CHARPTR                 ipv6Addr;

    if(IsCamSearchActiveForClient(sessionIndex) == ACTIVE)
    {
        cmdResp = GetAcqListOfCameras(searchResult, &resultCnt);
    }
    else if(IsAdvCamSearchActiveForClient(sessionIndex) == ACTIVE)
    {
        cmdResp = GetAcqListOfAdvSearchCameras(sessionIndex, searchResult, &resultCnt);
    }

    if(cmdResp == CMD_SUCCESS)
    {
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
        }

        for(index = 0; index < resultCnt; index++)
        {
            /* Prioritize global IPv6 address if available, otherwise use the link-local address */
            if (searchResult[index].ipv6Addr[IPV6_ADDR_GLOBAL][0] != '\0')
            {
                ipv6Addr = &searchResult[index].ipv6Addr[IPV6_ADDR_GLOBAL][0];
            }
            else
            {
                ipv6Addr = &searchResult[index].ipv6Addr[IPV6_ADDR_LINKLOCAL][0];
            }

            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%d%c" "%d%c%s%c" "%s%c%s%c%d%c" "%s%c%s%c" "%d%c%d%c%c",
                               SOI, (index + 1), FSP ,searchResult[index].camStatus, FSP, searchResult[index].camIndex, FSP,
                               searchResult[index].camName, FSP, searchResult[index].ipv4Addr, FSP, ipv6Addr, FSP, searchResult[index].httpPort, FSP,
                               searchResult[index].brand, FSP, searchResult[index].model, FSP, searchResult[index].onvifSupport, FSP,
                               searchResult[index].onvifPort, FSP, EOI);
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
                break;
            }
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    }

    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to calculate CPU load
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetCpuLoadCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    // Make initial Buffer
    UINT32 outLen = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d%c%c%c",
            SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, GetCurrCpuUsage(), FSP, EOI, EOM);
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the user rights for each camera when user is viewer
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetUserRightsCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8					cameraIndex;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];

    do
    {
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            sizeWritten = snprintf(resStringPtr, resStringSize, "%d%c", userAccountConfig.userPrivilege[cameraIndex].privilegeGroup, FSP);
            if(sizeWritten >= resStringSize)
            {
                cmdResp = CMD_MAX_BUFFER_LIMIT;
                break;
            }

            resStringPtr += sizeWritten;
            resStringSize -= sizeWritten;
        }

        if(cmdResp != CMD_SUCCESS)
        {
            break;
        }

        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c%c", EOI, EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the Video and Date format according to Timezone index received.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL GetRegionalSettingsCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT32 					sizeWritten, resStringSize = MAX_REPLY_SZ;
    CHARPTR 				resStringPtr = &replyMsg[clientCbType][0];
    UINT64 					tempData;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if(userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if(ParseStringGetVal(pCmdStr, &tempData,1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if((tempData < MIN_TIMEZONE) || (tempData > MAX_TIMEZONE))
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        sizeWritten = snprintf(resStringPtr, resStringSize, "%d%c%d%c", RegionalInfo[(tempData-1)][0],FSP,RegionalInfo[(tempData-1)][1], FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c%c", EOI, EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to send device client window layout file
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL SendLayoutFileCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_PROCESS_ERROR;
    FILE                *fp;
    CHAR                strBuff[MAX_STRING_LENGTH];
    CHAR                LayoutFile[200];

    USERWISE_LAYOUT_FILE(LayoutFile,GetUserAccountIndex(sessionIndex));
    if(ParseStr(pCmdStr, FSP, strBuff, MAX_STRING_LENGTH) == SUCCESS)
    {
        fp = fopen(LayoutFile,"w");
        if(fp != NULL)
        {
            fputs(strBuff, fp);
            fclose(fp);
            cmdResp = CMD_SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to receive device client window layout file
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ReceiveLayoutFileCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    FILE                *fp;
    CHAR                recvBuff[MAX_STRING_LENGTH] = { 0 };
    CHAR                strBuff[MAX_STRING_LENGTH]  = { 0 };
    UINT32              sizeWritten, resStringSize;
    CHARPTR             resStringPtr;
    CHAR                LayoutFile[200];
    UINT16              outLen = 0;

    do
    {
        USERWISE_LAYOUT_FILE(LayoutFile,GetUserAccountIndex(sessionIndex));
        fp = fopen(LayoutFile,"r");
        if(fp == NULL)
        {
            cmdResp = CMD_NO_RECORD_FOUND;
            break;
        }

        while((fgets(recvBuff,sizeof(recvBuff), fp)) != NULL)
        {
            outLen += snprintf(strBuff + outLen, MAX_STRING_LENGTH - outLen, "%s", recvBuff);
            if(outLen >= MAX_STRING_LENGTH)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = MAX_STRING_LENGTH;
                break;
            }
        }
        fclose(fp);

        resStringPtr  = &replyMsg[clientCbType][0];
        resStringSize = MAX_REPLY_SZ;
        sizeWritten = snprintf(resStringPtr, resStringSize, "%c%s%c%d%c%c%d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp,FSP, SOI, 1, FSP);
        if(sizeWritten >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%s%c", strBuff, FSP)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        resStringPtr += sizeWritten;
        resStringSize -= sizeWritten;
        if((sizeWritten = snprintf(resStringPtr, resStringSize, "%c%c", EOI, EOM)) >= resStringSize)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], strlen(&replyMsg[clientCbType][0]), MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to give auto config report to the client
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL AutoConfigStatusReportCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    AUTO_CONFIG_STATUS_LIST_t   *autoConfigStatusList;
    UINT8                       index;
    UINT32                      outLen;
    CHARPTR                     respStringPtr = &replyMsg[clientCbType][0];
    const UINT32                respStringSize = MAX_REPLY_SZ;

    autoConfigStatusList = GetAutoConfigStatusReportData(sessionIndex);
    if(autoConfigStatusList == NULL)
    {
        clientCmdRespCb[clientCbType](CMD_INVALID_SYNTAX, clientSocket, TRUE);
        return SUCCESS;
    }

    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    for(index = 0; index < MAX_CONFIGURE_STATUS_LIST; index++)
    {
        if(autoConfigStatusList->autoConfigStatusReport[index].configFailReason == MAX_AUTO_CONFIG_FAIL_REASON)
        {
            break;
        }

        //Validate buffer size for add last character including NULL
        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c" "%s%c" "%d%c" "%d%c" "%s%c" "%d%c" "%c",
                    SOI, (index + 1), FSP ,
                    autoConfigStatusList->autoConfigStatusReport[index].detectedIpAddress, FSP,
                    autoConfigStatusList->autoConfigStatusReport[index].autoConfigStatus,FSP,
                    autoConfigStatusList->autoConfigStatusReport[index].cameraIndex,FSP,
                    autoConfigStatusList->autoConfigStatusReport[index].changedIpAddress,FSP,
                    autoConfigStatusList->autoConfigStatusReport[index].configFailReason,FSP, EOI);
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
            break;
        }
    }

    outLen += snprintf(respStringPtr + outLen , respStringSize - outLen, "%c", EOM);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start transmiting audio(from device/mobile client) to local client: Used when speak to device
 *          is performed by device client or mobile client. This command will be sent by local client.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL StartTxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e cmdResp = CMD_SUCCESS;

    /* Check if audio transfer thread is running */
    if (FALSE == IsAudioTransferInProcess())
    {
        WPRINT(NETWORK_MANAGER, "audio transfer thread already exited: [session=%d]", sessionIndex);
        cmdResp = CMD_AUD_SND_REQ_FAIL;
    }
    else
    {
        UpdateAudioSendFd(clientSocket);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, (cmdResp == CMD_SUCCESS) ? FALSE : TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop transmiting audio(from device/mobile client) to local client: Used when speak to device.
 *          command is sent by local client.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL StopTxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT64              stopReasonCode = CMD_AUD_SND_REQ_FAIL;

    if (ParseStringGetVal(pCmdStr, &stopReasonCode, 1, FSP) == FAIL)
    {
        cmdResp = CMD_INVALID_SYNTAX;
    }

    /* Notify the audio transfer thread to terminate audio transmission */
    StopClientAudioTransmission(stopReasonCode);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start receiving audio from device/mobile client: User for two way audio
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL StartRxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    AUDIO_OUT_CONFIG_t  audioOutConfig;
    UINT64 			    destination;
    UINT64			    cameraIndex;
    CLIENT_AUD_INFO_t   clientAudRxInfo = { 0 };

    /* Init clientAudRxInfo variable */
    clientAudRxInfo.destination = CLIENT_AUDIO_DESTINATION_MAX;
    clientAudRxInfo.cameraIndex = INVALID_CAMERA_INDEX;

    do
    {
        /* Parse destination where audio is to be forwarded (camera or local client) */
        if(ParseStringGetVal(pCmdStr,&destination,1,FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Parse camera index. In case of speak to device it will be 0 */
        if(ParseStringGetVal(pCmdStr,&cameraIndex,1,FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if((destination >= CLIENT_AUDIO_DESTINATION_MAX) || (cameraIndex > getMaxCameraForCurrentVariant()))
        {
            cmdResp = CMD_INVALID_FIELD_VALUE;
            break;
        }

        /* If audio-in command is from local client and audion-in support is not available */
        if ((sessionIndex == USER_LOCAL) && (FALSE == IsAudioInPortAvailable()))
        {
            DPRINT(NETWORK_MANAGER, "audio in support is not available: [session=%d]", sessionIndex);
            cmdResp = CMD_AUD_SND_REQ_FAIL;
            break;
        }

        /* If already one device client is sending audio, send channel busy */
        if (TRUE == IsAudioTransferInProcess())
        {
            DPRINT(NETWORK_MANAGER, "audio channel already busy: [session=%d]", sessionIndex);
            cmdResp = CMD_AUD_CHANNEL_BUSY;
            break;
        }

        clientAudRxInfo.destination = (CLIENT_AUDIO_DESTINATION_e)destination;
        clientAudRxInfo.sessionIndex = sessionIndex;
        clientAudRxInfo.clientSockFd = clientSocket;
        clientAudRxInfo.clientCbType = clientCbType;

        if(clientAudRxInfo.destination == CLIENT_AUDIO_DESTINATION_DEVICE)
        {
            /* Check whether audio out port available or not in the NVR */
            if (FALSE == IsAudioOutPortAvailable())
            {
                cmdResp = CMD_NO_AUD_OUT_AVAILABLE;
                break;
            }

            clientAudRxInfo.cameraIndex = cameraIndex;
            ReadAudioOutConfig(&audioOutConfig);
            if((audioOutConfig.priority <= AUDIO_OUT_PRIORITY_NONE) || (audioOutConfig.priority >= AUDIO_OUT_PRIORITY_MAX))
            {
                cmdResp = CMD_INVALID_FIELD_VALUE;
                break;
            }
        }
        else if(clientAudRxInfo.destination == CLIENT_AUDIO_DESTINATION_CAMERA)
        {
            USER_ACCOUNT_CONFIG_t userAccountConfig;

            /* Check if user has priviledge of audio out for particular camera */
            clientAudRxInfo.cameraIndex = GET_CAMERA_INDEX(cameraIndex);
            ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
            if (userAccountConfig.userPrivilege[clientAudRxInfo.cameraIndex].privilegeBitField.audioOut != ENABLE)
            {
                DPRINT(NETWORK_MANAGER, "no rights to access rx audio: [camera=%d]", clientAudRxInfo.cameraIndex);
                cmdResp = CMD_NO_PRIVILEGE;
                break;
            }

            /* Allocate data transfer fd on success */
            cmdResp = ProcessClientAudioToCamera(&clientAudRxInfo);
            if (cmdResp != CMD_SUCCESS)
            {
                break;
            }
        }

        if (FAIL == StartClientAudioTransfer(&clientAudRxInfo))
        {
            cmdResp = CMD_AUD_SND_REQ_FAIL;
            break;
        }

        return SUCCESS;

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop receiving audio from device/mobile client: User for two way audio
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL StopRxClientAudioCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e cmdResp = CMD_SUCCESS;

    StopClientAudioReception(sessionIndex);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to get the camera inititated list
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetCameraInitiatedListCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_PROCESS_ERROR;
    AUTO_INIT_CAM_LIST_INFO	searchResult[MAX_AUTO_INIT_CAM_LIST];
    UINT8					index = 0;
    UINT8					resultCnt = 0;
    UINT32					outLen;
    CHARPTR					respStringPtr = &replyMsg[clientCbType][0];
    const UINT32            respStringSize = MAX_REPLY_SZ;

    cmdResp = GetCamInitList(searchResult, &resultCnt);
    if(cmdResp == CMD_SUCCESS)
    {
        //Validate buffer size for add last two character including NULL
        outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%03d%c", SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP);
        if(outLen > respStringSize - 2)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize - 2;
        }

        for(index = 0; index < resultCnt; index++)
        {
            //Validate buffer size for add last two character including NULL
            outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c%s%c%s%c%s%c%c",
                               SOI, (index + 1), FSP ,searchResult[index].cameraMacAddr, FSP, searchResult[index].camIpAddr, FSP,
                               searchResult[index].cameraModelname, FSP, EOI);
            if(outLen > respStringSize - 2)
            {
                EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
                outLen = respStringSize - 2;
            }
        }

        outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
        if(outLen > respStringSize)
        {
            EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
            outLen = respStringSize;
        }

        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    }

    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to add the inititated camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL AddCameraInitiatedCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT8					index = 0;
    AUTO_INIT_CAM_LIST_INFO	cameraList[MAX_AUTO_INIT_CAM_LIST];

    while(**pCmdStr != EOM)
    {
        memset(&cameraList[index], 0, sizeof(AUTO_INIT_CAM_LIST_INFO));
        if((ParseStr(pCmdStr, FSP, cameraList[index].cameraMacAddr, sizeof(cameraList[index].cameraMacAddr)) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].camIpAddr, sizeof(cameraList[index].camIpAddr)) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].cameraModelname, sizeof(cameraList[index].cameraModelname)) == FAIL))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        index++;
        if(index == MAX_AUTO_INIT_CAM_LIST)
        {
            break;
        }
    }

    if(index == 0)
    {
        cmdResp = CMD_PROCESS_ERROR;
    }
    else
    {
        cmdResp = AddInitiatedCam(cameraList, index);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute to reject the inititated camera
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL RejectCameraInitiatedCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    UINT8					index = 0;
    AUTO_INIT_CAM_LIST_INFO	cameraList[MAX_AUTO_INIT_CAM_LIST];

    while(**pCmdStr != EOM)
    {
        memset(&cameraList[index], 0, sizeof(AUTO_INIT_CAM_LIST_INFO));
        if((ParseStr(pCmdStr, FSP, cameraList[index].cameraMacAddr, sizeof(cameraList[index].cameraMacAddr)) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].camIpAddr, sizeof(cameraList[index].camIpAddr)) == FAIL)
                || (ParseStr(pCmdStr, FSP, cameraList[index].cameraModelname, sizeof(cameraList[index].cameraModelname)) == FAIL))
        {
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        index++;
        if(index == MAX_AUTO_INIT_CAM_LIST)
        {
            break;
        }
    }

    if(index == 0)
    {
        cmdResp = CMD_PROCESS_ERROR;
    }
    else
    {
        cmdResp = RejectInitiatedCam(cameraList, index);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API execute the manual backup command
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL ManualBackupCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;
    UINT64 					tempData;
    UINT64 					startTime;
    UINT64					endTime;
    struct tm               startTmTime;
    struct tm               endTmTime;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if (userAccountConfig.userGroup == VIEWER)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        if (ParseStringGetVal(pCmdStr, &tempData,1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }
        startTime = (UINT64)tempData;

        if (ParseStringGetVal(pCmdStr, &tempData,1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }
        endTime = (UINT64)tempData;

        CONVERT_CMD_TIME_TO_BROKEN_TIME(startTime, startTmTime);
        CONVERT_CMD_TIME_TO_BROKEN_TIME(endTime, endTmTime);
        cmdResp = StartManualBackup(startTmTime, endTmTime);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get list of configured system's languages
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetLanguageCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8           indexId = 0;
    UINT32          outLen;
    CHARPTR         respStringPtr = &replyMsg[clientCbType][0];
    DIR             *dir;
    struct dirent   *entry;
    CHAR            languageList[MAX_LANGUAGE][MAX_LANGUAGE_FILE_NAME_LEN];
    UINT8           noOfLanguage = 0;
    CHARPTR         subStr;
    const UINT32    respStringSize = MAX_REPLY_SZ;

    dir = opendir(LANGUAGES_DIR_PATH);
    if (dir == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open dir: [path=%s]", LANGUAGES_DIR_PATH);
        return FAIL;
    }

    memset(languageList, '\0', sizeof(languageList));
    snprintf(languageList[noOfLanguage], MAX_LANGUAGE_FILE_NAME_LEN, "English");
    noOfLanguage++;

    while ((entry = readdir(dir)) != NULL)
    {
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
        {
            continue;
        }

        if (strcmp(SAMPLE_CSV_FILE_NAME, entry->d_name) == 0)
        {
            continue;
        }

        subStr = strstr(entry->d_name, ".csv");
        if(subStr == NULL)
        {
            continue;
        }

        subStr[0] = '\0';
        snprintf(languageList[noOfLanguage], MAX_LANGUAGE_FILE_NAME_LEN, "%s", entry->d_name);
        noOfLanguage++;
    }
    closedir(dir);

    outLen = snprintf(respStringPtr, respStringSize, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c%d%c" "%s%c" "%s%c" "%s%c" "%s%c" "%s%c" "%s%c" "%c",
                       SOI, (indexId + 1), FSP , languageList[0], FSP, languageList[1], FSP, languageList[2], FSP,
            languageList[3],FSP, languageList[4],FSP, languageList[5],FSP, EOI);
    if(outLen > respStringSize - 2)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize - 2;
    }

    outLen += snprintf(respStringPtr + outLen, respStringSize - outLen, "%c", EOM);
    if(outLen > respStringSize)
    {
        EPRINT(NETWORK_MANAGER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = respStringSize;
    }

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get user's configured language
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetUserLanguageCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT32                  outLen;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    /* Read user's configuration for current configured language */
    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    outLen = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%s%c%c%c",
                      SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, userAccountConfig.preferredLanguage, FSP, EOI, EOM);

    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType], outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&clientSocket);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set language for user in configuration
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL SetUserLanguageCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e        cmdResp = CMD_SUCCESS;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    /* Read user's configuration for current configured language */
    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

    /* Parse and update language from message */
    if (ParseStr(pCmdStr, FSP, userAccountConfig.preferredLanguage, sizeof(userAccountConfig.preferredLanguage)) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse user language");
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        /* Write updated user's language in configuration */
        WriteSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

        /* Adding entry in Event Log */
        WriteConfigChangeEvent(sessionIndex, TBL_USER_ACCOUNT_CFG, EVENT_CHANGE);
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Give p2p connection status
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT32              outLen;
    UINT64              tempData;
    STATUS_CMD_ID_e     reqStatusCmd;
    INT32               status = 0;

    do
    {
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Get the status command */
        reqStatusCmd = (UINT8)tempData;
        switch (reqStatusCmd)
        {
            case STATUS_CMD_ID_P2P:
                status = GetP2pStatus();
                break;

            case STATUS_CMD_ID_SAMAS:
                status = GetDiConnectionStatus();
                break;

            default:
                cmdResp = CMD_PROCESS_ERROR;
                break;
        }

        /* If error found then send error response */
        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        /* Prepare response string */
        outLen = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d%c%d%c%c%c",
                SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, reqStatusCmd, FSP, status, FSP, EOI, EOM);

        /* send reply to client */
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType], outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while (0);

    /* Send negative reply to client */
    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search firmware on configured server (e.g. FTP server)
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL SearchFirmwareCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if (userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        cmdResp = CheckManualFirmwareUpdate(clientCbType, clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start system firmware upgrade with latest found firmware in "Search Firmware Command"
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL StartUpgradeCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
    if (userAccountConfig.userGroup != ADMIN)
    {
        cmdResp = CMD_NO_PRIVILEGE;
    }
    else
    {
        cmdResp = UpgradeSystemFirmware(clientCbType, clientSocket);
        if (cmdResp == CMD_SUCCESS)
        {
            return SUCCESS;
        }
    }

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the DHCP server lease status information
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetDhcpLeaseCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT16                      leaseCnt;
    UINT16                      totalLeaseEntry;
    CHARPTR                     pRespStr = &replyMsg[clientCbType][0];;
    UINT32                      sizeWritten;
    DHCP_SERVER_LEASE_STATUS_t  leaseInfo[DHCP_SERVER_LEASE_CLIENT_MAX];

    do
    {
        /* Add response message header */
        sizeWritten = snprintf(pRespStr, MAX_REPLY_SZ, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
        if (sizeWritten >= MAX_REPLY_SZ)
        {
            /* Buffer is small to add message header */
            EPRINT(NETWORK_MANAGER, "buffer is small to add msg header");
            break;
        }

        /* Get all lease status */
        totalLeaseEntry = GetDhcpServerLeaseStatus(leaseInfo);
        if (totalLeaseEntry)
        {
            for (leaseCnt = 0; leaseCnt < totalLeaseEntry; leaseCnt++)
            {
                /* Add response message data */
                sizeWritten += snprintf(pRespStr + sizeWritten, MAX_REPLY_SZ - sizeWritten, "%c%d%c%s%c%s%c%s%c%d%c%c",
                                        SOI, OUR_TO_CLIENT_INDEX(leaseCnt), FSP, leaseInfo[leaseCnt].assignIpAddr, FSP,
                                        leaseInfo[leaseCnt].clientMacAddr, FSP, leaseInfo[leaseCnt].clientHostname, FSP,
                                        (UINT32)leaseInfo[leaseCnt].leaseExpireTime, FSP, EOI);
                if (sizeWritten >= MAX_REPLY_SZ)
                {
                    /* Buffer is small to add message data */
                    EPRINT(NETWORK_MANAGER, "buffer is small to add msg data: [leaseCnt=%d], [ip=%s]", leaseCnt, leaseInfo[leaseCnt].assignIpAddr);
                    break;
                }
            }
        }

        /* Add response message footer */
        sizeWritten += snprintf(pRespStr + sizeWritten, MAX_REPLY_SZ - sizeWritten, "%c", EOM);
        if (sizeWritten >= MAX_REPLY_SZ)
        {
            /* Buffer is small to add message data */
            EPRINT(NETWORK_MANAGER, "buffer is small to add msg footer");
            break;
        }

        /* Send positive reply to client */
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType], sizeWritten, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while (0);

    /* Send negative reply to client */
    clientCmdRespCb[clientCbType](CMD_PROCESS_ERROR, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the camera capability information
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetCapabilityCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    UINT32              sizeWritten;
    UINT64              tempData;
    UINT8               camIndex;
    CAPABILITY_CMD_ID_e reqCapCmd;
    CAPABILITY_TYPE     capability = 0;

    do
    {
        /* Parse the camera number */
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse camera");
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Get the camera index */
        camIndex = (UINT8)GET_CAMERA_INDEX(tempData);

        /* Parse the required capability index */
        if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse capability cmd index: [camera=%d]", camIndex);
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Get the capability command */
        reqCapCmd = (UINT8)tempData;
        switch(reqCapCmd)
        {
            case CAPABILITY_CMD_ID_TEXT_OVERLAY:
            {
                /* Get the max text overlay supported by camera */
                capability = GetSupportedTextOverlay(camIndex);
            }
            break;

            case CAPABILITY_CMD_ID_IMAGING_CAPABILITY:
            {
                /* Get image setting capability supported by camera */
                cmdResp = GetImagingCapability(camIndex, &capability);
            }
            break;

            /* Get Media 2 support */
            case CAPABILITY_CMD_ID_ONVIF_MEDIA2_SUPPORT:
            {
                capability = GetOnvifMedia2SupportCapability(camIndex);
            }
            break;

            default:
            {
                EPRINT(NETWORK_MANAGER, "invld capability command: [camera=%d], [capCmd=%d]", camIndex, reqCapCmd);
                cmdResp = CMD_PROCESS_ERROR;
            }
            break;
        }

        /* If error found then send error response */
        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        /* Prepare response string */
        sizeWritten = snprintf(&replyMsg[clientCbType][0], MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d%c%d%c%c%c",
                SOM, headerReq[RPL_CMD], FSP, cmdResp, FSP, SOI, 1, FSP, reqCapCmd, FSP, capability, FSP, EOI, EOM);
        if (sizeWritten >= MAX_REPLY_SZ)
        {
            cmdResp = CMD_MAX_BUFFER_LIMIT;
            break;
        }

        /* Send response string and close socket */
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType][0], sizeWritten, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    /* Send negative reply to client */
    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief EnablePushNotificationCmd
 * @param pCmdStr
 * @param clientCbType
 * @param clientSocket
 * @param sessionIndex
 * @return
 */
static BOOL EnablePushNotificationCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_INVALID_SYNTAX;
    UINT64 					notificationEnableF = FALSE;
    CHAR 					fcmToken[FCM_TOKEN_LENGTH_MAX];
    CHAR                    deviceModelName[FCM_DEVICE_MODEL_NAME_LEN_MAX];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

        if (userAccountConfig.managePushNotificationRights == DISABLE)
        {
            EPRINT(NETWORK_MANAGER, "no user rights to manage push notification: [username=%s]", userAccountConfig.username);
            cmdResp = CMD_PUSH_NOTIFICATION_NOT_ALLOWED;
            break;
        }

        /* enable/disable flag */
        if (ParseStringGetVal(pCmdStr, &notificationEnableF, 1, FSP) == FAIL)
        {
            break;
        }

        /* device fcm token */
        if (ParseStr(pCmdStr, FSP, fcmToken, FCM_TOKEN_LENGTH_MAX) == FAIL)
        {
            break;
        }

        /* device model name */
        if (ParseNStr(pCmdStr, FSP, deviceModelName, FCM_DEVICE_MODEL_NAME_LEN_MAX) == FAIL)
        {
            break;
        }

        if (('\0' == fcmToken[0]) || ('\0' == deviceModelName[0]))
        {
            EPRINT(NETWORK_MANAGER, "invalid fcm token or device model name");
            cmdResp = CMD_PUSH_NOTIFICATION_NOT_ALLOWED;
            break;
        }

        cmdResp = ManagePushNotificationSts(sessionIndex, (BOOL)notificationEnableF, fcmToken, deviceModelName);

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetPushNotificationStatusCmd
 * @param pCmdStr
 * @param clientCbType
 * @param clientSocket
 * @param sessionIndex
 * @return
 */
static BOOL GetPushNotificationStatusCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_PUSH_NOTIFICATION_DISABLED;
    CHAR 					fcmToken[FCM_TOKEN_LENGTH_MAX] = "";
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

    do
    {
        /* device fcm token */
        if (ParseStr(pCmdStr, FSP, fcmToken, FCM_TOKEN_LENGTH_MAX) == FAIL)
        {
            break;
        }

        if ('\0' == fcmToken[0])
        {
            EPRINT(NETWORK_MANAGER, "fcm token not found for push notification sts: [username=%s]", userAccountConfig.username);
            break;
        }

        /* get the notification flag value */
        cmdResp = GetDevicePushNotificationEnableSts(fcmToken);

    } while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetPushNotificationDevListCmd
 * @param pCmdStr
 * @param clientCbType
 * @param clientSocket
 * @param sessionIndex
 * @return
 */
static BOOL GetPushNotificationDevListCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT8                       deviceCnt;
    CHARPTR                     pRespStr = &replyMsg[clientCbType][0];
    UINT32                      sizeWritten;
    PUSH_NOTIFY_DEVICE_STS_LIST_t deviceList[FCM_PUSH_NOTIFY_DEVICES_MAX];

    do
    {
        /* Add response message header */
        sizeWritten = snprintf(pRespStr, MAX_REPLY_SZ, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);

        if (sizeWritten >= MAX_REPLY_SZ)
        {
            /* Buffer is small to add message header */
            EPRINT(NETWORK_MANAGER, "buffer is small to add msg header");
            break;
        }

        /* Get all device status list */
        if (CMD_SUCCESS != GetPushNotificationDeviceStsList(deviceList))
        {
            break;
        }

        for (deviceCnt = 0; deviceCnt < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceCnt++)
        {
            /* Add response message data */
            sizeWritten += snprintf(pRespStr + sizeWritten, MAX_REPLY_SZ - sizeWritten, "%c%d%c%s%c%s%c%s%c%d%c%c",
                                    SOI, OUR_TO_CLIENT_INDEX(deviceCnt), FSP, deviceList[deviceCnt].username, FSP,
                                    deviceList[deviceCnt].deviceFcmToken, FSP, deviceList[deviceCnt].deviceModelName, FSP,
                                    deviceList[deviceCnt].deviceInactivityTimer, FSP, EOI);

            if (sizeWritten >= MAX_REPLY_SZ)
            {
                /* Buffer is small to add message data */
                EPRINT(NETWORK_MANAGER, "buffer is small to add msg data: [device=%d]", deviceCnt);
                break;
            }
        }

        /* Add response message footer */
        sizeWritten += snprintf(pRespStr + sizeWritten, MAX_REPLY_SZ - sizeWritten, "%c", EOM);
        if (sizeWritten >= MAX_REPLY_SZ)
        {
            /* Buffer is small to add message data */
            EPRINT(NETWORK_MANAGER, "buffer is small to add msg footer");
            break;
        }

        /* Send positive reply to client */
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)&replyMsg[clientCbType], sizeWritten, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while (0);

    /* Send negative reply to client */
    clientCmdRespCb[clientCbType](CMD_PROCESS_ERROR, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetPushNotificationDevListCmd
 * @param pCmdStr
 * @param clientCbType
 * @param clientSocket
 * @param sessionIndex
 * @return
 */
static BOOL DeletePushNotificationDevCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e		cmdResp = CMD_PROCESS_ERROR;
    CHAR 					fcmToken[FCM_TOKEN_LENGTH_MAX];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);
        if (userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

        /* device fcm token */
        if (ParseStr(pCmdStr, FSP, fcmToken, FCM_TOKEN_LENGTH_MAX) == FAIL)
        {
            break;
        }

        /* delete push notification device */
        cmdResp = DeletePushNotificationDevice(fcmToken);

    } while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process test email command and send email on provided email address
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL TestEmailIdCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e    cmdResp;
    SMTP_CONFIG_t       smtpConfig;
    CHAR                testEmailId[PWD_RECOVERY_EMAIL_ID_LEN_MAX];

    do
    {
        /* getting email-ID from payload */
        if (ParseStr(pCmdStr, FSP, testEmailId, PWD_RECOVERY_EMAIL_ID_LEN_MAX) == FAIL)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse cmd str");
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        ReadSmtpConfig(&smtpConfig);

        /* Check Email service Configuration */
        if ((smtpConfig.smtp == DISABLE) || (smtpConfig.server[0] == '\0') || (smtpConfig.senderAddress[0] == '\0')
                || (smtpConfig.username[0] == '\0') || (smtpConfig.password[0] == '\0'))
        {
            DPRINT(NETWORK_MANAGER, "mail server not configured");
            cmdResp = CMD_EMAIL_SERVICE_DISABLED;
            break;
        }

        /* Providing email-id & smtp config to sent test mail */
        cmdResp = SendTestEmail(testEmailId, &smtpConfig, clientCmdRespCb[clientCbType], clientSocket);
        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        /* Test email sent to "EMAIL" thread, Response to client will be sent from the same */
        return SUCCESS;

    }while(0);

    /* test email sending failed, Hence sending command response to the client */
    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will send password recovery config of user
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetPasswordResetInfoCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT32                      outLen;
    CHARPTR                     pRespStr = &replyMsg[clientCbType][0];
    PASSWORD_RECOVERY_CONFIG_t 	pwdRecoveryCfg;

    do
    {
        /* Get user's password recovery config */
        ReadSinglePasswordRecoveryConfig(GetUserAccountIndex(sessionIndex), &pwdRecoveryCfg);

        /* Prepare response message */
        outLen = snprintf(pRespStr, MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%s%c%d%c%s%c%d%c%s%c%d%c%s%c%c%c",
                          SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, pwdRecoveryCfg.emailId, FSP,
                          pwdRecoveryCfg.securityQuestionInfo[0].questionId, FSP, pwdRecoveryCfg.securityQuestionInfo[0].answer, FSP,
                          pwdRecoveryCfg.securityQuestionInfo[1].questionId, FSP, pwdRecoveryCfg.securityQuestionInfo[1].answer, FSP,
                          pwdRecoveryCfg.securityQuestionInfo[2].questionId, FSP, pwdRecoveryCfg.securityQuestionInfo[2].answer, FSP, EOI, EOM);
        if (outLen >= MAX_REPLY_SZ)
        {
            EPRINT(NETWORK_MANAGER, "buffer is small to add msg data");
            break;
        }

        /* Sending positive response to client with payload */
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)pRespStr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    /* Send negative reply to client */
    clientCmdRespCb[clientCbType](CMD_MAX_BUFFER_LIMIT, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will set password recovery config of user
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL SetPasswordResetInfoCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    PASSWORD_RECOVERY_CONFIG_t  pwdRecoveryCfg;
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    UINT32                      secQueCnt;
    UINT64                      tempData;

    do
    {
        /* Read user's password recovery config */
        ReadSinglePasswordRecoveryConfig(GetUserAccountIndex(sessionIndex), &pwdRecoveryCfg);

        /* getting email-ID */
        if (ParseStr(pCmdStr, FSP, pwdRecoveryCfg.emailId, PWD_RECOVERY_EMAIL_ID_LEN_MAX) == FAIL)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse email-id");
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* getting security questions info */
        for (secQueCnt = 0; secQueCnt < PWD_RECOVERY_SECURITY_QUESTION_MAX; secQueCnt++)
        {
            /* Parse security question id */
            if (ParseStringGetVal(pCmdStr, &tempData, 1, FSP) == FAIL)
            {
                EPRINT(NETWORK_MANAGER, "fail to parse security question id: [secQueCnt=%d]", secQueCnt);
                cmdResp = CMD_INVALID_SYNTAX;
                break;
            }

            /* if 2 question IDs are same */
            if ((secQueCnt != 0) && (pwdRecoveryCfg.securityQuestionInfo[secQueCnt-1].questionId == tempData))
            {
                EPRINT(NETWORK_MANAGER, "two security question id are same");
                cmdResp = CMD_INVALID_SYNTAX;
                break;
            }

            /* Parse security answer */
            pwdRecoveryCfg.securityQuestionInfo[secQueCnt].questionId = tempData;
            if (ParseStr(pCmdStr, FSP, pwdRecoveryCfg.securityQuestionInfo[secQueCnt].answer, PWD_RECOVERY_ANSWER_LEN_MAX) == FAIL)
            {
                EPRINT(NETWORK_MANAGER, "fail to parse security answer: [secQueCnt=%d]", secQueCnt);
                cmdResp = CMD_INVALID_SYNTAX;
                break;
            }
        }

        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        /* Update user's password recovery info in config */
        WriteSinglePasswordRecoveryConfig(GetUserAccountIndex(sessionIndex), &pwdRecoveryCfg);

        /* Adding entry in Event Log */
        WriteConfigChangeEvent(sessionIndex, TBL_PASSWORD_RECOVERY_CFG, EVENT_CHANGE);

    } while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will send manual backup loaction of user
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return
 */
static BOOL GetManualBackupLocationCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    UINT32                      outLen;
    CHARPTR                     pRespStr = &replyMsg[clientCbType][0];
    MANUAL_BACKUP_CONFIG_t      manualBackupCfg;

    do
    {
        /* Get Manual Backup Location */
        ReadManualBackupConfig(&manualBackupCfg);

        /* Prepare response message */
        outLen = snprintf(pRespStr, MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d%c%c%c",
                          SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, manualBackupCfg.backupLocation, FSP, EOI, EOM);
        if (outLen >= MAX_REPLY_SZ)
        {
            EPRINT(NETWORK_MANAGER, "buffer is small to add msg data");
            break;
        }

        /* Sending positive response to client with payload */
        sendCmdCb[clientCbType](clientSocket, (UINT8PTR)pRespStr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&clientSocket);
        return SUCCESS;

    }while(0);

    /* Send negative reply to client */
    clientCmdRespCb[clientCbType](CMD_MAX_BUFFER_LIMIT, clientSocket, TRUE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will validate user's credentials. Like, username, password, User Type, etc.
 * @param   pCmdStr
 * @param   clientCbType
 * @param   clientSocket
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL ValidateUserCredentialCmd(CHARPTR *pCmdStr, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket, UINT8 sessionIndex)
{
    NET_CMD_STATUS_e 		cmdResp = CMD_SUCCESS;
    UINT8					userIndex;
    CHAR					username[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR					password[MAX_USER_ACCOUNT_PASSWORD_WIDTH];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

    do
    {
        if (ParseStr(pCmdStr, FSP, username, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if (ParseStr(pCmdStr, FSP, password, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        for (userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if (strcmp(userAccountConfig.username, username) == STATUS_OK)
            {
                break;
            }
        }

        if (userIndex >= MAX_USER_ACCOUNT)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if (strcmp(userAccountConfig.password, password) != STATUS_OK)
        {
            cmdResp = CMD_CRED_INVALID;
            break;
        }

        if (userAccountConfig.userGroup != ADMIN)
        {
            cmdResp = CMD_NO_PRIVILEGE;
            break;
        }

    }while(0);

    clientCmdRespCb[clientCbType](cmdResp, clientSocket, TRUE);
    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
