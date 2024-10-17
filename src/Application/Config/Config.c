//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   Config.c
@brief  This module consists of constant configuration variables.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Include */
#include <stdint.h>

/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
// Note : Due to UTF-8 support, Device name, Username & Password size has been modified and appended into existing structure.
GENERAL_CONFIG_t DfltGeneralCfg =
{
    DEVICE_NAME,					// name of the device
	DFLT_FILE_RECORD_DURATION,		// single file record duration in minutes
	DFLT_AUTO_LOGOUT_TIME,			// auto logout time duration in minutes
	ENABLE,							// auto system restart on power up enabled
	DFLT_HTTP_PORT,					// HTTP port
	DFLT_TCP_PORT,					// TCP port
	{
		DFLT_RTP_PORT_START,		// starting RTP port
		DFLT_RTP_PORT_END			// ending RTP port
	},
	DFLT_ONVIF_PORT,				// ONVIF port
	DFLT_DEVICE_ID,					// Device id
	DFLT_VIDEO_SYSTEM_TYPE,
	ENABLE,
	DATE_FORMAT_DDMMYYY,
	TIME_FROMAT_12_HOURS,
	REC_NATIVE_FORMAT,
	DFLT_NVR_ANALOG_CAM_NO,
    MAX_CAMERA,
    TRUE,                           // Auto Configure Flag
	TRUE,
	DFLT_AUTO_CONFIG_START_IP,
	DFLT_AUTO_CONFIG_END_IP,
	TRUE,
    DFLT_AUTO_CONFIG_ENCODE_MAIN,   // video encoding method
    DFLT_AUTO_CONFIG_RES_MAIN,      // image resolution
    DFLT_PAL_FRAME_RATE,            // frame rate
	DFLT_AUTO_CONFIG_QUALITY_MAIN,
    DISABLE,                        // control to enable/disable audio
    CBR,                            // Bitrate Mode
    BITRATE_2048,                   // quality of stream
    DFLT_CAM_GOP,                   // GOP
    DFLT_AUTO_CONFIG_ENCODE_SUB,    // sub video encoding method
    DFLT_AUTO_CONFIG_RES_SUB,       // sub image resolution
    DFLT_PAL_FRAME_RATE,            // sub frame rate
    DFLT_AUTO_CONFIG_QUALITY_SUB,   // sub quality
    DISABLE,                        // sub control to enable/disable audio
    CBR,                            // sub Bitrate Mode
    BITRATE_1024,                   // sub quality of stream
    DFLT_CAM_GOP,                   // sub GOP
	DISABLE,
	DFLT_DI_SERVER_IP,
	DFLT_DI_SERVER_PORT,
	ENABLE,
	DFLT_VIDEO_POPUP_DURATION,
	DFLT_PRE_VIDEO_LOSS_DURATION,
    DFLT_USER_NAME,
    DFLT_USER_PSWD,
	DISABLE,
	DFLT_FORWARDED_TCP_PORT,
	ENABLE,
	DFLT_AUTOADD_CAM_TCP_PORT,
	DFLT_POLL_DURATION,
	DFLT_POLL_INTERVAL,
	DISABLE,
};

// Note : Due to UTF-8 support new NTP_PARAMETER structure has been appended into existing structure.
const DATE_TIME_CONFIG_t DfltDateTimeCfg =
{
	DFLT_TIMEZONE,					// GMT+05:30
	DATE_TIME_UPDATE_AUTO,			// Mode of time update: Auto
	{
		WISCONSIN_TIME_SERVER,		// NTP server: Wisconsin Time Server
		"",							// user defined server: BLANK
		NTP_UPDATE_24HOUR			// NTP update period: 24 hour
	},
    DISABLE,                        // Auto-Update Regional settings
	{
        WISCONSIN_TIME_SERVER,		// NTP server: Wisconsin Time Server
        "",							// user defined server: BLANK
        NTP_UPDATE_24HOUR			// NTP update period: 24 hour
    },
    ENABLE,                         // Auto Sync Time-zone(ONVIF)
    ENABLE,                         // SetUtcTime flag
};

