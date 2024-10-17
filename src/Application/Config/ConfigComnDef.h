#if !defined CONFIGURATIONCOMMONDEF_H
#define CONFIGURATIONCOMMONDEF_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   ConfigComnDef.h
@brief  File contains Configuration data structures [structure, macro and enum] of NVR
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DeviceDefine.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_CAMERA_CONFIG       (96)
#define CAMERA_CONFIG_MAX_V1    (64)    // 96 channel NVR support (camera config changed from 64 to 96)
#define VERSION_OFFSET          (1)
#define APP_CONFIG_DIR_PATH     CONFIG_DIR_PATH "/appConfig"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    TBL_GENERAL_CFG,					// tableId 01
    TBL_DATE_TIME_CFG,					// tableId 02
    TBL_DST_CFG,						// tableId 03
    TBL_RS232_CFG,						// tableId 04
    TBL_LAN1_CFG,						// tableId 05
    TBL_LAN2_CFG,						// tableId 06
    TBL_IP_FILTER_ENABLE_CFG,			// tableId 07
    TBL_IP_FILTER_CFG,					// tableId 08
    TBL_DDNS_CFG,						// tableId 09
    TBL_SMTP_CFG,						// tableId 10
    TBL_FTP_CFG,						// tableId 11
    TBL_TCP_NOTIFY_CFG,					// tableId 12
    TBL_FILE_ACCESS_CFG,				// tableId 13
    TBL_HDD_CFG,						// tableId 14
    TBL_MATRIX_DNS_SERVER_CFG,			// tableId 15
    TBL_USER_ACCOUNT_CFG,				// tableId 16
    TBL_CAMERA_CFG,						// tableId 17
    TBL_STREAM_CFG,						// tableId 18
    TBL_ENABLE_SCHEDULE_RECORD_CFG,		// tableId 19
    TBL_SCHEDULE_RECORD_CFG,			// tableId 20
    TBL_ALARM_RECORD_CFG,				// tableId 21
    TBL_PRESET_POSITION_CFG,			// tableId 22
    TBL_MANUAL_PRESET_TOUR_CFG,			// tableId 23
    TBL_AUTO_PRESET_TOUR_CFG,			// tableId 24
    TBL_PRESET_TOUR_CFG,				// tableId 25
    TBL_SYS_SENOR_CFG,					// tableId 26
    TBL_SYS_ALARM_CFG,					// tableId 27
    TBL_UPLOAD_IMAGE_CFG,				// tableId 28
    TBL_STORAGE_CFG,					// tableId 29
    TBL_SCHEDULE_BACKUP_CFG,			// tableId 30
    TBL_MANUAL_BACKUP_CFG,				// tableId 31
    TBL_CAMERA_EVENT_ACTION_PARAM_CFG,	// tableId 32
    TBL_CAMERA_EVENT_SCHEDUL_CFG,		// tableId 33
    TBL_SENSOR_EVENT_ACTION_PARAM_CFG,	// tableId 34
    TBL_SENSOR_EVENT_SCHEDULE_CFG,		// tableId 35
    TBL_SYSTEM_EVENT_ACTION_PARAM_CFG,	// tableId 36
    TBL_COSEC_RECORD_CFG,				// tableId 37
    TBL_CAMERA_ALARM_CFG,				// tableId 38
    TBL_DEFAULT_ROUTING_CFG,			// tableId 39
    TBL_STATIC_ROUTING_CFG,				// tableID 40
    TBL_BROADBAND_PROFILE_CFG,			// tableID 41
    TBL_BROADBAND_CFG,					// tableID 42
    TBL_SMS_CFG,						// tableID 43
    TBL_MANUAL_RECORD_CFG,				// tableID 44
    TBL_NW_DRIVE_CFG,					// tableID 45
    TBL_IP_CAMERA_CFG,					// tableID 46
    TBL_ANALOG_CAMERA_CFG,				// tableID 47
    TBL_PTZ_INTERFACE_CFG,				// tableID 48
    TBL_AUDIO_CFG,						// tableID 49
    TBL_DEVICE_INFO_CLIENT_CFG,		    // tableID 50
    TBL_NETWORK_DEVICE_CFG,			    // tableID 51
    TBL_SNAPSHOT_CFG,					// tableID 52
    TBL_SNAPSHOT_SCHEDULE_CFG,			// tableID 53
    TBL_LOGIN_POLICY_CFG,				// tableID 54
    TBL_AUDIO_OUT_CFG,					// tableID 55
    TBL_P2P_CFG,                        // tableID 56
    TBL_IMG_SETTING_CFG,				// tableID 57
    TBL_DHCP_SERVER_CFG,                // tableID 58
    TBL_FIRMWARE_MANAGEMENT_CFG,        // tableID 59
    TBL_FCM_PUSH_NOTIFICATION_CFG,      // tableID 60
    TBL_PASSWORD_RECOVERY_CFG,          // tableID 61
    TBL_STORAGE_ALLOCATION_CFG,         // tableID 62
    MAX_TBL_CFG
}TBL_CFG_e;

typedef enum
{
    GENERAL_CONFIG_ID = 0,
    DATE_TIME_CONFIG_ID,
    DST_CONFIG_ID,
    LAN1_CONFIG_ID,
    LAN2_CONFIG_ID,
    MAC_SERVER_CONFIG_ID,
    IP_FILTER_CONFIG_ID,
    DDNS_CONFIG_ID,
    SMTP_CONFIG_ID,
    FTP_UPLOAD_CONFIG_ID,
    TCP_NOTIFY_CONFIG_ID,
    FILE_ACCESS_CONFIG_ID,
    HDD_CONFIG_ID,
    MATRIX_DNS_SERVER_CONFIG_ID,
    USER_ACCOUNT_CONFIG_ID,
    CAMERA_CONFIG_ID,
    STREAM_CONFIG_ID,
    SCHEDULE_RECORD_CONFIG_ID,
    ALARM_RECORD_CONFIG_ID,
    PTZ_PRESET_CONFIG_ID,
    PRESET_TOUR_CONFIG_ID,
    TOUR_SCHEDULE_CONFIG_ID,
    IMAGE_UPLOAD_CONFIG_ID,
    SENSOR_CONFIG_ID,
    ALARM_CONFIG_ID,
    STORAGE_CONFIG_ID,
    SCHEDULE_BACKUP_CONFIG_ID,
    MANUAL_BACKUP_CONFIG_ID,
    CAMERA_EVENT_ACTION_CONFIG_ID,
    SENSOR_EVENT_ACTION_CONFIG_ID,
    SYSTEM_EVENT_ACTION_CONFIG_ID,
    COSEC_PRE_RECORD_SETTINGS,
    CAMERA_ALARM_CONFIG_ID,
    STATIC_ROUTING_CONFIG_ID,
    BROAD_BAND_CONFIG_ID,
    SMS_CONFIG_ID,
    MANUAL_RECORD_CONFIG_ID,
    NETWORK_DRIVE_SETTINGS,
    IP_CAMERA_CONFIG_ID,
    NETWORK_DEVICES_CONFIG_ID,
    SNAPSHOT_CONFIG_ID,
    SNAPSHOT_SCHEDULE_CONFIG_ID,
    LOGIN_POLICY_CONFIG_ID,
    AUDIO_OUT_CONFIG_ID,
    P2P_CONFIG_ID,
    IMG_SETTING_CONFIG_ID,
    DHCP_SERVER_CONFIG_ID,
    FIRMWARE_MANAGEMENT_CONFIG_ID,
    FCM_PUSH_NOTIFICATION_CONFIG_ID,
    PASSWORD_RECOVERY_CONFIG_ID,
    STORAGE_ALLOCATION_CONFIG_ID,
    MAX_CONFIG_ID

}CONFIG_INDEX_e;

typedef enum LOCAL_CONFIG_INDEX_e
{
    LOCAL_AUDIO_CONFIG_ID = 0,
    LOCAL_DISPLAY_CONFIG_ID,
    LOCAL_OTHER_LOGIN_CONFIG_ID,
	LOCAL_WIZ_OTHER_LOGIN_CONFIG_ID,
	MAX_LOCAL_CONFIG_ID

}LOCAL_CONFIG_INDEX_e;

/** ******************************************************************************************* **/
/**                                  GENERAL SETTINGS                                           **/
/** ******************************************************************************************* **/
#define DEVICE_NAME                 "Matrix-NVR"
#define MAX_DEVICE_NAME_WIDTH_V1	(25)
#define MAX_DEVICE_NAME_WIDTH		(61) // UTF-8 MultiLanguage

#define DFLT_FILE_RECORD_DURATION	(15)
#define DFLT_AUTO_LOGOUT_TIME		(10)
#define DFLT_DEVICE_ID				(1)
#define DFLT_HTTP_PORT				(80)
#define DFLT_HTTPS_PORT				(443)
#define DFLT_TCP_PORT				(8000)

#define DFLT_RTP_PORT_START			(50000)
#define DFLT_RTP_PORT_END			(50999)

#define DFLT_ONVIF_PORT				(80)
#define DFLT_VIDEO_SYSTEM_TYPE		VIDEO_SYS_PAL
#define DFLT_NVR_ANALOG_CAM_NO		(0)

