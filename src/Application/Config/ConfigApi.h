#if !defined CONFIG_API_H
#define CONFIG_API_H

#ifdef __cplusplus
extern "C" {
#endif
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   ConfigApi.h
@brief  This module is designed to allow the user to operate over different configuration parameters
        of the device. Concept of this module is adopted from the "Configuration Manager" of Access
        Control System. It provides functions to perform initialization, set to default, read and write
        operations over these parameters. Initialization function initializes the configuration module
        to the values either default or last set by user. Set to default function loads default values
        to the configuration. Write function updates configuration with the user supplied values, only
        if they differ. Read function outputs the values of configuration to the user. Initialization
        of the user module must be done only after initialization of configuration module. It is user
        modules responsibility to update user copy by reading it from configuration module, during
        initialization. Whenever configuration is changed, user module is notified, so that it can
        take appropriate action against the change and updating of user configuration copy.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"
#include "Utils.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
extern void NcGeneralCfgUpdate(GENERAL_CONFIG_t updateConfig, GENERAL_CONFIG_t *oldGenCfg);
//-------------------------------------------------------------------------------------------------
extern void ManualBackupCfgUpdate(MANUAL_BACKUP_CONFIG_t newCfg);
//-------------------------------------------------------------------------------------------------
extern void ScheduleBackupCfgUpdate(SCHEDULE_BACKUP_CONFIG_t *newCfg, SCHEDULE_BACKUP_CONFIG_t *oldCfg);
//-------------------------------------------------------------------------------------------------
extern void UtilsGeneralCfgUpdate(GENERAL_CONFIG_t generalConfig, GENERAL_CONFIG_t *oldgeneralConfig);
//-------------------------------------------------------------------------------------------------
extern void ImgUpldCfg(IMAGE_UPLOAD_CONFIG_t imageUploadCfg, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void UpdateDateTimeConfig(DATE_TIME_CONFIG_t newDateTimeConfig, DATE_TIME_CONFIG_t *oldDateTimeConfig);
//-------------------------------------------------------------------------------------------------
extern void UpdateDstConfig(DST_CONFIG_t newDstConfig);
//-------------------------------------------------------------------------------------------------
extern void UpdateDdnsConfig(DDNS_CONFIG_t newDdnsConfig, DDNS_CONFIG_t *oldDdnsConfig);
//-------------------------------------------------------------------------------------------------
extern void UpdateTourScheduleCfg(UINT8 camIndex, TOUR_SCHEDULE_CONFIG_t newTourSchCfg, TOUR_SCHEDULE_CONFIG_t *oldSchedule);
//-------------------------------------------------------------------------------------------------
extern void UpdateSensorConfig(SENSOR_CONFIG_t newSensorConfig, SENSOR_CONFIG_t *oldSensorConfig, UINT8 sensorNo);
//-------------------------------------------------------------------------------------------------
extern void UpdateAlarmConfig(ALARM_CONFIG_t newCfg, ALARM_CONFIG_t *oldCfg, UINT8 alarmNo);
//-------------------------------------------------------------------------------------------------
extern void EvntHndlrCamEventCfgUpdate(CAMERA_EVENT_CONFIG_t newCfg, CAMERA_EVENT_CONFIG_t *oldCfg, UINT8 camIndex, UINT8 camEventIndex);
//-------------------------------------------------------------------------------------------------
extern void EvntHndlrSensorEventCfgUpdate(SENSOR_EVENT_CONFIG_t newCfg, SENSOR_EVENT_CONFIG_t *oldCfg, UINT8 sensorIndex);
//-------------------------------------------------------------------------------------------------
extern void EvntHndlrSystemEventCfgUpdate(SYSTEM_EVENT_CONFIG_t newCfg, SYSTEM_EVENT_CONFIG_t *oldCfg, UINT8 systemIndex);
//-------------------------------------------------------------------------------------------------
extern void CiGeneralCfgUpdate(GENERAL_CONFIG_t updateConfig, GENERAL_CONFIG_t *oldGenCfg);
//-------------------------------------------------------------------------------------------------
extern void CiCameraConfigNotify(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void CiStreamConfigNotify(STREAM_CONFIG_t *newStreamConfig, STREAM_CONFIG_t *oldStreamConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void NwFileAccessCfgUpdate(FILE_ACCESS_CONFIG_t newFileAccessCfg);
//-------------------------------------------------------------------------------------------------
extern void UserAccountCfgUpdate(USER_ACCOUNT_CONFIG_t newUsrAccontCfg, USER_ACCOUNT_CONFIG_t *oldUsrAccontCfg, UINT8 usrIndx);
//-------------------------------------------------------------------------------------------------
extern void NwGeneralCfgUpdate(GENERAL_CONFIG_t newGenCfg, GENERAL_CONFIG_t *oldGenCfg);
//-------------------------------------------------------------------------------------------------
extern BOOL UpdateLanConfig(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig);
//-------------------------------------------------------------------------------------------------
extern void CiPtzPositionConfigNotify(PTZ_PRESET_CONFIG_t newPtzPresetCfg, PTZ_PRESET_CONFIG_t *oldPtzPresetCfg, UINT8 cameraIndex, UINT8 ptzIndex);
//-------------------------------------------------------------------------------------------------
extern void RmConfigChangeNotify(MANUAL_RECORD_CONFIG_t newManualRecordConfig, MANUAL_RECORD_CONFIG_t *oldManualRecordConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void UpdateTourCfg(UINT8 camIndex, PRESET_TOUR_CONFIG_t newTourCfg, PRESET_TOUR_CONFIG_t *oldPresetPtr);
//-------------------------------------------------------------------------------------------------
extern BOOL UpdateStaticRouting(STATIC_ROUTING_CONFIG_t newCopy, STATIC_ROUTING_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void MobileBroadbandCfgUpdate(BROAD_BAND_CONFIG_t newCopy, BROAD_BAND_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void DmFileAccessCfgUpdate(FILE_ACCESS_CONFIG_t newFileAccessCfg, FILE_ACCESS_CONFIG_t *oldFileAccessCfg);
//-------------------------------------------------------------------------------------------------
extern BOOL DmCfgChange(HDD_CONFIG_t newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
extern void DmStorageCfgUpdate(STORAGE_CONFIG_t newStorageConfig, STORAGE_CONFIG_t *oldStorageConfig);
//-------------------------------------------------------------------------------------------------
extern void NotifyAlarmConfigChange(CAMERA_ALARM_CONFIG_t newAlarmConfig, CAMERA_ALARM_CONFIG_t *oldAlarmConfig, UINT8 cameraIndex, UINT8 alarmIndex);
//-------------------------------------------------------------------------------------------------
extern BOOL GetNetworkParamInfo(NETWORK_PORT_e portType, LAN_CONFIG_t *networkInfo);
//-------------------------------------------------------------------------------------------------
extern BOOL DmNddCfgChange(NETWORK_DRIVE_CONFIG_t newNddConfig, NETWORK_DRIVE_CONFIG_t *oldNddConfig, UINT8 nwDriveIndex);
//-------------------------------------------------------------------------------------------------
extern BOOL DmFTPUploadCfgUpdate(FTP_SERVER_e index);
//-------------------------------------------------------------------------------------------------
extern void TcpConfigUpdate(TCP_NOTIFY_CONFIG_t newCopyPtr, TCP_NOTIFY_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void UpdateMatrixDnsServerCfg(MATRIX_DNS_SERVER_CONFIG_t *newCopy, MATRIX_DNS_SERVER_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern NET_CMD_STATUS_e ValidateCameraCfg(CAMERA_CONFIG_t *newCfg, CAMERA_CONFIG_t *oldCfg);
//-------------------------------------------------------------------------------------------------
extern NET_CMD_STATUS_e ValidatePtzPresetCfg(PTZ_PRESET_CONFIG_t *newCfg, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern NET_CMD_STATUS_e ValidateStaticRoutingCfg(STATIC_ROUTING_CONFIG_t newCfg);
//-------------------------------------------------------------------------------------------------
extern NET_CMD_STATUS_e ValidateLanConfig(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig);
//-------------------------------------------------------------------------------------------------
extern NET_CMD_STATUS_e ValidateHddConfig(HDD_CONFIG_t newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
extern void CiIpCameraConfigNotify(IP_CAMERA_CONFIG_t newIpCfg, IP_CAMERA_CONFIG_t *oldIpCfg, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void RMConfigNotify(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void DiGeneralConfigNotify(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig);
//-------------------------------------------------------------------------------------------------
extern void CiLanCfgUpdate(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig);
//-------------------------------------------------------------------------------------------------
extern void CiServerConfigChangeNotify(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig);
//-------------------------------------------------------------------------------------------------
extern void CiStreamCamConfigChange(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig,	UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
extern void LoginPolicyCfgUpdate(LOGIN_POLICY_CONFIG_t newLoginPolicyCfg, LOGIN_POLICY_CONFIG_t *oldLoginPolicyCfg);
//-------------------------------------------------------------------------------------------------
extern void P2pConfigNotify(P2P_CONFIG_t newP2PConfig, P2P_CONFIG_t *oldP2PConfig);
//-------------------------------------------------------------------------------------------------
extern void P2pLanConfigUpdate(LAN_CONFIG_ID_e lanNo);
//-------------------------------------------------------------------------------------------------
extern void P2pMobileBroadbandCfgUpdate(void);
//-------------------------------------------------------------------------------------------------
extern void P2pUpdateDefaultRoute(STATIC_ROUTING_CONFIG_t newCopy, STATIC_ROUTING_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void P2pDateTimeCfgUpdate(DATE_TIME_CONFIG_t newCopy, DATE_TIME_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void UpdateSmtpConfig(SMTP_CONFIG_t newCopy, SMTP_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void ImageSettingConfigNotify(IMG_SETTING_CONFIG_t newCopy, IMG_SETTING_CONFIG_t *oldCopy, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
extern void DhcpServerConfigNotify(DHCP_SERVER_CONFIG_t newCopy, DHCP_SERVER_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void DhcpServerLanConfigUpdate(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig);
//-------------------------------------------------------------------------------------------------
extern void FirmwareManagementConfigNotify(FIRMWARE_MANAGEMENT_CONFIG_t newCopy, FIRMWARE_MANAGEMENT_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern BOOL FcmPushNotifyConfigUpdate(FCM_PUSH_NOTIFY_CONFIG_t newCopy, FCM_PUSH_NOTIFY_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
extern void StorageAllocationConfigNotify(STORAGE_ALLOCATION_CONFIG_t newCopy, STORAGE_ALLOCATION_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @DEFINES
//#################################################################################################
#define GENERAL_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                   {NcGeneralCfgUpdate(NEW_COPY, OLD_COPY);        \
                                                                                     UtilsGeneralCfgUpdate(NEW_COPY, OLD_COPY);     \
                                                                                     CiGeneralCfgUpdate(NEW_COPY, OLD_COPY);        \
                                                                                     NwGeneralCfgUpdate(NEW_COPY, OLD_COPY);        \
                                                                                     DiGeneralConfigNotify(NEW_COPY,OLD_COPY);      \
                                                                                     CiServerConfigChangeNotify(NEW_COPY,OLD_COPY);}

#define DATE_TIME_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                 {UpdateDateTimeConfig(NEW_COPY, OLD_COPY);      \
                                                                                     P2pDateTimeCfgUpdate(NEW_COPY, OLD_COPY);}

#define DST_CONFIG_CHANGE(NEW_COPY)                                                 {UpdateDstConfig(NEW_COPY);}
#define LAN_CONFIG_CHANGE(LAN_IDX, NEW_COPY,OLD_COPY)                               {UpdateLanConfig(LAN_IDX, NEW_COPY, OLD_COPY);  \
                                                                                     CiLanCfgUpdate(LAN_IDX, NEW_COPY, OLD_COPY);   \
                                                                                     P2pLanConfigUpdate(LAN_IDX);                   \
                                                                                     DhcpServerLanConfigUpdate(LAN_IDX, NEW_COPY, OLD_COPY);}

#define DDNS_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                      {UpdateDdnsConfig(NEW_COPY, OLD_COPY);}
#define FTP_UPLOAD_CONFIG_CHANGE(INDEX)                                             {DmFTPUploadCfgUpdate(INDEX);}
#define TCP_NOTIFY_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                {TcpConfigUpdate(NEW_COPY, OLD_COPY);}
#define FILE_ACCESS_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                               {DmFileAccessCfgUpdate(NEW_COPY,OLD_COPY);}
#define MATRIX_DNS_SERVER_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                         UpdateMatrixDnsServerCfg(NEW_COPY, OLD_COPY);
#define USER_ACCOUNT_CONFIG_CHANGE(NEW_COPY, OLD_COPY, USER_INDEX)                  UserAccountCfgUpdate(NEW_COPY, OLD_COPY, USER_INDEX);
#define CAMERA_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX)                      {CiCameraConfigNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX);    \
                                                                                     RMConfigNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX);          \
                                                                                     CiStreamCamConfigChange(NEW_COPY, OLD_COPY, CAMERA_INDEX);}
#define STREAM_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX)                      {CiStreamConfigNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX);}
#define PTZ_PRESET_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX, PTZ_INDEX);      {CiPtzPositionConfigNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX, PTZ_INDEX);}
#define PRESET_TOUR_CONFIG_CHANGE(CAMERA_INDEX, NEW_COPY, OLD_COPY)                 {UpdateTourCfg(CAMERA_INDEX, NEW_COPY, OLD_COPY);}
#define TOUR_SCHEDULE_CONFIG_CHANGE(CAMERA_INDEX, NEW_COPY, OLD_COPY)               {UpdateTourScheduleCfg(CAMERA_INDEX, NEW_COPY, OLD_COPY);}
#define SENSOR_CONFIG_CHANGE(NEW_COPY, OLD_COPY, SENSOR_INDEX)                      {UpdateSensorConfig(NEW_COPY, OLD_COPY, SENSOR_INDEX);}
#define ALARM_CONFIG_CHANGE(NEW_COPY, OLD_COPY, ALARM_INDEX)                        {UpdateAlarmConfig(NEW_COPY, OLD_COPY, ALARM_INDEX);}
#define	DM_HDD_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                    DmCfgChange(NEW_COPY, OLD_COPY)
#define STORAGE_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                   DmStorageCfgUpdate(NEW_COPY, OLD_COPY)
#define SCHEDULE_BACKUP_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                           {ScheduleBackupCfgUpdate(NEW_COPY, OLD_COPY);}
#define MANUAL_BACKUP_CONFIG_CHANGE(NEW_COPY)                                       {ManualBackupCfgUpdate(NEW_COPY);}
#define CAMERA_ALARM_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX, ALARM_INDEX)   {NotifyAlarmConfigChange(NEW_COPY, OLD_COPY, CAMERA_INDEX, ALARM_INDEX);}
#define CAMERA_EVENT_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX, EVENT_INDEX)   {EvntHndlrCamEventCfgUpdate(NEW_COPY, OLD_COPY, CAMERA_INDEX, EVENT_INDEX);}
#define SENSOR_EVENT_CONFIG_CHANGE(NEW_COPY, OLD_COPY, SENSOR_INDEX)                {EvntHndlrSensorEventCfgUpdate(NEW_COPY, OLD_COPY, SENSOR_INDEX);}
#define SYSTEM_EVENT_CONFIG_CHANGE(NEW_COPY, OLD_COPY, SYSTEM_INDEX)                {EvntHndlrSystemEventCfgUpdate(NEW_COPY, OLD_COPY, SYSTEM_INDEX);}
#define	STATIC_ROUTE_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                              {UpdateStaticRouting(NEW_COPY, OLD_COPY);       \
                                                                                     P2pUpdateDefaultRoute(NEW_COPY, OLD_COPY);}
#define BROAD_BAND_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                {MobileBroadbandCfgUpdate(NEW_COPY, OLD_COPY);  \
                                                                                     P2pMobileBroadbandCfgUpdate();}
#define MANUAL_RECORD_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX)               {RmConfigChangeNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX);}
#define NETWORK_DRIVE_CONFIG_CHANGE(NEW_COPY, OLD_COPY, NW_DRIVE_INDEX)             {DmNddCfgChange(NEW_COPY, OLD_COPY, NW_DRIVE_INDEX);}
#define	IP_CAMERA_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX)                   {CiIpCameraConfigNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX);}
#define LOGIN_POLICY_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                              {LoginPolicyCfgUpdate(NEW_COPY, OLD_COPY);}
#define P2P_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                       {P2pConfigNotify(NEW_COPY, OLD_COPY);}
#define SMTP_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                                      {UpdateSmtpConfig(NEW_COPY, OLD_COPY);}
#define IMAGE_SETTING_CONFIG_CHANGE(NEW_COPY, OLD_COPY, CAMERA_INDEX)               {ImageSettingConfigNotify(NEW_COPY, OLD_COPY, CAMERA_INDEX);}
#define DHCP_SERVER_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                               {DhcpServerConfigNotify(NEW_COPY, OLD_COPY);}
#define FIRMWARE_MANAGEMENT_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                       {FirmwareManagementConfigNotify(NEW_COPY, OLD_COPY);}
#define FCM_PUSH_NOTIFY_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                           {FcmPushNotifyConfigUpdate(NEW_COPY, OLD_COPY);}
#define STORAGE_ALLOCATION_CONFIG_CHANGE(NEW_COPY, OLD_COPY)                        {StorageAllocationConfigNotify(NEW_COPY, OLD_COPY);}

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	WHOLE_CONFIG,
	SINGLE_ENTRY,
	MAX_CONFIG_REQ_e

}CONFIG_REQ_e;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
/** ******************************************************************************************* **/
/**                                     General APIs                                            **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
void InitSysConfig(void);
//-------------------------------------------------------------------------------------------------
void ResetButtonAction(void);
//-------------------------------------------------------------------------------------------------
BOOL RemoveConfigFile(CONFIG_INDEX_e cfgIndex);
//-------------------------------------------------------------------------------------------------
BOOL RemoveLocalConfigFile(LOCAL_CONFIG_INDEX_e cfgIndex);
//-------------------------------------------------------------------------------------------------

/** ******************************************************************************************* **/
/**                                  Config Default APIs                                        **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltGeneralConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltDateTimeConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltDstConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltLan1Config(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltLan2Config(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltMacServerConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltIpFilterConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltDdnsConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSmtpConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleFtpUploadConfig(UINT8 ftpIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltFtpUploadConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltTcpNotifyConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltFileAccessConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltHddConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltMatrixDnsServerConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltUserAccountConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltCameraConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltStreamConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltScheduleRecordConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltAlarmRecordConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltPtzPresetConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltPresetTourConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltTourScheduleConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltImageUploadConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSensorConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltAlarmConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltStorageConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltScheduleBackupConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltManualBackupConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltCameraAlarmConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltCameraEventConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSensorEventConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSystemEventConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltStaticRoutingConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltBroadBandConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSmsConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltNetworkDriveConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSnapshotConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSnapShotScheduleConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltAdaptiveRecordingConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltLoginPolicyConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltAudioOutConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltP2PConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltImageSettingConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltDhcpServerConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltFirmwareManagementConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltFcmPushNotificationConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltPasswordRecoveryConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltStorageAllocationConfig(void);
//-------------------------------------------------------------------------------------------------

/** ******************************************************************************************* **/
/**          Config Default Single Entry APIs (for multi dimentional config)                    **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleUserAccountConfig(UINT8 userIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleCameraConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleStreamConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleScheduleRecordConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleAlarmRecordConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSinglePtzPresetConfig(UINT8 cameraIndex, UINT8 ptzIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSinglePresetTourConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleTourScheduleConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleImageUploadConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleSensorConfig(UINT8 sensorIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleAlarmConfig(UINT8 alarmIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleCameraAlarmConfig(UINT8 cameraIndex, UINT8 alarmIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleCameraEventConfig(UINT8 cameraIndex, UINT8 eventIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleSensorEventConfig(UINT8 sensorIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleSystemEventConfig(UINT8 eventIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleNetworkDriveConfig(UINT8 nwDriveIdx);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleCosecPreRecConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleManualRecordConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleIpCameraConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleNetwokDeviceConfig(UINT8 deviceIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleSnapshotConfig(UINT8 deviceIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleSnapShotScheduleConfig(UINT8 deviceIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleAdaptiveRecConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSingleImageSettingConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSinglePushNotificationConfig(UINT8 deviceIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DfltSinglePasswordRecoveryConfig(UINT8 userIndex);
//-------------------------------------------------------------------------------------------------

/** ******************************************************************************************* **/
/**                                   Config Write APIs                                         **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteGeneralConfig(GENERAL_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteDateTimeConfig(DATE_TIME_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteDstConfig(DST_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteLan1Config(LAN_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteLan2Config(LAN_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteMacServerConfig(MAC_SERVER_CNFG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteIpFilterConfig(IP_FILTER_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteDdnsConfig(DDNS_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSmtpConfig(SMTP_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleFtpUploadConfig(UINT8 ftpIndex, FTP_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteFtpUploadConfig(FTP_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteTcpNotifyConfig(TCP_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteFcmPushNotifyConfig(FCM_PUSH_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteFileAccessConfig(FILE_ACCESS_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteHddConfig(HDD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteMatrixDnsServerConfig(MATRIX_DNS_SERVER_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteUserAccountConfig(USER_ACCOUNT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteCameraConfig(CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteStreamConfig(STREAM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteScheduleRecordConfig(SCHEDULE_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteAlarmRecordConfig(ALARM_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WritePtzPresetConfig(PTZ_PRESET_CONFIG_t **userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WritePresetTourConfig(PRESET_TOUR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteTourScheduleConfig(TOUR_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteImageUploadConfig(IMAGE_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSensorConfig(SENSOR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteAlarmConfig(ALARM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteStorageConfig(STORAGE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteScheduleBackupConfig(SCHEDULE_BACKUP_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteManualBackupConfig(MANUAL_BACKUP_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteCameraAlarmConfig(CAMERA_ALARM_CONFIG_t **userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteCameraEventConfig(CAMERA_EVENT_CONFIG_t **userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSensorEventConfig(SENSOR_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSystemEventConfig(SYSTEM_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteStaticRoutingConfig(STATIC_ROUTING_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteBraodBandConfig(BROAD_BAND_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSmsConfig(SMS_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteNetworkDriveConfig(NETWORK_DRIVE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSnapshotConfig(SNAPSHOT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSnapshotScheduleConfig(SNAPSHOT_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteLoginPolicyConfig(LOGIN_POLICY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteAudioOutConfig(AUDIO_OUT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteP2PConfig(P2P_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteImageSettingConfig(IMG_SETTING_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteDhcpServerConfig(DHCP_SERVER_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteFirmwareManagementConfig(FIRMWARE_MANAGEMENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteFcmPushNotifyConfig(FCM_PUSH_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WritePasswordRecoveryConfig(PASSWORD_RECOVERY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteStorageAllocationConfig(STORAGE_ALLOCATION_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------

/** ******************************************************************************************* **/
/**            Config Write Single Entry APIs (for multi dimentional config)                    **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleUserAccountConfig(UINT8 userIndex, USER_ACCOUNT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleCameraConfig(UINT8 cameraIndex, CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleStreamConfig(UINT8 cameraIndex, STREAM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleScheduleRecordConfig(UINT8 cameraIndex, SCHEDULE_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleAlarmRecordConfig(UINT8 cameraIndex, ALARM_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSinglePtzPresetConfig(UINT8 cameraIndex, UINT8 ptzIndex, PTZ_PRESET_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSinglePresetTourConfig(UINT8 cameraIndex, PRESET_TOUR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleTourScheduleConfig(UINT8 cameraIndex, TOUR_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleImageUploadConfig(UINT8 cameraIndex, IMAGE_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleSensorConfig(UINT8 sensorIndex, SENSOR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleAlarmConfig(UINT8 alarmIndex, ALARM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleCameraAlarmConfig(UINT8 cameraIndex, UINT8 alarmIndex, CAMERA_ALARM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleCameraEventConfig(UINT8 cameraIndex, UINT8 eventIndex, CAMERA_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleSensorEventConfig(UINT8 sensorIndex, SENSOR_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleSystemEventConfig(UINT8 eventIndex, SYSTEM_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleNetworkDriveConfig(UINT8 nwDriveIdx, NETWORK_DRIVE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleIpCameraConfig(UINT8 cameraIndex, IP_CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleCosecPreRecConfig(UINT8 cameraIndex, COSEC_REC_PARAM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleManualRecordConfig(UINT8 cameraIndex, MANUAL_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleNetworkDeviceConfig(UINT8 deviceIndex, NETWORK_DEVICE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleSnapshotConfig(UINT8 cameraIndex, SNAPSHOT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleSnapshotScheduleConfig(UINT8 cameraIndex, SNAPSHOT_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleImageSettingConfig(UINT8 cameraIndex, IMG_SETTING_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSingleFcmPushNotificationConfig(UINT8 deviceIndex, FCM_PUSH_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteSinglePasswordRecoveryConfig(UINT8 userIndex, PASSWORD_RECOVERY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------

/** ******************************************************************************************* **/
/**                                   Config Read APIs                                          **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
BOOL ReadGeneralConfig(GENERAL_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadDateTimeConfig(DATE_TIME_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadDstConfig(DST_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadLan1Config(LAN_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadLan2Config(LAN_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadMacServerConfig(MAC_SERVER_CNFG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadIpFilterConfig(IP_FILTER_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadDdnsConfig(DDNS_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSmtpConfig(SMTP_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleFtpUploadConfig(UINT8 ftpIndex, FTP_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadFtpUploadConfig(FTP_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadTcpNotifyConfig(TCP_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadFileAccessConfig(FILE_ACCESS_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadHddConfig(HDD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadMatrixDnsServerConfig(MATRIX_DNS_SERVER_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadUserAccountConfig(USER_ACCOUNT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadCameraConfig(CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadStreamConfig(STREAM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadScheduleRecordConfig(SCHEDULE_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadAlarmRecordConfig(ALARM_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadPtzPresetConfig(PTZ_PRESET_CONFIG_t **userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadPresetTourConfig(PRESET_TOUR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadTourScheduleConfig(TOUR_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadImageUploadConfig(IMAGE_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSensorConfig(SENSOR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadAlarmConfig(ALARM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadStorageConfig(STORAGE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadScheduleBackupConfig(SCHEDULE_BACKUP_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadManualBackupConfig(MANUAL_BACKUP_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadCameraAlarmConfig(CAMERA_ALARM_CONFIG_t **userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadCameraEventConfig(CAMERA_EVENT_CONFIG_t **userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSensorEventConfig(SENSOR_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSystemEventConfig(SYSTEM_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadStaticRoutingConfig(STATIC_ROUTING_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadBroadBandConfig(BROAD_BAND_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSmsConfig(SMS_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadNetworkDriveConfig(NETWORK_DRIVE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadIpCameraConfig(IP_CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSnapshotConfig(SNAPSHOT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSnapshotScheduleConfig(SNAPSHOT_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadLoginPolicyConfig(LOGIN_POLICY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadAudioOutConfig(AUDIO_OUT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadP2PConfig(P2P_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadImageSettingConfig(IMG_SETTING_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadDhcpServerConfig(DHCP_SERVER_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadFirmwareManagementConfig(FIRMWARE_MANAGEMENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadFcmPushNotifyConfig(FCM_PUSH_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadPasswordRecoveryConfig(PASSWORD_RECOVERY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadStorageAllocationConfig(STORAGE_ALLOCATION_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------

/** ******************************************************************************************* **/
/**             Config Read Single Entry APIs (for multi dimentional config)                    **/
/** ******************************************************************************************* **/
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleCameraConfig(UINT8 cameraIndex, CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleUserAccountConfig(UINT8 userIndex, USER_ACCOUNT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleStreamConfig(UINT8 cameraIndex, STREAM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleScheduleRecordConfig(UINT8 cameraIndex, SCHEDULE_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleAlarmRecordConfig(UINT8 cameraIndex, ALARM_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSinglePtzPresetConfig(UINT8 cameraIndex, UINT8 ptzIndex, PTZ_PRESET_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSinglePresetTourConfig(UINT8 cameraIndex, PRESET_TOUR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleTourScheduleConfig(UINT8 cameraIndex, TOUR_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleImageUploadConfig(UINT8 cameraIndex, IMAGE_UPLOAD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleSensorConfig(UINT8 sensorIndex, SENSOR_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleAlarmConfig(UINT8 alarmIndex, ALARM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleCameraAlarmConfig(UINT8 cameraIndex, UINT8 alarmIndex, CAMERA_ALARM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleCameraEventConfig(UINT8 cameraIndex, UINT8 eventIndex, CAMERA_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleSensorEventConfig(UINT8 sensorIndex, SENSOR_EVENT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleSystemEventConfig(UINT8 eventIndex, SYSTEM_EVENT_CONFIG_t *userCopy)	;
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleNetworkDriveConfig(UINT8 nwDriveIdx, NETWORK_DRIVE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleCosecPreRecConfig(UINT8 cameraIndex, COSEC_REC_PARAM_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleManualRecordConfig(UINT8 cameraIndex, MANUAL_RECORD_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleIpCameraConfig(UINT8 cameraIndex, IP_CAMERA_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleNetworkDeviceConfig(UINT8 deviceIndex, NETWORK_DEVICE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleSnapshotConfig(UINT8 deviceIndex, SNAPSHOT_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleSnapshotScheduleConfig(UINT8 deviceIndex, SNAPSHOT_SCHEDULE_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadLan1ConfigCms(LAN_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadLan2ConfigCms(LAN_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleImageSettingConfig(UINT8 cameraIndex, IMG_SETTING_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSingleFcmPushNotificationConfig(UINT8 deviceIndex, FCM_PUSH_NOTIFY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
BOOL ReadSinglePasswordRecoveryConfig(UINT8 userIndex, PASSWORD_RECOVERY_CONFIG_t *userCopy);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif  // CONFIG_API_H