const DST_CONFIG_t DfltDstCfg =
{
	DISABLE,						// DST disabled

	// DST forward settings
	{
		{
			SUNDAY,					// First day of the week
			FIRST_WEEK,				// First Week of the Month
			JANUARY					// First month of the year
		},
		{00, 00},					// Time instance of the day, HH:MM
	},

	// DST reverse settings
	{
		{
			SUNDAY,					// First day of the week
			FIRST_WEEK,				// First Week of the Month
			JANUARY					// First month of the year
		},
		{00, 00},					// Time instance of the day, HH:MM
	},
	{00, 00}						// Time period to offset, HH:MM
};

const LAN_CONFIG_t DfltLanCfg[MAX_LAN_PORT] =
{
    {
        // Ip addressing mode
        IP_ADDR_MODE_IPV4,              // ipv4 addressing mode

        // LAN 1 IPv4 parameters
        {
            IPV4_ASSIGN_STATIC,         // Static IP assign mode

            //Network parameter
            {
                "192.168.1.123",        // IP address
                "255.255.255.0",        // Subnet Mask
                "192.168.1.1",          // Gateway
            },

            // DNS paramters
            {
                DNS_STATIC,             // DNS server Mode
                "192.168.1.1",          // Primary DNS address
                ""                      // secondary DNS address
            },

            // PPPoE parameter new // UTF-8 MultiLanguage
            {
                "",                     // PPPoE username, BLANK
                ""                      // PPPoE password, BLANK
            }
        },

        // LAN 1 IPv6 parameters
        {
            IPV6_ASSIGN_DHCP,           // DHCP IP assign mode

            //Network parameter
            {
                "",                     // IP address
                0,                      // Preferred lifetime
                0,                      // Valid lifetime
                DFLT_IPV6_PREFIX_LEN,   // Prefix length
                "",                     // Gateway address
                0,                      // Gateway metric
            },

            // DNS paramters
            {
                DNS_STATIC,             // DNS server Mode
                "2001:4860:4860::8888", // Primary DNS address
                ""                      // secondary DNS address
            },
        }
    },

    {
        // Ip addressing mode
        IP_ADDR_MODE_IPV4,              // ipv4 addressing mode

        // LAN 2 IPv4 parameters
        {
            IPV4_ASSIGN_STATIC,         // Static IP assign mode

            //Network parameter
            {
                "192.168.2.2",			// IP address
                "255.255.255.0",        // Subnet Mask
                "192.168.2.1",          // Gateway
            },

            // DNS paramters
            {
                DNS_STATIC,             // DNS server Mode
                "192.168.1.1",          // Primary DNS address
                ""                      // secondary DNS address
            },

            // PPPoE parameter new // UTF-8 multilanguage
            {
                "",                     // PPPoE username, BLANK
                ""                      // PPPoE password, BLANK
            }
        },

        // LAN 2 IPv6 parameters
        {
            IPV6_ASSIGN_STATIC,         // Static IP assign mode

            //Network parameter
            {
                "",                     // IP address
                0,                      // Preferred lifetime
                0,                      // Valid lifetime
                DFLT_IPV6_PREFIX_LEN,   // Prefix length
                "",                     // Gateway address
                0,                      // Gateway metric
            },

            // DNS paramters
            {
                DNS_STATIC,             // DNS server Mode
                "",                     // Primary DNS address
                ""                      // secondary DNS address
            },
        }
    }
};

const MAC_SERVER_CNFG_t DfltMacServerCfg =
{
    CONNECT_THROUGH_HOSTNAME,       // Connect via host name
    "192.168.1.1",                  // IP Address of the MAC Server
    "matrixdnserv.com",             // URL
    80,                             // port
    "MACService.svc",               //
    0x76F,                          // Key
    "629MAC403"                     // Default Password
};