#define MAX_ENCODER_NAME_LEN		(12)
#define MAX_PTZ_SPEED_STEPS         (10)
#define MATRIX_CAMERA_SPEED_MAX     (100)

#define DFLT_AUTO_CONFIG_START_IP   "192.168.1.126"
#define DFLT_AUTO_CONFIG_END_IP     "192.168.1.255"

#define	DFLT_DI_SERVER_IP           ""

#define DFLT_DI_SERVER_PORT         (8151)

#define DFLT_AUTO_CONFIG_ENCODE_MAIN	"H264"
#define DFLT_AUTO_CONFIG_ENCODE_SUB		"Motion JPEG"
#define DFLT_AUTO_CONFIG_RES_MAIN		"1280x720"
#define DFLT_AUTO_CONFIG_RES_SUB		"704x576"

#define DFLT_AUTO_CONFIG_QUALITY_MAIN   (4)
#define DFLT_AUTO_CONFIG_QUALITY_SUB    (4)
#define DFLT_PRE_VIDEO_LOSS_DURATION    (10)

#define MAX_USERNAME_WIDTH_V1           (25)
#define MAX_PASSWORD_WIDTH_V1           (17)
#define MAX_USERNAME_WIDTH              (101)   // new size as per UTF-8 MultiLanguage
#define MAX_PASSWORD_WIDTH              (65)    // new size as per UTF-8 MultiLanguage

#define	DFLT_USER_NAME                  "admin"
#define	DFLT_USER_PSWD                  "admin"
#define DFLT_FORWARDED_TCP_PORT         (8001)
#define DFLT_AUTOADD_CAM_TCP_PORT       (8151)
#define DFLT_POLL_DURATION              (5)
#define DFLT_POLL_INTERVAL              (0)

typedef enum
{
	VBR = 0,
	CBR = 1,
	MAX_BITRATE_MODE

}BITRATE_MODE_e;

typedef enum
{
	BITRATE_32,		// Bitrate is in Kbps
	BITRATE_64,
	BITRATE_128,
	BITRATE_256,
	BITRATE_384,
	BITRATE_512,
	BITRATE_768,
	BITRATE_1024,
	BITRATE_1536,
	BITRATE_2048,
	BITRATE_3072,
	BITRATE_4096,
	BITRATE_6144,
	BITRATE_8192,
	BITRATE_12288,
	BITRATE_16384,
	MAX_BITRATE_VALUE

}BITRATE_VALUE_e;

typedef struct
{
    UINT16 start;   // Starting Number of RTP port
    UINT16 end;     // Ending Number of RTP port

}RTP_PORT_RANGE_t;

typedef enum
{
	VIDEO_SYS_NTSC,
	VIDEO_SYS_PAL,
	MAX_VIDEO_SYS

}CAM_VIDEO_SYSTEM_e;

typedef enum
{
	DATE_FORMAT_DDMMYYY 	= 0,
	DATE_FORMAT_MMDDYYY 	= 1,
	DATE_FORMAT_YYYYMMDD 	= 2,
	DATE_FORMAT_WWWDDMMYYYY = 3,
	MAX_DATE_FORMAT

}DATE_FORMAT_e;

typedef enum
{
	TIME_FROMAT_12_HOURS,
	TIME_FORMAT_24_HOURS,
	MAX_TIME_FORMAT

}TIME_FORMAT_e;

typedef enum
{
	REC_NATIVE_FORMAT,
	REC_BOTH_FORMAT,
	REC_AVI_FORMAT,
	MAX_REC_FORMAT

}RECORD_FORMAT_TYPE_e;

/** ******************************************************************************************* **/
/**                                DATE-TIME SETTINGS                                           **/
/** ******************************************************************************************* **/
#define MIN_TIMEZONE				(1)
#define MAX_TIMEZONE				(75)
#define DFLT_TIMEZONE				(49)
#define MAX_REG_INFO 				(2)     // Use for getting info based on TimeZone
#define MAX_AUTO_TIMEZONE_INFO		(2)     // Used for Auto Timezone on internet connectivity
#define MAX_AUTO_TIMEZONE_NAMES		(142)   // Total time zone names used to identify timezone  index in Auto time zone feature
#define MAX_USER_NTP_NAME_WIDTH_V1	(41)
#define MAX_USER_NTP_NAME_WIDTH		(161)   // UTF-8 MultiLanguage

typedef enum
{
	USER_DEFINED_NTP = 0,										// user defined ntp server
	WISCONSIN_TIME_SERVER,										// wisconsin time server
	WINDOWS_TIME_SERVER,										// windows time server
	NIST_TIME_SERVER,											// nist time server
	MAX_TIME_SERVER												// max number of ntp servers

}NTP_SERVER_e;

typedef enum
{
	NTP_UPDATE_6HOUR = 0,										// update NTP every 6 hour
	NTP_UPDATE_12HOUR,											// update NTP every 12 hour
	NTP_UPDATE_24HOUR,											// update NTP every 24 hour
	MAX_NTP_UPDATE												// max number of update time intervals

}NTP_UPDATE_INTERVAL_e;

typedef enum
{
	DATE_TIME_UPDATE_MANUAL = 0,								// Time Update mode manual
	DATE_TIME_UPDATE_AUTO,										// Time Update mode auto
	MAX_DATE_TIME_UPDATE_MODE									// max types of date time update mode

}DATE_TIME_UPDATE_MODE_e;

typedef struct
{
	NTP_SERVER_e server;										// NTP server, predefined or user-defined
	CHAR userDefinedServer[MAX_USER_NTP_NAME_WIDTH_V1];			// user defined server name
	NTP_UPDATE_INTERVAL_e updateInterval;						// NTP update interval, in hours

}NTP_PARAMETER_V1_t; // UTF - 8 MultiLanguage

typedef struct
{
	NTP_SERVER_e server;										// NTP server, predefined or user-defined
	CHAR userDefinedServer[MAX_USER_NTP_NAME_WIDTH];			// user defined server name
	NTP_UPDATE_INTERVAL_e updateInterval;						// NTP update interval, in hours

}NTP_PARAMETER_t;

/** ******************************************************************************************* **/
/**                                    DST SETTINGS                                             **/
/** ******************************************************************************************* **/
typedef struct
{
	DAY_WD_WK_MM_t day;											// provides day instance for DST
	TIME_HH_MM_t instance;										// provides time instance for DST

}DST_CLOCK_t;

/** ******************************************************************************************* **/
/**                                     LAN SETTINGS                                            **/
/** ******************************************************************************************* **/
#define MAX_MAC_ADDRESS_WIDTH			(18)
#define DFLT_IPV6_PREFIX_LEN            (64)
#define MAX_PPPOE_USERNAME_WIDTH_V1		(17)
#define MAX_PPPOE_PASSWORD_WIDTH_V1		(17)
#define MAX_PPPOE_USERNAME_WIDTH_V2		(65)    // new size as per UTF-8 MultiLanguage
#define MAX_PPPOE_PASSWORD_WIDTH_V2		(65)    // new size as per UTF-8 MultiLanguage

#define MAX_PPPOE_USERNAME_WIDTH    	(241)   // Extend array limit
#define MAX_PPPOE_PASSWORD_WIDTH        (161)   // Extend array limit

typedef enum
{
	LAN1_PORT = 0,
	LAN2_PORT,
	MAX_LAN_PORT

}LAN_CONFIG_ID_e;

typedef enum
{
    IP_ADDR_MODE_IPV4,                                          // ipv4 mode
    IP_ADDR_MODE_DUAL_STACK,                                    // both ipv4 and ipv6 mode
    MAX_IP_ADDR_MODE                                            // max number of ip addressing modes

}IP_ADDR_MODE_e;

typedef enum
{
    IPV4_ASSIGN_STATIC = 0,										// Static IP assignment
    IPV4_ASSIGN_DHCP,											// assignment through DHCPv4
    IPV4_ASSIGN_PPPOE,											// assignment through PPPoE
    MAX_IPV4_ASSIGN_MODE										// max number of ip assignment modes

}IPV4_ASSIGN_MODE_e;

typedef enum
{
    IPV6_ASSIGN_STATIC = 0,										// Static IP assignment
    IPV6_ASSIGN_DHCP,											// assignment through DHCPv6
    IPV6_ASSIGN_SLAAC,											// assignment through SLAAC
    MAX_IPV6_ASSIGN_MODE                                        // max number of ip assignment modes

}IPV6_ASSIGN_MODE_e;

typedef enum
{
    IP_ADDR_TYPE_IPV4,
    IP_ADDR_TYPE_IPV6,
    IP_ADDR_TYPE_MAX
} IP_ADDR_TYPE_e;

typedef struct
{
	CHAR username[MAX_PPPOE_USERNAME_WIDTH_V1];					// Username for PPPoE authentication
	CHAR password[MAX_PPPOE_PASSWORD_WIDTH_V1];					// Password for PPPoE authentication

}PPPOE_PARAMETER_V1_t;

