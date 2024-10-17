//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   ConfigOldFileIO.c
@brief  This module convert configuration from older version to latest version. It reads older
        version configuration from file, converts into newer version and write back to same file.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "Config.h"
#include "ConfigOld.h"
#include "UserSession.h"
#include "CameraDatabase.h"
#include "CameraInitiation.h"

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL loadOldGeneralConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldDateTimeConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldLan1Config(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldLan2Config(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldStaticRoutingConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldIpFilterConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldDdnsConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSmtpConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldFtpConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldTcpConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldFileAccessConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldDnsConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldUserAccountConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldCameraConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldPresetPositionConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldPresetTourConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSystemSensorConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSystemAlarmConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldStorageConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldScheduleBackupConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldManualBackupConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldUploadImageConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldCameraEventAndActionConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSystemEventAndActionConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldBroadBandConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSMSConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldNetworkDriveConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldIpCameraConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSensorEventAndActionConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldNetworkDevicesConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldSnapshotConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldScheduleRecordConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldLoginPolicyConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldFirmwareManagementConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL loadOldP2pConfig(INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
static BOOL writeConfigFile(INT32 fileFd, UINT32 fileVersion, VOIDPTR cfgData, ssize_t cfgSize);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldGeneralConfig(INT32 fileFd, UINT32 fileVersion)
{
    GENERAL_CONFIG_VER12_t generalCfg12;
    GENERAL_CONFIG_VER13_t generalCfg13;
    GENERAL_CONFIG_VER14_t generalCfg14;
    GENERAL_CONFIG_VER15_t generalCfg15;

    switch(fileVersion)
    {
        case 12:
        {
            if((sizeof(GENERAL_CONFIG_VER12_t)) != read(fileFd, &generalCfg12, sizeof(GENERAL_CONFIG_VER12_t)))
            {
                EPRINT(CONFIGURATION, "fail to read general config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        case 13:
        {
            if((sizeof(GENERAL_CONFIG_VER13_t)) != read(fileFd, &generalCfg13, sizeof(GENERAL_CONFIG_VER13_t)))
            {
                EPRINT(CONFIGURATION, "fail to read general config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        case 14:
        {
            if((sizeof(GENERAL_CONFIG_VER14_t)) != read(fileFd, &generalCfg14, sizeof(GENERAL_CONFIG_VER14_t)))
            {
                EPRINT(CONFIGURATION, "fail to read general config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        case 15:
        {
            if((sizeof(GENERAL_CONFIG_VER15_t)) != read(fileFd, &generalCfg15, sizeof(GENERAL_CONFIG_VER15_t)))
            {
                EPRINT(CONFIGURATION, "fail to read general config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        default:
        {
            EPRINT(CONFIGURATION, "invld general config file version: [version=%d]", fileVersion);
        }
        return FAIL;
    }

    if(fileVersion == 12)
    {
        memcpy(&generalCfg13, &generalCfg12, sizeof(GENERAL_CONFIG_VER12_t));
        generalCfg13.autoAddCamFlag = DfltGeneralCfg.autoAddCamFlag;
        generalCfg13.autoAddCamTcpPort = DfltGeneralCfg.autoAddCamTcpPort;
        generalCfg13.pollDuration = DfltGeneralCfg.pollDuration;
        generalCfg13.pollInterval = DfltGeneralCfg.pollInterval;
        fileVersion++;
    }

    if(fileVersion == 13)
    {
        memcpy(&generalCfg14, &generalCfg13, sizeof(GENERAL_CONFIG_VER13_t));
        generalCfg14.netAcceleration = DfltGeneralCfg.netAcceleration;
        fileVersion++;
    }

    if(fileVersion == 14)
    {
        memcpy(&generalCfg15, &generalCfg14, sizeof(GENERAL_CONFIG_VER14_t));
        snprintf(generalCfg15.deviceName, sizeof(generalCfg15.deviceName), "%s", generalCfg15.deviceNameV1);
        snprintf(generalCfg15.userName, sizeof(generalCfg15.userName), "%s", generalCfg15.userNameV1);
        snprintf(generalCfg15.password, sizeof(generalCfg15.password), "%s", generalCfg15.passwordV1);
        fileVersion++;
    }

    if (fileVersion == 15)
    {
        snprintf(generalCfg.deviceName, sizeof(generalCfg.deviceName), "%s", generalCfg15.deviceName);
        generalCfg.fileRecordDuration = generalCfg15.fileRecordDuration;
        generalCfg.autoLogoutDuration = generalCfg15.autoLogoutDuration;
        generalCfg.autoPowerOn = generalCfg15.autoPowerOn ;
        generalCfg.httpPort = generalCfg15.httpPort;
        generalCfg.tcpPort = generalCfg15.tcpPort;
        generalCfg.rtpPort = generalCfg15.rtpPort;
        generalCfg.onvifPort = generalCfg15.onvifPort;
        generalCfg.deviceId = generalCfg15.deviceId;
        generalCfg.videoSystemType = generalCfg15.videoSystemType;
        generalCfg.integrateCosec = generalCfg15.integrateCosec;
        generalCfg.dateFormat = generalCfg15.dateFormat;
        generalCfg.timeFormat = generalCfg15.timeFormat;
        generalCfg.recordFormatType = generalCfg15.recordFormatType;
        generalCfg.analogCamNo = generalCfg15.analogCamNo;
        generalCfg.ipCamNo = generalCfg15.ipCamNo;
        generalCfg.autoConfigureCameraFlag = generalCfg15.autoConfigureCameraFlag;
        generalCfg.retainIpAddresses = generalCfg15.retainIpAddresses;
        snprintf(generalCfg.autoConfigStartIp, sizeof(generalCfg.autoConfigStartIp), "%s", generalCfg15.autoConfigStartIp);
        snprintf(generalCfg.autoConfigEndIp, sizeof(generalCfg.autoConfigEndIp), "%s", generalCfg15.autoConfigEndIp);
        generalCfg.retainDfltProfile = generalCfg15.retainDfltProfile;
        snprintf(generalCfg.videoEncoding, sizeof(generalCfg.videoEncoding), "%s", generalCfg15.videoEncoding);
        snprintf(generalCfg.resolution, sizeof(generalCfg.resolution), "%s", generalCfg15.resolution);
        generalCfg.framerate = generalCfg15.framerate;
        generalCfg.quality = generalCfg15.quality;
        generalCfg.enableAudio = generalCfg15.enableAudio;
        generalCfg.bitrateMode = generalCfg15.bitrateMode;
        generalCfg.bitrateValue = generalCfg15.bitrateValue;
        generalCfg.gop = generalCfg15.gop;
        snprintf(generalCfg.videoEncodingSub, sizeof(generalCfg.videoEncodingSub), "%s", generalCfg15.videoEncodingSub);
        snprintf(generalCfg.resolutionSub, sizeof(generalCfg.resolutionSub), "%s", generalCfg15.resolutionSub);
        generalCfg.framerateSub = generalCfg15.framerateSub;
        generalCfg.qualitySub = generalCfg15.qualitySub;
        generalCfg.enableAudioSub = generalCfg15.enableAudioSub;
        generalCfg.bitrateModeSub = generalCfg15.bitrateModeSub;
        generalCfg.bitrateValueSub = generalCfg15.bitrateValueSub;
        generalCfg.gopSub = generalCfg15.gopSub;
        generalCfg.devInitWithServerF = generalCfg15.devInitWithServerF;
        snprintf(generalCfg.devInitServerIpAddr, sizeof(generalCfg.devInitServerIpAddr), "%s", generalCfg15.devInitServerIpAddr);
        generalCfg.devInitServerPort = generalCfg15.devInitServerPort;
        generalCfg.autoRecordFailFlag = generalCfg15.autoRecordFailFlag;
        generalCfg.videoPopupDuration = generalCfg15.videoPopupDuration;
        generalCfg.preVideoLossDuration = generalCfg15.preVideoLossDuration;
        snprintf(generalCfg.userName, sizeof(generalCfg.userName), "%s", generalCfg15.userName);
        snprintf(generalCfg.password, sizeof(generalCfg.password), "%s", generalCfg15.password);
        generalCfg.startLiveView = generalCfg15.startLiveView;
        generalCfg.forwardedTcpPort = generalCfg15.forwardedTcpPort;
        generalCfg.autoAddCamFlag = generalCfg15.autoAddCamFlag;
        generalCfg.autoAddCamTcpPort = generalCfg15.autoAddCamTcpPort;
        generalCfg.pollDuration = generalCfg15.pollDuration;
        generalCfg.pollInterval = generalCfg15.pollInterval;
        generalCfg.netAcceleration = generalCfg15.netAcceleration;
        fileVersion++;
    }

    fileVersion = GENERAL_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &generalCfg, sizeof(GENERAL_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write general config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldDateTimeConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 2:
        {
            if((sizeof(DATE_TIME_CONFIG_VER2_t)) != read(fileFd, &dateTimeCfg, sizeof(DATE_TIME_CONFIG_VER2_t)))
            {
                return FAIL;
            }
        }
        break;

        case 3:
        {
            if((sizeof(DATE_TIME_CONFIG_VER3_t)) != read(fileFd, &dateTimeCfg, sizeof(DATE_TIME_CONFIG_VER3_t)))
            {
                return FAIL;
            }
        }
        break;

        case 4:
        {
            if((sizeof(DATE_TIME_CONFIG_VER4_t)) != read(fileFd, &dateTimeCfg, sizeof(DATE_TIME_CONFIG_VER4_t)))
            {
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    UINT32 oldFileVersion = fileVersion;
    if(fileVersion == 2)
    {
        // copy old NTP parameter into new (UTF-8) structure
        dateTimeCfg.ntp.server = dateTimeCfg.ntpV1.server;
        dateTimeCfg.ntp.updateInterval = dateTimeCfg.ntpV1.updateInterval;
        snprintf(dateTimeCfg.ntp.userDefinedServer, sizeof(dateTimeCfg.ntp.userDefinedServer), "%s", dateTimeCfg.ntpV1.userDefinedServer);
        fileVersion = 3;
    }

    if(fileVersion == 3)
    {
        // Auto Sync Time zone flag added. Set the default value.
        dateTimeCfg.syncTimeZoneToOnvifCam = DfltDateTimeCfg.syncTimeZoneToOnvifCam;
        fileVersion = 4;
    }

    if(fileVersion == 4)
    {
        //As per SRS
        if(oldFileVersion < 3)
        {
            dateTimeCfg.setUtcTime = DISABLE;
        }
        else
        {
            dateTimeCfg.setUtcTime = DfltDateTimeCfg.setUtcTime;
        }
        fileVersion = 5;
    }

    fileVersion = DATE_TIME_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &dateTimeCfg, sizeof(DATE_TIME_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write date-time config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldLan1Config(INT32 fileFd, UINT32 fileVersion)
{
    LAN_CONFIG_VER2_t lanCfg2;
    LAN_CONFIG_VER3_t lanCfg3;
    LAN_CONFIG_VER4_t lanCfg4;

    switch(fileVersion)
    {
        case 2:
        {
            if ((sizeof(LAN_CONFIG_VER2_t)) != read(fileFd, &lanCfg2, sizeof(LAN_CONFIG_VER2_t)))
            {
                return FAIL;
            }
        }
        break;

        case 3:
        {
            if ((sizeof(LAN_CONFIG_VER3_t)) != read(fileFd, &lanCfg3, sizeof(LAN_CONFIG_VER3_t)))
            {
                return FAIL;
            }
        }
        break;

        case 4:
        {
            if ((sizeof(LAN_CONFIG_VER4_t)) != read(fileFd, &lanCfg4, sizeof(LAN_CONFIG_VER4_t)))
            {
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 2)
    {
        lanCfg3.ipAssignMode = lanCfg2.ipAssignMode;
        snprintf(lanCfg3.lan.ipAddress, sizeof(lanCfg3.lan.ipAddress), "%s", lanCfg2.lan.ipAddress);
        snprintf(lanCfg3.lan.subnetMask, sizeof(lanCfg3.lan.subnetMask), "%s", lanCfg2.lan.subnetMask);
        snprintf(lanCfg3.lan.gateway, sizeof(lanCfg3.lan.gateway), "%s", lanCfg2.lan.gateway);
        snprintf(lanCfg3.pppoeV2.username, sizeof(lanCfg3.pppoeV2.username), "%s", lanCfg2.pppoe.username);
        snprintf(lanCfg3.pppoeV2.password, sizeof(lanCfg3.pppoeV2.password), "%s", lanCfg2.pppoe.password);
        lanCfg3.dns.mode = lanCfg2.dns.mode;
        snprintf(lanCfg3.dns.primaryAddress, sizeof(lanCfg2.dns.primaryAddress), "%s", lanCfg2.dns.primaryAddress);
        snprintf(lanCfg3.dns.secondaryAddress, sizeof(lanCfg2.dns.secondaryAddress), "%s", lanCfg2.dns.secondaryAddress);
        fileVersion++;
    }

    if (fileVersion == 3)
    {
        lanCfg4.ipAssignMode = lanCfg3.ipAssignMode;
        snprintf(lanCfg4.lan.ipAddress, sizeof(lanCfg4.lan.ipAddress), "%s", lanCfg3.lan.ipAddress);
        snprintf(lanCfg4.lan.subnetMask, sizeof(lanCfg4.lan.subnetMask), "%s", lanCfg3.lan.subnetMask);
        snprintf(lanCfg4.lan.gateway, sizeof(lanCfg4.lan.gateway), "%s", lanCfg3.lan.gateway);
        snprintf(lanCfg4.pppoe.username, sizeof(lanCfg4.pppoe.username), "%s", lanCfg3.pppoeV2.username);
        snprintf(lanCfg4.pppoe.password, sizeof(lanCfg4.pppoe.password), "%s", lanCfg3.pppoeV2.password);
        lanCfg4.dns.mode = lanCfg3.dns.mode;
        snprintf(lanCfg4.dns.primaryAddress, sizeof(lanCfg4.dns.primaryAddress), "%s", lanCfg3.dns.primaryAddress);
        snprintf(lanCfg4.dns.secondaryAddress, sizeof(lanCfg4.dns.secondaryAddress), "%s", lanCfg3.dns.secondaryAddress);
        fileVersion++;
    }

    if(fileVersion == 4)
    {
        lanCfg[LAN1_PORT].ipv4.ipAssignMode = lanCfg4.ipAssignMode;
        snprintf(lanCfg[LAN1_PORT].ipv4.lan.ipAddress, sizeof(lanCfg[LAN1_PORT].ipv4.lan.ipAddress), "%s", lanCfg4.lan.ipAddress);
        snprintf(lanCfg[LAN1_PORT].ipv4.lan.subnetMask, sizeof(lanCfg[LAN1_PORT].ipv4.lan.subnetMask), "%s", lanCfg4.lan.subnetMask);
        snprintf(lanCfg[LAN1_PORT].ipv4.lan.gateway, sizeof(lanCfg[LAN1_PORT].ipv4.lan.gateway), "%s", lanCfg4.lan.gateway);
        lanCfg[LAN1_PORT].ipv4.dns.mode = lanCfg4.dns.mode;
        snprintf(lanCfg[LAN1_PORT].ipv4.dns.primaryAddress, sizeof(lanCfg[LAN1_PORT].ipv4.dns.primaryAddress), "%s", lanCfg4.dns.primaryAddress);
        snprintf(lanCfg[LAN1_PORT].ipv4.dns.secondaryAddress, sizeof(lanCfg[LAN1_PORT].ipv4.dns.secondaryAddress), "%s", lanCfg4.dns.secondaryAddress);
        snprintf(lanCfg[LAN1_PORT].ipv4.pppoe.username, sizeof(lanCfg[LAN1_PORT].ipv4.pppoe.username), "%s", lanCfg4.pppoe.username);
        snprintf(lanCfg[LAN1_PORT].ipv4.pppoe.password, sizeof(lanCfg[LAN1_PORT].ipv4.pppoe.password), "%s", lanCfg4.pppoe.password);
        lanCfg[LAN1_PORT].ipAddrMode = DfltLanCfg[LAN1_PORT].ipAddrMode;
        memcpy(&lanCfg[LAN1_PORT].ipv6, &DfltLanCfg[LAN1_PORT].ipv6, sizeof(IPV6_LAN_CONFIG_t));
        fileVersion++;
    }

    fileVersion = LAN_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &lanCfg[LAN1_PORT], sizeof(LAN_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write lan1 config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldLan2Config(INT32 fileFd, UINT32 fileVersion)
{
    LAN_CONFIG_VER2_t lanCfg2;
    LAN_CONFIG_VER3_t lanCfg3;
    LAN_CONFIG_VER4_t lanCfg4;

    switch(fileVersion)
    {
        case 2:
        {
            if ((sizeof(LAN_CONFIG_VER2_t)) != read(fileFd, &lanCfg2, sizeof(LAN_CONFIG_VER2_t)))
            {
                return FAIL;
            }
        }
        break;

        case 3:
        {
            if ((sizeof(LAN_CONFIG_VER3_t)) != read(fileFd, &lanCfg3, sizeof(LAN_CONFIG_VER3_t)))
            {
                return FAIL;
            }
        }
        break;

        case 4:
        {
            if ((sizeof(LAN_CONFIG_VER4_t)) != read(fileFd, &lanCfg4, sizeof(LAN_CONFIG_VER4_t)))
            {
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 2)
    {
        lanCfg3.ipAssignMode = DfltLanCfg[LAN2_PORT].ipv4.ipAssignMode;
        snprintf(lanCfg3.lan.ipAddress, sizeof(lanCfg3.lan.ipAddress), "%s", lanCfg2.lan.ipAddress);
        snprintf(lanCfg3.lan.subnetMask, sizeof(lanCfg3.lan.subnetMask), "%s", lanCfg2.lan.subnetMask);
        snprintf(lanCfg3.lan.gateway, sizeof(lanCfg3.lan.gateway), "%s", lanCfg2.lan.gateway);
        snprintf(lanCfg3.pppoeV2.username, sizeof(lanCfg3.pppoeV2.username), "%s", DfltLanCfg[LAN2_PORT].ipv4.pppoe.username);
        snprintf(lanCfg3.pppoeV2.password, sizeof(lanCfg3.pppoeV2.password), "%s", DfltLanCfg[LAN2_PORT].ipv4.pppoe.password);
        lanCfg3.dns.mode = DfltLanCfg[LAN2_PORT].ipv4.dns.mode;
        snprintf(lanCfg3.dns.primaryAddress, sizeof(lanCfg2.dns.primaryAddress), "%s", DfltLanCfg[LAN2_PORT].ipv4.dns.primaryAddress);
        snprintf(lanCfg3.dns.secondaryAddress, sizeof(lanCfg2.dns.secondaryAddress), "%s", DfltLanCfg[LAN2_PORT].ipv4.dns.secondaryAddress);
        fileVersion++;
    }

    if (fileVersion == 3)
    {
        lanCfg4.ipAssignMode = DfltLanCfg[LAN2_PORT].ipv4.ipAssignMode;
        snprintf(lanCfg4.lan.ipAddress, sizeof(lanCfg4.lan.ipAddress), "%s", lanCfg3.lan.ipAddress);
        snprintf(lanCfg4.lan.subnetMask, sizeof(lanCfg4.lan.subnetMask), "%s", lanCfg3.lan.subnetMask);
        snprintf(lanCfg4.lan.gateway, sizeof(lanCfg4.lan.gateway), "%s", lanCfg3.lan.gateway);
        snprintf(lanCfg4.pppoe.username, sizeof(lanCfg4.pppoe.username), "%s", DfltLanCfg[LAN2_PORT].ipv4.pppoe.username);
        snprintf(lanCfg4.pppoe.password, sizeof(lanCfg4.pppoe.password), "%s", DfltLanCfg[LAN2_PORT].ipv4.pppoe.password);
        lanCfg4.dns.mode = DfltLanCfg[LAN2_PORT].ipv4.dns.mode;
        snprintf(lanCfg4.dns.primaryAddress, sizeof(lanCfg4.dns.primaryAddress), "%s", DfltLanCfg[LAN2_PORT].ipv4.dns.primaryAddress);
        snprintf(lanCfg4.dns.secondaryAddress, sizeof(lanCfg4.dns.secondaryAddress), "%s", DfltLanCfg[LAN2_PORT].ipv4.dns.secondaryAddress);
        fileVersion++;
    }

    if (fileVersion == 4)
    {
        lanCfg[LAN2_PORT].ipv4.ipAssignMode = DfltLanCfg[LAN2_PORT].ipv4.ipAssignMode;
        snprintf(lanCfg[LAN2_PORT].ipv4.lan.ipAddress, sizeof(lanCfg[LAN2_PORT].ipv4.lan.ipAddress), "%s", lanCfg4.lan.ipAddress);
        snprintf(lanCfg[LAN2_PORT].ipv4.lan.subnetMask, sizeof(lanCfg[LAN2_PORT].ipv4.lan.subnetMask), "%s", lanCfg4.lan.subnetMask);
        snprintf(lanCfg[LAN2_PORT].ipv4.lan.gateway, sizeof(lanCfg[LAN2_PORT].ipv4.lan.gateway), "%s", lanCfg4.lan.gateway);
        lanCfg[LAN2_PORT].ipv4.dns.mode = lanCfg4.dns.mode;
        snprintf(lanCfg[LAN2_PORT].ipv4.dns.primaryAddress, sizeof(lanCfg[LAN2_PORT].ipv4.dns.primaryAddress), "%s",
                 DfltLanCfg[LAN2_PORT].ipv4.dns.primaryAddress);
        snprintf(lanCfg[LAN2_PORT].ipv4.dns.secondaryAddress, sizeof(lanCfg[LAN2_PORT].ipv4.dns.secondaryAddress), "%s",
                 DfltLanCfg[LAN2_PORT].ipv4.dns.secondaryAddress);
        snprintf(lanCfg[LAN2_PORT].ipv4.pppoe.username, sizeof(lanCfg[LAN2_PORT].ipv4.pppoe.username), "%s", DfltLanCfg[LAN2_PORT].ipv4.pppoe.username);
        snprintf(lanCfg[LAN2_PORT].ipv4.pppoe.password, sizeof(lanCfg[LAN2_PORT].ipv4.pppoe.password), "%s", DfltLanCfg[LAN2_PORT].ipv4.pppoe.password);
        lanCfg[LAN2_PORT].ipAddrMode = DfltLanCfg[LAN2_PORT].ipAddrMode;
        memcpy(&lanCfg[LAN2_PORT].ipv6, &DfltLanCfg[LAN2_PORT].ipv6, sizeof(DfltLanCfg[LAN2_PORT].ipv6));
        fileVersion++;
    }

    fileVersion = LAN_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &lanCfg[LAN2_PORT], sizeof(LAN_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write lan2 config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldStaticRoutingConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                           cnt;
    STATIC_ROUTING_CONFIG_VER3_t    staticRoutingCfg3;

    switch(fileVersion)
    {
        case 3:
        {
            if ((sizeof(STATIC_ROUTING_CONFIG_VER3_t)) != read(fileFd, &staticRoutingCfg3, sizeof(STATIC_ROUTING_CONFIG_VER3_t)))
            {
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 3)
    {
        staticRoutingCfg.defaultPort = staticRoutingCfg3.defaultPort;
        for(cnt = 0; cnt < MAX_STATIC_ROUTING_ENTRY; cnt++)
        {
            snprintf(staticRoutingCfg.entry[cnt].networkAddr, sizeof(staticRoutingCfg.entry[cnt].networkAddr), "%s", staticRoutingCfg3.entry[cnt].networkAddr);
            staticRoutingCfg.entry[cnt].subnetMask = staticRoutingCfg3.entry[cnt].subnetMask;
            staticRoutingCfg.entry[cnt].routePort = staticRoutingCfg3.entry[cnt].routePort;
        }

        fileVersion++;
    }

    fileVersion = STATIC_ROUTING_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &staticRoutingCfg, sizeof(STATIC_ROUTING_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write static routing config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldIpFilterConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;
    IP_FILTER_CONFIG_VER1_t ipFilterCfg1;

    switch(fileVersion)
    {
        case 1:
        {
            if((sizeof(IP_FILTER_CONFIG_VER1_t)) != read(fileFd, &ipFilterCfg1, sizeof(IP_FILTER_CONFIG_VER1_t)))
            {
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 1)
    {
        ipFilterCfg.ipFilter = ipFilterCfg1.ipFilter;
        ipFilterCfg.mode = ipFilterCfg1.mode;

        for(cnt = 0; cnt < MAX_IP_FILTER; cnt++)
        {
            snprintf(ipFilterCfg.filter[cnt].startAddress, sizeof(ipFilterCfg.filter[cnt].startAddress), "%s", ipFilterCfg1.filter[cnt].startAddress);
            snprintf(ipFilterCfg.filter[cnt].endAddress, sizeof(ipFilterCfg.filter[cnt].endAddress), "%s", ipFilterCfg1.filter[cnt].endAddress);
        }

        fileVersion++;
    }

    fileVersion = IP_FILTER_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &ipFilterCfg, sizeof(IP_FILTER_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write ipfilter config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldDdnsConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            if((sizeof(DDNS_CONFIG_VER1_t)) != read(fileFd, &ddnsCfg, sizeof(DDNS_CONFIG_VER1_t)))
            {
                return FAIL;
            }

            // copy old DDNS parameter into new (UTf-8) variables
            snprintf(ddnsCfg.username, sizeof(ddnsCfg.username), "%s", ddnsCfg.usernameV1);
            snprintf(ddnsCfg.password, sizeof(ddnsCfg.password), "%s", ddnsCfg.passwordV1);
            snprintf(ddnsCfg.hostname, sizeof(ddnsCfg.hostname), "%s", ddnsCfg.hostnameV1);
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = DDNS_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &ddnsCfg, sizeof(DDNS_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write ddns config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSmtpConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            SMTP_CONFIG_VER1_t smtpCfg1;

            if((sizeof(SMTP_CONFIG_VER1_t)) != read(fileFd, &smtpCfg1, sizeof(SMTP_CONFIG_VER1_t)))
            {
                return FAIL;
            }

            // copy old SMTP parameter into new (UTf-8) variables
            smtpCfg.smtp = smtpCfg1.smtp;
            snprintf(smtpCfg.server, sizeof(smtpCfg.server), "%s", smtpCfg1.server);
            smtpCfg.serverPort = smtpCfg1.serverPort;
            snprintf(smtpCfg.username, sizeof(smtpCfg.username), "%s", smtpCfg1.username);
            snprintf(smtpCfg.password, sizeof(smtpCfg.password), "%s", smtpCfg1.password);
            snprintf(smtpCfg.senderAddress, sizeof(smtpCfg.senderAddress), "%s", smtpCfg1.senderAddress);
            smtpCfg.encryptionType = smtpCfg1.ssl;
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = SMTP_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &smtpCfg, sizeof(SMTP_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write smtp config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldFtpConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;

    switch(fileVersion)
    {
        case 1:
        {
            FTP_UPLOAD_CONFIG_VER1_t ftpUploadCfg1[MAX_FTP_SERVER];

            for(cnt = 0; cnt < MAX_FTP_SERVER; cnt++)
            {
                if((sizeof(FTP_UPLOAD_CONFIG_VER1_t)) != read(fileFd, &ftpUploadCfg1[cnt], sizeof(FTP_UPLOAD_CONFIG_VER1_t)))
                {
                    return FAIL;
                }
            }

            // copy old FTP parameter into new (UTf-8) variables
            for(cnt = 0; cnt < MAX_FTP_SERVER; cnt++)
            {
                ftpUploadCfg[cnt].ftp = ftpUploadCfg1[cnt].ftp;
                snprintf(ftpUploadCfg[cnt].server, sizeof(ftpUploadCfg[cnt].server), "%s", ftpUploadCfg1[cnt].server);
                ftpUploadCfg[cnt].serverPort = ftpUploadCfg1[cnt].serverPort;
                snprintf(ftpUploadCfg[cnt].username, sizeof(ftpUploadCfg[cnt].username), "%s", ftpUploadCfg1[cnt].username);
                snprintf(ftpUploadCfg[cnt].password, sizeof(ftpUploadCfg[cnt].password), "%s", ftpUploadCfg1[cnt].password);
                snprintf(ftpUploadCfg[cnt].uploadPath, sizeof(ftpUploadCfg[cnt].uploadPath), "%s", ftpUploadCfg1[cnt].uploadPath);
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = FTP_UPLOAD_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, ftpUploadCfg, (sizeof(FTP_UPLOAD_CONFIG_t)*MAX_FTP_SERVER)))
    {
        EPRINT(CONFIGURATION, "fail to write ftp upload config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldTcpConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            TCP_NOTIFY_CONFIG_VER1_t tcpNotifyCfg1;

            if((sizeof(TCP_NOTIFY_CONFIG_VER1_t)) != read(fileFd, &tcpNotifyCfg1, sizeof(TCP_NOTIFY_CONFIG_VER1_t)))
            {
                return FAIL;
            }

            // copy old TCP notify parameter into new (UTf-8) variables
            tcpNotifyCfg.tcpNotify = tcpNotifyCfg1.tcpNotify;
            tcpNotifyCfg.serverPort = tcpNotifyCfg1.serverPort;
            snprintf(tcpNotifyCfg.server, sizeof(tcpNotifyCfg.server), "%s", tcpNotifyCfg1.server);
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = TCP_NOTIFY_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &tcpNotifyCfg, sizeof(TCP_NOTIFY_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write tcp notify config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldFileAccessConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch (fileVersion)
    {
        case 1:
        {
            FILE_ACCESS_CONFIG_VER1_t fileAccessCfgV1;

            if (read(fileFd, &fileAccessCfgV1, sizeof(FILE_ACCESS_CONFIG_VER1_t)) != sizeof(FILE_ACCESS_CONFIG_VER1_t))
            {
                EPRINT(CONFIGURATION, "load old file access config failed: [version=%d]", fileVersion);
                return FAIL;
            }

            fileAccessCfg.ftpAccess = fileAccessCfgV1.ftpAccess;
            fileAccessCfg.ftpport = fileAccessCfgV1.ftpport;
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = FILE_ACCESS_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &fileAccessCfg, sizeof(FILE_ACCESS_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write file access config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldDnsConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            MATRIX_DNS_SERVER_CONFIG_VER1_t matrixDnsServerCfg1;

            if((sizeof(MATRIX_DNS_SERVER_CONFIG_VER1_t)) != read(fileFd, &matrixDnsServerCfg1, sizeof(MATRIX_DNS_SERVER_CONFIG_VER1_t)))
            {
                return FAIL;
            }

            // copy old DNS parameter into new (UTF-8) variables
            matrixDnsServerCfg.enMacClient = matrixDnsServerCfg1.enMacClient;
            matrixDnsServerCfg.forwardedPort = matrixDnsServerCfg1.forwardedPort;
            snprintf(matrixDnsServerCfg.hostName, sizeof(matrixDnsServerCfg.hostName), "%s", matrixDnsServerCfg1.hostName);
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = MATRIX_DNS_SERVER_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &matrixDnsServerCfg, sizeof(MATRIX_DNS_SERVER_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write matrix dns config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldUserAccountConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                           cnt, loop;
    USER_ACCOUNT_CONFIG_VER8_t      userAccountCfgV8[MAX_USER_ACCOUNT_VER9];
    USER_ACCOUNT_CONFIG_VER9_10_t   userAccountCfgV9_10[MAX_USER_ACCOUNT];
    USER_ACCOUNT_CONFIG_VER11_t     userAccountCfgV11[MAX_USER_ACCOUNT];

    switch(fileVersion)
    {
        case 8:
        {
            for(cnt = 0; cnt < MAX_USER_ACCOUNT_VER9; cnt++)
            {
                if((sizeof(USER_ACCOUNT_CONFIG_VER8_t)) != read(fileFd, &userAccountCfgV8[cnt], (sizeof(USER_ACCOUNT_CONFIG_VER8_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read user account config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 9:
        {
            for(cnt = 0; cnt < MAX_USER_ACCOUNT_VER9; cnt++)
            {
                if((sizeof(USER_ACCOUNT_CONFIG_VER9_10_t)) != read(fileFd, &userAccountCfgV9_10[cnt], (sizeof(USER_ACCOUNT_CONFIG_VER9_10_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read user account config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;


        case 10:
        {
            for(cnt = 0; cnt < MAX_USER_ACCOUNT; cnt++)
            {
                if((sizeof(USER_ACCOUNT_CONFIG_VER9_10_t)) != read(fileFd, &userAccountCfgV9_10[cnt], (sizeof(USER_ACCOUNT_CONFIG_VER9_10_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read user account config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 11:
        {
            for(cnt = 0; cnt < MAX_USER_ACCOUNT; cnt++)
            {
                if((sizeof(USER_ACCOUNT_CONFIG_VER11_t)) != read(fileFd, &userAccountCfgV11[cnt], (sizeof(USER_ACCOUNT_CONFIG_VER11_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read user account config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if(fileVersion == 8)
    {
        for(cnt = 0; cnt < MAX_USER_ACCOUNT_VER9; cnt++)
        {
            // Note: Here size of username and password field has been changed to accommodate UTF-8 char, so we can not use 'memcpy()'
            snprintf(userAccountCfgV9_10[cnt].username, sizeof(userAccountCfgV9_10[cnt].username), "%s", userAccountCfgV8[cnt].username);
            userAccountCfgV9_10[cnt].userGroup = userAccountCfgV8[cnt].userGroup;
            snprintf(userAccountCfgV9_10[cnt].password, sizeof(userAccountCfgV9_10[cnt].password), "%s", userAccountCfgV8[cnt].password);
            userAccountCfgV9_10[cnt].userStatus = userAccountCfgV8[cnt].userStatus;
            userAccountCfgV9_10[cnt].multiLogin = userAccountCfgV8[cnt].multiLogin;
            userAccountCfgV9_10[cnt].loginSessionDuration = userAccountCfgV8[cnt].loginSessionDuration;
            memcpy(userAccountCfgV9_10[cnt].userPrivilege, userAccountCfgV8[cnt].userPrivilege, sizeof(userAccountCfgV9_10[cnt].userPrivilege));
            userAccountCfgV9_10[cnt].syncNetDev = userAccountCfgV8[cnt].syncNetDev;
            snprintf(userAccountCfgV9_10[cnt].preferredLanguage, sizeof(userAccountCfgV9_10[cnt].preferredLanguage), "%s", DfltUserAccountCfg.preferredLanguage);
        }
        fileVersion++;
    }

    if(fileVersion == 9)
    {
        /* No need to copy previous user as we have same structure for V9 and V10 */
        for(cnt = MAX_USER_ACCOUNT_VER9; cnt < MAX_USER_ACCOUNT; cnt++)
        {
            snprintf(userAccountCfgV9_10[cnt].username, sizeof(userAccountCfgV9_10[cnt].username), "%s", DfltUserAccountCfg.username);
            userAccountCfgV9_10[cnt].userGroup = DfltUserAccountCfg.userGroup;
            snprintf(userAccountCfgV9_10[cnt].password, sizeof(userAccountCfgV9_10[cnt].password), "%s", DfltUserAccountCfg.password);
            userAccountCfgV9_10[cnt].userStatus = DfltUserAccountCfg.userStatus;
            userAccountCfgV9_10[cnt].multiLogin = DfltUserAccountCfg.multiLogin;
            userAccountCfgV9_10[cnt].loginSessionDuration = DfltUserAccountCfg.loginSessionDuration;
            memcpy(userAccountCfgV9_10[cnt].userPrivilege, DfltUserAccountCfg.userPrivilege, sizeof(userAccountCfgV9_10[cnt].userPrivilege));
            userAccountCfgV9_10[cnt].syncNetDev = DfltUserAccountCfg.syncNetDev;
            snprintf(userAccountCfgV9_10[cnt].preferredLanguage, sizeof(userAccountCfgV9_10[cnt].preferredLanguage), "%s", DfltUserAccountCfg.preferredLanguage);
        }
        fileVersion++;
    }

    if(fileVersion == 10)
    {
        for(cnt = 0; cnt < MAX_USER_ACCOUNT; cnt++)
        {
            memcpy(&userAccountCfgV11[cnt], &userAccountCfgV9_10[cnt], sizeof(userAccountCfgV9_10[cnt]));
            /* disable new added push notification flag */
            userAccountCfgV11[cnt].managePushNotificationRights = (cnt == USER_ADMIN) ? ENABLE : DISABLE;
        }
        fileVersion++;
    }

    if(fileVersion == 11)
    {
        for(cnt = 0; cnt < MAX_USER_ACCOUNT; cnt++)
        {
            snprintf(userAccountCfg[cnt].username, sizeof(userAccountCfg[cnt].username), "%s", userAccountCfgV11[cnt].username);
            userAccountCfg[cnt].userGroup = userAccountCfgV11[cnt].userGroup;
            snprintf(userAccountCfg[cnt].password, sizeof(userAccountCfg[cnt].password), "%s", userAccountCfgV11[cnt].password);
            userAccountCfg[cnt].userStatus = userAccountCfgV11[cnt].userStatus;
            userAccountCfg[cnt].multiLogin = userAccountCfgV11[cnt].multiLogin;
            userAccountCfg[cnt].loginSessionDuration = userAccountCfgV11[cnt].loginSessionDuration;
            userAccountCfg[cnt].syncNetDev = userAccountCfgV11[cnt].syncNetDev;
            snprintf(userAccountCfg[cnt].preferredLanguage, sizeof(userAccountCfg[cnt].preferredLanguage), "%s", userAccountCfgV11[cnt].preferredLanguage);
            userAccountCfg[cnt].managePushNotificationRights = userAccountCfgV11[cnt].managePushNotificationRights;

            memcpy(userAccountCfg[cnt].userPrivilege, userAccountCfgV11[cnt].userPrivilege, sizeof(userAccountCfgV11[cnt].userPrivilege));
            for (loop = CAMERA_CONFIG_MAX_V1; loop < MAX_CAMERA_CONFIG; loop++)
            {
                userAccountCfg[cnt].userPrivilege[loop] = DfltUserAccountCfg.userPrivilege[loop];
            }
        }
        fileVersion++;
    }

    fileVersion = USER_ACCOUNT_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, userAccountCfg, sizeof(userAccountCfg)))
    {
        EPRINT(CONFIGURATION, "fail to write user account config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldCameraConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                   cnt, loop;
    CAMERA_CONFIG_VER3_t    cameraCfgV3[MAX_CAMERA];
    CAMERA_CONFIG_VER4_t    cameraCfgV4[MAX_CAMERA];
    CAMERA_CONFIG_VER5_t    cameraCfgV5[MAX_CAMERA];

    switch(fileVersion)
    {
        case 3:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if(sizeof(CAMERA_CONFIG_VER3_t) != read(fileFd, &cameraCfgV3[cnt], sizeof(CAMERA_CONFIG_VER3_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read camera config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 4:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if(sizeof(CAMERA_CONFIG_VER4_t) != read(fileFd, &cameraCfgV4[cnt], sizeof(CAMERA_CONFIG_VER4_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read camera config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 5:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if(sizeof(CAMERA_CONFIG_VER5_t) != read(fileFd, &cameraCfgV5[cnt], sizeof(CAMERA_CONFIG_VER5_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read camera config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if(fileVersion == 3)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            memcpy(&cameraCfgV4[cnt], &cameraCfgV3[cnt], sizeof(CAMERA_CONFIG_VER3_t));
            snprintf(cameraCfgV4[cnt].name, sizeof(cameraCfgV4[cnt].name), "%s", cameraCfgV4[cnt].nameV1);
            snprintf(cameraCfgV4[cnt].channelName, sizeof(cameraCfgV4[cnt].channelName), "%s", cameraCfgV4[cnt].channelNameV1);
        }
        fileVersion = 4;
    }

    if(fileVersion == 4)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            cameraCfgV5[cnt].camera = cameraCfgV4[cnt].camera;
            snprintf(cameraCfgV5[cnt].name, sizeof(cameraCfgV4[cnt].name), "%s", cameraCfgV4[cnt].name);
            cameraCfgV5[cnt].type = cameraCfgV4[cnt].type;
            cameraCfgV5[cnt].logMotionEvents = cameraCfgV4[cnt].logMotionEvents;
            cameraCfgV5[cnt].motionReDetectionDelay = cameraCfgV4[cnt].motionReDetectionDelay;
            cameraCfgV5[cnt].cameraNamePos = cameraCfgV4[cnt].cameraNamePos;
            cameraCfgV5[cnt].statusPos = cameraCfgV4[cnt].statusPos;
            cameraCfgV5[cnt].dateTimeOverlay = cameraCfgV4[cnt].dateTimeOverlay;
            cameraCfgV5[cnt].dateTimePos = cameraCfgV4[cnt].dateTimePos;
            cameraCfgV5[cnt].channelNameOverlay = cameraCfgV4[cnt].channelNameOverlay;
            snprintf(cameraCfgV5[cnt].channelName, sizeof(cameraCfgV4[cnt].channelName), "%s", cameraCfgV4[cnt].channelName);
            cameraCfgV5[cnt].channelNamePos = cameraCfgV4[cnt].channelNamePos;
            snprintf(cameraCfgV5[cnt].mobileNum, sizeof(cameraCfgV4[cnt].mobileNum), "%s", cameraCfgV4[cnt].mobileNum);
            cameraCfgV5[cnt].privacyMaskStaus = cameraCfgV4[cnt].privacyMaskStaus;
            memcpy(cameraCfgV5[cnt].privacyMask, cameraCfgV4[cnt].privacyMask, sizeof(cameraCfgV4[cnt].privacyMask));
            cameraCfgV5[cnt].motionDetectionStatus = cameraCfgV4[cnt].motionDetectionStatus;
            memcpy(cameraCfgV5[cnt].motionArea, cameraCfgV4[cnt].motionArea, sizeof(cameraCfgV4[cnt].motionArea));
            cameraCfgV5[cnt].recordingStream = cameraCfgV4[cnt].recordingStream;
            memset(cameraCfgV5[cnt].motionAreaBlockBits, 0, sizeof(cameraCfgV5[cnt].motionAreaBlockBits));
        }
        fileVersion = 5;
    }

    if(fileVersion == 5)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            cameraCfg[cnt].camera = cameraCfgV5[cnt].camera;
            snprintf(cameraCfg[cnt].name, sizeof(cameraCfgV5[cnt].name), "%s", cameraCfgV5[cnt].name);
            cameraCfg[cnt].type = cameraCfgV5[cnt].type;
            cameraCfg[cnt].logMotionEvents = cameraCfgV5[cnt].logMotionEvents;
            cameraCfg[cnt].motionReDetectionDelay = cameraCfgV5[cnt].motionReDetectionDelay;
            cameraCfg[cnt].cameraNamePos = cameraCfgV5[cnt].cameraNamePos;
            cameraCfg[cnt].statusPos = cameraCfgV5[cnt].statusPos;
            cameraCfg[cnt].dateTimeOverlay = cameraCfgV5[cnt].dateTimeOverlay;
            cameraCfg[cnt].dateTimePos = cameraCfgV5[cnt].dateTimePos;
            cameraCfg[cnt].channelNameOverlay = cameraCfgV5[cnt].channelNameOverlay;
            snprintf(cameraCfg[cnt].channelName[0], sizeof(cameraCfgV5[cnt].channelName), "%s", cameraCfgV5[cnt].channelName);
            cameraCfg[cnt].channelNamePos[0] = cameraCfgV5[cnt].channelNamePos;
            for (loop = 1; loop < TEXT_OVERLAY_MAX; loop++)
            {
                snprintf(cameraCfg[cnt].channelName[loop], sizeof(cameraCfg[cnt].channelName[loop]), "%s", DfltCameraCfg.channelName[loop]);
                cameraCfg[cnt].channelNamePos[loop] = DfltCameraCfg.channelNamePos[loop];
            }
            snprintf(cameraCfg[cnt].mobileNum, sizeof(cameraCfgV5[cnt].mobileNum), "%s", cameraCfgV5[cnt].mobileNum);
            cameraCfg[cnt].privacyMaskStaus = cameraCfgV5[cnt].privacyMaskStaus;
            cameraCfg[cnt].motionDetectionStatus = cameraCfgV5[cnt].motionDetectionStatus;
            cameraCfg[cnt].recordingStream = cameraCfgV5[cnt].recordingStream;
        }
        fileVersion = 6;
    }

    fileVersion = CAMERA_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, cameraCfg, (sizeof(CAMERA_CONFIG_t) * getMaxCameraForCurrentVariant())))
    {
        EPRINT(CONFIGURATION, "fail to write camera config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldPresetPositionConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                       cnt, loop;
    PTZ_PRESET_CONFIG_VER1_t    ptzPresetCfgV1[MAX_CAMERA];
    PTZ_PRESET_CONFIG_VER2_t    ptzPresetCfgV2[MAX_CAMERA];

    switch(fileVersion)
    {
        case 1:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if(sizeof(PTZ_PRESET_CONFIG_VER1_t) != read(fileFd, &ptzPresetCfgV1[cnt], sizeof(PTZ_PRESET_CONFIG_VER1_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read preset position config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 2:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if(sizeof(PTZ_PRESET_CONFIG_VER2_t) != read(fileFd, &ptzPresetCfgV2[cnt], sizeof(PTZ_PRESET_CONFIG_VER2_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read preset position config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 1)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            for(loop = 0; loop < MAX_PRESET_POSITION; loop++)
            {
                snprintf(ptzPresetCfgV2[cnt].name[loop], sizeof(ptzPresetCfgV2[cnt].name[loop]), "%s", ptzPresetCfgV1[cnt].name[loop]);
            }
        }
        fileVersion = 2;
    }

    if (fileVersion == 2)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            for(loop = 0; loop < MAX_PRESET_POSITION; loop++)
            {
                snprintf(ptzPresetCfg[cnt][loop].name, sizeof(ptzPresetCfg[cnt][loop].name), "%s", ptzPresetCfgV2[cnt].name[loop]);
                snprintf(ptzPresetCfg[cnt][loop].token, sizeof(ptzPresetCfg[cnt][loop].token), "%s", DfltPtzPresetCfg.name);
            }
        }
        fileVersion = 3;
    }

    fileVersion = PTZ_PRESET_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, ptzPresetCfg, (sizeof(PTZ_PRESET_CONFIG_t) * getMaxCameraForCurrentVariant() * MAX_PRESET_POSITION)))
    {
        EPRINT(CONFIGURATION, "fail to write ptz preset config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldPresetTourConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                       cnt;
    UINT8                       loop;
    PRESET_TOUR_CONFIG_VER3_t   presetTourCfgV3[MAX_CAMERA];

    switch(fileVersion)
    {
        case 3:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((sizeof(PRESET_TOUR_CONFIG_VER3_t)) != read(fileFd, &presetTourCfgV3[cnt], (sizeof(PRESET_TOUR_CONFIG_VER3_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read preset tour config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if(fileVersion == 3)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            presetTourCfg[cnt].manualTourOverride = presetTourCfgV3[cnt].manualTourOverride;
            presetTourCfg[cnt].manualTour = presetTourCfgV3[cnt].manualTour;
            presetTourCfg[cnt].activeTourOverride = presetTourCfgV3[cnt].activeTourOverride;
            for(loop = 0; loop < MAX_TOUR_NUMBER; loop++)
            {
                snprintf(presetTourCfg[cnt].tour[loop].name, sizeof(presetTourCfg[cnt].tour[loop].name), "%s", presetTourCfgV3[cnt].tour[loop].name);
                presetTourCfg[cnt].tour[loop].tourPattern = presetTourCfgV3[cnt].tour[loop].tourPattern;
                presetTourCfg[cnt].tour[loop].pauseBetweenTour = presetTourCfgV3[cnt].tour[loop].pauseBetweenTour;
                memcpy(&presetTourCfg[cnt].tour[loop].ptz[0],&presetTourCfgV3[cnt].tour[loop].ptz[0],(sizeof(ORDER_NUMBER_t)*MAX_ORDER_COUNT));
            }
        }
        fileVersion = 4;
    }

    fileVersion = PRESET_TOUR_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, presetTourCfg, (sizeof(PRESET_TOUR_CONFIG_t) * getMaxCameraForCurrentVariant())))
    {
        EPRINT(CONFIGURATION, "fail to write preset tour config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSystemSensorConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;

    switch(fileVersion)
    {
        case 1:
        {
            SENSOR_CONFIG_VER1_t sensorCfg1[MAX_SENSOR];
            for(cnt = 0; cnt < MAX_SENSOR; cnt++)
            {
                if((sizeof(SENSOR_CONFIG_VER1_t)) != read(fileFd, &sensorCfg1[cnt], sizeof(SENSOR_CONFIG_VER1_t)))
                {
                    return FAIL;
                }
            }

            // copy old System sensor parameter into new (UTf-8) variables

            for(cnt = 0; cnt < MAX_SENSOR; cnt++)
            {
                sensorCfg[cnt].sensorDetect = sensorCfg1[cnt].sensorDetect;
                snprintf(sensorCfg[cnt].name, sizeof(sensorCfg[cnt].name), "%s", sensorCfg1[cnt].name);
                sensorCfg[cnt].normalMode = sensorCfg1[cnt].normalMode;
                sensorCfg[cnt].debounceTime = sensorCfg1[cnt].debounceTime;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = SENSOR_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, sensorCfg, (sizeof(SENSOR_CONFIG_t)*MAX_SENSOR)))
    {
        EPRINT(CONFIGURATION, "fail to write sensor config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSystemAlarmConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;

    switch(fileVersion)
    {
        case 1:
        {
            ALARM_CONFIG_VER1_t alarmCfg1[MAX_ALARM];

            for(cnt = 0; cnt < MAX_ALARM; cnt++)
            {
                if((sizeof(ALARM_CONFIG_VER1_t)) != read(fileFd, &alarmCfg1[cnt], sizeof(ALARM_CONFIG_VER1_t)))
                {
                    return FAIL;
                }
            }

            // copy old System alarm parameter into new (UTf-8) variables
            for(cnt = 0; cnt < MAX_ALARM; cnt++)
            {
                alarmCfg[cnt].alarmOutput = alarmCfg1[cnt].alarmOutput;
                snprintf(alarmCfg[cnt].name, sizeof(alarmCfg[cnt].name), "%s", alarmCfg1[cnt].name);
                alarmCfg[cnt].activeMode = alarmCfg1[cnt].activeMode;
                alarmCfg[cnt].pulsePeriod = alarmCfg1[cnt].pulsePeriod;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = ALARM_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, alarmCfg, (sizeof(ALARM_CONFIG_t)*MAX_ALARM)))
    {
        EPRINT(CONFIGURATION, "fail to write alarm config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldStorageConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;

    switch(fileVersion)
    {
        case 2:
        {
            STORAGE_CONFIG_VER2_t storageCfgV2;

            if((sizeof(STORAGE_CONFIG_VER2_t)) != read(fileFd, &storageCfgV2, sizeof(STORAGE_CONFIG_VER2_t)))
            {
                EPRINT(CONFIGURATION, "fail to read storage config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }

            storageCfg.hddFull = storageCfgV2.hddFull;
            storageCfg.recordRetentionStatus = storageCfgV2.recordRetentionStatus;
            storageCfg.recordRetentionType = storageCfgV2.recordRetentionType;
            storageCfg.driveWiseRecordCleanupByDays = storageCfgV2.driveWiseRecordCleanupByDays;
            storageCfg.hddStorage = storageCfgV2.hddStorage;
            storageCfg.backUpRetentionStatus = storageCfgV2.backUpRetentionStatus;
            for(cnt = 0; cnt < CAMERA_CONFIG_MAX_V1; cnt++)
            {
                storageCfg.cameraWiseRecordCleanupByDays[cnt] = storageCfgV2.cameraWiseRecordCleanupByDays[cnt];
                storageCfg.cameraWiseBackUpCleanupByDays[cnt] = storageCfgV2.cameraWiseBackUpCleanupByDays[cnt];
            }

            /* set default config for camera 65 to 96*/
            for(cnt = CAMERA_CONFIG_MAX_V1; cnt < MAX_CAMERA_CONFIG; cnt++)
            {
                storageCfg.cameraWiseRecordCleanupByDays[cnt] = DfltStorageCfg.cameraWiseRecordCleanupByDays[cnt];
                storageCfg.cameraWiseBackUpCleanupByDays[cnt] = DfltStorageCfg.cameraWiseBackUpCleanupByDays[cnt];
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = STORAGE_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &storageCfg, sizeof(STORAGE_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write storage config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldScheduleBackupConfig(INT32 fileFd, UINT32 fileVersion)
{
    SCHEDULE_BACKUP_CONFIG_VER3_t   scheduleBackupCfgV3;

    switch(fileVersion)
    {
        case 3:
        {
            if((sizeof(SCHEDULE_BACKUP_CONFIG_VER3_t)) != read(fileFd, &scheduleBackupCfgV3, sizeof(SCHEDULE_BACKUP_CONFIG_VER3_t)))
            {
                EPRINT(CONFIGURATION, "fail to read schedule backup config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        default:
        {
           return FAIL;
        }
    }

    if(fileVersion == 3)
    {
        scheduleBackupCfg.scheduleBackup = scheduleBackupCfgV3.scheduleBackup;
        scheduleBackupCfg.mode = scheduleBackupCfgV3.mode;
        scheduleBackupCfg.everyHourMode = scheduleBackupCfgV3.everyHourMode;
        scheduleBackupCfg.everydayBackup = scheduleBackupCfgV3.everydayBackup;
        scheduleBackupCfg.weeklyBackup = scheduleBackupCfgV3.weeklyBackup;
        scheduleBackupCfg.camera.bitMask[0] = scheduleBackupCfgV3.camera;
        scheduleBackupCfg.backupLocation = scheduleBackupCfgV3.backupLocation;

        /* set default config for camera bit mask (for 65 to 96 Camera)*/
        scheduleBackupCfg.camera.bitMask[1] = DfltScheduleBackupCfg.camera.bitMask[1];
        fileVersion++;
    }

    fileVersion = SCHEDULE_BACKUP_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &scheduleBackupCfg, sizeof(SCHEDULE_BACKUP_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write schedule backup config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldManualBackupConfig(INT32 fileFd, UINT32 fileVersion)
{
    MANUAL_BACKUP_CONFIG_VER4_t   manualBackupCfgV4;

    switch(fileVersion)
    {
        case 4:
        {
            if((sizeof(MANUAL_BACKUP_CONFIG_VER4_t)) != read(fileFd, &manualBackupCfgV4, sizeof(MANUAL_BACKUP_CONFIG_VER4_t)))
            {
                EPRINT(CONFIGURATION, "fail to read manual backup config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        default:
        {
           return FAIL;
        }
    }

    if(fileVersion == 4)
    {
        manualBackupCfg.manualBackup = manualBackupCfgV4.manualBackup;
        manualBackupCfg.duration = manualBackupCfgV4.duration;
        manualBackupCfg.camera.bitMask[0] = manualBackupCfgV4.camera;
        manualBackupCfg.backupLocation = manualBackupCfgV4.backupLocation;

        /* set default config for camera bit mask (for 65 to 96 Camera)*/
        manualBackupCfg.camera.bitMask[1] = DfltManualBackupCfg.camera.bitMask[1];
        fileVersion++;
    }

    fileVersion = MANUAL_BACKUP_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &manualBackupCfg, sizeof(MANUAL_BACKUP_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write manual backup config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldUploadImageConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;

    switch(fileVersion)
    {
        case 2:
        {
            IMAGE_UPLOAD_CONFIG_VER2_t imageUploadCfg2[MAX_CAMERA];

            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((sizeof(IMAGE_UPLOAD_CONFIG_VER2_t)) != read(fileFd, &imageUploadCfg2[cnt], sizeof(IMAGE_UPLOAD_CONFIG_VER2_t)))
                {
                    return FAIL;
                }
            }

            // copy old Image upload parameter into new (UTf-8) variables
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                snprintf(imageUploadCfg[cnt].resolution, sizeof(imageUploadCfg[cnt].resolution), "%s", imageUploadCfg2[cnt].resolution);
                imageUploadCfg[cnt].uploadLocation = imageUploadCfg2[cnt].uploadLocation;

                /* update max images allowed per minute */
                if (imageUploadCfg2[cnt].uploadImageRate > MAX_IMAGE_UPLOAD_PER_MINUTE)
                {
                    imageUploadCfg2[cnt].uploadImageRate = MAX_IMAGE_UPLOAD_PER_MINUTE;
                }

                imageUploadCfg[cnt].uploadImageRate = imageUploadCfg2[cnt].uploadImageRate;
                snprintf(imageUploadCfg[cnt].uploadToEmail.emailAddress,
                         sizeof(imageUploadCfg[cnt].uploadToEmail.emailAddress), "%s", imageUploadCfg2[cnt].uploadToEmail.emailAddress);
                snprintf(imageUploadCfg[cnt].uploadToEmail.subject,
                         sizeof(imageUploadCfg[cnt].uploadToEmail.subject), "%s", imageUploadCfg2[cnt].uploadToEmail.subject);
                snprintf(imageUploadCfg[cnt].uploadToEmail.message,
                         sizeof(imageUploadCfg[cnt].uploadToEmail.message), "%s", imageUploadCfg2[cnt].uploadToEmail.message);
            }
        }
        break;

        case 3:
        {
            IMAGE_UPLOAD_CONFIG_t imageUploadCfg3[MAX_CAMERA];

            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if ((sizeof(IMAGE_UPLOAD_CONFIG_t)) != read(fileFd, &imageUploadCfg3[cnt], sizeof(IMAGE_UPLOAD_CONFIG_t)))
                {
                    return FAIL;
                }

                /* update max images allowed per minute */
                if (imageUploadCfg3[cnt].uploadImageRate > MAX_IMAGE_UPLOAD_PER_MINUTE)
                {
                    imageUploadCfg3[cnt].uploadImageRate = MAX_IMAGE_UPLOAD_PER_MINUTE;
                }

                /* copy to global structure */
                memcpy(&imageUploadCfg[cnt], &imageUploadCfg3[cnt], sizeof(IMAGE_UPLOAD_CONFIG_t));
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = IMAGE_UPLOAD_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, imageUploadCfg, (sizeof(IMAGE_UPLOAD_CONFIG_t)*getMaxCameraForCurrentVariant())))
    {
        EPRINT(CONFIGURATION, "fail to write image upload config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldCameraEventAndActionConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                           camIdx, evtIdx;
    UINT32                          configVersion = fileVersion;
    CAMERA_EVENT_CONFIG_VER4_5_t    camEvtCfgV4_5;
    CAMERA_EVENT_CONFIG_VER6_t      camEvtCfgV6;
    CAMERA_EVENT_CONFIG_VER7_8_t    camEvtCfgV7_8;

    for(camIdx = 0; camIdx < getMaxCameraForCurrentVariant(); camIdx++)
    {
        for(evtIdx = 0; evtIdx < MAX_CAMERA_EVENT; evtIdx++)
        {
            fileVersion = configVersion;
            switch(fileVersion)
            {
                case 4:
                {
                    if (evtIdx >= MAX_CAMERA_EVENT_VER4)
                    {
                        break;
                    }

                    if (sizeof(CAMERA_EVENT_CONFIG_VER4_5_t) != read(fileFd, &camEvtCfgV4_5, sizeof(CAMERA_EVENT_CONFIG_VER4_5_t)))
                    {
                        EPRINT(CONFIGURATION, "fail to read camera event config file: [camera=%d], [event=%d], [version=%d], [err=%s]",
                               camIdx, evtIdx, fileVersion, STR_ERR);
                        return FAIL;
                    }
                }
                break;

                case 5:
                {
                    if (evtIdx >= MAX_CAMERA_EVENT_VER7)
                    {
                        break;
                    }

                    if (sizeof(CAMERA_EVENT_CONFIG_VER4_5_t) != read(fileFd, &camEvtCfgV4_5, sizeof(CAMERA_EVENT_CONFIG_VER4_5_t)))
                    {
                        EPRINT(CONFIGURATION, "fail to read camera event config file: [camera=%d], [event=%d], [version=%d], [err=%s]",
                               camIdx, evtIdx, fileVersion, STR_ERR);
                        return FAIL;
                    }
                }
                break;

                case 6:
                {
                    if (evtIdx >= MAX_CAMERA_EVENT_VER7)
                    {
                        break;
                    }

                    if (sizeof(CAMERA_EVENT_CONFIG_VER6_t) != read(fileFd, &camEvtCfgV6, sizeof(CAMERA_EVENT_CONFIG_VER6_t)))
                    {
                        EPRINT(CONFIGURATION, "fail to read camera event config file: [camera=%d], [event=%d], [version=%d], [err=%s]",
                               camIdx, evtIdx, fileVersion, STR_ERR);
                        return FAIL;
                    }
                }
                break;

                case 7:
                {
                    if (evtIdx >= MAX_CAMERA_EVENT_VER7)
                    {
                        break;
                    }

                    if (sizeof(CAMERA_EVENT_CONFIG_VER7_8_t) != read(fileFd, &camEvtCfgV7_8, sizeof(CAMERA_EVENT_CONFIG_VER7_8_t)))
                    {
                        EPRINT(CONFIGURATION, "fail to read camera event config file: [camera=%d], [event=%d], [version=%d], [err=%s]",
                               camIdx, evtIdx, fileVersion, STR_ERR);
                        return FAIL;
                    }
                }
                break;

                case 8:
                {
                    if (sizeof(CAMERA_EVENT_CONFIG_VER7_8_t) != read(fileFd, &camEvtCfgV7_8, sizeof(CAMERA_EVENT_CONFIG_VER7_8_t)))
                    {
                        EPRINT(CONFIGURATION, "fail to read camera event config file: [camera=%d], [event=%d], [version=%d], [err=%s]",
                               camIdx, evtIdx, fileVersion, STR_ERR);
                        return FAIL;
                    }
                }
                break;

                default:
                {
                    /* Invalid version */
                }
                return FAIL;
            }

            if (fileVersion == 4)
            {
                if ((evtIdx >= MAX_CAMERA_EVENT_VER4) && (evtIdx < MAX_CAMERA_EVENT_VER7))
                {
                    camEvtCfgV4_5.action = DfltCameraEventActionCfg.action;
                    memcpy(camEvtCfgV4_5.actionParam.alarmRecord, DfltCameraEventActionCfg.actionParam.alarmRecord, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    memcpy(camEvtCfgV4_5.actionParam.uploadImage, DfltCameraEventActionCfg.actionParam.uploadImage, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    snprintf(camEvtCfgV4_5.actionParam.sendEmail.emailAddress, sizeof(camEvtCfgV4_5.actionParam.sendEmail.emailAddress), "%s", DfltCameraEventActionCfg.actionParam.sendEmail.emailAddress);
                    snprintf(camEvtCfgV4_5.actionParam.sendEmail.subject, sizeof(camEvtCfgV4_5.actionParam.sendEmail.subject), "%s", DfltCameraEventActionCfg.actionParam.sendEmail.subject);
                    snprintf(camEvtCfgV4_5.actionParam.sendEmail.message, sizeof(camEvtCfgV4_5.actionParam.sendEmail.message), "%s", DfltCameraEventActionCfg.actionParam.sendEmail.message);
                    snprintf(camEvtCfgV4_5.actionParam.sendTcp, sizeof(camEvtCfgV4_5.actionParam.sendTcp), "%s", DfltCameraEventActionCfg.actionParam.sendTcp);
                    snprintf(camEvtCfgV4_5.actionParam.smsParameter.mobileNumber1, sizeof(camEvtCfgV4_5.actionParam.smsParameter.mobileNumber1), "%s", DfltCameraEventActionCfg.actionParam.smsParameter.mobileNumber1);
                    snprintf(camEvtCfgV4_5.actionParam.smsParameter.mobileNumber2, sizeof(camEvtCfgV4_5.actionParam.smsParameter.mobileNumber2), "%s", DfltCameraEventActionCfg.actionParam.smsParameter.mobileNumber2);
                    snprintf(camEvtCfgV4_5.actionParam.smsParameter.message, sizeof(camEvtCfgV4_5.actionParam.smsParameter.message), "%s", DfltCameraEventActionCfg.actionParam.smsParameter.message);
                    camEvtCfgV4_5.actionParam.gotoPosition = DfltCameraEventActionCfg.actionParam.gotoPosition;
                    memcpy(camEvtCfgV4_5.actionParam.systemAlarmOutput, DfltCameraEventActionCfg.actionParam.systemAlarmOutput, (sizeof(BOOL) * MAX_ALARM));
                    camEvtCfgV4_5.actionParam.cameraAlarmOutput = DfltCameraEventActionCfg.actionParam.cameraAlarmOutput;
                    memcpy(camEvtCfgV4_5.weeklySchedule, DfltCameraEventActionCfg.weeklySchedule, (sizeof(WEEKLY_ACTION_SCHEDULE_t) * MAX_WEEK_DAYS));
                    camEvtCfgV4_5.copyToCam = DfltCameraEventActionCfg.copyToCam.bitMask[0];
                }

                fileVersion++;
            }

            if(fileVersion == 5)
            {
                if (evtIdx < MAX_CAMERA_EVENT_VER7)
                {
                    camEvtCfgV6.action = camEvtCfgV4_5.action;
                    memcpy(camEvtCfgV6.actionParam.alarmRecord, camEvtCfgV4_5.actionParam.alarmRecord, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    memcpy(camEvtCfgV6.actionParam.uploadImage, camEvtCfgV4_5.actionParam.uploadImage, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    snprintf(camEvtCfgV6.actionParam.sendEmail.emailAddress, sizeof(camEvtCfgV6.actionParam.sendEmail.emailAddress), "%s", camEvtCfgV4_5.actionParam.sendEmail.emailAddress);
                    snprintf(camEvtCfgV6.actionParam.sendEmail.subject, sizeof(camEvtCfgV6.actionParam.sendEmail.subject), "%s", camEvtCfgV4_5.actionParam.sendEmail.subject);
                    snprintf(camEvtCfgV6.actionParam.sendEmail.message, sizeof(camEvtCfgV6.actionParam.sendEmail.message), "%s", camEvtCfgV4_5.actionParam.sendEmail.message);
                    snprintf(camEvtCfgV6.actionParam.sendTcp, sizeof(camEvtCfgV6.actionParam.sendTcp), "%s", camEvtCfgV4_5.actionParam.sendTcp);
                    snprintf(camEvtCfgV6.actionParam.smsParameter.mobileNumber1, sizeof(camEvtCfgV6.actionParam.smsParameter.mobileNumber1), "%s", camEvtCfgV4_5.actionParam.smsParameter.mobileNumber1);
                    snprintf(camEvtCfgV6.actionParam.smsParameter.mobileNumber2, sizeof(camEvtCfgV6.actionParam.smsParameter.mobileNumber2), "%s", camEvtCfgV4_5.actionParam.smsParameter.mobileNumber2);
                    snprintf(camEvtCfgV6.actionParam.smsParameter.message, sizeof(camEvtCfgV6.actionParam.smsParameter.message), "%s", camEvtCfgV4_5.actionParam.smsParameter.message);
                    camEvtCfgV6.actionParam.gotoPosition = camEvtCfgV4_5.actionParam.gotoPosition;
                    memcpy(camEvtCfgV6.actionParam.systemAlarmOutput, camEvtCfgV4_5.actionParam.systemAlarmOutput, (sizeof(BOOL) * MAX_ALARM));
                    camEvtCfgV6.actionParam.cameraAlarmOutput = camEvtCfgV4_5.actionParam.cameraAlarmOutput;
                    memcpy(camEvtCfgV6.weeklySchedule, camEvtCfgV4_5.weeklySchedule, (sizeof(WEEKLY_ACTION_SCHEDULE_t) * MAX_WEEK_DAYS));
                    camEvtCfgV6.copyToCam = camEvtCfgV4_5.copyToCam;
                }

                fileVersion++;
            }

            if(fileVersion == 6)
            {
                if (evtIdx < MAX_CAMERA_EVENT_VER7)
                {
                    camEvtCfgV7_8.action = camEvtCfgV6.action;
                    memcpy(camEvtCfgV7_8.actionParam.alarmRecord, camEvtCfgV6.actionParam.alarmRecord, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    memcpy(camEvtCfgV7_8.actionParam.uploadImage, camEvtCfgV6.actionParam.uploadImage, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    snprintf(camEvtCfgV7_8.actionParam.sendEmail.emailAddress, sizeof(camEvtCfgV7_8.actionParam.sendEmail.emailAddress), "%s", camEvtCfgV6.actionParam.sendEmail.emailAddress);
                    snprintf(camEvtCfgV7_8.actionParam.sendEmail.subject, sizeof(camEvtCfgV7_8.actionParam.sendEmail.subject), "%s", camEvtCfgV6.actionParam.sendEmail.subject);
                    snprintf(camEvtCfgV7_8.actionParam.sendEmail.message, sizeof(camEvtCfgV7_8.actionParam.sendEmail.message), "%s", camEvtCfgV6.actionParam.sendEmail.message);
                    snprintf(camEvtCfgV7_8.actionParam.sendTcp, sizeof(camEvtCfgV7_8.actionParam.sendTcp), "%s", camEvtCfgV6.actionParam.sendTcp);
                    snprintf(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber1, sizeof(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber1), "%s", camEvtCfgV6.actionParam.smsParameter.mobileNumber1);
                    snprintf(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber2, sizeof(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber2), "%s", camEvtCfgV6.actionParam.smsParameter.mobileNumber2);
                    snprintf(camEvtCfgV7_8.actionParam.smsParameter.message, sizeof(camEvtCfgV7_8.actionParam.smsParameter.message), "%s", camEvtCfgV6.actionParam.smsParameter.message);
                    camEvtCfgV7_8.actionParam.gotoPosition = camEvtCfgV6.actionParam.gotoPosition;
                    memcpy(camEvtCfgV7_8.actionParam.systemAlarmOutput, camEvtCfgV6.actionParam.systemAlarmOutput, (sizeof(BOOL) * MAX_ALARM));
                    camEvtCfgV7_8.actionParam.cameraAlarmOutput = camEvtCfgV6.actionParam.cameraAlarmOutput;
                    memcpy(camEvtCfgV7_8.weeklySchedule, camEvtCfgV6.weeklySchedule, (sizeof(WEEKLY_ACTION_SCHEDULE_t) * MAX_WEEK_DAYS));
                    camEvtCfgV7_8.copyToCam = camEvtCfgV6.copyToCam;
                }

                fileVersion++;
            }

            if (fileVersion == 7)
            {
                if (evtIdx >= MAX_CAMERA_EVENT_VER7)
                {
                    camEvtCfgV7_8.action = DfltCameraEventActionCfg.action;
                    memcpy(camEvtCfgV7_8.actionParam.alarmRecord, DfltCameraEventActionCfg.actionParam.alarmRecord, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    memcpy(camEvtCfgV7_8.actionParam.uploadImage, DfltCameraEventActionCfg.actionParam.uploadImage, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                    snprintf(camEvtCfgV7_8.actionParam.sendEmail.emailAddress, sizeof(camEvtCfgV7_8.actionParam.sendEmail.emailAddress), "%s", DfltCameraEventActionCfg.actionParam.sendEmail.emailAddress);
                    snprintf(camEvtCfgV7_8.actionParam.sendEmail.subject, sizeof(camEvtCfgV7_8.actionParam.sendEmail.subject), "%s", DfltCameraEventActionCfg.actionParam.sendEmail.subject);
                    snprintf(camEvtCfgV7_8.actionParam.sendEmail.message, sizeof(camEvtCfgV7_8.actionParam.sendEmail.message), "%s", DfltCameraEventActionCfg.actionParam.sendEmail.message);
                    snprintf(camEvtCfgV7_8.actionParam.sendTcp, sizeof(camEvtCfgV7_8.actionParam.sendTcp), "%s", DfltCameraEventActionCfg.actionParam.sendTcp);
                    snprintf(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber1, sizeof(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber1), "%s", DfltCameraEventActionCfg.actionParam.smsParameter.mobileNumber1);
                    snprintf(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber2, sizeof(camEvtCfgV7_8.actionParam.smsParameter.mobileNumber2), "%s", DfltCameraEventActionCfg.actionParam.smsParameter.mobileNumber2);
                    snprintf(camEvtCfgV7_8.actionParam.smsParameter.message, sizeof(camEvtCfgV7_8.actionParam.smsParameter.message), "%s", DfltCameraEventActionCfg.actionParam.smsParameter.message);
                    camEvtCfgV7_8.actionParam.gotoPosition = DfltCameraEventActionCfg.actionParam.gotoPosition;
                    memcpy(camEvtCfgV7_8.actionParam.systemAlarmOutput, DfltCameraEventActionCfg.actionParam.systemAlarmOutput, (sizeof(BOOL) * MAX_ALARM));
                    camEvtCfgV7_8.actionParam.cameraAlarmOutput = DfltCameraEventActionCfg.actionParam.cameraAlarmOutput;
                    memcpy(camEvtCfgV7_8.weeklySchedule, DfltCameraEventActionCfg.weeklySchedule, (sizeof(WEEKLY_ACTION_SCHEDULE_t) * MAX_WEEK_DAYS));
                    camEvtCfgV7_8.copyToCam = DfltCameraEventActionCfg.copyToCam.bitMask[0];
                }

                fileVersion++;
            }

            if (fileVersion == 8)
            {
                cameraEventCfg[camIdx][evtIdx].action = camEvtCfgV7_8.action;
                memcpy(&cameraEventCfg[camIdx][evtIdx].actionParam.alarmRecord[0], camEvtCfgV7_8.actionParam.alarmRecord, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                memcpy(&cameraEventCfg[camIdx][evtIdx].actionParam.alarmRecord[CAMERA_CONFIG_MAX_V1],
                       &DfltCameraEventActionCfg.actionParam.alarmRecord[CAMERA_CONFIG_MAX_V1], (sizeof(BOOL) * (MAX_CAMERA_CONFIG - CAMERA_CONFIG_MAX_V1)));
                memcpy(&cameraEventCfg[camIdx][evtIdx].actionParam.uploadImage[0], camEvtCfgV7_8.actionParam.uploadImage, (sizeof(BOOL) * CAMERA_CONFIG_MAX_V1));
                memcpy(&cameraEventCfg[camIdx][evtIdx].actionParam.uploadImage[CAMERA_CONFIG_MAX_V1],
                       &DfltCameraEventActionCfg.actionParam.uploadImage[CAMERA_CONFIG_MAX_V1], (sizeof(BOOL) * (MAX_CAMERA_CONFIG - CAMERA_CONFIG_MAX_V1)));
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.sendEmail.emailAddress, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.sendEmail.emailAddress), "%s", camEvtCfgV7_8.actionParam.sendEmail.emailAddress);
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.sendEmail.subject, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.sendEmail.subject), "%s", camEvtCfgV7_8.actionParam.sendEmail.subject);
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.sendEmail.message, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.sendEmail.message), "%s", camEvtCfgV7_8.actionParam.sendEmail.message);
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.sendTcp, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.sendTcp), "%s", camEvtCfgV7_8.actionParam.sendTcp);
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.smsParameter.mobileNumber1, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.smsParameter.mobileNumber1), "%s", camEvtCfgV7_8.actionParam.smsParameter.mobileNumber1);
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.smsParameter.mobileNumber2, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.smsParameter.mobileNumber2), "%s", camEvtCfgV7_8.actionParam.smsParameter.mobileNumber2);
                snprintf(cameraEventCfg[camIdx][evtIdx].actionParam.smsParameter.message, sizeof(cameraEventCfg[camIdx][evtIdx].actionParam.smsParameter.message), "%s", camEvtCfgV7_8.actionParam.smsParameter.message);
                cameraEventCfg[camIdx][evtIdx].actionParam.gotoPosition = camEvtCfgV7_8.actionParam.gotoPosition;
                memcpy(cameraEventCfg[camIdx][evtIdx].actionParam.systemAlarmOutput, camEvtCfgV7_8.actionParam.systemAlarmOutput, (sizeof(BOOL) * MAX_ALARM));
                cameraEventCfg[camIdx][evtIdx].actionParam.cameraAlarmOutput = camEvtCfgV7_8.actionParam.cameraAlarmOutput;
                memcpy(cameraEventCfg[camIdx][evtIdx].weeklySchedule, camEvtCfgV7_8.weeklySchedule, (sizeof(WEEKLY_ACTION_SCHEDULE_t) * MAX_WEEK_DAYS));
                cameraEventCfg[camIdx][evtIdx].copyToCam.bitMask[0] = camEvtCfgV7_8.copyToCam;
                cameraEventCfg[camIdx][evtIdx].copyToCam.bitMask[1] = DfltCameraEventActionCfg.copyToCam.bitMask[1];

                fileVersion++;
            }
        }
    }

    fileVersion = CAMERA_EVENT_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, cameraEventCfg, (sizeof(CAMERA_EVENT_CONFIG_t) * getMaxCameraForCurrentVariant() * MAX_CAMERA_EVENT)))
    {
        EPRINT(CONFIGURATION, "fail to write camera event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSystemEventAndActionConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                           cnt, tempCnt;
    SYSTEM_EVENT_CONFIG_VER3_t      systemEventCfgV3[MAX_SYSTEM_EVENT_VER5];
    SYSTEM_EVENT_CONFIG_VER4_t      systemEventCfgV4[MAX_SYSTEM_EVENT_VER5];
    SYSTEM_EVENT_CONFIG_VER5_6_t    systemEventCfgV5_6[MAX_SYSTEM_EVENT];

    switch(fileVersion)
    {
        case 3:
        {
            for(cnt = 0; cnt < MAX_SYSTEM_EVENT_VER5; cnt++)
            {
                if(sizeof(SYSTEM_EVENT_CONFIG_VER3_t) != read(fileFd, &systemEventCfgV3[cnt], sizeof(SYSTEM_EVENT_CONFIG_VER3_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read system event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 4:
        {
            for(cnt = 0; cnt < MAX_SYSTEM_EVENT_VER5; cnt++)
            {
                if(sizeof(SYSTEM_EVENT_CONFIG_VER4_t) != read(fileFd, &systemEventCfgV4[cnt], sizeof(SYSTEM_EVENT_CONFIG_VER4_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read system event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 5:
        {
            for(cnt = 0; cnt < MAX_SYSTEM_EVENT_VER5; cnt++)
            {
                if(sizeof(SYSTEM_EVENT_CONFIG_VER5_6_t) != read(fileFd, &systemEventCfgV5_6[cnt], sizeof(SYSTEM_EVENT_CONFIG_VER5_6_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read system event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 6:
        {
            for(cnt = 0; cnt < MAX_SYSTEM_EVENT; cnt++)
            {
                if(sizeof(SYSTEM_EVENT_CONFIG_VER5_6_t) != read(fileFd, &systemEventCfgV5_6[cnt], sizeof(SYSTEM_EVENT_CONFIG_VER5_6_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read system event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if(fileVersion == 3)
    {
        for(cnt = 0; cnt < MAX_SYSTEM_EVENT_VER5; cnt++)
        {
            systemEventCfgV4[cnt].action = systemEventCfgV3[cnt].action;

            memcpy(&systemEventCfgV4[cnt].actionBits, &systemEventCfgV3[cnt].actionBits, sizeof(ACTION_BIT_u));

            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                systemEventCfgV4[cnt].actionParam.alarmRecord[tempCnt] = systemEventCfgV3[cnt].actionParam.alarmRecord[tempCnt];
                systemEventCfgV4[cnt].actionParam.uploadImage[tempCnt] = systemEventCfgV3[cnt].actionParam.uploadImage[tempCnt];
            }

            snprintf(systemEventCfgV4[cnt].actionParam.sendTcp, sizeof(systemEventCfgV4[cnt].actionParam.sendTcp), "%s", systemEventCfgV3[cnt].actionParam.sendTcp);
            memcpy(&systemEventCfgV4[cnt].actionParam.cameraAlarmOutput, &systemEventCfgV3[cnt].actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));

            // copy EMAIL_PARAMETER_VER1_t into EMAIL_PARAMETER_t - change due to UTF-8 MultiLanguage
            snprintf(systemEventCfgV4[cnt].actionParam.sendEmail.emailAddress, sizeof(systemEventCfgV4[cnt].actionParam.sendEmail.emailAddress), "%s", systemEventCfgV3[cnt].actionParam.sendEmail.emailAddress);
            snprintf(systemEventCfgV4[cnt].actionParam.sendEmail.subject, sizeof(systemEventCfgV4[cnt].actionParam.sendEmail.subject), "%s", systemEventCfgV3[cnt].actionParam.sendEmail.subject);
            snprintf(systemEventCfgV4[cnt].actionParam.sendEmail.message, sizeof(systemEventCfgV4[cnt].actionParam.sendEmail.message), "%s", systemEventCfgV3[cnt].actionParam.sendEmail.message);
            memcpy(&systemEventCfgV4[cnt].actionParam.gotoPosition, &systemEventCfgV3[cnt].actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));

            // copy SMS_PARAMTER_VER1_t into SMS_PARAMTER_t - change due to UTF-8 MultiLanguage
            snprintf(systemEventCfgV4[cnt].actionParam.smsParameter.mobileNumber1, sizeof(systemEventCfgV4[cnt].actionParam.smsParameter.mobileNumber1), "%s", systemEventCfgV3[cnt].actionParam.smsParameter.mobileNumber1);
            snprintf(systemEventCfgV4[cnt].actionParam.smsParameter.mobileNumber2, sizeof(systemEventCfgV4[cnt].actionParam.smsParameter.mobileNumber2), "%s", systemEventCfgV3[cnt].actionParam.smsParameter.mobileNumber2);
            snprintf(systemEventCfgV4[cnt].actionParam.smsParameter.message, sizeof(systemEventCfgV4[cnt].actionParam.smsParameter.message), "%s", systemEventCfgV3[cnt].actionParam.smsParameter.message);

            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                systemEventCfgV4[cnt].actionParam.systemAlarmOutput[tempCnt] = systemEventCfgV3[cnt].actionParam.systemAlarmOutput[tempCnt];
            }
        }
        fileVersion++;
    }

    if(fileVersion == 4)
    {
        for(cnt = 0; cnt < MAX_SYSTEM_EVENT_VER5; cnt++)
        {
            systemEventCfgV5_6[cnt].action = systemEventCfgV4[cnt].action;

            memcpy(&systemEventCfgV5_6[cnt].actionBits, &systemEventCfgV4[cnt].actionBits, sizeof(ACTION_BIT_u));

            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                systemEventCfgV5_6[cnt].actionParam.alarmRecord[tempCnt] = systemEventCfgV4[cnt].actionParam.alarmRecord[tempCnt];
                systemEventCfgV5_6[cnt].actionParam.uploadImage[tempCnt] = systemEventCfgV4[cnt].actionParam.uploadImage[tempCnt];
            }

            snprintf(systemEventCfgV5_6[cnt].actionParam.sendTcp, sizeof(systemEventCfgV5_6[cnt].actionParam.sendTcp), "%s", systemEventCfgV4[cnt].actionParam.sendTcp);
            memcpy(&systemEventCfgV5_6[cnt].actionParam.cameraAlarmOutput, &systemEventCfgV4[cnt].actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));
            snprintf(systemEventCfgV5_6[cnt].actionParam.sendEmail.emailAddress, sizeof(systemEventCfgV5_6[cnt].actionParam.sendEmail.emailAddress), "%s", systemEventCfgV4[cnt].actionParam.sendEmail.emailAddress);
            snprintf(systemEventCfgV5_6[cnt].actionParam.sendEmail.subject, sizeof(systemEventCfgV5_6[cnt].actionParam.sendEmail.subject), "%s", systemEventCfgV4[cnt].actionParam.sendEmail.subject);
            snprintf(systemEventCfgV5_6[cnt].actionParam.sendEmail.message, sizeof(systemEventCfgV5_6[cnt].actionParam.sendEmail.message), "%s", systemEventCfgV4[cnt].actionParam.sendEmail.message);
            memcpy(&systemEventCfgV5_6[cnt].actionParam.gotoPosition, &systemEventCfgV4[cnt].actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));
            snprintf(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber1, sizeof(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber1), "%s", systemEventCfgV4[cnt].actionParam.smsParameter.mobileNumber1);
            snprintf(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber2, sizeof(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber2), "%s", systemEventCfgV4[cnt].actionParam.smsParameter.mobileNumber2);
            snprintf(systemEventCfgV5_6[cnt].actionParam.smsParameter.message, sizeof(systemEventCfgV5_6[cnt].actionParam.smsParameter.message), "%s", systemEventCfgV4[cnt].actionParam.smsParameter.message);
            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                systemEventCfgV5_6[cnt].actionParam.systemAlarmOutput[tempCnt] = systemEventCfgV4[cnt].actionParam.systemAlarmOutput[tempCnt];
            }
        }
        fileVersion++;
    }

    if(fileVersion == 5)
    {
        for(cnt = MAX_SYSTEM_EVENT_VER5; cnt < MAX_SYSTEM_EVENT; cnt++)
        {
            systemEventCfgV5_6[cnt].action = DfltSystemEventActionCfg.action;

            memcpy(&systemEventCfgV5_6[cnt].actionBits, &DfltSystemEventActionCfg.actionBits, sizeof(ACTION_BIT_u));

            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                systemEventCfgV5_6[cnt].actionParam.alarmRecord[tempCnt] = DfltSystemEventActionCfg.actionParam.alarmRecord[tempCnt];
                systemEventCfgV5_6[cnt].actionParam.uploadImage[tempCnt] = DfltSystemEventActionCfg.actionParam.uploadImage[tempCnt];
            }

            snprintf(systemEventCfgV5_6[cnt].actionParam.sendTcp, sizeof(systemEventCfgV5_6[cnt].actionParam.sendTcp), "%s", DfltSystemEventActionCfg.actionParam.sendTcp);
            memcpy(&systemEventCfgV5_6[cnt].actionParam.cameraAlarmOutput, &DfltSystemEventActionCfg.actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));
            snprintf(systemEventCfgV5_6[cnt].actionParam.sendEmail.emailAddress, sizeof(systemEventCfgV5_6[cnt].actionParam.sendEmail.emailAddress), "%s", DfltSystemEventActionCfg.actionParam.sendEmail.emailAddress);
            snprintf(systemEventCfgV5_6[cnt].actionParam.sendEmail.subject, sizeof(systemEventCfgV5_6[cnt].actionParam.sendEmail.subject), "%s", DfltSystemEventActionCfg.actionParam.sendEmail.subject);
            snprintf(systemEventCfgV5_6[cnt].actionParam.sendEmail.message, sizeof(systemEventCfgV5_6[cnt].actionParam.sendEmail.message), "%s", DfltSystemEventActionCfg.actionParam.sendEmail.message);
            memcpy(&systemEventCfgV5_6[cnt].actionParam.gotoPosition, &DfltSystemEventActionCfg.actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));
            snprintf(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber1, sizeof(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber1), "%s", DfltSystemEventActionCfg.actionParam.smsParameter.mobileNumber1);
            snprintf(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber2, sizeof(systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber2), "%s", DfltSystemEventActionCfg.actionParam.smsParameter.mobileNumber2);
            snprintf(systemEventCfgV5_6[cnt].actionParam.smsParameter.message, sizeof(systemEventCfgV5_6[cnt].actionParam.smsParameter.message), "%s", DfltSystemEventActionCfg.actionParam.smsParameter.message);
            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                systemEventCfgV5_6[cnt].actionParam.systemAlarmOutput[tempCnt] = DfltSystemEventActionCfg.actionParam.systemAlarmOutput[tempCnt];
            }
        }
        fileVersion++;
    }

    if(fileVersion == 6)
    {
        for(cnt = 0; cnt < MAX_SYSTEM_EVENT; cnt++)
        {
            systemEventCfg[cnt].action = systemEventCfgV5_6[cnt].action;
            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                systemEventCfg[cnt].actionParam.alarmRecord[tempCnt] = systemEventCfgV5_6[cnt].actionParam.alarmRecord[tempCnt];
                systemEventCfg[cnt].actionParam.uploadImage[tempCnt] = systemEventCfgV5_6[cnt].actionParam.uploadImage[tempCnt];
            }

            for(tempCnt = CAMERA_CONFIG_MAX_V1; tempCnt < MAX_CAMERA_CONFIG; tempCnt++)
            {
                systemEventCfg[cnt].actionParam.alarmRecord[tempCnt] = DfltSystemEventActionCfg.actionParam.alarmRecord[tempCnt];
                systemEventCfg[cnt].actionParam.uploadImage[tempCnt] = DfltSystemEventActionCfg.actionParam.uploadImage[tempCnt];
            }

            snprintf(systemEventCfg[cnt].actionParam.sendEmail.emailAddress, sizeof(systemEventCfg[cnt].actionParam.sendEmail.emailAddress), "%s", systemEventCfgV5_6[cnt].actionParam.sendEmail.emailAddress);
            snprintf(systemEventCfg[cnt].actionParam.sendEmail.subject, sizeof(systemEventCfg[cnt].actionParam.sendEmail.subject), "%s", systemEventCfgV5_6[cnt].actionParam.sendEmail.subject);
            snprintf(systemEventCfg[cnt].actionParam.sendEmail.message, sizeof(systemEventCfg[cnt].actionParam.sendEmail.message), "%s", systemEventCfgV5_6[cnt].actionParam.sendEmail.message);
            snprintf(systemEventCfg[cnt].actionParam.sendTcp, sizeof(systemEventCfg[cnt].actionParam.sendTcp), "%s", systemEventCfgV5_6[cnt].actionParam.sendTcp);
            snprintf(systemEventCfg[cnt].actionParam.smsParameter.mobileNumber1, sizeof(systemEventCfg[cnt].actionParam.smsParameter.mobileNumber1), "%s", systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber1);
            snprintf(systemEventCfg[cnt].actionParam.smsParameter.mobileNumber2, sizeof(systemEventCfg[cnt].actionParam.smsParameter.mobileNumber2), "%s", systemEventCfgV5_6[cnt].actionParam.smsParameter.mobileNumber2);
            snprintf(systemEventCfg[cnt].actionParam.smsParameter.message, sizeof(systemEventCfg[cnt].actionParam.smsParameter.message), "%s", systemEventCfgV5_6[cnt].actionParam.smsParameter.message);
            memcpy(&systemEventCfg[cnt].actionParam.gotoPosition, &systemEventCfgV5_6[cnt].actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));
            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                systemEventCfg[cnt].actionParam.systemAlarmOutput[tempCnt] = systemEventCfgV5_6[cnt].actionParam.systemAlarmOutput[tempCnt];
            }
            memcpy(&systemEventCfg[cnt].actionParam.cameraAlarmOutput, &systemEventCfgV5_6[cnt].actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));
            memcpy(&systemEventCfg[cnt].actionBits, &systemEventCfgV5_6[cnt].actionBits, sizeof(ACTION_BIT_u));
        }
        fileVersion++;
    }

    fileVersion = SYSTEM_EVENT_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, systemEventCfg, (sizeof(SYSTEM_EVENT_CONFIG_t) * MAX_SYSTEM_EVENT)))
    {
        EPRINT(CONFIGURATION, "fail to write system event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldBroadBandConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                       cnt;
    BROAD_BAND_CONFIG_VER2_t    broadBandCfg2;
    BROAD_BAND_CONFIG_VER3_t    broadBandCfg3;

    switch(fileVersion)
    {
        case 2:
        {
            if((sizeof(BROAD_BAND_CONFIG_VER2_t)) != read(fileFd, &broadBandCfg2, sizeof(BROAD_BAND_CONFIG_VER2_t)))
            {
                return FAIL;
            }
        }
        break;

        case 3:
        {
            if((sizeof(BROAD_BAND_CONFIG_VER3_t)) != read(fileFd, &broadBandCfg3, sizeof(BROAD_BAND_CONFIG_VER3_t)))
            {
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 2)
    {
        broadBandCfg3.activeProfile = broadBandCfg2.activeProfile;
        for(cnt = 0; cnt < MAX_BROADBAND_PROFILE; cnt++)
        {
            snprintf(broadBandCfg3.broadBandCfg[cnt].profileName, sizeof(broadBandCfg3.broadBandCfg[cnt].profileName), "%s", broadBandCfg2.broadBandCfg[cnt].profileName);
            snprintf(broadBandCfg3.broadBandCfg[cnt].dialNumber, sizeof(broadBandCfg3.broadBandCfg[cnt].dialNumber), "%s", broadBandCfg2.broadBandCfg[cnt].dialNumber);
            snprintf(broadBandCfg3.broadBandCfg[cnt].userName, sizeof(broadBandCfg3.broadBandCfg[cnt].userName), "%s", broadBandCfg2.broadBandCfg[cnt].userName);
            snprintf(broadBandCfg3.broadBandCfg[cnt].password, sizeof(broadBandCfg3.broadBandCfg[cnt].password), "%s", broadBandCfg2.broadBandCfg[cnt].password);
            broadBandCfg3.broadBandCfg[cnt].serviceType = broadBandCfg2.broadBandCfg[cnt].serviceType;
            snprintf(broadBandCfg3.broadBandCfg[cnt].apn, sizeof(broadBandCfg3.broadBandCfg[cnt].apn), "%s", broadBandCfg2.broadBandCfg[cnt].apn);
        }

        fileVersion++;
    }

    if (fileVersion == 3)
    {
        broadBandCfg.activeProfile = broadBandCfg3.activeProfile;
        for(cnt = 0; cnt < MAX_BROADBAND_PROFILE; cnt++)
        {
            if (broadBandCfg3.broadBandCfg[cnt].serviceType != GSM_MOBILE_SERVICE)
            {
                broadBandCfg.broadBandCfg[cnt] = DfltBroadBandCfg.broadBandCfg[cnt];
            }
            else
            {
                if (strcmp(broadBandCfg3.broadBandCfg[cnt].profileName, "BSNL") == 0)
                {
                    if (strcmp(broadBandCfg3.broadBandCfg[cnt].dialNumber, "*99***1#") == 0)
                    {
                        snprintf(broadBandCfg3.broadBandCfg[cnt].dialNumber, sizeof(broadBandCfg3.broadBandCfg[cnt].dialNumber), "%s", DfltBroadBandCfg.broadBandCfg[1].dialNumber);
                    }
                }
                else if (strcmp(broadBandCfg3.broadBandCfg[cnt].profileName, "Vodafone") == 0)
                {
                    snprintf(broadBandCfg3.broadBandCfg[cnt].profileName, sizeof(broadBandCfg3.broadBandCfg[cnt].profileName), "%s", DfltBroadBandCfg.broadBandCfg[2].profileName);
                }
                else if (strcmp(broadBandCfg3.broadBandCfg[cnt].profileName, "TATA Photon+") == 0)
                {
                    snprintf(broadBandCfg3.broadBandCfg[cnt].profileName, sizeof(broadBandCfg3.broadBandCfg[cnt].profileName), "%s", DfltBroadBandCfg.broadBandCfg[3].profileName);
                }
                else if (strcmp(broadBandCfg3.broadBandCfg[cnt].profileName, "Reliance") == 0)
                {
                    snprintf(broadBandCfg3.broadBandCfg[cnt].profileName, sizeof(broadBandCfg3.broadBandCfg[cnt].profileName), "%s", DfltBroadBandCfg.broadBandCfg[4].profileName);
                }

                snprintf(broadBandCfg.broadBandCfg[cnt].profileName, sizeof(broadBandCfg.broadBandCfg[cnt].profileName), "%s", broadBandCfg3.broadBandCfg[cnt].profileName);
                snprintf(broadBandCfg.broadBandCfg[cnt].dialNumber, sizeof(broadBandCfg.broadBandCfg[cnt].dialNumber), "%s", broadBandCfg3.broadBandCfg[cnt].dialNumber);
                snprintf(broadBandCfg.broadBandCfg[cnt].userName, sizeof(broadBandCfg.broadBandCfg[cnt].userName), "%s", broadBandCfg3.broadBandCfg[cnt].userName);
                snprintf(broadBandCfg.broadBandCfg[cnt].password, sizeof(broadBandCfg.broadBandCfg[cnt].password), "%s", broadBandCfg3.broadBandCfg[cnt].password);
                snprintf(broadBandCfg.broadBandCfg[cnt].apn, sizeof(broadBandCfg.broadBandCfg[cnt].apn), "%s", broadBandCfg3.broadBandCfg[cnt].apn);
            }
        }

        fileVersion++;
    }

    fileVersion = BROAD_BAND_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &broadBandCfg, sizeof(BROAD_BAND_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write broadband config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSMSConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            SMS_CONFIG_VER1_t smsCfg1;

            if((sizeof(SMS_CONFIG_VER1_t)) != read(fileFd, &smsCfg1, sizeof(SMS_CONFIG_VER1_t)))
            {
                return FAIL;
            }

            // copy old SMS parameter into new (UTf-8) variables
            smsCfg.mode = smsCfg1.mode;
            smsCfg.smsServer = smsCfg1.smsServer;
            snprintf(smsCfg.userName, sizeof(smsCfg.userName), "%s", smsCfg1.userName);
            snprintf(smsCfg.password, sizeof(smsCfg.password), "%s", smsCfg1.password);
            snprintf(smsCfg.sendorId, sizeof(smsCfg.sendorId), "%s", smsCfg1.sendorId);
            smsCfg.sendAsFlash = smsCfg1.sendAsFlash;
        }
        break;

        case 2:
        {
            SMS_CONFIG_VER2_t smsCfgV2;

            if((sizeof(SMS_CONFIG_VER2_t)) != read(fileFd, &smsCfgV2, sizeof(SMS_CONFIG_VER2_t)))
            {
                return FAIL;
            }

            smsCfg.mode = smsCfgV2.mode;
            smsCfg.smsServer = smsCfgV2.smsServer;
            snprintf(smsCfg.userName, sizeof(smsCfg.userName), "%s", smsCfgV2.userName);
            snprintf(smsCfg.password, sizeof(smsCfg.password), "%s", smsCfgV2.password);
            snprintf(smsCfg.sendorId, sizeof(smsCfg.sendorId), "%s", smsCfgV2.sendorId);
            smsCfg.sendAsFlash = smsCfgV2.sendAsFlash;
        }
        break;
        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = SMS_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &smsCfg, sizeof(SMS_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write sms config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldNetworkDriveConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                           cnt;
    NETWORK_DRIVE_CONFIG_VER2_t     networkDrvCfg2[MAX_NW_DRIVE];
    NETWORK_DRIVE_CONFIG_VER3_t     networkDrvCfg3[MAX_NW_DRIVE];

    switch(fileVersion)
    {
        case 2:
        {
            for(cnt = 0; cnt < MAX_NW_DRIVE; cnt++)
            {
                if((sizeof(NETWORK_DRIVE_CONFIG_VER2_t)) != read(fileFd, &networkDrvCfg2[cnt], (sizeof(NETWORK_DRIVE_CONFIG_VER2_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read ndd config file: [ndd=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 3:
        {
            for(cnt = 0; cnt < MAX_NW_DRIVE; cnt++)
            {
                if((sizeof(NETWORK_DRIVE_CONFIG_VER3_t)) != read(fileFd, &networkDrvCfg3[cnt], (sizeof(NETWORK_DRIVE_CONFIG_VER3_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read ndd config file: [ndd=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 2)
    {
        for(cnt = 0; cnt < MAX_NW_DRIVE; cnt++)
        {
            networkDrvCfg3[cnt].enable = networkDrvCfg2[cnt].enable;
            networkDrvCfg3[cnt].fileSys = networkDrvCfg2[cnt].fileSys;
            snprintf(networkDrvCfg3[cnt].dfltFolder, sizeof(networkDrvCfg3[cnt].dfltFolder), "%s", networkDrvCfg2[cnt].dfltFolder);
            snprintf(networkDrvCfg3[cnt].ipAddr, sizeof(networkDrvCfg3[cnt].ipAddr), "%s", networkDrvCfg2[cnt].ipAddr);
            snprintf(networkDrvCfg3[cnt].name, sizeof(networkDrvCfg3[cnt].name), "%s", networkDrvCfg2[cnt].name);
            snprintf(networkDrvCfg3[cnt].userName, sizeof(networkDrvCfg3[cnt].userName), "%s", networkDrvCfg2[cnt].userName);
            snprintf(networkDrvCfg3[cnt].password, sizeof(networkDrvCfg3[cnt].password), "%s", networkDrvCfg2[cnt].password);
        }

        fileVersion++;
    }


    if (fileVersion == 3)
    {
        for(cnt = 0; cnt < MAX_NW_DRIVE; cnt++)
        {
            nwDriveCfg[cnt].enable = networkDrvCfg3[cnt].enable;
            nwDriveCfg[cnt].fileSys = networkDrvCfg3[cnt].fileSys;
            snprintf(nwDriveCfg[cnt].dfltFolder, sizeof(nwDriveCfg[cnt].dfltFolder), "%s", networkDrvCfg3[cnt].dfltFolder);
            snprintf(nwDriveCfg[cnt].ipAddr, sizeof(nwDriveCfg[cnt].ipAddr), "%s", networkDrvCfg3[cnt].ipAddr);
            snprintf(nwDriveCfg[cnt].name, sizeof(nwDriveCfg[cnt].name), "%s", networkDrvCfg3[cnt].name);
            snprintf(nwDriveCfg[cnt].userName, sizeof(nwDriveCfg[cnt].userName), "%s", networkDrvCfg3[cnt].userName);
            snprintf(nwDriveCfg[cnt].password, sizeof(nwDriveCfg[cnt].password), "%s", networkDrvCfg3[cnt].password);
        }

        fileVersion++;
    }

    fileVersion = NETWORK_DRIVE_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, nwDriveCfg, (sizeof(NETWORK_DRIVE_CONFIG_t) * MAX_NW_DRIVE)))
    {
        EPRINT(CONFIGURATION, "fail to write ndd config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldIpCameraConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                   cnt;
    IP_CAMERA_CONFIG_VER4_t ipCamV4Config[MAX_CAMERA];
    IP_CAMERA_CONFIG_VER5_t ipCamV5Config[MAX_CAMERA];
    IP_CAMERA_CONFIG_VER6_t ipCamV6Config[MAX_CAMERA];
    IP_CAMERA_CONFIG_VER7_t ipCamV7Config[MAX_CAMERA];
    IP_CAMERA_CONFIG_VER8_t ipCamV8Config[MAX_CAMERA];
    IP_CAMERA_CONFIG_VER9_t ipCamV9Config[MAX_CAMERA];

    switch(fileVersion)
    {
        case 4:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((sizeof(IP_CAMERA_CONFIG_VER4_t)) != read(fileFd, &ipCamV4Config[cnt], (sizeof(IP_CAMERA_CONFIG_VER4_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read ip camera config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 5:
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((sizeof(IP_CAMERA_CONFIG_VER5_t)) != read(fileFd, &ipCamV5Config[cnt], (sizeof(IP_CAMERA_CONFIG_VER5_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read ip camera config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 6:
        {
            for (cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if (sizeof(IP_CAMERA_CONFIG_VER6_t) != read(fileFd, &ipCamV6Config[cnt], sizeof(IP_CAMERA_CONFIG_VER6_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read ip camera config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 7:
        {
            for (cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if (sizeof(IP_CAMERA_CONFIG_VER7_t) != read(fileFd, &ipCamV7Config[cnt], sizeof(IP_CAMERA_CONFIG_VER7_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read ip camera config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 8:
        {
            for (cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if (sizeof(IP_CAMERA_CONFIG_VER8_t) != read(fileFd, &ipCamV8Config[cnt], sizeof(IP_CAMERA_CONFIG_VER8_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read ip camera config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 9:
        {
            for (cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if (sizeof(IP_CAMERA_CONFIG_VER9_t) != read(fileFd, &ipCamV9Config[cnt], sizeof(IP_CAMERA_CONFIG_VER9_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read ip camera config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if (fileVersion == 4)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            /* Mac address added at last in the V5 structure */
            memcpy(&ipCamV5Config[cnt], &ipCamV4Config[cnt], sizeof(IP_CAMERA_CONFIG_VER4_t));

            /* default all configuration which are not part of version 3/4 */
            snprintf(ipCamV5Config[cnt].macAddr, sizeof(ipCamV5Config[cnt].macAddr), "%s", DfltIpCameraCfg.macAddr);
        }
        fileVersion = 5;
    }

    if (fileVersion == 5)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            /* User defined string length updated for multi-language support */
            snprintf(ipCamV6Config[cnt].brand, sizeof(ipCamV6Config[cnt].brand), "%s", ipCamV5Config[cnt].brand);
            snprintf(ipCamV6Config[cnt].model, sizeof(ipCamV6Config[cnt].model), "%s", ipCamV5Config[cnt].model);
            snprintf(ipCamV6Config[cnt].url, sizeof(ipCamV6Config[cnt].url), "%s", ipCamV5Config[cnt].url);
            snprintf(ipCamV6Config[cnt].cameraAddress, sizeof(ipCamV6Config[cnt].cameraAddress), "%s", ipCamV5Config[cnt].cameraAddress);
            ipCamV6Config[cnt].httpPort = ipCamV5Config[cnt].httpPort;
            ipCamV6Config[cnt].rtspPort = ipCamV5Config[cnt].rtspPort;
            ipCamV6Config[cnt].onvifPort = ipCamV5Config[cnt].onvifPort;
            ipCamV6Config[cnt].accessThroughNat = ipCamV5Config[cnt].accessThroughNat;
            snprintf(ipCamV6Config[cnt].routerAddress, sizeof(ipCamV6Config[cnt].routerAddress), "%s", ipCamV5Config[cnt].routerAddress);
            ipCamV6Config[cnt].forwardedHttpPort = ipCamV5Config[cnt].forwardedHttpPort;
            ipCamV6Config[cnt].forawrdedRtspPort = ipCamV5Config[cnt].forawrdedRtspPort;
            ipCamV6Config[cnt].forwardedOnvifPort = ipCamV5Config[cnt].forwardedOnvifPort;
            snprintf(ipCamV6Config[cnt].username, sizeof(ipCamV6Config[cnt].username), "%s", ipCamV5Config[cnt].username);
            snprintf(ipCamV6Config[cnt].password, sizeof(ipCamV6Config[cnt].password), "%s", ipCamV5Config[cnt].password);
            ipCamV6Config[cnt].onvifSupportF = ipCamV5Config[cnt].onvifSupportF;
            ipCamV6Config[cnt].rtspProtocol = ipCamV5Config[cnt].rtspProtocol;
            snprintf(ipCamV6Config[cnt].macAddr, sizeof(ipCamV6Config[cnt].macAddr), "%s", ipCamV5Config[cnt].macAddr);
        }
        fileVersion = 6;
    }

    if (fileVersion == 6)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            /* If in previous version camera brand is any other except matrix and axis then default all configuration of that camera */
            if ((ipCamV6Config[cnt].onvifSupportF == DISABLE) && (strcmp(ipCamV6Config[cnt].brand, MATRIX_BRAND_NAME) != STATUS_OK)
                    && (strcmp(ipCamV6Config[cnt].brand, "AXIS") != STATUS_OK) && (strcmp(ipCamV6Config[cnt].brand, "GENERIC") != STATUS_OK))
            {
                /* Set default status for this camera to default all configuration */
                cameraConfigDefaultF[cnt] = TRUE;
            }

            /* If in previous version access through NAT was enable, copy configurations */
            if (ipCamV6Config[cnt].accessThroughNat == TRUE)
            {
                snprintf(ipCamV7Config[cnt].cameraAddress, sizeof(ipCamV7Config[cnt].cameraAddress), "%s", ipCamV6Config[cnt].routerAddress);
                ipCamV7Config[cnt].httpPort = ipCamV6Config[cnt].forwardedHttpPort;
                ipCamV7Config[cnt].rtspPort = ipCamV6Config[cnt].forawrdedRtspPort;
                ipCamV7Config[cnt].onvifPort = ipCamV6Config[cnt].forwardedOnvifPort;
            }
            else
            {
                snprintf(ipCamV7Config[cnt].cameraAddress, sizeof(ipCamV7Config[cnt].cameraAddress), "%s", ipCamV6Config[cnt].cameraAddress);
                ipCamV7Config[cnt].httpPort = ipCamV6Config[cnt].httpPort;
                ipCamV7Config[cnt].rtspPort = ipCamV6Config[cnt].rtspPort;
                ipCamV7Config[cnt].onvifPort = ipCamV6Config[cnt].onvifPort;
            }

            snprintf(ipCamV7Config[cnt].brand, sizeof(ipCamV7Config[cnt].brand), "%s", ipCamV6Config[cnt].brand);
            snprintf(ipCamV7Config[cnt].model, sizeof(ipCamV7Config[cnt].model), "%s", ipCamV6Config[cnt].model);
            snprintf(ipCamV7Config[cnt].url, sizeof(ipCamV7Config[cnt].url), "%s", ipCamV6Config[cnt].url);
            snprintf(ipCamV7Config[cnt].username, sizeof(ipCamV7Config[cnt].username), "%s", ipCamV6Config[cnt].username);
            snprintf(ipCamV7Config[cnt].password, sizeof(ipCamV7Config[cnt].password), "%s", ipCamV6Config[cnt].password);
            ipCamV7Config[cnt].onvifSupportF = ipCamV6Config[cnt].onvifSupportF;
            ipCamV7Config[cnt].rtspProtocol = ipCamV6Config[cnt].rtspProtocol;
            snprintf(ipCamV7Config[cnt].macAddr, sizeof(ipCamV7Config[cnt].macAddr), "%s", ipCamV6Config[cnt].macAddr);
        }
        fileVersion = 7;
    }

    if (fileVersion == 7)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            /* Camera password length increased from 16 to 20 */
            snprintf(ipCamV8Config[cnt].cameraAddress, sizeof(ipCamV8Config[cnt].cameraAddress), "%s", ipCamV7Config[cnt].cameraAddress);
            ipCamV8Config[cnt].httpPort = ipCamV7Config[cnt].httpPort;
            ipCamV8Config[cnt].rtspPort = ipCamV7Config[cnt].rtspPort;
            ipCamV8Config[cnt].onvifPort = ipCamV7Config[cnt].onvifPort;
            snprintf(ipCamV8Config[cnt].brand, sizeof(ipCamV8Config[cnt].brand), "%s", ipCamV7Config[cnt].brand);
            snprintf(ipCamV8Config[cnt].model, sizeof(ipCamV8Config[cnt].model), "%s", ipCamV7Config[cnt].model);
            snprintf(ipCamV8Config[cnt].url, sizeof(ipCamV8Config[cnt].url), "%s", ipCamV7Config[cnt].url);
            snprintf(ipCamV8Config[cnt].username, sizeof(ipCamV8Config[cnt].username), "%s", ipCamV7Config[cnt].username);
            snprintf(ipCamV8Config[cnt].password, sizeof(ipCamV8Config[cnt].password), "%s", ipCamV7Config[cnt].password);
            ipCamV8Config[cnt].onvifSupportF = ipCamV7Config[cnt].onvifSupportF;
            ipCamV8Config[cnt].rtspProtocol = ipCamV7Config[cnt].rtspProtocol;
            snprintf(ipCamV8Config[cnt].macAddr, sizeof(ipCamV8Config[cnt].macAddr), "%s", ipCamV7Config[cnt].macAddr);
        }
        fileVersion = 8;
    }

    if (fileVersion == 8)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            /* Matrix camera model name correction */
            snprintf(ipCamV9Config[cnt].cameraAddress, sizeof(ipCamV9Config[cnt].cameraAddress), "%s", ipCamV8Config[cnt].cameraAddress);
            ipCamV9Config[cnt].httpPort = ipCamV8Config[cnt].httpPort;
            ipCamV9Config[cnt].rtspPort = ipCamV8Config[cnt].rtspPort;
            ipCamV9Config[cnt].onvifPort = ipCamV8Config[cnt].onvifPort;

            /* Correct the matrix camera model name which added through BM or CI */
            if ((ipCamV8Config[cnt].onvifSupportF == DISABLE) &&
                    ((cameraCfg[cnt].type == AUTO_ADDED_CAMERA) || (strcmp(ipCamV8Config[cnt].brand, MATRIX_BRAND_NAME) == 0)))
            {
                /* Update matrix camera model name if found in old camera list */
                GetUpdatedMatrixCameraModelName(ipCamV8Config[cnt].model, sizeof(ipCamV8Config[cnt].model));
            }

            snprintf(ipCamV9Config[cnt].brand, sizeof(ipCamV9Config[cnt].brand), "%s", ipCamV8Config[cnt].brand);
            snprintf(ipCamV9Config[cnt].model, sizeof(ipCamV9Config[cnt].model), "%s", ipCamV8Config[cnt].model);
            snprintf(ipCamV9Config[cnt].url, sizeof(ipCamV9Config[cnt].url), "%s", ipCamV8Config[cnt].url);
            snprintf(ipCamV9Config[cnt].username, sizeof(ipCamV9Config[cnt].username), "%s", ipCamV8Config[cnt].username);
            snprintf(ipCamV9Config[cnt].password, sizeof(ipCamV9Config[cnt].password), "%s", ipCamV8Config[cnt].password);
            ipCamV9Config[cnt].onvifSupportF = ipCamV8Config[cnt].onvifSupportF;
            ipCamV9Config[cnt].rtspProtocol = ipCamV8Config[cnt].rtspProtocol;
            snprintf(ipCamV9Config[cnt].macAddr, sizeof(ipCamV9Config[cnt].macAddr), "%s", ipCamV8Config[cnt].macAddr);
        }
        fileVersion = 9;
    }

    if (fileVersion == 9)
    {
        for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
        {
            /* If in previous version camera brand is any other except matrix then default all configuration of that camera */
            if ((ipCamV9Config[cnt].onvifSupportF == DISABLE) &&
                    (strcmp(ipCamV9Config[cnt].brand, MATRIX_BRAND_NAME) != STATUS_OK) && (strcmp(ipCamV6Config[cnt].brand, "GENERIC") != STATUS_OK))
            {
                /* Set default status for this camera to default all configuration */
                cameraConfigDefaultF[cnt] = TRUE;
            }

            /* Copy whole configuration of camera */
            memcpy(&ipCameraCfg[cnt], &ipCamV9Config[cnt], sizeof(ipCamV9Config[cnt]));
        }
        fileVersion = 10;
    }

    fileVersion = IP_CAMERA_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, ipCameraCfg, (sizeof(IP_CAMERA_CONFIG_t) * getMaxCameraForCurrentVariant())))
    {
        EPRINT(CONFIGURATION, "fail to write ip camera config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSensorEventAndActionConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                       cnt, tempCnt;
    SENSOR_EVENT_CONFIG_VER2_t  sensorEventCfgV2[MAX_SENSOR_EVENT];
    SENSOR_EVENT_CONFIG_VER3_t  sensorEventCfgV3[MAX_SENSOR_EVENT];
    SENSOR_EVENT_CONFIG_VER4_t  sensorEventCfgV4[MAX_SENSOR_EVENT];

    switch(fileVersion)
    {
        case 2:
        {
            for(cnt = 0; cnt < MAX_SENSOR_EVENT; cnt++)
            {
                if((sizeof(SENSOR_EVENT_CONFIG_VER2_t)) != read(fileFd, &sensorEventCfgV2[cnt], (sizeof(SENSOR_EVENT_CONFIG_VER2_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read sensor event config file: [sensor=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 3:
        {
            for(cnt = 0; cnt < MAX_SENSOR_EVENT; cnt++)
            {
                if((sizeof(SENSOR_EVENT_CONFIG_VER3_t)) != read(fileFd, &sensorEventCfgV3[cnt], (sizeof(SENSOR_EVENT_CONFIG_VER3_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read sensor event config file: [sensor=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        case 4:
        {
            for(cnt = 0; cnt < MAX_SENSOR_EVENT; cnt++)
            {
                if((sizeof(SENSOR_EVENT_CONFIG_VER4_t)) != read(fileFd, &sensorEventCfgV4[cnt], (sizeof(SENSOR_EVENT_CONFIG_VER4_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read sensor event config file: [sensor=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    if(fileVersion == 2)
    {
        for(cnt = 0; cnt < MAX_SENSOR_EVENT; cnt++)
        {
            sensorEventCfgV3[cnt].action = sensorEventCfgV2[cnt].action;
            for(tempCnt = 0; tempCnt < MAX_WEEK_DAYS; tempCnt++)
            {
                memcpy(&sensorEventCfgV3[cnt].weeklySchedule[tempCnt], &sensorEventCfgV2[cnt].weeklySchedule[tempCnt], sizeof(WEEKLY_ACTION_SCHEDULE_t));
            }

            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                sensorEventCfgV3[cnt].actionParam.alarmRecord[tempCnt] = sensorEventCfgV2[cnt].actionParam.alarmRecord[tempCnt];
                sensorEventCfgV3[cnt].actionParam.uploadImage[tempCnt] = sensorEventCfgV2[cnt].actionParam.uploadImage[tempCnt];
            }

            snprintf(sensorEventCfgV3[cnt].actionParam.sendTcp, sizeof(sensorEventCfgV3[cnt].actionParam.sendTcp), "%s", sensorEventCfgV2[cnt].actionParam.sendTcp);

            // copy EMAIL_PARAMETER_VER1_t into EMAIL_PARAMETER_t - change due to UTF-8 MultiLanguage
            snprintf(sensorEventCfgV3[cnt].actionParam.sendEmail.emailAddress, sizeof(sensorEventCfgV3[cnt].actionParam.sendEmail.emailAddress), "%s", sensorEventCfgV2[cnt].actionParam.sendEmail.emailAddress);
            snprintf(sensorEventCfgV3[cnt].actionParam.sendEmail.subject, sizeof(sensorEventCfgV3[cnt].actionParam.sendEmail.subject), "%s", sensorEventCfgV2[cnt].actionParam.sendEmail.subject);
            snprintf(sensorEventCfgV3[cnt].actionParam.sendEmail.message, sizeof(sensorEventCfgV3[cnt].actionParam.sendEmail.message), "%s", sensorEventCfgV2[cnt].actionParam.sendEmail.message);
            memcpy(&sensorEventCfgV3[cnt].actionParam.cameraAlarmOutput, &sensorEventCfgV2[cnt].actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));

            // copy SMS_PARAMTER_VER1_t into SMS_PARAMTER_t - change due to UTF-8 MultiLanguage
            snprintf(sensorEventCfgV3[cnt].actionParam.smsParameter.mobileNumber1, sizeof(sensorEventCfgV3[cnt].actionParam.smsParameter.mobileNumber1), "%s", sensorEventCfgV2[cnt].actionParam.smsParameter.mobileNumber1);
            snprintf(sensorEventCfgV3[cnt].actionParam.smsParameter.mobileNumber2, sizeof(sensorEventCfgV3[cnt].actionParam.smsParameter.mobileNumber2), "%s", sensorEventCfgV2[cnt].actionParam.smsParameter.mobileNumber2);
            snprintf(sensorEventCfgV3[cnt].actionParam.smsParameter.message, sizeof(sensorEventCfgV3[cnt].actionParam.smsParameter.message), "%s", sensorEventCfgV2[cnt].actionParam.smsParameter.message);
            memcpy(&sensorEventCfgV3[cnt].actionParam.gotoPosition, &sensorEventCfgV2[cnt].actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));

            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                sensorEventCfgV3[cnt].actionParam.systemAlarmOutput[tempCnt] = sensorEventCfgV2[cnt].actionParam.systemAlarmOutput[tempCnt];
            }
        }
        fileVersion++;
    }

    if(fileVersion == 3)
    {
        for(cnt = 0; cnt < MAX_SENSOR_EVENT; cnt++)
        {
            sensorEventCfgV4[cnt].action = sensorEventCfgV3[cnt].action;
            for(tempCnt = 0; tempCnt < MAX_WEEK_DAYS; tempCnt++)
            {
                memcpy(&sensorEventCfgV4[cnt].weeklySchedule[tempCnt], &sensorEventCfgV3[cnt].weeklySchedule[tempCnt], sizeof(WEEKLY_ACTION_SCHEDULE_t));
            }

            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                sensorEventCfgV4[cnt].actionParam.alarmRecord[tempCnt] = sensorEventCfgV3[cnt].actionParam.alarmRecord[tempCnt];
                sensorEventCfgV4[cnt].actionParam.uploadImage[tempCnt] = sensorEventCfgV3[cnt].actionParam.uploadImage[tempCnt];
            }

            snprintf(sensorEventCfgV4[cnt].actionParam.sendTcp, sizeof(sensorEventCfgV4[cnt].actionParam.sendTcp), "%s", sensorEventCfgV3[cnt].actionParam.sendTcp);

            // copy EMAIL_PARAMETER_VER1_t into EMAIL_PARAMETER_t - change due to UTF-8 MultiLanguage
            snprintf(sensorEventCfgV4[cnt].actionParam.sendEmail.emailAddress, sizeof(sensorEventCfgV4[cnt].actionParam.sendEmail.emailAddress), "%s", sensorEventCfgV3[cnt].actionParam.sendEmail.emailAddress);
            snprintf(sensorEventCfgV4[cnt].actionParam.sendEmail.subject, sizeof(sensorEventCfgV4[cnt].actionParam.sendEmail.subject), "%s", sensorEventCfgV3[cnt].actionParam.sendEmail.subject);
            snprintf(sensorEventCfgV4[cnt].actionParam.sendEmail.message, sizeof(sensorEventCfgV4[cnt].actionParam.sendEmail.message), "%s", sensorEventCfgV3[cnt].actionParam.sendEmail.message);
            memcpy(&sensorEventCfgV4[cnt].actionParam.cameraAlarmOutput, &sensorEventCfgV3[cnt].actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));

            // copy SMS_PARAMTER_VER1_t into SMS_PARAMTER_t - change due to UTF-8 MultiLanguage
            snprintf(sensorEventCfgV4[cnt].actionParam.smsParameter.mobileNumber1, sizeof(sensorEventCfgV4[cnt].actionParam.smsParameter.mobileNumber1), "%s", sensorEventCfgV3[cnt].actionParam.smsParameter.mobileNumber1);
            snprintf(sensorEventCfgV4[cnt].actionParam.smsParameter.mobileNumber2, sizeof(sensorEventCfgV4[cnt].actionParam.smsParameter.mobileNumber2), "%s", sensorEventCfgV3[cnt].actionParam.smsParameter.mobileNumber2);
            snprintf(sensorEventCfgV4[cnt].actionParam.smsParameter.message, sizeof(sensorEventCfgV4[cnt].actionParam.smsParameter.message), "%s", sensorEventCfgV3[cnt].actionParam.smsParameter.message);
            memcpy(&sensorEventCfgV4[cnt].actionParam.gotoPosition, &sensorEventCfgV3[cnt].actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));

            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                sensorEventCfgV4[cnt].actionParam.systemAlarmOutput[tempCnt] = sensorEventCfgV3[cnt].actionParam.systemAlarmOutput[tempCnt];
            }
        }
        fileVersion++;
    }

    if(fileVersion == 4)
    {
        for(cnt = 0; cnt < MAX_SENSOR_EVENT; cnt++)
        {
            sensorEventCfg[cnt].action = sensorEventCfgV4[cnt].action;
            for(tempCnt = 0; tempCnt < CAMERA_CONFIG_MAX_V1; tempCnt++)
            {
                sensorEventCfg[cnt].actionParam.alarmRecord[tempCnt] = sensorEventCfgV4[cnt].actionParam.alarmRecord[tempCnt];
                sensorEventCfg[cnt].actionParam.uploadImage[tempCnt] = sensorEventCfgV4[cnt].actionParam.uploadImage[tempCnt];
            }

            for(tempCnt = CAMERA_CONFIG_MAX_V1; tempCnt < MAX_CAMERA_CONFIG; tempCnt++)
            {
                sensorEventCfg[cnt].actionParam.alarmRecord[tempCnt] = DfltSensorEventActionCfg.actionParam.alarmRecord[tempCnt];
                sensorEventCfg[cnt].actionParam.uploadImage[tempCnt] = DfltSensorEventActionCfg.actionParam.uploadImage[tempCnt];
            }

            snprintf(sensorEventCfg[cnt].actionParam.sendEmail.emailAddress, sizeof(sensorEventCfg[cnt].actionParam.sendEmail.emailAddress), "%s", sensorEventCfgV4[cnt].actionParam.sendEmail.emailAddress);
            snprintf(sensorEventCfg[cnt].actionParam.sendEmail.subject, sizeof(sensorEventCfg[cnt].actionParam.sendEmail.subject), "%s", sensorEventCfgV4[cnt].actionParam.sendEmail.subject);
            snprintf(sensorEventCfg[cnt].actionParam.sendEmail.message, sizeof(sensorEventCfg[cnt].actionParam.sendEmail.message), "%s", sensorEventCfgV4[cnt].actionParam.sendEmail.message);
            snprintf(sensorEventCfg[cnt].actionParam.sendTcp, sizeof(sensorEventCfg[cnt].actionParam.sendTcp), "%s", sensorEventCfgV4[cnt].actionParam.sendTcp);
            snprintf(sensorEventCfg[cnt].actionParam.smsParameter.mobileNumber1, sizeof(sensorEventCfg[cnt].actionParam.smsParameter.mobileNumber1), "%s", sensorEventCfgV4[cnt].actionParam.smsParameter.mobileNumber1);
            snprintf(sensorEventCfg[cnt].actionParam.smsParameter.mobileNumber2, sizeof(sensorEventCfg[cnt].actionParam.smsParameter.mobileNumber2), "%s", sensorEventCfgV4[cnt].actionParam.smsParameter.mobileNumber2);
            snprintf(sensorEventCfg[cnt].actionParam.smsParameter.message, sizeof(sensorEventCfg[cnt].actionParam.smsParameter.message), "%s", sensorEventCfgV4[cnt].actionParam.smsParameter.message);
            memcpy(&sensorEventCfg[cnt].actionParam.gotoPosition, &sensorEventCfgV4[cnt].actionParam.gotoPosition, sizeof(GOTO_PRESET_POSITION_t));
            for(tempCnt = 0; tempCnt < MAX_ALARM; tempCnt++)
            {
                sensorEventCfg[cnt].actionParam.systemAlarmOutput[tempCnt] = sensorEventCfgV4[cnt].actionParam.systemAlarmOutput[tempCnt];
            }
            memcpy(&sensorEventCfg[cnt].actionParam.cameraAlarmOutput, &sensorEventCfgV4[cnt].actionParam.cameraAlarmOutput, sizeof(CAMERA_ALARM_PARAMETER_t));

            for(tempCnt = 0; tempCnt < MAX_WEEK_DAYS; tempCnt++)
            {
                memcpy(&sensorEventCfg[cnt].weeklySchedule[tempCnt], &sensorEventCfgV4[cnt].weeklySchedule[tempCnt], sizeof(WEEKLY_ACTION_SCHEDULE_t));
            }
        }
        fileVersion++;
    }

    fileVersion = SENSOR_EVENT_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, sensorEventCfg, (sizeof(SENSOR_EVENT_CONFIG_t) * MAX_SENSOR_EVENT)))
    {
        EPRINT(CONFIGURATION, "fail to write sensor event config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldNetworkDevicesConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8                           cnt;
    NETWORK_DEVICE_CONFIG_VER4_t    networkDevicesCfgV4[MAX_NETWORK_DEVICES];

    switch(fileVersion)
    {
        case 4:
        {
            for(cnt = 0; cnt < MAX_NETWORK_DEVICES; cnt++)
            {
                if((sizeof(NETWORK_DEVICE_CONFIG_VER4_t)) != read(fileFd, &networkDevicesCfgV4[cnt], (sizeof(NETWORK_DEVICE_CONFIG_VER4_t))))
                {
                    EPRINT(CONFIGURATION, "fail to read network device config file: [device=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }
            }

            for(cnt = 0; cnt < MAX_NETWORK_DEVICES; cnt++)
            {
                snprintf(networkDeviceCfg[cnt].deviceName, sizeof(networkDeviceCfg[cnt].deviceName), "%s", networkDevicesCfgV4[cnt].deviceName);
                networkDeviceCfg[cnt].registerMode = networkDevicesCfgV4[cnt].registerMode;
                snprintf(networkDeviceCfg[cnt].registerModeAddress, sizeof(networkDeviceCfg[cnt].registerModeAddress), "%s", networkDevicesCfgV4[cnt].registerModeAddress);
                networkDeviceCfg[cnt].port = networkDevicesCfgV4[cnt].port;
                snprintf(networkDeviceCfg[cnt].username, sizeof(networkDeviceCfg[cnt].username), "%s", networkDevicesCfgV4[cnt].username);
                snprintf(networkDeviceCfg[cnt].password, sizeof(networkDeviceCfg[cnt].password), "%s", networkDevicesCfgV4[cnt].password);
                networkDeviceCfg[cnt].enable = networkDevicesCfgV4[cnt].enable;
                networkDeviceCfg[cnt].autoLogin = networkDevicesCfgV4[cnt].autoLogin;
                networkDeviceCfg[cnt].liveStreamType = networkDevicesCfgV4[cnt].liveStreamType;
                networkDeviceCfg[cnt].preferDeviceCredential = networkDevicesCfgV4[cnt].preferDeviceCredential;
                networkDeviceCfg[cnt].forwardedTcpPort = networkDevicesCfgV4[cnt].forwardedTcpPort;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = NETWORK_DEVICE_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, networkDeviceCfg, (sizeof(NETWORK_DEVICE_CONFIG_t) * MAX_NETWORK_DEVICES)))
    {
        EPRINT(CONFIGURATION, "fail to write network device config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldSnapshotConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8 cnt;

    switch(fileVersion)
    {
        case 1:
        {
            SNAPSHOT_CONFIG_VER1_t snapShotCfgVer1;

            for (cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((sizeof(SNAPSHOT_CONFIG_VER1_t)) != read(fileFd, &snapShotCfgVer1, sizeof(SNAPSHOT_CONFIG_VER1_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read snapshot config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }

                snapShotCfg[cnt].snapShotEnable = snapShotCfgVer1.snapShotEnable;
                snapShotCfg[cnt].snapShotuploadImageRate = snapShotCfgVer1.snapShotuploadImageRate;
                snapShotCfg[cnt].snapShotuploadLocation = snapShotCfgVer1.snapShotuploadLocation;
                /* copy Email Parameter into new (UTF-8) variables */
                snprintf(snapShotCfg[cnt].snapShotuploadToEmail.emailAddress, MAX_EMAIL_ADDRESS_WIDTH, "%s", snapShotCfgVer1.snapShotuploadToEmail.emailAddress);
                snprintf(snapShotCfg[cnt].snapShotuploadToEmail.message, MAX_EMAIL_MESSAGE_WIDTH, "%s", snapShotCfgVer1.snapShotuploadToEmail.message);
                snprintf(snapShotCfg[cnt].snapShotuploadToEmail.subject, MAX_EMAIL_SUBJECT_WIDTH, "%s", snapShotCfgVer1.snapShotuploadToEmail.subject);

                /* max image upload rate reduced to 5 */
                if (snapShotCfg[cnt].snapShotuploadImageRate > MAX_IMAGE_UPLOAD_PER_MINUTE)
                {
                    snapShotCfg[cnt].snapShotuploadImageRate = MAX_IMAGE_UPLOAD_PER_MINUTE;
                }
            }
        }
        break;

        case 2:
        {
            for (cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((sizeof(SNAPSHOT_CONFIG_t)) != read(fileFd, &snapShotCfg[cnt], sizeof(SNAPSHOT_CONFIG_t)))
                {
                    EPRINT(CONFIGURATION, "fail to read snapshot config file: [camera=%d], [version=%d], [err=%s]", cnt, fileVersion, STR_ERR);
                    return FAIL;
                }

                /* max image upload rate reduced to 5 */
                if (snapShotCfg[cnt].snapShotuploadImageRate > MAX_IMAGE_UPLOAD_PER_MINUTE)
                {
                    snapShotCfg[cnt].snapShotuploadImageRate = MAX_IMAGE_UPLOAD_PER_MINUTE;
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = SNAPSHOT_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, snapShotCfg, (sizeof(SNAPSHOT_CONFIG_t)*getMaxCameraForCurrentVariant())))
    {
        EPRINT(CONFIGURATION, "fail to write snapshot config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldLoginPolicyConfig(INT32 fileFd, UINT32 fileVersion)
{
    UINT8					tUserIndex;
    USER_ACCOUNT_CONFIG_t	tUserAccountConfig;
    UINT8					tMinCharReq = 0;

    switch(fileVersion)
    {
        case 1:
        {
            LOGIN_POLICY_CONFIG_VER1_t loginPolicyCfgVer1;

            if((sizeof(LOGIN_POLICY_CONFIG_VER1_t)) != read(fileFd, &loginPolicyCfgVer1, sizeof(LOGIN_POLICY_CONFIG_VER1_t)))
            {
                EPRINT(CONFIGURATION, "fail to read login policy config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }

            if(DEFAULT_PASSWORD_POLICY_PASSWORD_LEN > loginPolicyCfgVer1.minPassLength)
            {
                loginPolicyCfg.minPassLength = DEFAULT_PASSWORD_POLICY_PASSWORD_LEN;
            }
            else
            {
                loginPolicyCfg.minPassLength = loginPolicyCfgVer1.minPassLength;
            }
            loginPolicyCfg.passResetStatus = loginPolicyCfgVer1.passResetStatus;
            loginPolicyCfg.passResetPeriod = loginPolicyCfgVer1.passResetPeriod;
            loginPolicyCfg.lockAccountStatus = loginPolicyCfgVer1.lockAccountStatus;
            loginPolicyCfg.invalidLoginAttempt = loginPolicyCfgVer1.invalidLoginAttempt;
            loginPolicyCfg.autoLockTimer = loginPolicyCfgVer1.autoLockTimer;

            // Default Password Reset Info, if Password Strength not set to HIGH
            // if PassExpiryFile not present, then system will set password reset for all users
            if(FALSE == IsPassExpiryFilePresent())
            {
                break;
            }

            for(tUserIndex = 0; tUserIndex < MAX_USER_ACCOUNT; tUserIndex++)
            {
                ReadSingleUserAccountConfig(tUserIndex, &tUserAccountConfig);
                if (tUserAccountConfig.username[0] == '\0')
                {
                    continue;
                }

                if(CMD_SUCCESS != CheckPasswordPolicy(&loginPolicyCfg, &tUserAccountConfig, &tMinCharReq))
                {
                    CreateNewPasswordExpiryInfo(tUserIndex, &tUserAccountConfig, TRUE);
                }
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = LOGIN_POLICY_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &loginPolicyCfg, sizeof(LOGIN_POLICY_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write login policy config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldScheduleRecordConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            /* NOTE:
             * Version 2 onwards adaptive recording configuration is merged in schedule recording configuration. Data structure
             * for both version 1 & 2 will be same, but interpretation of variable 'scheduleRecordCfg.dailyRecord[weekDay].recordEntireDay'
             * will be done as bit wise value storing flags of schedule recording + adaptive recording of entire day. */
            if (read(fileFd, &scheduleRecordCfg, (sizeof(SCHEDULE_RECORD_CONFIG_VER1_t) * getMaxCameraForCurrentVariant())) != (ssize_t)(sizeof(SCHEDULE_RECORD_CONFIG_VER1_t) * getMaxCameraForCurrentVariant()))
            {
                EPRINT(CONFIGURATION, "fail to read schedule record config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = SCHEDULE_RECORD_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, scheduleRecordCfg, (sizeof(SCHEDULE_RECORD_CONFIG_t) * getMaxCameraForCurrentVariant())))
    {
        EPRINT(CONFIGURATION, "fail to write schedule record config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldFirmwareManagementConfig(INT32 fileFd, UINT32 fileVersion)
{
    switch(fileVersion)
    {
        case 1:
        {
            if (read(fileFd, &firmwareManagementCfg, sizeof(FIRMWARE_MANAGEMENT_CONFIG_t)) != sizeof(FIRMWARE_MANAGEMENT_CONFIG_t))
            {
                EPRINT(CONFIGURATION, "fail to read firmware management config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }

            /* Matrix's FTP server path updated */
            snprintf(firmwareManagementCfg.ftpServerInfo[FIRMWARE_FTP_SERVER_MATRIX].uploadPath,
                     sizeof(firmwareManagementCfg.ftpServerInfo[FIRMWARE_FTP_SERVER_MATRIX].uploadPath), "%s", DfltFirmwareManagementCfg.ftpServerInfo[FIRMWARE_FTP_SERVER_MATRIX].uploadPath);
        }
        break;

        default:
        {
            /* Invalid version */
        }
        return FAIL;
    }

    fileVersion = FIRMWARE_MANAGEMENT_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &firmwareManagementCfg, sizeof(FIRMWARE_MANAGEMENT_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write firmware management config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function should be call when file version is differing From current configuration
 *          version. It will load old configuration from file into new configuration structure.
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
static BOOL loadOldP2pConfig(INT32 fileFd, UINT32 fileVersion)
{
    P2P_CONFIG_VER1_t p2pCfgV1;

    switch(fileVersion)
    {
        case 1:
        {
            if ((sizeof(P2P_CONFIG_VER1_t)) != read(fileFd, &p2pCfgV1, sizeof(P2P_CONFIG_VER1_t)))
            {
                EPRINT(CONFIGURATION, "fail to read P2P config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
                return FAIL;
            }            
        }
        break;

        default:
        {
            EPRINT(CONFIGURATION, "invld P2P config file version: [version=%d]", fileVersion);
        }
        return FAIL;
    }

    if (fileVersion == 1)
    {
        p2pCfg.p2pEnable = p2pCfgV1.p2pEnable;
        p2pCfg.fallbackToRelayServer = DfltP2PCfg.fallbackToRelayServer;
        fileVersion = 2;
    }

    fileVersion = P2P_CONFIG_VERSION;
    if (FAIL == writeConfigFile(fileFd, fileVersion, &p2pCfg, sizeof(P2P_CONFIG_t)))
    {
        EPRINT(CONFIGURATION, "fail to write P2P config file: [version=%d], [err=%s]", fileVersion, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Convert old config file to new config file and load in variable
 * @param   configId
 * @param   fileFd
 * @param   fileVersion
 * @return  SUCCESS/FAIL
 */
BOOL LoadOldConfigParam(UINT8 configId, INT32 fileFd, UINT32 fileVersion)
{
    /* Skip file version and start from beginning */
    lseek(fileFd, 4, SEEK_SET);

    switch(configId)
    {
        case GENERAL_CONFIG_ID:
            return loadOldGeneralConfig(fileFd, fileVersion);

        case DATE_TIME_CONFIG_ID:
            return loadOldDateTimeConfig(fileFd,fileVersion);

        case LAN1_CONFIG_ID:
            return loadOldLan1Config(fileFd,fileVersion);

        case LAN2_CONFIG_ID:
            return loadOldLan2Config(fileFd,fileVersion);

        case STATIC_ROUTING_CONFIG_ID:
            return loadOldStaticRoutingConfig(fileFd,fileVersion);

        case IP_FILTER_CONFIG_ID:
            return loadOldIpFilterConfig(fileFd,fileVersion);

        case DDNS_CONFIG_ID:
            return loadOldDdnsConfig(fileFd,fileVersion);

        case SMTP_CONFIG_ID:
            return loadOldSmtpConfig(fileFd,fileVersion);

        case FTP_UPLOAD_CONFIG_ID:
            return loadOldFtpConfig(fileFd,fileVersion);

        case TCP_NOTIFY_CONFIG_ID:
            return loadOldTcpConfig(fileFd,fileVersion);

        case FILE_ACCESS_CONFIG_ID:
            return loadOldFileAccessConfig(fileFd, fileVersion);

        case MATRIX_DNS_SERVER_CONFIG_ID:
            return loadOldDnsConfig(fileFd,fileVersion);

        case USER_ACCOUNT_CONFIG_ID:
            return loadOldUserAccountConfig(fileFd, fileVersion);

        case CAMERA_CONFIG_ID:
            return loadOldCameraConfig(fileFd, fileVersion);

        case SCHEDULE_RECORD_CONFIG_ID:
            return loadOldScheduleRecordConfig(fileFd, fileVersion);

        case PTZ_PRESET_CONFIG_ID:
            return loadOldPresetPositionConfig(fileFd, fileVersion);

        case PRESET_TOUR_CONFIG_ID:
            return loadOldPresetTourConfig(fileFd, fileVersion);

        case IMAGE_UPLOAD_CONFIG_ID:
            return loadOldUploadImageConfig(fileFd, fileVersion);

        case SENSOR_CONFIG_ID:
            return loadOldSystemSensorConfig(fileFd, fileVersion);

        case ALARM_CONFIG_ID:
            return loadOldSystemAlarmConfig(fileFd, fileVersion);

        case STORAGE_CONFIG_ID:
            return loadOldStorageConfig(fileFd, fileVersion);

        case SCHEDULE_BACKUP_CONFIG_ID:
            return loadOldScheduleBackupConfig(fileFd, fileVersion);

        case MANUAL_BACKUP_CONFIG_ID:
            return loadOldManualBackupConfig(fileFd, fileVersion);

        case CAMERA_EVENT_ACTION_CONFIG_ID:
            return loadOldCameraEventAndActionConfig(fileFd, fileVersion);

        case SENSOR_EVENT_ACTION_CONFIG_ID:
            return loadOldSensorEventAndActionConfig(fileFd, fileVersion);

        case SYSTEM_EVENT_ACTION_CONFIG_ID:
            return loadOldSystemEventAndActionConfig(fileFd, fileVersion);

        case BROAD_BAND_CONFIG_ID:
            return loadOldBroadBandConfig(fileFd, fileVersion);

        case SMS_CONFIG_ID:
            return loadOldSMSConfig(fileFd, fileVersion);

        case NETWORK_DRIVE_SETTINGS:
            return loadOldNetworkDriveConfig(fileFd, fileVersion);

        case IP_CAMERA_CONFIG_ID:
            return loadOldIpCameraConfig(fileFd, fileVersion);

        case NETWORK_DEVICES_CONFIG_ID:
            return loadOldNetworkDevicesConfig(fileFd, fileVersion);

        case SNAPSHOT_CONFIG_ID:
            return loadOldSnapshotConfig(fileFd, fileVersion);

        case LOGIN_POLICY_CONFIG_ID:
            return loadOldLoginPolicyConfig(fileFd, fileVersion);

        case P2P_CONFIG_ID:
            return loadOldP2pConfig(fileFd, fileVersion);

        case FIRMWARE_MANAGEMENT_CONFIG_ID:
            return loadOldFirmwareManagementConfig(fileFd, fileVersion);

        default:
            return SUCCESS;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update old config file and write new config
 * @param   fileFd
 * @param   fileVersion
 * @param   cfgData
 * @param   cfgSize
 * @return  SUCCESS/FAIL
 */
static BOOL writeConfigFile(INT32 fileFd, UINT32 fileVersion, VOIDPTR cfgData, ssize_t cfgSize)
{
    if (lseek(fileFd, 0, SEEK_SET) == -1)
    {
        return FAIL;
    }

    if (sizeof(fileVersion) != write(fileFd, &fileVersion, sizeof(fileVersion)))
    {
        return FAIL;
    }

    if (cfgSize != write(fileFd, cfgData, cfgSize))
    {
        return FAIL;
    }

    /* Dump config data into file */
    fsync(fileFd);

    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