const IP_FILTER_CONFIG_t DfltIpFilterCfg =
{
	DISABLE,																		// Filter Disable
	FILTERED_ALLOW,																	// Filter mode allow
	{
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <00 to 07> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <08 to 15> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <16 to 23> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <24 to 31> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <32 to 39> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <40 to 47> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <48 to 55> BLANK
		{"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},		// IP address range <56 to 63> BLANK
	}
};

const DDNS_CONFIG_t DfltDdnsCfg =
{
	DISABLE,						// Disable DDNS service
    DDNS_SERVER_DYN_DNS_ORG,        // Default DYNDNS.ORG
	"",								// username, BLANK
	"",								// Password, BLANK
	"",								// Hostname, BLANK
	DFLT_DDNS_UPDATE_DURATION,
	// UTF-8 multilanguage
	"",								// username, BLANK
	"",								// Password, BLANK
	""								// Hostname, BLANK
};

const SMTP_CONFIG_t DfltSmtpCfg =
{
	DISABLE,						// Disable SMTP server
	"",								// SMTP server name, BLANK
	DFLT_SMTP_PORT,					// SMTP server port, 25
	"",								// username, BLANK
	"",								// password, BLANK
	"",								// sender's mail address, BLANK
    SMTP_ENCRYPTION_TYPE_NONE       // no email encryption
};

const FTP_UPLOAD_CONFIG_t DfltFtpUploadCfg =
{
	DISABLE,						// Disable FTP service
	"",								// FTP server name
	DFLT_FTP_PORT,					// FTP port number
	"",								// FTP user name
	"",								// FTP password
	""								// FTP upload path
};

const TCP_NOTIFY_CONFIG_t DfltTcpNotifyCfg =
{
	DISABLE,						// Disable TCP notification
	"",								// TCP server name, BLANK
	DFLT_TCP_NOTIFY_PORT			// TCP port number, 80
};

const FILE_ACCESS_CONFIG_t DfltFileAccessCfg =
{	
	DISABLE,						// enable FTP service
	DFLT_FTP_PORT					// FTP port number
};

const HDD_CONFIG_t DfltHddCfg =
{
	SINGLE_DISK_VOLUME,				// HDD mode, single disk volume
	LOCAL_HARD_DISK
};

const MATRIX_DNS_SERVER_CONFIG_t DfltMatrixDnsServerCfg =
{
	DISABLE,
	"",
	DFLT_MATRIX_DNS_SERVER_FORWARDED_PORT
};

const USER_ACCOUNT_CONFIG_t DfltUserAccountCfg =
{
    "",                                                                 // user name
    ADMIN,                                                              // user group
    "",                                                                 // password
    DISABLE,                                                            // userStatus
    ENABLE,                                                             // multiLogin
    DFLT_LOGIN_LIMIT_DURATION,                                          // loginSessionDuration
	{
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 01 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 02 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 03 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 04 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 05 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 06 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 07 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 08 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 09 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 10 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 11 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 12 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 13 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 14 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 15 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 16 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 17 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 18 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 19 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 20 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 21 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 22 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 23 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 24 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 25 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 26 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 27 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 28 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 29 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 30 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 31 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 32 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 33 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 34 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 35 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 36 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 37 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 38 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 39 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 40 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 41 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 42 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 43 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 44 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 45 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 46 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 47 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 48 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 49 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 50 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 51 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 52 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 53 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 54 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 55 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 56 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 57 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 58 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 59 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 60 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 61 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 62 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 63 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 64 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 65 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 66 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 67 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 68 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 69 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 70 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 71 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 72 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 73 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 74 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 75 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 76 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 77 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 78 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 79 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 80 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 81 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 82 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 83 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 84 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 85 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 86 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 87 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 88 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 89 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 90 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 91 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 92 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 93 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 94 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 95 privilege
        {{ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE, ENABLE}},		// camera 96 privilege
    },
    ENABLE,																// syncNetDev
    "English",                                                          // preffered language
    DISABLE                                                             // manage push notification rights
};

const CAMERA_CONFIG_t DfltCameraCfg =
{
    DISABLE,                    // Enable/Disable Camera
    "",                         // Camera Name
	IP_CAMERA,
	ENABLE,
	DFLT_MOTION_RE_DETECTION_DELAY,
	OSD_POS_TOP_LEFT,
	OSD_POS_BOTTOM_RIGHT,
	ENABLE,
    OSD_POS_BOTTOM_LEFT,
	DISABLE,
    {"", "", "", "", "", ""},   // channel name
    {OSD_POS_BOTTOM_LEFT, OSD_POS_BOTTOM_LEFT, OSD_POS_BOTTOM_LEFT, OSD_POS_BOTTOM_LEFT, OSD_POS_BOTTOM_LEFT, OSD_POS_BOTTOM_LEFT},
	"",
	DISABLE,
    DISABLE,
	MAIN_STREAM,
};

const STREAM_CONFIG_t DfltCameraStreamCfg =
{
    "",                         // video encoding method
    "",                         // image resolution
    0,                          // frame rate
    0,                          // quality of stream
    DISABLE,                    // control to enable/disable audio
	VBR,
	BITRATE_2048,
	DFLT_CAM_GOP,
	"",
    "",                         // sub image resolution
    0,                          // sub frame rate
    0,                          // sub quality of stream
    DISABLE,                    // sub control to enable/disable audio
	VBR,
	BITRATE_1024,
	DFLT_CAM_GOP,
	DFLT_MAIN_STREAM_PROFILE, 	// main Stream profile no
	DFLT_SUB_STREAM_PROFILE 	// sub Stream profile no
};

const SCHEDULE_RECORD_CONFIG_t DfltScheduleRecordCfg =
{
	ENABLE,
	{

		{	// Schedule Record for Sunday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		},

		{	// Schedule Record for Monday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		},

		{	// Schedule Record for Tuesday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		},

		{	// Schedule Record for Wednesday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		},

		{	// Schedule Record for Thursday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		},

		{	// Schedule Record for Friday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		},

		{	// Schedule Record for Saturday
            0x01,               // entireDay schedule recording enable, all other for adaptive disable
			{
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}},
				{{00,00}, {00,00}}
			}
		}
	}
};

const ALARM_RECORD_CONFIG_t DfltAlarmRecordCfg =
{
	DFLT_PRE_RECORD_TIME,
	DFLT_POST_RECORD_TIME
};

const PTZ_PRESET_CONFIG_t DfltPtzPresetCfg =
{
    "",
    ""
};

const PRESET_TOUR_CONFIG_t DfltPresetTourCfg =
{
	DISABLE,
	DFLT_MANUAL_TOUR,
	{
		{
			"",
			LOOPING,
			0,
			{
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10}
			},
		},
		{
			"",
			LOOPING,
			0,
			{
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10}
			},
		},
		{
			"",
			LOOPING,
			0,
			{
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10}
			},
		},
		{
			"",
			LOOPING,
			0,
			{
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10}
			},
		},
		{
			"",
			LOOPING,
			0,
			{
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10},
				{0, 10}, {0, 10}, {0, 10}, {0, 10}, {0, 10}
			},
		},
	},
	DISABLE
};