typedef struct
{
    CHAR username[MAX_PPPOE_USERNAME_WIDTH_V2];					// Username for PPPoE authentication
    CHAR password[MAX_PPPOE_PASSWORD_WIDTH_V2];					// Password for PPPoE authentication

}PPPOE_PARAMETER_V2_t; // UTF-8 Multilanguage

typedef struct
{
    CHAR username[MAX_PPPOE_USERNAME_WIDTH];					// Username for PPPoE authentication
    CHAR password[MAX_PPPOE_PASSWORD_WIDTH];					// Password for PPPoE authentication

}PPPOE_PARAMETER_t; // Extend size

typedef enum
{
	DNS_STATIC = 0,												// Static DNS address
	DNS_AUTO,													// Auto DNS address
	MAX_DNS_MODE												// max number of dns address mode

}DNS_MODE_e;

typedef struct
{
    DNS_MODE_e  mode;                                           // DNS addressing mode
    CHAR        primaryAddress[IPV4_ADDR_LEN_MAX];              // primary DNS address
    CHAR        secondaryAddress[IPV4_ADDR_LEN_MAX];            // secondary DNS address

}DNS_PARAMETER_V1_t;

typedef struct
{
    DNS_MODE_e  mode;                                           // DNS addressing mode
    CHAR        primaryAddress[IPV6_ADDR_LEN_MAX];              // primary DNS address
    CHAR        secondaryAddress[IPV6_ADDR_LEN_MAX];            // secondary DNS address

}DNS_PARAMETER_t;

typedef struct
{
    CHAR ipAddress[IPV4_ADDR_LEN_MAX];                          // ip address of lan
    CHAR subnetMask[IPV4_ADDR_LEN_MAX];                         // Subnet mask of lan
    CHAR gateway[IPV4_ADDR_LEN_MAX];                            // gateway address of lan

}IPV4_NETWORK_PARAMETER_t;

typedef struct
{
    CHAR    ipAddress[IPV6_ADDR_LEN_MAX];                       // ip address of lan
    UINT32  ipPreferredTime;                                    // ip address preferred time (Useful in dynamic ip assignment)
    UINT32  ipValidTime;                                        // ip address valid time (Useful in dynamic ip assignment)
    UINT8   prefixLen;                                          // subnet prefix length
    CHAR    gateway[IPV6_ADDR_LEN_MAX];                         // gateway address of lan
    UINT32  gatewayMetric;                                      // gateway address priority metric (Useful in dynamic ip assignment)

}IPV6_NETWORK_PARAMETER_t;

typedef struct
{
    IPV4_ASSIGN_MODE_e          ipAssignMode;                   // ip assignment options
    IPV4_NETWORK_PARAMETER_t    lan;                            // network parameter for lan1, <ip, subnet mask, gateway>
    DNS_PARAMETER_t             dns;
    PPPOE_PARAMETER_t           pppoe;                          // PPPOE parameters, <username, password>

} IPV4_LAN_CONFIG_t;

typedef struct
{
    IPV6_ASSIGN_MODE_e          ipAssignMode;                   // ip assignment options
    IPV6_NETWORK_PARAMETER_t    lan;                            // network parameter for lan1, <ip, subnet mask, gateway>
    DNS_PARAMETER_t             dns;

} IPV6_LAN_CONFIG_t;

/** ******************************************************************************************* **/
/**                                 MAC SERVER SETTINGS                                         **/
/** ******************************************************************************************* **/
#define	MAX_MAC_SERVER_NAME_LEN			(41)
#define	MAX_MAC_SERVICE_LEN				(41)
#define	MAX_MAC_SERVER_PASSWORD_LEN		(16)

typedef enum
{
	CONNECT_THROUGH_IP,
	CONNECT_THROUGH_HOSTNAME,
	MAX_SERVER_CONNECT_MODE,

}SERVER_CONNECT_MODE_e;

/** ******************************************************************************************* **/
/**                                  IP FILTER SETTINGS                                         **/
/** ******************************************************************************************* **/
#define MAX_IP_FILTER					(64)

typedef enum
{
	FILTERED_ALLOW = 0,											// Allow specified IP range
	FILTERED_DENY,												// Block Specified IP range
	MAX_FILTER_MODE

}IP_ADDRESS_FILTER_MODE_e;

typedef struct
{
    CHAR startAddress[IPV4_ADDR_LEN_MAX];                       // Start address of Filter range
    CHAR endAddress[IPV4_ADDR_LEN_MAX];                         // End address of filter range

}IP_ADDRESS_RANGE_V1_t;

typedef struct
{
    CHAR startAddress[IPV6_ADDR_LEN_MAX];                       // Start address of Filter range
    CHAR endAddress[IPV6_ADDR_LEN_MAX];                         // End address of filter range

}IP_ADDRESS_RANGE_t;

/** ******************************************************************************************* **/
/**                                     DDNS SETTINGS                                           **/
/** ******************************************************************************************* **/
#define MAX_DDNS_USERNAME_WIDTH_V1		(41)
#define MAX_DDNS_PASSWORD_WIDTH_V1		(25)
#define MAX_DDNS_HOSTNAME_WIDTH_V1		(41)

// new size as per UTF-8 multilanguage
#define MAX_DDNS_USERNAME_WIDTH			(161)
#define MAX_DDNS_PASSWORD_WIDTH			(101)
#define MAX_DDNS_HOSTNAME_WIDTH			(161)

#define MAX_DDNS_UPDATE_DURATION		(99)
#define MIN_DDNS_UPDATE_DURATION		(5)
#define DFLT_DDNS_UPDATE_DURATION		(5)

typedef enum
{
    DDNS_SERVER_DYN_DNS_ORG = 0,                                // dyndns.org
    MAX_DDNS_SERVER                                             // max number of ddns server option

}DDNS_SERVER_e;

/** ******************************************************************************************* **/
/**                                 SMTP(EMAIL) SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MAX_SMTP_SERVER_NAME_WIDTH_V1	(41)
#define DFLT_SMTP_PORT					(25)
#define MAX_SMTP_USERNAME_WIDTH_V1		(41)
#define MAX_SMTP_PASSWORD_WIDTH_V1		(25)
#define MAX_SMTP_EMAILID_WIDTH_V1		(41)

// new size as per UTF-8
#define MAX_SMTP_SERVER_NAME_WIDTH		(161)
#define MAX_SMTP_USERNAME_WIDTH			(161)
#define MAX_SMTP_PASSWORD_WIDTH			(101)
#define MAX_SMTP_EMAILID_WIDTH			(161)

typedef enum
{
    SMTP_ENCRYPTION_TYPE_NONE = 0,
    SMTP_ENCRYPTION_TYPE_SSL,
    SMTP_ENCRYPTION_TYPE_TLS,
    SMTP_ENCRYPTION_TYPE_MAX

}SMTP_ENCRYPTION_TYPE_e;

/** ******************************************************************************************* **/
/**                                  FTP UPLOAD SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MAX_FTP_SERVER_NAME_WIDTH_V1	(41)
#define DFLT_FTP_PORT					(21)
#define MAX_FTP_USERNAME_WIDTH_V1		(41)
#define MAX_FTP_PASSWORD_WIDTH_V1		(25)
#define MAX_FTP_UPLOAD_PATH_WIDTH_V1	(256)

// new size as per UTF-8
#define MAX_FTP_SERVER_NAME_WIDTH       (161)
#define MAX_FTP_USERNAME_WIDTH          (161)
#define MAX_FTP_PASSWORD_WIDTH          (101)
#define MAX_FTP_UPLOAD_PATH_WIDTH       (1021)

typedef enum
{
	FTP_SERVER1 = 0,
	FTP_SERVER2,
	MAX_FTP_SERVER

}FTP_SERVER_e;

/** ******************************************************************************************* **/
/**                                  TCP NOTIFY SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MAX_TCP_SERVER_NAME_WIDTH_V1	(41)
#define MAX_TCP_SERVER_NAME_WIDTH		(161) // new size as per UTF-8
#define DFLT_TCP_NOTIFY_PORT			(80)


/** ******************************************************************************************* **/
/**                                      HDD SETTINGS                                           **/
/** ******************************************************************************************* **/
typedef enum
{
	SINGLE_DISK_VOLUME = 0,										// treats each HDD as single volume
	RAID_0,														// stores stripped data on alternate volumes
	RAID_1,														// stores copy of data on alternate volume
	RAID_5,														// stores stripped data and parity on alternate volume
	RAID_10,													// stores copy of data and stripped data on alternate volume
	MAX_HDD_MODE												// max number of hdd management options

}HDD_MODE_e;

typedef enum
{
	LOCAL_HARD_DISK = 0,
	REMOTE_NAS1,
	REMOTE_NAS2,
	MAX_RECORDING_MODE,
	ALL_DRIVE,
}RECORD_ON_DISK_e;

/** ******************************************************************************************* **/
/**                              MATRIX DNS SERVER SETTINGS                                     **/
/** ******************************************************************************************* **/
#define DFLT_MATRIX_DNS_SERVER_FORWARDED_PORT	(80)
#define MAX_MATRIX_DNS_SERVER_HOST_NAME_LEN_V1  (31)
#define MAX_MATRIX_DNS_SERVER_HOST_NAME_LEN 	(121) // new size as per UTF-8

