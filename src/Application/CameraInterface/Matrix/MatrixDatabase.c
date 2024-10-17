//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MatrixDataBase.c
@brief      This File Contains Database of All Matrix Camera Models supported by NVR.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "CameraDatabase.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MATRIX_OEM_SUPPORTED_FPS    (MX_ADD(1) | MX_ADD (2) | MX_ADD(4) | MX_ADD(6) | MX_ADD(8) \
                                    | MX_ADD(10) | MX_ADD(12) | MX_ADD(15) | MX_ADD(16) |MX_ADD(18) \
                                    | MX_ADD(20) | MX_ADD(22) | MX_ADD(25))

#define TIANDY_OEM_SUPPORTED_25FPS  (MX_ADD(1) | MX_ADD(5) | MX_ADD(10) | MX_ADD(15) | MX_ADD(20) | MX_ADD(25))
#define TIANDY_OEM_SUPPORTED_50FPS  (TIANDY_OEM_SUPPORTED_25FPS | MX_ADD(50))

#define MATRIX_IP_SUPPORTED_05FPS   AddRange(1, 5)
#define MATRIX_IP_SUPPORTED_08FPS   AddRange(1, 8)
#define MATRIX_IP_SUPPORTED_10FPS   AddRange(1, 10)
#define MATRIX_IP_SUPPORTED_15FPS   AddRange(1, 15)
#define MATRIX_IP_SUPPORTED_25FPS   AddRange(1, 25)
#define MATRIX_IP_SUPPORTED_30FPS   AddRange(1, 30)
#define MATRIX_IP_SUPPORTED_60FPS   AddRange(1, 60)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    MATRIX_GROUP_01 = 0,
    MATRIX_GROUP_02,
    MATRIX_GROUP_03,
    MATRIX_GROUP_04,
    MATRIX_GROUP_05,
    MATRIX_GROUP_06,
    MATRIX_GROUP_07,
    MATRIX_GROUP_08,
    MATRIX_GROUP_09,
    MATRIX_GROUP_10,
    MATRIX_GROUP_11,
    MATRIX_GROUP_12,
    MATRIX_GROUP_13,
    MATRIX_GROUP_14,
    MATRIX_GROUP_15,
    MATRIX_GROUP_16,
    MATRIX_GROUP_17,
    MATRIX_GROUP_18,
    MATRIX_GROUP_19,
    MATRIX_GROUP_20,
    MATRIX_GROUP_21,
    MATRIX_GROUP_22,
    MATRIX_GROUP_23,
    MATRIX_GROUP_24,
    MATRIX_GROUP_25,
    MATRIX_GROUP_26,
    MATRIX_GROUP_27,
    MATRIX_GROUP_28,
    MATRIX_GROUP_29,
    MATRIX_GROUP_30,
    MATRIX_GROUP_MAX
}MATRIX_GROUP_e;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
/* array for storing old camera names */
static const OLD_MATRIX_CAMERA_MODEL_NAME_t oldMatrixCameraModelName[] =
{                                                                       /* enum no */
    {MATRIX_MODEL_SATATYA_PZCR20ML_25CWP,   "SATATYA PZCR20ML25CWP"},   /* 1 */
    {MATRIX_MODEL_SATATYA_PZCR20ML_33CWP,   "SATATYA PZCR20ML33CWP"},   /* 2 */
    {MATRIX_MODEL_SATATYA_CIBR13FL_40CW ,   "SATATYA CIBR13FL40CW"},    /* 3 */
    {MATRIX_MODEL_SATATYA_CIDR13FL_40CW ,   "SATATYA CIDR13FL40CW"},    /* 4 */
    {MATRIX_MODEL_SATATYA_CIDRP20VL_130CW,  "SATATYA CIDRP20VL130CW"},  /* 5 */
    {MATRIX_MODEL_SATATYA_CIBR30FL_36CG,    "SATATYA CIBR30FL36CG"},    /* 6 */
    {MATRIX_MODEL_SATATYA_CIBR30FL_60CG,    "SATATYA CIBR30FL60CG"},    /* 7 */
    {MATRIX_MODEL_SATATYA_CIDR30FL_36CW,    "SATATYA CIDR30FL36CW"},    /* 8 */
    {MATRIX_MODEL_SATATYA_CIDR30FL_60CW,    "SATATYA CIDR30FL60CW"},    /* 9 */
    {MATRIX_MODEL_SATATYA_CIDR20FL_36CW_S,  "CIDR20FL36CW-S"},          /* 10 */
    {MATRIX_MODEL_SATATYA_CIDR20FL_60CW_S,  "CIDR20FL60CW-S"},          /* 11 */
    {MATRIX_MODEL_SATATYA_CIDR20VL_12CW_S,  "CIDR20VL12CW-S"},          /* 12 */
    {MATRIX_MODEL_SATATYA_CIBR20FL_36CW_S,  "CIBR20FL36CW-S"},          /* 13 */
    {MATRIX_MODEL_SATATYA_CIBR20FL_60CW_S,  "CIBR20FL60CW-S"},          /* 14 */
    {MATRIX_MODEL_SATATYA_CIBR20VL_12CW_S,  "CIBR20VL12CW-S"},          /* 15 */
    {MATRIX_MODEL_SATATYA_CIDR30FL_36CW_S,  "CIDR30FL36CW-S"},          /* 16 */
    {MATRIX_MODEL_SATATYA_CIDR30FL_60CW_S,  "CIDR30FL60CW-S"},          /* 17 */
    {MATRIX_MODEL_SATATYA_CIDR30VL_12CW_S,  "CIDR30VL12CW-S"},          /* 18 */
    {MATRIX_MODEL_SATATYA_CIBR30FL_36CW_S,  "CIBR30FL36CW-S"},          /* 19 */
    {MATRIX_MODEL_SATATYA_CIBR30FL_60CW_S,  "CIBR30FL60CW-S"},          /* 20 */
    {MATRIX_MODEL_SATATYA_CIBR30VL_12CW_S,  "CIBR30VL12CW-S"},          /* 21 */
    {MATRIX_MODEL_SATATYA_CIDR20VL_12CW_P,  "CIDR20VL12CW-P"},          /* 22 */
    {MATRIX_MODEL_SATATYA_CIBR20VL_12CW_P,  "CIBR20VL12CW-P"},          /* 23 */
    {MATRIX_MODEL_SATATYA_CIDR30VL_12CW_P,  "CIDR30VL12CW-P"},          /* 24 */
    {MATRIX_MODEL_SATATYA_CIBR30VL_12CW_P,  "CIBR30VL12CW-P"},          /* 25 */
    {MATRIX_MODEL_SATATYA_CIDR20FL_28CW_S,  "CIDR20FL28CW-S"},          /* 26 */
    {MATRIX_MODEL_SATATYA_CIBR20FL_28CW_S,  "CIBR20FL28CW-S"},          /* 27 */
    {MATRIX_MODEL_SATATYA_CIDR30FL_28CW_S,  "CIDR30FL28CW-S"},          /* 28 */
    {MATRIX_MODEL_SATATYA_CIBR30FL_28CW_S,  "CIBR30FL28CW-S"},          /* 29 */
    {MATRIX_MODEL_SATATYA_CIDR50VL_12CW_P,  "CIDR50VL12CWP"},           /* 77 */
    {MAX_MATRIX_CAMERA_MODEL,               ""}
};

