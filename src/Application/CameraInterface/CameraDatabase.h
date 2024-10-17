#ifndef CAMERADATABASE_H_
#define CAMERADATABASE_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		CameraDatabase.h
@brief      This file provides interface for camera database
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <math.h>

/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
//macro to add elements
#define MX_ADD(a)                           ((UINT64)1 << a)
#define AddRange(MIN_RANGE,MAX_RANGE)       ((((UINT64)1<<MAX_RANGE)-((UINT64)1<<(MIN_RANGE-(UINT64)1)))<<(UINT64)1)
#define MAX_CAMERADETAIL_USERNAME_WIDTH     17
#define MAX_CAMERADETAIL_PASSWORD_WIDTH     9
#define MAX_PROFILE_SUPPORT                 5
#define CODEC_PROFILE_LEN                   50

/* Maximum camera model for brand */
#define CAMERA_BRAND_MODEL_MAX              MAX_MATRIX_CAMERA_MODEL

#define MATRIX_CAM_HTTP_RESP_CODE_STR       "response-code="
#define MATRIX_CAM_HTTP_RESP_CODE_0_STR     "response-code=0"
#define MATRIX_CAM_HTTP_RESP_CODE_7_STR     "response-code=7"

#define SET_IMAGE_SETTING_VALUE(value, step, min, least)        roundf(step ? (value / step) + ((min == least) ? 0 : min) : value + ((min == least) ? 0 : min))
#define GET_IMAGE_SETTING_STEP_VALUE(min, max, steps)           (float)steps/((max > min) ? (max - min) : 1)
#define IS_VALID_IMAGE_SETTING_RANGE(min, max)                  ((min != max) && (max > min))
#define GET_IMAGE_SETTING_VALUE(value, step, min, max, least)   roundf((value > max) ? (max - ((min == least) ? 0 : min)) * step : \
                                                                    ((value <= min) ? least : (value - ((min == least) ? 0 : min)) * step))

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef UINT32  CAPABILITY_TYPE;

typedef enum
{
    CAMERA_BRAND_NONE = 0,  // 0 : no camera brand
    CAMERA_BRAND_GENERIC,   // 1 : generic camera
    CAMERA_BRAND_MATRIX,    // 2 : matrix camera
    MAX_CAMERA_BRAND        // 3 : max number of camera brand

}CAMERA_BRAND_e;

