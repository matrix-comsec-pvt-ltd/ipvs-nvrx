//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   ConfigFileIO.c
@brief  The module is designed to manage the configuration parameters of the device. It provides
        InitXXXConfig, DefaultXXXConfig, DefaultSingleXXXConfig (only for multiple element files),
        writeXXXConfig, WriteSingleXXXConfig (only for multiple element files), readXXXConfig functions
        to perform initialization, set to default, single set to default, write, single write and read
        operation respectively. XXX in function names mentioned above indicates particular configuration.
        In actual coding XXX will be replaced by name of configuration, i.e. ReadGeneralConfig,
        WriteCameraConfig, LoadDefaultLan1Config. Logic for different configurations remains the same.
        Only the data structures are different. Local master copy of individual configurations is kept in
        the module; they serve as main working copy. Configuration files maintain backup of all configuration
        parameters, whenever configurations are changed, files are updated as well. User modules also have
        to keep their local user copies of interest. Any read / write operation will be done through the
        master copy only. Each configuration is given version number. In the file, version number is stored
        at the beginning followed by configuration values and for local copy it is defined as macro. Purpose
        of version is to avoid the conflict between configuration structures of local copy and file.
        InitXXXConfig function should be called at the start-up of the program. It compares the version of
        file with the version of local copy. If they match, operation is carried on; master copy is updated
        with content of the file. Otherwise default values are loaded into master copy and to the file.
        DefaultXXXConfig function loads the default values to the master copy and updates the same in the file.
        DefaultSingleXXXConfig function loads the default values to the particular element of master copy
        and same in the file. writeXXXConfig function receives a user copy and updates the master copy
        and file with the content of user copy. WriteSingleXXXConfig function receives a user copy and
        index of the element to be edited. Element specified by index is updated in master copy and in
        the file with content of user copy. readXXXConfig function outputs the values of master copy to
        the user copy.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdarg.h>
#include <stdint.h>

/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "Config.h"
#include "NetworkManager.h"
#include "ViewerUserSession.h"
#include "ConfigOld.h"
#include "DhcpServer.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define CONFIG_FILE_EXT ".cfg"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    CONFIG_NO_DIMENSION = 0,
	CONFIG_ONE_DIMENSION,
	CONFIG_TWO_DIMENSION,

}CONFIG_DIMENSION_e;

