//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DeviceInfo.c
@brief      This file used to provide the device information to other modules of NVR.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Utils.h"
#include "DebugLog.h"
#include "InputOutput.h"

//#################################################################################################
// @EXTERN
//#################################################################################################


//#################################################################################################
// @DATA_TYPES
//#################################################################################################

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static CHAR                 swVerStr[DEVICE_SW_VERSION_LEN_MAX]; /* Example: V07R11.01 */
static CHAR                 nvrSoftwareModelStr[DEVICE_MODEL_STR_LEN_MAX + DEVICE_SW_VERSION_LEN_MAX + 4];
static NVR_DEVICE_INFO_t    deviceInfo;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize and provide device information
 */
void InitDeviceInfo(void)
{
    /* Reset all variables of device information */
    memset(&deviceInfo, 0, sizeof(NVR_DEVICE_INFO_t));

    /* Get board variant */
    deviceInfo.productVariant = GetProductVariant();

    /* Init max cameras, no. of LAN ports and HDDs according to variant */
    #if defined(OEM_JCI)
    deviceInfo.noOfLanPort = 2;
    deviceInfo.noOfHdd = 8;
    deviceInfo.audioIn = 1;
    deviceInfo.USBPort = 3;
    deviceInfo.maxSensorInput = 2;
    deviceInfo.maxAlarmOutput = 1;

    switch (deviceInfo.productVariant)
    {
        case HRIN_1208_18_SR:
        {
            deviceInfo.maxCameras = 12;
        }
        break;

        case HRIN_2808_18_SR:
        {
            deviceInfo.maxCameras = 28;
        }
        break;

        case HRIN_4808_18_SR:
        {
            deviceInfo.maxCameras = 48;
        }
        break;

        case HRIN_6408_18_SR:
        {
            deviceInfo.maxCameras = 64;
        }
        break;

        default:
        {
            deviceInfo.maxCameras = 12;
            EPRINT(SYS_LOG, "invld product variant found: [productVariant=%d]", deviceInfo.productVariant);
        }
        break;
    }
    #else
    switch (deviceInfo.productVariant)
    {
        case NVR0401XSP2:
        {
            deviceInfo.maxCameras = 4;
            deviceInfo.noOfLanPort = 1;
            deviceInfo.noOfHdd = 1;
        }
        break;

        case NVR0801X:
        case NVR0801XP2:
        case NVR0801XSP2:
        {
            deviceInfo.maxCameras = 8;
            deviceInfo.noOfLanPort = 1;
            deviceInfo.noOfHdd = 1;
        }
        break;

        case NVR1601X:
        case NVR1601XP2:
        case NVR1601XSP2:
        {
            deviceInfo.maxCameras = 16;
            deviceInfo.noOfLanPort = 1;
            deviceInfo.noOfHdd = 1;
        }
        break;

        case NVR1602X:
        case NVR1602XP2:
        {
            deviceInfo.maxCameras = 16;
            deviceInfo.noOfLanPort = 2;
            deviceInfo.noOfHdd = 2;
        }
        break;

        case NVR3202X:
        case NVR3202XP2:
        {
            deviceInfo.maxCameras = 32;
            deviceInfo.noOfLanPort = 2;
            deviceInfo.noOfHdd = 2;
        }
        break;

        case NVR3204X:
        case NVR3204XP2:
        {
            deviceInfo.maxCameras = 32;
            deviceInfo.noOfLanPort = 2;
            deviceInfo.noOfHdd = 4;
        }
        break;

        case NVR6404X:
        case NVR6404XP2:
        {
            deviceInfo.maxCameras = 64;
            deviceInfo.noOfLanPort = 2;
            deviceInfo.noOfHdd = 4;
        }
        break;

        case NVR6408X:
        case NVR6408XP2:
        {
            deviceInfo.maxCameras = 64;
            deviceInfo.noOfLanPort = 2;
            deviceInfo.noOfHdd = 8;
        }
        break;

        case NVR9608XP2:
        {
            deviceInfo.maxCameras = 96;
            deviceInfo.noOfLanPort = 2;
            deviceInfo.noOfHdd = 8;
        }
        break;

        default:
        {
            deviceInfo.maxCameras = 8;
            deviceInfo.noOfLanPort = 1;
            deviceInfo.noOfHdd = 1;
            EPRINT(SYS_LOG, "invld product variant found: [productVariant=%d]", deviceInfo.productVariant);
        }
        break;
    }

    /* Init no. of sensors & alarms, audio-in port and usb ports according to variant */
    switch (deviceInfo.productVariant)
    {
        case NVR0401XSP2:
        case NVR0801XSP2:
        case NVR1601XSP2:
        {
            deviceInfo.audioIn = 0;
            deviceInfo.USBPort = 2;
            deviceInfo.maxSensorInput = 0;
            deviceInfo.maxAlarmOutput = 0;
        }
        break;

        default:
        {
            deviceInfo.audioIn = 1;
            deviceInfo.USBPort = 3;
            deviceInfo.maxSensorInput = 2;
            deviceInfo.maxAlarmOutput = 1;
        }
        break;
    }
    #endif

    /* Set NVR device information */
    deviceInfo.softwareVersion = SOFTWARE_VERSION;
    deviceInfo.softwareRevision = SOFTWARE_REVISION;
    deviceInfo.commVersion = COMMUNICATION_VERSION;
    deviceInfo.commRevision = COMMUNICATION_REVISION;
    deviceInfo.productSubRevision = PRODUCT_SUB_REVISION;
    deviceInfo.maxIpCameras = deviceInfo.maxCameras;
    deviceInfo.configuredIpCameras = deviceInfo.maxCameras;
    deviceInfo.audioOut = 1;
    deviceInfo.noOfNdd = 2;
    deviceInfo.HDMI1 = 1;
    deviceInfo.videoStandard = DFLT_VIDEO_SYSTEM_TYPE;
    deviceInfo.diskCheckingReq = FALSE;

    /* Prepare software version string */
    snprintf(swVerStr, sizeof(swVerStr), "V%02dR%02d.%d", SOFTWARE_VERSION, SOFTWARE_REVISION, PRODUCT_SUB_REVISION);
    snprintf(nvrSoftwareModelStr, sizeof(nvrSoftwareModelStr), "%s - %s", GetNvrModelStr(), swVerStr);

    DPRINT(SYS_LOG, "Product Variant            :%d", deviceInfo.productVariant);
    DPRINT(SYS_LOG, "Software Version           :%d", deviceInfo.softwareVersion);
    DPRINT(SYS_LOG, "Software Revision          :%d", deviceInfo.softwareRevision);
    DPRINT(SYS_LOG, "Product Subversion         :%d", deviceInfo.productSubRevision);
    DPRINT(SYS_LOG, "Communication Version      :%d", deviceInfo.commVersion);
    DPRINT(SYS_LOG, "Communication Revision     :%d", deviceInfo.commRevision);
    DPRINT(SYS_LOG, "Max Cameras                :%d", deviceInfo.maxCameras);
    DPRINT(SYS_LOG, "Max Ip Cameras             :%d", deviceInfo.maxIpCameras);
    DPRINT(SYS_LOG, "Configured Ip Cameras      :%d", deviceInfo.configuredIpCameras);
    DPRINT(SYS_LOG, "Max Sensor Input           :%d", deviceInfo.maxSensorInput);
    DPRINT(SYS_LOG, "Max Alarm Output           :%d", deviceInfo.maxAlarmOutput);
    DPRINT(SYS_LOG, "Max Audio In               :%d", deviceInfo.audioIn);
    DPRINT(SYS_LOG, "Max Audio Out              :%d", deviceInfo.audioOut);
    DPRINT(SYS_LOG, "No Of HDD                  :%d", deviceInfo.noOfHdd);
    DPRINT(SYS_LOG, "No Of NDD                  :%d", deviceInfo.noOfNdd);
    DPRINT(SYS_LOG, "No Of Lan Port             :%d", deviceInfo.noOfLanPort);
    DPRINT(SYS_LOG, "No Of VGA                  :%d", deviceInfo.noOfVGA);
    DPRINT(SYS_LOG, "HDMI1                      :%d", deviceInfo.HDMI1);
    DPRINT(SYS_LOG, "HDMI2                      :%d", deviceInfo.HDMI2);
    DPRINT(SYS_LOG, "USB Port                   :%d", deviceInfo.USBPort);
    DPRINT(SYS_LOG, "Video Standard             :%d", deviceInfo.videoStandard);
    DPRINT(SYS_LOG, "Max Main Encoding Capacity :%d", deviceInfo.maxMainEncodingCap);
    DPRINT(SYS_LOG, "Max Sub Encoding Capacity  :%d", deviceInfo.maxSubEncodingCap);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get information of nvr device
 * @param   pDeviceInfo
 */
void GetNvrDeviceInfo(NVR_DEVICE_INFO_t *pDeviceInfo)
{
    memcpy(pDeviceInfo, &deviceInfo, sizeof(NVR_DEVICE_INFO_t));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get NVR model string from model type
 * @return  NVR model string
 */
const CHAR *GetNvrModelStr(void)
{
    switch (deviceInfo.productVariant)
    {
        #if defined(OEM_JCI)
        CASE_TO_STR(HRIN_1208_18_SR, 0);
        CASE_TO_STR(HRIN_2808_18_SR, 0);
        CASE_TO_STR(HRIN_4808_18_SR, 0);
        CASE_TO_STR(HRIN_6408_18_SR, 0);
        #else
        CASE_TO_STR(NVR0801X, 0);
        CASE_TO_STR(NVR1601X, 0);
        CASE_TO_STR(NVR1602X, 0);
        CASE_TO_STR(NVR3202X, 0);
        CASE_TO_STR(NVR3204X, 0);
        CASE_TO_STR(NVR6404X, 0);
        CASE_TO_STR(NVR6408X, 0);

        CASE_TO_STR(NVR0801XP2, 0);
        CASE_TO_STR(NVR1601XP2, 0);
        CASE_TO_STR(NVR1602XP2, 0);
        CASE_TO_STR(NVR3202XP2, 0);
        CASE_TO_STR(NVR3204XP2, 0);
        CASE_TO_STR(NVR6404XP2, 0);
        CASE_TO_STR(NVR6408XP2, 0);
        CASE_TO_STR(NVR9608XP2, 0);

        CASE_TO_STR(NVR0401XSP2, 0);
        CASE_TO_STR(NVR0801XSP2, 0);
        CASE_TO_STR(NVR1601XSP2, 0);
        #endif

        default:
            return ("UNKNOWN");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API returns the number of IP camera
 * @return  No. of camera
 */
inline UINT8 getMaxCameraForCurrentVariant(void)
{
    return deviceInfo.maxCameras;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API returns the number of HardDisk support
 * @return  HardDisk count
 */
inline UINT8 getMaxHardDiskForCurrentVariant(void)
{
    return deviceInfo.noOfHdd;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get NVR model type
 * @return  NVR model type
 */
inline NVR_VARIANT_e GetNvrModelType(void)
{
    return deviceInfo.productVariant;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get product build date-time string
 * @return  Build date-time string
 */
const CHAR *GetBuildDateTimeStr(void)
{
    /* Get product build date-time string */
    return (__DATE__ " " __TIME__);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get software version string
 * @return  Version string
 */
const CHAR *GetSoftwareVersionStr(void)
{
    /* Get software version string */
    return swVerStr;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get NVR model and software version string
 * @return  NVR0801XP2 - V08R01.1
 */
const CHAR *GetNvrDeviceInfoStr(void)
{
    /* Get NVR model and software version string */
    return nvrSoftwareModelStr;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the number of lan port in the system
 * @return  1 or 2
 */
inline UINT8 GetNoOfLanPort(void)
{
    /* Provide number of lan port */
    return deviceInfo.noOfLanPort;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is NVR having Audio In Port
 * @return  TRUE if Audio In port is available else FALSE
 */
BOOL IsAudioInPortAvailable(void)
{
    /* Return TRUE if Audio In port is available else FALSE */
    return (deviceInfo.audioIn ? TRUE : FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is NVR having Audio Out Port
 * @return  TRUE if Audio Out port is available else FALSE
 */
BOOL IsAudioOutPortAvailable(void)
{
    /* Return TRUE if Audio Out port is available else FALSE */
    return (deviceInfo.audioOut ? TRUE : FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the number of sensor input
 * @return  Returns sensor input
 */
inline UINT8 GetNoOfSensorInput(void)
{
    /* Provide number of sensor input */
    return deviceInfo.maxSensorInput;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the number of alarm output
 * @return  Returns alarm output
 */
inline UINT8 GetNoOfAlarmOutput(void)
{
    /* Provide number of alarm output */
    return deviceInfo.maxAlarmOutput;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