typedef enum
{
    CAMERA_MODEL_NONE = 0,
	MAX_NONE_CAMERA_MODEL,

	//----------------------------------

    GENERIC_CAMERA_MODEL_NONE = 0,
    GENERIC_HTTP_MODEL,
    GENERIC_RTSP_UDP_MODEL,
    GENERIC_RTSP_TCP_MODEL,
	MAX_GENERIC_CAMERA_MODEL,

    //----------------------------------

	//MATRIX BRAND
    MATRIX_MODEL_NONE = 0,

    /* TIANDY OEM camera */
    MATRIX_MODEL_SATATYA_PZCR20ML_25CWP,
    MATRIX_MODEL_SATATYA_PZCR20ML_33CWP,

    /* ZAVIO OEM camera */
    MATRIX_MODEL_SATATYA_CIBR13FL_40CW,
    MATRIX_MODEL_SATATYA_CIDR13FL_40CW,

    /* HIK-VISION OEM camera */
    MATRIX_MODEL_SATATYA_CIDRP20VL_130CW,

    MATRIX_MODEL_SATATYA_CIBR30FL_36CG,
    MATRIX_MODEL_SATATYA_CIBR30FL_60CG,
    MATRIX_MODEL_SATATYA_CIDR30FL_36CW,
    MATRIX_MODEL_SATATYA_CIDR30FL_60CW,
    MATRIX_MODEL_SATATYA_CIDR20FL_36CW_S,
    MATRIX_MODEL_SATATYA_CIDR20FL_60CW_S,
    MATRIX_MODEL_SATATYA_CIDR20VL_12CW_S,
    MATRIX_MODEL_SATATYA_CIBR20FL_36CW_S,
    MATRIX_MODEL_SATATYA_CIBR20FL_60CW_S,
    MATRIX_MODEL_SATATYA_CIBR20VL_12CW_S,
    MATRIX_MODEL_SATATYA_CIDR30FL_36CW_S,
    MATRIX_MODEL_SATATYA_CIDR30FL_60CW_S,
    MATRIX_MODEL_SATATYA_CIDR30VL_12CW_S,
    MATRIX_MODEL_SATATYA_CIBR30FL_36CW_S,
    MATRIX_MODEL_SATATYA_CIBR30FL_60CW_S,
    MATRIX_MODEL_SATATYA_CIBR30VL_12CW_S,
    MATRIX_MODEL_SATATYA_CIDR20VL_12CW_P,
    MATRIX_MODEL_SATATYA_CIBR20VL_12CW_P,
    MATRIX_MODEL_SATATYA_CIDR30VL_12CW_P,
    MATRIX_MODEL_SATATYA_CIBR30VL_12CW_P,
    MATRIX_MODEL_SATATYA_CIDR20FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIBR20FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIDR30FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIBR30FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIDR20FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIDR20FL_36CW_S,
    MATRIX_MODEL_SATATYA_MIDR20FL_60CW_S,
    MATRIX_MODEL_SATATYA_MIDR20FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIDR20FL_36CW_P,
    MATRIX_MODEL_SATATYA_MIDR20FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIDR20FL_36CW_P,
    MATRIX_MODEL_SATATYA_CIBR20FL_36CW_P,
    MATRIX_MODEL_SATATYA_CIDR20FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIBR20FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIDR30FL_36CW_P,
    MATRIX_MODEL_SATATYA_CIBR30FL_36CW_P,
    MATRIX_MODEL_SATATYA_CIDR30FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIBR30FL_60CW_P,

    MATRIX_MODEL_SATATYA_MIDR30FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIDR30FL_36CW_S,
    MATRIX_MODEL_SATATYA_MIDR30FL_60CW_S,
    MATRIX_MODEL_SATATYA_MIBR30FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIBR30FL_36CW_S,
    MATRIX_MODEL_SATATYA_MIBR30FL_60CW_S,

    MATRIX_MODEL_SATATYA_MIDR30FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIDR30FL_36CW_P,
    MATRIX_MODEL_SATATYA_MIDR30FL_60CW_P,
    MATRIX_MODEL_SATATYA_MIBR30FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIBR30FL_36CW_P,
    MATRIX_MODEL_SATATYA_MIBR30FL_60CW_P,

    MATRIX_MODEL_SATATYA_MIBR20FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIBR20FL_36CW_S,
    MATRIX_MODEL_SATATYA_MIBR20FL_60CW_S,

    MATRIX_MODEL_SATATYA_MIBR20FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIBR20FL_36CW_P,
    MATRIX_MODEL_SATATYA_MIBR20FL_60CW_P,

	// 5 MP Standard Variants Models
    MATRIX_MODEL_SATATYA_CIBR50FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIBR50FL_40CW_S,
    MATRIX_MODEL_SATATYA_CIBR50FL_60CW_S,
    MATRIX_MODEL_SATATYA_CIDR50FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIDR50FL_40CW_S,
    MATRIX_MODEL_SATATYA_CIDR50FL_60CW_S,
    MATRIX_MODEL_SATATYA_CIBR50VL_12CW_S,
    MATRIX_MODEL_SATATYA_CIDR50VL_12CW_S,

	// 5 MP Premium Variants Models
    MATRIX_MODEL_SATATYA_CIBR50FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIBR50FL_40CW_P,
    MATRIX_MODEL_SATATYA_CIBR50FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIDR50FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIDR50FL_40CW_P,
    MATRIX_MODEL_SATATYA_CIDR50FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIBR50VL_12CW_P,
    MATRIX_MODEL_SATATYA_CIDR50VL_12CW_P,

    MATRIX_MODEL_SATATYA_MIBR50FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIBR50FL_40CW_S,
    MATRIX_MODEL_SATATYA_MIBR50FL_60CW_S,
    MATRIX_MODEL_SATATYA_MIDR50FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIDR50FL_40CW_S,
    MATRIX_MODEL_SATATYA_MIDR50FL_60CW_S,
    MATRIX_MODEL_SATATYA_MIBR50FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIBR50FL_40CW_P,
    MATRIX_MODEL_SATATYA_MIBR50FL_60CW_P,
    MATRIX_MODEL_SATATYA_MIDR50FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIDR50FL_40CW_P,
    MATRIX_MODEL_SATATYA_MIDR50FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIDR20FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIBR20FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIDR30FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIBR30FL_28CW_P,

    // NETRA-8MP Project Series Standard Variants
    MATRIX_MODEL_SATATYA_CIBR80FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIBR80FL_36CW_S,
    MATRIX_MODEL_SATATYA_CIBR80FL_60CW_S,

    MATRIX_MODEL_SATATYA_CIDR80FL_28CW_S,
    MATRIX_MODEL_SATATYA_CIDR80FL_36CW_S,
    MATRIX_MODEL_SATATYA_CIDR80FL_60CW_S,

    // NETRA-8MP Project Series Premium Variants
    MATRIX_MODEL_SATATYA_CIBR80FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIBR80FL_36CW_P,
    MATRIX_MODEL_SATATYA_CIBR80FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIBR80ML_12CW_P,

    MATRIX_MODEL_SATATYA_CIDR80FL_28CW_P,
    MATRIX_MODEL_SATATYA_CIDR80FL_36CW_P,
    MATRIX_MODEL_SATATYA_CIDR80FL_60CW_P,
    MATRIX_MODEL_SATATYA_CIDR80ML_12CW_P,

    // SAMIKSHA-8MP Professional Series Standard Variants
    MATRIX_MODEL_SATATYA_MIBR80FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIBR80FL_36CW_S,
    MATRIX_MODEL_SATATYA_MIBR80FL_60CW_S,

    MATRIX_MODEL_SATATYA_MIDR80FL_28CW_S,
    MATRIX_MODEL_SATATYA_MIDR80FL_36CW_S,
    MATRIX_MODEL_SATATYA_MIDR80FL_60CW_S,

    // SAMIKSHA-8MP Professional Series Premium Variants
    MATRIX_MODEL_SATATYA_MIBR80FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIBR80FL_36CW_P,
    MATRIX_MODEL_SATATYA_MIBR80FL_60CW_P,

    MATRIX_MODEL_SATATYA_MIDR80FL_28CW_P,
    MATRIX_MODEL_SATATYA_MIDR80FL_36CW_P,
    MATRIX_MODEL_SATATYA_MIDR80FL_60CW_P,

    // TURRET-2MP Project Series Premium Variants
    MATRIX_MODEL_SATATYA_MITR20FL_28CW_S,
    MATRIX_MODEL_SATATYA_MITR20FL_36CW_S,
    MATRIX_MODEL_SATATYA_MITR20FL_60CW_S,

    // TURRET-5MP Project Series Premium Variants
    MATRIX_MODEL_SATATYA_MITR50FL_28CW_S,
    MATRIX_MODEL_SATATYA_MITR50FL_40CW_S,
    MATRIX_MODEL_SATATYA_MITR50FL_60CW_S,

    // RUGGEDIZED-2MP
    MATRIX_MODEL_SATATYA_RIDR20FL_28CW_P,
    MATRIX_MODEL_SATATYA_RIDR20FL_36CW_P,
    MATRIX_MODEL_SATATYA_RIDR20FL_60CW_P,

    // RUGGEDIZED-5MP
    MATRIX_MODEL_SATATYA_RIDR50FL_28CW_P,
    MATRIX_MODEL_SATATYA_RIDR50FL_40CW_P,
    MATRIX_MODEL_SATATYA_RIDR50FL_60CW_P,

    // TURRET-2MP All Color
    MATRIX_MODEL_SATATYA_MITC20FL_28CW_S,
    MATRIX_MODEL_SATATYA_MITC20FL_36CW_S,
    MATRIX_MODEL_SATATYA_MITC20FL_60CW_S,

    // TURRET-5MP All Color
    MATRIX_MODEL_SATATYA_MITC50FL_28CW_S,
    MATRIX_MODEL_SATATYA_MITC50FL_40CW_S,
    MATRIX_MODEL_SATATYA_MITC50FL_60CW_S,

    // MATRIX PTZ Camera
    MATRIX_MODEL_SATATYA_PTZ_2040_P,

	MAX_MATRIX_CAMERA_MODEL,
}CAMERA_MODEL_e;

