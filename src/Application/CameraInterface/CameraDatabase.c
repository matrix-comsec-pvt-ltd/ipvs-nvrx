//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file       CameraDatabase.c
@brief      Store model wise information of camera capabilities and profile related parameters
            for supported brands
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "CameraInterface.h"
#include "MxOnvifClient.h"
#include "CameraInitiation.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define RESOL_HEIGHT_WIDTH_SEP  'x'

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
    CAMERA_MODEL_MIN,
    CAMERA_MODEL_MAX,
    CAMERA_MODEL_LIMIT

}CAMERA_MODEL_LIMIT_e;

typedef struct
{
    UINT32              bitrate;
    BITRATE_VALUE_e     mapVal;

}BITRATE_VALUE_MAP_t;

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void getResolutionPointer(RESOLUTION_PARAMETER_t *resolutionSupported, UINT8 codecNum, UINT8PTR *resolutionVal);
//-------------------------------------------------------------------------------------------------
static void getResolutionPointerforProfile(RESOLUTION_PARAMETER_t *resolutionSupported, UINT8 profileNum, UINT8 codecNum, UINT8PTR *resolutionVal);
//-------------------------------------------------------------------------------------------------
static void getFramerateVal(FRAMERATE_PARAMETER_t *framerateSupported, UINT8 codecNum, UINT8 resolutionIndex, UINT64PTR framerateVal);
//-------------------------------------------------------------------------------------------------
static void getFramerateValforProfile(FRAMERATE_PARAMETER_t *framerateSupported, UINT8 profileNum, UINT8 codecNum, UINT64PTR framerateVal);
//-------------------------------------------------------------------------------------------------
static BOOL getResolutionIndex(CHARPTR resolStr, UINT8PTR resolutionVal, UINT8PTR resolutionIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLE
//#################################################################################################
static pthread_mutex_t 					onvifModelParameterLock[MAX_CAMERA];
static CAMERA_ONVIF_MODEL_PARAMETER_t	onvifCamModelInfo[MAX_CAMERA];

static const CAMERA_MODEL_INFO_t GenericModelInfo[MAX_GENERIC_CAMERA_MODEL] =
{
    {
        .modelName = "",
        .modelParameter = NULL
    },
    // GENERIC MODEL 1
    {
        .modelName = "HTTP",
        .modelParameter = NULL
    },
    // GENERIC MODEL 2
    {
        .modelName = "RTSP UDP",
        .modelParameter = NULL
    },
    // GENERIC MODEL 3
    {
        .modelName = "RTSP TCP",
        .modelParameter = NULL
    }
};

static const CHARPTR codecStr[]=
{
	"",				//VIDEO_CODEC_NONE
	"Motion JPEG",	//VIDEO_MJPG
	"H264",			//VIDEO_H264
	"MPEG4",		//VIDEO_MPEG4
    "",             //VIDEO_CODEC_UNUSED
	"H265",			//VIDEO_H265
	NULL			//END OF CODEC
};

static const CHARPTR resolutionStr[MAX_RESOLUTION] =
{
    "",				//index 0
	"160x90",		//index 1
	"160x100",		//index 2
	"160x112",		//index 3
	"160x120",		//index 4
	"160x128",		//index 5
	"176x112",		//index 6
	"176x120",		//index 7
	"176x144",		//index 8
	"192x144",		//index 9
	"192x192",		//index 10
	"240x135",		//index 11
	"240x180",		//index 12
	"256x192",		//index 13
	"256x256",		//index 14
	"320x180",		//index 15
	"320x192",		//index 16
	"320x200",		//index 17
	"320x240",		//index 18
	"320x256",		//index 19
	"320x320",		//index 20
	"352x240",		//index 21
	"352x244",		//index 22
	"352x288",		//index 23
	"384x216",		//index 24
	"384x288",		//index 25
	"384x384",		//index 26
	"480x270",		//index 27
	"480x300",		//index 28
	"480x360",		//index 29
	"512x384",		//index 30
	"512x512",		//index 31
	"528x320",		//index 32
	"528x328",		//index 33
	"640x360",		//index 34
	"640x368",		//index 35
	"640x400",		//index 36
	"640x480",		//index 37
	"640x512",		//index 38
	"704x240",		//index 39
	"704x288",		//index 40
	"704x480",		//index 41
	"704x570",		//index 42
	"704x576",		//index 43
	"720x480",		//index 44
	"720x576",		//index 45
	"768x576",		//index 46
	"768x768",		//index 47
	"800x450",		//index 48
	"800x480",		//index 49
	"800x500",		//index 50
	"800x600",		//index 51
	"860x540",		//index 52
	"960x480",		//index 53
	"960x540",		//index 54
	"960x576",		//index 55
	"960x720",		//index 56
	"960x768",		//index 57
	"1024x576",		//index 58
	"1024x640",		//index 59
	"1024x768",		//index 60
	"1056x1056",	//index 61
	"1140x1080",	//index 62
	"1280x720",		//index 63
	"1280x800",		//index 64
	"1280x960",		//index 65
	"1280x1024",	//index 66
	"1280x1280",	//index 67
	"1286x972",		//index 68
	"1296x968",		//index 69
	"1296x972",		//index 70
	"1360x768",		//index 71
	"1376x768",		//index 72
	"1440x912",		//index 73
	"1472x960",		//index 74
	"1536x1536",	//index 75
	"1600x904",		//index 76
	"1600x912",		//index 77
	"1600x1200",	//index 78
	"1680x1056",	//index 79
	"1824x1376",	//index 80
	"1920x1080",	//index 81
	"1920x1200",	//index 82
	"1920x1440",	//index 83
	"2032x1920",	//index 84
	"2048x1536",	//index 85
	"2560x1600",	//index 86
	"2560x1920",	//index 87
	"2592x1944",	//index 88
	"2944x1920",	//index 89
	"3648x2752",	//index 90
	"1408x920",		//index 91
	"1120x630",		//index 92
	"528x384",		//index 93
	"1600x720",		//index 94
	"2592x1520",	//index 95
	"2688x1520",	//index 96
	"2560x1440",	//index 97
	"4096x2160",	//index 98
	"3840x2160",	//index 99
	"2560x2048",	//index 100
	"1400x1050",	//index 101
	"2304x1296",	//index 102
	"3072x2048",    //index 103
    "3072x1728",    //index 104
    "2944x1656",    //index 105
    "3200x1800",    //index 106
};

static const CHARPTR bitRateStr[MAX_BITRATE_VALUE] =
{
	"32 kbps",		// Bitrate is in Kbps
	"64 kbps",
	"128 kbps",
	"256 kbps",
	"384 kbps",
	"512 kbps",
	"768 kbps",
	"1024 kbps",
	"1536 kbps",
	"2048 kbps",
	"3072 kbps",
	"4096 kbps",
	"6144 kbps",
	"8192 kbps",
	"12288 kbps",
	"16384 kbps",
};

static const BOOL osdSupportedF[MAX_CAMERA_BRAND] =
{
        NO,     // none
        NO,     // generic
        YES,    // matrix
};

static const CAMERA_BRAND_INFO_t cameraBrandInfo[MAX_CAMERA_BRAND] =
{
	//CAMERA_BRAND_NONE,0
	{
        .brandName      = "CAMERA_BRAND_NONE",
        .maxBrandModel  = MAX_NONE_CAMERA_MODEL,
        .username       = "",
        .password       = "",
        .modelInfo      = NULL,
	},
	//CAMERA_BRAND_GENERIC,1
	{
        .brandName      = "GENERIC",
        .maxBrandModel  = MAX_GENERIC_CAMERA_MODEL,
        .username       = "admin",
        .password       = "admin",
        .modelInfo      = (CAMERA_MODEL_INFO_t*)GenericModelInfo,
	},
    // CAMERA_BRAND_MATRIX,2
	{
        .brandName		= MATRIX_BRAND_NAME,
        .maxBrandModel  = MAX_MATRIX_CAMERA_MODEL,
        .username       = "admin",
        .password       = "admin",
        .modelInfo      = (CAMERA_MODEL_INFO_t*)MatrixModelInfo,
	},
};

static const CAMERA_MODEL_e validateModel[MAX_CAMERA_BRAND][CAMERA_MODEL_LIMIT] =
{
	{CAMERA_MODEL_NONE, 		MAX_NONE_CAMERA_MODEL			},	//0
	{GENERIC_CAMERA_MODEL_NONE, MAX_GENERIC_CAMERA_MODEL		},	//1
    {MATRIX_MODEL_NONE,			MAX_MATRIX_CAMERA_MODEL			},	//2
};

//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief InitOnvifModelParameter: This Function used to initaialize the onvif camera model parameter
 * @return
 */
void InitOnvifModelParameter(void)
{
    UINT8 model;
    UINT8 profileIndex;

    for(model = 0; model < getMaxCameraForCurrentVariant(); model++)
	{
        MUTEX_INIT(onvifModelParameterLock[model], NULL);
		onvifCamModelInfo[model].capabilitySupported = 0;

		for(profileIndex = 0; profileIndex < MAX_PROFILE_SUPPORT; profileIndex++)
		{
			onvifCamModelInfo[model].profileWiseParam[profileIndex] = NULL;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is given camera brand is vaid or not
 * @param   brand
 * @return  TRUE on valid else FALSE
 */
BOOL IsValidCameraBrand(CAMERA_BRAND_e brand)
{
    if ((brand > CAMERA_BRAND_NONE) && (brand < MAX_CAMERA_BRAND))
    {
        return TRUE;
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetBrandNum: get the brand number from brand name
 * @param brand
 * @param brandNum
 * @return
 */
BOOL GetBrandNum(CHARPTR brand, CAMERA_BRAND_e *brandNum)
{
    CAMERA_BRAND_e camBrand;

	for(camBrand = CAMERA_BRAND_NONE; camBrand < MAX_CAMERA_BRAND; camBrand++)
	{
        if((strcmp(brand, cameraBrandInfo[camBrand].brandName)) == STATUS_OK)
		{
			break;
		}
	}

    if (FALSE == IsValidCameraBrand(camBrand))
	{
        return FAIL;
	}

    *brandNum = camBrand;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetModelNum: get model number from model name
 * @param brand
 * @param model
 * @param modelNum
 * @return
 */
BOOL GetModelNum(CHARPTR brand, CHARPTR model, CAMERA_MODEL_e *modelNum)
{
	UINT8 			camModel;
	CAMERA_BRAND_e 	brandNum;

    if ((GetBrandNum(brand, &brandNum) == FAIL) || (cameraBrandInfo[brandNum].modelInfo == NULL))
	{
        return FAIL;
    }

    for(camModel = 0; camModel < cameraBrandInfo[brandNum].maxBrandModel; camModel++)
    {
        if(strcmp(model, cameraBrandInfo[brandNum].modelInfo[camModel].modelName) == STATUS_OK)
        {
            break;
        }
    }

    if ((camModel > CAMERA_MODEL_NONE) && (camModel < cameraBrandInfo[brandNum].maxBrandModel))
    {
        *modelNum = camModel;
        return SUCCESS;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get and validate camera brand and model for BM and CI
 * @param   camIndex
 * @param   pBrand
 * @param   pModel
 * @return  Status
 */
NET_CMD_STATUS_e GetAndValidateCameraBrandModel(UINT8 camIndex, CAMERA_BRAND_e *pBrand, CAMERA_MODEL_e *pModel)
{
    /* Check camera type BM or CI? */
    if (CameraType(camIndex) == AUTO_ADDED_CAMERA)
    {
        /* Get brand and model of CI */
        GetCiCameraBrandModelNumber(camIndex, pBrand, pModel);
    }
    else
    {
        /* Get brand and model of BM */
        GetCameraBrandModelNumber(camIndex, pBrand, pModel);
    }

    /* Is model information of brand available? */
    if ((*pBrand >= MAX_CAMERA_BRAND) || (cameraBrandInfo[*pBrand].modelInfo == NULL))
    {
        /* It is generic camera brand */
        return CMD_GENERIC_CAM_STREAM_CONFIG;
    }

    /* Is valid camera brand and model? */
    if (ValidateCamBrandModel(*pBrand, *pModel) != SUCCESS)
    {
        /* Fail to validate */
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    /* Valid camera brand-model found */
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoCodecNum: get video codec number from codec names
 * @param codec
 * @param codecNum
 * @return
 */
BOOL GetVideoCodecNum(CHARPTR codec, STREAM_CODEC_TYPE_e *codecNum)
{
	*codecNum = ConvertStringToIndex(codec, codecStr, MAX_VIDEO_CODEC);
    if((*codecNum > VIDEO_CODEC_NONE) && (*codecNum <  MAX_VIDEO_CODEC))
	{
        return SUCCESS;
	}

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCameraBrand: give list of supported brand name
 * @param brand
 * @param maxCameraBrand
 * @return
 */
NET_CMD_STATUS_e GetCameraBrand(CHARPTR *brand, UINT8PTR maxCameraBrand)
{
    UINT8 camBrand;

    if ((brand == NULL) || (maxCameraBrand == NULL))
	{
        return CMD_PROCESS_ERROR;
    }

    /* Skip CAMERA_BRAND_NONE */
    for(camBrand = CAMERA_BRAND_GENERIC; camBrand < MAX_CAMERA_BRAND; camBrand++)
    {
        brand[camBrand - 1] = cameraBrandInfo[camBrand].brandName;
    }

    *maxCameraBrand = (camBrand - 1);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCameraModels: give list of supported model for this brand
 * @param brand
 * @param model
 * @param maxCameraModel
 * @return
 */
NET_CMD_STATUS_e GetCameraModels(CHARPTR brand, CHARPTR *model, UINT8PTR maxCameraModel)
{
    UINT8           camModel;
    CAMERA_BRAND_e  brandNum;

    if ((brand == NULL) || (model == NULL) || (maxCameraModel == NULL))
	{
        return CMD_PROCESS_ERROR;
    }

    if ((GetBrandNum(brand, &brandNum)) == FAIL)
    {
        EPRINT(CAMERA_INTERFACE, "camera brand doesn't exist: [brand=%s]", brand);
        return CMD_PROCESS_ERROR;
    }

    if (cameraBrandInfo[brandNum].modelInfo == NULL)
    {
        EPRINT(CAMERA_INTERFACE, "no model parameters available: [brand=%s]", brand);
        return CMD_PROCESS_ERROR;
    }

    for(camModel = 1; camModel < cameraBrandInfo[brandNum].maxBrandModel; camModel++)
    {
        model[camModel - 1]	= cameraBrandInfo[brandNum].modelInfo[camModel].modelName;
    }

    *maxCameraModel = (camModel - 1);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCameraUsernamePassword: get username and password from the database
 * @param brand
 * @param username
 * @param password
 * @return
 */
NET_CMD_STATUS_e GetCameraUsernamePassword(CHARPTR brand, CHARPTR username, CHARPTR password)
{
    CAMERA_BRAND_e brandNum;

	if((brand != NULL) && (GetBrandNum(brand, &brandNum) == SUCCESS))
	{
        snprintf(username,MAX_CAMERADETAIL_USERNAME_WIDTH,"%s", cameraBrandInfo[brandNum].username);
        snprintf(password,MAX_CAMERADETAIL_PASSWORD_WIDTH,"%s", cameraBrandInfo[brandNum].password);
	}
	else
	{
        snprintf(username,MAX_CAMERADETAIL_USERNAME_WIDTH, "admin");
        snprintf(password,MAX_CAMERADETAIL_PASSWORD_WIDTH, "admin");
	}

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetMaxSupportedProfile: get maximum supported profile for this camera
 * @param cameraIndex
 * @param numOfProfSupported
 * @return
 */
NET_CMD_STATUS_e GetMaxSupportedProfile(UINT8 cameraIndex, UINT8PTR numOfProfSupported)
{
    NET_CMD_STATUS_e    status = CMD_SUCCESS;
    CAMERA_CONFIG_t     camConfig;

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (numOfProfSupported == NULL) ||
            ((CameraType(cameraIndex) != IP_CAMERA) && (CameraType(cameraIndex) != AUTO_ADDED_CAMERA)))
    {
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    ReadSingleCameraConfig(cameraIndex, &camConfig);
    if (camConfig.camera == DISABLE)
    {
        return CMD_CHANNEL_DISABLED;
    }

    if (OnvifSupportF(cameraIndex) == TRUE)
    {
        *numOfProfSupported = GetOnvifConfiguredProfile(cameraIndex);
    }
    else
    {
        CAMERA_BRAND_e  brand = 0;
        CAMERA_MODEL_e  model = 0;
        *numOfProfSupported = 0;
        status = GetAndValidateCameraBrandModel(cameraIndex, &brand, &model);
        if (status == CMD_GENERIC_CAM_STREAM_CONFIG)
        {
            *numOfProfSupported = 1;
        }
        else if (status != CMD_SUCCESS)
        {
            return status;
        }
        else
        {
            if (cameraBrandInfo[brand].modelInfo[model].profileParam.maxSupportedProfile == 0)
            {
                return CMD_CAM_STREAM_CONFIG_ERROR;
            }

            *numOfProfSupported = cameraBrandInfo[brand].modelInfo[model].profileParam.maxSupportedProfile;
        }
    }

    if (*numOfProfSupported == 1)
    {
        SetCurrentSubStreamProfile(cameraIndex);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSupportedCodec: get supported codec from given camera and stream type
 * @param cameraIndex
 * @param streamType
 * @param profileNo
 * @param supportedCodec
 * @param maxSupportedCodec
 * @return
 */
NET_CMD_STATUS_e GetSupportedCodec(UINT8 cameraIndex, VIDEO_TYPE_e streamType, UINT8 profileNo, CHARPTR *supportedCodec, UINT8PTR maxSupportedCodec)
{
    UINT8               codecIndex, cameraCodec = 0;
    NET_CMD_STATUS_e    status = CMD_CAM_STREAM_CONFIG_ERROR;

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (supportedCodec == NULL) || (maxSupportedCodec == NULL) ||
            ((CameraType(cameraIndex) != IP_CAMERA) && (CameraType(cameraIndex) != AUTO_ADDED_CAMERA)))
    {
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    *maxSupportedCodec = 0;
    if (OnvifSupportF(cameraIndex) == TRUE)
    {
        if ((profileNo - 1) < MAX_PROFILE_SUPPORT)
        {
            MUTEX_LOCK(onvifModelParameterLock[cameraIndex]);
            if((profileNo >= 1) && (onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1] != NULL))
            {
                cameraCodec = onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1]->codecSupported;
            }
            else
            {
                status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            }
            MUTEX_UNLOCK(onvifModelParameterLock[cameraIndex]);
        }
    }
    else
    {
        CAMERA_BRAND_e brand = 0;
        CAMERA_MODEL_e model = 0;
        status = GetAndValidateCameraBrandModel(cameraIndex, &brand, &model);
        if (status != CMD_SUCCESS)
        {
            return status;
        }

        if (cameraBrandInfo[brand].modelInfo[model].modelParameter == NULL)
        {
            return CMD_CAM_STREAM_CONFIG_ERROR;
        }

        if (cameraBrandInfo[brand].modelInfo[model].modelParameter->isParamProfileDependent == YES)
        {
            cameraCodec = cameraBrandInfo[brand].modelInfo[model].modelParameter->profileWiseCodecSupported[profileNo - 1];
        }
        else
        {
            cameraCodec = cameraBrandInfo[brand].modelInfo[model].modelParameter->codecSupported;
        }
    }

    if (cameraCodec == 0)
    {
        return status;
    }

    codecIndex = VIDEO_MJPG;
    while(codecIndex != MAX_VIDEO_CODEC)
    {
        if((cameraCodec >> codecIndex) & 1)
        {
            supportedCodec[*maxSupportedCodec] = codecStr[codecIndex];
            (*maxSupportedCodec)++;
        }
        codecIndex++;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSupportedResolution: get supported resolution for given camera and stream type
 * @param cameraIndex
 * @param codecStrPtr
 * @param streamType
 * @param profileNo
 * @param supportedResolution
 * @param maxSupportedResolution
 * @return
 */
NET_CMD_STATUS_e GetSupportedResolution(UINT8 cameraIndex, CHARPTR codecStrPtr,	VIDEO_TYPE_e streamType,
                                        UINT8 profileNo, CHARPTR *supportedResolution, UINT8PTR maxSupportedResolution)
{
	UINT8 					 	resolutionIndex;
	NET_CMD_STATUS_e		 	status = CMD_CAM_STREAM_CONFIG_ERROR;
	UINT8PTR 					resolutionVal = NULL;
	STREAM_CODEC_TYPE_e		 	codecNum;
    RESOLUTION_PARAMETER_t		*pResSup = NULL;
    CAMERA_MODEL_PARAMETER_t 	*modelParameter = NULL;

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (supportedResolution == NULL) || (maxSupportedResolution == NULL) ||
            (codecStrPtr == NULL) || ((CameraType(cameraIndex) != IP_CAMERA) && (CameraType(cameraIndex) != AUTO_ADDED_CAMERA)))
    {
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    BOOL onvifSupportF = OnvifSupportF(cameraIndex);
    if (onvifSupportF == TRUE)
    {
        MUTEX_LOCK(onvifModelParameterLock[cameraIndex]);
        if((profileNo - 1) < MAX_PROFILE_SUPPORT)
        {
            if((profileNo > 0) && (onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1] != NULL))
            {
                pResSup = &(onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1]->resolutionSupported);
            }
            else
            {
                status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            }
        }
    }
    else
    {
        CAMERA_BRAND_e  brand = 0;
        CAMERA_MODEL_e  model = 0;
        status = GetAndValidateCameraBrandModel(cameraIndex, &brand, &model);
        if (status != CMD_SUCCESS)
        {
            return status;
        }

        modelParameter = cameraBrandInfo[brand].modelInfo[model].modelParameter;
        if(modelParameter != NULL)
        {
            pResSup = &modelParameter->resolutionSupported;
        }
    }

    if (pResSup != NULL)
    {
        GetVideoCodecNum(codecStrPtr, &codecNum);
        if(codecNum == MAX_VIDEO_CODEC)
        {
            status = CMD_GENERIC_CAM_STREAM_CONFIG;
        }
        else
        {
            if((modelParameter != NULL) && (modelParameter->isParamProfileDependent == YES))
            {
                getResolutionPointerforProfile(pResSup, (profileNo - 1), (codecNum - 1), &resolutionVal);
            }
            else
            {
                getResolutionPointer(pResSup, (codecNum - 1), &resolutionVal);
            }

            if(resolutionVal != NULL)
            {
                for(resolutionIndex = 0; resolutionVal[resolutionIndex] < MAX_RESOLUTION; resolutionIndex++)
                {
                    supportedResolution[resolutionIndex] = resolutionStr[resolutionVal[resolutionIndex]];
                }

                *maxSupportedResolution = resolutionIndex;
                status = CMD_SUCCESS;
            }
        }
    }

    if (onvifSupportF == TRUE)
    {
        MUTEX_UNLOCK(onvifModelParameterLock[cameraIndex]);
    }

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSupportedFramerate: get supported frame rate for requested camera and stream type
 * @param camIndex
 * @param encoder
 * @param resolution
 * @param streamType
 * @param profileNo
 * @param framerateVal
 * @return
 */
NET_CMD_STATUS_e GetSupportedFramerate(UINT8 camIndex, CHARPTR encoder, CHARPTR resolution, VIDEO_TYPE_e streamType, UINT8 profileNo, UINT64PTR framerateVal)
{
    UINT8PTR 			     	resolutionVal;
	UINT8 					 	resolutionIndex;
	STREAM_CODEC_TYPE_e		 	codecNum;
	NET_CMD_STATUS_e 		 	status = CMD_CAM_STREAM_CONFIG_ERROR;
    RESOLUTION_PARAMETER_t		*pResSup = NULL;
    FRAMERATE_PARAMETER_t		*pFrameRateSup = NULL;
    CAMERA_MODEL_PARAMETER_t 	*modelParameter = NULL;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (encoder == NULL) || (resolution == NULL) || (framerateVal == NULL) ||
            ((CameraType(camIndex) != IP_CAMERA) && (CameraType(camIndex) != AUTO_ADDED_CAMERA)))
    {
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    *framerateVal = 0;
    BOOL onvifSupportF = OnvifSupportF(camIndex);
    if(onvifSupportF == TRUE)
    {
        MUTEX_LOCK(onvifModelParameterLock[camIndex]);
        if((profileNo - 1) < MAX_PROFILE_SUPPORT)
        {
            if((profileNo > 0) && (onvifCamModelInfo[camIndex].profileWiseParam[profileNo - 1] != NULL))
            {
                pResSup = &onvifCamModelInfo[camIndex].profileWiseParam[profileNo - 1]->resolutionSupported;
                pFrameRateSup = &onvifCamModelInfo[camIndex].profileWiseParam[profileNo - 1]->framerateSupported;
            }
            else
            {
                status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            }
        }
    }
    else
    {
        CAMERA_BRAND_e brand = 0;
        CAMERA_MODEL_e model = 0;
        status = GetAndValidateCameraBrandModel(camIndex, &brand, &model);
        if (status != CMD_SUCCESS)
        {
            return status;
        }

        modelParameter = cameraBrandInfo[brand].modelInfo[model].modelParameter;
        if(modelParameter != NULL)
        {
            pResSup = &modelParameter->resolutionSupported;
            pFrameRateSup = &modelParameter->framerateSupported;
        }
    }

    if ((pResSup != NULL) && (pFrameRateSup != NULL))
    {
        GetVideoCodecNum(encoder, &codecNum);
        if(codecNum == MAX_VIDEO_CODEC)
        {
            status = CMD_GENERIC_CAM_STREAM_CONFIG;
        }
        else
        {
            if((modelParameter != NULL) && (modelParameter->isParamProfileDependent == YES))
            {
                getResolutionPointerforProfile(pResSup, (profileNo - 1), (codecNum - 1), &resolutionVal);
            }
            else
            {
                getResolutionPointer(pResSup, (codecNum - 1), &resolutionVal);
            }

            if (getResolutionIndex(resolution, resolutionVal, &resolutionIndex) == SUCCESS)
            {
                if((modelParameter != NULL) && (modelParameter->isParamProfileDependent == YES))
                {
                    getFramerateValforProfile(pFrameRateSup, (profileNo - 1), (codecNum - 1), framerateVal);
                }
                else
                {
                    getFramerateVal(pFrameRateSup, (codecNum - 1), resolutionIndex, framerateVal);
                }

                *framerateVal = (*framerateVal >> 1);
                status = CMD_SUCCESS;
            }
        }
    }

    if(onvifSupportF == TRUE)
    {
        MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);
    }

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetSupportedBitRate: get supported bit rate for requested camera profile
 * @param   cameraIndex
 * @param   codecStr
 * @param   streamType
 * @param   profileNo
 * @param   supportedBitrate
 * @param   maxSupportedBitrate
 * @param   minBitrateIndex
 * @param   maxBitrateIndex
 * @return
 */
NET_CMD_STATUS_e GetSupportedBitRate(UINT8 cameraIndex, CHARPTR codecStr, VIDEO_TYPE_e streamType, UINT8 profileNo, CHARPTR *supportedBitrate,
                                     UINT8PTR maxSupportedBitrate, BITRATE_VALUE_e *minBitrateIndex, BITRATE_VALUE_e *maxBitrateIndex)
{
    UINT8               loop = 0, count = 0;
    NET_CMD_STATUS_e    status = CMD_CAM_STREAM_CONFIG_ERROR;
    STREAM_CODEC_TYPE_e codecNum;

    if((cameraIndex >= getMaxCameraForCurrentVariant()) || (supportedBitrate == NULL) || (maxSupportedBitrate == NULL) ||
            (codecStr == NULL) || (minBitrateIndex == NULL) || (maxBitrateIndex == NULL) ||
            ((CameraType(cameraIndex) != IP_CAMERA) && (CameraType(cameraIndex) != AUTO_ADDED_CAMERA)))
	{
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    *minBitrateIndex = BITRATE_32;
    *maxBitrateIndex = BITRATE_16384;
    BOOL onvifSupportF = OnvifSupportF(cameraIndex);
    if (profileNo == 0)
    {
        return CMD_ONVIF_CAM_CAPABILITY_ERROR;
    }

    if ((profileNo - 1) >= MAX_PROFILE_SUPPORT)
    {
        return CMD_PROCESS_ERROR;
    }

    if (onvifSupportF == TRUE)
    {
        MUTEX_LOCK(onvifModelParameterLock[cameraIndex]);
        GetVideoCodecNum(codecStr, &codecNum);
        if(codecNum == MAX_VIDEO_CODEC)
        {
            status = CMD_GENERIC_CAM_STREAM_CONFIG;
        }
        else
        {
            for(loop = 0; loop < MAX_BITRATE_VALUE; loop++)
            {
                if(onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1] != NULL)
                {
                    if(onvifBitrateValue[loop] >= onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1]->minBitRateSupport[codecNum - 1])
                    {
                        *minBitrateIndex = loop;
                        break;
                    }
                }
                else
                {
                    status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
                    break;
                }
            }

            for(loop = 0; loop < MAX_BITRATE_VALUE; loop++)
            {
                if(onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1] != NULL)
                {
                    if(onvifBitrateValue[MAX_BITRATE_VALUE - (loop + 1)]
                                         <= onvifCamModelInfo[cameraIndex].profileWiseParam[profileNo - 1]->maxBitRateSupport[codecNum - 1])
                    {
                        *maxBitrateIndex = (MAX_BITRATE_VALUE - (loop + 1));
                        break;
                    }
                }
                else
                {
                    status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
                    *maxBitrateIndex = MAX_BITRATE_VALUE;
                    break;
                }
            }
        }
    }
    else
    {
        CAMERA_BRAND_e brand = 0;
        CAMERA_MODEL_e model = 0;
        status = GetAndValidateCameraBrandModel(cameraIndex, &brand, &model);
        if (status == CMD_GENERIC_CAM_STREAM_CONFIG)
        {
            return status;
        }
    }

    if(*maxBitrateIndex < MAX_BITRATE_VALUE)
    {
        count = 0;
        for(loop = *minBitrateIndex; loop <= *maxBitrateIndex; loop++)
        {
            supportedBitrate[count] = bitRateStr[loop];
            count++;
        }
        *maxSupportedBitrate = count;
        status = CMD_SUCCESS;
    }

    if(onvifSupportF == TRUE)
    {
        MUTEX_UNLOCK(onvifModelParameterLock[cameraIndex]);
    }

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSupportedQuality: get supported quality range for requested camera profile
 * @param camIndex
 * @param codecStr
 * @param streamType
 * @param profileNo
 * @param supportedQuality
 * @return
 */
NET_CMD_STATUS_e GetSupportedQuality(UINT8 camIndex, CHARPTR codecStr, VIDEO_TYPE_e streamType, UINT8 profileNo, UINT8PTR supportedQuality)
{
    NET_CMD_STATUS_e    status = CMD_CAM_STREAM_CONFIG_ERROR;
    STREAM_CODEC_TYPE_e codecNum;
    UINT8PTR            pQualitySup = NULL;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (codecStr == NULL) || (supportedQuality == NULL) ||
            ((CameraType(camIndex) != IP_CAMERA) && (CameraType(camIndex) != AUTO_ADDED_CAMERA)))
	{
        return CMD_CAM_STREAM_CONFIG_ERROR;
    }

    BOOL onvifSupportF = OnvifSupportF(camIndex);
    if(onvifSupportF == TRUE)
    {
        MUTEX_LOCK(onvifModelParameterLock[camIndex]);
        if((profileNo - 1) < MAX_PROFILE_SUPPORT)
        {
            if((profileNo > 0) && (onvifCamModelInfo[camIndex].profileWiseParam[profileNo - 1] != NULL))
            {
                pQualitySup = onvifCamModelInfo[camIndex].profileWiseParam[profileNo - 1]->qualitySupported;
            }
            else
            {
                status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            }
        }
        else
        {
            status = CMD_PROCESS_ERROR;
        }
    }
    else
    {
        CAMERA_BRAND_e brand = 0;
        CAMERA_MODEL_e model = 0;
        status = GetAndValidateCameraBrandModel(camIndex, &brand, &model);
        if (status != CMD_SUCCESS)
        {
            return status;
        }

        if (cameraBrandInfo[brand].modelInfo[model].modelParameter != NULL)
        {
            pQualitySup = cameraBrandInfo[brand].modelInfo[model].modelParameter->qualitySupported;
        }
    }

    if (pQualitySup != NULL)
    {
        if(GetVideoCodecNum(codecStr, &codecNum) == SUCCESS)
        {
            *supportedQuality = pQualitySup[codecNum - 1];
            status = CMD_SUCCESS;
        }
        else
        {
            status = CMD_GENERIC_CAM_STREAM_CONFIG;
        }
    }

    if(onvifSupportF == TRUE)
    {
        MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);
    }

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSupportedCapability: get supported capablities for requested camera
 * @param camIndex
 * @param camCapability
 * @return
 */
NET_CMD_STATUS_e GetSupportedCapability(UINT8 camIndex, CAMERA_CAPABILTY_INFO_t *camCapability)
{
    CAPABILITY_TYPE supportedCapability;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (camCapability == NULL) ||
            ((CameraType(camIndex) != IP_CAMERA) && (CameraType(camIndex) != AUTO_ADDED_CAMERA)))
    {
        EPRINT(CAMERA_INTERFACE, "invld input param: [camera=%d]", camIndex);
        return CMD_PROCESS_ERROR;
    }

    if (OnvifSupportF(camIndex) == TRUE)
    {
        MUTEX_LOCK(onvifModelParameterLock[camIndex]);
        supportedCapability = onvifCamModelInfo[camIndex].capabilitySupported;
        MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);

        camCapability->motionWinConfigSupport = (((supportedCapability>>MOTION_WIN_CONFIG_SUPPORT) & 1) ? YES : NO);
        camCapability->privacymaskWinConfigSupport = (((supportedCapability>>PRIVACYMASK_CONFIG_SUPPORT) & 1) ? YES : NO);
    }
    else
    {
        CAMERA_BRAND_e  brand = 0;
        CAMERA_MODEL_e  model = 0;
        if (GetAndValidateCameraBrandModel(camIndex, &brand, &model) != CMD_SUCCESS)
        {
            return CMD_PROCESS_ERROR;
        }

        if (cameraBrandInfo[brand].modelInfo[model].modelParameter == NULL)
        {
            return CMD_PROCESS_ERROR;
        }

        supportedCapability = cameraBrandInfo[brand].modelInfo[model].modelParameter->capabilitySupported;
        camCapability->motionWinConfigSupport = (cameraBrandInfo[brand].modelInfo[model].motionWindowSettingMethod != MOTION_AREA_METHOD_NO_SUPPORT) ? YES : NO;
        camCapability->privacymaskWinConfigSupport = (cameraBrandInfo[brand].modelInfo[model].privacyMaskWindowSettingMethod != PRIVACY_MASK_METHOD_NO_SUPPORT) ? YES : NO;
    }

    camCapability->audioSupport = (((supportedCapability >> AUDIO_MIC_SUPPORT) & 1) ? YES : NO);
    camCapability->motionDetectionSupport = (((supportedCapability >> MOTION_DETECTION_SUPPORT) & 1) ? YES : NO);
    camCapability->noMotionDetectionSupport = (((supportedCapability >> NO_MOTION_DETECTION_SUPPORT) & 1) ? YES : NO);
    camCapability->viewTamperSupport = (((supportedCapability >> TAMPER_DETECTION_SUPPORT) & 1) ? YES : NO);
    camCapability->audioDetectionSupport = NO;
    camCapability->maxSensorInput = ((supportedCapability >> CAMERA_ALARM_INPUT1_SUPPORT) & 1) + ((supportedCapability >> CAMERA_ALARM_INPUT2_SUPPORT) & 1) + ((supportedCapability >> CAMERA_ALARM_INPUT3_SUPPORT) & 1);
    camCapability->maxAlarmOutput = ((supportedCapability >> CAMERA_ALARM_OUTPUT1_SUPPORT) & 1) + ((supportedCapability >> CAMERA_ALARM_OUTPUT2_SUPPORT) & 1) + ((supportedCapability >> CAMERA_ALARM_OUTPUT3_SUPPORT) & 1);
    camCapability->ptzSupport = (((supportedCapability >> PTZ_SUPPORT) & 1) ? YES : NO);
    camCapability->lineCrossingSupport = (((supportedCapability>>LINE_CROSSING_SUPPORT) & 1) ? YES : NO);
    camCapability->objectIntrusionSupport = (((supportedCapability>>OBJECT_INTRUSION_SUPPORT) & 1) ? YES : NO);
    camCapability->audioExceptionSupport = (((supportedCapability>>AUDIO_EXCEPTION_SUPPORT) & 1) ? YES : NO);
    camCapability->missingObjectSupport = (((supportedCapability>>MISSING_OBJECT_SUPPORT) & 1) ? YES : NO);
    camCapability->suspiciousObjectSupport = (((supportedCapability>>SUSPICIOUS_OBJECT_SUPPORT) & 1) ? YES : NO);
    camCapability->loiteringSupport = (((supportedCapability>>LOITERING_SUPPORT) & 1) ? YES : NO);
    camCapability->objectCounting = (((supportedCapability>>OBJECT_COUNTING_SUPPORT) & 1) ? YES : NO);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getResolutionPointer: get supported resolutions for requested codec
 * @param resolutionSupported
 * @param codecNum
 * @param resolutionVal
 * @return
 */
static void getResolutionPointer(RESOLUTION_PARAMETER_t *resolutionSupported, UINT8 codecNum, UINT8PTR *resolutionVal)
{
	if(resolutionSupported->isResolutionCodecDependent == YES)
	{
		//codec mjpeg starts from 0
		*resolutionVal = ((UINT8PTR *)resolutionSupported->resolutionAvailable)[codecNum];
	}
	else
	{
		*resolutionVal = resolutionSupported->resolutionAvailable;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getResolutionPointerforProfile: get supported resolution for requested profile
 * @param   resolutionSupported
 * @param   profileNum
 * @param   codecNum
 * @param   resolutionVal
 * @return
 */
static void getResolutionPointerforProfile(RESOLUTION_PARAMETER_t *resolutionSupported, UINT8 profileNum, UINT8 codecNum, UINT8PTR *resolutionVal)
{
    if (resolutionSupported->isResolutionCodecDependent == YES)
    {
        /* Get profile and codec dependent resolution */
        UINT8 *profile = (UINT8 *)((size_t *)resolutionSupported->resolutionAvailable)[profileNum];
        *resolutionVal = (UINT8 *)((size_t *)profile)[codecNum];
    }
    else
    {
        *resolutionVal = ((UINT8PTR *)resolutionSupported->resolutionAvailable)[profileNum];
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getResolutionIndex: get resolution index
 * @param resolStr
 * @param resolutionVal
 * @param resolutionIndex
 * @return
 */
static BOOL getResolutionIndex(CHARPTR resolStr, UINT8PTR resolutionVal, UINT8PTR resolutionIndex)
{
    UINT8 resolIndex;

    if (resolutionVal == NULL)
	{
        return FAIL;
    }

    for(resolIndex = 0; resolutionVal[resolIndex] < MAX_RESOLUTION; resolIndex++)
    {
        if((strcmp(resolStr, resolutionStr[resolutionVal[resolIndex]])) == STATUS_OK)
        {
            break;
        }
    }

    if (resolIndex >= MAX_RESOLUTION)
    {
        return FAIL;
    }

    *resolutionIndex = resolIndex;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getFramerateVal: get supported frame rate for request codec
 * @param framerateSupported
 * @param codecNum
 * @param resolutionIndex
 * @param framerateVal
 * @return
 */
static void getFramerateVal(FRAMERATE_PARAMETER_t *framerateSupported, UINT8 codecNum, UINT8 resolutionIndex, UINT64PTR framerateVal)
{
    if(framerateSupported->isFramerateCodecDependent == YES)
    {
        if(framerateSupported->isFramerateResolutionDependent == YES)
        {
            /* Hisilicon board have 32bit OS and Rockchip board have 64bit OS. framerateAvailable is pointing to an array of pointers.
             * While accessing index of framerateAvailable, it should jump 4 bytes for 32bit OS and 8 bytes for 64bit OS.
             * Hence, Forcefully typecast framerateAvailable pointer as per OS */
            size_t *framerateAvailable = (size_t*)framerateSupported->framerateAvailable;

            /* Now again we have to typecast it to UINT64 pointer as value pointed by above address (addrValue) which is of type UINT64 array */
            UINT64PTR codecFrameRate = (UINT64PTR)framerateAvailable[codecNum];
            *framerateVal = codecFrameRate[resolutionIndex];
            DPRINT(CAMERA_INTERFACE, "framerate is resolution dependent");
        }
        else
        {
            *framerateVal = framerateSupported->framerateAvailable[codecNum];
        }
    }
    else if(framerateSupported->isFramerateResolutionDependent == YES)
    {
        *framerateVal = framerateSupported->framerateAvailable[resolutionIndex];
    }
    else
    {
        *framerateVal = *(framerateSupported->framerateAvailable);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getFramerateValforProfile: get supported framerate for requested profile
 * @param framerateSupported
 * @param profileNum
 * @param codecNum
 * @param framerateVal
 * @return
 */
static void getFramerateValforProfile(FRAMERATE_PARAMETER_t * framerateSupported, UINT8 profileNum, UINT8 codecNum, UINT64PTR framerateVal)
{
    if(framerateSupported->isFramerateCodecDependent == YES)
    {
        size_t *framerateAvailable = (size_t*)framerateSupported->framerateAvailable;
        UINT64PTR profileWiseFramerate = (UINT64PTR)framerateAvailable[profileNum];
        *framerateVal = profileWiseFramerate[codecNum];
    }
    else
    {
        *framerateVal = framerateSupported->framerateAvailable[profileNum];
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SaveOnvifCameraCapabiltyProfileWise: store camera capabilities received through onvif into
 *                                             onvifCamModelInfo for requested camera profile
 * @param cameraIndex
 * @param profileIndex
 * @param onvifProfParam
 */
void SaveOnvifCameraCapabiltyProfileWise(UINT8 cameraIndex, UINT8 profileIndex, PROFILE_WISE_OPTION_PARAM_t *onvifProfParam)
{
    if (onvifProfParam == NULL)
	{
        EPRINT(CAMERA_INTERFACE, "onvif profile not available: [camera=%d], [profile=%d]", cameraIndex, profileIndex);
        return;
    }

    if (profileIndex >= MAX_PROFILE_SUPPORT)
    {
        EPRINT(CAMERA_INTERFACE, "invld onvif profile: [camera=%d], [profile=%d]", cameraIndex, profileIndex);
        FREE_MEMORY(onvifProfParam);
        return;
    }

    MUTEX_LOCK(onvifModelParameterLock[cameraIndex]);
    onvifCamModelInfo[cameraIndex].profileWiseParam[profileIndex] = onvifProfParam;
    MUTEX_UNLOCK(onvifModelParameterLock[cameraIndex]);
    //DPRINT(CAMERA_INTERFACE, "onvif profile added: [camera=%d], [profile=%d]", cameraIndex, profileIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SaveOnvifCameraCapabilty: store camera capabilities received through onvif into
 *                                  onvifCamModelInfo for requested camera
 * @param cameraIndex
 * @param capabillity
 */
void SaveOnvifCameraCapabilty(UINT8 cameraIndex, CAPABILITY_TYPE capabillity)
{
    MUTEX_LOCK(onvifModelParameterLock[cameraIndex]);
	onvifCamModelInfo[cameraIndex].capabilitySupported = capabillity;
    MUTEX_UNLOCK(onvifModelParameterLock[cameraIndex]);
    DPRINT(CAMERA_INTERFACE, "onvif capabillity added: [camera=%d]", cameraIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get resolution id from width and height
 * @param   width
 * @param   height
 * @param   resolPtr
 * @return
 */
BOOL GetResolutionId(UINT16 width, UINT16 height, RESOLUTION_e *resolPtr)
{
    RESOLUTION_e    resId;
	CHAR			resStr[16];

    *resolPtr = RESOLUTION_NONE;
    if ((width == 0) || (height == 0))
	{
        return FAIL;
    }

    /* Prepare resolution string from height and width */
    snprintf(resStr, sizeof(resStr), "%dx%d", width, height);

    for(resId = (RESOLUTION_NONE + 1); resId < MAX_RESOLUTION; resId++)
    {
        if (strcmp(resStr, resolutionStr[resId]))
        {
            continue;
        }

        *resolPtr = resId;
        return SUCCESS;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetResolutionNum
 * @param resolStr
 * @param resolutionIndex
 * @return
 */
BOOL GetResolutionNum(CHARPTR resolStr, RESOLUTION_e *resolutionIndex)
{
    RESOLUTION_e frameRes = ConvertStringToIndex(resolStr, resolutionStr, MAX_RESOLUTION);
    if(frameRes >= MAX_RESOLUTION)
	{
        return FAIL;
    }

    *resolutionIndex = frameRes;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetResolutionHeightWidth
 * @param resolution
 * @param height
 * @param width
 * @return
 */
BOOL GetResolutionHeightWidth(CHARPTR resolution, UINT16PTR height, UINT16PTR width)
{
	CHAR		string[10];
	CHARPTR 	startStr,endStr;
	UINT8 		parseVal;
	UINT64 		resolVal[2];
	CHAR 		searchChar[2] = {RESOL_HEIGHT_WIDTH_SEP, '\0'};

	if(resolution == NULL)
	{
        EPRINT(CAMERA_INTERFACE, "invld input param");
        return FAIL;
	}

    startStr = resolution;
    for(parseVal = 0; parseVal < 2; parseVal++)
    {
        endStr = strchr(startStr, searchChar[parseVal]);
        if(endStr == NULL)
        {
            return FAIL;
        }

        strncpy(string, startStr, endStr - startStr);
        string[endStr - startStr] = '\0';
        if(AsciiToInt(string, &resolVal[parseVal]) == FAIL)
        {
            return FAIL;
        }

        startStr = (endStr + 1);
    }

    *width 	= resolVal[0];
    *height = resolVal[1];
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetResolutionHeightWidthString: parse height and width from resolution
 * @param resolution
 * @param resolutionHeight
 * @param resolutionWidth
 * @param resHeightLen
 * @param resWidthLen
 * @return
 */
BOOL GetResolutionHeightWidthString(CHARPTR resolution,CHARPTR resolutionHeight,CHARPTR resolutionWidth, const UINT8 resHeightLen, const UINT8 resWidthLen)
{
    CHARPTR tmpPtr = strchr(resolution, RESOL_HEIGHT_WIDTH_SEP);
    if (tmpPtr == NULL)
    {
        return FALSE;
    }

    snprintf(resolutionWidth, (tmpPtr - resolution + 1) , "%s", resolution);
    tmpPtr++;
    snprintf(resolutionHeight, resHeightLen, "%s", tmpPtr);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetResolutionString
 * @param resolution
 * @param resIndex
 * @return
 */
BOOL GetResolutionString(CHARPTR resolution, RESOLUTION_e resIndex)
{
    if (resIndex >= MAX_RESOLUTION)
	{
        return FALSE;
    }

    snprintf(resolution,MAX_RESOLUTION_NAME_LEN,"%s", resolutionStr[resIndex]);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetSupportedTextOverlay
 * @param   camIndex
 * @return
 */
UINT8 GetSupportedTextOverlay(UINT8 camIndex)
{
    CAMERA_BRAND_e      brand = 0;
    CAMERA_MODEL_e      model = 0;
    IP_CAMERA_CONFIG_t  ipCameraCfg;

    /* Read IP camera config to check onvif support */
    ReadSingleIpCameraConfig(camIndex, &ipCameraCfg);
    do
    {
        /* Check onvif support */
        if (TRUE == ipCameraCfg.onvifSupportF)
        {
            break;
        }

        /* Get camera brand and model */
        if (CMD_SUCCESS != GetAndValidateCameraBrandModel(camIndex, &brand, &model))
        {
            break;
        }

        /* Check camera brand */
        if (brand != CAMERA_BRAND_MATRIX)
        {
            break;
        }

        /* Check camera model */
        if (((model >= MATRIX_MODEL_SATATYA_CIBR80FL_28CW_S) && (model <= MATRIX_MODEL_SATATYA_MIDR80FL_60CW_P)) ||
                ((model >= MATRIX_MODEL_SATATYA_MITR50FL_28CW_S) && (model <= MATRIX_MODEL_SATATYA_RIDR50FL_60CW_P)) ||
                ((model >= MATRIX_MODEL_SATATYA_MITC50FL_28CW_S) && (model <= MATRIX_MODEL_SATATYA_PTZ_2040_P)))
        {
            /* PTZ, 8MP models, Ruggedized 2MP/5MP & Turret 5MP models have 6 text overlay */
            return TEXT_OVERLAY_MAX;
        }

    }while(0);

    /* Other cases have only 1 text overlay */
    return TEXT_OVERLAY_MIN;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ValidatePTZFeatureForBrandModel
 * @param brand
 * @param model
 * @param ptzFeature
 * @return
 */
BOOL ValidatePTZFeatureForBrandModel(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PTZ_FEATURE_e ptzFeature)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return FAIL;
    }

    PTZ_SUPPORT_TYPE_t *ptzFeaturType = &cameraBrandInfo[brand].modelInfo[model].ptzFeatureType;
    DPRINT(CAMERA_INTERFACE, "ptz info: [brand=%d], [model=%d], [ptzFeature=%d], [moveType=%d]",
           brand, model, ptzFeature, ptzFeaturType->moveType[ptzFeature]);
    if(ptzFeaturType->moveType[ptzFeature] != PTZ_CONTINOUS_MOVE)
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ValidateCamBrandModel
 * @param brand
 * @param model
 * @return
 */
BOOL ValidateCamBrandModel(CAMERA_BRAND_e brand, CAMERA_MODEL_e model)
{
    if (FALSE == IsValidCameraBrand(brand))
    {
        return FAIL;
    }

    if((model > validateModel[brand][CAMERA_MODEL_MIN]) && (model < validateModel[brand][CAMERA_MODEL_MAX]))
    {
        return SUCCESS;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Free all profiles of camera
 * @param   camIndex
 */
void FreeCameraCapability(UINT8 camIndex)
{
	STREAM_CODEC_TYPE_e			codec;
	PROFILE_WISE_OPTION_PARAM_t	*profileParam;
	UINT8						profileIndex = 0;

    MUTEX_LOCK(onvifModelParameterLock[camIndex]);
	for(profileIndex = 0; profileIndex < MAX_PROFILE_SUPPORT; profileIndex++)
	{
        if (onvifCamModelInfo[camIndex].profileWiseParam[profileIndex] == NULL)
		{
            continue;
        }

        profileParam = onvifCamModelInfo[camIndex].profileWiseParam[profileIndex];
        FREE_MEMORY(profileParam->framerateSupported.framerateAvailable);

        for (codec = VIDEO_MJPG; codec < MAX_VIDEO_CODEC; codec++)
        {
            if (profileParam->codecSupported & MX_ADD(codec))
            {
                FREE_MEMORY(((UINT8PTR *)(profileParam->resolutionSupported.resolutionAvailable))[codec - 1]);
            }
        }

        FREE_MEMORY(profileParam->resolutionSupported.resolutionAvailable);
        FREE_MEMORY(onvifCamModelInfo[camIndex].profileWiseParam[profileIndex]);
	}
    MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);

    //DPRINT(CAMERA_INTERFACE, "all onvif profiles deleted: [camera=%d]", camIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Free given profile of camera
 * @param   camIndex
 * @param   profileIndex
 */
void FreeCameraCapabilityProfileWise(UINT8 camIndex, UINT8 profileIndex)
{
    STREAM_CODEC_TYPE_e			codec;
    PROFILE_WISE_OPTION_PARAM_t	*profileParam;

    if (profileIndex >= MAX_PROFILE_SUPPORT)
    {
        EPRINT(CAMERA_INTERFACE, "invld onvif profile: [camera=%d], [profile=%d]", camIndex, profileIndex);
        return;
    }

    MUTEX_LOCK(onvifModelParameterLock[camIndex]);
    if (onvifCamModelInfo[camIndex].profileWiseParam[profileIndex] == NULL)
    {
        MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);
        return;
    }
    profileParam = onvifCamModelInfo[camIndex].profileWiseParam[profileIndex];
    FREE_MEMORY(profileParam->framerateSupported.framerateAvailable);

    for(codec = VIDEO_MJPG; codec < MAX_VIDEO_CODEC; codec++)
    {
        if (profileParam->codecSupported & MX_ADD(codec))
        {
            FREE_MEMORY(((UINT8PTR *)(profileParam->resolutionSupported.resolutionAvailable))[codec - 1]);
            profileParam->profileSupported[codec - 1] = NULL;
        }
    }

    FREE_MEMORY(profileParam->resolutionSupported.resolutionAvailable);
    FREE_MEMORY(onvifCamModelInfo[camIndex].profileWiseParam[profileIndex]);
    MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);
    //DPRINT(CAMERA_INTERFACE, "onvif profile deleted: [camera=%d], [profile=%d]", camIndex, profileIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetDefaultBitRateMax:
 * @param camIndex
 * @param profileIndex
 * @param codecNum
 * @return
 */
UINT16 GetDefaultBitRateMax(UINT8 camIndex, UINT8 profileIndex, STREAM_CODEC_TYPE_e codecNum)
{
    if(profileIndex >= MAX_PROFILE_SUPPORT)
	{
        return 0;
    }

    MUTEX_LOCK(onvifModelParameterLock[camIndex]);
    UINT16 bitRateMax = onvifCamModelInfo[camIndex].profileWiseParam[profileIndex]->maxBitRateSupport[codecNum - 1];
    MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);
	return bitRateMax;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSupportedProfileForCodec - Required By Onvif client for Media2 services
 * @param camIndex
 * @param profileIndex
 * @param codecNum
 * @param profileName
 * @return
 */
void GetSupportedProfileForCodec(UINT8 camIndex, UINT8 profileIndex, STREAM_CODEC_TYPE_e codecNum, CHARPTR profileName)
{
    if (profileName == NULL)
    {
        return;
    }

    if (profileIndex >= MAX_PROFILE_SUPPORT)
    {
        EPRINT(CAMERA_INTERFACE, "invld onvif profile: [camera=%d], [profile=%d], [codec=%d]", camIndex, profileIndex, codecNum);
        return;
    }

    MUTEX_LOCK(onvifModelParameterLock[camIndex]);
    if ((onvifCamModelInfo[camIndex].profileWiseParam[profileIndex] != NULL) &&
        (onvifCamModelInfo[camIndex].profileWiseParam[profileIndex]->profileSupported[codecNum-1] != NULL))
    {
        snprintf(profileName, CODEC_PROFILE_LEN, "%s", onvifCamModelInfo[camIndex].profileWiseParam[profileIndex]->profileSupported[codecNum-1]);
    }
    MUTEX_UNLOCK(onvifModelParameterLock[camIndex]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CheckforVideoAnalyticsSupport
 * @param   cameraIndex
 * @return
 */
MOTION_AREA_METHOD_e CheckforVideoAnalyticsSupport(UINT8 cameraIndex)
{
    if(OnvifSupportF(cameraIndex) == TRUE)
	{
        if ((onvifCamModelInfo[cameraIndex].capabilitySupported >> MOTION_WIN_CONFIG_SUPPORT) & 1)
        {
            return MOTION_AREA_METHOD_BLOCK;
        }
        return MOTION_AREA_METHOD_NO_SUPPORT;
	}

    CAMERA_BRAND_e  brand = 0;
    CAMERA_MODEL_e  model = 0;
    if(CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
	{
        GetCiCameraBrandModelNumber(cameraIndex, &brand, &model);
	}
	else
	{
        GetCameraBrandModelNumber(cameraIndex, &brand, &model);
	}

    if (FALSE == IsValidCameraBrand(brand))
    {
        return MOTION_AREA_METHOD_NO_SUPPORT;
    }

    return (cameraBrandInfo[brand].modelInfo[model].motionWindowSettingMethod);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CheckforPrivacyMaskSupport
 * @param cameraIndex
 * @return
 */
PRIVACY_MASK_METHOD_e CheckforPrivacyMaskSupport(UINT8 cameraIndex)
{
    if(OnvifSupportF(cameraIndex) == TRUE)
	{
        if ((onvifCamModelInfo[cameraIndex].capabilitySupported >> PRIVACYMASK_CONFIG_SUPPORT) & 1)
        {
            return PRIVACY_MASK_METHOD_POINT;
        }
        return PRIVACY_MASK_METHOD_NO_SUPPORT;
	}

    CAMERA_BRAND_e  brand = 0;
    CAMERA_MODEL_e  model = 0;
    if(CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
	{
        GetCiCameraBrandModelNumber(cameraIndex, &brand, &model);
	}
	else
	{
        GetCameraBrandModelNumber(cameraIndex, &brand, &model);
	}

    if (FALSE == IsValidCameraBrand(brand))
    {
        return PRIVACY_MASK_METHOD_NO_SUPPORT;
    }

    return (cameraBrandInfo[brand].modelInfo[model].privacyMaskWindowSettingMethod);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CheckOsdSupport
 * @param cameraIndex
 * @return
 */
BOOL CheckOsdSupport(UINT8 cameraIndex)
{
	CAMERA_BRAND_e brand;
	CAMERA_MODEL_e model;

    GetCameraBrandModelNumber(cameraIndex, &brand, &model);
	if(brand >= MAX_CAMERA_BRAND)
	{
		return NO;
	}

	return osdSupportedF[brand];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetSdkNumForMatrix
 * @param   model
 * @return  SDK Number
 */
MATRIX_SDK_NUM_e GetSdkNumForMatrix(CAMERA_MODEL_e model)
{
    if ((model >= MATRIX_MODEL_SATATYA_PZCR20ML_25CWP) && (model <= MATRIX_MODEL_SATATYA_CIDRP20VL_130CW))
    {
        return MATRIX_OEM_SDK;
    }
    else if ((model >= MATRIX_MODEL_SATATYA_CIBR30FL_36CG) && (model < MAX_MATRIX_CAMERA_MODEL))
    {
        return MATRIX_IP_SDK;
    }
    else
    {
        return MAX_MATRIX_SDK;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Privacy mask mapping required or not for camera brand
 * @param   brand
 * @param   model
 * @return  Returns TRUE if required else returns FALSE
 */
BOOL IsPrivacyMaskMappingRequired(CAMERA_BRAND_e brand, CAMERA_MODEL_e model)
{
    if (brand != CAMERA_BRAND_MATRIX)
    {
        return TRUE;
    }

    /* OEM camera doesn't have privacy mask support as per matrix's requirement */
    if ((model > MATRIX_MODEL_NONE) && (model < MATRIX_MODEL_SATATYA_CIBR30FL_36CG))
    {
        return FALSE;
    }

    /* All matrix's camera support privacy mask */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetTheBitRateValue
 * @param   bitrate
 * @param   value
 * @return
 */
BOOL GetTheBitRateValue(CHARPTR bitrate, BITRATE_VALUE_e *value)
{
    UINT64  tagValue;
    UINT32  intValue;

    if(SUCCESS != AsciiToInt(bitrate, &tagValue))
	{
        EPRINT(CAMERA_INTERFACE, "fail to convert ascii to int");
		return FAIL;
	}

    intValue = (UINT32)tagValue;
    for (*value = 0; *value < MAX_BITRATE_VALUE; (*value)++)
    {
        if (intValue <= bitRateValueForMatrixIP[*value])
        {
            return SUCCESS;
        }
    }

    EPRINT(CAMERA_INTERFACE, "bitrate value not handled: [bitrate=%s]", bitrate);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ConvertCameraResolutionToString
 * @param resolution
 * @param width
 * @param height
 * @return
 */
void ConvertCameraResolutionToString(CHARPTR resolution, UINT16 width, UINT16 height)
{
    if ((width != 0) && (height != 0))
    {
        snprintf(resolution, MAX_RESOLUTION_NAME_LEN, "%hdx%hd", width, height);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetMaxSupportedPrivacyMaskWindow
 * @param brand
 * @param model
 * @return
 */
UINT8 GetMaxSupportedPrivacyMaskWindow(CAMERA_BRAND_e brand, CAMERA_MODEL_e model)
{
	return (cameraBrandInfo[brand].modelInfo[model].maxSupportedPrivacyMaskWindow);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief IsNoMotionEventSupported
 * @param cameraIndex
 * @return
 */
BOOL IsNoMotionEventSupported(UINT8 cameraIndex)
{
    CAMERA_CAPABILTY_INFO_t cameraCapability;

    if (CMD_SUCCESS != GetSupportedCapability(cameraIndex, &cameraCapability))
    {
        return NO;
    }

    return (cameraCapability.noMotionDetectionSupport);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get CI camera brand and model number from camera brand and model string
 * @param   brandStr
 * @param   modelStr
 * @param   brand
 * @param   model
 * @return
 */
void GetCiCameraBrandModelNumber(UINT8 cameraIndex, CAMERA_BRAND_e *brand, CAMERA_MODEL_e* model)
{
    CAMERA_BRAND_e  brandIndex;
    CAMERA_MODEL_e  modelIndex;
    CHARPTR         brandStr, modelStr;

    if ((brand == NULL) || (model == NULL))
	{
        return;
    }

    GetCameraBrandModelString(cameraIndex, &brandStr, &modelStr);
    for (brandIndex = CAMERA_BRAND_NONE; brandIndex < MAX_CAMERA_BRAND; brandIndex++)
    {
        if((strcasecmp(brandStr, cameraBrandInfo[brandIndex].brandName)) == 0)
        {
            *brand = brandIndex;
            break;
        }
    }

    if(brandIndex >= MAX_CAMERA_BRAND)
    {
        return;
    }

    for (modelIndex = 0; modelIndex < cameraBrandInfo[brandIndex].maxBrandModel; modelIndex++)
    {
        if((strcasecmp(modelStr, cameraBrandInfo[brandIndex].modelInfo[modelIndex].modelName)) == 0)
        {
            *model = modelIndex;
            break;
        }
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
