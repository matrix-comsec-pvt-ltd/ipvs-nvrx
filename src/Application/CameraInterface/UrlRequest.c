//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file       UrlRequest.c
@brief      Provide function pointers for url and parsing functions for supported brands
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "UrlRequest.h"
#include "DateTime.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Get scale window width, height etc for privacy mask window */
#define GET_CAMERA_MAPPED_WINDOW_SIZE(size, scale, res) (((UINT32)res*size)/scale)
#define GET_NVR_MAPPED_WINDOW_SIZE(size, scale, res)    (size ? (UINT32)((size * scale) / (UINT32)res) : 0)

//#################################################################################################
// @VARIABLES
//#################################################################################################
static URL_FUNCTION_t * getUrl[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (URL_FUNCTION_t *)&MatrixUrl,                                       // 2 : Matrix camera
};

static PARSER_FUNC * parseFunc[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (PARSER_FUNC *)&MatrixParser,                                       // 2 : Matrix Camera
};

static PARSER_CONFIG_FUNC * parseCnfgFunc[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (PARSER_CONFIG_FUNC *)&MatrixCnfgParser,                            // 2 : Matrix Camera
};

static PARSER_DEVICE_INFO_FUNC * parseGetDevInfoFunc[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (PARSER_DEVICE_INFO_FUNC *)&MatrixDeviceInfoParser,                 // 2 : Matrix Camera
};

static PARSER_GET_MOTION_WINDOW_RESPONSE * parseGetMotionWindowResponseFunc[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (PARSER_GET_MOTION_WINDOW_RESPONSE *)&MatrixGetMotionWindowParser,	// 2 : Matrix Camera
};

static PARSER_PRIVACYMSAK_RESPONSE * parseGetPrivacyMaskResponseFunc[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (PARSER_PRIVACYMSAK_RESPONSE *)&MatrixPrivacyMaskParser,            // 2 : Matrix camera
};

static PARSER_IMAGE_SETTING_RESPONSE *parseGetImageSettingResponseFunc[MAX_CAMERA_BRAND] =
{
    NULL,                                                               // 0 : camera brand none
    NULL,                                                               // 1 : generic camera
    (PARSER_IMAGE_SETTING_RESPONSE *)&MatrixImageSettingParser,         // 2 : Matrix camera
};

static const CHARPTR camInitDateFormat[MAX_DATE_FORMAT] =
{
    "DD/MM/YYYY",		//index 0
    "MM/DD/YYYY",		//index 1
    "YYYY/MM/DD",		//index 2
    "WWW DD/MM/YYYY",	//index 2
};

static const CHARPTR camInitTimeFormat[MAX_TIME_FORMAT] =
{
    "12 hour",		//index 0
    "24 hour",		//index 1
};

static const UINT32 matrixFixedWindowResolution[MAX_MATRIX_SDK][MAX_FRAME_DIMENSION] =
{
    {    0,    0 },
    { 1920, 1080 },
};

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e prepareBmReqestUrl(UINT8 cameraIndex, CAMERA_BRAND_e brand, CAMERA_MODEL_e model, REQ_URL_e reqType,
                                           CAMERA_REQUEST_t *pCamReqInfo, VOIDPTR pInArg1, VOIDPTR pInArg2, VOIDPTR pInArg3, VOIDPTR pInArg4);
//-------------------------------------------------------------------------------------------------
static BOOL getPrivacyMaskCameraResolution(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, UINT32PTR pWindowWidth, UINT32PTR pWindowHeight);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTION DEFINATION
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief GetUrl: get url function pointer for each brand
 * @param brand
 * @return
 */