/** ******************************************************************************************* **/
/**                                 USER ACCOUNT SETTINGS                                       **/
/** ******************************************************************************************* **/
#define MAX_USER_ACCOUNT_USERNAME_WIDTH_VER1    (25)
#define MAX_USER_ACCOUNT_PASSWORD_WIDTH_VER1    (17)

// new size as per UTF-8 multilanguage
#define MAX_USER_ACCOUNT_USERNAME_WIDTH         (101)
#define MAX_USER_ACCOUNT_PASSWORD_WIDTH         (65)

#define	MAX_USER_ACCOUNT_VER9                   (120)
#define	MAX_USER_ACCOUNT                        (128)
#define DFLT_LOGIN_LIMIT_DURATION               (5)

#define LOCAL_USER_NAME                         "local"
#define LOCAL_USER_PSWD                         "local"

#define	ADMIN_USER_NAME                         "admin"
#define	ADMIN_USER_PSWD                         "admin"

#define OPERATOR_USER_NAME                      "operator"
#define OPERATOR_USER_PSWD                      "operator"

#define VIEWER_USER_NAME                        "viewer"
#define VIEWER_USER_PSWD                        "viewer"

typedef enum
{
	USER_LOCAL = 0,						// refers to the default local user
	USER_ADMIN,							// default admin user
	USER_OPERATOR,						// default operator user
	USER_VIEWER,						// default viewer user
	MAX_DFLT_USER

}DFLT_USER_e;

typedef enum
{
	ADMIN = 0,							// administrator category
	OPERATOR,							// operator category
	VIEWER,								// viewer category
	MAX_USER_GROUP						// max number of user group type

}USER_GROUP_e;

typedef struct
{
	UINT8 audio:1;
	UINT8 ptzControl:1;
	UINT8 playback:1;
	UINT8 monitor:1;
	UINT8 videoPopup:1;
	UINT8 audioOut:1;
	UINT8 reserved:2;

}USER_PRIVILEGE_t;

typedef union
{
	USER_PRIVILEGE_t	privilegeBitField;
	UINT8 				privilegeGroup;

}USER_PRIVILEGE_u;

/** ******************************************************************************************* **/
/**                                    CAMERA SETTINGS                                          **/
/** ******************************************************************************************* **/
#define MAX_CAMERA_NAME_WIDTH_V1				(17)
#define MAX_CHANNEL_NAME_WIDTH_V1				(11)

// new size as per UTF-8 multilanguage
#define MAX_CAMERA_NAME_WIDTH					(65)
#define MAX_CHANNEL_NAME_WIDTH					(41)

#define DFLT_MOTION_RE_DETECTION_DELAY			(5)
#define MAX_MOBILE_NUM_WIDTH					(15)

#define DFLT_PRIVACY_MASK_X_CORDINATE           (0)
#define DFLT_PRIVACY_MASK_Y_CORDINATE           (0)
#define DFLT_PRIVACY_MASK_WIDTH                 (0)
#define DFLT_PRIVACY_MASK_HEIGHT                (0)
#define	MAX_PRIVACY_MASKS_V5                    (4)
#define	MAX_PRIVACY_MASKS                       (8)
#define	MOTION_POINT_AREA_MAX               	(4)
#define MOTION_BLOCK_AREA_MAX_V7                (200)
#define MOTION_AREA_BLOCK_BYTES                 (198)
#define	TEXT_OVERLAY_MIN                        (1)
#define	TEXT_OVERLAY_MAX                        (6)

typedef enum
{
	ANALOG_CAMERA,
	IP_CAMERA,
	AUTO_ADDED_CAMERA,
	MAX_CAMERA_TYPE

}CAMERA_TYPE_e;

typedef enum
{
	OSD_POS_NONE,
	OSD_POS_TOP_LEFT,
	OSD_POS_TOP_RIGHT,
	OSD_POS_BOTTOM_LEFT,
	OSD_POS_BOTTOM_RIGHT,
	MAX_OSD_POS

}OSD_POS_e;

typedef enum
{
	DISK_ACT_NORMAL,
	DISK_ACT_CONFIG_CHANGE,
	DISK_ACT_FORMAT,
	DISK_ACT_IO_ERROR,
	DISK_ACT_RECOVERY,
    DISK_ACT_RAID_UPDATE,
	DISK_ACT_MAX
}DISK_ACT_e;

typedef struct
{
	UINT32			startXPoint;
	UINT32			startYPoint;
	UINT32			width;
	UINT32			height;

}PRIVACY_MASK_CONFIG_t;

typedef struct
{

	UINT32			startRaw;
	UINT32			startColumn;
	UINT32			endRaw;
	UINT32			endColumn;
	UINT8			sensitivity;

}MOTION_POINT_AREA_t;

typedef struct
{
    CHAR blockBitString[MOTION_AREA_BLOCK_BYTES];
    UINT8 sensitivity;
    BOOL noMotionSupportF;
    BOOL isNoMotionEvent;
    UINT16 noMotionDuration;

} MOTION_BLOCK_METHOD_PARAM_t;

/** ******************************************************************************************* **/
/**                                    STREAM SETTINGS                                          **/
/** ******************************************************************************************* **/
#define MIN_FRAME_RATE					(0)
#define MAX_FRAME_RATE					(30)
#define DFLT_NTSC_FRAME_RATE			(30)
#define DFLT_PAL_FRAME_RATE				(25)
#define DFLT_IMAGE_QUALITY				(5)
#define DFLT_CAM_GOP					(50)
#define MIN_CAM_GOP                     (1)
#define MAX_CAM_GOP                     (100)

#define DFLT_MAIN_STREAM_PROFILE		(1)
#define DFLT_SUB_STREAM_PROFILE			(2)

#define MAX_ENCODER_NAME_LEN			(12)
#define MAX_RESOLUTION_NAME_LEN			(10)

/** ******************************************************************************************* **/
/**                             SCHEDULE RECORD SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MAX_DAILY_SCHEDULE              (6)

typedef struct
{
	TIME_HH_MM_t startTime;										// start instance of schedule recording
	TIME_HH_MM_t endTime;										// end instance of schedule recording

}SCHEDULE_PERIOD_t;

typedef enum
{
    RECORD_SCHEDULE_ENTIRE_DAY = 0,
    RECORD_ADAPTIVE_ENTIRE_DAY,
    RECORD_ADAPTIVE_PERIOD_1,
    RECORD_ADAPTIVE_PERIOD_2,
    RECORD_ADAPTIVE_PERIOD_3,
    RECORD_ADAPTIVE_PERIOD_4,
    RECORD_ADAPTIVE_PERIOD_5,
    RECORD_ADAPTIVE_PERIOD_6

}RECORD_BIT_FIELDS_e;

typedef struct
{
    /**
     *  Note: In case of schedule recording configuration, this 'recordEntireDay' will be used as 8 bit fields.
     *        first and second bit will be used to store flags for scheduleEntireDay and adaptiveEntireDay respectively.
     *        remaining 6 bit will be used to store period wise flags of adaptive recording.
     */
    BOOL 				recordEntireDay;        				// control to turn ON/OFF entire day recording
	SCHEDULE_PERIOD_t 	period[MAX_DAILY_SCHEDULE];				// 6 different schedule recording period

}DAY_RECORD_SCHEDULE_t;

/** ******************************************************************************************* **/
/**                               ALARM RECORD SETTINGS                                         **/
/** ******************************************************************************************* **/
#define DFLT_PRE_RECORD_TIME                (5)
#define DFLT_POST_RECORD_TIME               (30)

/** ******************************************************************************************* **/
/**                          PTZ PRESET POSITIONS SETTINGS                                      **/
/** ******************************************************************************************* **/
#define MAX_PRESET_POSITION					(40)
#define MAX_PRESET_POSITION_NAME_WIDTH_VER1	(17) // old size without multilanguage
#define MAX_PRESET_POSITION_NAME_WIDTH		(65) // UTF-8 MultiLanguage
#define MAX_PRESET_TOKEN_LEN                (21)

typedef enum
{
    MATRIX_PTZ_ACTION_TOP = 1,
    MATRIX_PTZ_ACTION_TOP_RIGHT,
    MATRIX_PTZ_ACTION_RIGHT,
    MATRIX_PTZ_ACTION_BOTTOM_RIGHT,
    MATRIX_PTZ_ACTION_BOTTOM,
    MATRIX_PTZ_ACTION_BOTTOM_LEFT,
    MATRIX_PTZ_ACTION_LEFT,
    MATRIX_PTZ_ACTION_TOP_LEFT,

}MATRIX_PTZ_ACTION_e;

typedef enum
{
    PTZ_PAN_LEFT = 0,
    PTZ_PAN_RIGHT,
    MAX_PTZ_PAN_OPTION,

    PTZ_TILT_UP = 0,
    PTZ_TILT_DOWN,
    MAX_PTZ_TILT_OPTION,

    PTZ_ZOOM_OUT = 0,
    PTZ_ZOOM_IN,
    MAX_PTZ_ZOOM_OPTION

}PTZ_OPTION_e;

