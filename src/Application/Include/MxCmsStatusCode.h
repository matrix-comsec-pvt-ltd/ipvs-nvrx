#if !defined MXCMSSTATUSCODE_H
#define MXCMSSTATUSCODE_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxCmsStatusCode.h
@brief      It provides status codes of the network commands to be returned to CMS.
*/
//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum NET_CMD_STATUS_e
{
    CMD_SUCCESS = 0,                            // 00 : command success [generic]
    CMD_INVALID_MESSAGE,                        // 01 : error 0x01 [network manager]
    CMD_INVALID_SESSION,                        // 02 : error 0x02 [network manager]
    CMD_INVALID_SYNTAX,                         // 03 : error 0x03 [network manager]
    CMD_IP_BLOCKED,                             // 04 : IP blocked; connection failed [network manager]
    CMD_INVALID_CREDENTIAL,                     // 05 : invalid credential; login failed [network manager]
    CMD_USER_DISABLED,                          // 06 : User Default
    CMD_USER_BLOCKED,                           // 07 : User Block
    CMD_MULTILOGIN,                             // 08 : Multi login not allow
    CMD_MAX_USER_SESSION,                       // 09 : maximum user session reached; login failed [network manager]
    CMD_RESOURCE_LIMIT,                         // 10 : resource not available [generic]
    CMD_NO_PRIVILEGE,                           // 11 : access denied; no privilege [network manager]
    CMD_INVALID_TABLE_ID,                       // 12 : invalid table id [configuration command]
    CMD_INVALID_INDEX_ID,                       // 13 : invalid index id [configuration command]
    CMD_INVALID_FIELD_ID,                       // 14 : invalid field id [configuration command]
    CMD_INVALID_INDEX_RANGE,                    // 15 : invalid index range [configuration command]
    CMD_INVALID_FIELD_RANGE,                    // 16 : invalid field range [configuration command]
    CMD_INVALID_FIELD_VALUE,                    // 17 : invalid field value [configuration command]
    CMD_MAX_STREAM_LIMIT,                       // 18 : maximum stream limit reached [media streamer]
    CMD_NO_PTZ_PROTOCOL,                        // 19 : DVR use
    CMD_NO_RECORD_FOUND,                        // 20 : no record found for search criteria [disk manager]
    CMD_SCHEDULE_TOUR_ON,                       // 21 : schedule tour is on; manual tour not allowed [PTZ tour]
    CMD_RECORDING_ON,                           // 22 : recording is on; manual tour not allowed [record manager / PTZ tour]
    CMD_MAX_BUFFER_LIMIT,                       // 23 : error 0x23 [generic]
    CMD_PROCESS_ERROR,                          // 24 : operation failed [generic]
    CMD_TESTING_ON,                             // 25 : testing already in process [TCP / SMTP / SMS / FTP]
    CMD_RESERVED_26,                            // 26 : Reserved (Used in DVR)
    CMD_NO_MODEM_FOUND,                         // 27 : no modem found [SMS]
    CMD_SIM_REGISTER_FAIL,                      // 28 : SIM registration failed [SMS]
    CMD_TESTING_FAIL,                           // 29 : testing failed [TCP / SMTP / SMS / FTP]
    CMD_SERVER_DISABLED,                        // 30 : server is disabled [TCP / SMTP / SMS / FTP]
    CMD_CONNECTIVITY_ERROR,                     // 31 : error in connection [TCP / SMTP / SMS / FTP]
    CMD_NO_WRITE_PERMISSION,                    // 32 : no write permission [FTP]
    CMD_INVALID_FILE_NAME,                      // 33 : invalid file name [system upgrade]
    CMD_CHANNEL_DISABLED,                       // 34 : camera disabled [camera interface]
    CMD_CHANNEL_BLOCKED,                        // 35 : DVR use
    CMD_SUB_STREAM_DISABLED,                    // 36 : DVR use
    CMD_BACKUP_IN_PROCESS,                      // 37 : backup in process; unable to format disk [disk manager / backup]
    CMD_ALARM_DISABLED,                         // 38 : alarm output disabled [IO]
    CMD_TOUR_NOT_SET,                           // 39 : manual PTZ tour not configured [PTZ tour]
    CMD_MAN_RECORD_DISABLED,                    // 40 : manual record is disabled [PTZ tour]
    CMD_NO_DISK_FOUND,                          // 41 : no Disk found [disk manager]
    CMD_NO_EVENT_FOUND,                         // 42 : no events found [event logger]
    CMD_ADMIN_CANT_BLOCKED,                     // 43 : admin user can not block
    CMD_ADDRESS_ALREADY_ASSIGNED,               // 44 : DVR use (PTZ address already assigned)
    CMD_FILE_EXTRACT_ERROR,                     // 45 : file extraction error [system upgrade]
    CMD_FILE_MISSING,                           // 46 : upgrade file missing [system upgrade]
    CMD_REQUEST_IN_PROGRESS,                    // 47 : request already in process [generic]
    CMD_NO_EVENTS,                              // 48 : in response to long poll [network manager]
    CMD_EVENT_AVAILABLE,                        // 49 : in response to long poll [network manager]
    CMD_MORE_DATA,                              // 50 : more data / records available [record manager]
    CMD_MORE_EVENTS,                            // 51 : more events available [event logger]
    CMD_AUDIO_DISABLED,                         // 52 : audio disabled
    CMD_INVALID_UPDATE_MODE,                    // 53 : invalid mode of time update [date n time]
    CMD_STREAM_ALREADY_ON,                      // 54 : stream is already on [camera interface]
    CMD_PLAYBACK_STREAM_ON,                     // 55 : play-back already on [media streamer]
    CMD_MANUAL_RECORDING_ON,                    // 56 : manual recording already on [record manager]
    CMD_MANUAL_TOUR_ON,                         // 57 : manual tour already on [PTZ tour]
    CMD_MANUAL_RECORDING_OFF,                   // 58 : manual recording already off [PTZ tour]
    CMD_MANUAL_TOUR_OFF,                        // 59 : manual tour already off [PTZ tour]
    CMD_INVALID_SEQUENCE,                       // 60 : Invalid file transfer sequence number
    CMD_INVALID_DATA_LENGTH,                    // 61 : Invalid file transfer data length
    CMD_INVALID_FILE_LENGTH,                    // 62 : Invalid file transfer total file size
    CMD_INVALID_FILE_SIZE,                      // 63 : Invalid file size
    CMD_UPGRADE_IN_PROCESS,                     // 64 : system upgrade is going on [system upgrade]
    CMD_FORMAT_IN_PROCESS,                      // 65 : format is going on [disk manager]
    CMD_DDNS_UPDATE_FAILED,                     // 66 : DDNS update fail
    CMD_CAM_REQUEST_IN_PROCESS,                 // 67 : Previous camera request already in progess
    CMD_FEATURE_NOT_SUPPORTED,                  // 68 : Feature not supported
    CMD_CODEC_NOT_SUPPORTED,                    // 69 : Codec not supported
    CMD_CAM_REQUEST_FAILED,                     // 70 : Camera Request fail
    CMD_CAM_DISCONNECTED,                       // 71 : Camera disconnected
    CMD_USB_DEVICE_IN_USE,                      // 72 : USB Device still in use
    CMD_FTP_INVALID_CREDENTIAL,                 // 73 : Invalid user name password for FTP
    CMD_BACKUP_NOT_IN_PROG,                     // 74 : Backup not in progress
    CMD_MAN_TRG_ALREADY_ON,                     // 75 : manual trigger already on
    CMD_MAN_TRG_ALREADY_OFF,                    // 76 : manual trigger already off
    CMD_CONFIG_RESTORE_IN_PROCESS,              // 77 : operation failed,Configuration restore in process
    CMD_ERROR_SUBMIT_ROUTE_TABLE,               // 78 : Destination address should not be in the Subnet of LAN1/LAN2
    CMD_DEST_ADDR_IS_NOT_NW_ADDR,               // 79 : Destination Address should be the network address
    CMD_UNABLE_TO_FORMAT,                       // 80 : Another disk is being formatted,unable to format
    CMD_LAN1_LAN2_SAME_SUBNET,                  // 81 : LAN1 and LAN2 should not be in same network
    CMD_SNAPSHOT_FAILED,                        // 82 : Snapshot failed
    CMD_SNAPSHOT_SERVER_DISABLED,               // 83 : snapshot failed FTP Server disabled
    CMD_SNAPSHOT_CONNECTION_ERROR,              // 84 : snapshot failed Error in connection
    CMD_SNAPSHOT_NO_WRITE_PERMISSION,           // 85 : snapshot failed No write permission
    CMD_CAM_PARAM_NOT_CONFIGURED,               // 86 : command parameter not configured
    CMD_ONVIF_CAM_CAPABILITY_ERROR,             // 87 : Unable to get parameters from ONVIF camera
    CMD_NON_CONFIGURABLE_PARAMETER,             // 88 : Failed-Non Configurable parameter
    CMD_FIRMWARE_NOT_FOUND,                     // 89 : No Firmware Found
    CMD_FIRMWARE_UP_TO_DATE,                    // 90 : Your device firmware is Up to Date
    CMD_SMS_ACC_EXPIRED,                        // 91 : SMS Account expired
    CMD_SMS_ACC_INSUFF_CREDITS,                 // 92 : SMS Account insufficient balance
    CMD_INVALID_MOBILE_NO,                      // 93 : Invalid mobile number
    CMD_IP_SUBNET_MISMATCH,                     // 94 : IP and Subnet mismatch
    CMD_PLAYBACK_PROCESS_ERROR,                 // 95 : Playback processing error
    CMD_INVALID_HOST_SIZE,                      // 96 : Invalid host size
    CMD_FILE_DOWNGRED_FAIL,                     // 97 : Firmware file not downgrade
    CMD_HOST_NAME_DUPLICATION,                  // 98 : Matrix dns hostname duplicate
    CMD_HOST_NAME_REG_FAIL,                     // 99 : Matrix dns hostname reg fail
    CMD_MAX_CAMERA_CONFIGURED,                  // 100 : Max Camera configured
    CMD_CRED_INVALID,                           // 101 : Invalid cred
    CMD_RESERVED_102,                           // 102 : Reserved
    CMD_REC_MEDIA_ERR,                          // 103 : Recording Mediam related functionality running so recording can not started.
    CMD_REC_MEDIA_FULL,                         // 104 : Recording mediam full
    CMD_RESERVED_105,                           // 105 : Reserved (Used in DVR)
    CMD_DUP_IP_ADDR,                            // 106 : LAN1 and LAN2 same IP Address
    CMD_BRND_MDL_MISMATCH,                      // 107 : Brand and model Mismatch
    CMD_IP_GATEWAY_SAME_SUBNET,                 // 108 : IP and Default gateway should be in Same subnet
    CMD_REC_DRIVE_CONFIG_CHANGE,                // 109 : Hdd formatting so sync command not allowed
    CMD_GENERIC_CAM_STREAM_CONFIG,              // 110 : Stream Configuration not supported in Generic Camera
    CMD_CAM_STREAM_CONFIG_ERROR,                // 111 : Stream Configuration not supported for this Camera
    CMD_SUB_STREAM_NOT_SUPPORTED,               // 112 : Sub Stream not supported
    CMD_MAX_IP_CAMERA_CONFIGURED,               // 113 : All Camera are configured default one to add new
    CMD_RESERVED_114,                           // 114 : Reserved (Used in DVR)
    CMD_AVI_SEARCH_NOT_ALLOWED,                 // 115 : Search in avi not allowed
    CMD_LOGIN_SESSION_DURATION_OVER,            // 116 : If Allowed Access Duration limit is reached
    CMD_INSTANT_PLAYBACK_FAIL,                  // 117 : unable to play instant plaback
    CMD_MOTION_WINDOW_FAILED,                   // 118 : Unable to set configuration at camera end
    CMD_IP_CHANGE_FAIL,                         // 119 : when changeCamIpAddrUrl gets failed
    CMD_REBOOT_FAIL,                            // 120 : after chaning the ip address reboot url gets failed
    CMD_NO_CAMERA_FOUND,                        // 121 : While searching if no camera is detected
    CMD_AUTO_SEARCH_COMPLETE,                   // 122 : auto Search compelete
    CMD_ACTIVE_TOUR_PAUSE,                      // 123 : preset tour pause
    CMD_AUTO_SEARCH_START,                      // 124 : indicate auto search process is started
    CMD_AUTO_CONFIG_RANGE_INVALID,              // 125 : indicate auto config ip address range is invalid
    CMD_RESET_PASSWORD,                         // 126 : password is not as per password policy
    CMD_PASSWORD_EXPIRE,                        // 127 : password is expire, now reset password
    CMD_USER_ACCOUNT_LOCK,                      // 128 : user is lock for lock duration.
    CMD_MIN_PASSWORD_CHAR_REQUIRED,             // 129 : minimum password characters required
    CMD_HIGH_PASSWORD_SEC_REQ,                  // 130 : high password security required
    CMD_MEDIUM_PASSWORD_SEC_REQ,                // 131 : medium password security required
    CMD_STREAM_NOT_AVAILABLE,                   // 132 : camera stream not available (For Multi OS)
    CMD_REQ_FAIL_CHNG_AUD_OUT_PRI,              // 133 : Request failed. Change Audio Out priority
    CMD_AUD_CHANNEL_BUSY,                       // 134 : Audio channel busy
    CMD_NO_AUD_OUT_AVAILABLE,                   // 135 : No Audio Out port available
    CMD_AUD_SND_REQ_FAIL,                       // 136 : Audio sending request failed
    CMD_AUD_SND_STP_PRO_LCL_CLNT_REQ,           // 137 : Audio sending stopped. Processing local client request
    CMD_HDD_PORT_COMBINATION_WRONG,             // 138 : Please check current HDD combination
    CMD_HDD_LOGICAL_VOLUME_SIZE,                // 139 : RAID Creation failed as total logical volume size exceeded maximum allowed volume size

	CMD_SMTP_SERVER_CONNECTION_ERROR = 1000,	// 1000 : Cannot connect to given Server & Port.
	CMD_SMTP_CONNECTION_REFUSED,				// 1001 : Connection Refused by SMTP server.
	CMD_SMTP_SERVER_UNAVAILABLE, 				// 1002 : SMTP server not available at the moment
	CMD_SMTP_RECIPIENT_MAILBOX_FULL,			// 1003 : Recipient Mailbox full.
	CMD_SMTP_RECIPIENT_SERVER_NOT_RESPONDING,	// 1004 : Recipient server not responding.
	CMD_SMTP_MAIL_NOT_ACCEPTED,					// 1005 : Mail not accepted.
	CMD_SMTP_AUTHENTICATION_FAILED,				// 1006 : Authentication Failed.Verify Entered Details.
	CMD_SMTP_INVALID_EMAIL_ADDR,				// 1007 : Sender/Recipient Mailbox address invalid or unavailable.
	CMD_SMTP_SERVER_STORAGE_FULL,				// 1008 : SMTP server storage limit exceeded.
	CMD_SMTP_TRANSACTION_FAILED,				// 1009 : Transaction Failed. Email is Spam/Blacklisted.

	CMD_LANG_FILE_NOT_FOUND,					// 1010 : Language File Not Found
	CMD_LANG_IMPORT_FAILED,						// 1011 : Language File Import Failed.
	CMD_LANG_EXPORT_FAILED,						// 1012 : Language File Export Failed.
	CMD_LANG_DELETE_FAILED,						// 1013 : Error while deleting language file
	CMD_LANG_TRANSLATION_FAILED,				// 1014 : Error while translating language file.
	CMD_MAX_LANG_IMPORTED,						// 1015 : Max Languages imported
	CMD_LANG_INVALID_FILE,						// 1016 : Invalid Language file
    CMD_LANG_TRANSLATING_ERROR,                 // 1017 : Language file contains '&' abd '<'

    CMD_INVALID_USERNAME = 1019,                // 1019 : Invalid username
    CMD_MISSING_VERIFICATION_DETAILS,           // 1020 : Verification details are not found

    CMD_MISMATCH_SECURITY_ANSWERS = 1022,       // 1022 : Wrong Answers
    CMD_MISMATCH_OTP,                           // 1023 : Mismatch OTP
    CMD_EMAIL_SERVICE_DISABLED,                 // 1024 : Email Service is not enabled
    CMD_PWD_RST_SESSION_NOT_AVAILABLE,          // 1025 : Password reset session limit exceeded
    CMD_PUSH_NOTIFICATION_NOT_ALLOWED,          // 1026 : Push Notifcation flag in User account is disabled
    CMD_PUSH_NOTIFICATION_DISABLED,             // 1027 : Push Notifcation is disabled for given user & fcm token

    CMD_PWD_RST_ALREADY_IN_PROGRESS = 1030,     // 1030 : Password reset already in progress
    CMD_SESSION_EXPIRED                         // 1031 : Session Expired

}NET_CMD_STATUS_e;

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // MXCMSSTATUSCODE_H