typedef enum
{
	RESOLUTION_NONE = 0,	//index 0
	RESOLUTION_160x90,		//index 1
	RESOLUTION_160x100,		//index 2
	RESOLUTION_160x112,		//index 3
	RESOLUTION_160x120_QQVGA,//index 4
	RESOLUTION_160x128,		//index 5
	RESOLUTION_176x112,		//index 6
	RESOLUTION_176x120,		//index 7
	RESOLUTION_176x144_QCIF,//index 8
	RESOLUTION_192x144,		//index 9
	RESOLUTION_192x192,		//index 10
	RESOLUTION_240x135,		//index 11
	RESOLUTION_240x180,		//index 12
	RESOLUTION_256x192,		//index 13
	RESOLUTION_256x256,		//index 14
	RESOLUTION_320x180,		//index 15
	RESOLUTION_320x192,		//index 16
	RESOLUTION_320x200,		//index 17
	RESOLUTION_320x240_QVGA,//index 18
	RESOLUTION_320x256,		//index 19
	RESOLUTION_320x320,		//index 20
	RESOLUTION_352x240,		//index 21
	RESOLUTION_352x244,		//index 22
	RESOLUTION_352x288_CIF,	//index 23
	RESOLUTION_384x216,		//index 24
	RESOLUTION_384x288,		//index 25
	RESOLUTION_384x384,		//index 26
	RESOLUTION_480x270,		//index 27
	RESOLUTION_480x300,		//index 28
	RESOLUTION_480x360,		//index 29
	RESOLUTION_512x384,		//index 30
	RESOLUTION_512x512,		//index 31
	RESOLUTION_528x320,		//index 32
	RESOLUTION_528x328,		//index 33
	RESOLUTION_640x360,		//index 34
	RESOLUTION_640x368,		//index 35
	RESOLUTION_640x400,		//index 36
	RESOLUTION_640x480_VGA,	//index 37
	RESOLUTION_640x512,		//index 38
	RESOLUTION_704x240,		//index 39
	RESOLUTION_704x288_2CIF,//index 40
	RESOLUTION_704x480,		//index 41
	RESOLUTION_704x570,		//index 42
	RESOLUTION_704x576_2CIFEXP,	//index 43
	RESOLUTION_720x480,		//index 44
	RESOLUTION_720x576,		//index 45
	RESOLUTION_768x576,		//index 46
	RESOLUTION_768x768,		//index 47
	RESOLUTION_800x450,		//index 48
	RESOLUTION_800x480,		//index 49
	RESOLUTION_800x500,		//index 50
	RESOLUTION_800x600_SVGA,//index 51
	RESOLUTION_860x540,		//index 52
	RESOLUTION_960x480,		//index 53
	RESOLUTION_960x540,		//index 54
	RESOLUTION_960x576,		//index 55
	RESOLUTION_960x720,		//index 56
	RESOLUTION_960x768,		//index 57
	RESOLUTION_1024x576,	//index 58
	RESOLUTION_1024x640,	//index 59
	RESOLUTION_1024x768,	//index 60
	RESOLUTION_1056x1056,	//index 61
	RESOLUTION_1140x1080,	//index 62
	RESOLUTION_1280x720,	//index 63
	RESOLUTION_1280x800,	//index 64
	RESOLUTION_1280x960,	//index 65
	RESOLUTION_1280x1024_SXGA,//index 66
	RESOLUTION_1280x1280,	//index 67
	RESOLUTION_1286x972,	//index 68
	RESOLUTION_1296x968,	//index 69
	RESOLUTION_1296x972,	//index 70
	RESOLUTION_1360x768,	//index 71
	RESOLUTION_1376x768,	//index 72
	RESOLUTION_1440x912,	//index 73
	RESOLUTION_1472x960,	//index 74
	RESOLUTION_1536x1536,	//index 75
	RESOLUTION_1600x904,	//index 76
	RESOLUTION_1600x912,	//index 77
	RESOLUTION_1600x1200,	//index 78
	RESOLUTION_1680x1056,	//index 79
	RESOLUTION_1824x1376,	//index 80
	RESOLUTION_1920x1080,	//index 81
	RESOLUTION_1920x1200,	//index 82
	RESOLUTION_1920x1440,	//index 83
	RESOLUTION_2032x1920,	//index 84
	RESOLUTION_2048x1536,	//index 85
	RESOLUTION_2560x1600,	//index 86
	RESOLUTION_2560x1920,	//index 87
	RESOLUTION_2592x1944,	//index 88
	RESOLUTION_2944x1920,	//index 89
	RESOLUTION_3648x2752,	//index 90
	RESOLUTION_1408x920,	//index 91
	RESOLUTION_1120x630,	//index 92
	RESOLUTION_528x384,		//index 93
	RESOLUTION_1600x720,	//index 94
	RESOLUTION_2592x1520,	//index 95
	RESOLUTION_2688x1520,	//index 96
	RESOLUTION_2560x1440,	//index 97
	RESOLUTION_4096x2160,	//index 98
	RESOLUTION_3840x2160,	//index 99
	RESOLUTION_2560x2048,	//index 100
	RESOLUTION_1400x1050,	//index 101
	RESOLUTION_2304x1296,	//index 102
	RESOLUTION_3072x2048,   //index 103
	RESOLUTION_3072x1728,   //index 104
	RESOLUTION_2944x1656,   //index 105
    RESOLUTION_3200x1800,   //index 106
	MAX_RESOLUTION
}RESOLUTION_e;