const TOUR_SCHEDULE_CONFIG_t DfltTourScheduleCfg =
{
	{
		{	//******* schedule for SUNDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		},
		{	//******* schedule for MONDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		},
		{	//******* schedule for TUESDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		},
		{	//******* schedule for WEDNESDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		},
		{	//******* schedule for THURSDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		},
		{	//******* schedule for FRIDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		},
		{	//******* schedule for SATURDAY
			DISABLE,
			DFLT_SCHEDULE_TOUR,
			{
				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				},

				{
					{{00, 00},{00, 00}},
					DFLT_SCHEDULE_TOUR
				}
			}
		}
	}
};

const IMAGE_UPLOAD_CONFIG_t DfltImageUploadCfg =
{
	"",
	UPLOAD_TO_FTP_SERVER1,
	{
		"",
		"",
		""
	},
	DFLT_IMAGE_UPLOAD_PER_MINUTE
};

const SENSOR_CONFIG_t DfltSensorCfg =
{
    DISABLE,                    // Disable sensor input
    "",                         // name of the sensor
    NORMALLY_OPEN,              // operating mode, normally open
    DFLT_DEBOUNCE_TIME          // de-bounce zero
};

const ALARM_CONFIG_t DfltAlarmCfg =
{
	ENABLE,						// enable alarm output
	"",							// name of the alarm
	ALARM_INTERLOCK,			// action, event based
	DFLT_ALARM_TIME_PERIOD		// time in seconds, if time based
};

const STORAGE_CONFIG_t DfltStorageCfg =
{
	// HDD full options
	{
		OVERWRITE,				// overwrite oldest file
		DFLT_HDD_FULL_CLEANUP	// clean up HDD to 10% to make room for recording
	},

	DISABLE,
	RECORDING_DRIVE_WISE_RECORD_RETENTION,
	DFLT_HDD_CLEANUP_BY_DAYS,
	// storage alert options
	{
        ENABLE,                 // enable storage alert
        DFLT_HDD_ALERT_CAPACITY // when disk space is lesser than 10GB
	},
	DISABLE,
    {
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 01 - 04
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 05 - 08
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 08 - 12
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 13 - 16
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 17 - 20
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 21 - 24
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 25 - 28
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 29 - 32
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 33 - 36
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 37 - 40
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 41 - 44
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 45 - 48
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 49 - 52
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 53 - 56
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 57 - 60
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 61 - 64
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 65 - 68
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 69 - 72
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 73 - 76
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 77 - 80
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 81 - 84
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 85 - 88
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 89 - 92
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 92 - 96
    },
    {
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 01 - 04
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 05 - 08
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 08 - 12
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 13 - 16
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 17 - 20
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 21 - 24
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 25 - 28
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 29 - 32
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 33 - 36
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 37 - 40
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 41 - 44
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 45 - 48
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 49 - 52
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 53 - 56
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 57 - 60
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 61 - 64
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 65 - 68
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 69 - 72
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 73 - 76
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 77 - 80
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 81 - 84
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 85 - 88
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 89 - 92
        DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, DFLT_CAMERA_WISE_CLEANUP_DAYS, // Camera 92 - 96
    },
};

const SCHEDULE_BACKUP_CONFIG_t DfltScheduleBackupCfg =
{
    DISABLE,                // enable schedule backup
    SB_EVERYDAY,
    SB_EVERY_6_HOUR,        // at every 6 hours
    {
        00                  // daily at this time instance
    },
    {
        SUNDAY,             // weekly at this day
        00                  // and at this time
    },
    {
        {
            UINT64_MAX,     // back up enabled for all camera
            UINT64_MAX
        }
    },
    SB_TO_USB_DEVICE        // backup to usb device
};

const MANUAL_BACKUP_CONFIG_t DfltManualBackupCfg =
{
    DISABLE,                // manual back up, disabled
    MB_LAST_1_HOUR,         // backup files of last 1 hours
    {
        {
            UINT64_MAX,     // back up enabled for all camera
            UINT64_MAX
        }
    },
	MB_TO_USB_DEVICE
};

const CAMERA_ALARM_CONFIG_t DfltCameraAlarmCfg =
{
	ALARM_INTERLOCK,
	10
};

const CAMERA_EVENT_CONFIG_t DfltCameraEventActionCfg =
{
    DISABLE,                // action disabled

	{
		{	//******* Record Camera control flags
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	//******* upload image control flags
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	// email ID, subject, message
			"",
			"%E",
			"%E at %T on %D"
		},

		{	// tcp notification message
			"%E at %T on %D"
		},

		{	// mobile, message parameter
			"",
			"",
			"%E at %T on %D"
		},

		{	// camera number and preset position
			1,
			0
		},

		{	// system alarm output control flag
			DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	// camera alarm output control flag
			1,
            {DISABLE, DISABLE, DISABLE},    // disable camera output
		},
	},

	{
		{	//******* event action schedule for sunday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for monday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for tuesday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for wednesday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for thursday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for friday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for saturday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		}
	},

    {
        { 0, 0 }    // copy to camera
    }

};

const SENSOR_EVENT_CONFIG_t DfltSensorEventActionCfg =
{
    DISABLE,// action disabled

	{
		{	//******* Record Camera control flags
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	//******* upload image control flags
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	// email ID, subject, message
			"",
			"%E",
			"%E at %T on %D"
		},

		{	// tcp notification message
			"%E at %T on %D"
		},

		{	// mobile, message parameter
			"",
			"",
			"%E at %T on %D"
		},

		{	// camera number and preset position
			1,
			0
		},

		{	// system alarm output control flag
			DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	// camera alarm output control flag
			1,
            {DISABLE, DISABLE, DISABLE},    // disable camera output
		}
	},

	{
		{	//******* event action schedule for sunday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for monday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for tuesday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for wednesday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for thursday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for friday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		},
		{	//******* event action schedule for saturday
			DISABLE,																					// entire day action flag
            {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}},
			{
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				},
				{
					{00,00}, {00,00},																	// time duration to accept event
                    {{DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE}}
				}
			}
		}
	}
};


const SYSTEM_EVENT_CONFIG_t DfltSystemEventActionCfg =
{
    ENABLE, // action disabled

	{
		{	//******* Record Camera control flags
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	//******* upload image control flags
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
			DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
            DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	// email ID, subject, message
			"",
			"%E",
			"%E at %T on %D"
		},

		{	// tcp notification message
			"%E at %T on %D"
		},

		{	// mobile, message parameter
			"",
			"",
			"%E at %T on %D"
		},

		{	// camera number and preset position
			1,
			0
		},

		{	// system alarm output control flag
			DISABLE, DISABLE, DISABLE, DISABLE,
		},

		{	// camera alarm output control flag
			1,
			{DISABLE, DISABLE, DISABLE},				// disable camera output
		}
	},

	{
		{	// entire day action flag
            ENABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE
		}
	}
};

//************* Default Cosec Pre Record Settings ****************//
const COSEC_REC_PARAM_CONFIG_t	DfltCosecPreRecConfig =
{
	DISABLE,
	DEFAULT_COSEC_PRE_REC_DURATION
};

// Default Static Routing configurations
const STATIC_ROUTING_CONFIG_t DfltStaticRoutingCfg =
{
	(LAN1_PORT + 1),
	{
		{
			"",
			0,
			0
		},
		{
			"",
			0,
			0
		},
		{
			"",
			0,
			0
		},
		{
			"",
			0,
			0
		},
		{
			"",
			0,
			0
		}
	}
};

const BROAD_BAND_CONFIG_t DfltBroadBandCfg =
{
	0,
	{
		{
			"Airtel",
			"*99#",
			"",
			"",
			"airtelgprs.com"
		},
		{
			"BSNL",
            "*99#",
			"",
			"",
			"bsnl.net"
		},
		{
            "VI",
			"*99#",
			"",
			"",
			"portalnmms"
		},
		{
            "Jio",
            "*99#",
            "",
            "",
            "jionet"
		},
		{
            "Custom",
            "",
			"",
			"",
			""
		}
	}
};

//************ Default SMS Settings **************
const SMS_CONFIG_t DfltSmsCfg =
{
	SMS_MODE_HTTP,
	SMS_SERVER_SMS_GATEWAYE,
	"",
	"",
	"",
	FALSE
};

//************ Default Manual Record Configurations **************
const MANUAL_RECORD_CONFIG_t DfltManualRecordCfg =
{
    ENABLE,
};

//******** Default Network Drive Configuration ****************
const NETWORK_DRIVE_CONFIG_t DfltNetworkDriveCfg =
{
	DISABLE,			// Network drive enable
	"",					// Name of the drive
	"",					// ip address of the drive
	"",					// User name
	"",					// Password
	CIFS_FILE_SYS,		// file system
	""					// Default folder
};

//************* Default Ip Camera Configuration ****************//
const IP_CAMERA_CONFIG_t DfltIpCameraCfg =
{
    "",                 // Camera Brand
    "",                 // Camera Model
    "/video.cgi",       // Camera URL
    "",                 // Camera IP,
    DFLT_HTTP_PORT,     // Camera HTTP port
    DFLT_RTSP_PORT,     // Camera RTSP port
    "",                 // user name
    "",                 // password
    FALSE,              // ONVIF support flag
    DFLT_ONVIF_PORT,    // ONVIF port
    RTSP_OVER_TCP,      // RTSP Protocol
    "",                 // MAC addr
};

//************* Default Network Device Configuration ****************//
const NETWORK_DEVICE_CONFIG_t DfltNetworkDeviceCfg =
{
	"",
	REGISTER_MODE_IP_ADDRESS,
	"",
	DFLT_PORT_ADDRESS,
	DFLT_USERNAME,
	"",
	DISABLE,
	ENABLE,
	LIVE_STREAM_MAIN,
	DISABLE,
	DFLT_FORWARDED_TCP_PORT,
};

const SNAPSHOT_CONFIG_t DfltSnapShotCfg =
{
	DISABLE,
	UPLOAD_TO_EMAIL_SERVER,
	{
		"",
		"",
		""
	},
	DFLT_IMAGE_UPLOAD_PER_MINUTE
};

const SNAPSHOT_SCHEDULE_CONFIG_t DfltSnapShotSchdCfg =
{
	{
		DISABLE,
		{
			{{00,00}, {00,00}},
			{{00,00}, {00,00}},
			{{00,00}, {00,00}},
			{{00,00}, {00,00}},
			{{00,00}, {00,00}},
			{{00,00}, {00,00}}
		},
	},
	DEFAULT_SNAPSHOT_WEEKSELECTED
};

const LOGIN_POLICY_CONFIG_t DfltLoginPolicyCfg =
{
	DEFAULT_PASSWORD_POLICY_PASSWORD_LEN,
	DISABLE,
	DEFAULT_PASSWORD_RESET_DURATION,
	LOGIN_LOCK_DISABLE,
	DEFAULT_PASSWORD_INVALID_ATTEMPT,
	DEFAULT_AUTO_LOCK_TIMER,
};

const AUDIO_OUT_CONFIG_t DfltAudioOutCfg =
{
    AUDIO_OUT_PRIORITY_CLIENT
};

const P2P_CONFIG_t DfltP2PCfg =
{
    DISABLE,
    DISABLE
};

const IMG_SETTING_CONFIG_t DfltImageSettingCfg =
{
    50,                         /* brightness */
    50,                         /* contrast */
    50,                         /* saturation */
    50,                         /* hue */
    50,                         /* sharpness */
    WHITE_BALANCE_MODE_AUTO,    /* white balance */
    WDR_MODE_OFF,               /* WDR mode */
    50,                         /* WDR strength */
    BACKLIGHT_MODE_OFF,         /* backlight control */
    EXPOSURE_RATIO_MODE_AUTO,   /* exposure ratio mode */
    8,                          /* exposure ratio */
    EXPOSURE_MODE_AUTO,         /* exposure mode */
    FLICKER_MODE_NONE,          /* flicker mode */
    0,                          /* flicker strength */
    HLC_MODE_OFF,               /* hlc mode */
    40000,                      /* exposure time */
    20,                         /* exposure gain */
    0,                          /* exposure iris */
    NORMAL_LIGHT_GAIN_64X,      /* normal light max gain */
    4,                          /* normal light average luminance */
    LED_MODE_AUTO,              /* LED mode */
    6,                          /* LED sensitivity */
};

const DHCP_SERVER_CONFIG_t DfltDhcpServerCfg =
{
    DISABLE,
    LAN1_PORT,
    "",
    8,
    "",
    "",
    24,
};

const FIRMWARE_MANAGEMENT_CONFIG_t DfltFirmwareManagementCfg =
{
    AUTO_FIRMWARE_UPGRADE_NEVER,
    {0, 0},
    FIRMWARE_FTP_SERVER_MATRIX,
    {
        {
            DISABLE,                        // Dummy
            "matrixtelecomsolutions.com",   // Firmware server address
            DFLT_FTP_PORT,					// Firmware server port
            "MatrixNVRread",                // Firmware server user name
            "4jVm03@q",                     // Firmware server password
            #if defined(HI3536_NVRH)
            "SecurityProducts/SATATYA/SATATYA_NVRX/NVR1602X_32X_64X",   /* Hi-silicon NVRH firmware download path */
            #elif defined(HI3536_NVRL)
            "SecurityProducts/SATATYA/SATATYA_NVRX/NVR8X_1601X",        /* Hi-silicon NVRL firmware download path */
            #elif defined(RK3568_NVRL)
            "SecurityProducts/SATATYA/SATATYA_NVRX/NVR8XP2_16XP2",      /* Rockchip NVRL firmware download path */
            #elif defined(RK3588_NVRH)
            "SecurityProducts/SATATYA/SATATYA_NVRX/NVR32XP2_64XP2",     /* Rockchip NVRH firmware download path */
            #endif
        },
        {
            DISABLE,                        // Dummy
            "",                             // Firmware server address
            DFLT_FTP_PORT,					// Firmware server port
            "",                             // Firmware server user name
            "",                             // Firmware server password
            "",                             // Firmware server download path
        }
    }
};

const FCM_PUSH_NOTIFY_CONFIG_t DfltFcmPushNotificationCfg =
{
    "",                                     // Username
    "",                                     // FCM token
    "",                                     // Phone model name
};

const PASSWORD_RECOVERY_CONFIG_t DfltPasswordRecoveryCfg =
{
    "",                                     // email-ID
    {{0, ""}, {1, ""}, {2, ""}}             // security questions info (question id & answer)
};

const STORAGE_ALLOCATION_CONFIG_t DfltStorageAllocationCfg =
{
    {
        /* Allocate all volumes and all cameras in group 1 bydefault */
        {UINT16_MAX, {{UINT64_MAX, UINT64_MAX}}},   /* Volume and camera allocation to Group 1 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 2 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 3 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 4 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 5 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 6 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 7 */
        {         0, {{         0,          0}}},   /* Volume and camera allocation to Group 8 */
    }
};

//#################################################################################################
// @END OF FILE
//#################################################################################################