typedef struct
{
	CHARPTR 			fileName;
	UINT8				version;
    UINT8				type;       // No Dimension, 1D or 2D

    VOIDPTR 			memPtr;     // Pointer to memory where configuration is stored

    // size of basic element, e.g for camera event its a 2D configuration, these value should be sizeof(CAMERA_EVENT_CONFIG_t)
	UINT16				size;
	UINT8 				maxXIndx;
	UINT8				maxYIndx;

	// Pointer to default configuration copy
	VOIDPTR				dfltCnfgPtr;

}CONFIG_PARAM_t;

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL initXXXConfig(UINT8 configId);
//-------------------------------------------------------------------------------------------------
static void makeConfigModelCompatibile(UINT8 configId);
//-------------------------------------------------------------------------------------------------
static BOOL isConfigCameraDependent(CONFIG_INDEX_e configId);
//-------------------------------------------------------------------------------------------------
static void decomposeQuery(UINT8 configId, va_list list, UINT8PTR query, UINT8PTR xIndex, UINT8PTR yIndex);
//-------------------------------------------------------------------------------------------------
static BOOL getConfigOffsetAndSize(UINT8 configId, UINT8 query, UINT8 xIndex, UINT8 yIndex, UINT32PTR offset, UINT32PTR configSize);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e dfltXXXConfig(UINT8 configId, ...);
//-------------------------------------------------------------------------------------------------
static BOOL readXXXConfig(UINT8 configId, VOIDPTR userCopy, ...);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e writeXXXConfig(UINT8 configId, VOIDPTR userCopy, ...);
//-------------------------------------------------------------------------------------------------
static BOOL dfltXXXConfigInternal(UINT8 configId, BOOL systemInit, UINT8 query, UINT8 xIndex, UINT8 yIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e	writeXXXConfigInternal(UINT8 configId, BOOL systemInit, VOIDPTR userCopy, UINT8 query, UINT8 xIndex, UINT8 yIndex);
//-------------------------------------------------------------------------------------------------
static BOOL modifyDfltXXXConfigCopy(UINT8 configId);
//-------------------------------------------------------------------------------------------------
static void prepDfltXXXConfig(UINT8 configId, VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 xIndex, UINT8 yIndex);
//-------------------------------------------------------------------------------------------------
static void prepDfltSingleUserAccCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 userIndex);
//-------------------------------------------------------------------------------------------------
static void prepDfltSingleCameraCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static void prepDfltSingleSensorCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 xIndex);
//-------------------------------------------------------------------------------------------------
static void prepDfltDhcpServerCfg(VOIDPTR userCopy, VOIDPTR dfltCopy);
//-------------------------------------------------------------------------------------------------
static void prepDfltSingleAlarmCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 xIndex);
//-------------------------------------------------------------------------------------------------
static void prepDfltSinglePresetTourCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void prepDfltSingleImageSettingCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e validateSingleUserAccCfg(USER_ACCOUNT_CONFIG_t *newUsrAccontCfg, USER_ACCOUNT_CONFIG_t * oldUsrAccontCfg, UINT8 userIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e validateXXXConfigInternal(UINT8 configId, VOIDPTR newCopy, VOIDPTR oldCopy, UINT8 xIndex, UINT8 yIndex);
//-------------------------------------------------------------------------------------------------
static void notifyXXXConfigInternal(UINT8 configId, VOIDPTR newCopy, VOIDPTR oldCopy,UINT8 xIndex, UINT8 yIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @GLOBAL VARIABLE
//#################################################################################################
/* This variable is used to default all configuration related to camera if camera brand in previous configuration is found other than matrix. */
BOOL cameraConfigDefaultF[MAX_CAMERA];

GENERAL_CONFIG_t				generalCfg;
DATE_TIME_CONFIG_t				dateTimeCfg;
DST_CONFIG_t					dstCfg;
LAN_CONFIG_t					lanCfg[MAX_LAN_PORT];
MAC_SERVER_CNFG_t				macServerCfg;
IP_FILTER_CONFIG_t				ipFilterCfg;
DDNS_CONFIG_t					ddnsCfg;
SMTP_CONFIG_t					smtpCfg;
FTP_UPLOAD_CONFIG_t				ftpUploadCfg[MAX_FTP_SERVER];
TCP_NOTIFY_CONFIG_t				tcpNotifyCfg;
FILE_ACCESS_CONFIG_t			fileAccessCfg;
HDD_CONFIG_t					hddCfg;
MATRIX_DNS_SERVER_CONFIG_t		matrixDnsServerCfg;
USER_ACCOUNT_CONFIG_t			userAccountCfg[MAX_USER_ACCOUNT];
CAMERA_CONFIG_t					cameraCfg[MAX_CAMERA];
STREAM_CONFIG_t					streamCfg[MAX_CAMERA];
SCHEDULE_RECORD_CONFIG_t		scheduleRecordCfg[MAX_CAMERA];
ALARM_RECORD_CONFIG_t			alarmRecordCfg[MAX_CAMERA];
PTZ_PRESET_CONFIG_t				ptzPresetCfg[MAX_CAMERA][MAX_PRESET_POSITION];
PRESET_TOUR_CONFIG_t			presetTourCfg[MAX_CAMERA];
TOUR_SCHEDULE_CONFIG_t			tourScheduleCfg[MAX_CAMERA];
IMAGE_UPLOAD_CONFIG_t			imageUploadCfg[MAX_CAMERA];
SENSOR_CONFIG_t					sensorCfg[MAX_SENSOR];
ALARM_CONFIG_t					alarmCfg[MAX_ALARM];
STORAGE_CONFIG_t				storageCfg;
SCHEDULE_BACKUP_CONFIG_t		scheduleBackupCfg;
MANUAL_BACKUP_CONFIG_t			manualBackupCfg;
CAMERA_ALARM_CONFIG_t			cameraAlarmCfg[MAX_CAMERA][MAX_CAMERA_ALARM];
CAMERA_EVENT_CONFIG_t			cameraEventCfg[MAX_CAMERA][MAX_CAMERA_EVENT];
SENSOR_EVENT_CONFIG_t			sensorEventCfg[MAX_SENSOR_EVENT];
SYSTEM_EVENT_CONFIG_t			systemEventCfg[MAX_SYSTEM_EVENT];
COSEC_REC_PARAM_CONFIG_t		cosecPreRecCfg[MAX_CAMERA];
STATIC_ROUTING_CONFIG_t			staticRoutingCfg;
BROAD_BAND_CONFIG_t				broadBandCfg;
SMS_CONFIG_t					smsCfg;
MANUAL_RECORD_CONFIG_t			manualRecordCfg[MAX_CAMERA];
NETWORK_DRIVE_CONFIG_t			nwDriveCfg[MAX_NW_DRIVE];
IP_CAMERA_CONFIG_t 				ipCameraCfg[MAX_CAMERA];
NETWORK_DEVICE_CONFIG_t			networkDeviceCfg[MAX_NETWORK_DEVICES];
SNAPSHOT_CONFIG_t 				snapShotCfg[MAX_CAMERA];
SNAPSHOT_SCHEDULE_CONFIG_t   	snapShotSchdCfg[MAX_CAMERA];
LOGIN_POLICY_CONFIG_t			loginPolicyCfg;
AUDIO_OUT_CONFIG_t              audioOutCfg;
P2P_CONFIG_t                    p2pCfg;
IMG_SETTING_CONFIG_t            imageSettingCfg[MAX_CAMERA];
DHCP_SERVER_CONFIG_t            dhcpServerCfg;
FIRMWARE_MANAGEMENT_CONFIG_t    firmwareManagementCfg;
FCM_PUSH_NOTIFY_CONFIG_t        fcmPushNotificationCfg[FCM_PUSH_NOTIFY_DEVICES_MAX];
PASSWORD_RECOVERY_CONFIG_t      passwordRecoveryCfg[MAX_USER_ACCOUNT];
STORAGE_ALLOCATION_CONFIG_t     storageAllocationCfg;

//#################################################################################################
// @STATIC VARIABLE
//#################################################################################################
// Configuration Protection Lock for all Configuration
static pthread_rwlock_t 		rwLock[MAX_CONFIG_ID];

static CONFIG_PARAM_t configParam[MAX_CONFIG_ID] =
{
    // 1 - fileName;
    // 2 - version;
    // 3 - type;
    // 4 - memPtr;
    // 5 - size;
    // 6 - maxXIndx;
    // 7 - maxYIndx;
    // 8 - dfltCnfgPtr;

    // GENERAL_CONFIG_ID = 0,               /* 0: Table 1 */
    {
        "General",
        GENERAL_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &generalCfg,
        sizeof(GENERAL_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltGeneralCfg,
    },
    // DATE_TIME_CONFIG_ID,					/* 1: Table 2 */
    {
        "DateTime",
        DATE_TIME_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &dateTimeCfg,
        sizeof(DATE_TIME_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltDateTimeCfg,
    },
    // DST_CONFIG_ID,						/* 2: Table 3 */
    {
        "Dst",
        DST_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &dstCfg,
        sizeof(DST_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltDstCfg,
    },
    // LAN1_CONFIG_ID,						/* 3: Table 5 */
    {
        "Lan1",
        LAN_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &lanCfg[LAN1_PORT],
        sizeof(LAN_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltLanCfg[LAN1_PORT],
    },
    // LAN2_CONFIG_ID,						/* 4: Table 6 */
    {
        "Lan2",
        LAN_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &lanCfg[LAN2_PORT],
        sizeof(LAN_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltLanCfg[LAN2_PORT],
    },
    // MAC_SERVER_CONFIG_ID,				/* 5: No Table */
    {
        "MacServer",
        MAC_SERVER_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &macServerCfg,
        sizeof(MAC_SERVER_CNFG_t),
        0,
        0,
        (VOIDPTR)&DfltMacServerCfg,
    },
    // IP_FILTER_CONFIG_ID,					/* 6: Table 7 & 8 */
    {
        "IpFilter",
        IP_FILTER_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &ipFilterCfg,
        sizeof(IP_FILTER_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltIpFilterCfg,
    },
    // DDNS_CONFIG_ID,						/* 7: Table 9 */
    {
        "Ddns",
        DDNS_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &ddnsCfg,
        sizeof(DDNS_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltDdnsCfg,
    },
    // SMTP_CONFIG_ID,						/* 8: Table 10 */
    {
        "Smtp",
        SMTP_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &smtpCfg,
        sizeof(SMTP_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltSmtpCfg,
    },
    // FTP_UPLOAD_CONFIG_ID,				/* 9: Table 11 */
    {
        "FtpUpload",
        FTP_UPLOAD_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &ftpUploadCfg,
        sizeof(FTP_UPLOAD_CONFIG_t),
        MAX_FTP_SERVER,
        0,
        (VOIDPTR)&DfltFtpUploadCfg,
    },
    // TCP_NOTIFY_CONFIG_ID,				/* 10: Table 12 */
    {
        "TcpNotify",
        TCP_NOTIFY_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &tcpNotifyCfg,
        sizeof(TCP_NOTIFY_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltTcpNotifyCfg,
    },
    // FILE_ACCESS_CONFIG_ID,				/* 11: Table 13 */
    {
        "FileAccess",
        FILE_ACCESS_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &fileAccessCfg,
        sizeof(FILE_ACCESS_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltFileAccessCfg,
    },
    // HDD_CONFIG_ID,						/* 12: Table 14 */
    {
        "Hdd",
        HDD_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &hddCfg,
        sizeof(HDD_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltHddCfg,
    },
    // MATRIX_DNS_SERVER_CONFIG_ID,         /* 13: Table 15 */
    {
        "MatrixDnsServer",
        MATRIX_DNS_SERVER_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &matrixDnsServerCfg,
        sizeof(MATRIX_DNS_SERVER_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltMatrixDnsServerCfg,
    },
    // USER_ACCOUNT_CONFIG_ID,				/* 14: Table 16 */
    {
        "UserAccount",
        USER_ACCOUNT_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &userAccountCfg,
        sizeof(USER_ACCOUNT_CONFIG_t),
        MAX_USER_ACCOUNT,
        0,
        (VOIDPTR)&DfltUserAccountCfg,
    },
    // CAMERA_CONFIG_ID,					/* 15: Table 17 */
    {
        "Camera",
        CAMERA_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &cameraCfg,
        sizeof(CAMERA_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltCameraCfg,
    },
    // 	STREAM_CONFIG_ID,					/* 16: Table 18 */
    {
        "Stream",
        STREAM_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &streamCfg,
        sizeof(STREAM_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltCameraStreamCfg,
    },
    // SCHEDULE_RECORD_CONFIG_ID,  			/* 17: Table 19 & 20 */
    {
        "ScheduleRecord",
        SCHEDULE_RECORD_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &scheduleRecordCfg,
        sizeof(SCHEDULE_RECORD_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltScheduleRecordCfg,
    },
    // ALARM_RECORD_CONFIG_ID,				/* 18: Table 21 */
    {
        "AlarmRecord",
        ALARM_RECORD_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &alarmRecordCfg,
        sizeof(ALARM_RECORD_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltAlarmRecordCfg,
    },
    // PTZ_PRESET_CONFIG_ID,				/* 19: Table 22 */
    {
        "PtzPreset",
        PTZ_PRESET_CONFIG_VERSION,
        CONFIG_TWO_DIMENSION,
        &ptzPresetCfg,
        sizeof(PTZ_PRESET_CONFIG_t),
        MAX_CAMERA,
        MAX_PRESET_POSITION,
        (VOIDPTR)&DfltPtzPresetCfg,
    },
    // PRESET_TOUR_CONFIG_ID,				/* 20: Table 23 & 24 */
    {
        "PresetTour",
        PRESET_TOUR_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &presetTourCfg,
        sizeof(PRESET_TOUR_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltPresetTourCfg,
    },
    // TOUR_SCHEDULE_CONFIG_ID,				/* 21: Table 25 */
    {
        "TourSchedule",
        TOUR_SCHEDULE_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &tourScheduleCfg,
        sizeof(TOUR_SCHEDULE_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltTourScheduleCfg,
    },
    // IMAGE_UPLOAD_CONFIG_ID,				/* 22: Table 26 */
    {
        "ImageUpload",
        IMAGE_UPLOAD_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &imageUploadCfg,
        sizeof(IMAGE_UPLOAD_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltImageUploadCfg,
    },
    // SENSOR_CONFIG_ID,					/* 23: Table 27 */
    {
        "Sensor",
        SENSOR_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &sensorCfg,
        sizeof(SENSOR_CONFIG_t),
        MAX_SENSOR,
        0,
        (VOIDPTR)&DfltSensorCfg,
    },
    // ALARM_CONFIG_ID,						/* 24: Table 28 */
    {
        "Alarm",
        ALARM_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &alarmCfg,
        sizeof(ALARM_CONFIG_t),
        MAX_ALARM,
        0,
        (VOIDPTR)&DfltAlarmCfg,
    },
    // STORAGE_CONFIG_ID,					/* 25: Table 29 */
    {
        "Storage",
        STORAGE_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &storageCfg,
        sizeof(STORAGE_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltStorageCfg,
    },
    // SCHEDULE_BACKUP_CONFIG_ID,			/* 26: Table 30 */
    {
        "ScheduleBackup",
        SCHEDULE_BACKUP_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &scheduleBackupCfg,
        sizeof(SCHEDULE_BACKUP_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltScheduleBackupCfg,
    },
    // MANUAL_BACKUP_CONFIG_ID,				/* 27: Table 31 */
    {
        "ManualBackup",
        MANUAL_BACKUP_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &manualBackupCfg,
        sizeof(MANUAL_BACKUP_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltManualBackupCfg,
    },
    // CAMERA_EVENT_ACTION_CONFIG_ID, 		/* 28: Table 32 & 33 */
    {
        "CameraEventAction",
        CAMERA_EVENT_CONFIG_VERSION,
        CONFIG_TWO_DIMENSION,
        &cameraEventCfg,
        sizeof(CAMERA_EVENT_CONFIG_t),
        MAX_CAMERA,
        MAX_CAMERA_EVENT,
        (VOIDPTR)&DfltCameraEventActionCfg,
    },
    // SENSOR_EVENT_ACTION_CONFIG_ID,		/* 29: Table 34 & 35 */
    {
        "SensorEventAction",
        SENSOR_EVENT_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &sensorEventCfg,
        sizeof(SENSOR_EVENT_CONFIG_t),
        MAX_SENSOR_EVENT,
        0,
        (VOIDPTR)&DfltSensorEventActionCfg,
    },
    // SYSTEM_EVENT_ACTION_CONFIG_ID,       /* 30: Table 36 */
    {
        "SystemEventAction",
        SYSTEM_EVENT_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &systemEventCfg,
        sizeof(SYSTEM_EVENT_CONFIG_t),
        MAX_SYSTEM_EVENT,
        0,
        (VOIDPTR)&DfltSystemEventActionCfg,
    },
    // COSEC_PRE_RECORD_SETTINGS,           /* 31: Table 37 */
    {
        "CosecPreRecord",
        COSEC_PREALRM_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &cosecPreRecCfg,
        sizeof(COSEC_REC_PARAM_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltCosecPreRecConfig,
    },
    // CAMERA_ALARM_CONFIG_ID               /* 32: Table 38 */
    {
        "CameraAlarm",
        CAMERA_ALARM_CONFIG_VERSION,
        CONFIG_TWO_DIMENSION,
        &cameraAlarmCfg,
        sizeof(CAMERA_ALARM_CONFIG_t),
        MAX_CAMERA,
        MAX_CAMERA_ALARM,
        (VOIDPTR)&DfltCameraAlarmCfg,
    },
    // STATIC_ROUTING_CONFIG_ID,			/* 33: Table 39 & 40 */
    {
        "StaticRouting",
        STATIC_ROUTING_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &staticRoutingCfg,
        sizeof(STATIC_ROUTING_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltStaticRoutingCfg,
    },
    // BROAD_BAND_CONFIG_ID,				/* 34: Table 41 & 42 */
    {
        "BroadBand",
        BROAD_BAND_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &broadBandCfg,
        sizeof(BROAD_BAND_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltBroadBandCfg,
    },
    // SMS_CONFIG_ID,						/* 35: Table 43 */
    {
        "Sms",
        SMS_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &smsCfg,
        sizeof(SMS_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltSmsCfg,
    },
    // MANUAL_RECORD_CONFIG_ID,				/* 36: Table 44 */
    {
        "ManualRecord",
        MANUAL_RECORD_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &manualRecordCfg,
        sizeof(MANUAL_RECORD_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltManualRecordCfg,
    },
    // NETWORK_DRIVE_SETTINGS,				/* 37: Table 45 */
    {
        "NwDrive",
        NETWORK_DRIVE_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &nwDriveCfg,
        sizeof(NETWORK_DRIVE_CONFIG_t),
        MAX_NW_DRIVE,
        0,
        (VOIDPTR)&DfltNetworkDriveCfg,
    },
    // IP_CAMERA_CONFIG_ID,					/* 38: Table 46 */
    {
        "IpCamera",
        IP_CAMERA_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &ipCameraCfg,
        sizeof(IP_CAMERA_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltIpCameraCfg,
    },
    // NETWORK_DEVICE_CONFIG_ID			 	/* 39: Table 51 */
    {
        "NetworkDevice",
        NETWORK_DEVICE_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &networkDeviceCfg,
        sizeof(NETWORK_DEVICE_CONFIG_t),
        MAX_NETWORK_DEVICES,
        0,
        (VOIDPTR)&DfltNetworkDeviceCfg,
    },
    // SNAPSHOT_CONFIG_ID					/* 40: Table 52 */
    {
        "Snapshot",
        SNAPSHOT_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &snapShotCfg,
        sizeof(SNAPSHOT_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltSnapShotCfg,
    },
    // SNAPSHOT_SCHEDULE_CONFIG_ID          /* 41: Table 53 */
    {
        "SnapshotSchedule",
        SNAPSHOT_SCHEDULE_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &snapShotSchdCfg,
        sizeof(SNAPSHOT_SCHEDULE_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltSnapShotSchdCfg,
    },
    // LOGIN_POLICY_CONFIG_ID				/* 42: Table 54 */
    {
        "LoginPolicy",
        LOGIN_POLICY_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &loginPolicyCfg,
        sizeof(LOGIN_POLICY_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltLoginPolicyCfg,
    },
    // AUDIO_OUT_CONFIG_ID                  /* 43: Table 55 */
    {
        "AudioOut",
        AUDIO_OUT_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &audioOutCfg,
        sizeof(AUDIO_OUT_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltAudioOutCfg,
    },
    // P2P_CONFIG_ID                        /* 44: Table 56 */
    {
        "P2P",
        P2P_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &p2pCfg,
        sizeof(P2P_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltP2PCfg,
    },
    // IMAGE_SETTING_ID                     /* 45: Table 57 */
    {
        "ImageSetting",
        IMAGE_SETTING_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &imageSettingCfg,
        sizeof (IMG_SETTING_CONFIG_t),
        MAX_CAMERA,
        0,
        (VOIDPTR)&DfltImageSettingCfg,
    },
    // DHCP_SERVER_CONFIG_ID                /* 46: Table 58 */
    {
        "DhcpServer",
        DHCP_SERVER_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &dhcpServerCfg,
        sizeof(DHCP_SERVER_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltDhcpServerCfg,
    },
    // FIRMWARE_MANAGEMENT_CONFIG_ID        /* 47: Table 59 */
    {
        "FirmwareManagement",
        FIRMWARE_MANAGEMENT_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &firmwareManagementCfg,
        sizeof(FIRMWARE_MANAGEMENT_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltFirmwareManagementCfg,
    },
    // FCM_PUSH_NOTIFICATION_CONFIG_ID      /* 48: Table 60 */
    {
        "FcmPushNotification",
        FCM_PUSH_NOTIFY_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &fcmPushNotificationCfg,
        sizeof(FCM_PUSH_NOTIFY_CONFIG_t),
        FCM_PUSH_NOTIFY_DEVICES_MAX,
        0,
        (VOIDPTR)&DfltFcmPushNotificationCfg,
    },
    // PASSWORD_RECOVERY_CONFIG_ID,         /* 49: Table 61 */
    {
        "PasswordRecovery",
        PASSWORD_RECOVERY_CONFIG_VERSION,
        CONFIG_ONE_DIMENSION,
        &passwordRecoveryCfg,
        sizeof(PASSWORD_RECOVERY_CONFIG_t),
        MAX_USER_ACCOUNT,
        0,
        (VOIDPTR)&DfltPasswordRecoveryCfg,
    },
    // STORAGE_ALLOCATION_CONFIG_ID,        /* 50: Table 62 */
    {
        "StorageAllocation",
        STORAGE_ALLOCATION_CONFIG_VERSION,
        CONFIG_NO_DIMENSION,
        &storageAllocationCfg,
        sizeof(STORAGE_ALLOCATION_CONFIG_t),
        0,
        0,
        (VOIDPTR)&DfltStorageAllocationCfg,
    },
};

static const CHARPTR configQueryString[MAX_CONFIG_REQ_e] =
{
    "Whole Config",
    "Single Entry"
};

static const CHARPTR localConfigFileName[MAX_LOCAL_CONFIG_ID] =
{
    "AudioConfig",
    "DisplayConfig",
    "OtherLoginParam",
    "WizOtherLoginParam"
};

// This pointer is constant and used in evry configuration parameters
static const CONFIG_PARAM_t* const sysConfigParam = (CONFIG_PARAM_t *)configParam;

//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be called during start up of the program. It checks presence of
 *          configuration folder; if it doesn't exist, it creates one. Then it calls the initialization
 *          functions of individual configuration.
 */
void InitSysConfig(void)
{
    UINT8                       configId, cameraIndex, entryIdx;
    GENERAL_CONFIG_t            genCfg;
    NETWORK_DEVICE_CONFIG_t     networkDeviceCfg;

    for (cameraIndex = 0; cameraIndex < MAX_CAMERA; cameraIndex++)
    {
        cameraConfigDefaultF[cameraIndex] = FALSE;
    }

    /* This value should be modify only one time at bootup, maxIndexPerTable variable must not modify at any other place.
     * Runtime Detection and totalIndexPerEveryTable pointer will used in everyplace instead of maxIndexPerTable */
    for(configId = 0; configId < MAX_CONFIG_ID; configId++)
	{
        /* Is config camera dependent? */
        if (TRUE == isConfigCameraDependent(configId))
        {
            configParam[configId].maxXIndx = getMaxCameraForCurrentVariant();
        }
	}

    /* This should be not modify to any other place */
    DfltGeneralCfg.ipCamNo = getMaxCameraForCurrentVariant();

    /* Initialise all configuration */
    for(configId = 0; configId < MAX_CONFIG_ID; configId++)
	{
        initXXXConfig(configId);
    }

    /* Check if need to default camera configuration */
    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        if (FALSE == cameraConfigDefaultF[cameraIndex])
        {
            continue;
        }

        /* Default all camera related configuration because it is not compatible now */
        dfltXXXConfigInternal(CAMERA_CONFIG_ID, TRUE, SINGLE_ENTRY, cameraIndex, 0);
        dfltXXXConfigInternal(IP_CAMERA_CONFIG_ID, TRUE, SINGLE_ENTRY, cameraIndex, 0);
        dfltXXXConfigInternal(STREAM_CONFIG_ID, TRUE, SINGLE_ENTRY, cameraIndex, 0);
    }

    /* Read general configuration */
	ReadGeneralConfig(&genCfg);

    for (entryIdx = 0; entryIdx < MAX_NETWORK_DEVICES; entryIdx++)
    {
        /* read the remote device name from NetworkDevice config file */
        ReadSingleNetworkDeviceConfig(entryIdx, &networkDeviceCfg);
        if ((networkDeviceCfg.enable == DISABLE) || (networkDeviceCfg.deviceName[0] == '\0'))
        {
            continue;
        }

        /* compare the default device name with remote device name */
        if (strcmp(networkDeviceCfg.deviceName, DfltGeneralCfg.deviceName) == STATUS_OK)
        {
            /* if remote device name is same as default device name, disable the remote device */
            networkDeviceCfg.enable = DISABLE;

            /* write into the config file, TRUE flag indicates not to notify respective module at system init */
            writeXXXConfigInternal(NETWORK_DEVICES_CONFIG_ID, TRUE, &networkDeviceCfg, SINGLE_ENTRY, entryIdx, 0);
            continue;
        }

        /* compare the local device name with remote device name */
        if (strcmp(networkDeviceCfg.deviceName, genCfg.deviceName) == STATUS_OK)
        {
             /* if remote device name is same as local device name, update local device name as default device name */
            snprintf(genCfg.deviceName, MAX_DEVICE_NAME_WIDTH, "%s", DfltGeneralCfg.deviceName);

            /* write into the config file, TRUE flag indicates not to notify respective module at system init */
            writeXXXConfigInternal(GENERAL_CONFIG_ID, TRUE, &genCfg, WHOLE_CONFIG, 0, 0);
        }
    }

    /* if max camera for current model and written in configuration is mismatch */
    if (getMaxCameraForCurrentVariant() != genCfg.ipCamNo)
	{
        /* On boot up, We derive the model type using hardware GPIO pins and based on that, we derive the number of cameras
         * supported by the derived model. We also write number of cameras in general configuration (generalCfg.ipCamNo)
         * When board type is changed or config restore of other model then handle the camera config mismatch case. */
        EPRINT(CONFIGURATION, "ip camera config mismatch: [configured=%d], [modelCamera=%d]", genCfg.ipCamNo, getMaxCameraForCurrentVariant());

        /* Is model have more cameras then configured? */
        if (getMaxCameraForCurrentVariant() > genCfg.ipCamNo)
        {
            /* Default the remaining camera configuration */
            for (cameraIndex = genCfg.ipCamNo; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
            {
                /* Check all configuration id */
                for (configId = 0; configId < MAX_CONFIG_ID; configId++)
                {
                    /* Is config camera dependent? */
                    if (FALSE == isConfigCameraDependent(configId))
                    {
                        /* Nothing to do if config is not camera dependent */
                        continue;
                    }

                    /* Camera dependent config is one or two dimension type */
                    if (configParam[configId].type == CONFIG_ONE_DIMENSION)
                    {
                        /* Default the one dimension config */
                        dfltXXXConfigInternal(configId, TRUE, SINGLE_ENTRY, cameraIndex, 0);
                    }
                    else if (configParam[configId].type == CONFIG_TWO_DIMENSION)
                    {
                        /* Default the all two dimension config entries */
                        for (entryIdx = 0; entryIdx < configParam[configId].maxYIndx; entryIdx++)
                        {
                            dfltXXXConfigInternal(configId, TRUE, SINGLE_ENTRY, cameraIndex, entryIdx);
                        }
                    }
                }
            }
        }

        /* Update latest found cameras in general config */
        genCfg.ipCamNo = getMaxCameraForCurrentVariant();

        /* Write into the config file, TRUE flag indicates not to notify respective module at system init */
        writeXXXConfigInternal(GENERAL_CONFIG_ID, TRUE, &genCfg, WHOLE_CONFIG, 0, 0);
    }

    /* Update the config file before using it to handle different cases (e.g. config restore of 2 LAN device to 1 LAN port device etc.)
     * These type of scenarios may happen when config restore from 1602XP2 device to 0801XP2/1601XP2, from 0801XP2/1601XP2/1602XP2
     * to 0801XSP2/1601XP2 and other few cases. */
    for (configId = 0; configId < MAX_CONFIG_ID; configId++)
    {
        /* Handle config compatibility cases for different NVR models */
        makeConfigModelCompatibile(configId);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes configuration file related to given config index.
 * @param   cfgIndex
 * @return  SUCCESS/FAIL
 */
BOOL RemoveConfigFile(CONFIG_INDEX_e cfgIndex)
{
    CHAR filePath[100];

    snprintf(filePath, sizeof(filePath), APP_CONFIG_DIR_PATH "/%s" CONFIG_FILE_EXT, (sysConfigParam + cfgIndex)->fileName);
	if (unlink(filePath) != STATUS_OK)
	{
        EPRINT(CONFIGURATION, "fail to remove file: [path=%s], [err=%s]", filePath, STR_ERR);
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes configuration file related to given config index.
 * @param   cfgIndex
 * @return  SUCCESS/FAIL
 */
BOOL RemoveLocalConfigFile(LOCAL_CONFIG_INDEX_e cfgIndex)
{
    CHAR filePath[100];

    snprintf(filePath, sizeof(filePath), APP_CONFIG_DIR_PATH "/%s" CONFIG_FILE_EXT, localConfigFileName[cfgIndex]);
	if (unlink(filePath) != STATUS_OK)
	{
        EPRINT(CONFIGURATION, "fail to remove file: [path=%s], [err=%s]", filePath, STR_ERR);
        return FAIL;
	}

    return SUCCESS;
}

/** ******************************************************************************************* **/
/**                                  Config Default APIs                                        **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
/**
 * @brief   dfltXXXConfig function loads default values to the configuration
 * @return  Status
 */
NET_CMD_STATUS_e DfltGeneralConfig(void)                    {return dfltXXXConfig(GENERAL_CONFIG_ID);}
NET_CMD_STATUS_e DfltDateTimeConfig(void)                   {return dfltXXXConfig(DATE_TIME_CONFIG_ID);}
NET_CMD_STATUS_e DfltDstConfig(void)                        {return dfltXXXConfig(DST_CONFIG_ID);}
NET_CMD_STATUS_e DfltLan1Config(void)                       {return dfltXXXConfig(LAN1_CONFIG_ID);}
NET_CMD_STATUS_e DfltLan2Config(void)                       {return dfltXXXConfig(LAN2_CONFIG_ID);}
NET_CMD_STATUS_e DfltMacServerConfig(void)                  {return dfltXXXConfig(MAC_SERVER_CONFIG_ID);}
NET_CMD_STATUS_e DfltIpFilterConfig(void)                   {return dfltXXXConfig(IP_FILTER_CONFIG_ID);}
NET_CMD_STATUS_e DfltDdnsConfig(void)                       {return dfltXXXConfig(DDNS_CONFIG_ID);}
NET_CMD_STATUS_e DfltSmtpConfig(void)                       {return dfltXXXConfig(SMTP_CONFIG_ID);}
NET_CMD_STATUS_e DfltSingleFtpUploadConfig(UINT8 ftpIndex)  {return dfltXXXConfig(FTP_UPLOAD_CONFIG_ID, SINGLE_ENTRY, ftpIndex);}
NET_CMD_STATUS_e DfltFtpUploadConfig(void)                  {return dfltXXXConfig(FTP_UPLOAD_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltTcpNotifyConfig(void)                  {return dfltXXXConfig(TCP_NOTIFY_CONFIG_ID);}
NET_CMD_STATUS_e DfltFileAccessConfig(void)                 {return dfltXXXConfig(FILE_ACCESS_CONFIG_ID);}
NET_CMD_STATUS_e DfltHddConfig(void)                        {return dfltXXXConfig(HDD_CONFIG_ID);}
NET_CMD_STATUS_e DfltMatrixDnsServerConfig(void)            {return dfltXXXConfig(MATRIX_DNS_SERVER_CONFIG_ID);}
NET_CMD_STATUS_e DfltUserAccountConfig(void)                {return dfltXXXConfig(USER_ACCOUNT_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltCameraConfig(void)                     {return dfltXXXConfig(CAMERA_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltStreamConfig(void)                     {return dfltXXXConfig(STREAM_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltScheduleRecordConfig(void)             {return dfltXXXConfig(SCHEDULE_RECORD_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltAlarmRecordConfig(void)                {return dfltXXXConfig(ALARM_RECORD_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltPtzPresetConfig(void)                  {return dfltXXXConfig(PTZ_PRESET_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltPresetTourConfig(void)                 {return dfltXXXConfig(PRESET_TOUR_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltTourScheduleConfig(void)               {return dfltXXXConfig(TOUR_SCHEDULE_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltImageUploadConfig(void)                {return dfltXXXConfig(IMAGE_UPLOAD_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltSensorConfig(void)                     {return dfltXXXConfig(SENSOR_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltAlarmConfig(void)                      {return dfltXXXConfig(ALARM_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltStorageConfig(void)                    {return dfltXXXConfig(STORAGE_CONFIG_ID);}
NET_CMD_STATUS_e DfltScheduleBackupConfig(void)             {return dfltXXXConfig(SCHEDULE_BACKUP_CONFIG_ID);}
NET_CMD_STATUS_e DfltManualBackupConfig(void)               {return dfltXXXConfig(MANUAL_BACKUP_CONFIG_ID);}
NET_CMD_STATUS_e DfltCameraAlarmConfig(void)                {return dfltXXXConfig(CAMERA_ALARM_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltCameraEventConfig(void)                {return dfltXXXConfig(CAMERA_ALARM_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltSensorEventConfig(void)                {return dfltXXXConfig(SENSOR_EVENT_ACTION_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltSystemEventConfig(void)                {return dfltXXXConfig(SYSTEM_EVENT_ACTION_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltStaticRoutingConfig(void)              {return dfltXXXConfig(STATIC_ROUTING_CONFIG_ID);}
NET_CMD_STATUS_e DfltBroadBandConfig(void)                  {return dfltXXXConfig(BROAD_BAND_CONFIG_ID);}
NET_CMD_STATUS_e DfltSmsConfig(void)                        {return dfltXXXConfig(SMS_CONFIG_ID);}
NET_CMD_STATUS_e DfltNetworkDriveConfig(void)               {return dfltXXXConfig(NETWORK_DRIVE_SETTINGS, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltSnapshotConfig(void)                   {return dfltXXXConfig(SNAPSHOT_CONFIG_ID,WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltSnapShotScheduleConfig(void)           {return dfltXXXConfig(SNAPSHOT_SCHEDULE_CONFIG_ID,WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltLoginPolicyConfig(void)                {return dfltXXXConfig(LOGIN_POLICY_CONFIG_ID,WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltAudioOutConfig(void)                   {return dfltXXXConfig(AUDIO_OUT_CONFIG_ID,WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltP2PConfig(void)                        {return dfltXXXConfig(P2P_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltImageSettingConfig(void)               {return dfltXXXConfig(IMG_SETTING_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltDhcpServerConfig(void)                 {return dfltXXXConfig(DHCP_SERVER_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltFirmwareManagementConfig(void)         {return dfltXXXConfig(FIRMWARE_MANAGEMENT_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltFcmPushNotificationConfig(void)        {return dfltXXXConfig(FCM_PUSH_NOTIFICATION_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltPasswordRecoveryConfig(void)           {return dfltXXXConfig(PASSWORD_RECOVERY_CONFIG_ID, WHOLE_CONFIG);}
NET_CMD_STATUS_e DfltStorageAllocationConfig(void)          {return dfltXXXConfig(STORAGE_ALLOCATION_CONFIG_ID, WHOLE_CONFIG);}

/** ******************************************************************************************* **/
/**          Config Default Single Entry APIs (for multi dimentional config)                    **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
/**
 * @brief   DfltSingleXXXConfig function loads default values to the element of configuration, pointed by index.
 * @param   userIndex
 * @return  Status
 */
NET_CMD_STATUS_e DfltSingleUserAccountConfig(UINT8 userIndex)                       {return dfltXXXConfig(USER_ACCOUNT_CONFIG_ID, SINGLE_ENTRY, userIndex);}
NET_CMD_STATUS_e DfltSingleCameraConfig(UINT8 cameraIndex)                          {return dfltXXXConfig(CAMERA_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleStreamConfig(UINT8 cameraIndex)                          {return dfltXXXConfig(STREAM_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleScheduleRecordConfig(UINT8 cameraIndex)                  {return dfltXXXConfig(SCHEDULE_RECORD_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleAlarmRecordConfig(UINT8 cameraIndex)                     {return dfltXXXConfig(ALARM_RECORD_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSinglePtzPresetConfig(UINT8 cameraIndex, UINT8 ptzIndex)       {return dfltXXXConfig(PTZ_PRESET_CONFIG_ID, SINGLE_ENTRY, cameraIndex, ptzIndex);}
NET_CMD_STATUS_e DfltSinglePresetTourConfig(UINT8 cameraIndex)                      {return dfltXXXConfig(PRESET_TOUR_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleTourScheduleConfig(UINT8 cameraIndex)                    {return dfltXXXConfig(TOUR_SCHEDULE_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleImageUploadConfig(UINT8 cameraIndex)                     {return dfltXXXConfig(IMAGE_UPLOAD_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleSensorConfig(UINT8 sensorIndex)                          {return dfltXXXConfig(SENSOR_CONFIG_ID, SINGLE_ENTRY, sensorIndex);}
NET_CMD_STATUS_e DfltSingleAlarmConfig(UINT8 alarmIndex)                            {return dfltXXXConfig(ALARM_CONFIG_ID, SINGLE_ENTRY, alarmIndex);}
NET_CMD_STATUS_e DfltSingleCameraAlarmConfig(UINT8 cameraIndex, UINT8 alarmIndex)   {return dfltXXXConfig(CAMERA_ALARM_CONFIG_ID, SINGLE_ENTRY, cameraIndex, alarmIndex);}
NET_CMD_STATUS_e DfltSingleCameraEventConfig(UINT8 cameraIndex, UINT8 eventIndex)   {return dfltXXXConfig(CAMERA_EVENT_ACTION_CONFIG_ID, SINGLE_ENTRY, cameraIndex, eventIndex);}
NET_CMD_STATUS_e DfltSingleSensorEventConfig(UINT8 sensorIndex)                     {return dfltXXXConfig(SENSOR_EVENT_ACTION_CONFIG_ID, SINGLE_ENTRY, sensorIndex);}
NET_CMD_STATUS_e DfltSingleSystemEventConfig(UINT8 eventIndex)                      {return dfltXXXConfig(SYSTEM_EVENT_ACTION_CONFIG_ID, SINGLE_ENTRY, eventIndex);}
NET_CMD_STATUS_e DfltSingleNetworkDriveConfig(UINT8 nwDriveIdx)                     {return dfltXXXConfig(NETWORK_DRIVE_SETTINGS, SINGLE_ENTRY, nwDriveIdx);}
NET_CMD_STATUS_e DfltSingleCosecPreRecConfig(UINT8 cameraIndex)                     {return dfltXXXConfig(COSEC_PRE_RECORD_SETTINGS, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleManualRecordConfig(UINT8 cameraIndex)                    {return dfltXXXConfig(MANUAL_RECORD_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleIpCameraConfig(UINT8 cameraIndex)                        {return dfltXXXConfig(IP_CAMERA_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSingleNetwokDeviceConfig(UINT8 deviceIndex)                    {return dfltXXXConfig(NETWORK_DEVICES_CONFIG_ID, SINGLE_ENTRY, deviceIndex);}
NET_CMD_STATUS_e DfltSingleSnapshotConfig(UINT8 deviceIndex)                        {return dfltXXXConfig(SNAPSHOT_CONFIG_ID,SINGLE_ENTRY, deviceIndex);}
NET_CMD_STATUS_e DfltSingleSnapShotScheduleConfig(UINT8 deviceIndex)                {return dfltXXXConfig(SNAPSHOT_SCHEDULE_CONFIG_ID,SINGLE_ENTRY, deviceIndex);}
NET_CMD_STATUS_e DfltSingleImageSettingConfig(UINT8 cameraIndex)                    {return dfltXXXConfig(IMG_SETTING_CONFIG_ID, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e DfltSinglePushNotificationConfig(UINT8 deviceIndex)                {return dfltXXXConfig(FCM_PUSH_NOTIFICATION_CONFIG_ID, SINGLE_ENTRY, deviceIndex);}
NET_CMD_STATUS_e DfltSinglePasswordRecoveryConfig(UINT8 userIndex)                  {return dfltXXXConfig(PASSWORD_RECOVERY_CONFIG_ID, SINGLE_ENTRY, userIndex);}

/** ******************************************************************************************* **/
/**                                   Config Write APIs                                         **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API updates configuration with the user supplied values; only if they differ.
 * @param   userCopy
 * @return  Status
 */
NET_CMD_STATUS_e WriteGeneralConfig(GENERAL_CONFIG_t *userCopy)                         {return writeXXXConfig(GENERAL_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteDateTimeConfig(DATE_TIME_CONFIG_t *userCopy)                      {return writeXXXConfig(DATE_TIME_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteDstConfig(DST_CONFIG_t *userCopy)                                 {return writeXXXConfig(DST_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteLan1Config(LAN_CONFIG_t *userCopy)                                {return writeXXXConfig(LAN1_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteLan2Config(LAN_CONFIG_t *userCopy)                                {return writeXXXConfig(LAN2_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteMacServerConfig(MAC_SERVER_CNFG_t *userCopy)                      {return writeXXXConfig(MAC_SERVER_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteIpFilterConfig(IP_FILTER_CONFIG_t *userCopy)                      {return writeXXXConfig(IP_FILTER_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteDdnsConfig(DDNS_CONFIG_t *userCopy)                               {return writeXXXConfig(DDNS_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteSmtpConfig(SMTP_CONFIG_t *userCopy)                               {return writeXXXConfig(SMTP_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteFtpUploadConfig(FTP_UPLOAD_CONFIG_t *userCopy)                    {return writeXXXConfig(FTP_UPLOAD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteTcpNotifyConfig(TCP_NOTIFY_CONFIG_t *userCopy)                    {return writeXXXConfig(TCP_NOTIFY_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteFileAccessConfig(FILE_ACCESS_CONFIG_t *userCopy)                  {return writeXXXConfig(FILE_ACCESS_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteHddConfig(HDD_CONFIG_t *userCopy)                                 {return writeXXXConfig(HDD_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteMatrixDnsServerConfig(MATRIX_DNS_SERVER_CONFIG_t *userCopy)       {return writeXXXConfig(MATRIX_DNS_SERVER_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteUserAccountConfig(USER_ACCOUNT_CONFIG_t *userCopy)                {return writeXXXConfig(USER_ACCOUNT_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteCameraConfig(CAMERA_CONFIG_t *userCopy)                           {return writeXXXConfig(CAMERA_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteStreamConfig(STREAM_CONFIG_t *userCopy)                           {return writeXXXConfig(STREAM_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteScheduleRecordConfig(SCHEDULE_RECORD_CONFIG_t *userCopy)          {return writeXXXConfig(SCHEDULE_RECORD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteAlarmRecordConfig(ALARM_RECORD_CONFIG_t *userCopy)                {return writeXXXConfig(ALARM_RECORD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WritePtzPresetConfig(PTZ_PRESET_CONFIG_t **userCopy)                   {return writeXXXConfig(PTZ_PRESET_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WritePresetTourConfig(PRESET_TOUR_CONFIG_t *userCopy)                  {return writeXXXConfig(PRESET_TOUR_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteTourScheduleConfig(TOUR_SCHEDULE_CONFIG_t *userCopy)              {return writeXXXConfig(TOUR_SCHEDULE_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteImageUploadConfig(IMAGE_UPLOAD_CONFIG_t *userCopy)                {return writeXXXConfig(IMAGE_UPLOAD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteSensorConfig(SENSOR_CONFIG_t *userCopy)                           {return writeXXXConfig(SENSOR_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteAlarmConfig(ALARM_CONFIG_t *userCopy)                             {return writeXXXConfig(ALARM_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteStorageConfig(STORAGE_CONFIG_t *userCopy)                         {return writeXXXConfig(STORAGE_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteScheduleBackupConfig(SCHEDULE_BACKUP_CONFIG_t *userCopy)          {return writeXXXConfig(SCHEDULE_BACKUP_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteManualBackupConfig(MANUAL_BACKUP_CONFIG_t *userCopy)              {return writeXXXConfig(MANUAL_BACKUP_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteCameraAlarmConfig(CAMERA_ALARM_CONFIG_t **userCopy)               {return writeXXXConfig(CAMERA_ALARM_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteCameraEventConfig(CAMERA_EVENT_CONFIG_t **userCopy)               {return writeXXXConfig(CAMERA_EVENT_ACTION_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteSensorEventConfig(SENSOR_EVENT_CONFIG_t *userCopy)                {return writeXXXConfig(SENSOR_EVENT_ACTION_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteSystemEventConfig(SYSTEM_EVENT_CONFIG_t *userCopy)                {return writeXXXConfig(SYSTEM_EVENT_ACTION_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteStaticRoutingConfig(STATIC_ROUTING_CONFIG_t *userCopy)            {return writeXXXConfig(STATIC_ROUTING_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteBraodBandConfig(BROAD_BAND_CONFIG_t * userCopy)                   {return writeXXXConfig(BROAD_BAND_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteSmsConfig(SMS_CONFIG_t * userCopy)                                {return writeXXXConfig(SMS_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteNetworkDriveConfig(NETWORK_DRIVE_CONFIG_t *userCopy)              {return writeXXXConfig(NETWORK_DRIVE_SETTINGS, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteSnapshotConfig(SNAPSHOT_CONFIG_t *userCopy)                       {return writeXXXConfig(SNAPSHOT_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteSnapshotScheduleConfig(SNAPSHOT_SCHEDULE_CONFIG_t *userCopy)      {return writeXXXConfig(SNAPSHOT_SCHEDULE_CONFIG_ID, userCopy, WHOLE_CONFIG);}
NET_CMD_STATUS_e WriteLoginPolicyConfig(LOGIN_POLICY_CONFIG_t *userCopy)                {return writeXXXConfig(LOGIN_POLICY_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteAudioOutConfig(AUDIO_OUT_CONFIG_t *userCopy)                      {return writeXXXConfig(AUDIO_OUT_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteP2PConfig(P2P_CONFIG_t *userCopy)                                 {return writeXXXConfig(P2P_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteImageSettingConfig(IMG_SETTING_CONFIG_t *userCopy)                {return writeXXXConfig(IMG_SETTING_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteDhcpServerConfig(DHCP_SERVER_CONFIG_t *userCopy)                  {return writeXXXConfig(DHCP_SERVER_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteFirmwareManagementConfig(FIRMWARE_MANAGEMENT_CONFIG_t *userCopy)  {return writeXXXConfig(FIRMWARE_MANAGEMENT_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteFcmPushNotifyConfig(FCM_PUSH_NOTIFY_CONFIG_t *userCopy)           {return writeXXXConfig(FCM_PUSH_NOTIFICATION_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WritePasswordRecoveryConfig(PASSWORD_RECOVERY_CONFIG_t *userCopy)      {return writeXXXConfig(PASSWORD_RECOVERY_CONFIG_ID, userCopy);}
NET_CMD_STATUS_e WriteStorageAllocationConfig(STORAGE_ALLOCATION_CONFIG_t *userCopy)    {return writeXXXConfig(STORAGE_ALLOCATION_CONFIG_ID, userCopy);}

/** ******************************************************************************************* **/
/**            Config Write Single Entry APIs (for multi dimentional config)                    **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API updates element of the configuration, pointed by index, with the user supplied
 *          values, only if they differ. It also notifies user modules about the changes.
 * @param   userIndex
 * @param   userCopy
 * @return  Status
 */
NET_CMD_STATUS_e WriteSingleUserAccountConfig(UINT8 userIndex, USER_ACCOUNT_CONFIG_t *userCopy)                     {return writeXXXConfig(USER_ACCOUNT_CONFIG_ID, userCopy, SINGLE_ENTRY, userIndex);}
NET_CMD_STATUS_e WriteSingleCameraConfig(UINT8 cameraIndex, CAMERA_CONFIG_t *userCopy)                              {return writeXXXConfig(CAMERA_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleStreamConfig(UINT8 cameraIndex, STREAM_CONFIG_t *userCopy)                              {return writeXXXConfig(STREAM_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleScheduleRecordConfig(UINT8 cameraIndex, SCHEDULE_RECORD_CONFIG_t *userCopy)             {return writeXXXConfig(SCHEDULE_RECORD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleFtpUploadConfig(UINT8 ftpIndex, FTP_UPLOAD_CONFIG_t *userCopy)                          {return writeXXXConfig(FTP_UPLOAD_CONFIG_ID, userCopy, SINGLE_ENTRY, ftpIndex);}
NET_CMD_STATUS_e WriteSingleAlarmRecordConfig(UINT8 cameraIndex, ALARM_RECORD_CONFIG_t *userCopy)                   {return writeXXXConfig(ALARM_RECORD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSinglePtzPresetConfig(UINT8 cameraIndex, UINT8 ptzIndex, PTZ_PRESET_CONFIG_t *userCopy)       {return writeXXXConfig(PTZ_PRESET_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex, ptzIndex);}
NET_CMD_STATUS_e WriteSinglePresetTourConfig(UINT8 cameraIndex, PRESET_TOUR_CONFIG_t *userCopy)                     {return writeXXXConfig(PRESET_TOUR_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleTourScheduleConfig(UINT8 cameraIndex, TOUR_SCHEDULE_CONFIG_t *userCopy)                 {return writeXXXConfig(TOUR_SCHEDULE_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleImageUploadConfig(UINT8 cameraIndex, IMAGE_UPLOAD_CONFIG_t *userCopy)                   {return writeXXXConfig(IMAGE_UPLOAD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleSensorConfig(UINT8 sensorIndex, SENSOR_CONFIG_t *userCopy)                              {return writeXXXConfig(SENSOR_CONFIG_ID, userCopy, SINGLE_ENTRY, sensorIndex);}
NET_CMD_STATUS_e WriteSingleAlarmConfig(UINT8 alarmIndex, ALARM_CONFIG_t *userCopy)                                 {return writeXXXConfig(ALARM_CONFIG_ID, userCopy, SINGLE_ENTRY, alarmIndex);}
NET_CMD_STATUS_e WriteSingleCameraAlarmConfig(UINT8 cameraIndex, UINT8 alarmIndex, CAMERA_ALARM_CONFIG_t *userCopy)	{return writeXXXConfig(CAMERA_ALARM_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex, alarmIndex);}
NET_CMD_STATUS_e WriteSingleCameraEventConfig(UINT8 cameraIndex, UINT8 eventIndex, CAMERA_EVENT_CONFIG_t *userCopy) {return writeXXXConfig(CAMERA_EVENT_ACTION_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex, eventIndex);}
NET_CMD_STATUS_e WriteSingleSensorEventConfig(UINT8 sensorIndex, SENSOR_EVENT_CONFIG_t *userCopy)                   {return writeXXXConfig(SENSOR_EVENT_ACTION_CONFIG_ID, userCopy, SINGLE_ENTRY, sensorIndex);}
NET_CMD_STATUS_e WriteSingleSystemEventConfig(UINT8 eventIndex, SYSTEM_EVENT_CONFIG_t *userCopy)                    {return writeXXXConfig(SYSTEM_EVENT_ACTION_CONFIG_ID, userCopy, SINGLE_ENTRY, eventIndex);}
NET_CMD_STATUS_e WriteSingleNetworkDriveConfig(UINT8 nwDriveIdx, NETWORK_DRIVE_CONFIG_t *userCopy)                  {return writeXXXConfig(NETWORK_DRIVE_SETTINGS, userCopy, SINGLE_ENTRY, nwDriveIdx);}
NET_CMD_STATUS_e WriteSingleCosecPreRecConfig(UINT8 cameraIndex, COSEC_REC_PARAM_CONFIG_t *userCopy)                {return writeXXXConfig(COSEC_PRE_RECORD_SETTINGS, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleManualRecordConfig(UINT8 cameraIndex, MANUAL_RECORD_CONFIG_t * userCopy)                {return writeXXXConfig(MANUAL_RECORD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleIpCameraConfig(UINT8 cameraIndex, IP_CAMERA_CONFIG_t *userCopy)                         {return writeXXXConfig(IP_CAMERA_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleNetworkDeviceConfig(UINT8 deviceIndex, NETWORK_DEVICE_CONFIG_t *userCopy)               {return writeXXXConfig(NETWORK_DEVICES_CONFIG_ID, userCopy, SINGLE_ENTRY, deviceIndex);}
NET_CMD_STATUS_e WriteSingleSnapshotConfig(UINT8 cameraIndex, SNAPSHOT_CONFIG_t *userCopy)                          {return writeXXXConfig(SNAPSHOT_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleSnapshotScheduleConfig(UINT8 cameraIndex, SNAPSHOT_SCHEDULE_CONFIG_t *userCopy)         {return writeXXXConfig(SNAPSHOT_SCHEDULE_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleImageSettingConfig(UINT8 cameraIndex, IMG_SETTING_CONFIG_t *userCopy)                   {return writeXXXConfig(IMG_SETTING_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
NET_CMD_STATUS_e WriteSingleFcmPushNotificationConfig(UINT8 deviceIndex, FCM_PUSH_NOTIFY_CONFIG_t *userCopy)        {return writeXXXConfig(FCM_PUSH_NOTIFICATION_CONFIG_ID, userCopy, SINGLE_ENTRY, deviceIndex);}
NET_CMD_STATUS_e WriteSinglePasswordRecoveryConfig(UINT8 userIndex, PASSWORD_RECOVERY_CONFIG_t *userCopy)           {return writeXXXConfig(PASSWORD_RECOVERY_CONFIG_ID, userCopy, SINGLE_ENTRY, userIndex);}

/** ******************************************************************************************* **/
/**                                   Config Read APIs                                          **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API outputs the values of configuration to the user.
 * @param   userCopy
 * @return  SUCCESS/FAIL
 */
BOOL ReadGeneralConfig(GENERAL_CONFIG_t *userCopy)                          {return readXXXConfig(GENERAL_CONFIG_ID, userCopy);}
BOOL ReadDateTimeConfig(DATE_TIME_CONFIG_t *userCopy)                       {return readXXXConfig(DATE_TIME_CONFIG_ID, userCopy);}
BOOL ReadDstConfig(DST_CONFIG_t *userCopy)                                  {return readXXXConfig(DST_CONFIG_ID, userCopy);}
BOOL ReadLan1Config(LAN_CONFIG_t *userCopy)                                 {return readXXXConfig(LAN1_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadLan2Config(LAN_CONFIG_t *userCopy)                                 {return readXXXConfig(LAN2_CONFIG_ID, userCopy);}
BOOL ReadMacServerConfig(MAC_SERVER_CNFG_t *userCopy)                       {return readXXXConfig(MAC_SERVER_CONFIG_ID, userCopy);}
BOOL ReadIpFilterConfig(IP_FILTER_CONFIG_t *userCopy)                       {return readXXXConfig(IP_FILTER_CONFIG_ID, userCopy);}
BOOL ReadDdnsConfig(DDNS_CONFIG_t *userCopy)                                {return readXXXConfig(DDNS_CONFIG_ID, userCopy);}
BOOL ReadSmtpConfig(SMTP_CONFIG_t *userCopy)                                {return readXXXConfig(SMTP_CONFIG_ID, userCopy);}
BOOL ReadFtpUploadConfig(FTP_UPLOAD_CONFIG_t *userCopy)                     {return readXXXConfig(FTP_UPLOAD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadTcpNotifyConfig(TCP_NOTIFY_CONFIG_t *userCopy)                     {return readXXXConfig(TCP_NOTIFY_CONFIG_ID, userCopy);}
BOOL ReadFileAccessConfig(FILE_ACCESS_CONFIG_t *userCopy)                   {return readXXXConfig(FILE_ACCESS_CONFIG_ID, userCopy);}
BOOL ReadHddConfig(HDD_CONFIG_t *userCopy)                                  {return readXXXConfig(HDD_CONFIG_ID, userCopy);}
BOOL ReadMatrixDnsServerConfig(MATRIX_DNS_SERVER_CONFIG_t *userCopy)        {return readXXXConfig(MATRIX_DNS_SERVER_CONFIG_ID, userCopy);}
BOOL ReadUserAccountConfig(USER_ACCOUNT_CONFIG_t *userCopy)                 {return readXXXConfig(USER_ACCOUNT_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadCameraConfig(CAMERA_CONFIG_t *userCopy)                            {return readXXXConfig(CAMERA_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadStreamConfig(STREAM_CONFIG_t *userCopy)                            {return readXXXConfig(STREAM_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadScheduleRecordConfig(SCHEDULE_RECORD_CONFIG_t *userCopy)           {return readXXXConfig(SCHEDULE_RECORD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadAlarmRecordConfig(ALARM_RECORD_CONFIG_t *userCopy)                 {return readXXXConfig(ALARM_RECORD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadPtzPresetConfig(PTZ_PRESET_CONFIG_t **userCopy)                    {return readXXXConfig(PTZ_PRESET_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadPresetTourConfig(PRESET_TOUR_CONFIG_t *userCopy)                   {return readXXXConfig(PRESET_TOUR_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadTourScheduleConfig(TOUR_SCHEDULE_CONFIG_t *userCopy)               {return readXXXConfig(TOUR_SCHEDULE_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadImageUploadConfig(IMAGE_UPLOAD_CONFIG_t *userCopy)                 {return readXXXConfig(IMAGE_UPLOAD_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadSensorConfig(SENSOR_CONFIG_t *userCopy)                            {return readXXXConfig(SENSOR_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadAlarmConfig(ALARM_CONFIG_t *userCopy)                              {return readXXXConfig(ALARM_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadStorageConfig(STORAGE_CONFIG_t *userCopy)                          {return readXXXConfig(STORAGE_CONFIG_ID, userCopy);}
BOOL ReadScheduleBackupConfig(SCHEDULE_BACKUP_CONFIG_t *userCopy)           {return readXXXConfig(SCHEDULE_BACKUP_CONFIG_ID, userCopy);}
BOOL ReadManualBackupConfig(MANUAL_BACKUP_CONFIG_t *userCopy)               {return readXXXConfig(MANUAL_BACKUP_CONFIG_ID, userCopy);}
BOOL ReadCameraAlarmConfig(CAMERA_ALARM_CONFIG_t **userCopy)                {return readXXXConfig(CAMERA_ALARM_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadCameraEventConfig(CAMERA_EVENT_CONFIG_t **userCopy)                {return readXXXConfig(CAMERA_EVENT_ACTION_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadSensorEventConfig(SENSOR_EVENT_CONFIG_t *userCopy)                 {return readXXXConfig(SENSOR_EVENT_ACTION_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadSystemEventConfig(SYSTEM_EVENT_CONFIG_t *userCopy)                 {return readXXXConfig(SYSTEM_EVENT_ACTION_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadStaticRoutingConfig(STATIC_ROUTING_CONFIG_t *userCopy)             {return readXXXConfig(STATIC_ROUTING_CONFIG_ID, userCopy);}
BOOL ReadBroadBandConfig(BROAD_BAND_CONFIG_t *userCopy)                     {return readXXXConfig(BROAD_BAND_CONFIG_ID, userCopy);}
BOOL ReadSmsConfig(SMS_CONFIG_t *userCopy)                                  {return readXXXConfig(SMS_CONFIG_ID, userCopy);}
BOOL ReadNetworkDriveConfig(NETWORK_DRIVE_CONFIG_t *userCopy)               {return readXXXConfig(NETWORK_DRIVE_SETTINGS, userCopy, WHOLE_CONFIG);}
BOOL ReadIpCameraConfig(IP_CAMERA_CONFIG_t *userCopy)                       {return readXXXConfig(IP_CAMERA_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadSnapshotConfig(SNAPSHOT_CONFIG_t *userCopy)                        {return readXXXConfig(SNAPSHOT_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadSnapshotScheduleConfig(SNAPSHOT_SCHEDULE_CONFIG_t *userCopy)       {return readXXXConfig(SNAPSHOT_SCHEDULE_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadLoginPolicyConfig(LOGIN_POLICY_CONFIG_t *userCopy)                 {return readXXXConfig(LOGIN_POLICY_CONFIG_ID, userCopy);}
BOOL ReadAudioOutConfig(AUDIO_OUT_CONFIG_t *userCopy)                       {return readXXXConfig(AUDIO_OUT_CONFIG_ID, userCopy);}
BOOL ReadP2PConfig(P2P_CONFIG_t *userCopy)                                  {return readXXXConfig(P2P_CONFIG_ID, userCopy);}
BOOL ReadImageSettingConfig(IMG_SETTING_CONFIG_t *userCopy)                 {return readXXXConfig(IMG_SETTING_CONFIG_ID, userCopy);}
BOOL ReadDhcpServerConfig(DHCP_SERVER_CONFIG_t *userCopy)                   {return readXXXConfig(DHCP_SERVER_CONFIG_ID, userCopy);}
BOOL ReadFirmwareManagementConfig(FIRMWARE_MANAGEMENT_CONFIG_t *userCopy)   {return readXXXConfig(FIRMWARE_MANAGEMENT_CONFIG_ID, userCopy);}
BOOL ReadFcmPushNotifyConfig(FCM_PUSH_NOTIFY_CONFIG_t *userCopy)            {return readXXXConfig(FCM_PUSH_NOTIFICATION_CONFIG_ID, userCopy);}
BOOL ReadPasswordRecoveryConfig(PASSWORD_RECOVERY_CONFIG_t *userCopy)       {return readXXXConfig(PASSWORD_RECOVERY_CONFIG_ID, userCopy, WHOLE_CONFIG);}
BOOL ReadStorageAllocationConfig(STORAGE_ALLOCATION_CONFIG_t *userCopy)     {return readXXXConfig(STORAGE_ALLOCATION_CONFIG_ID, userCopy, WHOLE_CONFIG);}

/** ******************************************************************************************* **/
/**             Config Read Single Entry APIs (for multi dimentional config)                    **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API updates element of the configuration, pointed by index, with the user supplied
 *          values, only if they differ. It also notifies user modules about the changes.
 * @param   cameraIndex
 * @param   userCopy
 * @return  SUCCESS/FAIL
 */
BOOL ReadSingleCameraConfig(UINT8 cameraIndex, CAMERA_CONFIG_t *userCopy)                               {return readXXXConfig(CAMERA_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleUserAccountConfig(UINT8 userIndex, USER_ACCOUNT_CONFIG_t *userCopy)                      {return readXXXConfig(USER_ACCOUNT_CONFIG_ID, userCopy, SINGLE_ENTRY, userIndex);}
BOOL ReadSingleStreamConfig(UINT8 cameraIndex, STREAM_CONFIG_t *userCopy)                               {return readXXXConfig(STREAM_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleFtpUploadConfig(UINT8 ftpIndex, FTP_UPLOAD_CONFIG_t *userCopy)                           {return readXXXConfig(FTP_UPLOAD_CONFIG_ID, userCopy, SINGLE_ENTRY, ftpIndex);}
BOOL ReadSingleScheduleRecordConfig(UINT8 cameraIndex, SCHEDULE_RECORD_CONFIG_t *userCopy)              {return readXXXConfig(SCHEDULE_RECORD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleAlarmRecordConfig(UINT8 cameraIndex, ALARM_RECORD_CONFIG_t *userCopy)                    {return readXXXConfig(ALARM_RECORD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSinglePtzPresetConfig(UINT8 cameraIndex, UINT8 ptzIndex, PTZ_PRESET_CONFIG_t *userCopy)        {return readXXXConfig(PTZ_PRESET_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex, ptzIndex);}
BOOL ReadSinglePresetTourConfig(UINT8 cameraIndex, PRESET_TOUR_CONFIG_t *userCopy)                      {return readXXXConfig(PRESET_TOUR_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleTourScheduleConfig(UINT8 cameraIndex, TOUR_SCHEDULE_CONFIG_t *userCopy)                  {return readXXXConfig(TOUR_SCHEDULE_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleImageUploadConfig(UINT8 cameraIndex, IMAGE_UPLOAD_CONFIG_t *userCopy)                    {return readXXXConfig(IMAGE_UPLOAD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleSensorConfig(UINT8 sensorIndex, SENSOR_CONFIG_t *userCopy)                               {return readXXXConfig(SENSOR_CONFIG_ID, userCopy, SINGLE_ENTRY, sensorIndex);}
BOOL ReadSingleAlarmConfig(UINT8 alarmIndex, ALARM_CONFIG_t *userCopy)                                  {return readXXXConfig(ALARM_CONFIG_ID, userCopy, SINGLE_ENTRY, alarmIndex);}
BOOL ReadSingleCameraAlarmConfig(UINT8 cameraIndex, UINT8 alarmIndex, CAMERA_ALARM_CONFIG_t *userCopy)	{return readXXXConfig(CAMERA_ALARM_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex, alarmIndex);}
BOOL ReadSingleCameraEventConfig(UINT8 cameraIndex, UINT8 eventIndex, CAMERA_EVENT_CONFIG_t *userCopy)	{return readXXXConfig(CAMERA_EVENT_ACTION_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex, eventIndex);}
BOOL ReadSingleSensorEventConfig(UINT8 sensorIndex, SENSOR_EVENT_CONFIG_t *userCopy)                    {return readXXXConfig(SENSOR_EVENT_ACTION_CONFIG_ID, userCopy, SINGLE_ENTRY, sensorIndex);}
BOOL ReadSingleSystemEventConfig(UINT8 eventIndex, SYSTEM_EVENT_CONFIG_t *userCopy)                     {return readXXXConfig(SYSTEM_EVENT_ACTION_CONFIG_ID, userCopy, SINGLE_ENTRY, eventIndex);}
BOOL ReadSingleNetworkDriveConfig(UINT8 nwDriveIdx, NETWORK_DRIVE_CONFIG_t *userCopy)                   {return readXXXConfig(NETWORK_DRIVE_SETTINGS, userCopy, SINGLE_ENTRY, nwDriveIdx);}
BOOL ReadSingleCosecPreRecConfig(UINT8 cameraIndex, COSEC_REC_PARAM_CONFIG_t *userCopy)                 {return readXXXConfig(COSEC_PRE_RECORD_SETTINGS, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleManualRecordConfig(UINT8 cameraIndex, MANUAL_RECORD_CONFIG_t *userCopy)                  {return readXXXConfig(MANUAL_RECORD_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleIpCameraConfig(UINT8 cameraIndex, IP_CAMERA_CONFIG_t *userCopy)                          {return readXXXConfig(IP_CAMERA_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleNetworkDeviceConfig(UINT8 deviceIndex, NETWORK_DEVICE_CONFIG_t *userCopy)                {return readXXXConfig(NETWORK_DEVICES_CONFIG_ID, userCopy, SINGLE_ENTRY, deviceIndex);}
BOOL ReadSingleSnapshotConfig(UINT8 deviceIndex, SNAPSHOT_CONFIG_t *userCopy)                           {return readXXXConfig(SNAPSHOT_CONFIG_ID, userCopy, SINGLE_ENTRY, deviceIndex);}
BOOL ReadSingleSnapshotScheduleConfig(UINT8 deviceIndex, SNAPSHOT_SCHEDULE_CONFIG_t *userCopy)          {return readXXXConfig(SNAPSHOT_SCHEDULE_CONFIG_ID, userCopy, SINGLE_ENTRY, deviceIndex);}
BOOL ReadSingleImageSettingConfig(UINT8 cameraIndex, IMG_SETTING_CONFIG_t *userCopy)                    {return readXXXConfig(IMG_SETTING_CONFIG_ID, userCopy, SINGLE_ENTRY, cameraIndex);}
BOOL ReadSingleFcmPushNotificationConfig(UINT8 deviceIndex, FCM_PUSH_NOTIFY_CONFIG_t *userCopy)         {return readXXXConfig(FCM_PUSH_NOTIFICATION_CONFIG_ID, userCopy, SINGLE_ENTRY, deviceIndex);}
BOOL ReadSinglePasswordRecoveryConfig(UINT8 userIndex, PASSWORD_RECOVERY_CONFIG_t *userCopy)            {return readXXXConfig(PASSWORD_RECOVERY_CONFIG_ID, userCopy, SINGLE_ENTRY, userIndex);}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API loads user copy with the content of the master copy. master copy is protected
 *          during update to avoid device malfunction as a reason of invalid access.
 * @param   userCopy
 * @return  SUCCESS/FAIL
 */
BOOL ReadLan1ConfigCms(LAN_CONFIG_t *userCopy)
{
	// IF any of the input / output parameter is invalid
	if (userCopy == NULL)
	{
        EPRINT(CONFIGURATION, "invld input param");
        return FAIL;
	}

    // Update user copy with content of master copy
    if(GetNetworkParamInfo(NETWORK_PORT_LAN1, userCopy) == FAIL)
    {
        return FAIL;
    }

	// Return status
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API loads user copy with the content of the master copy. master copy is protected
 *          during update to avoid device malfunction as a reason of invalid access.
 * @param   userCopy
 * @return  SUCCESS/FAIL
 */
BOOL ReadLan2ConfigCms(LAN_CONFIG_t *userCopy)
{
	// IF any of the input / output parameter is invalid
	if (userCopy == NULL)
	{
        EPRINT(CONFIGURATION, "invld input param");
        return FAIL;
	}

    // Update user copy with content of master copy
    if (GetNetworkParamInfo(NETWORK_PORT_LAN2, userCopy) == FAIL)
    {
        return FAIL;
    }

	// Return status
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API uses when user press the RESET button key. On RESET Button this function default
 *          LAN1 parameter, password of admin user and default HTTP PORT.
 */
void ResetButtonAction(void)
{
    LOCAL_CONFIG_INDEX_e localConfigId;

	// Default admin User config
	DfltSingleUserAccountConfig(USER_ADMIN);

	// Default Password Reset Info
	RemovePassExpiryFile();

	// Default Debug Configuration
	RemoveDebugConfigFile();

	// reset viewer user sesion info
	ResetAllDefaultUser(TRUE);

	// default general config
	DfltGeneralConfig();

	// first ip address of LAN 1 should be default
	DfltLan1Config();

    for (localConfigId = 0; localConfigId < MAX_LOCAL_CONFIG_ID; localConfigId++)
	{
        if (RemoveLocalConfigFile(localConfigId) == FAIL)
		{
            EPRINT(CONFIGURATION, "fail to remove local config file: [localConfigId=%d]", localConfigId);
			break;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialise the requested configuration. It reads file If not present/ version
 *          mismatch, dumps the default configuration in file else it reads the configurations from file
 * @param   configId
 * @return  SUCCESS/FAIL
 */
static BOOL initXXXConfig(UINT8 configId)
{
    INT32   fileFd = INVALID_FILE_FD;
    CHAR    filePath[100];
    UINT32  fileVersion = 0;
    UINT32  configSize;
    UINT32  offset;
    INT32   readSize;
    BOOL    filePresent = TRUE;

    /* Init config file read-write lock */
	pthread_rwlock_init(&rwLock[configId], NULL);

    /* Get config file size and offset */
    getConfigOffsetAndSize(configId, WHOLE_CONFIG, 0, 0, &offset, &configSize);

    /* Prepare config file path and check file is present or not */
    snprintf(filePath, sizeof(filePath), APP_CONFIG_DIR_PATH "/%s" CONFIG_FILE_EXT, (sysConfigParam + configId)->fileName);
    if (access(filePath, F_OK) != STATUS_OK)
	{
        filePresent = FALSE;
        EPRINT(CONFIGURATION, "config file not present: [file=%s], [err=%s]", (sysConfigParam + configId)->fileName, STR_ERR);
    }

    /* If file is not present then it will create the file else it will open the file */
    fileFd = open(filePath, CREATE_RDWR_SYNC_MODE, USR_RW_GRP_R_OTH_R);
    if (fileFd == INVALID_FILE_FD)
	{
        EPRINT(CONFIGURATION, "fail to open file: [file=%s], [err=%s]", (sysConfigParam + configId)->fileName, STR_ERR);
        return FAIL;
	}

    /* Read config file version */
    if (read(fileFd, &fileVersion, sizeof(UINT32)) < 0)
	{
        EPRINT(CONFIGURATION, "fail to read file: [file=%s], [err=%s]", (sysConfigParam + configId)->fileName, STR_ERR);
        close(fileFd);
        return FAIL;
	}

    /* If file version doesn't match with configuration version */
    if (fileVersion != (sysConfigParam + configId)->version)
    {
        EPRINT(CONFIGURATION, "config version mismatch: [file=%s], [fileVersion=%d], [CurrentVersion=%d]",
               (sysConfigParam + configId)->fileName, fileVersion, (sysConfigParam + configId)->version);

        /* Convert old config version to new config version */
        if ((filePresent == FALSE) || (FAIL == LoadOldConfigParam(configId, fileFd, fileVersion)))
        {
            EPRINT(CONFIGURATION, "fail to access config file : [filePresent=%d] - error loading old config", filePresent);
            /* Write correct version at beginning */
            lseek(fileFd, 0, SEEK_SET);
            fileVersion = (sysConfigParam + configId)->version;

            /* If failed to write version to configuration file */
            if (write(fileFd, &fileVersion, sizeof(UINT32)) != (INT32)sizeof(UINT32))
            {
                EPRINT(CONFIGURATION, "fail to write file version: [file=%s], [err=%s]", (sysConfigParam + configId)->fileName, STR_ERR);
            }
            else
            {
                /* Dump default configuration and pass systemInit = TRUE to prevent notify to all modules at bootup */
                dfltXXXConfigInternal(configId, TRUE, WHOLE_CONFIG, 0, 0);

                /*  NOTE: patch to make an "admin" user entry in ftp user list */
                if (configId == USER_ACCOUNT_CONFIG_ID)
                {
                    AddLinuxUser(ADMIN_USER_NAME, ADMIN_USER_PSWD);
                    RemovePassExpiryFile();

                    /* We have not read the password recovery configuration yet. Hence delete the file */
                    RemoveConfigFile(PASSWORD_RECOVERY_CONFIG_ID);
                }
            }
        }
    }

    /* Seek data position */
    lseek(fileFd, 4, SEEK_SET);
    readSize = read(fileFd, (sysConfigParam + configId)->memPtr, configSize);

    /* If failed to read from configuration file to master copy */
    if (readSize != (INT32)configSize)
    {
        EPRINT(CONFIGURATION, "fail to read file: [file=%s], [err=%s], [readSize=%d], [configSize=%d]",
               (sysConfigParam + configId)->fileName, STR_ERR, readSize, configSize);
        close(fileFd);
        return FAIL;
    }

    /* Close file if opened properly */
    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   We allow config restore from NVR1602XP2 to NVR0801XP2 & NVR1601XP2. We have to check
 *          config which doesn't support by models. Make default or update with valid config.
 * @param   configId
 */
static void makeConfigModelCompatibile(UINT8 configId)
{
#if !defined(OEM_JCI)
    switch(configId)
    {
        case GENERAL_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR1601X:
                case NVR1601XP2:
                case NVR1602XP2:
                case NVR1601XSP2:
                {
                    /* Read general configuration */
                    GENERAL_CONFIG_t generalCfg;
                    ReadGeneralConfig(&generalCfg);

                    /* Check recording format type */
                    if (generalCfg.recordFormatType != REC_NATIVE_FORMAT)
                    {
                        /* Update record format type in general config */
                        EPRINT(CONFIGURATION, "recording format type is not native: [model=%s], [type=%d]", GetNvrModelStr(), generalCfg.recordFormatType);
                        generalCfg.recordFormatType = REC_NATIVE_FORMAT;
                        writeXXXConfigInternal(configId, TRUE, &generalCfg, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case HDD_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0801XP2:
                case NVR1601XP2:
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    /* Read hdd configuration */
                    HDD_CONFIG_t hddCfg;
                    ReadHddConfig(&hddCfg);

                    /* If recording mode is not single disk volume then default the config */
                    if (hddCfg.mode != SINGLE_DISK_VOLUME)
                    {
                        /* Default hdd config due to recording mode is not supported by model */
                        EPRINT(CONFIGURATION, "recording mode is raid: [model=%s], [mode=%d]", GetNvrModelStr(), hddCfg.mode);
                        dfltXXXConfigInternal(configId, TRUE, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                case NVR3202X:
                case NVR3202XP2:
                {
                    /* Read hdd configuration */
                    HDD_CONFIG_t hddCfg;
                    ReadHddConfig(&hddCfg);

                    /* If recording mode is not single disk volume, raid-0 or raid-1 then default the config */
                    if (hddCfg.mode >= RAID_5)
                    {
                        /* Default hdd config due to recording mode is not supported by model */
                        EPRINT(CONFIGURATION, "recording mode is raid5 or higher: [model=%s], [mode=%d]", GetNvrModelStr(), hddCfg.mode);
                        dfltXXXConfigInternal(configId, TRUE, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case SENSOR_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    BOOL  isConfigChanged = FALSE;
                    UINT8 sensorId;

                    /* Read all sensor configuration */
                    SENSOR_CONFIG_t sensorCfg[MAX_SENSOR];
                    ReadSensorConfig(sensorCfg);

                    /* Sensor must not be enabled for this model */
                    for (sensorId = 0; sensorId < MAX_SENSOR; sensorId++)
                    {
                        /* If sensor configuration is enabled then make it disabled */
                        if (ENABLE == sensorCfg[sensorId].sensorDetect)
                        {
                            isConfigChanged = TRUE;
                            sensorCfg[sensorId].sensorDetect = DISABLE;
                        }
                    }

                    /* Is config changed? */
                    if (TRUE == isConfigChanged)
                    {
                        /* Update sensor configuration */
                        EPRINT(CONFIGURATION, "sensor config is enabled: [model=%s]", GetNvrModelStr());
                        writeXXXConfigInternal(configId, TRUE, sensorCfg, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case ALARM_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    BOOL  isConfigChanged = FALSE;
                    UINT8 alarmId;

                    /* Read all alarm configuration */
                    ALARM_CONFIG_t alarmCfg[MAX_ALARM];
                    ReadAlarmConfig(alarmCfg);

                    /* Alarm must not be enabled for this model */
                    for (alarmId = 0; alarmId < MAX_ALARM; alarmId++)
                    {
                        /* If alarm configuration is enabled then make it disabled */
                        if (ENABLE == alarmCfg[alarmId].alarmOutput)
                        {
                            isConfigChanged = TRUE;
                            alarmCfg[alarmId].alarmOutput = DISABLE;
                        }
                    }

                    /* Is config changed? */
                    if (TRUE == isConfigChanged)
                    {
                        /* Update alarm configuration */
                        EPRINT(CONFIGURATION, "alarm config is enabled: [model=%s]", GetNvrModelStr());
                        writeXXXConfigInternal(configId, TRUE, alarmCfg, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case CAMERA_EVENT_ACTION_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    BOOL                    isConfigChanged = FALSE;
                    UINT8                   cameraIndex, cameraEvent, alarmId, day, action;
                    CAMERA_EVENT_CONFIG_t   cameraEventCfg;

                    /* Read all camera's all event configuration one by one and disable alarm out config if enabled */
                    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
                    {
                        for (cameraEvent = 0; cameraEvent < MAX_CAMERA_EVENT; cameraEvent++)
                        {
                            /* Read camera event configuration */
                            isConfigChanged = FALSE;
                            ReadSingleCameraEventConfig(cameraIndex, cameraEvent, &cameraEventCfg);

                            /* Disable alarm out config in action set if enabled */
                            for (alarmId = 0; alarmId < MAX_ALARM; alarmId++)
                            {
                                if (ENABLE == cameraEventCfg.actionParam.systemAlarmOutput[alarmId])
                                {
                                    isConfigChanged = TRUE;
                                    cameraEventCfg.actionParam.systemAlarmOutput[alarmId] = DISABLE;
                                }
                            }

                            /* Disable alarm out config in action schedule if enabled */
                            for (day = 0; day < MAX_WEEK_DAYS; day++)
                            {
                                if (ENABLE == cameraEventCfg.weeklySchedule[day].entireDayAction.actionBitField.systemAlarmOutput)
                                {
                                    isConfigChanged = TRUE;
                                    cameraEventCfg.weeklySchedule[day].entireDayAction.actionBitField.systemAlarmOutput = DISABLE;
                                }

                                for (action = 0; action < MAX_EVENT_SCHEDULE; action++)
                                {
                                    if (ENABLE == cameraEventCfg.weeklySchedule[day].actionControl[action].scheduleAction.actionBitField.systemAlarmOutput)
                                    {
                                        isConfigChanged = TRUE;
                                        cameraEventCfg.weeklySchedule[day].actionControl[action].scheduleAction.actionBitField.systemAlarmOutput = DISABLE;
                                    }
                                }
                            }

                            /* Is config changed? */
                            if (TRUE == isConfigChanged)
                            {
                                /* Update camera event and action configuration */
                                EPRINT(CONFIGURATION, "alarm out config is enabled in camera event: [model=%s], [camera=%d], [event=%d]",
                                       GetNvrModelStr(), cameraIndex, cameraEvent);
                                writeXXXConfigInternal(configId, TRUE, &cameraEventCfg, SINGLE_ENTRY, cameraIndex, cameraEvent);
                            }
                        }
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case SENSOR_EVENT_ACTION_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    UINT8                   sensorEvent;
                    SENSOR_EVENT_CONFIG_t   sensorEventCfg;

                    /* Sensor event and action configuration must not enable for this model */
                    for (sensorEvent = 0; sensorEvent < MAX_SENSOR_EVENT; sensorEvent++)
                    {
                        /* Read sensor event and action config */
                        ReadSingleSensorEventConfig(sensorEvent, &sensorEventCfg);

                        /* Default sensor event and action config if it is enabled */
                        if (ENABLE == sensorEventCfg.action)
                        {
                            /* Update sensor event and action configuration */
                            EPRINT(CONFIGURATION, "sensor event config is enabled: [model=%s], [event=%d]", GetNvrModelStr(), sensorEvent);
                            dfltXXXConfigInternal(configId, TRUE, SINGLE_ENTRY, sensorEvent, 0);
                        }
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case SYSTEM_EVENT_ACTION_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    BOOL                    isConfigChanged = FALSE;
                    UINT8                   systemEvent, alarmId;
                    SYSTEM_EVENT_CONFIG_t   systemEventCfg;

                    /* Read all system event configuration one by one and disable alarm out config if enabled */
                    for (systemEvent = 0; systemEvent < MAX_SYSTEM_EVENT; systemEvent++)
                    {
                        /* Read system event configuration */
                        isConfigChanged = FALSE;
                        ReadSingleSystemEventConfig(systemEvent, &systemEventCfg);

                        /* Disable alarm out config in action if enabled */
                        if (ENABLE == systemEventCfg.actionBits.actionBitField.systemAlarmOutput)
                        {
                            isConfigChanged = TRUE;
                            systemEventCfg.actionBits.actionBitField.systemAlarmOutput = DISABLE;
                        }

                        /* Disable alarm out config in action set if enabled */
                        for (alarmId = 0; alarmId < MAX_ALARM; alarmId++)
                        {
                            if (ENABLE == systemEventCfg.actionParam.systemAlarmOutput[alarmId])
                            {
                                isConfigChanged = TRUE;
                                systemEventCfg.actionParam.systemAlarmOutput[alarmId] = DISABLE;
                            }
                        }

                        /* Is config changed? */
                        if (TRUE == isConfigChanged)
                        {
                            /* Update camera event and action configuration */
                            EPRINT(CONFIGURATION, "alarm out config is enabled in system event: [model=%s], [event=%d]", GetNvrModelStr(), systemEvent);
                            writeXXXConfigInternal(configId, TRUE, &systemEventCfg, SINGLE_ENTRY, systemEvent, 0);
                        }
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case STATIC_ROUTING_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0801XP2:
                case NVR1601XP2:
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    BOOL  isConfigChanged = FALSE;
                    UINT8 entryId, entryCnt = 0;

                    /* Read static routing configuration */
                    STATIC_ROUTING_CONFIG_t staticRoutingCfg, newRoutingCfg;
                    ReadStaticRoutingConfig(&staticRoutingCfg);

                    /* If default gateway is LAN2 then we have to remove it */
                    if (staticRoutingCfg.defaultPort == (NETWORK_PORT_LAN2 + 1))
                    {
                        WPRINT(CONFIGURATION, "default route is LAN2: [model=%s]", GetNvrModelStr());
                        staticRoutingCfg.defaultPort = (NETWORK_PORT_LAN1 + 1);
                        isConfigChanged = TRUE;
                    }

                    /* Update new routing config copy */
                    newRoutingCfg = staticRoutingCfg;

                    /* Check if LAN2 is configured in static routing then we have to remove it */
                    for (entryId = 0; entryId < MAX_STATIC_ROUTING_ENTRY; entryId++)
                    {
                        newRoutingCfg.entry[entryId] = DfltStaticRoutingCfg.entry[entryId];

                        if ((staticRoutingCfg.entry[entryId].routePort == (NETWORK_PORT_LAN2 + 1))
                                || (staticRoutingCfg.entry[entryId].routePort == staticRoutingCfg.defaultPort))
                        {
                            /* We have to ommit this entry from config */
                            EPRINT(CONFIGURATION, "LAN2 or default route is in static routing table: [model=%s], [routePort=%d]",
                                   GetNvrModelStr(), staticRoutingCfg.entry[entryId].routePort);
                            isConfigChanged = TRUE;
                            continue;
                        }

                        newRoutingCfg.entry[entryCnt++] = staticRoutingCfg.entry[entryId];
                    }

                    /* Is config changed? */
                    if (TRUE == isConfigChanged)
                    {
                        /* Update static routing configuration */
                        writeXXXConfigInternal(configId, TRUE, &newRoutingCfg, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        case DHCP_SERVER_CONFIG_ID:
        {
            switch(GetNvrModelType())
            {
                case NVR0801XP2:
                case NVR1601XP2:
                case NVR0401XSP2:
                case NVR0801XSP2:
                case NVR1601XSP2:
                {
                    /* Read dhcp server configuration */
                    DHCP_SERVER_CONFIG_t dhcpServerCfg;
                    ReadDhcpServerConfig(&dhcpServerCfg);

                    /* If dhcp server configured on LAN2 port then change it to LAN1 and disable DHCP server */
                    if (dhcpServerCfg.lanPort == LAN2_PORT)
                    {
                        EPRINT(CONFIGURATION, "DHCP server is configured on LAN2: [model=%s]", GetNvrModelStr());
                        dfltXXXConfigInternal(configId, TRUE, WHOLE_CONFIG, 0, 0);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do for other models */
                }
                break;
            }
        }
        break;

        default:
        {
            /* Nothing to do for other config */
        }
        break;
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is configuration dependent on number of cameras?
 * @param   configId
 * @return  Returns TRUE if camera dependent config else returns FALSE
 */
static BOOL isConfigCameraDependent(CONFIG_INDEX_e configId)
{
    /* Is config camera dependent? */
    switch(configId)
    {
        case CAMERA_CONFIG_ID:              // 16
        case STREAM_CONFIG_ID:              // 17
        case SCHEDULE_RECORD_CONFIG_ID:		// 18
        case ALARM_RECORD_CONFIG_ID:		// 19
        case PTZ_PRESET_CONFIG_ID:			// 20
        case PRESET_TOUR_CONFIG_ID:			// 21
        case TOUR_SCHEDULE_CONFIG_ID:		// 22
        case IMAGE_UPLOAD_CONFIG_ID:		// 23
        case CAMERA_EVENT_ACTION_CONFIG_ID: // 29
        case COSEC_PRE_RECORD_SETTINGS:		// 32
        case CAMERA_ALARM_CONFIG_ID:		// 33
        case IP_CAMERA_CONFIG_ID:			// 46
        case SNAPSHOT_CONFIG_ID:			// 52
        case SNAPSHOT_SCHEDULE_CONFIG_ID:	// 53
        case IMG_SETTING_CONFIG_ID:         // 57
            return TRUE;

        default:
            return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the type of configuration, if it has dimensions than parse gets
 *          input form variable arguments and gives it in output.
 * @param   configId
 * @param   list
 * @param   query
 * @param   xIndex
 * @param   yIndex
 */
static void decomposeQuery(UINT8 configId, va_list list, UINT8PTR query, UINT8PTR xIndex, UINT8PTR yIndex)
{
	// Initialisation
	(*query) = WHOLE_CONFIG;
	(*xIndex) = 0;
	(*yIndex) = 0;

    if ((sysConfigParam + configId)->type == CONFIG_NO_DIMENSION)
	{
        return;
    }

    (*query) = va_arg(list, INT32);
    if ((*query) != SINGLE_ENTRY)
    {
        return;
    }

    if ((sysConfigParam + configId)->type == CONFIG_TWO_DIMENSION)
    {
        (*xIndex) = va_arg(list, INT32);
        (*yIndex) = va_arg(list, INT32);
    }
    else if ((sysConfigParam + configId)->type == CONFIG_ONE_DIMENSION)
    {
        (*xIndex) = va_arg(list, INT32);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives the size of requested bytes & offset based on request.
 * @param   configId
 * @param   query
 * @param   xIndex
 * @param   yIndex
 * @param   offset
 * @param   configSize
 * @return  SUCCESS/FAIL
 */
static BOOL getConfigOffsetAndSize(UINT8 configId, UINT8 query, UINT8 xIndex, UINT8 yIndex, UINT32PTR offset, UINT32PTR configSize)
{
	(*offset) = 0;
	(*configSize) = 0;

	if ((sysConfigParam + configId)->type == CONFIG_TWO_DIMENSION)
	{
		if (query == SINGLE_ENTRY)
		{
            if (xIndex >= (sysConfigParam + configId)->maxXIndx || yIndex >= (sysConfigParam + configId)->maxYIndx)
			{
                return FAIL;
			}

            (*offset) = (((sysConfigParam + configId)->size * xIndex * (sysConfigParam + configId)->maxYIndx) + ((sysConfigParam + configId)->size * yIndex));
            (*configSize) = (sysConfigParam + configId)->size;
		}
		else
		{
			(*offset) = 0;
            (*configSize) = (((sysConfigParam + configId)->size) * ((sysConfigParam + configId)->maxXIndx) * ((sysConfigParam + configId)->maxYIndx));
		}
	}
	else if ((sysConfigParam + configId)->type == CONFIG_ONE_DIMENSION)
	{
		if (query == SINGLE_ENTRY)
		{
			if (xIndex >= (sysConfigParam + configId)->maxXIndx)
			{
                return FAIL;
			}

            (*offset) = ((sysConfigParam + configId)->size * xIndex);
            (*configSize) = (sysConfigParam + configId)->size;
		}
		else
		{
			(*offset) = 0;
            (*configSize) = ((sysConfigParam + configId)->size * (sysConfigParam + configId)->maxXIndx);
		}
	}
	else
	{
		(*offset) = 0;
		(*configSize) = (sysConfigParam + configId)->size;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function defaults the requested config index.
 * @param   configId
 * @return  Status
 */
static NET_CMD_STATUS_e dfltXXXConfig(UINT8 configId, ...)
{
    va_list list;
    UINT8   query;
    UINT8   xIndex;
    UINT8   yIndex;

	va_start(list, configId);
	decomposeQuery(configId, list, &query, &xIndex, &yIndex);
	va_end(list);
    if (SUCCESS != dfltXXXConfigInternal(configId, FALSE, query, xIndex, yIndex))
    {
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives configuration copy.
 * @param   configId
 * @param   userCopy
 * @return  SUCCESS/FAIL
 */
static BOOL readXXXConfig(UINT8 configId, VOIDPTR userCopy, ...)
{
    va_list list;
    UINT32  configSize;
    UINT32  offset;
    UINT8   query;
    UINT8   xIndex;
    UINT8   yIndex;

    // extract request parameters
	va_start(list, userCopy);
	decomposeQuery(configId, list, &query, &xIndex, &yIndex);
	va_end(list);

    // IF any of the input / output parameter is invalid. Get Offset and Size of memory to be copied
    if ((userCopy == NULL) || (getConfigOffsetAndSize(configId, query, xIndex, yIndex, &offset, &configSize) == FAIL))
	{
        EPRINT(CONFIGURATION, "invld input parameter: [file=%s], [query=%s], [xIndex=%d] [yIndex=%d]",
               (sysConfigParam + configId)->fileName, configQueryString[query], xIndex, yIndex);
        return FAIL;
	}

    // LOCK READ access to master copy
    pthread_rwlock_rdlock(&rwLock[configId]);

    // Update user copy with content of master copy
    memcpy(userCopy, ((sysConfigParam + configId)->memPtr + offset), configSize);

    // UNLOCK READ access to master copy
    pthread_rwlock_unlock(&rwLock[configId]);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function writes the requested configuration.
 * @param   configId
 * @param   userCopy
 * @return  SUCCESS/FAIL
 */
static NET_CMD_STATUS_e writeXXXConfig(UINT8 configId, VOIDPTR userCopy, ...)
{
	va_list	list;
	UINT8	query;
	UINT8	xIndex;
	UINT8	yIndex;

	va_start(list, userCopy);
	decomposeQuery(configId, list, &query, &xIndex, &yIndex);
	va_end(list);
	return writeXXXConfigInternal(configId, FALSE, userCopy, query, xIndex, yIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is internally used to default configuration.
 * @param   configId
 * @param   systemInit
 * @param   query
 * @param   xIndex
 * @param   yIndex
 * @return  SUCCESS/FAIL
 */
static BOOL dfltXXXConfigInternal(UINT8 configId, BOOL systemInit, UINT8 query, UINT8 xIndex, UINT8 yIndex)
{
    VOIDPTR memPtrLock = NULL;
    VOIDPTR memPtr = NULL;
    UINT8   loop;
    UINT8   subLoop;
    UINT32  offset;
    UINT32  configSize;

	// Get Amount of memory needed for holding this configuration
    if (FAIL == getConfigOffsetAndSize(configId, query, xIndex, yIndex, &offset, &configSize))
    {
        return FAIL;
    }

	// check if we need to allocate memory for storing configuration
	// 1) We need to modify default configuration copy
	// 2) in case of 2D or 3D configuration making whole configuration default
    if ((modifyDfltXXXConfigCopy(configId) == TRUE) || (((sysConfigParam + configId)->type != CONFIG_NO_DIMENSION) && (query != SINGLE_ENTRY)))
	{
		// If failed to get this memory return FAIL
		if ((memPtrLock = malloc(configSize)) == NULL)
		{
			return FAIL;
		}
	}

	memPtr = memPtrLock;
    /* Now we need to construct default configuration copy based on request and configuration type.
     * Some configuration needs to modify their default copy At run time. e.g for camera configuration
     * Name of the camera must be in form Camera1, Camera2, Camera3 so we have kept separate functions
     * that handles this. In case of 2D configuration we need to dump default single copy multiple
     * times to make whole configuration */
	if ((sysConfigParam + configId)->type == CONFIG_TWO_DIMENSION)
	{
		// Check whether we need to modify default copy or not
		if (modifyDfltXXXConfigCopy(configId) == FALSE)
		{
			if (query == SINGLE_ENTRY)
			{
				memPtr = (sysConfigParam + configId)->dfltCnfgPtr;
			}
			else
			{
				// Outer- LOOP for X Index
				for (loop = 0; loop < (sysConfigParam + configId)->maxXIndx; loop++)
				{
					// Inner- LOOP for Y Index
					for (subLoop = 0; subLoop < (sysConfigParam + configId)->maxYIndx; subLoop++)
					{
                        memcpy((memPtr + ((sysConfigParam + configId)->size * loop * (sysConfigParam + configId)->maxYIndx)
                                + ((sysConfigParam + configId)->size * subLoop)),
								(sysConfigParam + configId)->dfltCnfgPtr, (sysConfigParam + configId)->size);
					}
				}
			}
		}
		else
		{
			if (query == SINGLE_ENTRY)
			{
				// prepare default configuration
                prepDfltXXXConfig(configId, memPtr, (sysConfigParam + configId)->dfltCnfgPtr, xIndex, yIndex);
			}
			else
			{
				// Outer- LOOP for X Index
				for (loop = 0; loop < (sysConfigParam + configId)->maxXIndx; loop++)
				{
					// Inner- LOOP for Y Index
					for (subLoop = 0; subLoop < (sysConfigParam + configId)->maxYIndx; subLoop++)
					{
                        prepDfltXXXConfig(configId, (memPtr + ((sysConfigParam + configId)->size * loop * (sysConfigParam + configId)->maxYIndx)
                                        + ((sysConfigParam + configId)->size * subLoop)), (sysConfigParam + configId)->dfltCnfgPtr, loop, subLoop);
					}
				}
			}
		}
	}
	else if ((sysConfigParam + configId)->type == CONFIG_ONE_DIMENSION)
	{
		// Check whether we need to modify default copy or not
		if (modifyDfltXXXConfigCopy(configId) == FALSE)
		{
			if (query == SINGLE_ENTRY)
			{
				memPtr = (sysConfigParam + configId)->dfltCnfgPtr;
			}
			else
			{
				for (loop = 0; loop < (sysConfigParam + configId)->maxXIndx; loop++)
				{
                    memcpy((memPtr + (sysConfigParam + configId)->size *loop),
                           (sysConfigParam + configId)->dfltCnfgPtr, (sysConfigParam + configId)->size);
				}
			}
		}
		else
		{
			if (query == SINGLE_ENTRY)
			{
				// prepare default configuration
                prepDfltXXXConfig(configId, memPtr, (sysConfigParam + configId)->dfltCnfgPtr, xIndex, yIndex);
			}
			else
			{
				for (loop = 0; loop < (sysConfigParam + configId)->maxXIndx; loop++)
				{
                    prepDfltXXXConfig(configId, (memPtr + (sysConfigParam + configId)->size * loop),
                                      (sysConfigParam + configId)->dfltCnfgPtr, loop, yIndex);
				}
			}
		}
	}
	else
	{
		// Check whether we need to modify default copy or not
		if (modifyDfltXXXConfigCopy(configId) == FALSE)
		{
			memPtr = (sysConfigParam + configId)->dfltCnfgPtr;
		}
		else
		{
			// prepare default configuration
            prepDfltXXXConfig(configId, memPtr, (sysConfigParam + configId)->dfltCnfgPtr, xIndex, yIndex);
		}
	}

	// Write this copy to file
    if (writeXXXConfigInternal(configId, systemInit, memPtr, query, xIndex, yIndex) != CMD_SUCCESS)
	{
        /* Free memory if allocated */
        FREE_MEMORY(memPtrLock);
        return FAIL;
	}

    /* Free memory if allocated */
    FREE_MEMORY(memPtrLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is internally used for writing configuration.
 * @param   configId
 * @param   systemInit
 * @param   userCopy
 * @param   query
 * @param   xIndex
 * @param   yIndex
 * @return  Status
 */
static NET_CMD_STATUS_e	writeXXXConfigInternal(UINT8 configId, BOOL systemInit, VOIDPTR userCopy, UINT8 query, UINT8 xIndex, UINT8 yIndex)
{
	UINT32				configSize;
	UINT32				offset;
	INT32 				fd;
    VOIDPTR				oldCopy = NULL;
	CHAR				filePath[100];
    NET_CMD_STATUS_e	retVal = CMD_PROCESS_ERROR;
    BOOL				notifyModule = FALSE;

    // IF any of the input / output parameter is invalid. Get configuration size and offset
    if ((userCopy == NULL) || (getConfigOffsetAndSize(configId, query, xIndex, yIndex, &offset, &configSize) == FAIL) || (configId >= MAX_CONFIG_ID))
	{
        EPRINT(CONFIGURATION, "invld input parameter: [file=%s], [query=%s], [xIndex=%d] [yIndex=%d]",
               (sysConfigParam + configId)->fileName, configQueryString[query], xIndex, yIndex);
        return CMD_PROCESS_ERROR;
	}

    // Allocate memory for preserving oldCopy to notify other modules. Don't write configuration if failed to allocate memory
    else if ((systemInit == FALSE) && ((oldCopy = malloc(configSize)) == NULL))
	{
        EPRINT(CONFIGURATION, "fail to alloc memory: [file=%s]", (sysConfigParam + configId)->fileName);
        return CMD_PROCESS_ERROR;
	}

    // LOCK Write access to master copy
    pthread_rwlock_wrlock(&rwLock[configId]);

    // Don't compare User Copy with master copy At System Init. For other case IF master copy differs from user copy
    if ((systemInit == TRUE) || (memcmp(((sysConfigParam + configId)->memPtr + offset), userCopy, configSize) != STATUS_OK))
    {
        if (systemInit == FALSE)
        {
            // Copy Old copy to temporary buffer
            memcpy(oldCopy, ((sysConfigParam + configId)->memPtr + offset), configSize);
            // UNLOCK Write access to master copy
            pthread_rwlock_unlock(&rwLock[configId]);

            retVal = validateXXXConfigInternal(configId, userCopy, oldCopy, xIndex, yIndex);
            // LOCK Write access to master copy
            pthread_rwlock_wrlock(&rwLock[configId]);
        }
        else
        {
            retVal = CMD_SUCCESS;
        }

        // don't need to Pre-Notify at system init
        if (retVal == CMD_SUCCESS)
        {
            retVal = CMD_PROCESS_ERROR;

            snprintf(filePath, sizeof(filePath), APP_CONFIG_DIR_PATH "/%s" CONFIG_FILE_EXT, (sysConfigParam + configId)->fileName);

            // IF failed to open file in read / write mode
            if ((fd = open(filePath, READ_WRITE_SYNC_MODE)) == INVALID_FILE_FD)
            {
                EPRINT(CONFIGURATION, "fail to open file: [file=%s], [err=%s]", (sysConfigParam + configId)->fileName, STR_ERR);
            }
            else
            {
                /* Skip config file version number size (4 bytes) */
                lseek(fd, (4 + offset), SEEK_SET);

                // IF failed to write indexed master copy to file
                if (write(fd, userCopy, configSize) != (INT32)configSize)
                {
                    EPRINT(CONFIGURATION, "fail to write file: [file=%s], [err=%s]", (sysConfigParam + configId)->fileName, STR_ERR);
                }
                else
                {
                    // Configuration Successfully written
                    retVal = CMD_SUCCESS;

                    // In case of System Initialisation, don't notify other modules as they might not be initialised yet.
                    if (systemInit == FALSE)
                    {
                        // notify respective module
                        notifyModule = TRUE;
                    }

                    // Update master copy with the content of user copy
                    memcpy(((sysConfigParam + configId)->memPtr + offset), userCopy, configSize);
                }

                // Close file
                close(fd);
            }
        }
        else
        {
            EPRINT(CONFIGURATION, "config validation fail: [file=%s]", (sysConfigParam + configId)->fileName);
        }
    }
    else
    {
        retVal = CMD_SUCCESS;
    }

    // UNLOCK Write access to master copy
    pthread_rwlock_unlock(&rwLock[configId]);

	// Send configuration change notification
	if (notifyModule == TRUE)
	{
		// We don't need to iterate in case of writing single entry / CONFIG_NO_DIMENSION
        if ((query == SINGLE_ENTRY) || ((sysConfigParam + configId)->type == CONFIG_NO_DIMENSION))
		{
			notifyXXXConfigInternal(configId, userCopy, oldCopy, xIndex, yIndex);
		}
		else
		{
			UINT8 tmpXIndex;
			UINT8 tmpYIndex;

			if ((sysConfigParam + configId)->type == CONFIG_ONE_DIMENSION)
			{
				// Iteration for X Dimension
				for (tmpXIndex = 0; tmpXIndex < (sysConfigParam + configId)->maxXIndx; tmpXIndex++)
				{
                    notifyXXXConfigInternal(configId, (userCopy + (tmpXIndex * (sysConfigParam + configId)->size)),
                                            (oldCopy + (tmpXIndex * (sysConfigParam + configId)->size)), tmpXIndex, yIndex);
				}
			}
			else if ((sysConfigParam + configId)->type == CONFIG_TWO_DIMENSION)
			{
				// Iteration for X Dimension
				for (tmpXIndex = 0; tmpXIndex < (sysConfigParam + configId)->maxXIndx; tmpXIndex++)
				{
					// Iteration for Y Dimension
					for (tmpYIndex = 0; tmpYIndex < (sysConfigParam + configId)->maxYIndx; tmpYIndex++)
                    {
                        notifyXXXConfigInternal(configId, (userCopy + (tmpXIndex * (sysConfigParam + configId)->maxYIndx *
                                        (sysConfigParam + configId)->size) + (tmpYIndex * (sysConfigParam + configId)->size)),
                                        (oldCopy + (tmpXIndex * (sysConfigParam + configId)->size)), tmpXIndex, tmpYIndex);
					}
				}
			}
		}
	}

	// Free allocated memory for storing old Configuration
    FREE_MEMORY(oldCopy);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns whether we need to modify default configuration copy or not.
 * @param   configId
 * @return  TRUE/FALSE
 */
static BOOL modifyDfltXXXConfigCopy(UINT8 configId)
{
	switch(configId)
	{
		// only this configuration need to be changed
        case USER_ACCOUNT_CONFIG_ID:
        case CAMERA_CONFIG_ID:
        case ALARM_CONFIG_ID:
        case PRESET_TOUR_CONFIG_ID:
        case SENSOR_CONFIG_ID:
        case DHCP_SERVER_CONFIG_ID:
        case IMG_SETTING_CONFIG_ID:
            return TRUE;

        default:
            return FALSE;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares default configuration copy of for requested configuration.
 * @param   configId
 * @param   userCopy
 * @param   dfltCopy
 * @param   xIndex
 * @param   yIndex
 */
static void prepDfltXXXConfig(UINT8 configId, VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 xIndex, UINT8 yIndex)
{
	switch(configId)
	{
        case USER_ACCOUNT_CONFIG_ID:
            prepDfltSingleUserAccCfg(userCopy, dfltCopy, xIndex);
            break;

        case CAMERA_CONFIG_ID:
            prepDfltSingleCameraCfg(userCopy, dfltCopy, xIndex);
            break;

        case ALARM_CONFIG_ID:
            prepDfltSingleAlarmCfg(userCopy, dfltCopy, xIndex);
            break;

        case PRESET_TOUR_CONFIG_ID:
            prepDfltSinglePresetTourCfg(userCopy, dfltCopy, xIndex);
            break;

        case SENSOR_CONFIG_ID:
            prepDfltSingleSensorCfg(userCopy, dfltCopy, xIndex);
            break;

        case DHCP_SERVER_CONFIG_ID:
            prepDfltDhcpServerCfg(userCopy, dfltCopy);
            break;

        case IMG_SETTING_CONFIG_ID:
            prepDfltSingleImageSettingCfg(userCopy, dfltCopy, xIndex);
            break;

        default:
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares Default User Account configuration.
 * @param   userCopy
 * @param   dfltCopy
 * @param   userIndex
 */
static void prepDfltSingleUserAccCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 userIndex)
{
    UINT8 camIndx;

	memcpy(userCopy, dfltCopy, sizeof(USER_ACCOUNT_CONFIG_t));
    USER_ACCOUNT_CONFIG_t *userAccount = (USER_ACCOUNT_CONFIG_t *)userCopy;

	switch(userIndex)
	{
        case USER_LOCAL:
            snprintf(userAccount->username, MAX_USER_ACCOUNT_USERNAME_WIDTH, LOCAL_USER_NAME);
            snprintf(userAccount->password, MAX_USER_ACCOUNT_PASSWORD_WIDTH, LOCAL_USER_PSWD);
            userAccount->userGroup = VIEWER;
            userAccount->userStatus = ENABLE;

            // As Local User don't have privilege for audio, play back, PTZ by default
            for(camIndx = 0; camIndx < getMaxCameraForCurrentVariant(); camIndx++)
            {
                userAccount->userPrivilege[camIndx].privilegeBitField.audio = DISABLE;
                userAccount->userPrivilege[camIndx].privilegeBitField.playback = DISABLE;
                userAccount->userPrivilege[camIndx].privilegeBitField.ptzControl = DISABLE;
                userAccount->userPrivilege[camIndx].privilegeBitField.videoPopup = DISABLE;
                userAccount->userPrivilege[camIndx].privilegeBitField.audioOut = DISABLE;
            }
            break;

        case USER_ADMIN:
            snprintf(userAccount->username, MAX_USER_ACCOUNT_USERNAME_WIDTH, ADMIN_USER_NAME);
            snprintf(userAccount->password, MAX_USER_ACCOUNT_PASSWORD_WIDTH, ADMIN_USER_PSWD);
            userAccount->userGroup = ADMIN;
            userAccount->userStatus = ENABLE;
            userAccount->managePushNotificationRights = ENABLE;
            break;

        case USER_OPERATOR:
            snprintf(userAccount->username, MAX_USER_ACCOUNT_USERNAME_WIDTH, OPERATOR_USER_NAME);
            snprintf(userAccount->password, MAX_USER_ACCOUNT_PASSWORD_WIDTH, OPERATOR_USER_PSWD);
            userAccount->userGroup = OPERATOR;
            userAccount->userStatus = DISABLE;
            break;

        case USER_VIEWER:
            snprintf(userAccount->username, MAX_USER_ACCOUNT_USERNAME_WIDTH, VIEWER_USER_NAME);
            snprintf(userAccount->password, MAX_USER_ACCOUNT_PASSWORD_WIDTH, VIEWER_USER_NAME);
            userAccount->userGroup = VIEWER;
            userAccount->userStatus = DISABLE;
            break;

        default:
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares Default Camera configuration.
 * @param   userCopy
 * @param   dfltCopy
 * @param   camIndex
 */
static void prepDfltSingleCameraCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 camIndex)
{
    CAMERA_CONFIG_t *camera = (CAMERA_CONFIG_t *)userCopy;
    memcpy(userCopy, dfltCopy, sizeof(CAMERA_CONFIG_t));
    snprintf(camera->name, MAX_CAMERA_NAME_WIDTH, "Camera-%d", camIndex + 1);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares Default Sensor configuration.
 * @param   userCopy
 * @param   dfltCopy
 * @param   xIndex
 */
static void prepDfltSingleSensorCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 xIndex)
{
    SENSOR_CONFIG_t *sensorPtr = (SENSOR_CONFIG_t *)userCopy;
	memcpy(userCopy, dfltCopy, sizeof(SENSOR_CONFIG_t));
    snprintf(sensorPtr->name, MAX_SENSOR_NAME_WIDTH, "Sensor-%d", xIndex + 1);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares default dhcp server.
 * @param   userCopy
 * @param   dfltCopy
 */
static void prepDfltDhcpServerCfg(VOIDPTR userCopy, VOIDPTR dfltCopy)
{
    DHCP_SERVER_CONFIG_t *pDhcpServerCfg = (DHCP_SERVER_CONFIG_t *)userCopy;
    memcpy(userCopy, dfltCopy, sizeof(DHCP_SERVER_CONFIG_t));
    pDhcpServerCfg->noOfHost = getMaxCameraForCurrentVariant();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares Default Alarm configuration.
 * @param   userCopy
 * @param   dfltCopy
 * @param   xIndex
 */
static void prepDfltSingleAlarmCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 xIndex)
{
    ALARM_CONFIG_t *alarmPtr = (ALARM_CONFIG_t *)userCopy;
	memcpy(userCopy, dfltCopy, sizeof(ALARM_CONFIG_t));
    snprintf(alarmPtr->name, MAX_ALARM_NAME_WIDTH, "Alarm-%d", xIndex + 1);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares Default Preset Tour configuration.
 * @param   userCopy
 * @param   dfltCopy
 * @param   cameraIndex
 */
static void prepDfltSinglePresetTourCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 cameraIndex)
{
    UINT8 tourIndex;

	memcpy(userCopy, dfltCopy, sizeof(PRESET_TOUR_CONFIG_t));
    PRESET_TOUR_CONFIG_t *presetTour = (PRESET_TOUR_CONFIG_t *)userCopy;
	for(tourIndex = 0; tourIndex < MAX_TOUR_NUMBER; tourIndex++)
	{
        snprintf(presetTour->tour[tourIndex].name, MAX_TOUR_NAME_WIDTH, "Tour-%d", tourIndex + 1);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares default image setting configuration.
 * @param   userCopy
 * @param   dfltCopy
 * @param   cameraIndex
 */
static void prepDfltSingleImageSettingCfg(VOIDPTR userCopy, VOIDPTR dfltCopy, UINT8 cameraIndex)
{
    IP_CAMERA_CONFIG_t ipCameraCfg;

    /* Get IP camera config to check ONVIF support */
    ReadSingleIpCameraConfig(cameraIndex, &ipCameraCfg);

    /* Copy default image setting configuration */
    memcpy(userCopy, dfltCopy, sizeof(IMG_SETTING_CONFIG_t));

    /* If camera added through BM/CI then change backlight mode to DWDR */
    if (ipCameraCfg.onvifSupportF == FALSE)
    {
        ((IMG_SETTING_CONFIG_t*)userCopy)->backlightMode = BACKLIGHT_MODE_DWDR;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function validates User Account Configuration change.
 * @param   newUsrAccontCfg
 * @param   oldUsrAccontCfg
 * @param   userIndex
 * @return
 */
static NET_CMD_STATUS_e validateSingleUserAccCfg(USER_ACCOUNT_CONFIG_t *newUsrAccontCfg, USER_ACCOUNT_CONFIG_t *oldUsrAccontCfg, UINT8 userIndex)
{
	switch(userIndex)
	{
        case USER_LOCAL:
            if ((strcmp(newUsrAccontCfg->username, oldUsrAccontCfg->username) != STATUS_OK)
                    || (strcmp(newUsrAccontCfg->password, oldUsrAccontCfg->password) != STATUS_OK)
                    || (newUsrAccontCfg->multiLogin != oldUsrAccontCfg->multiLogin)
                    || (newUsrAccontCfg->userGroup != oldUsrAccontCfg->userGroup)
                    || (newUsrAccontCfg->userStatus != oldUsrAccontCfg->userStatus))
            {
                EPRINT(CONFIGURATION, "local param mismatch");
                return CMD_PROCESS_ERROR;
            }
            break;

        case USER_ADMIN:
            if ((strcmp(newUsrAccontCfg->username, oldUsrAccontCfg->username) != STATUS_OK)
                    || (newUsrAccontCfg->userGroup != oldUsrAccontCfg->userGroup)
                    || (newUsrAccontCfg->userStatus != oldUsrAccontCfg->userStatus))
            {
                EPRINT(CONFIGURATION, "admin param mismatch");
                return CMD_PROCESS_ERROR;
            }

            if (memcmp((VOIDPTR)newUsrAccontCfg->userPrivilege, (VOIDPTR)oldUsrAccontCfg->userPrivilege,
                    ((getMaxCameraForCurrentVariant())*sizeof(newUsrAccontCfg->userPrivilege[0].privilegeGroup))) != STATUS_OK)
            {
                EPRINT(CONFIGURATION, "admin previlidge mismatch");
                return CMD_PROCESS_ERROR;
            }
            break;

        case USER_OPERATOR:
            if ((strcmp(newUsrAccontCfg->username, oldUsrAccontCfg->username) != STATUS_OK)
                    || (newUsrAccontCfg->userGroup != oldUsrAccontCfg->userGroup))
            {
                EPRINT(CONFIGURATION, "operator param mismatch");
                return CMD_PROCESS_ERROR;
            }

            if (memcmp((VOIDPTR)newUsrAccontCfg->userPrivilege, (VOIDPTR)oldUsrAccontCfg->userPrivilege,
                    ((getMaxCameraForCurrentVariant())*sizeof(newUsrAccontCfg->userPrivilege[0].privilegeGroup))) != STATUS_OK)
            {
                EPRINT(CONFIGURATION, "operator previlidge mismatch");
                return CMD_PROCESS_ERROR;
            }
            break;

        case USER_VIEWER:
            if ((strcmp(newUsrAccontCfg->username, oldUsrAccontCfg->username) != STATUS_OK)
                    || (newUsrAccontCfg->userGroup != oldUsrAccontCfg->userGroup))
            {
                EPRINT(CONFIGURATION, "viewer param mismatch");
                return CMD_PROCESS_ERROR;
            }
            break;

        default:
            break;
	}

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function validates configuration.
 * @param   configId
 * @param   newCopy
 * @param   oldCopy
 * @param   xIndex
 * @param   yIndex
 * @return  Status
 */
static NET_CMD_STATUS_e validateXXXConfigInternal(UINT8 configId, VOIDPTR newCopy, VOIDPTR oldCopy,	UINT8 xIndex, UINT8 yIndex)
{
	switch(configId)
	{
        case LAN1_CONFIG_ID:
            return ValidateLanConfig(LAN1_PORT, *((LAN_CONFIG_t *)newCopy));

        case LAN2_CONFIG_ID:
            return ValidateLanConfig(LAN2_PORT, *((LAN_CONFIG_t *)newCopy));

        case HDD_CONFIG_ID:
            return ValidateHddConfig(*((HDD_CONFIG_t *)newCopy), oldCopy);

        case CAMERA_CONFIG_ID:
            return ValidateCameraCfg((CAMERA_CONFIG_t *)newCopy, (CAMERA_CONFIG_t *)oldCopy);

        case STATIC_ROUTING_CONFIG_ID:
            return ValidateStaticRoutingCfg(*((STATIC_ROUTING_CONFIG_t *)newCopy));

        case USER_ACCOUNT_CONFIG_ID:
            return validateSingleUserAccCfg((USER_ACCOUNT_CONFIG_t *)newCopy, (USER_ACCOUNT_CONFIG_t *)oldCopy, xIndex);

        case DHCP_SERVER_CONFIG_ID:
            return ValidateDhcpServerCfg(*((DHCP_SERVER_CONFIG_t *)newCopy));

        case PTZ_PRESET_CONFIG_ID:
            return ValidatePtzPresetCfg((PTZ_PRESET_CONFIG_t *)newCopy, xIndex);

        default:
            break;
	}

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function notifies configuration change.
 * @param   configId
 * @param   newCopy
 * @param   oldCopy
 * @param   xIndex
 * @param   yIndex
 */
static void notifyXXXConfigInternal(UINT8 configId,	VOIDPTR newCopy, VOIDPTR oldCopy, UINT8 xIndex, UINT8 yIndex)
{
	switch(configId)
	{
        case GENERAL_CONFIG_ID:
            GENERAL_CONFIG_CHANGE(*((GENERAL_CONFIG_t *)newCopy), oldCopy);
            break;

        case DATE_TIME_CONFIG_ID:
            DATE_TIME_CONFIG_CHANGE(*((DATE_TIME_CONFIG_t *)newCopy), oldCopy);
            break;

        case DST_CONFIG_ID:
            DST_CONFIG_CHANGE(*((DST_CONFIG_t *)newCopy));
            break;

        case LAN1_CONFIG_ID:
            LAN_CONFIG_CHANGE(LAN1_PORT, *((LAN_CONFIG_t *)newCopy), oldCopy);
            break;

        case LAN2_CONFIG_ID:
            LAN_CONFIG_CHANGE(LAN2_PORT, *((LAN_CONFIG_t *)newCopy), oldCopy);
            break;

        case DDNS_CONFIG_ID:
            DDNS_CONFIG_CHANGE(*((DDNS_CONFIG_t *)newCopy), oldCopy);
            break;

        case SMTP_CONFIG_ID:
            SMTP_CONFIG_CHANGE(*((SMTP_CONFIG_t *)newCopy), oldCopy);
            break;

        case FTP_UPLOAD_CONFIG_ID:
            FTP_UPLOAD_CONFIG_CHANGE(xIndex);
            break;

        case TCP_NOTIFY_CONFIG_ID:
            TCP_NOTIFY_CONFIG_CHANGE(*((TCP_NOTIFY_CONFIG_t *)newCopy), oldCopy);
            break;

        case FILE_ACCESS_CONFIG_ID:
            FILE_ACCESS_CONFIG_CHANGE(*((FILE_ACCESS_CONFIG_t *)newCopy), oldCopy);
            break;

        case HDD_CONFIG_ID:
            DM_HDD_CONFIG_CHANGE(*((HDD_CONFIG_t *)newCopy), oldCopy);
            break;

        case MATRIX_DNS_SERVER_CONFIG_ID:
            MATRIX_DNS_SERVER_CONFIG_CHANGE((MATRIX_DNS_SERVER_CONFIG_t *)newCopy, (MATRIX_DNS_SERVER_CONFIG_t *)oldCopy);
            break;

        case USER_ACCOUNT_CONFIG_ID:
            USER_ACCOUNT_CONFIG_CHANGE(*((USER_ACCOUNT_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case CAMERA_CONFIG_ID:
            CAMERA_CONFIG_CHANGE(*((CAMERA_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case STREAM_CONFIG_ID:
            STREAM_CONFIG_CHANGE(((STREAM_CONFIG_t *)newCopy), ((STREAM_CONFIG_t *)oldCopy), xIndex);
            break;

        case PTZ_PRESET_CONFIG_ID:
            PTZ_PRESET_CONFIG_CHANGE(*((PTZ_PRESET_CONFIG_t*)newCopy), (PTZ_PRESET_CONFIG_t*)oldCopy, xIndex, yIndex);
            break;

        case PRESET_TOUR_CONFIG_ID:
            PRESET_TOUR_CONFIG_CHANGE(xIndex, *((PRESET_TOUR_CONFIG_t *)newCopy), oldCopy);
            break;

        case TOUR_SCHEDULE_CONFIG_ID:
            TOUR_SCHEDULE_CONFIG_CHANGE(xIndex, *((TOUR_SCHEDULE_CONFIG_t *)newCopy), oldCopy);
            break;

        case SENSOR_CONFIG_ID:
            SENSOR_CONFIG_CHANGE(*((SENSOR_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case ALARM_CONFIG_ID:
            ALARM_CONFIG_CHANGE(*((ALARM_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case STORAGE_CONFIG_ID:
            STORAGE_CONFIG_CHANGE(*((STORAGE_CONFIG_t *)newCopy), oldCopy);
            break;

        case SCHEDULE_BACKUP_CONFIG_ID:
            SCHEDULE_BACKUP_CONFIG_CHANGE(((SCHEDULE_BACKUP_CONFIG_t *)newCopy), ((SCHEDULE_BACKUP_CONFIG_t *)oldCopy));
            break;

        case MANUAL_BACKUP_CONFIG_ID:
            MANUAL_BACKUP_CONFIG_CHANGE(*((MANUAL_BACKUP_CONFIG_t *) newCopy));
            break;

        case CAMERA_EVENT_ACTION_CONFIG_ID:
            CAMERA_EVENT_CONFIG_CHANGE(*((CAMERA_EVENT_CONFIG_t *)newCopy), oldCopy, xIndex, yIndex);
            break;

        case SENSOR_EVENT_ACTION_CONFIG_ID:
            SENSOR_EVENT_CONFIG_CHANGE(*((SENSOR_EVENT_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case SYSTEM_EVENT_ACTION_CONFIG_ID:
            SYSTEM_EVENT_CONFIG_CHANGE(*((SYSTEM_EVENT_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case CAMERA_ALARM_CONFIG_ID:
            CAMERA_ALARM_CONFIG_CHANGE(*((CAMERA_ALARM_CONFIG_t *)newCopy), oldCopy, xIndex, yIndex);
            break;

        case STATIC_ROUTING_CONFIG_ID:
            STATIC_ROUTE_CONFIG_CHANGE(*((STATIC_ROUTING_CONFIG_t *)newCopy), oldCopy);
            break;

        case BROAD_BAND_CONFIG_ID:
            BROAD_BAND_CONFIG_CHANGE(*((BROAD_BAND_CONFIG_t *)newCopy), oldCopy);
            break;

        case MANUAL_RECORD_CONFIG_ID:
            MANUAL_RECORD_CONFIG_CHANGE(*((MANUAL_RECORD_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case NETWORK_DRIVE_SETTINGS:
            NETWORK_DRIVE_CONFIG_CHANGE(*((NETWORK_DRIVE_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case IP_CAMERA_CONFIG_ID:
            IP_CAMERA_CONFIG_CHANGE((* (IP_CAMERA_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case LOGIN_POLICY_CONFIG_ID:
            LOGIN_POLICY_CONFIG_CHANGE((* (LOGIN_POLICY_CONFIG_t *)newCopy), oldCopy);
            break;

        case P2P_CONFIG_ID:
            P2P_CONFIG_CHANGE((*(P2P_CONFIG_t *)newCopy), oldCopy);
            break;

        case IMG_SETTING_CONFIG_ID:
            IMAGE_SETTING_CONFIG_CHANGE((*(IMG_SETTING_CONFIG_t *)newCopy), oldCopy, xIndex);
            break;

        case DHCP_SERVER_CONFIG_ID:
            DHCP_SERVER_CONFIG_CHANGE((*(DHCP_SERVER_CONFIG_t *)newCopy), oldCopy);
            break;

        case FIRMWARE_MANAGEMENT_CONFIG_ID:
            FIRMWARE_MANAGEMENT_CONFIG_CHANGE((*(FIRMWARE_MANAGEMENT_CONFIG_t *)newCopy), oldCopy);
            break;

        case FCM_PUSH_NOTIFICATION_CONFIG_ID:
            FCM_PUSH_NOTIFY_CONFIG_CHANGE(*((FCM_PUSH_NOTIFY_CONFIG_t *)newCopy), oldCopy);
            break;

        case STORAGE_ALLOCATION_CONFIG_ID:
            STORAGE_ALLOCATION_CONFIG_CHANGE(*((STORAGE_ALLOCATION_CONFIG_t *)newCopy), oldCopy);
            break;

        default:
            break;
	}
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