typedef enum
{
    IRIS_CLOSE = 0,	// dark, -, close
    IRIS_OPEN,		// bright, +, open
    IRIS_AUTO,		// auto
    MAX_IRIS_OPTION

}CAMERA_IRIS_e;

typedef enum
{
    FOCUS_FAR = 0,
    FOCUS_NEAR,
    FOCUS_AUTO,
    MAX_FOCUS_OPTION

}CAMERA_FOCUS_e;

typedef struct
{
    BOOL    action;
    UINT8   presetIndex;
    CHAR    presetName[MAX_PRESET_POSITION_NAME_WIDTH];

}PTZ_PRESET_INFO_t;

/** ******************************************************************************************* **/
/**                               PRESET TOUR SETTINGS                                          **/
/** ******************************************************************************************* **/
#define MAX_TOUR_NUMBER                     (5)

#define MAX_TOUR_NAME_WIDTH_VER1            (17)
#define MAX_TOUR_NAME_WIDTH                 (65) // new size as per UTF-8 multilanguage

#define DFLT_MANUAL_TOUR                    (0)

#define MIN_PAUSE_BETWEEN_TOUR              (0)
#define MAX_PAUSE_BETWEEN_TOUR              (60)
#define DFLT_PAUSE_BETWEEN_TOUR             (0)

#define DFLT_PRESET_POSTION                 (0)

#define MIN_PRESET_VIEW_TIME                (1)
#define MAX_PRESET_VIEW_TIME                (255)
#define DFLT_PRESET_VIEW_TIME               (10)

#define MAX_ORDER_COUNT                     (40)

#define DFLT_VIDEO_POPUP_DURATION           (10)

typedef enum
{
	LOOPING = 0,												// preset tour in looping mode
	ZIGZAG,														// preset tour in zig-zag mode
	RANDOM,														// preset mode in random mode
	MAX_TOUR_ORDER

}TOUR_PATTERN_e;

typedef struct
{
	UINT8 presetPosition;										// index of preset position
	UINT8 viewTime;												// viewing time in seconds

}ORDER_NUMBER_t;

typedef struct
{
	CHAR name[MAX_TOUR_NAME_WIDTH];								// user defined name of the tour
	TOUR_PATTERN_e tourPattern;									// order of sequence for tour
	UINT8 pauseBetweenTour;										// time to pause between two consecutive tours
	ORDER_NUMBER_t ptz[MAX_ORDER_COUNT];						// sequential order of the preset positions for particular tour

}TOUR_t; // new size as per UTF-8 multilanguage

/** ******************************************************************************************* **/
/**                           PRESET TOUR SCHEDULE SETTINGS                                     **/
/** ******************************************************************************************* **/
#define MAX_TOUR_SCHEDULE           (2)
#define DFLT_SCHEDULE_TOUR          (0)

typedef struct
{
	SCHEDULE_PERIOD_t period;									// duration for touring, two duration per day
	UINT8 presetTour;											// preset tour number to be activated

}TOUR_SCHEDULE_t;

typedef struct
{
	BOOL entireDay;												// control bit to turn ON/OFF tour entire day option
	UINT8 presetTour;											// preset tour number to be activated
	TOUR_SCHEDULE_t schedule[MAX_TOUR_SCHEDULE];

}DAILY_TOUR_SCHEDULE_t;

/** ******************************************************************************************* **/
/**                            SYSTEM SENSOR INPUT SETTINGS                                     **/
/** ******************************************************************************************* **/
#define MAX_SENSOR_NAME_WIDTH_OLD	(17)
#define MAX_SENSOR_NAME_WIDTH		(65) 						// new size as per UTF-8

#define MIN_DEBOUNCE_TIME			(0)
#define MAX_DEBOUNCE_TIME			(10)
#define DFLT_DEBOUNCE_TIME			(0)

typedef enum
{
	SENSOR_1 = 0,
	SENSOR_2,
	SENSOR_3,
	SENSOR_4,
	SENSOR_5,
	SENSOR_6,
	SENSOR_7,
	SENSOR_8,
	MAX_SENSOR
}SENSOR_NUMBER_e;

typedef enum
{
	NORMALLY_OPEN = 0,											// sensor to be operated in normally open condition
	NORMALLY_CLOSE,												// sensor to be operated in normally close condition
	MAX_SENSOR_MODE

}SENSOR_MODE_e;

/** ******************************************************************************************* **/
/**                            SYSTEM ALARM OUTPUT SETTINGS                                     **/
/** ******************************************************************************************* **/
#define MAX_ALARM_NAME_WIDTH_OLD	(17)
#define MAX_ALARM_NAME_WIDTH		(65)						// new size as per UTF-8
#define MAX_ALARM					(4)
#define DFLT_ALARM_TIME_PERIOD		(10)

typedef enum
{
	ALARM_INTERLOCK = 0,										// activate alarm until event expires
	ALARM_PULSE,												// activate alarm as per the time period
	MAX_ALARM_MODE

}ALARM_MODE_e;

/** ******************************************************************************************* **/
/**                               UPLOAD IMAGE SETTINGS                                         **/
/** ******************************************************************************************* **/
#define MAX_EMAIL_ADDRESS_WIDTH_VER1	(101)
#define MAX_EMAIL_SUBJECT_WIDTH_VER1	(51)
#define MAX_EMAIL_MESSAGE_WIDTH_VER1	(301)

// new size as per UTF-8 multilanguage
#define MAX_EMAIL_ADDRESS_WIDTH			(401)
#define MAX_EMAIL_SUBJECT_WIDTH			(201)
#define MAX_EMAIL_MESSAGE_WIDTH			(1201)

#define MAX_IMAGE_UPLOAD_RESOLUTION		(99)

#define MIN_IMAGE_UPLOAD_PER_MINUTE		(1)
#define DFLT_IMAGE_UPLOAD_PER_MINUTE	(1)
#define MAX_IMAGE_UPLOAD_PER_MINUTE		(5)

typedef enum
{
    UPLOAD_TO_EMAIL_SERVER = 0, //0 upload through email server
    UPLOAD_TO_FTP_SERVER1,      //1 upload through FTP server1
    UPLOAD_TO_FTP_SERVER2,      //2 upload through FTP server2
    UPLOAD_TO_NW_DRIVE1,        //3 upload through N/W Drive1
    UPLOAD_TO_NW_DRIVE2,        //4 upload through N/W Drive2
	MAX_IMAGE_UPLOAD_SERVER

}UPLOAD_LOCATION_e;

typedef struct
{
	CHAR emailAddress[MAX_EMAIL_ADDRESS_WIDTH];	// mail address to send the images to
	CHAR subject[MAX_EMAIL_SUBJECT_WIDTH];		// subject of the mail
	CHAR message[MAX_EMAIL_MESSAGE_WIDTH];		// message of the mail

}EMAIL_PARAMETER_t; // new size as per UTF-8 multilanguage

/** ******************************************************************************************* **/
/**                                   STORAGE SETTINGS                                          **/
/** ******************************************************************************************* **/
#define MIN_HDD_FULL_CLEANUP		(5)
#define MAX_HDD_FULL_CLEANUP		(100)
#define DFLT_HDD_FULL_CLEANUP		(10)

#define MAX_HDD_CLEANUP_BY_DAYS		(60)
#define MIN_HDD_CLEANUP_BY_DAYS		(1)
#define DFLT_HDD_CLEANUP_BY_DAYS	(15)
#define DFLT_CAMERA_WISE_CLEANUP_DAYS	(10)

#define MIN_HDD_ALERT_CAPACITY		(1)
#define MAX_HDD_ALERT_CAPACITY		(99)
#define DFLT_HDD_ALERT_CAPACITY		(5)

typedef enum
{
	ALERT_AND_STOP = 0, 	// stop recording on HDD full
	OVERWRITE,				// over write on HDD full
	CLEANUP,				// clean up on HDD full
	MAX_HDD_ACTION_MODE

}HDD_FULL_ACTION_MODE_e;

typedef struct
{
	HDD_FULL_ACTION_MODE_e 		actionMode;				// action mode for HDD full event
	UINT8 						percentCleanup;			// number of percentage for cleanup

}HDD_FULL_t;

typedef struct
{
	BOOL 						alert;					// Flag to indicate status of storage alert
	UINT8 						remainingCapacity;		// storage in GB to start alert

}STORAGE_ALERT_t;

typedef enum
{
	RECORDING_DRIVE_WISE_RECORD_RETENTION,
	CAMERA_WISE_RECORD_RETENTION,
	MAX_RECORD_RETENTION_TYPE

}RECORD_RETENTION_TYPE_e;

/** ******************************************************************************************* **/
/**                            SCHEDULE BACKUP SETTINGS                                         **/
/** ******************************************************************************************* **/
typedef enum
{
	SB_EVERY_HOUR,					// take backup every hour
	SB_EVERYDAY,					// take backup everyday
	SB_WEEKLY,						// take backup weekly
	MAX_SB_MODE						// max number of schedule backup mode

}SB_MODE_e;