static const UINT16 onvifBitrateValue[MAX_BITRATE_VALUE] =
{
	32,
	64,
	128,
	256,
	384,
	512,
	768,
	1024,
	1536,
	2048,
	3072,
	4096,
	6144,
	8192,
	12288,
	16384,
};

//camera Capability related Parameters
typedef enum
{
	AUDIO_MIC_SUPPORT,
	AUDIO_SPEAKER_SUPPORT,
	MOTION_DETECTION_SUPPORT,
	TAMPER_DETECTION_SUPPORT,
	AUDIO_DETECTION_SUPPORT,
	CAMERA_ALARM_INPUT1_SUPPORT,
	CAMERA_ALARM_INPUT2_SUPPORT,
	CAMERA_ALARM_INPUT3_SUPPORT,
	CAMERA_ALARM_INPUT4_SUPPORT,
	CAMERA_ALARM_OUTPUT1_SUPPORT,
	CAMERA_ALARM_OUTPUT2_SUPPORT,
	CAMERA_ALARM_OUTPUT3_SUPPORT,
	CAMERA_ALARM_OUTPUT4_SUPPORT,
	PTZ_SUPPORT,
	FOCUS_SUPPORT,
	IRIS_SUPPORT,

	MOTION_WIN_CONFIG_SUPPORT,
	PRIVACYMASK_CONFIG_SUPPORT,

	LINE_CROSSING_SUPPORT,
	OBJECT_INTRUSION_SUPPORT,
	AUDIO_EXCEPTION_SUPPORT,
	MISSING_OBJECT_SUPPORT,
	SUSPICIOUS_OBJECT_SUPPORT,
	LOITERING_SUPPORT,
    OBJECT_COUNTING_SUPPORT,
    NO_MOTION_DETECTION_SUPPORT,
    MAX_CAMERA_CAPABILITY
}CAMERA_CAPABILITY_e;

