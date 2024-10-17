//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxOnvifClient.c
@brief      This Module handles all ONVIF communications with IP camera.Also Provides services to
            camera Interface Module. For every new request from camera interface module, a new
            thread is created to serve request which will send multiple ONVIF commands to camera to
            serve request and response is return to camera interface module via Callback.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "OnvifUtils.h"
#include "DebugLog.h"
#include "ConfigApi.h"
#include "Utils.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static INT32 unPack_count(CHAR Pack[], INT32 count);
//-------------------------------------------------------------------------------------------------
static INT8PTR Pack_init(UINT8 Pack[], UINT8 Byte);
//-------------------------------------------------------------------------------------------------
static UINT8PTR End_byte(INT8PTR pCount);
//-------------------------------------------------------------------------------------------------
static INT8PTR Pack_byte(INT8PTR pCount, UINT8 Byte);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function encode string into tiff6 packbit.
 * @param   array - message string to be packed
 * @param   count - length of array
 * @param   Pack - packed string
 * @return  length of packed string
 */
INT32 Tiff6_PackBits(UINT8 array[], INT32 count, UINT8 Pack[])
{
    INT32   i = 0;
    INT8PTR pCount = Pack_init(Pack, array[i]);

    i++;
    for (; i < count; i++)
    {
        pCount = Pack_byte(pCount, array[i]);
    }

    UINT8PTR End = End_byte(pCount);
    *End = '\0';
    return (End - Pack);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function decode base 64 to string.
 * @param   Pack - message string to be decoded
 * @param   count - length of message
 * @param   array - decoded output string
 * @return  length of unpacked string
 */
INT32 Tiff6_unPackBits(CHAR Pack[], INT32 count, UINT8 array[])
{
    INT32   c, n, nRes = 0;
    INT8PTR pCount = (INT8PTR)Pack;

    if (!array)
    {
        return unPack_count(Pack, count);
    }

    while ((CHARPTR)pCount < (Pack+count))
    {
        c = *pCount;
        if (c < 0)
        {
            n = (1-c);
            memset(&array[nRes], pCount[1], n);
            nRes += n;
        }
        else
        {
            n = (1+c);
            memcpy(&array[nRes], &pCount[1], n);
            nRes += n;
        }

        pCount = (INT8PTR)End_byte(pCount);
    }
    return nRes;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function count packstring length.
 * @param   Pack
 * @param   count
 * @return  Unpacked count
 */
static INT32 unPack_count(CHAR Pack[], INT32 count)
{
    INT32   c, nRes = 0;
    INT8PTR pCount = (INT8PTR)Pack;

    while ((CHARPTR)pCount < (Pack+count))
    {
        c = *pCount;
        if (c < 0)
        {
            nRes += (1-c);
        }
        else
        {
            nRes += (1+c);
        }

        pCount = (INT8PTR)End_byte(pCount);
    }

    return nRes;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function initalize packbits.
 * @param   Pack
 * @param   Byte
 * @return
 */
static INT8PTR Pack_init(UINT8 Pack[], UINT8 Byte)
{
    INT8PTR pCount = (INT8PTR)Pack;

    *pCount = 0;
    Pack[1] = Byte;

    return pCount;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   End_byte
 * @param   pCount
 * @return
 */
static UINT8PTR End_byte(INT8PTR pCount)
{
    UINT8PTR Pack = (UINT8PTR)(pCount+1);

    INT8 c = *pCount;
    if (c > 0)
    {
        Pack = &Pack[c+1];
    }
    else
    {
        Pack = &Pack[1];
    }

    return Pack;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pack_byte
 * @param   Count
 * @param   Byte
 * @return
 */
static INT8PTR Pack_byte(INT8PTR pCount, UINT8 Byte)
{
    INT8        c = *pCount;
    UINT8PTR    pEnd = End_byte(pCount);

    if (127 == c || c == -127)
    {
        return Pack_init(pEnd, Byte);
    }

    UINT8PTR Pack = pEnd-1;
    if (*(Pack) == Byte)
    {
        if (c > 0)
        {
            (*pCount) = c-1;
            pCount = Pack_byte(Pack_init(Pack, Byte), Byte);
        }
        else
        {
            (*pCount)--;
        }
    }
    else
    {
        if (c >= 0)
        {
            *pEnd = Byte;
            (*pCount)++;
        }
        else
        {
            pCount = Pack_init(pEnd, Byte);
        }
    }

    return pCount;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse URL to get relative URL
 * @param   pIpAddr: Required IP address in URL
 * @param   pStartUri: Input URL
 * @param   pRelativeUrl: Parsed URL
 * @param   relativeUrlLen: Relative URL buffer length
 * @return  Returns SUCCESS on success else returns FAIL on failure
 */
BOOL ParseRequestUri(const CHAR *pIpAddr, CHAR *pStartUri, CHAR *pRelativeUrl, const UINT16 relativeUrlLen)
{
    CHAR *pTokenStr = NULL;
    CHAR *pSearchStr;
    CHAR cameraUrl[MAX_CAMERA_URI_WIDTH];

    /* Validate stream URI */
    if ((pIpAddr == NULL) || (pStartUri == NULL))
    {
        return FAIL;
    }

    /* Copy camera url in local buffer */
    snprintf(cameraUrl, sizeof(cameraUrl), "%s", pStartUri);
    pSearchStr = cameraUrl;

    /* Is required IP address found in URL? */
    if (NULL == strstr(cameraUrl, pIpAddr))
    {
        EPRINT(ONVIF_CLIENT, "ip addr not found in url: [ip=%s], [url=%s]", pIpAddr, pStartUri);
        return FAIL;
    }

    /* Entry point example : http://192.168.101.251/onvif/device_service http://[6fff::55]/onvif/device_service */
    while ((pTokenStr = strtok_r(NULL, " ", &pSearchStr)))
    {
        /* Is required ip address available in this token? */
        if (NULL != strstr(pTokenStr, pIpAddr))
        {
            break;
        }
    }

    /* Is required token found? */
    if (NULL == pTokenStr)
    {
        EPRINT(ONVIF_CLIENT, "ip addr not found in token: [ip=%s], [url=%s]", pIpAddr, pStartUri);
        return FAIL;
    }

    /* Search for start of ip address */
    pTokenStr = strstr(pTokenStr, "://");
    if (NULL == pTokenStr)
    {
        return FAIL;
    }

    /* Jump to start of ip address */
    pTokenStr += strlen("://");

    /* Search for relative URL */
    pTokenStr = strchr(pTokenStr, '/');
    if (NULL == pTokenStr)
    {
        pRelativeUrl[0] = '/';
        pRelativeUrl[1] = '\0';
    }
    else
    {
        snprintf(pRelativeUrl, relativeUrlLen, "%s", pTokenStr);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse RTSP port from stream URI
 * @param   pStartUri
 * @param   pRtspPort
 * @return  Returns SUCCESS on success else returns FAIL on failure
 */
BOOL ParseRtspPort(CHAR *pStartUri, UINT16 *pRtspPort)
{
    /* Validate stream URI */
    if (NULL == pStartUri)
    {
        return FAIL;
    }

    /* Search for start of ip address */
    pStartUri = strstr(pStartUri, "://");
    if (NULL == pStartUri)
    {
        return FAIL;
    }

    /* Jump to start of ip address */
    pStartUri += strlen("://");

    /* Search for start of rtsp port */
    pStartUri = strchr(pStartUri, ':');
    if (NULL == pStartUri)
    {
        return FAIL;
    }

    /* Jump next to start of rtsp port */
    pStartUri++;

    /* Search for end of rtsp port */
    CHAR *pEndUri = strchr(pStartUri, '/');
    if (NULL == pEndUri)
    {
        return FAIL;
    }

    CHAR  portValStr[6];
    UINT8 portStrLen = (pEndUri - pStartUri)+1;

    /* Check valid rtsp port string */
    if (portStrLen > sizeof(portValStr))
    {
        return FAIL;
    }

    /* Copy rtsp port string in buffer */
    snprintf(portValStr, portStrLen, "%s", pStartUri);

    /* Convert string to value */
    *pRtspPort = atoi(portValStr);
    if (*pRtspPort == 0)
    {
        return FAIL;
    }

    /* rtsp port parsed successfully */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Migrate ONVIF PTZ preset token configuration from local config to system config. It is
 *          added to make preset token conpatible with system configuration.
 */
void MigratePtzPresetOnvifTokenConfig(void)
{
    INT32                   fileFd;
    UINT8                   camIndex;
    UINT8                   presetNum;
    INT32                   tokenLen;
    UINT32                  presetFileVer;
    CHAR                    onvifPresetToken[20] = {0};
    PTZ_PRESET_CONFIG_t     ptzPresetCfg;
    const CHAR              *pPtzPresetFile = "/etc/onvifPresetInfo.cfg";

    /* Is ptz preset local config file present? */
    if (access(pPtzPresetFile, F_OK) != STATUS_OK)
    {
        /* We already migrated local config */
        return;
    }

    /* Open ptz preset local config file for migration */
    fileFd = open(pPtzPresetFile, READ_WRITE_SYNC_MODE);
    if (fileFd == INVALID_FILE_FD)
    {
        /* Fail to open the file, Hence migration is not possible */
        EPRINT(ONVIF_CLIENT, "fail to open ptz preset config file: [err=%s]", STR_ERR);
        unlink(pPtzPresetFile);
        return;
    }

    /* Read preset file version for compatibility */
    if (read(fileFd, &presetFileVer, sizeof(UINT32)) < (ssize_t)sizeof(UINT32))
    {
        /* Fail to read the file, Hence migration is not possible */
        EPRINT(ONVIF_CLIENT, "fail to read ptz preset config file version: [err=%s]", STR_ERR);
        close(fileFd);
        unlink(pPtzPresetFile);
        return;
    }

    /* In version 0, token length was 10bytes (Per camera = Max preset position * Max preset token size) */
    /* In version 1, token length was 20bytes (Per camera = Max preset position * Max preset token size) */
    tokenLen = (presetFileVer == 0) ? 10 : 20;

    /* Migrate preset configuration for all cameras */
    DPRINT(ONVIF_CLIENT, "ptz preset config migration start: [presetFileVer=%d]", presetFileVer);
    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        /* Migrate all presets of the camera */
        for (presetNum = 0; presetNum < MAX_PRESET_POSITION; presetNum++)
        {
            /* Read current preset config of camera */
            if (read(fileFd, onvifPresetToken, tokenLen) < tokenLen)
            {
                /* Fail to read the file, Hence migration is not possible */
                EPRINT(ONVIF_CLIENT, "fail to read ptz preset config file: [camera=%d], [err=%s]", camIndex, STR_ERR);
                close(fileFd);
                unlink(pPtzPresetFile);
                return;
            }

            /* Read ptz preset system config */
            ReadSinglePtzPresetConfig(camIndex, presetNum, &ptzPresetCfg);
            if (ptzPresetCfg.name[0] == '\0')
            {
                /* Don't update the token if name is not present */
                continue;
            }

            /* Update token and write into system config again */
            snprintf(ptzPresetCfg.token, MAX_PRESET_TOKEN_LEN, "%s", onvifPresetToken);
            WriteSinglePtzPresetConfig(camIndex, presetNum, &ptzPresetCfg);
        }
    }
    DPRINT(ONVIF_CLIENT, "ptz preset config migration done: [presetFileVer=%d]", presetFileVer);

    /* Close and remove the file as migration is done */
    close(fileFd);
    unlink(pPtzPresetFile);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