typedef enum
{
	SB_EVERY_1_HOUR,				// take backup every 1 hour
	SB_EVERY_2_HOUR,				// take backup every 2 hour
	SB_EVERY_3_HOUR,				// take backup every 3 hour
	SB_EVERY_4_HOUR,				// take backup every 4 hour
	SB_EVERY_6_HOUR,				// take backup every 6 hour
	MAX_SB_EVERY_HOUR_MODE,

}SB_EVERY_HOUR_MODE_e;				// Schedule Back-Up Every hour mode

typedef enum
{
	SB_TO_USB_DEVICE,				// backup to the usb device
	SB_TO_FTP_SERVER_1,				// backup to the FTP Server 1
	SB_TO_FTP_SERVER_2,				// backup to the FTP Server 2
	SB_TO_NETWORK_DRIVE_1,			// backup to Network Drive 1
	SB_TO_NETWORK_DRIVE_2,			// backup to Network Drive 2
	MAX_SB_LOCATION					// max number of backup locations

}SB_LOCATION_e;

typedef struct
{
	UINT8 hour;						// backup time instance for daily schedule

}SB_EVERYDAY_CONFIG_t;

typedef struct
{
	WEEK_DAY_e	weekday;			// backup day instance for weekly schedule
	UINT8 		hour;				// backup time instance for weekly schedule

}SB_WEEKLY_CONFIG_t;

/** ******************************************************************************************* **/
/**                               MANUAL BACKUP SETTINGS                                        **/
/** ******************************************************************************************* **/
typedef enum
{
    MB_LAST_1_HOUR = 0,             // take backup of last 1 hour

    // NOTE: We Have Options of Last 1 to 24 hours, maintained through giving direct value 23 to M_BACKUP_LAST_24HOUR
    MB_LAST_24_HOUR = 23,           // take backup of last 24 hour
    MB_PREVIOUS_DAY,                // take backup of last day
    MB_LAST_WEEK,                   // take backup of last week
    MB_ALL,                         // take all backup
    MB_CUSTOM,
    MAX_MB_DURATION                 // max manual backup mode

}MB_DURATION_e;

typedef enum
{
	MB_TO_USB_DEVICE,				// backup to the usb device
	MB_TO_NETWORK_DRIVE_1,			// backup to Network Drive 1
	MB_TO_NETWORK_DRIVE_2,			// backup to Network Drive 2
	MAX_MB_LOCATION					// max number of backup locations

}MB_LOCATION_e;

/** ******************************************************************************************* **/
/**                               CAMERA ALARM SETTINGS                                         **/
/** ******************************************************************************************* **/
#define CAMERA_ALARM_CONFIG_VERSION		(1)
#define MAX_CAMERA_ALARM				(3)

/** ******************************************************************************************* **/
/**                            EVENT AND ACTION SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MAX_EVENT_SCHEDULE				(6)
#define MAX_SENSOR_EVENT				(MAX_SENSOR)
#define MAX_MOBILE_NUMBER_WIDTH			(15)
#define MAX_SMS_WIDTH_VER1				(161)
#define MAX_SMS_WIDTH_VER2              (641)   // UTF-8 MultiLanguage
#define MAX_SMS_WIDTH					(1021)  // smslane v2 support added
#define MAX_TCP_MESSAGE_WIDTH_V1		(301)
#define MAX_TCP_MESSAGE_WIDTH			(1201)  // UTF-8 MultiLanguage

/* FCM Push Notification */
#define FCM_PUSH_NOTIFY_DEVICES_MAX     (10)
#define FCM_TOKEN_LENGTH_MAX			(200)
#define FCM_DEVICE_MODEL_NAME_LEN_MAX	(21)

typedef struct
{
	UINT16 startBeep:1;											// activate beep on an event
	UINT16 cameraAlarmOutput:1;									// activate camera alarm output
	UINT16 systemAlarmOutput:1;									// activate system alarm output
	UINT16 gotoPresetPtz:1;										// goto to preset position on an event
	UINT16 smsNotification:1;									// send sms on an event
	UINT16 tcpNotification:1;									// send tcp notification on an event
	UINT16 emailNotification:1;									// send email on an event
	UINT16 uploadImage:1;										// upload image on an event
	UINT16 alarmRecording:1;									// start alarm recording on an event
	UINT16 videoPopUp:1;										// Video pop up on a event
    UINT16 pushNotification:1;                                  // Push Notification
    UINT16 reserved:5;											// reserved (5 left from 16) bits

}ACTION_BIT_FIELD_t;

typedef union
{
	ACTION_BIT_FIELD_t actionBitField;
	UINT16 actionBitGroup;

}ACTION_BIT_u;

typedef enum
{
    SYS_EVT_MANUAL_TRIGGER = 0,
    SYS_EVT_ON_BOOT,
    SYS_EVT_STORAGE_ALERT,
    SYS_EVT_SYSTEM_ON_UPS,
    SYS_EVT_DISK_VOLUME_FULL,
    SYS_EVT_DISK_FAULT,
    SYS_EVT_SCHEDULE_BACKUP_FAIL,
    SYS_EVT_FIRMWARE_UPGRADE,
	MAX_SYSTEM_EVENT

}SYSTEM_EVENT_e;

/* Validate number of events in event and action. MAX_URL_REQUEST = MAX_CAMERA_EVENT directly mapped in module */
typedef enum
{
	MOTION_DETECT 		= 0,
	VIEW_TEMPERING 		= 1,
	CAMERA_SENSOR_1 	= 2,
	CAMERA_SENSOR_2 	= 3,
	CAMERA_SENSOR_3 	= 4,
	CONNECTION_FAILURE 	= 5,
	RECORDING_FAIL 		= 6,
	LINE_CROSS 			= 7,
	OBJECT_INTRUSION 	= 8,
	AUDIO_EXCEPTION 	= 9,
	MISSING_OBJECT 		= 10,
	SUSPICIOUS_OBJECT 	= 11,
	LOITERING 			= 12,
	CAMERA_ONLINE 		= 13,
	RECORDING_START     = 14,
	OBJECT_COUNTING		= 15,
    NO_MOTION_DETECTION	= 16,
	MAX_CAMERA_EVENT

}CAMERA_EVENT_e;

typedef enum
{
	CAMERA_EVENT = 0,
	SENSOR_EVENT,
	SYSTEM_EVENT,

}EVENT_TYPE_e;

typedef struct
{
	UINT8 cameraNumber;											// camera index
	UINT8 presetPosition;										// preset position number

}GOTO_PRESET_POSITION_t;

typedef struct
{
	CHAR mobileNumber1[MAX_MOBILE_NUMBER_WIDTH];				// mobile number 1
	CHAR mobileNumber2[MAX_MOBILE_NUMBER_WIDTH];				// mobile number 2
	CHAR message[MAX_SMS_WIDTH];								// message to send

}SMS_PARAMTER_t; // new size as per UTF-8 multilanguage

typedef struct
{
	TIME_HH_MM_t startTime;										// time before which event should not be responded
	TIME_HH_MM_t endTime;										// time after which event should not be responded
	ACTION_BIT_u scheduleAction;								// booleans for action against each schedule

}ACTION_CONTROL_PARAMETER_t;

typedef struct
{
	UINT8 cameraNumber;											// camera number, whose alarm outputs are configured
	BOOL alarm[MAX_CAMERA_ALARM];								// Flag to indicate status of camera output port

}CAMERA_ALARM_PARAMETER_t;

typedef struct
{
    BOOL                        alarmRecord[MAX_CAMERA_CONFIG]; // select cameras to record stream on occurrence of event
    BOOL                        uploadImage[MAX_CAMERA_CONFIG]; // select cameras to upload images from on occurrence of event
    EMAIL_PARAMETER_t           sendEmail;                      // email parameters to send notifications
    CHAR                        sendTcp[MAX_TCP_MESSAGE_WIDTH]; // TCP notification message
    SMS_PARAMTER_t              smsParameter;                   // SMS parameters for notification
    GOTO_PRESET_POSITION_t      gotoPosition;                   // preset position to set against an event
    BOOL                        systemAlarmOutput[MAX_ALARM];   // control bit to enable system alarm output
    CAMERA_ALARM_PARAMETER_t    cameraAlarmOutput;              // camera alarm parameters

}ACTION_PARAMETERS_t; // 96 channel NVR support (camera config changed from 64 to 96)

typedef struct
{
    BOOL                        actionEntireDay;                    // action against an event during entire day
    ACTION_BIT_u                entireDayAction;                    // booleans for actions against entire day
    ACTION_CONTROL_PARAMETER_t  actionControl[MAX_EVENT_SCHEDULE];  // parameters to control the actions against event

}WEEKLY_ACTION_SCHEDULE_t;

/** ******************************************************************************************* **/
/**                            COSEC PRE-RECORD SETTINGS                                        **/
/** ******************************************************************************************* **/
#define DEFAULT_COSEC_PRE_REC_DURATION	(5)
#define MAX_COSEC_PRE_REC_DURATION		(30)