//*************************************************************************************************
// PROFILE WISE SUPPORTED RESOLUTION LIST
//*************************************************************************************************
static const UINT8 profileResolution_01[] =
{
    RESOLUTION_320x240_QVGA,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x576_2CIFEXP,
    MAX_RESOLUTION
};

static const UINT8 profileResolution_02[] =
{
    RESOLUTION_320x240_QVGA,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1280x960,
    MAX_RESOLUTION
};

static const UINT8 profileResolution_03[] =
{
    RESOLUTION_320x240_QVGA,
    RESOLUTION_352x288_CIF,
    RESOLUTION_704x576_2CIFEXP,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_04[] =
{
    RESOLUTION_320x240_QVGA,
    RESOLUTION_352x288_CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_05[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    RESOLUTION_2048x1536,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_06[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    RESOLUTION_2048x1536,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_07[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_08[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_09[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_10[] =
{
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_11[] =
{
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x360,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_12[] =
{
    RESOLUTION_352x288_CIF,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_13[] =
{
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_14[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    RESOLUTION_2048x1536,
    RESOLUTION_2592x1520,
    RESOLUTION_2592x1944,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_15[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_16[] =
{
    RESOLUTION_176x144_QCIF,
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    RESOLUTION_2048x1536,
    RESOLUTION_2592x1520,
    RESOLUTION_2592x1944,
    RESOLUTION_3200x1800,
    RESOLUTION_3840x2160,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_17[] =
{
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x576_2CIFEXP,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_18[] =
{
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    RESOLUTION_1920x1080,
    RESOLUTION_2048x1536,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_19[] =
{
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_20[] =
{
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    RESOLUTION_704x288_2CIF,
    RESOLUTION_704x576_2CIFEXP,
    RESOLUTION_1280x720,
    MAX_RESOLUTION,
};

static const UINT8 profileResolution_21[] =
{
    RESOLUTION_352x288_CIF,
    RESOLUTION_640x480_VGA,
    MAX_RESOLUTION,
};

//*************************************************************************************************
// RESOLUTION LIST OF ALL PROFILES
//*************************************************************************************************
static UINT8 const *profileGroup_Resolution_01[] =
{
    profileResolution_01,
    profileResolution_02,
    NULL,
};

static UINT8 const *profileGroup_Resolution_02[] =
{
    profileResolution_03,
    profileResolution_04,
    NULL,
};

static UINT8 const *profileGroup_Resolution_03[] =
{
    profileResolution_06,
    profileResolution_07,
    profileResolution_07,
    profileResolution_08,
};

static UINT8 const *profileGroup_Resolution_04[] =
{
    profileResolution_09,
    profileResolution_07,
    profileResolution_09,
    profileResolution_08,
};

static UINT8 const *profileGroup_Resolution_05[] =
{
    profileResolution_06,
    profileResolution_07,
    profileResolution_06,
    profileResolution_08,
};

static UINT8 const *profileGroup_Resolution_06[] =
{
    profileResolution_10,
    profileResolution_11,
    profileResolution_12
};

static UINT8 const *profileGroup_Resolution_07[] =
{
    profileResolution_10,
    profileResolution_11,
    profileResolution_13
};

static UINT8 const *profileGroup_Resolution_08[] =
{
    profileResolution_14,
    profileResolution_15,
    profileResolution_06,
};

static UINT8 const *profileGroup_Resolution_09[] =
{
    profileResolution_09,
    profileResolution_19,
    profileResolution_13,
    profileResolution_20,
};

static UINT8 const *profileGroup_Resolution_10[] =
{
    profileResolution_14,
    profileResolution_17,
    profileResolution_18,
    profileResolution_10,
};

//*************************************************************************************************
// CODEC DEPENDENT PROFILE WISE RESOLUTION LIST
//*************************************************************************************************
static UINT8 const *codecResolution_01[] =
{
    NULL,
    profileResolution_16,
    NULL,
    NULL,
    profileResolution_16
};

static UINT8 const *codecResolution_02[] =
{
    NULL,
    profileResolution_17,
    NULL,
    NULL,
    profileResolution_17
};

static UINT8 const *codecResolution_03[] =
{
    profileResolution_18,
    profileResolution_17,
    NULL,
    NULL,
    profileResolution_17
};

static UINT8 const *codecResolution_04[] =
{
    profileResolution_10,
    profileResolution_10,
    NULL,
    NULL,
    profileResolution_10
};

static UINT8 const *codecResolution_05[] =
{
    NULL,
    profileResolution_09,
    NULL,
    NULL,
    profileResolution_09
};

static UINT8 const *codecResolution_06[] =
{
    profileResolution_10,
    profileResolution_17,
    NULL,
    NULL,
    profileResolution_17
};

static UINT8 const *codecResolution_07[] =
{
    NULL,
    profileResolution_14,
    NULL,
    NULL,
    profileResolution_14
};

static UINT8 const *codecResolution_08[] =
{
    NULL,
    profileResolution_19,
    NULL,
    NULL,
    profileResolution_19
};

static UINT8 const *codecResolution_09[] =
{
    profileResolution_13,
    profileResolution_21,
    NULL,
    NULL,
    profileResolution_21
};

static UINT8 const *codecResolution_10[] =
{
    profileResolution_08,
    profileResolution_08,
    NULL,
    NULL,
    profileResolution_08
};

static UINT8 const *codecResolution_11[] =
{
    NULL,
    profileResolution_13,
    NULL,
    NULL,
    profileResolution_13
};

//*************************************************************************************************
// CODEC DEPENDENT RESOLUTION LIST OF PROFILES
//*************************************************************************************************
static UINT8 const **profileCodecGroup_Resolution_01[] =
{
    codecResolution_01,
    codecResolution_02,
    codecResolution_03,
    codecResolution_04
};

static UINT8 const **profileCodecGroup_Resolution_02[] =
{
    codecResolution_05,
    codecResolution_02,
    codecResolution_06,
    codecResolution_04
};

static UINT8 const **profileCodecGroup_Resolution_03[] =
{
    codecResolution_07,
    codecResolution_02,
    codecResolution_06,
    codecResolution_04
};

static UINT8 const **profileCodecGroup_Resolution_04[] =
{
    codecResolution_07,
    codecResolution_02,
    codecResolution_03,
    codecResolution_04
};

static UINT8 const **profileCodecGroup_Resolution_05[] =
{
    codecResolution_11,
    codecResolution_08,
    codecResolution_09,
    codecResolution_10
};

//*************************************************************************************************
// ALL PROFILE FPS LIST
//*************************************************************************************************
static const UINT64 profileGroup_Fps_01 = MATRIX_OEM_SUPPORTED_FPS;

static const UINT64 profileGroup_Fps_02[] =
{
    TIANDY_OEM_SUPPORTED_25FPS,
    TIANDY_OEM_SUPPORTED_25FPS,
    TIANDY_OEM_SUPPORTED_25FPS
};

static const UINT64 profileGroup_Fps_03[] =
{
    TIANDY_OEM_SUPPORTED_50FPS,
    TIANDY_OEM_SUPPORTED_25FPS,
    TIANDY_OEM_SUPPORTED_25FPS
};

static const UINT64 profileGroup_Fps_04 = MATRIX_OEM_SUPPORTED_FPS;

static const UINT64 profileGroup_Fps_05 = MATRIX_IP_SUPPORTED_25FPS;

static const UINT64 profileGroup_Fps_06[] =
{
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_05FPS,
    MATRIX_IP_SUPPORTED_30FPS,
};

static const UINT64 profileGroup_Fps_07[] =
{
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_08FPS,
};

static const UINT64 profileGroup_Fps_08[] =
{
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_08FPS,
    MATRIX_IP_SUPPORTED_30FPS,
};

static const UINT64 profileGroup_Fps_09[] =
{
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_10FPS,
    MATRIX_IP_SUPPORTED_15FPS
};

static const UINT64 profileGroup_Fps_10[] =
{
    MATRIX_IP_SUPPORTED_60FPS,
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_08FPS,
    MATRIX_IP_SUPPORTED_30FPS,
};

//*************************************************************************************************
// CODEC DEPENDENT PROFILE WISE FPS LIST
//*************************************************************************************************
static const UINT64 codecFps_01[] =
{
    0,
    MATRIX_IP_SUPPORTED_30FPS,
    0,
    0,
    MATRIX_IP_SUPPORTED_30FPS
};

static const UINT64 codecFps_02[] =
{
    MATRIX_IP_SUPPORTED_05FPS,
    MATRIX_IP_SUPPORTED_15FPS,
    0,
    0,
    MATRIX_IP_SUPPORTED_15FPS
};

static const UINT64 codecFps_03[] =
{
    MATRIX_IP_SUPPORTED_30FPS,
    MATRIX_IP_SUPPORTED_30FPS,
    0,
    0,
    MATRIX_IP_SUPPORTED_30FPS
};

static const UINT64 codecFps_04[] =
{
    0,
    MATRIX_IP_SUPPORTED_60FPS,
    0,
    0,
    MATRIX_IP_SUPPORTED_60FPS
};

static const UINT64 codecFps_05[] =
{
    MATRIX_IP_SUPPORTED_08FPS,
    MATRIX_IP_SUPPORTED_15FPS,
    0,
    0,
    MATRIX_IP_SUPPORTED_15FPS
};

//*************************************************************************************************
// CODEC DEPENDENT FPS LIST OF PROFILES
//*************************************************************************************************
static const UINT64 *profileCodecGroup_Fps_01[] =
{
    codecFps_01,
    codecFps_01,
    codecFps_02,
    codecFps_03
};

static const UINT64 *profileCodecGroup_Fps_02[] =
{
    codecFps_04,
    codecFps_01,
    codecFps_05,
    codecFps_03
};

//*************************************************************************************************
// ALL PROFILE CODEC LIST
//*************************************************************************************************
static UINT8 profileGroup_Codec_01[] =
{
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_MJPG)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
};

static UINT8 profileGroup_Codec_02[] =
{
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_MJPG)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)),
};

static UINT8 profileGroup_Codec_03[] =
{
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
};

static UINT8 profileGroup_Codec_04[] =
{
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_MJPG)),
};

static UINT8 profileGroup_Codec_05[] =
{
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
    (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG))
};

static const CAMERA_MODEL_PARAMETER_t matrixModelParameter[MATRIX_GROUP_MAX] =
{
    //********************************** MATRIX_GROUP_01 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)),
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_01,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)&profileGroup_Fps_01,
        },
        .qualitySupported =
        {
            10, //VIDEO_MJPG
            10, //VIDEO_H264
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_02 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)),
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_02,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)&profileGroup_Fps_04,
        },
        .qualitySupported =
        {
            10, //VIDEO_MJPG
            10, //VIDEO_H264
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(PTZ_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT2_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_03 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)),
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileResolution_05,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)&profileGroup_Fps_05,
        },
        .qualitySupported =
        {
            4,  //VIDEO_MJPG
            4,  //VIDEO_H264
            0,
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_04 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_03,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,  //VIDEO_MJPG
            4,  //VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_05 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_04,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,  //VIDEO_MJPG
            4,  //VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_06 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_05,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(MISSING_OBJECT_SUPPORT)
                                        | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT) | MX_ADD(AUDIO_EXCEPTION_SUPPORT)
                                        | MX_ADD(FOCUS_SUPPORT)| MX_ADD(PTZ_SUPPORT)
                                        | MX_ADD(OBJECT_COUNTING_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_07 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_04,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,  //VIDEO_MJPG
            4,  //VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(MISSING_OBJECT_SUPPORT)
                                        | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT)| MX_ADD(AUDIO_EXCEPTION_SUPPORT)
                                        | MX_ADD(FOCUS_SUPPORT) | MX_ADD(PTZ_SUPPORT)
                                        | MX_ADD(OBJECT_COUNTING_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_08 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_02,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_04,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_09 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_02,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_04,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_10 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_04,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_11 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_05,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_12 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_05,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT)
                                        | MX_ADD(AUDIO_MIC_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_13 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_05,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_14 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_03,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_06,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)&profileGroup_Fps_02,
        },
        .qualitySupported =
        {
            5,	//VIDEO_MJPG
            5,	//VIDEO_H264
            0,
            0,
            5,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(AUDIO_MIC_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(MISSING_OBJECT_SUPPORT) | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT) | MX_ADD(PTZ_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_OUTPUT2_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_15 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_03,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_07,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)&profileGroup_Fps_03,
        },
        .qualitySupported =
        {
            5,	//VIDEO_MJPG
            5,	//VIDEO_H264
            0,
            0,
            5,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(AUDIO_MIC_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(MISSING_OBJECT_SUPPORT) | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT) | MX_ADD(PTZ_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_OUTPUT2_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_16 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_04,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_08,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_07,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_17 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_04,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_08,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_07,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_18 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_04,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_08,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_07,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(MISSING_OBJECT_SUPPORT)
                                        | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT) | MX_ADD(AUDIO_EXCEPTION_SUPPORT)
                                        | MX_ADD(FOCUS_SUPPORT)| MX_ADD(PTZ_SUPPORT) | MX_ADD(OBJECT_COUNTING_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_19 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_04,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_08,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_07,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_20 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_10,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_08,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT)),
    },


    //********************************** MATRIX_GROUP_21 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_01,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_09,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(LINE_CROSSING_SUPPORT))
    },

    //********************************** MATRIX_GROUP_22 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_01,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_09,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT))
    },

    //********************************** MATRIX_GROUP_23 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_01,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_09,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(AUDIO_EXCEPTION_SUPPORT))
    },

    //********************************** MATRIX_GROUP_24 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_H265)|MX_ADD(VIDEO_MJPG)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_01,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_09,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(MISSING_OBJECT_SUPPORT)
                                        | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT) | MX_ADD(AUDIO_EXCEPTION_SUPPORT)
                                        | MX_ADD(FOCUS_SUPPORT) | MX_ADD(PTZ_SUPPORT) | MX_ADD(OBJECT_COUNTING_SUPPORT))
    },

    //********************************** MATRIX_GROUP_25 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_01,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= NO,
            .resolutionAvailable		= (UINT8PTR)profileGroup_Resolution_09,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_06,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT)| MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)),
    },

    //********************************** MATRIX_GROUP_26 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_02,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_10,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(AUDIO_EXCEPTION_SUPPORT))
    },

    //********************************** MATRIX_GROUP_27 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_03,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_08,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT)
                                        | MX_ADD(AUDIO_SPEAKER_SUPPORT) | MX_ADD(AUDIO_EXCEPTION_SUPPORT))
    },

    //********************************** MATRIX_GROUP_28 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_05,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= YES,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileCodecGroup_Fps_01,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT))
    },

    //********************************** MATRIX_GROUP_29 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_04,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= NO,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileGroup_Fps_08,
        },
        .qualitySupported =
        {
            4,	//VIDEO_MJPG
            4,	//VIDEO_H264
            0,
            0,
            4,	//VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(NO_MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(OBJECT_INTRUSION_SUPPORT))
    },

    //********************************** MATRIX_GROUP_30 **********************************
    {
        .codecSupported	= (UINT8)(MX_ADD(VIDEO_H264)|MX_ADD(VIDEO_MJPG)|MX_ADD(VIDEO_H265)),
        .profileWiseCodecSupported = profileGroup_Codec_05,
        .isParamProfileDependent = YES,
        .resolutionSupported =
        {
            .isResolutionCodecDependent	= YES,
            .resolutionAvailable		= (UINT8PTR)profileCodecGroup_Resolution_02,
        },
        .framerateSupported	=
        {
            .isFramerateCodecDependent		= YES,
            .isFramerateResolutionDependent	= NO,
            .framerateAvailable				= (UINT64PTR)profileCodecGroup_Fps_02,
        },
        .qualitySupported =
        {
            4,  //VIDEO_MJPG
            4,  //VIDEO_H264
            0,
            0,
            4,  //VIDEO_H265
        },
        .capabilitySupported = (UINT32)(MX_ADD(MOTION_DETECTION_SUPPORT) | MX_ADD(TAMPER_DETECTION_SUPPORT)
                                        | MX_ADD(LINE_CROSSING_SUPPORT) | MX_ADD(AUDIO_MIC_SUPPORT)
                                        | MX_ADD(OBJECT_INTRUSION_SUPPORT) | MX_ADD(AUDIO_SPEAKER_SUPPORT)
                                        | MX_ADD(MISSING_OBJECT_SUPPORT) | MX_ADD(SUSPICIOUS_OBJECT_SUPPORT)
                                        | MX_ADD(OBJECT_COUNTING_SUPPORT) | MX_ADD(NO_MOTION_DETECTION_SUPPORT)
                                        | MX_ADD(LOITERING_SUPPORT) | MX_ADD(AUDIO_EXCEPTION_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_INPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_INPUT2_SUPPORT)
                                        | MX_ADD(CAMERA_ALARM_OUTPUT1_SUPPORT) | MX_ADD(CAMERA_ALARM_OUTPUT2_SUPPORT)
                                        | MX_ADD(PTZ_SUPPORT) | MX_ADD(FOCUS_SUPPORT) | MX_ADD(IRIS_SUPPORT)),
    }
};