URL_FUNCTION_t *GetUrl(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return getUrl[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ParseFunc: get parsing function pointer for each brand
 * @param brand
 * @return
 */
PARSER_FUNC *ParseFunc(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return parseFunc[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ParseCnfgFunc: get config parser function pointer for each brand
 * @param brand
 * @return
 */
PARSER_CONFIG_FUNC *ParseCnfgFunc(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return parseCnfgFunc[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ParseGetMotionWindowResponseFunc: get function pointer for motion window parser
 * @param brand
 * @return
 */
PARSER_GET_MOTION_WINDOW_RESPONSE *ParseGetMotionWindowResponseFunc(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return parseGetMotionWindowResponseFunc[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ParsePrivacyMaskRersponseFunc: get function pointer for privacy mask parser
 * @param brand
 * @return
 */
PARSER_PRIVACYMSAK_RESPONSE *ParsePrivacyMaskRersponseFunc(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return parseGetPrivacyMaskResponseFunc[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ParseGetDevInfoFunc: get function pointer for device info parser
 * @param brand
 * @return
 */
PARSER_DEVICE_INFO_FUNC *ParseGetDevInfoFunc(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return parseGetDevInfoFunc[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ParseImageSettingResponseFunc: get function pointer for image setting parser
 * @param   brand
 * @return
 */
PARSER_IMAGE_SETTING_RESPONSE *ParseImageSettingResponseFunc(CAMERA_BRAND_e brand)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return NULL;
    }

    return parseGetImageSettingResponseFunc[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera initiation request URL
 * @param   cameraIndex
 * @param   urlType
 * @param   cmdArg
 * @param   reqType
 * @param   callback
 * @param   pCamReqInfo
 * @param   pInArg
 */
void GetCiRequestUrl(UINT8 cameraIndex, REQ_URL_e urlType, UINT32 cmdArg, CAM_REQUEST_TYPE_e reqType,
                     VOIDPTR callback, CAMERA_REQUEST_t *pCamReqInfo, VOIDPTR pInArg)
{
    UINT8           loop;
    UINT8           reqCnt = 0;
    UINT16          outLen = 0;
    URL_REQUEST_t   *reqPayload = &pCamReqInfo->url[reqCnt];
    CHARPTR         reqMesStr = reqPayload[reqCnt].relativeUrl;

    /* Set camera request information */
    pCamReqInfo->numOfRequest = 0;
    pCamReqInfo->cameraIndex = cameraIndex;
    pCamReqInfo->url[reqCnt].protocolType = CAM_TCP_PROTOCOL;
    pCamReqInfo->url[reqCnt].requestType = reqType;
    pCamReqInfo->tcpCallback[reqType]= callback;

    switch(urlType)
    {
        case REQ_URL_GET_STREAM:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_CFG;
            reqPayload[reqCnt].requestType = CAM_REQ_MEDIA;
            reqPayload[reqCnt].protocolType = CAM_TCP_PROTOCOL;
            reqPayload[reqCnt].tcpRequestType = CI_SRT_LV_STRM;
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_STREAM_CFG:
        {
            UINT8                       codecIdx = 0;
            MATRIX_IP_CAM_RESOLUTION_e  resolutionNum = MATRIX_IP_CAM_RESOLUTION_8MP;
            UINT8                       streamProfileNum;
            CHARPTR                     pProfileCodec;
            CHARPTR                     pResolution;
            BITRATE_MODE_e              bitrateMode;
            UINT8                       framerate;
            BITRATE_VALUE_e             bitrateValue;
            UINT8                       gop;
            UINT8                       quality;
            BOOL                        enableAudio;
            STREAM_CONFIG_t             *pStreamConfig = (STREAM_CONFIG_t*)pInArg;

            if (MAIN_STREAM == cmdArg)
            {
                pProfileCodec = pStreamConfig->videoEncoding;
                pResolution = pStreamConfig->resolution;
                bitrateMode = pStreamConfig->bitrateMode;
                framerate = pStreamConfig->framerate;
                bitrateValue = pStreamConfig->bitrateValue;
                gop = pStreamConfig->gop;
                quality = pStreamConfig->quality - 1;
                enableAudio = pStreamConfig->enableAudio;
                streamProfileNum = pStreamConfig->mainStreamProfile - 1;
            }
            else
            {
                pProfileCodec = pStreamConfig->videoEncodingSub;
                pResolution = pStreamConfig->resolutionSub;
                bitrateMode = pStreamConfig->bitrateModeSub;
                framerate = pStreamConfig->framerateSub;
                bitrateValue = pStreamConfig->bitrateValueSub;
                gop = pStreamConfig->gopSub;
                quality = pStreamConfig->qualitySub - 1;
                enableAudio = pStreamConfig->enableAudioSub;
                streamProfileNum = pStreamConfig->subStreamProfile - 1;
            }

            GetResolutionNoforMatrixIpCamera(pResolution, &resolutionNum);
            if(strstr(pProfileCodec, actualCodecStr[VIDEO_H264]) != NULL)
            {
                codecIdx = 0;
            }
            else if(strstr(pProfileCodec, actualCodecStr[VIDEO_MJPG]) != NULL)
            {
                codecIdx = 1;
            }
            else if(strstr(pProfileCodec, actualCodecStr[VIDEO_H265]) != NULL)
            {
                codecIdx = 2;
            }

            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_CFG;
            outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d",
                              SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_STREAM_PROFILE_CFG, FSP, SOI, streamProfileNum);

            for(loop = CI_STREAM_CFG_NAME; loop < CI_STREAM_CFG_MAX; loop++)
            {
                switch (loop)
                {
                    case CI_STREAM_CFG_CODEC:
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%s", FSP, CI_STREAM_CFG_CODEC, FVS, codecStrForSetMatrixIP[codecIdx]);
                        break;

                    case CI_STREAM_CFG_RESL:
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%s", FSP, CI_STREAM_CFG_RESL, FVS, resolutionStrForSetMatrixIP[resolutionNum]);
                        break;

                    case CI_STREAM_CFG_FPS:
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_STREAM_CFG_FPS, FVS, framerate);
                        break;

                    case CI_STREAM_CFG_BITCTRL:
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%s", FSP, CI_STREAM_CFG_BITCTRL, FVS, bitRateModeStrForMatrixIP[bitrateMode]);
                        break;

                    case CI_STREAM_CFG_BITRATE:
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_STREAM_CFG_BITRATE, FVS, bitRateValueForMatrixIP[bitrateValue]);
                        break;

                    case CI_STREAM_CFG_IMGQLTY:
                        if (bitrateMode == VBR)
                        {
                            outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_STREAM_CFG_IMGQLTY, FVS, quality);
                        }
                        break;

                    case CI_STREAM_CFG_GOP:
                        if (streamProfileNum != 2)
                        {
                            outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_STREAM_CFG_GOP, FVS, gop);
                        }
                        break;

                    case CI_STREAM_CFG_AUD:
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_STREAM_CFG_AUD, FVS, enableAudio);
                        break;

                    default:
                        break;
                }
            }

            snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen,"%c%c%c%c",FSP,EOI,EOT,EOM);
            reqPayload[reqCnt].requestType = CAM_REQ_CONTROL;
            reqPayload[reqCnt].protocolType = CAM_TCP_PROTOCOL;
            reqPayload[reqCnt].tcpRequestType = CI_SET_CFG;
            reqCnt++;
            reqPayload[reqCnt].requestType = CAM_REQ_MEDIA;
            reqPayload[reqCnt].protocolType = CAM_TCP_PROTOCOL;
            reqPayload[reqCnt].tcpRequestType = CI_SRT_LV_STRM;
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_OSD:
        {
            OSD_PARAM_t *pOsdParam = (OSD_PARAM_t*)pInArg;

            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_CFG;
            outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d", SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_OVERLAY_CFG, FSP, SOI, 0);

            /* Enable or disable date-time overlay in camera */
            outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_OVERLAY_CFG_ENABLE, FVS, pOsdParam->dateTimeOverlay);

            /* Add date-time format and position if overlay is enabled */
            if (pOsdParam->dateTimeOverlay == ENABLE)
            {
                GENERAL_CONFIG_t genConfig;
                ReadGeneralConfig(&genConfig);
                outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%s%c%d%c%s%c%d%c%d",
                                   FSP, CI_OVERLAY_CFG_DATE, FVS, camInitDateFormat[genConfig.dateFormat],
                                   FSP, CI_OVERLAY_CFG_TIME, FVS, camInitTimeFormat[genConfig.timeFormat],
                                   FSP, CI_OVERLAY_CFG_DISP, FVS, pOsdParam->dateTimePos - 1);
            }

            /* Apply text overlay settings only if text overlay param changed */
            if (pOsdParam->textoverlayChanged == TRUE)
            {
                /* Enable or disable text overlay in camera */
                outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_OVERLAY_CFG_TXT_ENABLE, FVS, pOsdParam->channelNameOverlay);

                /* Add text and position if overlay is enabled */
                if (pOsdParam->channelNameOverlay == ENABLE)
                {
                    /* New 6 config fields added for 6 text overlay camera (8MP). Remaining will work with older config fields */
                    if (pOsdParam->channelNameOverlayMax > TEXT_OVERLAY_MIN)
                    {
                        for (loop = 0; loop < pOsdParam->channelNameOverlayMax; loop++)
                        {
                            outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d%c%d%c%s%c%d%c%d",
                                               FSP, CI_OVERLAY_CFG_TXT_ACTIVE1 + (loop * 3), FVS, ACTIVE,
                                               FSP, CI_OVERLAY_CFG_TXT1 + (loop * 3), FVS, pOsdParam->channelName[loop],
                                               FSP, CI_OVERLAY_CFG_TXT_POS1 + (loop * 3), FVS, pOsdParam ->channelNamePos[loop] - 1);
                        }
                    }
                    else
                    {
                        outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%s%c%d%c%d",
                                           FSP, CI_OVERLAY_CFG_TXT0, FVS, pOsdParam->channelName[0],
                                           FSP, CI_OVERLAY_CFG_TXT_POS0, FVS, pOsdParam ->channelNamePos[0] - 1);
                    }
                }
            }

            snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%c%c%c", FSP, EOI, EOT, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_PRIVACY_MASK:
        {
            CAMERA_BRAND_e          brand = CAMERA_BRAND_NONE;
            CAMERA_MODEL_e          model = MATRIX_MODEL_NONE;
            BOOL                    isMaskEnable = cmdArg;
            UINT8                   maskCnt;
            UINT8                   totalPrivacyMask;
            PRIVACY_MASK_CONFIG_t   *pPrivacyMask = (PRIVACY_MASK_CONFIG_t*)pInArg;

            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_CFG;
            outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d", SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_OVERLAY_CFG, FSP, SOI, 0);

            GetCiCameraBrandModelNumber(cameraIndex, &brand, &model);
            if ((brand != CAMERA_BRAND_MATRIX) || (model == MATRIX_MODEL_NONE) || (model >= MAX_MATRIX_CAMERA_MODEL))
            {
                break;
            }

            totalPrivacyMask = GetMaxSupportedPrivacyMaskWindow(brand, model);
            DPRINT(CAMERA_INTERFACE, "set ci privacy mask: [camera=%d], [totalMask=%d]", cameraIndex, totalPrivacyMask);
            outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_OVERLAY_CFG_ENBPRIMASK, FVS, isMaskEnable);

            if (isMaskEnable == ENABLE)
            {
                /* Send only maximum supported privacy mask */
                for (maskCnt = 0; maskCnt < totalPrivacyMask; maskCnt++)
                {
                    isMaskEnable = ENABLE;
                    if ((pPrivacyMask[maskCnt].startXPoint == 0) && (pPrivacyMask[maskCnt].startYPoint == 0) &&
                            (pPrivacyMask[maskCnt].width == 0) && (pPrivacyMask[maskCnt].height == 0))
                    {
                        isMaskEnable = DISABLE;
                    }

                    /* Derive the config field offset */
                    outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d", FSP, CI_OVERLAY_CFG_ENBMASK1 + (maskCnt * 5), FVS, isMaskEnable);
                    if (isMaskEnable == DISABLE)
                    {
                        continue;
                    }

                    outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
                                       FSP, CI_OVERLAY_CFG_STRTXMASK1 + (maskCnt * 5), FVS, pPrivacyMask[maskCnt].startXPoint,
                                       FSP, CI_OVERLAY_CFG_STRTYMASK1 + (maskCnt * 5), FVS, pPrivacyMask[maskCnt].startYPoint,
                                       FSP, CI_OVERLAY_CFG_WDTHMASK1 + (maskCnt * 5), FVS, pPrivacyMask[maskCnt].width,
                                       FSP, CI_OVERLAY_CFG_HTMASK1 + (maskCnt * 5), FVS, pPrivacyMask[maskCnt].height);
                }
            }

            snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%c%c%c", FSP, EOI, EOT, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_MOTION_WINDOW:
        {
            UINT8                       tempCnt = 0;
            CHAR                        **grid44_36Ptr;
            CHAR                        packedGridBytes[512];
            CHAR                        packedString[(MOTION_AREA_BLOCK_BYTES*2)+1] = {0};
            UINT16                      packedLen = 0;
            MOTION_BLOCK_METHOD_PARAM_t *pMotionParam = (MOTION_BLOCK_METHOD_PARAM_t*)pInArg;

            grid44_36Ptr = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
            if (NULL != grid44_36Ptr)
            {
                UnPackGridGeneral(pMotionParam->blockBitString, 44, 36, grid44_36Ptr, MOTION_AREA_BLOCK_BYTES);
                PackGridGeneral(grid44_36Ptr, 44, 36, packedGridBytes, &packedLen);
                Free2DArray((void**)grid44_36Ptr, 36);

                for(tempCnt = 0; tempCnt < MOTION_AREA_BLOCK_BYTES; tempCnt++)
                {
                    snprintf(&packedString[tempCnt*2], sizeof(packedString), "%02x", (UINT8)(packedGridBytes[tempCnt]));
                }
                packedString[tempCnt*2] = '\0';
            }

            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_CFG;
            outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d", SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_MOTION_DETECT_CFG, FSP, SOI, 0);

            outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d%c%d%c%s%c%d%c%d",
                               FSP, CI_MOTION_CFG_ENB, FVS, ENABLE,
                               FSP, CI_MOTION_CFG_CELL, FVS, packedString,
                               FSP, CI_MOTION_CFG_SENS, FVS, (pMotionParam->sensitivity * 10));

            if (YES == IsNoMotionEventSupported(cameraIndex))
            {
                outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d%c%d%c%d",
                                   FSP, CI_MOTION_CFG_IS_NO_MOTION_EVNT, FVS, pMotionParam->isNoMotionEvent,
                                   FSP, CI_MOTION_CFG_NO_MOTION_DURATION, FVS, pMotionParam->noMotionDuration);
            }

            snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%c%c%c", FSP, EOI, EOT, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_IMAGE_SETTING:
        {
            IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = (IMAGE_CAPABILITY_INFO_t*)pInArg;

            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_CFG;
            switch(reqType)
            {
                case CAM_REQ_IMG_APPEARANCE:
                {
                    outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d", SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_APPEARANCE_CFG, FSP, SOI, 0);

                    outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d",
                                       FSP, CI_APPEARANCE_CNFG_BRIGHTNESS, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->brightness.value, pImageCapsInfo->brightness.step, pImageCapsInfo->brightness.min, BRIGHTNESS_MIN),
                                       FSP, CI_APPEARANCE_CNFG_CONSTRAST, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->contrast.value, pImageCapsInfo->contrast.step, pImageCapsInfo->contrast.min, CONTRAST_MIN),
                                       FSP, CI_APPEARANCE_CNFG_SATURATION, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->saturation.value, pImageCapsInfo->saturation.step, pImageCapsInfo->saturation.min, SATURATION_MIN),
                                       FSP, CI_APPEARANCE_CNFG_HUE, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->hue.value, pImageCapsInfo->hue.step, pImageCapsInfo->hue.min, HUE_MIN),
                                       FSP, CI_APPEARANCE_CNFG_SHARPNESS, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->sharpness.value, pImageCapsInfo->sharpness.step, pImageCapsInfo->sharpness.min, SHARPNESS_MIN),
                                       FSP, CI_APPEARANCE_CNFG_WHITE_BALANCE, FVS, pImageCapsInfo->whiteBalance.mode);

                    snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%c%c%c", FSP, EOI, EOT, EOM);
                    pCamReqInfo->numOfRequest = ++reqCnt;
                }
                break;

                case CAM_REQ_IMG_ADV_APPEARANCE:
                {
                    outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d", SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_ADV_APPEARANCE_CFG, FSP, SOI, 0);

                    outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d" "%c%d%c%d",
                                       FSP, CI_ADV_APPEARANCE_CNFG_WDR, FVS, pImageCapsInfo->wdr.mode,
                                       FSP, CI_ADV_APPEARANCE_CNFG_EXPOSURE_RATIO_MODE, FVS, pImageCapsInfo->exposureRatioMode.mode,
                                       FSP, CI_ADV_APPEARANCE_CNFG_EXPOSURE_RATIO, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureRatio.value, pImageCapsInfo->exposureRatio.step, pImageCapsInfo->exposureRatio.min, EXPOSURE_RATIO_MIN),
                                       FSP, CI_ADV_APPEARANCE_CNFG_EXPOSURE_FLICKER, FVS, pImageCapsInfo->flicker.mode,
                                       FSP, CI_ADV_APPEARANCE_CNFG_EXPOSURE_MODE, FVS, pImageCapsInfo->exposureMode.mode,
                                       FSP, CI_ADV_APPEARANCE_CNFG_EXPOSURE_GAIN, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureGain.value, pImageCapsInfo->exposureGain.step, pImageCapsInfo->exposureGain.min, EXPOSURE_GAIN_MIN),
                                       FSP, CI_ADV_APPEARANCE_CNFG_EXPOSURE_IRIS, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureIris.value, pImageCapsInfo->exposureIris.step, pImageCapsInfo->exposureIris.min, EXPOSURE_IRIS_MIN),
                                       FSP, CI_ADV_APPEARANCE_CNFG_NORMAL_LIGHT_GAIN, FVS, pImageCapsInfo->normalLightGain.mode,
                                       FSP, CI_ADV_APPEARANCE_CNFG_NORMAL_LIGHT_LUMINANCE, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->normalLightLuminance.value, pImageCapsInfo->normalLightLuminance.step, pImageCapsInfo->normalLightLuminance.min, NORMAL_LIGHT_LUMINANCE_MIN),
                                       FSP, CI_ADV_APPEARANCE_CNFG_BACKLIGHT, FVS, pImageCapsInfo->backlightControl.mode,
                                       FSP, CI_ADV_APPEARANCE_CNFG_DWDR_STRENGTH, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->wdrStrength.value, pImageCapsInfo->wdrStrength.step, pImageCapsInfo->wdrStrength.min, WDR_STRENGTH_MIN));

                    snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%c%c%c", FSP, EOI, EOT, EOM);
                    pCamReqInfo->numOfRequest = ++reqCnt;
                }
                break;

                case CAM_REQ_IMG_DAY_NIGHT:
                {
                    outLen = snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%c%d", SOM, CIReqHeader[CI_SET_CFG], FSP, SOT, CI_DAY_NIGHT_CFG, FSP, SOI, 0);

                    outLen += snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%d%c%d" "%c%d%c%d",
                                       FSP, CI_DAY_NIGHT_CNFG_LED_SENSITIVITY, FVS, (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->irLedSensitivity.value, pImageCapsInfo->irLedSensitivity.step, pImageCapsInfo->irLedSensitivity.min, LED_SENSITIVITY_MIN),
                                       FSP, CI_DAY_NIGHT_CNFG_LED_MODE, FVS, pImageCapsInfo->irLed.mode);

                    snprintf(reqMesStr+outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c%c%c%c", FSP, EOI, EOT, EOM);
                    pCamReqInfo->numOfRequest = ++reqCnt;
                }
                break;

                default:
                {
                    EPRINT(CAMERA_INTERFACE, "invld set image setting request type: [camera=%d], [reqType=%d]", cameraIndex, reqType);
                }
                break;
            }
        }
        break;

        case REQ_URL_GET_STREAM_CFG:
        {
            /* cmdArg=from index, cmdArg=to index, 0=from field, 10=to field */
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_GET_CFG;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%d%c%d%c%d%c%d%c%c%c",
                     SOM, CIReqHeader[CI_GET_CFG], FSP, SOT, CI_STREAM_PROFILE_CFG, FSP, cmdArg, FSP, cmdArg, FSP, 0, FSP, 10, FSP, EOT, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_GET_PRIVACY_MASK:
        {
            /* for 3 privacy masks toField=24 & for 8 privacy masks toField=49 */
            CAMERA_BRAND_e  brand = 0;
            CAMERA_MODEL_e  model = 0;
            GetCiCameraBrandModelNumber(cameraIndex, &brand, &model);

            /* initial 9 fields + 5 fields for each privacy mask */
            /* 0=from index, 0=to index, 0=from field, (9 + (GetMaxSupportedPrivacyMaskWindow(brand, model) * 5))=to field */
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_GET_CFG;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%d%c%d%c%d%c%d%c%c%c",
                     SOM, CIReqHeader[CI_GET_CFG], FSP, SOT, CI_OVERLAY_CFG,
                     FSP, 0, FSP, 0, FSP, 0, FSP, (9 + (GetMaxSupportedPrivacyMaskWindow(brand, model) * 5)), FSP, EOT, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_GET_MOTION_WINDOW:
        {
            /* 0=from index, 0=to index, 2=from field, ((YES == IsNoMotionEventSupported(cameraIndex)) ? 6 : 4)=to field */
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_GET_CFG;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%d%c%d%c%d%c%d%c%c%c",
                     SOM, CIReqHeader[CI_GET_CFG], FSP, SOT, CI_MOTION_DETECT_CFG,
                     FSP, 0, FSP, 0, FSP, 2, FSP, (YES == IsNoMotionEventSupported(cameraIndex)) ? 6 : 4, FSP, EOT, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_GET_IMAGE_SETTING:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_GET_CFG;
            switch(reqType)
            {
                case CAM_REQ_IMG_APPEARANCE:
                {
                    /* 0=from index, 0=to index, 0=from field, 9=to field */
                    snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%d%c%d%c%d%c%d%c%c%c",
                             SOM, CIReqHeader[CI_GET_CFG], FSP, SOT, CI_APPEARANCE_CFG, FSP, 0, FSP, 0,
                             FSP, CI_APPEARANCE_CNFG_BRIGHTNESS, FSP, (CI_APPEARANCE_CNFG_MAX - 1), FSP, EOT, EOM);
                    pCamReqInfo->numOfRequest = ++reqCnt;
                }
                break;

                case CAM_REQ_IMG_ADV_APPEARANCE:
                {
                    /* 0=from index, 0=to index, 0=from field, 1=to field */
                    snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%d%c%d%c%d%c%d%c%c%c",
                             SOM, CIReqHeader[CI_GET_CFG], FSP, SOT, CI_ADV_APPEARANCE_CFG, FSP, 0, FSP, 0,
                             FSP, CI_ADV_APPEARANCE_CNFG_WDR, FSP, (CI_ADV_APPEARANCE_CNFG_MAX - 1), FSP, EOT, EOM);
                    pCamReqInfo->numOfRequest = ++reqCnt;
                }
                break;

                case CAM_REQ_IMG_DAY_NIGHT:
                {
                    /* 0=from index, 0=to index, 0=from field, 18=to field */
                    snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%c%d%c%d%c%d%c%d%c%d%c%c%c",
                             SOM, CIReqHeader[CI_GET_CFG], FSP, SOT, CI_DAY_NIGHT_CFG, FSP, 0, FSP, 0,
                             FSP, CI_DAY_NIGHT_CNFG_LED_SENSITIVITY, FSP, (CI_DAY_NIGHT_CNFG_MAX - 1), FSP, EOT, EOM);
                    pCamReqInfo->numOfRequest = ++reqCnt;
                }
                break;

                default:
                {
                    EPRINT(CAMERA_INTERFACE, "invld get image setting request type: [camera=%d], [reqType=%d]", cameraIndex, reqType);
                }
                break;
            }
        }
        break;

        case REQ_URL_START_STREAM:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SRT_LV_STRM;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_SRT_LV_STRM], FSP, cmdArg, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_STOP_STREAM:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_STP_LV_STRM;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_STP_LV_STRM], FSP, cmdArg, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_START_AUDIO:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SRT_AUD_OUT;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_SRT_AUD_OUT], FSP, cmdArg, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_STOP_AUDIO:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_STP_AUD_OUT;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_STP_AUD_OUT], FSP, cmdArg, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_GET_IMAGE:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SNP_SHT;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_SNP_SHT], FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_GET_IMAGE_CAPABILITY:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_GET_IMG_SET_CAP;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_GET_IMG_SET_CAP], FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_ALARM:
        {
            UINT8 *alarmIndex = (UINT8*)pInArg;
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_ALARM_OUTPUT;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%03d%c%03d%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD],
                     FSP, CIReqHeader[CI_ALARM_OUTPUT], FSP, cmdArg, FSP, 0, FSP, *alarmIndex, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_STORE_PTZ:
        {
            PTZ_PRESET_INFO_t *presetInfo = (PTZ_PRESET_INFO_t*)pInArg;
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_UPDATE_PRESET;

            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%d%c%d%c%s%c%c", SOM, CIReqHeader[CI_SET_CMD],
                     FSP, CIReqHeader[CI_UPDATE_PRESET], FSP, ((presetInfo->action == ADDED) ? 0 : 1), FSP, presetInfo->presetIndex, FSP,
                     1, FSP, presetInfo->presetName, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_GO_TO_PTZ:
        {
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_CALL_PRESET;

            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD],
                     FSP, CIReqHeader[CI_CALL_PRESET], FSP, cmdArg, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_PTZ:
        {
            PTZ_MSG_t   *ptzMsg = (PTZ_MSG_t*)pInArg;
            UINT8       operation;

            if (ptzMsg->zoom != MAX_PTZ_ZOOM_OPTION)
            {
                pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_ZOOM;

                snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%d%c%d%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD],
                         FSP, CIReqHeader[CI_SET_ZOOM], FSP, ptzMsg->zoom, FSP, 1, FSP,
                         (ptzMsg->speed * (MATRIX_CAMERA_SPEED_MAX / MAX_PTZ_SPEED_STEPS)), FSP, ptzMsg->action, FSP, EOM);
            }
            else
            {
                pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_PANTILT;

                if ((ptzMsg->pan != MAX_PTZ_PAN_OPTION) && (ptzMsg->tilt != MAX_PTZ_TILT_OPTION))
                {
                    if ((ptzMsg->pan == PTZ_PAN_LEFT) && (ptzMsg->tilt == PTZ_TILT_UP))
                    {
                        operation = MATRIX_PTZ_ACTION_TOP_LEFT;
                    }
                    else if ((ptzMsg->pan == PTZ_PAN_RIGHT) && (ptzMsg->tilt == PTZ_TILT_UP))
                    {
                        operation = MATRIX_PTZ_ACTION_TOP_RIGHT;
                    }
                    else if ((ptzMsg->pan == PTZ_PAN_RIGHT) && (ptzMsg->tilt == PTZ_TILT_DOWN))
                    {
                        operation = MATRIX_PTZ_ACTION_BOTTOM_RIGHT;
                    }
                    else
                    {
                        operation = MATRIX_PTZ_ACTION_BOTTOM_LEFT;
                    }
                }
                else
                {
                    if (ptzMsg->pan != MAX_PTZ_PAN_OPTION)
                    {
                        operation = (ptzMsg->pan == PTZ_PAN_LEFT) ? MATRIX_PTZ_ACTION_LEFT : MATRIX_PTZ_ACTION_RIGHT;
                    }
                    else
                    {
                        operation = (ptzMsg->tilt == PTZ_TILT_UP) ? MATRIX_PTZ_ACTION_TOP : MATRIX_PTZ_ACTION_BOTTOM;
                    }
                }

                snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%d%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD],
                         FSP, CIReqHeader[CI_SET_PANTILT], FSP, operation, FSP,
                         (ptzMsg->speed * (MATRIX_CAMERA_SPEED_MAX / MAX_PTZ_SPEED_STEPS)), FSP, ptzMsg->action, FSP, EOM);
            }

            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_FOCUS:
        {
            PTZ_MSG_t   *ptzMsg = (PTZ_MSG_t*)pInArg;
            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_FOCUS;

            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%d%c%d%c%d%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD],
                     FSP, CIReqHeader[CI_SET_FOCUS], FSP, ptzMsg->focus, FSP, 1, FSP,
                     (ptzMsg->speed * (MATRIX_CAMERA_SPEED_MAX / MAX_PTZ_SPEED_STEPS)), FSP, ptzMsg->action, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        case REQ_URL_SET_DATE_TIME:
        {
            struct tm localDateTime;
            if (SUCCESS != GetLocalTimeInBrokenTm(&localDateTime))
            {
                EPRINT(CAMERA_INTERFACE, "fail to get local time in broken time: [camera=%d]", cameraIndex);
                memset(&localDateTime, 0, sizeof(localDateTime));
            }

            pCamReqInfo->url[reqCnt].tcpRequestType = CI_SET_DATE_TIME;
            snprintf(reqMesStr, MAX_CAMERA_URI_WIDTH, "%c%s%c%s%c%02d%02d%04d%02d%02d%02d%c%c", SOM, CIReqHeader[CI_SET_CMD],
                     FSP, CIReqHeader[CI_SET_DATE_TIME], FSP,localDateTime.tm_mday, localDateTime.tm_mon+1,
                    localDateTime.tm_year, localDateTime.tm_hour, localDateTime.tm_min, localDateTime.tm_sec, FSP, EOM);
            pCamReqInfo->numOfRequest = ++reqCnt;
        }
        break;

        default:
        {
            EPRINT(CAMERA_INTERFACE, "invld url request type: [camera=%d], [reqType=%d]", cameraIndex, reqType);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera brand and model as well as url
 * @param   cameraIndex
 * @param   reqType
 * @param   pCamReqInfo
 * @param   pInArg1
 * @param   pInArg2
 * @param   pInArg3
 * @param   pInArg4
 * @return  Status
 */
NET_CMD_STATUS_e GetCameraBrandModelUrl(UINT8 cameraIndex, REQ_URL_e reqType, CAMERA_REQUEST_t *pCamReqInfo,
                                        VOIDPTR pInArg1, VOIDPTR pInArg2, VOIDPTR pInArg3, VOIDPTR pInArg4)
{
    CAMERA_BRAND_e  brand = CAMERA_BRAND_NONE;
    CAMERA_MODEL_e  model = CAMERA_MODEL_NONE;

    /* Get camera brand and model */
    GetCameraBrandModelNumber(cameraIndex, &brand, &model);

    /* No URL for generic brand */
    if (brand == CAMERA_BRAND_GENERIC)
    {
        EPRINT(CAMERA_INTERFACE, "feature not supported for generic brand: [camera=%d], [reqType=%d]", cameraIndex, reqType);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    /* Validate camera brand and model */
    if (ValidateCamBrandModel(brand, model) == FAIL)
    {
        EPRINT(CAMERA_INTERFACE, "fail to validate brand-model: [camera=%d], [reqType=%d], [brand=%d], [model=%d]", cameraIndex, reqType, brand, model);
        return CMD_CAM_PARAM_NOT_CONFIGURED;
    }

    /* Get the request url for assigned camera with brand and model */
    return prepareBmReqestUrl(cameraIndex, brand, model, reqType, pCamReqInfo, pInArg1, pInArg2, pInArg3, pInArg4);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get request url for brand and model
 * @param   cameraIndex
 * @param   brand
 * @param   model
 * @param   reqType
 * @param   pCamReqInfo
 * @param   pInArg1
 * @param   pInArg2
 * @param   pInArg3
 * @return  Status
 */
NET_CMD_STATUS_e GetBrandModelReqestUrl(UINT8 cameraIndex, CAMERA_BRAND_e brand, CAMERA_MODEL_e model, REQ_URL_e reqType,
                                        CAMERA_REQUEST_t *pCamReqInfo, VOIDPTR pInArg1, VOIDPTR pInArg2, VOIDPTR pInArg3)
{
    /* Get the request url for assigned/unassigned camera with specific brand and model */
    return prepareBmReqestUrl(cameraIndex, brand, model, reqType, pCamReqInfo, pInArg1, pInArg2, pInArg3, NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare request url for brand and model
 * @param   cameraIndex
 * @param   brand
 * @param   model
 * @param   reqType
 * @param   pCamReqInfo
 * @param   pInArg1
 * @param   pInArg2
 * @param   pInArg3
 * @param   pInArg4
 * @return  Status
 */
static NET_CMD_STATUS_e prepareBmReqestUrl(UINT8 cameraIndex, CAMERA_BRAND_e brand, CAMERA_MODEL_e model, REQ_URL_e reqType,
                                           CAMERA_REQUEST_t *pCamReqInfo, VOIDPTR pInArg1, VOIDPTR pInArg2, VOIDPTR pInArg3, VOIDPTR pInArg4)
{
    NET_CMD_STATUS_e cmdResp = CMD_SUCCESS;

    /* Get url function for brand */
    URL_FUNCTION_t *pGetUrl = GetUrl(brand);
    if (pGetUrl == NULL)
    {
        EPRINT(CAMERA_INTERFACE, "feature not supported: [camera=%d], [reqType=%d], [brand=%d], [model=%d]", cameraIndex, reqType, brand, model);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    switch(reqType)
    {
        case REQ_URL_GET_STREAM_CFG:
        {
            if (pGetUrl->toGetCurrStrmCfg == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "get stream config not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            VIDEO_TYPE_e    streamType = *(VIDEO_TYPE_e*)pInArg1;
            UINT8           profileIndex = *(UINT8*)pInArg2;

            /* Prepare get stream parameter url */
            cmdResp = pGetUrl->toGetCurrStrmCfg(model, profileIndex, pCamReqInfo->url, &pCamReqInfo->numOfRequest, streamType);
        }
        break;

        case REQ_URL_GET_STREAM:
        {
            if (pGetUrl->toGetStream == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "get stream not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            VIDEO_TYPE_e    streamType = *(VIDEO_TYPE_e*)pInArg1;
            STREAM_CONFIG_t *pStreamConfig = (STREAM_CONFIG_t*)pInArg2;
            BOOL            localForceConfigOnCamera = *(BOOL*)pInArg3;

            /* Prepare get stream url */
            cmdResp = pGetUrl->toGetStream(model, pStreamConfig, pCamReqInfo->url, &pCamReqInfo->numOfRequest, localForceConfigOnCamera, streamType);
        }
        break;

        case REQ_URL_SET_DATE_TIME:
        {
            if (pGetUrl->toSetDateTime == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set date-time not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            /* Prepare set date-time url */
            cmdResp = pGetUrl->toSetDateTime(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_OSD:
        {
            if (pGetUrl->toSetOsd == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set osd not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            OSD_PARAM_t *pOsdParam = (OSD_PARAM_t*)pInArg1;

            /* Prepare set osd url */
            cmdResp = pGetUrl->toSetOsd(model, pOsdParam, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_GET_EVENT:
        {
            if (pGetUrl->toReqEvent == NULL)
            {
                EPRINT(CAM_EVENT, "get event not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            CAMERA_EVENT_REQUEST_t  *pEventRequest = (CAMERA_EVENT_REQUEST_t*)pInArg1;
            CAMERA_EVENT_e          camEvent = *(CAMERA_EVENT_e*)pInArg2;
            EVENT_RESP_INFO_t       *pEventRespInfo = (EVENT_RESP_INFO_t*)pInArg3;

            /* Prepare get event url */
            cmdResp = pGetUrl->toReqEvent(model, camEvent, pEventRequest->url, &pEventRequest->numOfRequest, pEventRespInfo);
        }
        break;

        case REQ_URL_GET_IMAGE:
        {
            if (pGetUrl->toGetImage == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "get image not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            /* Prepare get image url */
            cmdResp = pGetUrl->toGetImage(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_START_AUDIO:
        {
            if (pGetUrl->toSendAudio == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "two way audio feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            AUD_TO_CAM_INFO_t   *pAudioToCamera = (AUD_TO_CAM_INFO_t*)pInArg1;
            SEND_AUDIO_INFO_t   *pSendAudioInfo = (SEND_AUDIO_INFO_t*)pInArg2;

            /* Prepare two way audio url */
            cmdResp = pGetUrl->toSendAudio(model, pAudioToCamera->url, &pAudioToCamera->noOfUrlReq, &pAudioToCamera->noOfStopReq, pSendAudioInfo);
        }
        break;

        case REQ_URL_SET_ALARM:
        {
            if (pGetUrl->toSetAlarm == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set alarm feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            UINT8   alarmIndex = *(UINT8*)pInArg1;
            BOOL    requestType = *(BOOL*)pInArg2;

            /* Prepare set alarm url */
            cmdResp = pGetUrl->toSetAlarm(model, alarmIndex, requestType, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_PASSWORD:
        {
            if (pGetUrl->toSetPasswd == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set password not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            CHARPTR pUsername = (CHARPTR)pInArg1;
            CHARPTR pPassword = (CHARPTR)pInArg2;

            /* Prepare set camera password url */
            cmdResp = pGetUrl->toSetPasswd(model, pUsername, pPassword, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_CHANGE_CAMERA_ADDR:
        {
            if (pGetUrl->toChangeCamIpAddr == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "change camera addr not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            IP_ADDR_PARAM_t *pNetworkParam = (IP_ADDR_PARAM_t*)pInArg1;

            /* Prepare change camera address url */
            cmdResp = pGetUrl->toChangeCamIpAddr(model, pNetworkParam, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_GET_MAX_PRIVACY_MASK:
        {
            if (pGetUrl->toGetMaxPrivacyMaskWindow != NULL)
            {
                /* Prepare get maximum supported privacy mask window url */
                cmdResp = pGetUrl->toGetMaxPrivacyMaskWindow(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
                break;
            }
        }

        /* FALL THROUGH */
        case REQ_URL_GET_PRIVACY_MASK:
        {
            if (pGetUrl->toGetPrivacyMask == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "privacy mask not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            /* It is usefull when fall through from max privacy mask to get privacy mask */
            UINT8 *pPrivacyMaskMax = (UINT8*)pInArg1;
            if (pPrivacyMaskMax)
            {
                /* Get max supported privacy mask for brand-model */
                *pPrivacyMaskMax = GetMaxSupportedPrivacyMaskWindow(brand, model);
            }

            /* Prepare get privacy mask url */
            cmdResp = pGetUrl->toGetPrivacyMask(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_PRIVACY_MASK:
        {
            if (pGetUrl->toSetPrivacyMask == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "privacy mask not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            if (CheckforPrivacyMaskSupport(cameraIndex) != PRIVACY_MASK_METHOD_POINT)
            {
                EPRINT(CAMERA_INTERFACE, "no analytics support for privacy mask: [camera=%d]", cameraIndex);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            BOOL                    privacyMaskStatus = *(BOOL*)pInArg1;
            PRIVACY_MASK_CONFIG_t   *pPrivacyMask = (PRIVACY_MASK_CONFIG_t*)pInArg2;
            CHARPTR                 *pPrivacyMaskName = (CHARPTR*)pInArg3;

            if (FALSE == IsPrivacyMaskMappingRequired(brand, model))
            {
                EPRINT(CAMERA_INTERFACE, "no privacy mask support for OEM: [camera=%d]", cameraIndex);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            if (FAIL == MapPrivacyMaskNvrToCamera(brand, model, pPrivacyMask))
            {
                EPRINT(CAMERA_INTERFACE, "fail to map privacy mask with resolution: [camera=%d]", cameraIndex);
                return CMD_PROCESS_ERROR;
            }

            /* Prepare set privacy mask url */
            cmdResp = pGetUrl->toSetPrivacyMask(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest, pPrivacyMask, privacyMaskStatus, pPrivacyMaskName);
        }
        break;

        case REQ_URL_GET_MOTION_WINDOW:
        {
            if (pGetUrl->toGetMotionWindow == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "motion window not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            /* Prepare get motion window url */
            cmdResp = pGetUrl->toGetMotionWindow(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_MOTION_WINDOW:
        {
            if (pGetUrl->toSetMotionWindow == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "motion window not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            MOTION_BLOCK_METHOD_PARAM_t *pMotionBlock = (MOTION_BLOCK_METHOD_PARAM_t *)pInArg1;

            /* Prepare set motion window url */
            cmdResp = pGetUrl->toSetMotionWindow(model, pMotionBlock, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_STORE_PTZ:
        {
            if (pGetUrl->toStorePtz == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "store ptz feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            UINT8   ptzIndex = *(UINT8*)pInArg1;
            BOOL    presetAction = *(BOOL*)pInArg2;
            CHARPTR presetName = (CHARPTR)pInArg3;

            /* Prepare store ptz url */
            cmdResp = pGetUrl->toStorePtz(model, ptzIndex, presetName, presetAction, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_GO_TO_PTZ:
        {
            if (pGetUrl->toGotoPtz == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "goto feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            UINT8 ptzIndex = *(UINT8*)pInArg1;

            /* Prepare goto ptz url */
            cmdResp = pGetUrl->toGotoPtz(model, ptzIndex, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_PTZ:
        {
            if (pGetUrl->toSetPtz == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set ptz feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            if (pInArg1 == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "missing ptz information: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_PROCESS_ERROR;
            }

            PTZ_MSG_t   *ptzMsg = (PTZ_MSG_t*)pInArg1;

            /* Prepare set ptz url */
            cmdResp = pGetUrl->toSetPtz(model, ptzMsg->pan, ptzMsg->tilt, ptzMsg->zoom, ptzMsg->action, ptzMsg->speed, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_FOCUS:
        {
            if (pGetUrl->toSetFocus == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set focus feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            PTZ_MSG_t   *ptzMsg = (PTZ_MSG_t*)pInArg1;

            /* Prepare set focus url */
            cmdResp = pGetUrl->toSetFocus(model, ptzMsg->focus, ptzMsg->action, ptzMsg->speed, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_SET_IRIS:
        {
            if (pGetUrl->toSetIris == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "set iris feature not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            PTZ_MSG_t   *ptzMsg = (PTZ_MSG_t*)pInArg1;

            /* Prepare set iris url */
            cmdResp = pGetUrl->toSetIris(model, ptzMsg->iris, ptzMsg->action, pCamReqInfo->url, &pCamReqInfo->numOfRequest);
        }
        break;

        case REQ_URL_GET_IMAGE_CAPABILITY:
        case REQ_URL_GET_IMAGE_SETTING:
        case REQ_URL_SET_IMAGE_SETTING:
        {
            if (pGetUrl->toImageSetting == NULL)
            {
                EPRINT(CAMERA_INTERFACE, "image setting not supported: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                return CMD_FEATURE_NOT_SUPPORTED;
            }

            IMAGE_SETTING_ACTION_e  imageAction;
            IMAGE_CAPABILITY_INFO_t *pImageCapInfo = (IMAGE_CAPABILITY_INFO_t*)pInArg1;

            if (reqType == REQ_URL_GET_IMAGE_CAPABILITY)
            {
                imageAction = IMAGE_SETTING_ACTION_GET_CAPABILITY;
            }
            else if (reqType == REQ_URL_GET_IMAGE_SETTING)
            {
                imageAction = IMAGE_SETTING_ACTION_GET_PARAM;
            }
            else
            {
                imageAction = IMAGE_SETTING_ACTION_SET_PARAM;
            }

            /* Prepare get image setting capability url */
            pCamReqInfo->numOfRequest = 0;
            cmdResp = pGetUrl->toImageSetting(model, pCamReqInfo->url, &pCamReqInfo->numOfRequest, imageAction, pImageCapInfo);
        }
        break;

        default:
        {
            cmdResp = CMD_PROCESS_ERROR;
            EPRINT(CAMERA_INTERFACE, "invld url request type: [camera=%d], [reqType=%d], [brand=%d], [model=%d]", cameraIndex, reqType, brand, model);
        }
        break;
    }

    /* URL generation status */
    return cmdResp;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get brand device inforamtion
 * @param   brand
 * @param   model
 * @param   pReqestUrl
 * @param   pNumOfRequest
 * @return  SUCCESS/FAIL
 */
BOOL GetBrandDeviceInfo(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, URL_REQUEST_t *pReqestUrl, UINT8 *pNumOfRequest)
{
    if ((brand == CAMERA_BRAND_NONE) && (brand >= MAX_CAMERA_BRAND))
    {
        EPRINT(CAMERA_INTERFACE, "invld brand number: [brandNum=%d]", brand);
        return FAIL;
    }

    /* Get url function for brand */
    URL_FUNCTION_t *pGetUrl = GetUrl(brand);
    if ((pGetUrl == NULL) || (pGetUrl->toGetDeviceInfo == NULL))
    {
        EPRINT(CAMERA_INTERFACE, "get device info not supported: [brand=%d]", brand);
        return FAIL;
    }

    /* Get camera brand and model device information url */
    if (pGetUrl->toGetDeviceInfo(model, pReqestUrl, pNumOfRequest) != CMD_SUCCESS)
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Mapping privacy mask with given resolution
 * @param   brand
 * @param   model
 * @param   pPrivacyMask
 * @param   maskWindowMax
 * @return
 */
BOOL MapPrivacyMaskNvrToCamera(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PRIVACY_MASK_CONFIG_t *pPrivacyMask)
{
    UINT8   regionCnt = 0;
    UINT32  windowWidth = 0;
    UINT32  windowHeight = 0;

    /* Is privacy mask camera resolution available? */
    if (FAIL == getPrivacyMaskCameraResolution(brand, model, &windowWidth, &windowHeight))
    {
        return FAIL;
    }

    while(regionCnt < MAX_PRIVACY_MASKS)
    {
        pPrivacyMask[regionCnt].startXPoint = GET_CAMERA_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].startXPoint, SCALING_WINDOW_WIDTH, windowWidth);
        pPrivacyMask[regionCnt].startYPoint = GET_CAMERA_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].startYPoint, SCALING_WINDOW_HEIGHT, windowHeight);
        pPrivacyMask[regionCnt].width = GET_CAMERA_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].width, SCALING_WINDOW_WIDTH, windowWidth);
        pPrivacyMask[regionCnt].height = GET_CAMERA_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].height, SCALING_WINDOW_HEIGHT, windowHeight);
        regionCnt++;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Mapping privacy mask cordinates from camera to NVR
 * @param   brand
 * @param   model
 * @param   pPrivacyMask
 * @return  SUCCESS/FAIL
 */
BOOL MapPrivacyMaskCameraToNvr(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PRIVACY_MASK_CONFIG_t *pPrivacyMask)
{
    UINT8   regionCnt = 0;
    UINT32  windowWidth = 0;
    UINT32  windowHeight = 0;

    /* Is privacy mask camera resolution available? */
    if (FAIL == getPrivacyMaskCameraResolution(brand, model, &windowWidth, &windowHeight))
    {
        return FAIL;
    }

    while(regionCnt < MAX_PRIVACY_MASKS)
    {
        pPrivacyMask[regionCnt].startXPoint = GET_NVR_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].startXPoint, SCALING_WINDOW_WIDTH, windowWidth);
        pPrivacyMask[regionCnt].startYPoint = GET_NVR_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].startYPoint, SCALING_WINDOW_HEIGHT, windowHeight);
        pPrivacyMask[regionCnt].width = GET_NVR_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].width, SCALING_WINDOW_WIDTH, windowWidth);
        pPrivacyMask[regionCnt].height = GET_NVR_MAPPED_WINDOW_SIZE(pPrivacyMask[regionCnt].height, SCALING_WINDOW_HEIGHT, windowHeight);
        regionCnt++;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get privacy camera resolution for mapping with NVR resolution
 * @param   brand
 * @param   model
 * @param   pWindowWidth
 * @param   pWindowHeight
 * @return  Returns TRUE if resolution available else returns FALSE
 * @note    NVR resolution is D1 and matrix camera resolution is FHD.
 */
static BOOL getPrivacyMaskCameraResolution(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, UINT32PTR pWindowWidth, UINT32PTR pWindowHeight)
{
    /* Only Matrix brand have privacy mask support excluding OEM cameras */
    if (brand != CAMERA_BRAND_MATRIX)
    {
        return FAIL;
    }

    /* Is valid matrix sdk available? */
    MATRIX_SDK_NUM_e matrixSdk = GetSdkNumForMatrix(model);
    if (matrixSdk >= MAX_MATRIX_SDK)
    {
        return FAIL;
    }

    /* Is matrix sdk have valid resolution? */
    if ((matrixFixedWindowResolution[matrixSdk][FRAME_WIDTH] == 0) || (matrixFixedWindowResolution[matrixSdk][FRAME_HEIGHT] == 0))
    {
        return FAIL;
    }

    /* Store resolution for privacy mask mapping */
    *pWindowWidth = matrixFixedWindowResolution[matrixSdk][FRAME_WIDTH];
    *pWindowHeight = matrixFixedWindowResolution[matrixSdk][FRAME_HEIGHT];
    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