typedef enum
{
	PTZ_MOVE_NOT_SUPPORTED,
	PTZ_RELATIVE_MOVE,
	PTZ_ABSOLUTE_MOVE,
	PTZ_CONTINOUS_MOVE,
	MAX_PTZ_MOVE_TYPE
}PTZ_TYPE_e;

typedef enum
{
	PTZ_PAN_CONTROL,
	PTZ_TILT_CONTROL,
	PTZ_ZOOM_CONTROL,
	PTZ_FOCUS_CONTROL,
	PTZ_IRIS_CONTROL,
	MAX_PTZ_CONTROL_TYPE

}PTZ_FEATURE_e;

typedef enum
{
    MATRIX_OEM_SDK = 0,
	MATRIX_IP_SDK,
	MAX_MATRIX_SDK,

}MATRIX_SDK_NUM_e;

typedef enum
{
    MOTION_AREA_METHOD_POINT = 0,
    MOTION_AREA_METHOD_BLOCK,
    MOTION_AREA_METHOD_NO_SUPPORT
}MOTION_AREA_METHOD_e;

typedef enum
{
    PRIVACY_MASK_METHOD_POINT = 0,
    PRIVACY_MASK_METHOD_BLOCK,
    PRIVACY_MASK_METHOD_NO_SUPPORT
}PRIVACY_MASK_METHOD_e;

typedef struct
{
    BOOL        isResolutionCodecDependent;
    UINT8PTR    resolutionAvailable;
}RESOLUTION_PARAMETER_t;

typedef struct
{
    BOOL        isFramerateCodecDependent;
    BOOL        isFramerateResolutionDependent;
    UINT64PTR   framerateAvailable;
}FRAMERATE_PARAMETER_t;