/** ******************************************************************************************* **/
/**                              STATIC ROUTING SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MAX_SUBNET_MASK                 (32)
#define MAX_STATIC_ROUTING_ENTRY        (5)

typedef enum
{
    NETWORK_PORT_LAN1,
    NETWORK_PORT_LAN2,
    NETWORK_PORT_USB_MODEM,
    NETWORK_PORT_MAX
}NETWORK_PORT_e;

typedef struct
{
    CHAR            networkAddr[IPV4_ADDR_LEN_MAX];     // Network Address
    UINT8           subnetMask;							// Index of Subnet Mask
    NETWORK_PORT_e  routePort;							// LAN1 / LAN2 port
}ROUTING_ENTRY_V1_t;

typedef struct
{
    CHAR            networkAddr[IPV6_ADDR_LEN_MAX];     // Network Address
    UINT8           subnetMask;							// Index of Subnet Mask (IPv4) / Prefix Length (IPv6)
    NETWORK_PORT_e 	routePort;							// LAN1 / LAN2 port
}ROUTING_ENTRY_t;

/** ******************************************************************************************* **/
/**                            MOBILE BROADBAND SETTINGS                                        **/
/** ******************************************************************************************* **/
#define BROADBAND_PROFILE_NAME_LEN_V1   (17)
#define BROADBAND_DIAL_NUMBER_LEN_V1	(17)
#define BROADBAND_USERNAME_LEN_V1		(41)
#define BROADBAND_PASSWORD_LEN_V1		(41)
#define BROADBAND_APN_LEN_V1			(41)
#define MAX_BROADBAND_PROFILE			(5)

// New size as per UTF-8 multilanguage
#define BROADBAND_PROFILE_NAME_LEN		(65)
#define BROADBAND_DIAL_NUMBER_LEN		(65)
#define BROADBAND_USERNAME_LEN			(161)
#define BROADBAND_PASSWORD_LEN			(161)
#define BROADBAND_APN_LEN				(161)

typedef enum
{
    GSM_MOBILE_SERVICE,
    CDMA_MOBILE_SERVICE,
    MAX_MOBILE_SERVICE
}MOBILE_SERVICE_TYPE_e;

typedef struct
{
    CHAR    profileName[BROADBAND_PROFILE_NAME_LEN];
    CHAR    dialNumber[BROADBAND_DIAL_NUMBER_LEN];
    CHAR    userName[BROADBAND_USERNAME_LEN];
    CHAR    password[BROADBAND_PASSWORD_LEN];
    CHAR    apn[BROADBAND_APN_LEN];
}BROAD_BAND_PROFILE_t;  // (3->4) Service type removed

/** ******************************************************************************************* **/
/**                                     SMS SETTINGS                                            **/
/** ******************************************************************************************* **/
#define MAX_SMS_ACC_USERNAME_WIDTH_V1		(51)
#define MAX_SMS_ACC_PASSWORD_WIDTH_V1		(31)
#define MAX_SMS_ACC_SENDOR_ID_WIDTH_V1		(26)
#define MAX_SMS_ACC_USERNAME_WIDTH_V2       (201)   // As per UTF-8 multilanguage
#define MAX_SMS_ACC_PASSWORD_WIDTH_V2       (121)   // As per UTF-8 multilanguage

// New size as per smslane V2 API
#define MAX_SMS_ACC_USERNAME_WIDTH			(401)
#define MAX_SMS_ACC_PASSWORD_WIDTH			(401)
#define MAX_SMS_ACC_SENDOR_ID_WIDTH			(101)

typedef enum
{
	SMS_MODE_HTTP = 0,
	SMS_MODE_BROADBAND,
	MAX_SMS_MODE

}SMS_MODE_e;

typedef enum
{
	SMS_SERVER_SMS_GATEWAYE = 0,
	SMS_SERVER_SMSLANE,
	SMS_SERVER_BUSINESS_SMS,
	SMS_SERVER_BULK_SMS,
	MAX_SMS_SERVER,

}SMS_SERVER_e;

/** ******************************************************************************************* **/
/**                              NETWORK DRIVE SETTINGS                                         **/
/** ******************************************************************************************* **/
#define MAX_NW_DRIVE							2

#define MAX_NW_DRIVE_NAME_LEN_V1				(41)
#define MAX_NW_DRIVE_USER_NAME_LEN_V1			(41)
#define MAX_NW_DRIVE_PASSWORD_LEN_VER1			(25)
#define MAX_NW_DRIVE_PASSWORD_LEN_V1			(17)
#define MAX_NW_DRIVE_DFLT_FOLDER_PATH_LEN_V1	(256)
// new size as per UTF-8
#define MAX_NW_DRIVE_NAME_LEN					(161)
#define MAX_NW_DRIVE_USER_NAME_LEN				(161)
#define MAX_NW_DRIVE_PASSWORD_LEN				(101)
#define MAX_NW_DRIVE_DOMAIN_NAME_LEN            (100)
#define MAX_NW_DRIVE_DFLT_FOLDER_PATH_LEN		(1021)

typedef enum
{
	CIFS_FILE_SYS,
	NFS_FILE_SYS,
	MAX_FILE_SYS
}FILE_SYS_TYPE_e;

/** ******************************************************************************************* **/
/**                                 IP CAMERA SETTINGS                                          **/
/** ******************************************************************************************* **/
#define MAX_CAMERA_URL_WIDTH			(150)
#define MAX_CAMERA_USERNAME_WIDTH_VER2	(17)
#define MAX_CAMERA_PASSWORD_WIDTH_VER2	(9)

#define MAX_CAMERA_USERNAME_WIDTH_VER4	(25)
#define MAX_CAMERA_PASSWORD_WIDTH_VER4	(17)
#define MAX_CAMERA_ADDRESS_WIDTH_VER1	(41)
#define MAX_ROUTER_ADDRESS_WIDTH_VER1	(41)
#define MAX_CAMERA_PASSWORD_WIDTH_VER7	(65)    /* Password length changed from 16 to 20 */

#define MAX_BRAND_NAME_LEN				(31)
#define MAX_MODEL_NAME_LEN				(31)
	// new size as per UTF-8
#define MAX_CAMERA_USERNAME_WIDTH		(101)
#define MAX_CAMERA_PASSWORD_WIDTH		(81)
#define MAX_CAMERA_ADDRESS_WIDTH		(161)
#define MAX_ROUTER_ADDRESS_WIDTH		(161)

#define DFLT_RTSP_PORT					(554)
#define DFLT_FORWARDED_HTTP_PORT		(80)
#define DFLT_FORWARDED_RTSP_PORT		(554)
#define DFLT_FORWARDED_ONVIF_PORT		(80)

typedef enum
{
	RTSP_OVER_TCP,
	RTSP_OVER_UDP,
    RTSP_OVER_HTTP,
	MAX_RTSP_PROTOCOL
}RTSP_PROTOCOL_e;

/** ******************************************************************************************* **/
/**                                   NETWORK DEVICE SETTINGS                                   **/
/** ******************************************************************************************* **/
#define MAX_NETWORK_DEVICES			   		(20)
#define MAX_NETWORK_DEVICE_NAME_LEN_V1		(17)
#define MAX_REGISTER_MODE_ADDRESS_LEN_V1	(40)
#define MAX_PORT_WIDTH						(5)
#define MAX_NETWORK_DEVICE_USERNAME_LEN_V1	(25)
#define MAX_NETWORK_DEVICE_PASSWORD_LEN_V1	(17)

// new size as per UTF-8
#define MAX_NETWORK_DEVICE_NAME_LEN			(65)
#define MAX_NETWORK_DEVICE_USERNAME_LEN		(101)
#define MAX_NETWORK_DEVICE_PASSWORD_LEN		(65)
#define MAX_REGISTER_MODE_ADDRESS_LEN		(161)

typedef enum
{
	REGISTER_MODE_IP_ADDRESS = 0,
	REGISTER_MODE_DDNS,
	REGISTER_MODE_MAC_ADDRESS,
	REGISTER_MODE_MATRIX_DDNS,
	MAX_REGISTER_MODES

}REGISTER_MODE_e;

typedef enum
{
	LIVE_STREAM_MAIN,
	LIVE_STREAM_SUB,
	LIVE_STREAM_OPTIMIZED,
	MAX_LIVE_STREAM
}LIVE_STREAM_TYPE_e;

#define DFLT_PORT_ADDRESS					(8000)
#define DFLT_USERNAME						"admin"

/** ******************************************************************************************* **/
/**                                     SNAPSHOT SETTINGS                                       **/
/** ******************************************************************************************* **/
#define DEFAULT_SNAPSHOT_WEEKSELECTED       (1)

/** ******************************************************************************************* **/
/**                                ADAPTIVE RECORDING SETTINGS                                  **/
/** ******************************************************************************************* **/
#define DEFAULT_ADAPTIVE_REC_WEEKSELECTED	(1)

/** ******************************************************************************************* **/
/**                                    LOGIN POLICY SETTINGS                                    **/
/** ******************************************************************************************* **/
#define MAX_PASSWORD_POLICY_PASSWORD_LEN		(16)
#define DEFAULT_PASSWORD_POLICY_PASSWORD_LEN	(5)

// in terms of days
#define MAX_PASSWORD_RESET_DURATION				(365)
#define DEFAULT_PASSWORD_RESET_DURATION			(90)