const CAMERA_MODEL_INFO_t MatrixModelInfo[MAX_MATRIX_CAMERA_MODEL] =
{
    //MATRIX_MODEL_NONE
    {
        .modelName = "",
        .modelParameter = NULL
    },

    // MATRIX_MODEL_SATATYA_PZCR20ML_25CWP
    {
        .modelName = "PZCR20ML25CWP",           //SATATYA PZCR20ML25CWP
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_14],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_CONTINOUS_MOVE,	// pan
                PTZ_CONTINOUS_MOVE, // tilt
                PTZ_CONTINOUS_MOVE, // zoom
                PTZ_CONTINOUS_MOVE, // focus
                PTZ_CONTINOUS_MOVE, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_PZCR20ML_33CWP
    {
        .modelName = "PZCR20ML33CWP",         //SATATYA PZCR20ML33CWP
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_15],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_CONTINOUS_MOVE,	// pan
                PTZ_CONTINOUS_MOVE, // tilt
                PTZ_CONTINOUS_MOVE, // zoom
                PTZ_CONTINOUS_MOVE, // focus
                PTZ_CONTINOUS_MOVE, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR13FL_40CW
    {
        .modelName = "CIBR13FL40CW",   //SATATYA CIBR13FL40CW
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_01],
        .profileParam =
        {
                .maxSupportedProfile = 2,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR13FL_40CW
    {
        .modelName = "CIDR13FL40CW",   //SATATYA CIDR13FL40CW
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_01],
        .profileParam =
        {
                .maxSupportedProfile = 2,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDRP20VL_130CW
    {
        .modelName = "CIDRP20VL130CW",   //SATATYA CIDRP20VL130CW
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_02],
        .profileParam =
        {
            .maxSupportedProfile = 2,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_CONTINOUS_MOVE,	// pan
                PTZ_CONTINOUS_MOVE, // tilt
                PTZ_CONTINOUS_MOVE, // zoom
                PTZ_CONTINOUS_MOVE, // focus
                PTZ_CONTINOUS_MOVE, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR30FL_36CG
    {
        .modelName = "CIBR30FL36CG",   //SATATYA CIBR30FL36CG
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_03],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR30FL_60CG
    {
        .modelName = "CIBR30FL60CG",   //SATATYA CIBR30FL60CG
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_03],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR30FL_36CW
    {
        .modelName = "CIDR30FL36CW",   //SATATYA CIDR30FL36CW
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_03],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR30FL_60CW
    {
        .modelName = "CIDR30FL60CW",   //SATATYA CIDR30FL60CW
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_03],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR20FL_36CW_S
    {
        .modelName = "CIDR20FL36CWS",   //CIDR20FL36CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR20FL_60CW_S
    {
        .modelName = "CIDR20FL60CWS",   //CIDR20FL60CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR20VL_12CW_S
    {
        .modelName = "CIDR20VL12CWS",   //CIDR20VL12CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR20FL_36CW_S
    {
        .modelName = "CIBR20FL36CWS",   //CIBR20FL36CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR20FL_60CW_S
    {
        .modelName = "CIBR20FL60CWS",   //CIBR20FL60CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR20VL_12CW_S
    {
        .modelName = "CIBR20VL12CWS",   //CIBR20VL12CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR30FL_36CW_S
    {
        .modelName = "CIDR30FL36CWS",   //CIDR30FL36CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR30FL_60CW_S
    {
        .modelName = "CIDR30FL60CWS",   //CIDR30FL60CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR30VL_12CW_S
    {
        .modelName = "CIDR30VL12CWS",   //CIDR30VL12CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR30FL_36CW_S
    {
        .modelName = "CIBR30FL36CWS",   //CIBR30FL36CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR30FL_60CW_S
    {
        .modelName = "CIBR30FL60CWS",   //CIBR30FL60CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR30VL_12CW_S
    {
        .modelName = "CIBR30VL12CWS",   //CIBR30VL12CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR20VL_12CW_P,
    {
        .modelName = "CIDR20MVL12CWP",  //CIDR20VL12CW-P
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_07],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR20VL_12CW_P,
    {
        .modelName = "CIBR20MVL12CWP",  //CIBR20VL12CW-P
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_07],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR30VL_12CW_P,
    {
        .modelName = "CIDR30MVL12CWP",  //CIDR30VL12CW-P
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_06],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR30VL_12CW_P,
    {
        .modelName = "CIBR30MVL12CWP",  //CIBR30VL12CW-P
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_06],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR20FL_28CW_S
    {
        .modelName = "CIDR20FL28CWS",   //CIDR20FL28CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR20FL_28CW_S
    {
        .modelName = "CIBR20FL28CWS",   //CIBR20FL28CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_05],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR30FL_28CW_S
    {
        .modelName = "CIDR30FL28CWS",   //CIDR30FL28CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR30FL_28CW_S
    {
        .modelName = "CIBR30FL28CWS",   //CIBR30FL28CW-S
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_MIDR20FL_28CW_S
    {
        .modelName = "MIDR20FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_08],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_MIDR20FL_36CW_S
    {
        .modelName = "MIDR20FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_08],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_MIDR20FL_60CW_S
    {
        .modelName = "MIDR20FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_08],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR20FL_28CW_P,
    {
        .modelName = "MIDR20FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_09],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR20FL_36CW_P,
    {
        .modelName = "MIDR20FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_09],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR20FL_60CW_P,
    {
        .modelName = "MIDR20FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_09],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR20FL_36CW_P,
    {
        .modelName = "CIDR20FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_10],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR20FL_36CW_P,
    {
        .modelName = "CIBR20FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_10],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR20FL_60CW_P,
    {
        .modelName = "CIDR20FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_10],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR20FL_60CW_P,
    {
        .modelName = "CIBR20FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_10],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR30FL_36CW_P,
    {
        .modelName = "CIDR30FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_13],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR30FL_36CW_P,
    {
        .modelName = "CIBR30FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_13],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR30FL_60CW_P,
    {
        .modelName = "CIDR30FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_13],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR30FL_60CW_P,
    {
        .modelName = "CIBR30FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_13],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR30FL_28CW_S,
    {
        .modelName = "MIDR30FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR30FL_36CW_S,
    {
        .modelName = "MIDR30FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR30FL_60CW_S,
    {
        .modelName = "MIDR30FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR30FL_28CW_S,
    {
        .modelName = "MIBR30FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR30FL_36CW_S,
    {
        .modelName = "MIBR30FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR30FL_60CW_S,
    {
        .modelName = "MIBR30FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_11],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR30FL_28CW_P,
    {
        .modelName = "MIDR30FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_12],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR30FL_36CW_P,
    {
        .modelName = "MIDR30FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_12],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR30FL_60CW_P,
    {
        .modelName = "MIDR30FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_12],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR30FL_28CW_P,
    {
        .modelName = "MIBR30FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_12],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR30FL_36CW_P,
    {
        .modelName = "MIBR30FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_12],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR30FL_60CW_P,
    {
        .modelName = "MIBR30FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_12],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR20FL_28CW_S,
    {
        .modelName = "MIBR20FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_08],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR20FL_36CW_S,
    {
        .modelName = "MIBR20FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_08],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR20FL_60CW_S,
    {
        .modelName = "MIBR20FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_08],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR20FL_28CW_P,
    {
        .modelName = "MIBR20FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_09],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR20FL_36CW_P,
    {
        .modelName = "MIBR20FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_09],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR20FL_60CW_P,
    {
        .modelName = "MIBR20FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_09],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR50FL_28CW_S
    {
        .modelName = "CIBR50FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR50FL_40CW_S
    {
        .modelName = "CIBR50FL40CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR50FL_60CW_S
    {
        .modelName = "CIBR50FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL SATATYA CIDR50FL28CW
    {
        .modelName = "CIDR50FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL SATATYA CIDR50FL40CW
    {
        .modelName = "CIDR50FL40CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL SATATYA CIDR50FL60CW
    {
        .modelName = "CIDR50FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIBR50VL_12CW_S
    {
        .modelName = "CIBR50VL12CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    // MATRIX_MODEL_SATATYA_CIDR50VL_12CW_S
    {
        .modelName = "CIDR50VL12CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR50FL_28CW_P,
    {
        .modelName = "CIBR50FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_19],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR50FL_40CW_P,
    {
        .modelName = "CIBR50FL40CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_19],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR50FL_60CW_P,
    {
        .modelName = "CIBR50FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_19],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR50FL_28CW_P,
    {
        .modelName = "CIDR50FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_19],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR50FL_40CW_P,
    {
        .modelName = "CIDR50FL40CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_19],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR50FL_60CW_P,
    {
        .modelName = "CIDR50FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_19],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR50VL_12CW_P,
    {
        .modelName = "CIBR50MVL12CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_18],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR50VL_12CW_P,
    {
        .modelName = "CIDR50MVL12CWP",   //CIDR50VL12CWP
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_18],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR50FL_28CW_S,
    {
        .modelName = "MIBR50FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR50FL_40CW_S,
    {
        .modelName = "MIBR50FL40CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR50FL_60CW_S,
    {
        .modelName = "MIBR50FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR50FL_28CW_S,
    {
        .modelName = "MIDR50FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR50FL_40CW_S,
    {
        .modelName = "MIDR50FL40CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR50FL_60CW_S,
    {
        .modelName = "MIDR50FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_16],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR50FL_28CW_P,
    {
        .modelName = "MIBR50FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_17],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR50FL_40CW_P,
    {
        .modelName = "MIBR50FL40CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_17],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIBR50FL_60CW_P,
    {
        .modelName = "MIBR50FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_17],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR50FL_28CW_P,
    {
        .modelName = "MIDR50FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_17],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR50FL_40CW_P,
    {
        .modelName = "MIDR50FL40CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_17],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MIDR50FL_60CW_P,
    {
        .modelName = "MIDR50FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_17],
        .profileParam =
        {
            .maxSupportedProfile = 3,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR20FL_28CW_P,
    {
        .modelName = "CIDR20FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_10],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR20FL_28CW_P,
    {
        .modelName = "CIBR20FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_10],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIDR30FL_28CW_P,
    {
        .modelName = "CIDR30FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_13],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR30FL_28CW_P,
    {
        .modelName = "CIBR30FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_13],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },
        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_CIBR80FL_28CW_S,
    {
        .modelName = "CIBR80FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIBR80FL_36CW_S,
    {
        .modelName = "CIBR80FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIBR80FL_60CW_S
    {
        .modelName = "CIBR80FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80FL_28CW_S
    {
        .modelName = "CIDR80FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80FL_36CW_S
    {
        .modelName = "CIDR80FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80FL_60CW_S
    {
        .modelName = "CIDR80FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIBR80FL_28CW_P
    {
        .modelName = "CIBR80FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_22],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIBR80FL_36CW_P
    {
        .modelName = "CIBR80FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_22],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIBR80FL_60CW_P
    {
        .modelName = "CIBR80FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_22],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIBR80ML_12CW_P
    {
        .modelName = "CIBR80ML12CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_24],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80FL_28CW_P
    {
        .modelName = "CIDR80FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_22],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80FL_36CW_P
    {
        .modelName = "CIDR80FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_22],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80FL_60CW_P
    {
        .modelName = "CIDR80FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_22],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_CIDR80ML_12CW_P
    {
        .modelName = "CIDR80ML12CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_24],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_CONTINOUS_MOVE,     // zoom
                PTZ_CONTINOUS_MOVE,     // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIBR80FL_28CW_S
    {
        .modelName = "MIBR80FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIBR80FL_36CW_S
    {
        .modelName = "MIBR80FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIBR80FL_60CW_S
    {
        .modelName = "MIBR80FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIDR80FL_28CW_S
    {
        .modelName = "MIDR80FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIDR80FL_36CW_S
    {
        .modelName = "MIDR80FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIDR80FL_60CW_S
    {
        .modelName = "MIDR80FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_21],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIBR80FL_28CW_P
    {
        .modelName = "MIBR80FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_23],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIBR80FL_36CW_P
    {
        .modelName = "MIBR80FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_23],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIBR80FL_60CW_P
    {
        .modelName = "MIBR80FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_23],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIDR80FL_28CW_P
    {
        .modelName = "MIDR80FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_23],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIDR80FL_36CW_P
    {
        .modelName = "MIDR80FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_23],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MIDR80FL_60CW_P
    {
        .modelName = "MIDR80FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_23],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MITR20FL_28CW_S
    {
        .modelName = "MITR20FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_25],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MITR20FL_36CW_S
    {
        .modelName = "MITR20FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_25],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MITR20FL_60CW_S
    {
        .modelName = "MITR20FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_25],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MITR50FL_28CW_S
    {
        .modelName = "MITR50FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_20],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MITR50FL_40CW_S
    {
        .modelName = "MITR50FL40CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_20],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MITR50FL_60CW_S
    {
        .modelName = "MITR50FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_20],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_RIDR20FL_28CW_P
    {
        .modelName = "RIDR20FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_26],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_RIDR20FL_36CW_P
    {
        .modelName = "RIDR20FL36CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_26],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_RIDR20FL_60CW_P
    {
        .modelName = "RIDR20FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_26],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_RIDR50FL_28CW_P
    {
        .modelName = "RIDR50FL28CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_27],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_RIDR50FL_40CW_P
    {
        .modelName = "RIDR50FL40CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_27],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_RIDR50FL_60CW_P
    {
        .modelName = "RIDR50FL60CWP",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_27],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MITC20FL_28CW_S
    {
        .modelName = "MITC20FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_28],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MITC20FL_36CW_S
    {
        .modelName = "MITC20FL36CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_28],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MITC20FL_60CW_S
    {
        .modelName = "MITC20FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_28],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 3,
    },

    //MATRIX_MODEL_SATATYA_MITC50FL_28CW_S
    {
        .modelName = "MITC50FL28CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_29],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MITC50FL_40CW_S
    {
        .modelName = "MITC50FL40CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_29],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_MITC50FL_60CW_S
    {
        .modelName = "MITC50FL60CWS",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_29],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_MOVE_NOT_SUPPORTED,	// pan
                PTZ_MOVE_NOT_SUPPORTED, // tilt
                PTZ_MOVE_NOT_SUPPORTED, // zoom
                PTZ_MOVE_NOT_SUPPORTED, // focus
                PTZ_MOVE_NOT_SUPPORTED, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },

    //MATRIX_MODEL_SATATYA_PTZ_2040_P
    {
        .modelName = "PTZ2040P",
        .modelParameter = (CAMERA_MODEL_PARAMETER_t *)&matrixModelParameter[MATRIX_GROUP_30],
        .profileParam =
        {
            .maxSupportedProfile = 4,
        },
        .ptzFeatureType =
        {
            .moveType =
            {
                PTZ_CONTINOUS_MOVE, // pan
                PTZ_CONTINOUS_MOVE, // tilt
                PTZ_CONTINOUS_MOVE, // zoom
                PTZ_CONTINOUS_MOVE, // focus
                PTZ_CONTINOUS_MOVE, // iris
            }
        },

        .motionWindowSettingMethod = MOTION_AREA_METHOD_BLOCK,
        .privacyMaskWindowSettingMethod = PRIVACY_MASK_METHOD_POINT,
        .maxSupportedPrivacyMaskWindow = 8,
    },
};

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will update matrix camera model name with new name if camera model found in the old list
 * @param   pCameraModel
 * @param   modelLenMax
 */
void GetUpdatedMatrixCameraModelName(CHAR *pCameraModel, UINT8 modelLenMax)
{
    UINT8 index;

    /* Replacing camera name with new name if found in old camera model list */
    for (index = 0; oldMatrixCameraModelName[index].cameraModel != MAX_MATRIX_CAMERA_MODEL; index++)
    {
        if (strcasecmp(pCameraModel, oldMatrixCameraModelName[index].modelName) == 0)
        {
            snprintf(pCameraModel, modelLenMax, "%s", MatrixModelInfo[oldMatrixCameraModelName[index].cameraModel].modelName);
            break;
        }
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