//structure for storing the model wise camera supported parameter
typedef struct
{
    UINT8                   codecSupported;
    UINT8                   codecSupportedSub;
    UINT8PTR                profileWiseCodecSupported;
    BOOL                    isParamProfileDependent;
    RESOLUTION_PARAMETER_t  resolutionSupported;
    RESOLUTION_PARAMETER_t  resolutionSupportedSub;
    FRAMERATE_PARAMETER_t   framerateSupported;
    FRAMERATE_PARAMETER_t   framerateSupportedSub;
    UINT8                   qualitySupported[MAX_VIDEO_CODEC - 1];
    UINT8                   qualitySupportedSub[MAX_VIDEO_CODEC - 1];
    CAPABILITY_TYPE         capabilitySupported;

}CAMERA_MODEL_PARAMETER_t;

typedef struct
{
    PTZ_TYPE_e  moveType[MAX_PTZ_CONTROL_TYPE];
}PTZ_SUPPORT_TYPE_t;

// onvif model parameter
typedef struct
{
	UINT8 						codecSupported;
	RESOLUTION_PARAMETER_t  	resolutionSupported;
	FRAMERATE_PARAMETER_t 		framerateSupported;
	UINT8						qualitySupported[MAX_VIDEO_CODEC - 1];
	UINT16						maxBitRateSupport[MAX_VIDEO_CODEC - 1];
	UINT16						minBitRateSupport[MAX_VIDEO_CODEC - 1];
    CHARPTR                     profileSupported[MAX_VIDEO_CODEC-1];           //required for Media2
}PROFILE_WISE_OPTION_PARAM_t;

//structure for storing the model wise camera supported parameter
typedef struct
{
    PROFILE_WISE_OPTION_PARAM_t     *profileWiseParam[MAX_PROFILE_SUPPORT];
	CAPABILITY_TYPE 				capabilitySupported;
}CAMERA_ONVIF_MODEL_PARAMETER_t;

typedef struct
{
	UINT8						maxSupportedProfile;
}CAMERA_PROFILE_SPECIFIC_PARAM_t;

/* Structure for storing camera model information */
typedef struct
{
	CHARPTR 				 		 	modelName;
	CAMERA_MODEL_PARAMETER_t 			*modelParameter;
	CAMERA_PROFILE_SPECIFIC_PARAM_t	 	profileParam;
	PTZ_SUPPORT_TYPE_t					ptzFeatureType;
    MOTION_AREA_METHOD_e                motionWindowSettingMethod;
    PRIVACY_MASK_METHOD_e               privacyMaskWindowSettingMethod;
	BOOL								osdSupportF;
	UINT8								maxSupportedPrivacyMaskWindow;

}CAMERA_MODEL_INFO_t;

typedef struct
{
	CAMERA_MODEL_INFO_t 		modelInfo;
	pthread_mutex_t     		onvifModelParameterLock;
}ONVIF_MODEL_INFO_t;

//structure for storing cameraBrand Info
typedef struct
{
	CHARPTR						brandName;
	UINT8 						maxBrandModel;
	CHAR 						username[MAX_CAMERA_USERNAME_WIDTH];
	CHAR 						password[MAX_CAMERA_PASSWORD_WIDTH];
    CAMERA_MODEL_INFO_t 		*modelInfo;
}CAMERA_BRAND_INFO_t;

typedef struct
{
    CAMERA_MODEL_e  cameraModel;
    CHARPTR         modelName;
}OLD_MATRIX_CAMERA_MODEL_NAME_t;

//structure for storing camera capability Information
typedef struct
{
    BOOL    audioSupport;
    BOOL    motionDetectionSupport;
    BOOL    noMotionDetectionSupport;
    BOOL    viewTamperSupport;
    BOOL    audioDetectionSupport;
    UINT8   maxSensorInput;
    UINT8   maxAlarmOutput;
    BOOL    ptzSupport;
    BOOL    motionWinConfigSupport;
    BOOL    privacymaskWinConfigSupport;
    BOOL    lineCrossingSupport;
    BOOL    objectIntrusionSupport;
    BOOL    audioExceptionSupport;
    BOOL    missingObjectSupport;
    BOOL    suspiciousObjectSupport;
    BOOL    loiteringSupport;
    BOOL    objectCounting;
}CAMERA_CAPABILTY_INFO_t;

typedef struct
{
    UINT32  min;
    UINT32  max;
    UINT32  value;
    float   step;
}IMG_CAP_RANGE_INFO_t;

typedef struct
{
    UINT8   modeSupported;
    UINT8   mode;
}IMG_CAP_MODE_INFO_t;