#define MAX_PASSWORD_INVALID_ATTEMPT			(99)
#define DEFAULT_PASSWORD_INVALID_ATTEMPT		(5)

// in terms of minutes
#define MAX_AUTO_LOCK_TIMER						(999)
#define DEFAULT_AUTO_LOCK_TIMER					(30)

typedef enum
{
	PASSWORD_STRENGTH_LOW = 0,
	PASSWORD_STRENGTH_MEDIUM,
	PASSWORD_STRENGTH_HIGH,
	MAX_PASSWORD_STRENGTH

}PASSWORD_STRENGTH_e;

typedef enum
{
	LOGIN_LOCK_DISABLE = 0,
	LOGIN_LOCK_ENABLE,
	MAX_LOGIN_LOCK_STATUS,

}LOGIN_LOCK_STATUS_e;

/** ******************************************************************************************* **/
/**                                  AUDIO-OUT SETTINGS                                         **/
/** ******************************************************************************************* **/
typedef enum
{
    AUDIO_OUT_PRIORITY_NONE = 0,
    AUDIO_OUT_PRIORITY_CLIENT,
    AUDIO_OUT_PRIORITY_CAMERA,
    AUDIO_OUT_PRIORITY_MAX

}AUDIO_OUT_PRIORITY_e;

/** ******************************************************************************************* **/
/**                                       IMAGE SETTINGS                                        **/
/** ******************************************************************************************* **/
#define IMAGE_SETTING_PARAM_MODE_INVALID    255
#define IMAGE_SETTING_CAPABILITY_MAX        IMG_SETTING_COPY_TO_CAMERA_START

typedef enum
{
    BRIGHTNESS_MIN = 0,
    BRIGHTNESS_MAX = 100,

    CONTRAST_MIN = 0,
    CONTRAST_MAX = 100,

    SATURATION_MIN = 0,
    SATURATION_MAX = 100,

    HUE_MIN = 0,
    HUE_MAX = 100,

    SHARPNESS_MIN = 0,
    SHARPNESS_MAX = 100,

    WDR_STRENGTH_MIN = 1,
    WDR_STRENGTH_MAX = 100,

    EXPOSURE_RATIO_MIN = 2,
    EXPOSURE_RATIO_MAX = 64,

    FLICKER_STRENGTH_MIN = 0,
    FLICKER_STRENGTH_MAX = 10,

    EXPOSURE_TIME_MIN = 10,
    EXPOSURE_TIME_MAX = 66666,

    EXPOSURE_GAIN_MIN = 0,
    EXPOSURE_GAIN_MAX = 100,

    EXPOSURE_IRIS_MIN = 0,
    EXPOSURE_IRIS_MAX = 4,

    NORMAL_LIGHT_LUMINANCE_MIN = 1,
    NORMAL_LIGHT_LUMINANCE_MAX = 7,

    LED_SENSITIVITY_MIN = 0,
    LED_SENSITIVITY_MAX = 15,

}IMAGE_SETTING_PARAM_RANGE_e;

typedef enum
{
    WHITE_BALANCE_MODE_AUTO = 0,
    WHITE_BALANCE_MODE_FLUORESCENT,
    WHITE_BALANCE_MODE_INCANDESCENT,
    WHITE_BALANCE_MODE_SUNNY,
    WHITE_BALANCE_MODE_MAX
}WHITE_BALANCE_MODE_e;

typedef enum
{
    WDR_MODE_OFF = 0,
    WDR_MODE_MANUAL,
    WDR_MODE_AUTO,
    WDR_MODE_MAX
}WDR_MODE_e;

typedef enum
{
    BACKLIGHT_MODE_OFF = 0,
    BACKLIGHT_MODE_BLC,
    BACKLIGHT_MODE_DWDR,
    BACKLIGHT_MODE_MAX
}BACKLIGHT_MODE_e;

typedef enum
{
    EXPOSURE_RATIO_MODE_AUTO = 0,
    EXPOSURE_RATIO_MODE_MANUAL,
    EXPOSURE_RATIO_MODE_MAX
}EXPOSURE_RATIO_MODE_e;

typedef enum
{
    EXPOSURE_MODE_AUTO = 0,
    EXPOSURE_MODE_MANUAL,
    EXPOSURE_MODE_MAX
}EXPOSURE_MODE_e;

typedef enum
{
    FLICKER_MODE_NONE = 0,
    FLICKER_MODE_50HZ,
    FLICKER_MODE_60HZ,
    FLICKER_MODE_AUTO,
    FLICKER_MODE_MAX
}FLICKER_MODE_e;

typedef enum
{
    HLC_MODE_OFF = 0,
    HLC_MODE_HIGH,
    HLC_MODE_MEDIUM,
    HLC_MODE_LOW,
    HLC_MODE_MAX
}HLC_MODE_e;

typedef enum
{
    NORMAL_LIGHT_GAIN_16X = 0,
    NORMAL_LIGHT_GAIN_32X,
    NORMAL_LIGHT_GAIN_48X,
    NORMAL_LIGHT_GAIN_64X,
    NORMAL_LIGHT_GAIN_96X,
    NORMAL_LIGHT_GAIN_128X,
    NORMAL_LIGHT_GAIN_MAX
}NORMAL_LIGHT_GAIN_e;

typedef enum
{
    LED_MODE_AUTO = 0,
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_MAX
}LED_MODE_e;

/* Image settings parameters */
typedef enum
{
    IMG_SETTING_BRIGHTNESS = 0,
    IMG_SETTING_CONTRAST,
    IMG_SETTING_SATURATION,
    IMG_SETTING_HUE,
    IMG_SETTING_SHARPNESS,
    IMG_SETTING_WHITE_BALANCE,
    IMG_SETTING_WDR_MODE,
    IMG_SETTING_WDR_STRENGTH,
    IMG_SETTING_BACKLIGHT,
    IMG_SETTING_EXPOSURE_RATIO_MODE,
    IMG_SETTING_EXPOSURE_RATIO,
    IMG_SETTING_EXPOSURE_MODE,
    IMG_SETTING_FLICKER,
    IMG_SETTING_FLICKER_STRENGTH,
    IMG_SETTING_HLC,
    IMG_SETTING_EXPOSURE_TIME,
    IMG_SETTING_EXPOSURE_GAIN,
    IMG_SETTING_EXPOSURE_IRIS,
    IMG_SETTING_NORMAL_LIGHT_GAIN,
    IMG_SETTING_NORMAL_LIGHT_LUMINANCE,
    IMG_SETTING_LED_MODE,
    IMG_SETTING_LED_SENSITIVITY,
    IMG_SETTING_COPY_TO_CAMERA_START,
    IMG_SETTING_COPY_TO_CAMERA_END = IMG_SETTING_COPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1,
    MAX_IMG_SETTING_CFG_FIELD
}IMG_SETTING_CFG_FIELD_e;

/** ******************************************************************************************* **/
/**                                    FIRMWARE MANAGEMENT                                      **/
/** ******************************************************************************************* **/
typedef enum
{
    AUTO_FIRMWARE_UPGRADE_NEVER = 0,
    AUTO_FIRMWARE_UPGRADE_NOTIFY_ONLY,
    AUTO_FIRMWARE_UPGRADE_DOWNLOAD_AND_UPGRADE,
    AUTO_FIRMWARE_UPGRADE_MODE_MAX,

}AUTO_FIRMWARE_UPGRADE_MODE_e;

typedef enum
{
    FIRMWARE_FTP_SERVER_MATRIX = 0,
    FIRMWARE_FTP_SERVER_CUSTOM,
    FIRMWARE_FTP_SERVER_MAX,

}FIRMWARE_FTP_SERVER_TYPE_e;

/** ******************************************************************************************* **/
/**                                     PASSWORD RECOVERY                                       **/
/** ******************************************************************************************* **/
#define PWD_RECOVERY_EMAIL_ID_LEN_MAX       (51)    /* Maximum password recovery email id length */
#define PWD_RECOVERY_SECURITY_QUESTION_MAX  (3)     /* Maximum password recovery questions */
#define PWD_RECOVERY_ANSWER_LEN_MAX         (26)    /* Maximum password recovery answer length */

typedef enum
{
    PWD_RST_MODE_MASK_NONE = 0x00,
    PWD_RST_MODE_MASK_OTP = 0x01,
    PWD_RST_MODE_MASK_QA = 0x02,
}PWD_RST_MODE_MASK_e;

typedef struct
{
    UINT8       questionId;
    CHAR        answer[PWD_RECOVERY_ANSWER_LEN_MAX];

}USER_SECURITY_QUESTION_INFO_t;

/** ******************************************************************************************* **/
/**                                      STORAGE ALLOCATION                                     **/
/** ******************************************************************************************* **/
#define STORAGE_ALLOCATION_GROUP_MAX        (8)

typedef struct
{
    UINT32              volumeAllocationMask;       /* Storage volume allocation to storage group */
    CAMERA_BIT_MASK_t   cameraAllocationMask;       /* Camera allocation to storage group */

} STORAGE_ALLOCATION_INFO_t;

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* CONFIGURATIONCOMMONDEF_H_ */