typedef struct
{
    IMG_CAP_MODE_INFO_t     modeInfo;
    IMG_CAP_RANGE_INFO_t    range;
}IMG_CAP_MODE_RANGE_INFO_t;

/* structure for storing supported image settings parameter and its range */
typedef struct
{
    CAPABILITY_TYPE         imagingCapability;
    IMG_CAP_RANGE_INFO_t    brightness;
    IMG_CAP_RANGE_INFO_t    contrast;
    IMG_CAP_RANGE_INFO_t    saturation;
    IMG_CAP_RANGE_INFO_t    hue;
    IMG_CAP_RANGE_INFO_t    sharpness;
    IMG_CAP_MODE_INFO_t     whiteBalance;
    IMG_CAP_MODE_INFO_t     wdr;
    IMG_CAP_RANGE_INFO_t    wdrStrength;
    IMG_CAP_MODE_INFO_t     backlightControl;
    IMG_CAP_MODE_INFO_t     exposureRatioMode;
    IMG_CAP_RANGE_INFO_t    exposureRatio;
    IMG_CAP_MODE_INFO_t     exposureMode;
    IMG_CAP_MODE_INFO_t     flicker;
    IMG_CAP_RANGE_INFO_t    flickerStrength;
    IMG_CAP_MODE_INFO_t     hlc;
    IMG_CAP_RANGE_INFO_t    exposureTime;
    IMG_CAP_RANGE_INFO_t    exposureGain;
    IMG_CAP_RANGE_INFO_t    exposureIris;
    IMG_CAP_MODE_INFO_t     normalLightGain;
    IMG_CAP_RANGE_INFO_t    normalLightLuminance;
    IMG_CAP_MODE_INFO_t     irLed;
    IMG_CAP_RANGE_INFO_t    irLedSensitivity;
}IMAGE_CAPABILITY_INFO_t;

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
/* Global variables present in MatrixUrl.c & MatrixDatabase.c with external linkage to other files */
extern const CHARPTR                            actualCodecStr[MAX_VIDEO_CODEC];
extern const CHARPTR                            resolutionStrForMatrixIP[MATRIX_IP_CAM_MAX_RESOLUTION];
extern const CHARPTR                            resolutionStrForSetMatrixIP[MATRIX_IP_CAM_MAX_RESOLUTION];
extern const UINT32                             bitRateValueForMatrixIP[MAX_BITRATE_VALUE];
extern const CHARPTR                            bitRateModeStrForMatrixIP[MAX_BITRATE_MODE];
extern const CHARPTR                            codecStrForSetMatrixIP[];
extern const CAMERA_MODEL_INFO_t                MatrixModelInfo[MAX_MATRIX_CAMERA_MODEL];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitOnvifModelParameter(void);
//-------------------------------------------------------------------------------------------------
BOOL IsValidCameraBrand(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetCameraBrand(CHARPTR *brand, UINT8PTR maxCameraBrand);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetCameraModels(CHARPTR brand,CHARPTR *model, UINT8PTR maxCameraModel);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetCameraUsernamePassword(CHARPTR brand, CHARPTR username, CHARPTR password);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetSupportedCodec(UINT8 cameraIndex, VIDEO_TYPE_e streamType, UINT8 profileNo,
                                   CHARPTR *supportedCodec, UINT8PTR maxSupportedCodec);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetMaxSupportedProfile(UINT8 cameraIndex,UINT8PTR numOfProfSupported);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetSupportedResolution(UINT8 cameraIndex, CHARPTR codecStrPtr, VIDEO_TYPE_e streamType, UINT8 profileNo,
                                        CHARPTR *supportedResolution, UINT8PTR maxSupportedResolution);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetSupportedFramerate(UINT8 camIndex, CHARPTR encoder, CHARPTR resolution, VIDEO_TYPE_e streamType, UINT8 profileNo, UINT64PTR framerateVal);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetSupportedBitRate(UINT8 cameraIndex, CHARPTR codecStr, VIDEO_TYPE_e streamType, UINT8 profileNo, CHARPTR *supportedBitrate,
                                     UINT8PTR maxSupportedBitrate, BITRATE_VALUE_e *minBitrateIndex, BITRATE_VALUE_e *maxBitrateIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetSupportedQuality(UINT8 camIndex, CHARPTR codecStr, VIDEO_TYPE_e streamType,
                                     UINT8 profileNo,UINT8PTR supportedQuality);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetSupportedCapability(UINT8 camIndex, CAMERA_CAPABILTY_INFO_t *camCapability);
//-------------------------------------------------------------------------------------------------
BOOL GetBrandNum(CHARPTR brand, CAMERA_BRAND_e *brandNum);
//-------------------------------------------------------------------------------------------------
BOOL GetModelNum(CHARPTR brand,CHARPTR model, CAMERA_MODEL_e *modelNum);
//-------------------------------------------------------------------------------------------------
BOOL GetVideoCodecNum(CHARPTR codec, STREAM_CODEC_TYPE_e *codecNum);
//-------------------------------------------------------------------------------------------------
void SaveOnvifCameraCapabiltyProfileWise(UINT8 cameraIndex, UINT8 profileIndex, PROFILE_WISE_OPTION_PARAM_t *onvifProfParam);
//-------------------------------------------------------------------------------------------------
void SaveOnvifCameraCapabilty(UINT8 cameraIndex, CAPABILITY_TYPE capabillity);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionId(UINT16 width, UINT16 height, RESOLUTION_e * resolPtr);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionNum(CHARPTR resolStr,RESOLUTION_e * resolutionIndex);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionHeightWidth(CHARPTR resolution, UINT16PTR height, UINT16PTR width);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionHeightWidthString(CHARPTR resolution, CHARPTR resolutionHeight,CHARPTR resolutionWidth,
                                    const UINT8 resHeightLen, const UINT8 resWidthLen);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionString(CHARPTR resolution, RESOLUTION_e resIndex);
//-------------------------------------------------------------------------------------------------
UINT8 GetSupportedTextOverlay(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
BOOL ValidateCamBrandModel(CAMERA_BRAND_e brand, CAMERA_MODEL_e model);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetAndValidateCameraBrandModel(UINT8 camIndex, CAMERA_BRAND_e *pBrand, CAMERA_MODEL_e *pModel);
//-------------------------------------------------------------------------------------------------
BOOL ValidatePTZFeatureForBrandModel(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PTZ_FEATURE_e ptzFeature);
//-------------------------------------------------------------------------------------------------
void FreeCameraCapability(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
void FreeCameraCapabilityProfileWise(UINT8 camIndex, UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
UINT16 GetDefaultBitRateMax(UINT8 camIndex, UINT8 profileIndex, STREAM_CODEC_TYPE_e codecNum);
//-------------------------------------------------------------------------------------------------
MOTION_AREA_METHOD_e CheckforVideoAnalyticsSupport(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
PRIVACY_MASK_METHOD_e CheckforPrivacyMaskSupport(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
BOOL CheckOsdSupport(UINT8 cameraIndex);
//------------------------------------------------------------------------------------------------
MATRIX_SDK_NUM_e GetSdkNumForMatrix(CAMERA_MODEL_e model);
//-------------------------------------------------------------------------------------------------
BOOL GetTheBitRateValue(CHARPTR bitrate, BITRATE_VALUE_e *value);
//-------------------------------------------------------------------------------------------------
BOOL IsPrivacyMaskMappingRequired(CAMERA_BRAND_e brand, CAMERA_MODEL_e model);
//-------------------------------------------------------------------------------------------------
void ConvertCameraResolutionToString(CHARPTR resolution, UINT16 width, UINT16 height);
//-------------------------------------------------------------------------------------------------
UINT8 GetMaxSupportedPrivacyMaskWindow(CAMERA_BRAND_e brand, CAMERA_MODEL_e model);
//-------------------------------------------------------------------------------------------------
BOOL IsNoMotionEventSupported(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void GetCiCameraBrandModelNumber(UINT8 cameraIndex, CAMERA_BRAND_e *brand, CAMERA_MODEL_e* model);
//-------------------------------------------------------------------------------------------------
void GetSupportedProfileForCodec(UINT8 camIndex, UINT8 profileIndex, STREAM_CODEC_TYPE_e codecNum,CHARPTR profileName);
//-------------------------------------------------------------------------------------------------
void GetUpdatedMatrixCameraModelName(CHAR *pCameraModel, UINT8 modelLenMax);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionNoGetProfileforMatrixIpCamera(CHARPTR resolutionStr, MATRIX_IP_CAM_RESOLUTION_e *resolutionNo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* CAMERADATABASE_H_ */
 
