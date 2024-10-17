//#################################################################################################
// ABOUT HiMPP (HiSilicon Media Processing Platform)
//#################################################################################################
/*
 -------------------------------------------------
 * HiMPP modules
 -------------------------------------------------
 * Video input unit (VIU): captures video pictures, crops, scales, and mirrors the pictures
   and outputs pictures with different resolutions

 * Video encoding (VDEC) module: decodes the encoded video streams and transfers the decoded streams to the VPSS or VOU.
   It can decode H.264/H.265, VC1, MPEG4, MPEG2, or AVS formats.

 * Video processing subsystem (VPSS): receives the pictures from the VIU or VDEC module and performs denoising,
   image enhancement, and sharpening on the pictures.  The VPSS can output the pictures from the same source
   as multi-channel pictures with different resolutions for encoding, previewing, or snapshot.

 * Video encoding (VENC) module: receives the pictures that are captured by the VIU and processed by the VPSS,
   and overlays the OSD pictures configured by using the REGION module, and encodes the pictures and output
   streams complying with different protocols.

 * Video output unit (VOU): receives the pictures processed by the VPSS, performs play control, and
   outputs pictures to external video devices based on the configured output protocols.

 * Video detection analysis (VDA) module: receives pictures from the VIU, performs motion detection analysis
   and cover detection analysis, and outputs results.

 * Audio input unit (AIU): captures the audio data

 * Audio encoding (AENC) module: encodes the captured audio data complying with multiple audio protocols and
 outputs encoded audio streams.

 * Audio decoding (ADEC) module: can decode the streams in different audio formats, and transmit
   the decoded data to the AOU for playing.

 * Audio output unit (AOU)

 * Image signal processor (ISP)

 * REGION module

 -------------------------------------------------
 * HiMPP Operations
 -------------------------------------------------
 Based on the features of the Hi35xx, the system control module supports the following operations:

 * Resets and initializes hardware components
 * Initializes and deinitializes HiMPP modules
 * Manages the mass physical memory and the status of HiMPP modules
 * Provides version information about the current HiMPP system.

 NOTE: The HiMPP must be initialized before the HiMPP services are enabled by the applications.
 Similarly, the HiMPP must be deinitialized to release resources after the HiMPP services are disabled by applications.
 */

//#################################################################################################
// INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>

/* Application Includes */
#include "DecDispLib.h"

/* HiSilicon Decoder Library Includes */
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vo.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_hdmi.h"
#include "hifb.h"
#include "acodec.h"

//#################################################################################################
// DEFINES
//#################################################################################################
/* uncomment this to print decoder stats */
//#define DECODER_STATS

#define	MAX_DEC_DISP_CHN                (72)
#define MAX_WINDOWS                     (64)
#define MAX_RESOLUTION_WIDTH            (1920)
#define MAX_RESOLUTION_HEIGHT           (1080)
#define DEF_VO_RESOLUTION               VO_OUTPUT_1080P60
#define DEFAULT_WINDOW_LAYOUT           WIND_LAYOUT_8x8_64CH

#define SEND_STREAM_MODE                (-1)
#define VO_LAYER_ID                     GRAPHICS_LAYER_G0
#define VO_DEVICE_ID                    DEVICE_DHD0
#define MAX_DEC_CAPABILITY              (960 * 1000 * 1000)     /* 960 Mega Pixel */
#define PIXEL_FORMAT                    PIXEL_FORMAT_YUV_SEMIPLANAR_420
#define MAX_CONTRAST_LEVEL              (100)   /* Range: 0 to 100 */
#define REF_MIN_CONTRAST                (30)
#define CONTRAST_MULTIPLIER             ((float)(MAX_CONTRAST_LEVEL -(REF_MIN_CONTRAST))/(float)MAX_CONTRAST_LEVEL)
#define MAX_VPSS_SCALE_SUPPORTED    	(15)

/* Audio */
#define AUDIO_AI_DEV                    0
#define AUDIO_AI_CHN                    0
#define AUDIO_AENC_CHN                  0
#define AIN_CHN_COUNT                   1
#define ACODEC_FILE                     "/dev/acodec"

/* Audio Test Application */
#define TEST_AUDIO_HDMI_AO_DEV          1
#define TEST_AUDIO_AI_DEV               0

/* macro to print API name in debug on failure */
#define API_FAIL_DEBUG(MPI_name, s32ret) EPRINT(GUI_LIB, "api %s failed: [s32ret=%#x]", MPI_name, s32ret);

//#################################################################################################
// TYPEDEF
//#################################################################################################
typedef enum
{
    GRAPHICS_LAYER_G0 = 0,
    VIDEO_LAYER_G1 = 1,
    CURSOR_LAYER_HC0 = 2,
    HI_MAX_LAYER

}HI_DISPLAY_LAYER_e;

typedef enum
{
    DECODER_CREATION,
    VO_CHN_ENABLE,
    VPSS_CREATION,
    BIND_VDEC_VPSS,
    BIND_VPSS_VO,
    MAX_STATE

}DECODER_CREATION_STATE_e;

typedef enum
{
    DEVICE_DHD0 = 0,
    DEVICE_DHD1 = 1,
    DEVICE_CURSOR = 2,
    HI_MAX_DEVICE
}HI_DEVICE_e;

typedef enum
{
    AUD_AO_START,
    AUD_BIND,
    MAX_AUD_STATE
}AUDIO_OUT_STATE_e;

/* Enum  Contain list of Audio Device */
typedef enum
{
    AUDIO_AO_DEV = 0,
    AUDIO_HDMI_AO_DEV = 1,
    MAX_AUDIO_DEVICE
}AUDIO_DEVICE_e;

typedef struct
{
    DISPLAY_RESOLUTION_e    dispResolution;
    WIND_LAYOUT_ID_e        windowLayout;
}DISPLAY_INFO_t;

// structure which contains information of decoder as well as display instance
typedef struct
{
    BOOL                iFrameRecv;
    BOOL                status;
    UINT8               decoderId;
    UINT16              noOfReferanceFrame;
    PAYLOAD_TYPE_E      vidCodec;
    UINT16              frameWidth;
    UINT16              frameHeight;
    UINT16              frameRate;          //for configured fps of camera or playback
    pthread_mutex_t     decDispParamMutex;
}DEC_DISP_LIST_t;

// structure which contains library information.
typedef struct
{
    DISPLAY_INFO_t  displayInfo;
    pthread_mutex_t ch2WindMapMutex;
    UINT8           ch2WinMap[MAX_WINDOWS];
}DEC_DISP_LIB_t;

typedef struct
{
    HI_S32 start_X[MAX_WINDOWS];
    HI_S32 start_Y[MAX_WINDOWS];
    HI_S32 winHeight[MAX_WINDOWS];
    HI_S32 winWidth[MAX_WINDOWS];
    HI_S32 totalNumberWin;
}WinGeometryParam_t;

// structure which contains information of Audio buffer
typedef struct
{
    UINT8           audioChannel;
    BOOL            terAudioThread;
    BOOL            threadRunningFlag;
    pthread_mutex_t audioTaskMutex;
    UINT32          wrPtr;
    UINT32          rdPtr;
    pthread_cond_t  streamSignal;
}AUDIO_STREAM_t;

//#################################################################################################
// GLOBALS
//#################################################################################################
static pthread_mutex_t		decDispListMutex = PTHREAD_MUTEX_INITIALIZER;
static DEC_DISP_LIST_t		decDispList[MAX_DEC_DISP_CHN];
static DEC_DISP_LIB_t		decDispLib;
static pthread_mutex_t      currentUsageMutex = PTHREAD_MUTEX_INITIALIZER;
static UINT32               decoderCurrentUsage = 0;
static pthread_t            hdmiThreadId;
#ifdef DECODER_STATS
static pthread_t            decoderDebug;
#endif
static pthread_cond_t 		hdmiSignal;
static pthread_mutex_t		hdmiLock;
static BOOL                 exitF;
static BOOL                 HDMIConStatus;
static HDMI_CALLBACK        hdmiCallBackFn = NULL;
static UINT32               tvAdjustOffset = 0;
static INT32                frameBufFd = -1;
static HI_S32               g_s32VBSource = 0;
static VO_INTF_SYNC_E       gCurr_vo_resolution = DEF_VO_RESOLUTION;

/* for dividing total height and width for single window geometry */
static const UINT8 winMultiple[WIND_LAYOUT_MAX] = {1, 2, 3, 4, 4, 3, 4, 5, 4, 4, 5, 5, 4, 5, 6, 8, 1, 2, 3, 4};

/* Table containing the maximum No. of window on layout & maximum No. of instant possible on Sequence */
static const UINT8 maxWindPerLayout[WIND_LAYOUT_MAX] = {1, 4, 6, 7, 8, 9, 10, 10, 13, 13, 13, 14, 16, 25, 36, 64, 1, 4, 9, 16};

static const PAYLOAD_TYPE_E hiVidCodecMap[MAX_VIDEO_CODEC] =
{
    0,
    PT_MJPEG,
    PT_H264,
    PT_MP4VIDEO,
    0,
    PT_H265
};

static const CHARPTR hiHdmiVidFormat[HI_HDMI_VIDEO_FMT_BUTT] =
{
    "1080P_60",             /* 0 */
    "1080P_50",             /* 1 */
    "1080P_30",             /* 2 */
    "1080P_25",             /* 3 */
    "1080P_24",             /* 4 */
    "1080i_60",             /* 5 */
    "1080i_50",             /* 6 */
    "720P_60",              /* 7 */
    "720P_50",              /* 8 */
    "576P_50",              /* 9 */
    "480P_60",              /* 10 */
    "PAL",                  /* 11 */
    "PAL_N",                /* 12 */
    "PAL_Nc",               /* 13 */
    "NTSC",                 /* 14 */
    "NTSC_J",               /* 15 */
    "NTSC_PAL_M",           /* 16 */
    "SECAM_SIN",            /* 17 */
    "SECAM_COS",            /* 18 */
    "861D_640X480_60",		/* 19 */
    "VESA_800X600_60",		/* 20 */
    "VESA_1024X768_60",		/* 21 */
    "VESA_1280X720_60",		/* 22 */
    "VESA_1280X800_60",		/* 23 */
    "VESA_1280X1024_60",	/* 24 */
    "VESA_1366X768_60",		/* 25 */
    "VESA_1440X900_60",		/* 26 */
    "VESA_1440X900_60_RB",	/* 27 */
    "VESA_1600X900_60_RB",	/* 28 */
    "VESA_1600X1200_60",	/* 29 */
    "VESA_1680X1050_60",	/* 30 */
    "VESA_1920X1080_60",	/* 31 */
    "VESA_1920X1200_60",	/* 32 */
    "VESA_2048X1152_60",	/* 33 */
    "2560x1440_30",         /* 34 */
    "2560x1440_60",         /* 35 */
    "2560x1600_60",         /* 36 */
    "1920x2160_30",         /* 37 */
    "3840X2160P_24",		/* 38 */
    "3840X2160P_25",		/* 39 */
    "3840X2160P_30",		/* 40 */
    "3840X2160P_50",		/* 41 */
    "3840X2160P_60",		/* 42 */
    "4096X2160P_24",		/* 43 */
    "4096X2160P_25",		/* 44 */
    "4096X2160P_30",		/* 45 */
    "4096X2160P_50",		/* 46 */
    "4096X2160P_60",		/* 47 */
    "VESA_USER_DEFINED"		/* 48 */
};

/* Audio out */
static AUDIO_STREAM_t               audioStream;
static BOOL                         audioOutInitStatus = FALSE;
static PAYLOAD_TYPE_E               gs_enPayloadType;
static HI_BOOL                      gs_bAioReSample[2];
static AUDIO_RESAMPLE_ATTR_S        *gs_pstAoReSmpAttr = NULL;
static AIO_ATTR_S                   stAioAttr;
static UINT8                        audioBuffer[MAX_AUDIO_BUFFER_SIZE];
static AUDIO_STATE_e                curAudioOutState = MAX_AUDIO_STATE;
static UINT8                        curAudioOutVol;

/* Audio In */
static BOOL                         audioInRunStatus = FALSE;
static PAYLOAD_TYPE_E               gs_enPayloadType_Ai = PT_G711U; /* encoding type for audio in */
static HI_S32                       audioInFd = INVALID_CONNECTION; /* fd to read audio data from Ai */

/* Audio Test Application */
static AIO_ATTR_S                   stTestAioAttr;
static AUDIO_RESAMPLE_ATTR_S        *gs_pstTestAoReSmpAttr = NULL;

//-------------------------------------------------------------------------------------------------
typedef void (*HI_HDMI_CallBack)(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL setDispResolution(DISPLAY_DEV_e dispDevId, DISPLAY_RESOLUTION_e dispResolution);
//-------------------------------------------------------------------------------------------------
static HI_S32 initMpp(void);
//-------------------------------------------------------------------------------------------------
static HI_S32 sysInit(VB_CONF_S *pstVbConf);
//-------------------------------------------------------------------------------------------------
static void sysExit(void);
//-------------------------------------------------------------------------------------------------
static HI_S32 createDecChn(DEC_DISP_PLAY_ID decChannelId, VDEC_CHN_ATTR_S* stVdecChnAttr);
//-------------------------------------------------------------------------------------------------
static HI_S32 deleteDecChn(VDEC_CHN s32ChnNum);
//-------------------------------------------------------------------------------------------------
static HI_S32 createVpssGrp(DEC_DISP_PLAY_ID decChannelId, HI_S32 s32ChnCnt, SIZE_S* resolution);
//-------------------------------------------------------------------------------------------------
static HI_S32 deleteVpssGrp(VPSS_GRP s32GrpCnt, HI_S32 s32ChnCnt);
//-------------------------------------------------------------------------------------------------
static HI_S32 startVoChn(VO_LAYER VoLayer, WIND_LAYOUT_ID_e windowLayout);
//-------------------------------------------------------------------------------------------------
static HI_S32 getVoWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W, HI_U32 *pu32H, HI_U32 *pu32Frm);
//-------------------------------------------------------------------------------------------------
static HI_S32 startVoDev(VO_DEV VoDev, VO_INTF_SYNC_E vo_res);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopVoDev(VO_DEV VoDev);
//-------------------------------------------------------------------------------------------------
static HI_S32 startVoLayer(VO_LAYER VoLayer, VO_INTF_SYNC_E vo_res);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopVoLayer(VO_LAYER VoLayer);
//-------------------------------------------------------------------------------------------------
static void stopVoChn(VO_LAYER VoLayer);
//-------------------------------------------------------------------------------------------------
static HI_S32 bindVdecVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp);
//-------------------------------------------------------------------------------------------------
static HI_S32 bindVpssVo(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 unBindVpssVo(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 unBindVdecVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp);
//-------------------------------------------------------------------------------------------------
static void *hdmi_detection_thread(void *data);
//-------------------------------------------------------------------------------------------------
#ifdef DECODER_STATS
static void *decoder_debug_thread(void *data);
#endif
//-------------------------------------------------------------------------------------------------
static void hdmiEventHandler(HI_HDMI_EVENT_TYPE_E eventId, void *appData);
//-------------------------------------------------------------------------------------------------
static BOOL startVoAndHdmi(VO_INTF_SYNC_E vo_resolution);
//-------------------------------------------------------------------------------------------------
static HI_S8 startHdmi(VO_INTF_SYNC_E enIntfSync);
//-------------------------------------------------------------------------------------------------
static UINT8 getTotalWindOnLayout(WIND_LAYOUT_ID_e windowLayout);
//-------------------------------------------------------------------------------------------------
static BOOL addDecoderUsage(UINT8 decoderId, UINT16 oldHeight, UINT16 oldWidth, UINT16 newHeight, UINT16 newWidth, UINT16 oldFps, UINT16 newFps);
//-------------------------------------------------------------------------------------------------
static void removeDecoderUsage(UINT16 height, UINT16 width, UINT16 fps);
//-------------------------------------------------------------------------------------------------
static HI_S32 getFbInfo(HI_U32 u32Width, HI_U32 u32Height);
//-------------------------------------------------------------------------------------------------
static HI_S8 generateWindowGeometry(WIND_LAYOUT_ID_e windLayoutId, VO_VIDEO_LAYER_ATTR_S stLayerAttr, WinGeometryParam_t *winGeometry);
//-------------------------------------------------------------------------------------------------
static INT32 getPageNo(DISPLAY_DEV_e displayId, UINT8 windId);
//-------------------------------------------------------------------------------------------------
static void deInitDecoderResources(UINT8 decoderId, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static HI_S32 clearPrevDecResources(DECODER_CREATION_STATE_e currState, UINT8 decoderId, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static UINT8 getWindowId(UINT8 decoderId);
//-------------------------------------------------------------------------------------------------
static HI_S32 updateChn2DecLink(UINT8 windId);
//-------------------------------------------------------------------------------------------------
static HI_S32 unlinkChn(UINT8 windId);
//-------------------------------------------------------------------------------------------------
static void *liveAudioPlay(void *pThArg);
//-------------------------------------------------------------------------------------------------
static HI_S32 unbindAoAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt, HI_BOOL bResampleEn, HI_BOOL bVqeEn);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopAudioDec(ADEC_CHN AdChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 bindAoAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 startAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt, AIO_ATTR_S* pstAioAttr, AUDIO_SAMPLE_RATE_E enInSampleRate,
                      HI_BOOL bResampleEn, HI_VOID* pstAoVqeAttr, HI_U32 u32AoVqeType);
//-------------------------------------------------------------------------------------------------
static HI_S32 startAudioDec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType);
//-------------------------------------------------------------------------------------------------
static HI_S32 audioCfgAcodec(AUDIO_SAMPLE_RATE_E enSample);
//-------------------------------------------------------------------------------------------------
static void setEnPayloadType(STREAM_CODEC_TYPE_e frameCodec);
//-------------------------------------------------------------------------------------------------
static BOOL FreeAudResources(AUDIO_OUT_STATE_e currState, AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
//-------------------------------------------------------------------------------------------------
static void initAudioStream(void);
//-------------------------------------------------------------------------------------------------
static HI_S32 startAudioOut(HI_VOID);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopAudioOut(HI_VOID);
//-------------------------------------------------------------------------------------------------
static HI_S32 startAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,AIO_ATTR_S* pstAioAttr, AUDIO_SAMPLE_RATE_E enOutSampleRate,
                      HI_BOOL bResampleEn, HI_VOID* pstAiVqeAttr, HI_U32 u32AiVqeType);
//-------------------------------------------------------------------------------------------------
static HI_S32 bindAoAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 unbindAoAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,HI_BOOL bResampleEn, HI_BOOL bVqeEn);
//-------------------------------------------------------------------------------------------------
static HI_S32 startAenc(HI_S32 sAencChn, HI_U32 u32AencPtNumPerFrm, PAYLOAD_TYPE_E enType);
//-------------------------------------------------------------------------------------------------
static HI_S32 stopAenc(HI_S32 AencChn);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize display device with default layout and resolution. And also
 *          initialize MAX_DECODE_CHN decoder instance of H264. And finally create a link for decode
 *          and display input frames
 * @param   resolution
 * @return  TRUE on success; FALSE otherwise
 */
BOOL InitDecodeDisplay(DISPLAY_RESOLUTION_e resolution)
{
    INT32               decDisLisCnt = 0;
    UINT8               winChn = 0;
    VO_INTF_SYNC_E      vo_resolution;

    EPRINT(GUI_LIB,"initialization of hi3536 decoder display library");

    sysExit();
    stopVoChn(VO_LAYER_ID);
    stopVoLayer(VO_LAYER_ID);
    stopVoDev(VO_DEVICE_ID);

    if(initMpp() != HI_SUCCESS)
    {
        EPRINT(GUI_LIB,"InitMpp failed");
        return FAIL;
    }

    exitF = FALSE;
    HDMIConStatus = FALSE;
    GUI_LIB_MUTEX_INIT(hdmiLock, NULL);
    pthread_cond_init(&hdmiSignal, NULL);

    decDispLib.displayInfo.dispResolution = resolution;
    decDispLib.displayInfo.windowLayout = DEFAULT_WINDOW_LAYOUT;

    /* Initialize Display with configured resolution(Using Default Refresh frequencies) */
    switch(decDispLib.displayInfo.dispResolution)
    {
        case DISPLAY_RESOLUTION_720P:
            vo_resolution = VO_OUTPUT_720P60;
            break;
        case DISPLAY_RESOLUTION_2160P:
            vo_resolution = VO_OUTPUT_3840x2160_30;
            break;
        default:
            vo_resolution = VO_OUTPUT_1080P60;
            break;
    }

    DPRINT(GUI_LIB,"[vo_resolution=%d] [display resolution=%d]", vo_resolution,decDispLib.displayInfo.dispResolution);

    if(FAIL == startVoAndHdmi(vo_resolution))
    {
        EPRINT(GUI_LIB,"startVoAndHdmi failed");
        sysExit();
        return  FAIL;
    }
    gCurr_vo_resolution = vo_resolution;

    /* Initialise thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (1 * MEGA_BYTE));

    /* Create HDMI Detection Thread */
    if (STATUS_OK != pthread_create(&hdmiThreadId, &attr, &hdmi_detection_thread, NULL))
    {
        EPRINT(GUI_LIB,"Failed to create hdmi_detection_thread");
    }
    pthread_attr_destroy(&attr);

    for(decDisLisCnt = 0;decDisLisCnt < MAX_DEC_DISP_CHN;decDisLisCnt++)
    {
        //Initially setting default parameter
        decDispList[decDisLisCnt].status        = FREE;
        decDispList[decDisLisCnt].vidCodec      = hiVidCodecMap[VIDEO_CODEC_NONE];
        decDispList[decDisLisCnt].frameHeight   = 0;
        decDispList[decDisLisCnt].frameWidth    = 0;
        decDispList[decDisLisCnt].frameRate     = 0;
        decDispList[decDisLisCnt].iFrameRecv    = FALSE;
        decDispList[decDisLisCnt].decoderId     = INVALID_DEC_DISP_ID;
        decDispList[decDisLisCnt].noOfReferanceFrame = 0;
        GUI_LIB_MUTEX_INIT(decDispList[decDisLisCnt].decDispParamMutex, NULL);
    }

    GUI_LIB_MUTEX_INIT(decDispLib.ch2WindMapMutex, NULL);

    for (winChn = 0; winChn < MAX_WINDOWS; winChn++)
    {
        decDispLib.ch2WinMap[winChn] = INVALID_DEC_DISP_ID;
    }

    initAudioStream();

#ifdef DECODER_STATS
    if(pthread_create(&decoderDebug, NULL, &decoder_debug_thread, NULL) != STATUS_OK)
        EPRINT(GUI_LIB,"decoder_debug_thread not created");
    else
        EPRINT(GUI_LIB,"decoder_debug_thread created");
#endif
	/* Set configured Display resolution */
	if(FAIL == setDispResolution((DISPLAY_DEV_e)0, resolution))
	{
		EPRINT(GUI_LIB,"Failed to set display resolution");
	}
    return SUCCESS;
}

#ifdef DECODER_STATS
//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will print following Decoder stats
 *          a) current decoder capacity used
 *          b) list of decoder ID used with current resolution with status(free or busy)
 *          c) Decoder performance stats :
 *              1) LeftByte : Number of bytes left to be decoded.
 *              2) LeftFrm  : Number of frames left to be decoded.
 *              3) RecvFrm,DecFrm : Total received frames and total decoded frames
 *              4) Error Count : Total 8 types of errors are covered
 *          d) Decoder ID & window ID mapping.
 *          for more details refer: svn://192.168.100.5/Briefcase/SWD_Briefcase/Security/RDK/HI3536/Hi3536 V100R001C02SPC060/01.software/
 *                                  board/document_en/HiMPP V3.0 Media Processing Software Development Reference
 * @param   data
 */
static void *decoder_debug_thread(void *data)
{
    INT32           decDisLisCnt = 0;
    UINT8           winChn = 0;
    VDEC_CHN_STAT_S stats;
    HI_S32          s32Ret;
    FILE            *fp = NULL;

    prctl(PR_SET_NAME, "Deoder_stats", 0, 0, 0);
    while(1)
    {
        sleep(10);

        fp = fopen(LOG_DIR_PATH "/Decoder_stats.txt","w");
        if(fp == NULL)
        {
            continue;
        }

        GUI_LIB_MUTEX_LOCK(currentUsageMutex);
        fprintf(fp,"Decoder Usage:[current=%u][Max=%d][avail=%u][curr2MP=%u][aval2MP=%u]\n\n",
                decoderCurrentUsage, MAX_DEC_CAPABILITY, (MAX_DEC_CAPABILITY - decoderCurrentUsage),
                (decoderCurrentUsage/(1920*1080*30)), ((MAX_DEC_CAPABILITY - decoderCurrentUsage)/(1920*1080*30)));
        GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
        fprintf(fp,"%-7s%-7s%-10s%-15s%-15s%-15s\n","SrNo","status","decoderId","frameWidth","frameHeight","frameRate");

        for(decDisLisCnt = 0;decDisLisCnt < MAX_DEC_DISP_CHN;decDisLisCnt++)
        {
            GUI_LIB_MUTEX_LOCK(decDispList[decDisLisCnt].decDispParamMutex);
            if((decDispList[decDisLisCnt].status!=FREE) && (decDispList[decDisLisCnt].decoderId !=INVALID_DEC_DISP_ID))
            {
                fprintf(fp,"%-7d%-7s%-10d%-15d%-15d%-15d\n",decDisLisCnt, (decDispList[decDisLisCnt].status ==FREE?"Free":"busy"),
                        decDispList[decDisLisCnt].decoderId, decDispList[decDisLisCnt].frameWidth,
                        decDispList[decDisLisCnt].frameHeight, decDispList[decDisLisCnt].frameRate);
            }
            GUI_LIB_MUTEX_UNLOCK(decDispList[decDisLisCnt].decDispParamMutex);
        }

        fprintf(fp,"\n%-6s%-10s%-10s%-10s%-10s%-10s%-7s%-7s%-7s%-7s%-7s%-7s%-7s\n",
                "DecID","LeftByte","LeftFrm","LeftPics","RecvFrm","DecFrm","ErFormt","ErPack","ErSize","ErBuf","ErPrt","ErRef","ErUSup");

        for(decDisLisCnt = 0;decDisLisCnt < MAX_DEC_DISP_CHN;decDisLisCnt++)
        {
            GUI_LIB_MUTEX_LOCK(decDispList[decDisLisCnt].decDispParamMutex);
            if((decDispList[decDisLisCnt].status!=FREE) && (decDispList[decDisLisCnt].decoderId !=INVALID_DEC_DISP_ID))
            {
                /*Queries the status of a VDEC channel.  */
                if((s32Ret = HI_MPI_VDEC_Query(decDisLisCnt,&stats) != HI_SUCCESS))
                {
                    API_FAIL_DEBUG("HI_MPI_VDEC_Query",s32Ret);
                }
                else
                {
                    //few param Will be INT_MAX in case of STREAM Mode(h265)
                    fprintf(fp,"%-6d%-10u%-10u%-10u%-10u%-10u%-7d%-7d%-7d%-7d%-7d%-7d%-7d\n",
                            decDisLisCnt,
                            stats.u32LeftStreamBytes,/* left stream bytes waiting for decode */
                            stats.u32LeftStreamFrames, /* left frames waiting for decode,only valid for H264D_MODE_FRAME */
                            stats.u32LeftPics,/* pics waiting for output */
                            stats.u32RecvStreamFrames,/* how many frames of stream has been received. valid when send by frame. */
                            stats.u32DecodeStreamFrames, /* how many frames of stream has been decoded. valid when send by frame. */
                            stats.stVdecDecErr.s32FormatErr, /* format error. eg: do not support filed */
                            stats.stVdecDecErr.s32PackErr, /* stream package error */
                            stats.stVdecDecErr.s32PicBufSizeErrSet,/* the buffer size of picture is not enough */
                            stats.stVdecDecErr.s32PicSizeErrSet,/* picture width or height is larger than chnnel width or height*/
                            stats.stVdecDecErr.s32PrtclNumErrSet,/* protocol num is not enough. eg: slice, pps, sps */
                            stats.stVdecDecErr.s32RefErrSet,/* refrence num is not enough */
                            stats.stVdecDecErr.s32StreamUnsprt); /* unsupport the stream specification */
                }
            }
            GUI_LIB_MUTEX_UNLOCK(decDispList[decDisLisCnt].decDispParamMutex);
        }

        GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
        fprintf(fp,"\n%-15s%-15s\n","Window","decoder");

        for (winChn = 0; winChn < MAX_WINDOWS; winChn++)
        {
            if(INVALID_DEC_DISP_ID != decDispLib.ch2WinMap[winChn])
            {
                fprintf(fp,"%-15d%-15d\n",winChn,decDispLib.ch2WinMap[winChn]);
            }
        }
        GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);
        fclose(fp);
    }
    pthread_exit(NULL);
}
#endif


//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function close the decode and display link as well as destroyall the decoder
 *          instance, Stop VO Layers and VO Dev
 */
void DeInitDecDispLib(void)
{
    HI_S32                  s32Ret = HI_SUCCESS;
    HI_U8                   channelCount;
    UINT8                   cnt = 0;
    MPP_CHN_S               stSrcChn;
    MPP_CHN_S               stDestChn;
    HI_HDMI_CALLBACK_FUNC_S hotPlughdmiEventHandler;

    EPRINT(GUI_LIB,"Deinit request recieved");
    for(channelCount=0; channelCount<MAX_WINDOWS; channelCount++)
    {
        stDestChn.enModId = HI_ID_VOU;
        stDestChn.s32DevId = VO_LAYER_ID;
        stDestChn.s32ChnId = channelCount;

        /*Obtains the information about a bound data source.*/
        if((s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn)) == HI_SUCCESS)
        {
            if((s32Ret = unBindVpssVo(VO_LAYER_ID, channelCount, stSrcChn.s32DevId, VPSS_CHN0)) != HI_SUCCESS)
            {
                EPRINT(GUI_LIB,"unBindVpssVo failed for [chnId=%d] [Error=%#x]", channelCount, s32Ret);
            }
        }
    }

    for(channelCount=0; channelCount<MAX_DEC_DISP_CHN; channelCount++)
    {
        if(unBindVdecVpss((VDEC_CHN)channelCount, (VPSS_GRP)channelCount) != HI_SUCCESS)
        {
            EPRINT(GUI_LIB,"unBindVdecVpss failed for [chnId=%d]",channelCount);
        }
    }

    /*Stop the VO Channel.*/
    stopVoChn(VO_LAYER_ID);

    /*Stop the VO Layer.*/
    stopVoLayer(VO_LAYER_ID);

    /*Stop the HDMI.*/
    HI_MPI_HDMI_Stop(HI_HDMI_ID_0);

    /*Unregister HDMI Callbak */
    memset(&hotPlughdmiEventHandler, 0, sizeof(HI_HDMI_CALLBACK_FUNC_S));
    hotPlughdmiEventHandler.pfnHdmiEventCallback = hdmiEventHandler;
    hotPlughdmiEventHandler.pPrivateData = NULL;
    HI_MPI_HDMI_UnRegCallbackFunc(HI_HDMI_ID_0,&hotPlughdmiEventHandler);

    /*Disable the HDMI.*/
    HI_MPI_HDMI_Close(HI_HDMI_ID_0);

    /*Deinitialize the HDMI.*/
    HI_MPI_HDMI_DeInit();
    stopVoDev(VO_DEVICE_ID);

    for(channelCount=0; channelCount<MAX_DEC_DISP_CHN; channelCount++)
    {
        if(deleteVpssGrp((VPSS_GRP)channelCount, VPSS_CHN0) != HI_SUCCESS)
        {
            EPRINT(GUI_LIB,"deleteVpssGrp failed for [channel=%d]",channelCount);
        }
    }

    for(channelCount=0; channelCount<MAX_DEC_DISP_CHN; channelCount++)
    {
        if(deleteDecChn((VDEC_CHN)channelCount) != HI_SUCCESS)
        {
            EPRINT(GUI_LIB,"deleteDecChn failed for [channel=%d]",channelCount);
        }
    }

    DPRINT(GUI_LIB,"vpss grp & dec chn deleted");
    GUI_LIB_MUTEX_LOCK(hdmiLock);
    exitF = TRUE;
    pthread_cond_signal(&hdmiSignal);
    GUI_LIB_MUTEX_UNLOCK(hdmiLock);
    pthread_join(hdmiThreadId,NULL);

    sysExit();
    for(cnt = 0; cnt < MAX_DEC_DISP_CHN; cnt++)
    {
        pthread_mutex_destroy(&decDispList[cnt].decDispParamMutex);
    }
    pthread_mutex_destroy(&decDispLib.ch2WindMapMutex);
    close(frameBufFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This callback will trigger on HDMI status change (connect --> disconnect and vice-versa)
 * @param   hdmiCallBack
 */
void HdmiRegCallBack(HDMI_CALLBACK hdmiCallBack)
{
    hdmiCallBackFn = hdmiCallBack;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initalizes Himpp module.it initalizes video buffer pool and set attributes
 *          of Himpp system and video buffer.
 * @return
 */
static HI_S32 initMpp(void)
{
    HI_S32              retVal = HI_SUCCESS;
    VB_CONF_S           vb_config;
    VDEC_MOD_PARAM_S	vdec_mod_param;

    /* public vbpool method has been previously tried, and found not appropriate for flexible resolution
     * streams bcoz it will create the block of same size for 1 resolution. below calculation is for demonstration purpose only. */

    memset(&vb_config ,0 ,sizeof(VB_CONF_S));
    //Do not change below table.it is tuned video buffer pool
    vb_config.u32MaxPoolCnt = 1;
    //combination working fine with 3 D1 channel
    vb_config.astCommPool[0].u32BlkSize = (960 * 540 * 3) >> 1;	/* 4K Preview */
    vb_config.astCommPool[0].u32BlkCnt  = 16 /* Channels */ * 6 /* No of frames */;
    vb_config.astCommPool[0].u32BlkSize = (720 * 576 * 3) >> 1;	/* Decode */
    vb_config.astCommPool[0].u32BlkCnt  = 64 /* Channels */ * 6 /* No of frames */;
    vb_config.astCommPool[0].u32BlkSize = (720 * 576 * 3) >> 1;	/* Other Activities */
    vb_config.astCommPool[0].u32BlkCnt  = 1;

    sysInit(&vb_config);
    memset(&vdec_mod_param, 0, sizeof(VDEC_MOD_PARAM_S));

    /*Obtains the parameters of a VDEC channel.*/
    retVal = HI_MPI_VDEC_GetModParam(&vdec_mod_param);
    if(HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_VDEC_GetModParam",retVal);
    }
    else
    {
        DPRINT(GUI_LIB,"[u32MiniBufMode=%u] [u32VBSource=%u]", vdec_mod_param.u32MiniBufMode, vdec_mod_param.u32VBSource);
    }

    vdec_mod_param.u32VBSource = 1;	/* 0:Module VB Pool 1:Private VB Pool 2:User VB Pool */
    /* Sets the parameters of a VDEC channel. */
    retVal = HI_MPI_VDEC_SetModParam(&vdec_mod_param);
    if(HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_VDEC_SetModParam",retVal);
    }
    else
    {
        g_s32VBSource = vdec_mod_param.u32VBSource;
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Deinitializes the HiMPP system &  HiMPP VB pool .
 * @return
 */
static void sysExit(void)
{
    INT32 loop;

    /* Exits a module public VB pool. */
    HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);

    /* Deinitializes the HiMPP system. Except the AENC and ADEC, the modules including the AIU, AOU, VIU, VOU, VENC,
     * VDEC, region management module, VDA module are destroyed or disabled */
    HI_MPI_SYS_Exit();

    for(loop = 0;loop < VB_UID_BUTT;loop++)
    {
        /* Exits a module public VB pool. */
        HI_MPI_VB_ExitModCommPool(loop);
    }

    for(loop = 0; loop < VB_MAX_POOLS; loop++)
    {
        /*Destroys a VB pool.  */
        HI_MPI_VB_DestroyPool(loop);
    }

    /*Deinitializes a HiMPP VB pool.  */
    HI_MPI_VB_Exit();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initalizes Himpp module.it initalizes video buffer pool and set attributes
 *          of Himpp system and video buffer.
 * @param   pstVbConf
 * @return
 */
static HI_S32 sysInit(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S  stSysConf = {0};
    HI_S32          s32Ret;
    HI_S32          i;

    /* Deinitializes the HiMPP system. Except the AENC and ADEC, the modules including the AIU, AOU, VIU, VOU, VENC,
     * VDEC, region management module, VDA module are destroyed or disabled */
    HI_MPI_SYS_Exit();

    for(i=0;i<VB_UID_BUTT;i++)
    {
        /* Exits a module public VB pool. */
        HI_MPI_VB_ExitModCommPool(i);
    }

    for(i=0; i<VB_MAX_POOLS; i++)
    {
        /*Destroys a VB pool.  */
        HI_MPI_VB_DestroyPool(i);
    }

    /*Deinitializes a HiMPP VB pool.  */
    HI_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        EPRINT(GUI_LIB,"ERROR: pstVbConf is null");
        return HI_FAILURE;
    }

    //maximum pool count must not be more than VB_MAX_POOLS
    if(pstVbConf->u32MaxPoolCnt >= VB_MAX_POOLS)
    {
        return HI_FAILURE;
    }

    /*Sets the attributes of a HiMPP VB pool. */
    s32Ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        API_FAIL_DEBUG("HI_MPI_VB_SetConf", s32Ret);
        return s32Ret;
    }

    /* Initializes a HiMPP VB pool. */
    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        API_FAIL_DEBUG("HI_MPI_VB_Init",s32Ret);
        return s32Ret;
    }

    /* system control config. stride of picture buffer must be aligned with this value.
     * you can choose a value from 1 to 1024, and it except 1 must be multiple of 16. */
    stSysConf.u32AlignWidth = 16;

    /*Sets system control parameters */
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_SetConf",s32Ret);
        return s32Ret;
    }

    /* Initializes the HiMPP system. Except the AENC and ADEC, the module including the AIU, AOU, VIU, VOU, VENC, VDEC,
     * region management module, VDA module are initialized. */
    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_Init",s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   First create decoder channels and then start receiving stream.
 * @param   decChannelId
 * @param   stVdecChnAttr
 * @return
 */
static HI_S32 createDecChn(DEC_DISP_PLAY_ID decChannelId, VDEC_CHN_ATTR_S* stVdecChnAttr)
{
    HI_S32	retVal;

    if (decChannelId >= VDEC_MAX_CHN_NUM)
    {
        EPRINT(GUI_LIB,"[decChannelId=%d] No free VDEC channel ",decChannelId);
        return HI_ERR_VDEC_INVALID_CHNID;
    }

    if (1 == g_s32VBSource)
    {
        HI_U32 u32BlkCnt = 0;
        /*Obtains the number of VBs in a private VB pool for a VDEC channel*/
        retVal = HI_MPI_VDEC_GetChnVBCnt(decChannelId, &u32BlkCnt);
        if (HI_SUCCESS != retVal)
        {
            API_FAIL_DEBUG("HI_MPI_VDEC_GetChnVBCnt", retVal);
        }
        else
        {
            DPRINT(GUI_LIB, "create new decoder channel: [decoderId=%d], [u32BlkCnt=%u]", decChannelId, u32BlkCnt);
        }

        u32BlkCnt = stVdecChnAttr->stVdecVideoAttr.u32RefFrameNum + 2 + 1;
        /*Sets the number of VBs in a private VB pool for a VDEC channel*/
        retVal = HI_MPI_VDEC_SetChnVBCnt(decChannelId, u32BlkCnt);
        if(retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VDEC_SetChnVBCnt", retVal);
            return retVal;
        }
    }

    /*  Creates a VDEC channel. */
    retVal = HI_MPI_VDEC_CreateChn(decChannelId, stVdecChnAttr);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VDEC_CreateChn",retVal);
        return retVal;
    }

    /* Starts to receive the streams sent by the user*/
    retVal = HI_MPI_VDEC_StartRecvStream(decChannelId);
    if(retVal != HI_SUCCESS)
    {
        /*Destroys a VDEC channel. */
        HI_MPI_VDEC_DestroyChn(decChannelId);
        API_FAIL_DEBUG("HI_MPI_VDEC_StartRecvStream",retVal);
        return retVal;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   stop receiving stream for respective channel and destroy decoder instance.
 * @param   s32ChnNum
 * @return
 */
static HI_S32 deleteDecChn(VDEC_CHN s32ChnNum)
{
    HI_S32 retVal;

    /* Stops receiving the streams sent by the user. */
    retVal = HI_MPI_VDEC_StopRecvStream(s32ChnNum);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VDEC_StopRecvStream",retVal);
        return retVal;
    }

    /*Destroys a VDEC channel. */
    retVal = HI_MPI_VDEC_DestroyChn(s32ChnNum);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VDEC_DestroyChn",retVal);
        return retVal;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Creates a VPSS group, Sets the parameters of a VPSS group & VPSS channel attributes Then
 *          Enables a VPSS channel and Start a VPSS group.
 * @param   decChannelId
 * @param   s32ChnCnt
 * @param   resolution
 * @return
 */
static HI_S32 createVpssGrp(DEC_DISP_PLAY_ID decChannelId, HI_S32 s32ChnCnt, SIZE_S* resolution)
{
    VPSS_GRP            VpssGrp;
    VPSS_CHN            VpssChn;
    VPSS_GRP_ATTR_S     stGrpAttr;
    VPSS_CHN_ATTR_S     stChnAttr;
    VPSS_GRP_PARAM_S    stVpssParam;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              j;

    memset(&stGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));
    memset(&stChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));
    memset(&stVpssParam, 0, sizeof(VPSS_GRP_PARAM_S));

    stGrpAttr.u32MaxW = resolution->u32Width;
    stGrpAttr.u32MaxH = resolution->u32Height;
    stGrpAttr.enPixFmt = PIXEL_FORMAT;

    stGrpAttr.bIeEn = HI_FALSE;     /*Reserved : Must be set to 0 */
    stGrpAttr.bNrEn = HI_TRUE;      /*NR  supported*/
    stGrpAttr.bDciEn = HI_FALSE;    /*DCI  supported*/
    stGrpAttr.bHistEn = HI_FALSE;   /*Reserved : Must be set to 0 */
    /* ES enable. The ES function is not supported when the picture width(in pixel) is greater than 3840. */
    stGrpAttr.bEsEn = HI_FALSE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE; /*DEI mode*/

    VpssGrp = decChannelId;
    /* create vpss group */
    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VPSS_CreateGrp",s32Ret);
        return s32Ret;
    }

    /* Obtains the parameters of a VPSS group*/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VPSS_GetGrpParam",s32Ret);
        return s32Ret;
    }

    stVpssParam.u32IeStrength = 0;
    /* Sets the parameters of a VPSS group */
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VPSS_SetGrpParam",s32Ret);
        return s32Ret;
    }

    /*** enable vpss chn, with frame ***/
    for(j=0; j<s32ChnCnt; j++)
    {
        VpssChn = j;
        stChnAttr.bSpEn = HI_FALSE;
        stChnAttr.bUVInvert = HI_FALSE;
        stChnAttr.bBorderEn = HI_FALSE;
        stChnAttr.stBorder.u32Color = 0xff00;
        stChnAttr.stBorder.u32LeftWidth = 2;
        stChnAttr.stBorder.u32RightWidth = 2;
        stChnAttr.stBorder.u32TopWidth = 2;
        stChnAttr.stBorder.u32BottomWidth = 2;

        /*Sets VPSS channel attributes.*/
        s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VPSS_SetChnAttr",s32Ret);
            break;
        }

        /* Enables a VPSS channel.*/
        s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
        if (s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VPSS_EnableChn",s32Ret);
            break;
        }
    }

    /*Start a VPSS group.*/
    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VPSS_StartGrp",s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disables a VPSS group ,Disables a VPSS channel and  Destroys
 * @param   s32GrpCnt
 * @param   s32ChnCnt
 * @return
 */
static HI_S32 deleteVpssGrp(VPSS_GRP s32GrpCnt, HI_S32 s32ChnCnt)
{
    HI_S32      retVal = HI_SUCCESS;
    VPSS_CHN    VpssChn;

    /* Disables a VPSS group */
    retVal = HI_MPI_VPSS_StopGrp(s32GrpCnt);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VPSS_StopGrp",retVal);
        return retVal;
    }

    for(VpssChn = 0; VpssChn < s32ChnCnt; VpssChn++)
    {
        /* Disables a VPSS channel.*/
        retVal = HI_MPI_VPSS_DisableChn(s32GrpCnt, VpssChn);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VPSS_DisableChn",retVal);
        }
    }

    if (retVal == HI_SUCCESS)
    {
        /*Destroys a VPSS group*/
        retVal = HI_MPI_VPSS_DestroyGrp(s32GrpCnt);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VPSS_DestroyGrp",retVal);
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sets the the attributes of a VO channel According to selected
 * @param   VoLayer
 * @param   windowLayout
 * @return
 */
static HI_S32 startVoChn(VO_LAYER VoLayer, WIND_LAYOUT_ID_e windowLayout)
{
    HI_S32                  winCount;
    HI_S32                  s32Ret;
    VO_CHN_ATTR_S           stChnAttr;
    VO_VIDEO_LAYER_ATTR_S   stLayerAttr;
    WinGeometryParam_t      winGeometry;

    if((VoLayer == VO_CAS_DEV_1+1)||(VoLayer == VO_CAS_DEV_2+1))
    {
        /* Obtains the attributes of the PIP layer*/
        s32Ret = HI_MPI_VO_GetVideoLayerAttr(0, &stLayerAttr);
        if (s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VO_GetVideoLayerAttr",s32Ret);
        }
    }
    else
    {
        /* Obtains the attributes of the PIP layer*/
        s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
        if (s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VO_GetVideoLayerAttr",s32Ret);
        }
    }

    s32Ret = generateWindowGeometry(windowLayout, stLayerAttr, &winGeometry);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("generateWindowGeometry", s32Ret);
        return s32Ret;
    }

    for (winCount=0; winCount<winGeometry.totalNumberWin; winCount++)
    {
        stChnAttr.stRect.s32X       = winGeometry.start_X[winCount];
        stChnAttr.stRect.s32Y       = winGeometry.start_Y[winCount];
        stChnAttr.stRect.u32Width   = winGeometry.winWidth[winCount];
        stChnAttr.stRect.u32Height  = winGeometry.winHeight[winCount];
        stChnAttr.u32Priority       = 0;
        stChnAttr.bDeflicker        = HI_FALSE;

        /* Sets the attributes of a specified VO channel. */
        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, winCount, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VO_SetChnAttr",s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getVoWH gives Height, widht & FrameRate respective to Resolution
 * @param   enIntfSync
 * @param   pu32W
 * @param   pu32H
 * @param   pu32Frm
 * @return
 */
static HI_S32 getVoWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W,HI_U32 *pu32H, HI_U32 *pu32Frm)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL          : *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
        case VO_OUTPUT_NTSC         : *pu32W = 720;  *pu32H = 480;  *pu32Frm = 30; break;
        case VO_OUTPUT_576P50       : *pu32W = 720;  *pu32H = 576;  *pu32Frm = 50; break;
        case VO_OUTPUT_480P60       : *pu32W = 720;  *pu32H = 480;  *pu32Frm = 60; break;
        case VO_OUTPUT_800x600_60   : *pu32W = 800;  *pu32H = 600;  *pu32Frm = 60; break;
        case VO_OUTPUT_720P50       : *pu32W = 1280; *pu32H = 720;  *pu32Frm = 50; break;
        case VO_OUTPUT_720P60       : *pu32W = 1280; *pu32H = 720;  *pu32Frm = 60; break;
        case VO_OUTPUT_1080I50      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
        case VO_OUTPUT_1080I60      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
        case VO_OUTPUT_1080P24      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 24; break;
        case VO_OUTPUT_1080P25      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 25; break;
        case VO_OUTPUT_1080P30      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 30; break;
        case VO_OUTPUT_1080P50      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
        case VO_OUTPUT_1080P60      : *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
        case VO_OUTPUT_1024x768_60  : *pu32W = 1024; *pu32H = 768;  *pu32Frm = 60; break;
        case VO_OUTPUT_1280x1024_60 : *pu32W = 1280; *pu32H = 1024; *pu32Frm = 60; break;
        case VO_OUTPUT_1366x768_60  : *pu32W = 1366; *pu32H = 768;  *pu32Frm = 60; break;
        case VO_OUTPUT_1440x900_60  : *pu32W = 1440; *pu32H = 900;  *pu32Frm = 60; break;
        case VO_OUTPUT_1280x800_60  : *pu32W = 1280; *pu32H = 800;  *pu32Frm = 60; break;
        case VO_OUTPUT_1600x1200_60 : *pu32W = 1600; *pu32H = 1200; *pu32Frm = 60; break;
        case VO_OUTPUT_1680x1050_60 : *pu32W = 1680; *pu32H = 1050; *pu32Frm = 60; break;
        case VO_OUTPUT_1920x1200_60 : *pu32W = 1920; *pu32H = 1200; *pu32Frm = 60; break;
        case VO_OUTPUT_3840x2160_25 : *pu32W = 3840; *pu32H = 2160; *pu32Frm = 25; break;
        case VO_OUTPUT_3840x2160_30 : *pu32W = 3840; *pu32H = 2160; *pu32Frm = 30; break;
        case VO_OUTPUT_3840x2160_50 : *pu32W = 3840; *pu32H = 2160; *pu32Frm = 50; break;
        case VO_OUTPUT_3840x2160_60 : *pu32W = 3840; *pu32H = 2160; *pu32Frm = 60; break;
        case VO_OUTPUT_USER         : *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
        case VO_OUTPUT_2560x1600_60 : *pu32W = 2560; *pu32H = 1600; *pu32Frm = 60; break;
        default:
            EPRINT(GUI_LIB,"vo [enIntfSync=%d] not support!",enIntfSync);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   startVoDev
 * @param   VoDev
 * @param   vo_res
 * @return
 */
static HI_S32 startVoDev(VO_DEV VoDev, VO_INTF_SYNC_E vo_res)
{
    HI_S32          mpp_ret;
    VO_PUB_ATTR_S	vo_pub_attr;

    memset(&vo_pub_attr, 0, sizeof(VO_PUB_ATTR_S));

    /* Obtains the public attributes of a VO device*/
    mpp_ret = HI_MPI_VO_GetPubAttr(VoDev, &vo_pub_attr);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_GetPubAttr",mpp_ret);
        return mpp_ret;
    }

    vo_pub_attr.u32BgColor = 0;
    vo_pub_attr.enIntfType = VO_INTF_HDMI;
    vo_pub_attr.enIntfSync = vo_res;
    /* Sets the public attributes of a VO device. */
    mpp_ret = HI_MPI_VO_SetPubAttr(VoDev, &vo_pub_attr);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_SetPubAttr",mpp_ret);
        return mpp_ret;
    }

    /* Enables a VO device. */
    mpp_ret = HI_MPI_VO_Enable(VoDev);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_Enable",mpp_ret);
        return mpp_ret;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sets the attributes of a video layer and Enables a video layer.
 * @param   VoLayer
 * @param   vo_res
 * @return
 */
static HI_S32 startVoLayer(VO_LAYER VoLayer, VO_INTF_SYNC_E vo_res)
{
    HI_S32                  mpp_ret;
    UINT32                  width, height, disp_frm_rt;
    VO_VIDEO_LAYER_ATTR_S   vo_vlayer_attr;

    memset(&vo_vlayer_attr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
    /*Obtains the attributes of the PIP layer.*/
    mpp_ret = HI_MPI_VO_GetVideoLayerAttr(GRAPHICS_LAYER_G0, &vo_vlayer_attr);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_GetVideoLayerAttr",mpp_ret);
        return mpp_ret;
    }

    getVoWH(vo_res, &width, &height, &disp_frm_rt);
    vo_vlayer_attr.stDispRect.u32Width = width;
    vo_vlayer_attr.stDispRect.u32Height = height;
    vo_vlayer_attr.stImageSize.u32Width = width;
    vo_vlayer_attr.stImageSize.u32Height = height;
    vo_vlayer_attr.u32DispFrmRt = disp_frm_rt;
    vo_vlayer_attr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    vo_vlayer_attr.bDoubleFrame = HI_FALSE;
    vo_vlayer_attr.bClusterMode = HI_FALSE;
    /* Sets the attributes of a video layer.*/
    mpp_ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &vo_vlayer_attr);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_SetVideoLayerAttr",mpp_ret);
        return mpp_ret;
    }

    mpp_ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_EnableVideoLayer",mpp_ret);
        return mpp_ret;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disables a video layer.
 * @param   VoLayer
 * @return
 */
static HI_S32 stopVoLayer(VO_LAYER VoLayer)
{
    /*Disables a video layer.*/
    HI_S32 s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VO_Disable",s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disables a video Device.
 * @param   VoDev
 * @return
 */
static HI_S32 stopVoDev(VO_DEV VoDev)
{
    /*Disables a VO device*/
    HI_S32 mpp_ret = HI_MPI_VO_Disable(VoDev);
    if(HI_SUCCESS != mpp_ret)
    {
        API_FAIL_DEBUG("HI_MPI_VO_Disable",mpp_ret);
        return mpp_ret;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disable Respective VO Channel depend on Layout selected
 * @param   VoLayer
 */
static void stopVoChn(VO_LAYER VoLayer)
{
    INT32	loop;
    HI_S32	mpp_ret;

    for(loop = 0;loop < MAX_DEC_DISP_CHN;loop++)
    {
        /* Disables a specified VO channel*/
        mpp_ret = HI_MPI_VO_DisableChn(VoLayer, loop);
        if(HI_SUCCESS != mpp_ret)
        {
            API_FAIL_DEBUG("HI_MPI_VO_DisableChn",mpp_ret);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Bind Data Source (VDEC) with Destination VPSS.
 * @param   VdChn
 * @param   VpssGrp
 * @return
 */
static HI_S32 bindVdecVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp)
{
    HI_S32      retVal;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    /*Binds a data source and a data receiver. */
    retVal = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_Bind",retVal);
    }
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   function Bind Data Source (VPSS) with Destination VO.
 * @param   VoLayer
 * @param   VoChn
 * @param   VpssGrp
 * @param   VpssChn
 * @return
 */
static HI_S32 bindVpssVo(VO_LAYER VoLayer,VO_CHN VoChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32      retVal;
    MPP_CHN_S   stSrcChn,stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    /*Binds a data source and a data receiver. */
    retVal = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_Bind",retVal);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UnBind Data Source (VPSS) with Destination VO.
 * @param   VoLayer
 * @param   VoChn
 * @param   VpssGrp
 * @param   VpssChn
 * @return
 */
static HI_S32 unBindVpssVo(VO_LAYER VoLayer,VO_CHN VoChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32      retVal;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    /* Hides a specified channel. */
    retVal =HI_MPI_VO_HideChn(VoLayer,VoChn);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VO_HideChn",retVal);
    }

    /*Unbind a data source from a data receiver. */
    retVal = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_UnBind",retVal);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UnBind Data Source (VDEC) with Destination VPSS.
 * @param   VdChn
 * @param   VpssGrp
 * @return
 */
static HI_S32 unBindVdecVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp)
{
    HI_S32 retVal;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;
    /*Unbind a data source from a data receiver. */
    retVal = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_UnBind",retVal);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   hdmi_detection_thread Detection Of HDMI
 * @param   data
 */
static void *hdmi_detection_thread(void *data)
{
    BOOL	hdmi_status;
    BOOL	new_hdmi_status;

    prctl(PR_SET_NAME, "HDMI Thread", 0, 0, 0);

    GUI_LIB_MUTEX_LOCK(hdmiLock);
    hdmi_status = HDMIConStatus;
    GUI_LIB_MUTEX_UNLOCK(hdmiLock);

    while(1)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        GUI_LIB_MUTEX_LOCK(hdmiLock);
        if(exitF)
        {
            GUI_LIB_MUTEX_UNLOCK(hdmiLock);
            break;
        }

        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        if(hdmi_status == HDMIConStatus)
        {
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            pthread_cond_wait(&hdmiSignal, &hdmiLock);
        }

        if(exitF)
        {
            GUI_LIB_MUTEX_UNLOCK(hdmiLock);
            break;
        }

        new_hdmi_status = HDMIConStatus;
        GUI_LIB_MUTEX_UNLOCK(hdmiLock);

        if(hdmi_status != new_hdmi_status)
        {
			if(TRUE == new_hdmi_status)
			{
				/* When HDMI Display is connected, Update Display resolution.
				 * 1. To set supported Refresh frequency */
				setDispResolution((DISPLAY_DEV_e)0, decDispLib.displayInfo.dispResolution);
                if(NULL != hdmiCallBackFn)
                {
                    (*hdmiCallBackFn)(new_hdmi_status);
                }
			}
			hdmi_status = new_hdmi_status;
        }
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   hdmiEventHandler
 * @param   eventId
 * @param   appData
 */
void hdmiEventHandler(HI_HDMI_EVENT_TYPE_E eventId, void *appData)
{
    switch(eventId)
    {
        case HI_HDMI_EVENT_HOTPLUG:
			DPRINT(GUI_LIB,"HDMI Display connected");
            GUI_LIB_MUTEX_LOCK(hdmiLock);
            HDMIConStatus = TRUE;
            pthread_cond_signal(&hdmiSignal);
            GUI_LIB_MUTEX_UNLOCK(hdmiLock);
            break;

        case HI_HDMI_EVENT_NO_PLUG:
			DPRINT(GUI_LIB,"HDMI Display disconnected");
            GUI_LIB_MUTEX_LOCK(hdmiLock);
            HDMIConStatus = FALSE;
            pthread_cond_signal(&hdmiSignal);
            GUI_LIB_MUTEX_UNLOCK(hdmiLock);
            break;

        default:
			EPRINT(GUI_LIB,"Unhandled HDMI event: [eventId=%#x]", eventId);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCurrentMainResolution
 * @param   pResolution
 * @param   pDispWidth
 * @param   pDispHeight
 * @param   pFrameRate
 */
void GetCurrentMainResolution(DISPLAY_RESOLUTION_e *pResolution, UINT32 *pDispWidth, UINT32 *pDispHeight, UINT32 *pFrameRate)
{
    /* Validate input params */
    if ((NULL == pResolution) || (NULL == pDispWidth) || (NULL == pDispHeight) || (NULL == pFrameRate))
    {
        EPRINT(GUI_LIB, "null ptr found: [pResolution=%p] [pDispWidth=%p] [pDispHeight=%p] [pFrameRate=%p]",
                    pResolution, pDispWidth, pDispHeight, pFrameRate);
        return;
    }

    *pResolution = decDispLib.displayInfo.dispResolution;
    getVoWH(gCurr_vo_resolution, pDispWidth, pDispHeight, pFrameRate);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   VO_HdmiConvertSync
 * @param   enIntfSync
 * @param   penVideoFmt
 * @return
 */
static HI_VOID VO_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync, HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
            break;

        case VO_OUTPUT_NTSC:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
            break;

        case VO_OUTPUT_1080P24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
            break;

        case VO_OUTPUT_1080P25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
            break;

        case VO_OUTPUT_1080P30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;

        case VO_OUTPUT_720P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
            break;

        case VO_OUTPUT_720P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
            break;

        case VO_OUTPUT_1080I50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
            break;

        case VO_OUTPUT_1080I60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
            break;

        case VO_OUTPUT_1080P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;

        case VO_OUTPUT_1080P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;

        case VO_OUTPUT_576P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
            break;

        case VO_OUTPUT_480P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
            break;

        case VO_OUTPUT_800x600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
            break;

        case VO_OUTPUT_1024x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;

        case VO_OUTPUT_1280x1024_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;

        case VO_OUTPUT_1366x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
            break;

        case VO_OUTPUT_1440x900_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
            break;

        case VO_OUTPUT_1280x800_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
            break;

        case VO_OUTPUT_1920x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1920X1200_60;
            break;

        case VO_OUTPUT_3840x2160_25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_25;
            break;

        case VO_OUTPUT_3840x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
            break;

        case VO_OUTPUT_3840x2160_50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_50;
            break;

        case VO_OUTPUT_3840x2160_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_60;
            break;

        case VO_OUTPUT_2560x1600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1600_60;
            break;

        default :
            EPRINT(GUI_LIB,"Unkonw VO_INTF_SYNC_E value!");
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startHdmi Initializes and start the HDMI.
 * @param enIntfSync
 * @return
 */
HI_S8 startHdmi(VO_INTF_SYNC_E enIntfSync)
{
    HI_S32                  retVal;
    HI_U8                   retStatus = HI_SUCCESS;
    HI_HDMI_ATTR_S          stAttr;
    HI_HDMI_VIDEO_FMT_E     enVideoFmt;
    HI_HDMI_CALLBACK_FUNC_S	hotPlughdmiEventHandler;

    HI_MPI_HDMI_Stop(HI_HDMI_ID_0);
    HI_MPI_HDMI_Close(HI_HDMI_ID_0);
    HI_MPI_HDMI_DeInit();

    VO_HdmiConvertSync(enIntfSync, &enVideoFmt);

    HI_MPI_HDMI_Init();

    HI_MPI_HDMI_Open(HI_HDMI_ID_0);

    memset(&hotPlughdmiEventHandler, 0, sizeof(HI_HDMI_CALLBACK_FUNC_S));

    hotPlughdmiEventHandler.pfnHdmiEventCallback = hdmiEventHandler;
    hotPlughdmiEventHandler.pPrivateData = NULL;

    HI_MPI_HDMI_RegCallbackFunc(HI_HDMI_ID_0, &hotPlughdmiEventHandler);

    retVal = HI_MPI_HDMI_SetAVMute(HI_HDMI_ID_0, HI_TRUE);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_HDMI_SetAVMute",retVal);
    }

    memset(&stAttr, 0, sizeof(HI_HDMI_ATTR_S));
    HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);

    stAttr.bEnableHdmi     = HI_TRUE;
    stAttr.bEnableVideo    = HI_TRUE;
    stAttr.enVideoFmt      = enVideoFmt;

    stAttr.enVidOutMode    = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    stAttr.bxvYCCMode      = HI_FALSE;

    stAttr.enDefaultMode   = HI_HDMI_FORCE_HDMI;
    stAttr.bEnableAudio    = HI_TRUE;
    stAttr.enSoundIntf     = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.enSampleRate    = HI_HDMI_SAMPLE_RATE_48K;

    stAttr.bIsMultiChannel = HI_FALSE;
    stAttr.enBitDepth      = HI_HDMI_BIT_DEPTH_16;
    stAttr.bEnableAviInfoFrame  = HI_TRUE;
    stAttr.bEnableAudInfoFrame = HI_TRUE;
    stAttr.u8I2SCtlVbit = 0;

    stAttr.bEnableSpdInfoFrame  = HI_FALSE;
    stAttr.bEnableMpegInfoFrame = HI_FALSE;

    stAttr.bDebugFlag = HI_FALSE;
    stAttr.bHDCPEnable = HI_FALSE;
    stAttr.b3DEnable = HI_FALSE;

    HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);

	/* Reference: HiMPP V3.0 FAQs.pdf Topic-3.7
	 * [Symptom]
	 * When the HDMI is connected to the 4K display (Samsung u28e590d), colors on the display
	 * are abnormal and the screen has a pink cast. The value of AVI Infoframe is set to enable by
	 * using HI_MPI_HDMI_SetAttr, but the value in the HDMI proc information is disable.
	 * [Cause Analysis]
	 * The time for HDMI protocol negotiation between the display and the chip is too long. When
	 * HI_MPI_HDMI_SetAttr is called to set AVI Infoframe, only the settings at the software
	 * layer are complete, and the negotiation at the bottom layer is not completed. When
	 * HI_MPI_HDMI_Start is called, the AVI Infoframe information is not set to the register.
	 * [Solution]
	 * After HI_MPI_HDMI_SetAttr is called, wait about 1s, and then call HI_MPI_HDMI_Start to
	 * start the HDMI */
	sleep(1);

    HI_MPI_HDMI_Start(HI_HDMI_ID_0);

    retVal = HI_MPI_HDMI_SetAVMute(HI_HDMI_ID_0, HI_FALSE);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_HDMI_SetAVMute",retVal);
    }

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It gives total number of window in particular layout.
 * @param   windowLayout
 * @return
 */
static UINT8 getTotalWindOnLayout(WIND_LAYOUT_ID_e windowLayout)
{
    if ((windowLayout < WIND_LAYOUT_MAX) && (windowLayout >= WIND_LAYOUT_1X1_1CH))
    {
        return maxWindPerLayout[windowLayout];
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check that new resolution changes is does not exceed decoder capability.
 * @param   decoderId
 * @param   oldHeight
 * @param   oldWidth
 * @param   newHeight
 * @param   newWidth
 * @param   oldFps
 * @param   newFps
 * @return
 */
static BOOL addDecoderUsage(UINT8 decoderId, UINT16 oldHeight, UINT16 oldWidth, UINT16 newHeight, UINT16 newWidth, UINT16 oldFps, UINT16 newFps)
{
    UINT32	reqDecUsage;
    UINT32	old_chnl_cap = (oldWidth * oldHeight) * oldFps;
    UINT32  new_chnl_cap = (newWidth * newHeight) * newFps;

    GUI_LIB_MUTEX_LOCK(currentUsageMutex);
    if(old_chnl_cap > decoderCurrentUsage)
    {
        GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
        return FAIL;
    }

    reqDecUsage = (decoderCurrentUsage - old_chnl_cap) + new_chnl_cap;
    if (reqDecUsage > (UINT32)MAX_DEC_CAPABILITY)
    {
        EPRINT(GUI_LIB, "no decoding capacity: [decoderId=%d], [used=%u], [free=%u], [required=%u], [res=%dx%d], [fps=%d]",
               decoderId, decoderCurrentUsage, (MAX_DEC_CAPABILITY - decoderCurrentUsage), new_chnl_cap, newWidth, newHeight, newFps);
        GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
        return FAIL;
    }

    decoderCurrentUsage = reqDecUsage;
    DPRINT(GUI_LIB, "acquired decoding capacity: [decoderId=%d], [acquired=%u], [used=%u], [free=%u]",
           decoderId, new_chnl_cap, decoderCurrentUsage, (MAX_DEC_CAPABILITY - decoderCurrentUsage));
    GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   update the current decoder usage of NVR system.
 * @param   height
 * @param   width
 * @param   fps
 */
static void removeDecoderUsage(UINT16 height, UINT16 width, UINT16 fps)
{
    UINT32 chnl_cap = (width * height) * fps;

    GUI_LIB_MUTEX_LOCK(currentUsageMutex);
    if (chnl_cap > decoderCurrentUsage)
    {
        decoderCurrentUsage = 0;
    }
    else
    {
        decoderCurrentUsage -= chnl_cap;
    }

    DPRINT(GUI_LIB, "release decoding capacity: [released=%u], [used=%u], [free=%u]", chnl_cap, decoderCurrentUsage, (MAX_DEC_CAPABILITY - decoderCurrentUsage));
    GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   open and set resolution for frame buffer.
 * @param   u32Width
 * @param   u32Height
 * @return
 */
static HI_S32 getFbInfo(HI_U32 u32Width, HI_U32 u32Height)
{
    struct fb_var_screeninfo    var;
    struct fb_fix_screeninfo    fix;
    static struct fb_bitfield   s_a32 = {24,8,0};
    static struct fb_bitfield   s_r32 = {16,8,0};
    static struct fb_bitfield   s_g32 = {8,8,0};
    static struct fb_bitfield   s_b32 = {0,8,0};
    HI_BOOL                     bShow;
    HIFB_POINT_S                stPoint;
    HIFB_ALPHA_S                hifb_alpha;
    HIFB_LAYER_INFO_S           stLayerInfo;
    HIFB_CAPABILITY_S           stCapability;

    if(frameBufFd < 0)
    {
        frameBufFd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
        if(frameBufFd < 0)
        {
            EPRINT(GUI_LIB,"failed to open /dev/fb0: [err=%s]", strerror(errno));
            return HI_FAILURE;
        }
        else
        {
			DPRINT(GUI_LIB,"[frameBufFd=%d]",frameBufFd);
        }
    }

    bShow = HI_FALSE;
    if(ioctl(frameBufFd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOPUT_SHOW_HIFB failed");
        return HI_FAILURE;
    }

    memset(&stCapability, 0, sizeof(HIFB_CAPABILITY_S));
    if(ioctl(frameBufFd, FBIOGET_CAPABILITY_HIFB, &stCapability) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_CAPABILITY_HIFB failed");
        return HI_FAILURE;
    }

    DPRINT(GUI_LIB,"[bKeyRgb=%d] [bKeyAlpha=%d] [bGlobalAlpha=%d] [bCmap=%d] [bHasCmapReg=%d] [bVoScale=%d] [bLayerSupported=%d]",
           stCapability.bKeyRgb, stCapability.bKeyAlpha, stCapability.bGlobalAlpha, stCapability.bCmap, stCapability.bHasCmapReg,
           stCapability.bVoScale, stCapability.bLayerSupported);
    DPRINT(GUI_LIB,"[u32MaxWidth=%u] [u32MaxHeight=%u] [u32VDefLevel=%u] [u32HDefLevel=%u] [u32MinWidth=%u] [u32MinHeight=%u] [bDcmp=%d] [bPreMul=%d]",
           stCapability.u32MaxWidth, stCapability.u32MaxHeight, stCapability.u32VDefLevel, stCapability.u32HDefLevel,
           stCapability.u32MinWidth, stCapability.u32MinHeight, stCapability.bDcmp, stCapability.bPreMul);

    memset(&stPoint, 0, sizeof(HIFB_POINT_S));
    stPoint.s32XPos = 0;
    stPoint.s32YPos = 0;
    if (ioctl(frameBufFd, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOPUT_SCREEN_ORIGIN_HIFB failed");
        return HI_FAILURE;
    }

    memset(&var, 0, sizeof(struct fb_var_screeninfo));
    if(ioctl(frameBufFd, FBIOGET_VSCREENINFO, &var) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_VSCREENINFO failed");
        return HI_FAILURE;
    }

    DPRINT(GUI_LIB,"[var.xres=%d] [var.yres=%d] [var.xres_virtual=%d] [var.yres_virtual=%d]",
           var.xres,var.yres,var.xres_virtual,var.yres_virtual);

    var.xres = u32Width;
    var.yres = u32Height;
    var.xres_virtual = u32Width;
    var.yres_virtual = u32Height;
    var.xoffset = 0;
    var.yoffset = 0;
    var.bits_per_pixel = 32;
    var.transp = s_a32;
    var.red = s_r32;
    var.green = s_g32;
    var.blue = s_b32;
    var.activate = FB_ACTIVATE_NOW;

    if(ioctl(frameBufFd, FBIOPUT_VSCREENINFO, &var) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOPUT_VSCREENINFO failed");
        return HI_FAILURE;
    }

    memset(&var, 0, sizeof(struct fb_var_screeninfo));
    if(ioctl(frameBufFd, FBIOGET_VSCREENINFO, &var) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_VSCREENINFO failed");
        return HI_FAILURE;
    }

    DPRINT(GUI_LIB,"[var.xres=%d] [var.yres=%d] [var.xres_virtual=%d] [var.yres_virtual=%d]",
           var.xres,var.yres,var.xres_virtual,var.yres_virtual);

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    /* get the fix screen info */
    if(ioctl(frameBufFd, FBIOGET_FSCREENINFO, &fix) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_FSCREENINFO failed");
        return HI_FAILURE;
    }
    DPRINT(GUI_LIB,"fix line_length %u", fix.line_length);

    memset(&stLayerInfo, 0, sizeof(HIFB_LAYER_INFO_S));
    /* get the layer info */
    if(ioctl(frameBufFd, FBIOGET_LAYER_INFO, &stLayerInfo) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_LAYER_INFO failed!");
        return HI_FAILURE;
    }

    DPRINT(GUI_LIB,"[BufMode=%#x] [eAntiflickerLevel=%#x] [s32XPos=%d] [s32YPos=%d] [u32CanvasWidth=%d] [u32CanvasHeight=%d]",
           stLayerInfo.BufMode, stLayerInfo.eAntiflickerLevel, stLayerInfo.s32XPos, stLayerInfo.s32YPos, stLayerInfo.u32CanvasWidth, stLayerInfo.u32CanvasHeight);
    DPRINT(GUI_LIB,"[u32DisplayWidth=%u] [u32DisplayHeight=%u] [u32ScreenWidth=%u] [u32ScreenHeight=%u] [bPreMul=%u] [u32Mask=%#x]",
           stLayerInfo.u32DisplayWidth, stLayerInfo.u32DisplayHeight, stLayerInfo.u32ScreenWidth, stLayerInfo.u32ScreenHeight, stLayerInfo.bPreMul, stLayerInfo.u32Mask);

    stLayerInfo.u32Mask = 0;
    stLayerInfo.BufMode = HIFB_LAYER_BUF_NONE;
    stLayerInfo.s32XPos = 0;
    stLayerInfo.s32YPos = 0;
    stLayerInfo.u32Mask |= (HIFB_LAYERMASK_BUFMODE | HIFB_LAYERMASK_POS);

    /* set the layer info */
    if(ioctl(frameBufFd, FBIOPUT_LAYER_INFO, &stLayerInfo) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOPUT_LAYER_INFO failed!");
        return HI_FAILURE;
    }

    memset(&stLayerInfo, 0, sizeof(HIFB_LAYER_INFO_S));
    /* get the layer info */
    if(ioctl(frameBufFd, FBIOGET_LAYER_INFO, &stLayerInfo) < 0)
    {
        EPRINT(GUI_LIB,"FBIOGET_LAYER_INFO failed!");
    }
    else
    {
        DPRINT(GUI_LIB,"[BufMode=%#x] [eAntiflickerLevel=%#x] [s32XPos=%d] [s32YPos=%d] [u32CanvasWidth=%d] [u32CanvasHeight=%d]",
               stLayerInfo.BufMode, stLayerInfo.eAntiflickerLevel, stLayerInfo.s32XPos, stLayerInfo.s32YPos, stLayerInfo.u32CanvasWidth, stLayerInfo.u32CanvasHeight);
        DPRINT(GUI_LIB,"[u32DisplayWidth=%u] [u32DisplayHeight=%u] [u32ScreenWidth=%u] [u32ScreenHeight=%u] [bPreMul=%u] [u32Mask=%#x]",
               stLayerInfo.u32DisplayWidth, stLayerInfo.u32DisplayHeight, stLayerInfo.u32ScreenWidth, stLayerInfo.u32ScreenHeight, stLayerInfo.bPreMul, stLayerInfo.u32Mask);
    }

    bShow = HI_TRUE;
    if(ioctl(frameBufFd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOPUT_SHOW_HIFB failed!");
        return HI_FAILURE;
    }

    memset(&hifb_alpha, 0, sizeof(HIFB_ALPHA_S));
    if(ioctl(frameBufFd, FBIOGET_ALPHA_HIFB, &hifb_alpha) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_ALPHA_HIFB failed");
        return HI_FAILURE;
    }

    DPRINT(GUI_LIB,"bAlphaEnable %d bAlphaChannel %d u8Alpha0 %#x u8Alpha1 %#x u8GlobalAlpha %#x",
           hifb_alpha.bAlphaEnable, hifb_alpha.bAlphaChannel,
           hifb_alpha.u8Alpha0, hifb_alpha.u8Alpha1, hifb_alpha.u8GlobalAlpha);

    hifb_alpha.bAlphaEnable = HI_TRUE;
    hifb_alpha.u8Alpha0 = 0;
    hifb_alpha.u8Alpha1 = 0xff;
    if(ioctl(frameBufFd, FBIOPUT_ALPHA_HIFB, &hifb_alpha) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOPUT_ALPHA_HIFB failed");
        return HI_FAILURE;
    }

    memset(&hifb_alpha, 0, sizeof(HIFB_ALPHA_S));
    if(ioctl(frameBufFd, FBIOGET_ALPHA_HIFB, &hifb_alpha) < 0)
    {
        close(frameBufFd);
        frameBufFd = -1;
        EPRINT(GUI_LIB,"FBIOGET_ALPHA_HIFB failed");
        return HI_FAILURE;
    }

    DPRINT(GUI_LIB,"bAlphaEnable %d bAlphaChannel %d u8Alpha0 %#x u8Alpha1 %#x u8GlobalAlpha %#x",
           hifb_alpha.bAlphaEnable, hifb_alpha.bAlphaChannel,
           hifb_alpha.u8Alpha0, hifb_alpha.u8Alpha1, hifb_alpha.u8GlobalAlpha);

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   calculate the actual screen height width in multiple of 8.
 * @param   dispDevId
 * @param   validScreen
 * @param   windLayoutId
 * @return
 */
BOOL GetValidScreenInfoPlayBack(DISPLAY_DEV_e dispDevId, VALID_SCREEN_INFO_t *validScreen, WIND_LAYOUT_ID_e windLayoutId)
{
    UINT32                  width,height;
    UINT32                  actualWidth,actualHeight;
    UINT32                  scrnWidth,scrnHeight;
    UINT32                  multiple;
    DISPLAY_RESOLUTION_e    dispResolution;

    if(NULL == validScreen)
    {
        return FAIL;
    }

    dispResolution = decDispLib.displayInfo.dispResolution;
    switch(dispResolution)
    {
        case DISPLAY_RESOLUTION_720P:
            scrnWidth = 1280;
            scrnHeight = 720;
            break;

        case DISPLAY_RESOLUTION_1080P:
            scrnWidth = 1920;
            scrnHeight = 1080;
            break;

        case DISPLAY_RESOLUTION_2160P:
            scrnWidth = 3840;
            scrnHeight = 2160;
            break;

        default:
            scrnWidth = 0;
            scrnHeight = 0;
            break;
    }

    /* Validate window layout */
    if (windLayoutId >= WIND_LAYOUT_MAX)
    {
        DPRINT(GUI_LIB,"Invld WindowLayout[%d] found. Fallback to default", windLayoutId);
        windLayoutId = WIND_LAYOUT_2X2_4CH;
    }

    multiple = winMultiple[windLayoutId];

    /* If the TV offset changes, the scrnWidth and scrnHeight value changes accordingly */
    scrnWidth = ((scrnWidth) - (LIVE_VIEW_WIDTH_OFFSET * (tvAdjustOffset)));
    scrnHeight = ((scrnHeight) - (LIVE_VIEW_HEIGHT_OFFSET * (tvAdjustOffset)) + (tvAdjustOffset));
    // Additional (tvAdjustOffset) offset is to lessen space between layout and sync PB toolbar

    /* As the timeline is considered for 1080p resolution, hence for other resolution scrnHeight and scrnWidth divided by 1080p height and width resp.*/
    actualWidth = scrnWidth - (SYNCPB_TIMELINE_WIDTH_1080P * scrnWidth/1920);
    actualHeight = scrnHeight - (SYNCPB_TIMELINE_HEIGHT_1080P * scrnHeight/1080);

    validScreen->actualStartX = (LIVE_VIEW_STARTX) * (tvAdjustOffset);
    validScreen->actualStartY = (LIVE_VIEW_STARTY) * (tvAdjustOffset);
    validScreen->actualWidth =  actualWidth;
    validScreen->actualHeight = actualHeight;

    //width = (validScreen->actualHeight % multiple);
    if((validScreen->actualWidth % 8) != 0)
    {
        validScreen->actualWidth -= (validScreen->actualWidth % 8);
    }
    width = (validScreen->actualWidth / multiple);
    if(width %2 != 0)
    {
        width -= 1;
    }
    validScreen->actualWidth = width * multiple;

    if((validScreen->actualHeight % 8) != 0)
    {
        validScreen->actualHeight -= (validScreen->actualHeight % 8);
    }
    height = (validScreen->actualHeight / multiple);
    if(height %2 != 0)
    {
        height -= 1;
    }
    validScreen->actualHeight = height * multiple;

    DPRINT(GUI_LIB,"[dispResolution=%d] [windLayoutId=%d] [tvAdjustOffset=%d] [actualStartX=%u] [actualStartY=%u] [actualW=%u] [actualH=%u]",
           dispResolution,windLayoutId,tvAdjustOffset, validScreen->actualStartX, validScreen->actualStartY, validScreen->actualWidth, validScreen->actualHeight);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   calculate the actual screen height width in multiple of 8.
 * @param   dispDevId
 * @param   validScreen
 * @param   windLayoutId
 * @return
 */
BOOL GetValidScreenInfo(DISPLAY_DEV_e dispDevId, VALID_SCREEN_INFO_t *validScreen, WIND_LAYOUT_ID_e windLayoutId)
{
    UINT32                  width,height;
    UINT32                  scrnWidth,scrnHeight;
    UINT32                  multiple = 0;
    DISPLAY_RESOLUTION_e    dispResolution;

    if(NULL == validScreen)
    {
        return FAIL;
    }

    dispResolution = decDispLib.displayInfo.dispResolution;
    switch(dispResolution)
    {
        case DISPLAY_RESOLUTION_720P:
            scrnWidth = 1280;
            scrnHeight = 720;
            break;

        case DISPLAY_RESOLUTION_1080P:
            scrnWidth = 1920;
            scrnHeight = 1080;
            break;

        case DISPLAY_RESOLUTION_2160P:
            scrnWidth = 3840;
            scrnHeight = 2160;
            break;

        default:
            scrnWidth = 0;
            scrnHeight = 0;
            break;
    }

    /* Validate window layout */
    if ((windLayoutId > WIND_LAYOUT_MAX) || (windLayoutId < WIND_LAYOUT_1X1_1CH))
    {
        EPRINT(GUI_LIB," Invld WindowLayout[%d] found. Fallback to default [4X4_16CH]", windLayoutId);
        windLayoutId = WIND_LAYOUT_4X4_16CH;
    }

    if((windLayoutId >= 0) && (windLayoutId < WIND_LAYOUT_MAX))
    {
        multiple = winMultiple[windLayoutId];
    }

    DPRINT(GUI_LIB,"[WindowLayout=%d], [multiple=%d] [tvAdjustOffset=%d]", windLayoutId, multiple, tvAdjustOffset);

    validScreen->actualStartX = (LIVE_VIEW_STARTX) * (tvAdjustOffset);
    validScreen->actualStartY = (LIVE_VIEW_STARTY) * (tvAdjustOffset);
    validScreen->actualWidth = ((scrnWidth) - (LIVE_VIEW_WIDTH_OFFSET * (tvAdjustOffset)));
    validScreen->actualHeight = ((scrnHeight) - (LIVE_VIEW_HEIGHT_OFFSET * (tvAdjustOffset)));

    if((validScreen->actualWidth % 8) != 0)
    {
        validScreen->actualWidth -= (validScreen->actualWidth % 8);
    }

    if (multiple)
    {
        width = (validScreen->actualWidth / multiple);
        if(width % 2 != 0)
        {
            width -= 1;
        }
        validScreen->actualWidth = width * multiple;
    }

    if((validScreen->actualHeight % 8) != 0) //((validScreen->actualHeight % 8) != 0)
    {
        validScreen->actualHeight -= (validScreen->actualHeight % 8);
    }

    if (multiple)
    {
        height = (validScreen->actualHeight / multiple);
        if(height % 2 != 0)
        {
            height -= 1;
        }
        validScreen->actualHeight = height * multiple;
    }

    DPRINT(GUI_LIB,"[dispResolution=%d] [windLayoutId=%d] [tvAdjustOffset=%d] [actualStartX=%u] [actualStartY=%u] [actualW=%u] [actualH=%u]",
           dispResolution,windLayoutId,tvAdjustOffset, validScreen->actualStartX, validScreen->actualStartY, validScreen->actualWidth, validScreen->actualHeight);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   change the state of any channel on any display device .
 * @param   decDispId
 * @param   dispDevId
 * @param   windId
 * @return
 */
BOOL UpdateDecoderStatusOnVideoLoss(DEC_DISP_PLAY_ID decDispId, DISPLAY_DEV_e dispDevId, UINT8 windId)
{
    if((dispDevId < MAX_DISPLAY_DEV) && (decDispId < MAX_DEC_DISP_CHN))
    {
        GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);
        decDispList[decDispId].iFrameRecv = FALSE;
        GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
    }
    else
    {
        EPRINT(GUI_LIB,"[dispDevId=%d] Error: invalid dispDevId",decDispId);
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function Sets the the attributes of a VO channel According to selected  WINDOW_LAYOUT.
 * @param   windLayoutId
 * @param   stLayerAttr
 * @param   winGeometry
 * @return
 */
static HI_S8 generateWindowGeometry(WIND_LAYOUT_ID_e windLayoutId, VO_VIDEO_LAYER_ATTR_S stLayerAttr, WinGeometryParam_t *winGeometry)
{
    HI_U32              scrnWidth=0,scrnHeight=0,winId=0;
    HI_U32              colMax=0,rowMax=0,row=0,col=0,widthAlign=8, heightAlign=1;
    HI_U32              xWidth,yHeight;
    HI_BOOL             status = HI_FALSE;
    VALID_SCREEN_INFO_t screenInfo;

    if(windLayoutId >= WIND_LAYOUT_1X1_PLAYBACK)
    {
        status = GetValidScreenInfoPlayBack(MAIN_VIDEO_DISPLAY, &screenInfo, windLayoutId);
    }
    else
    {
        status = GetValidScreenInfo(MAIN_VIDEO_DISPLAY, &screenInfo, windLayoutId);
    }

    if (status == HI_TRUE)
    {
        scrnWidth = screenInfo.actualWidth;
        scrnHeight = screenInfo.actualHeight;
        stLayerAttr.stDispRect.s32X = screenInfo.actualStartX;
        stLayerAttr.stDispRect.s32Y = screenInfo.actualStartY;
    }

    DPRINT(GUI_LIB,"[Width=%d] [Height=%d] [s32x=%d] [s32y=%d] [windLayout=%d]",
           scrnWidth, scrnHeight, stLayerAttr.stDispRect.s32X, stLayerAttr.stDispRect.s32Y, windLayoutId);

    widthAlign = 2;
    heightAlign = 2;

    switch (windLayoutId)
    {
        case WIND_LAYOUT_1X5_6CH:
            rowMax = 2;
            colMax = 3;
            break;

        case WIND_LAYOUT_1X9_10CH:
            rowMax = 4;
            colMax = 5;
            break;

        case WIND_LAYOUT_1X1_1CH:
            colMax = 1;
            rowMax = 1;
            break;

        case WIND_LAYOUT_2X2_4CH:
            colMax = 2;
            rowMax = 2;
            break;

        case WIND_LAYOUT_3X3_9CH:
            colMax = 3;
            rowMax = 3;
            break;

        case WIND_LAYOUT_4X4_16CH:
            colMax = 4;
            rowMax = 4;
            break;

        case WIND_LAYOUT_5x5_25CH:
            colMax = 5;
            rowMax = 5;
            break;

        case WIND_LAYOUT_6x6_36CH:
            colMax = 6;
            rowMax = 6;
            break;

        case WIND_LAYOUT_8x8_64CH:
            colMax = 8;
            rowMax = 8;
            break;

        case WIND_LAYOUT_1X1_PLAYBACK:
            colMax = 1;
            rowMax = 1;
            break;

        case WIND_LAYOUT_2X2_PLAYBACK:
            colMax = 2;
            rowMax = 2;
            break;

        case WIND_LAYOUT_3X3_PLAYBACK:
            colMax = 3;
            rowMax = 3;
            break;

        case WIND_LAYOUT_4X4_PLAYBACK:
            colMax = 4;
            rowMax = 4;
            break;

        default:
            DPRINT(GUI_LIB,"[windLayoutId=%d] Default case",windLayoutId);
            break;
    }

    if (windLayoutId < WIND_LAYOUT_MAX)
    {
        winGeometry->totalNumberWin = getTotalWindOnLayout(windLayoutId);
    }
    else
    {
        EPRINT(GUI_LIB,"[windLayoutId=%d] Error: Invalid windLayoutId",windLayoutId);
        return HI_FAILURE;
    }

    switch (windLayoutId)
    {
        case WIND_LAYOUT_1X1_1CH:
        case WIND_LAYOUT_2X2_4CH:
        case WIND_LAYOUT_3X3_9CH:
        case WIND_LAYOUT_4X4_16CH:
        case WIND_LAYOUT_5x5_25CH:
        case WIND_LAYOUT_6x6_36CH:
        case WIND_LAYOUT_8x8_64CH:
            if((rowMax > 0) && (colMax > 0) && (colMax == rowMax))
            {
                winGeometry->totalNumberWin = (rowMax*colMax);
                for(row=0; row<rowMax; row++)
                {
                    for(col=0; col<colMax; col++)
                    {
                        winId = row*colMax+col;
                        winGeometry->winWidth[winId]  = scrnWidth/colMax;
                        winGeometry->winHeight[winId] = scrnHeight/rowMax;
                        winGeometry->start_X[winId] = winGeometry->winWidth[winId]*col + stLayerAttr.stDispRect.s32X;
                        winGeometry->start_Y[winId] = winGeometry->winHeight[winId]*row + stLayerAttr.stDispRect.s32Y;
                    }
                }
            }
            break;

        case WIND_LAYOUT_1X5_6CH:
            winId = 0;
            winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth*rowMax)/colMax, widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight*rowMax)/colMax, heightAlign);
            winGeometry->start_X[winId] = stLayerAttr.stDispRect.s32X;
            winGeometry->start_Y[winId] = stLayerAttr.stDispRect.s32Y;

            for(row=0; row<rowMax; row++)
            {
                winId++;
                winGeometry->winWidth[winId] = ALIGN_BACK(winGeometry->winWidth[0]/2,widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(winGeometry->winHeight[0]/2,heightAlign);
                winGeometry->start_X[winId] = (winGeometry->winWidth[0] + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (winGeometry->winHeight[winId]*row + stLayerAttr.stDispRect.s32Y);
            }

            for(col=0; col<colMax; col++)
            {
                winId++;
                winGeometry->winWidth[winId] = ALIGN_BACK(winGeometry->winWidth[0]/2,widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(winGeometry->winHeight[0]/2,heightAlign);
                winGeometry->start_X[winId] = (winGeometry->winWidth[winId]*col + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (winGeometry->winHeight[0] + stLayerAttr.stDispRect.s32Y);
            }
            break;

        case WIND_LAYOUT_1X7_8CH:
            winId = 0;
            winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/4)*3, widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/4)*3, heightAlign);
            winGeometry->start_X[winId] = stLayerAttr.stDispRect.s32X;
            winGeometry->start_Y[winId] = stLayerAttr.stDispRect.s32Y;

            for(row=0; row<3; row++)
            {
                winId++;
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/4, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(winGeometry->winHeight[0]/3, heightAlign);
                winGeometry->start_X[winId] = (winGeometry->winWidth[0] + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (winGeometry->winHeight[winId]*row + stLayerAttr.stDispRect.s32Y);
            }

            for(col=0; col<4; col++)
            {
                winId++;
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/4, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(scrnHeight/4, heightAlign);
                winGeometry->start_X[winId] = ((winGeometry->winWidth[winId]*col) + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (winGeometry->winHeight[0] + stLayerAttr.stDispRect.s32Y);
            }
            break;

        case WIND_LAYOUT_1X9_10CH:
            winId = 0;
            winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/5)*4, widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/5)*4, heightAlign);
            winGeometry->start_X[winId] = stLayerAttr.stDispRect.s32X;
            winGeometry->start_Y[winId] = stLayerAttr.stDispRect.s32Y;

            for(row=0; row<4; row++)
            {
                winId++;
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/5, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(winGeometry->winHeight[0]/4, heightAlign);
                winGeometry->start_X[winId] = (winGeometry->winWidth[0] + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId]*row) + stLayerAttr.stDispRect.s32Y);
            }

            for(col=0; col<5; col++)
            {
                winId++;
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/5, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(scrnHeight/5, heightAlign);
                winGeometry->start_X[winId] = ((winGeometry->winWidth[winId]*col) + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (winGeometry->winHeight[0] + stLayerAttr.stDispRect.s32Y);
            }
            break;

        case WIND_LAYOUT_3X4_7CH:
            winId = 0;
            for(col=0; col<2; col++)
            {
                for(row=0; row<2; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/2), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/2), heightAlign);
                    winGeometry->start_X[winId] = (((scrnWidth/2) * row) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = (((scrnHeight/2) * col) + stLayerAttr.stDispRect.s32Y);
                    winId++;
                }
            }

            winId --;
            colMax = 4;
            rowMax = 4;
            for(col=0; col<2; col++)
            {
                for(row=0; row<2; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/colMax), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/rowMax), heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId] * (row + 2)) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId] * (col + 2)) + stLayerAttr.stDispRect.s32Y);
                    winId++;
                }
            }
            break;

        case WIND_LAYOUT_2X8_10CH:
            winId = 0;
            for(row=0; row<2; row++)
            {
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/2, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(scrnHeight/2, heightAlign);
                winGeometry->start_X[winId] = (((scrnWidth/2) * row) + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = stLayerAttr.stDispRect.s32Y;
                winId++;
            }
            rowMax = 4;
            colMax = 4;
            for(col=0; col<2; col++)
            {
                for(row=0; row<rowMax; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/colMax),widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/rowMax),heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId] * row) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId]*(col + 2)) + stLayerAttr.stDispRect.s32Y);
                    winId++;
                }
            }
            break;

        case WIND_LAYOUT_1X12_13CH:
            winId = 0;
            winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth /2), widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/2), heightAlign);
            winGeometry->start_X[winId] = stLayerAttr.stDispRect.s32X;
            winGeometry->start_Y[winId] = stLayerAttr.stDispRect.s32Y;

            rowMax = 2;
            colMax = 2;
            for(col=0; col<colMax; col++)
            {
                for(row=0; row<rowMax; row++)
                {
                    winId++;

                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/4), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/4), heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId] * (row + 2)) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId] * col) + stLayerAttr.stDispRect.s32Y);
                }
            }

            rowMax = 4;
            colMax = 4;
            for(col=0; col<2; col++)
            {
                for(row=0; row<rowMax; row++)
                {
                    winId++;
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/colMax), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/rowMax), heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId] * row) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId]*(col + 2)) + stLayerAttr.stDispRect.s32Y);
                }
            }
            break;

        case WIND_LAYOUT_1CX12_13CH:
            winId = 5;
            winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/2), widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/2), heightAlign);
            winGeometry->start_X[winId] = ((scrnWidth/4) + stLayerAttr.stDispRect.s32X);
            winGeometry->start_Y[winId] = ((scrnHeight/4) + stLayerAttr.stDispRect.s32Y);

            rowMax = 4;
            colMax = 4;
            winId = 0;
            for(col=0; col<1; col++)
            {
                for(row=0; row<rowMax; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/rowMax), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/colMax), heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId] * row) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId] * col) + stLayerAttr.stDispRect.s32Y);
                    winId++;
                }
            }

            for(col=0; col<2; col++)
            {
                for(row=0; row<2; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/4, widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK(scrnHeight/4, heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId]*(row * 3)) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId]*(col+1)) + stLayerAttr.stDispRect.s32Y);
                    // because we need to skip 5 number channel
                    winId++;
                    if(winId == 5)
                    {
                        winId++;
                    }
                }
            }

            for(col=0; col<1; col++)
            {
                for(row=0; row<rowMax; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/rowMax), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/colMax), heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId] * row) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = ((winGeometry->winHeight[winId] * 3) + stLayerAttr.stDispRect.s32Y);
                    winId++;
                }
            }
            break;

        case WIND_LAYOUT_4X9_13CH:
            winId = 0;
            colMax = 2;
            rowMax = 2;
            for(col=0; col<colMax; col++)
            {
                for(row=0; row<rowMax; row++)
                {
                    winGeometry->winWidth[winId] = ALIGN_BACK(((scrnWidth/5)*2), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK(((scrnHeight/5)*2), heightAlign);
                    winGeometry->start_X[winId] = ((winGeometry->winWidth[winId]*row) + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = (winGeometry->winHeight[winId]*col + stLayerAttr.stDispRect.s32Y);
                    winId++;
                    if(winId == 2)
                    {
                        winId = 4;
                    }
                }
            }

            winId = 2;
            for(row=0; row<4; row++)
            {
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/5, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/5), heightAlign);
                winGeometry->start_X[winId] = (((scrnWidth/5)*4) + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (winGeometry->winHeight[winId]*row + stLayerAttr.stDispRect.s32Y) ;
                winId++;
                if(winId == 4)
                {
                    winId = 6;
                }
            }

            for(col=0; col<5; col++)
            {
                winGeometry->winWidth[winId] = ALIGN_BACK(scrnWidth/5, widthAlign);
                winGeometry->winHeight[winId] = ALIGN_BACK(scrnHeight/5, heightAlign);
                winGeometry->start_X[winId] = (winGeometry->winWidth[winId]*col + stLayerAttr.stDispRect.s32X);
                winGeometry->start_Y[winId] = (((scrnHeight/5)*4) + stLayerAttr.stDispRect.s32Y);
                winId++;
            }
            break;

        case WIND_LAYOUT_2X12_14CH:
            winId = 0;
            winGeometry->winWidth[winId] = ALIGN_BACK(((scrnWidth/5)*3), widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK(((scrnHeight/5)*3), heightAlign);
            winGeometry->start_X[winId] = stLayerAttr.stDispRect.s32X;
            winGeometry->start_Y[winId]  = stLayerAttr.stDispRect.s32Y;
            winId++;

            winGeometry->winWidth[winId] = ALIGN_BACK(((scrnWidth/5)*2), widthAlign);
            winGeometry->winHeight[winId] = ALIGN_BACK(((scrnHeight/5)*2), heightAlign);
            winGeometry->start_X[winId] = (((scrnWidth/5)*3) + stLayerAttr.stDispRect.s32X);
            winGeometry->start_Y[winId]  = stLayerAttr.stDispRect.s32Y;

            colMax = 5;
            rowMax = 5;
            row=3;
            for(col=2; col<colMax; col++)
            {
                for(; row<rowMax; row++)
                {
                    winId++;
                    winGeometry->winWidth[winId] = ALIGN_BACK((scrnWidth/rowMax), widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK((scrnHeight/colMax), heightAlign);
                    winGeometry->start_X[winId] = (winGeometry->winWidth[winId]*row + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId]  = (winGeometry->winHeight[winId]*col + stLayerAttr.stDispRect.s32Y);
                }
                row=0;
            }
            break;

        case WIND_LAYOUT_1X1_PLAYBACK:
            winId = 0;
            winGeometry->start_X[winId] = stLayerAttr.stDispRect.s32X;
            winGeometry->start_Y[winId] = stLayerAttr.stDispRect.s32Y;
            winGeometry->winWidth[winId] = scrnWidth;
            winGeometry->winHeight[winId] = scrnHeight;
            break;

        case WIND_LAYOUT_2X2_PLAYBACK:
        case WIND_LAYOUT_4X4_PLAYBACK:
        case WIND_LAYOUT_3X3_PLAYBACK:
            xWidth = scrnWidth;
            yHeight = scrnHeight;

            if (scrnWidth == 1024)
            {
                xWidth = 848;
            }

            for(row=0; row<rowMax; row++)
            {
                for(col=0; col<colMax; col++)
                {
                    winId = row*colMax+col;
                    winGeometry->winWidth[winId] = ALIGN_BACK(xWidth/colMax, widthAlign);
                    winGeometry->winHeight[winId] = ALIGN_BACK(yHeight/rowMax, heightAlign);
                    winGeometry->start_X[winId] = (winGeometry->winWidth[winId]*col + stLayerAttr.stDispRect.s32X);
                    winGeometry->start_Y[winId] = (winGeometry->winHeight[winId]*row + stLayerAttr.stDispRect.s32Y);
                }
            }
            break;//for generating window geometry according to which the screen will be divided for sync playback

        default:
            EPRINT(GUI_LIB,"[windLayoutId=%d] Error: Invalid windLayoutId",windLayoutId);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Find the free Decoder Instance. This function starts rendering on given
 *          display device and window number.
 * @param   dispDevId
 * @param   windId
 * @param   decDispId
 * @return
 */
BOOL StartChannelView(DISPLAY_DEV_e dispDevId, UINT8 windId, DEC_DISP_PLAY_ID *decDispId)
{
    UINT8			decDisCnt;
    static UINT8    startCnt = 0;
    UINT8 		 	totalSession = 0;

    if (decDispId == NULL)
    {
        EPRINT(GUI_LIB,"Error: NULL Pointer");
        return FAIL;
    }

    if((dispDevId >= MAX_DISPLAY_DEV) || (windId >= MAX_WINDOWS))
    {
        EPRINT(GUI_LIB,"[dispDevId=%d] [windId=%d] Error: Invalid parameters",dispDevId,windId);
        *decDispId = INVALID_DEC_DISP_ID;
        return FAIL;
    }

    updateChn2DecLink(windId);
    // Find out free decoder display position
    GUI_LIB_MUTEX_LOCK(decDispListMutex);
    for (decDisCnt = startCnt;; decDisCnt++)
    {
        decDisCnt = decDisCnt%MAX_DEC_DISP_CHN;
        if (decDispList[decDisCnt].status == FREE)
        {
            decDispList[decDisCnt].status = BUSY;
            startCnt = decDisCnt;
            startCnt++;
            startCnt = startCnt%MAX_DEC_DISP_CHN;
            break;
        }
        else
        {
            startCnt = 0;
        }
        totalSession++;
        if (totalSession%MAX_DEC_DISP_CHN == 0)
        {
            decDisCnt = MAX_DEC_DISP_CHN;
            break;
        }
    }
    GUI_LIB_MUTEX_UNLOCK(decDispListMutex);

    if(decDisCnt >= MAX_DEC_DISP_CHN)
    {
        EPRINT(GUI_LIB,"Error: No Free Decoder for [windId=%d]",windId);
        *decDispId = INVALID_DEC_DISP_ID;
        return FAIL;
    }

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    GUI_LIB_MUTEX_LOCK(decDispList[decDisCnt].decDispParamMutex);
    decDispList[decDisCnt].decoderId = decDisCnt;
    decDispList[decDisCnt].iFrameRecv = FALSE;
    decDispList[decDisCnt].noOfReferanceFrame = 0;
    GUI_LIB_MUTEX_UNLOCK(decDispList[decDisCnt].decDispParamMutex);

    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    decDispLib.ch2WinMap[windId] = decDispList[decDisCnt].decoderId;
    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);
    *decDispId = decDisCnt;

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops video for DEC_DISP_PLAY_ID. This function stops rendering on given
 *          display device and window number And also stops decoding channel.
 * @param   decDispId
 * @return
 */
BOOL StopChannelView(DEC_DISP_PLAY_ID decDispId)
{
    UINT8   decoderId;
    UINT8   windowId;
    UINT16  frameHeight;
    UINT16  frameWidth;
    UINT16  frameRate;

    if (decDispId >= MAX_DEC_DISP_CHN)
    {
        EPRINT(GUI_LIB,"Error: [decDispId=%d] > [MAX_DEC_DISP_CHN=%d]", decDispId, MAX_DEC_DISP_CHN);
        return FAIL;
    }

    GUI_LIB_MUTEX_LOCK(decDispListMutex);

    if (decDispList[decDispId].status == BUSY)
    {
        decDispList[decDispId].status = PENDING;
        GUI_LIB_MUTEX_UNLOCK(decDispListMutex);

        GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);

        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        frameHeight = decDispList[decDispId].frameHeight;
        frameWidth = decDispList[decDispId].frameWidth;
        frameRate = decDispList[decDispId].frameRate;
        decoderId = decDispList[decDispId].decoderId;

        decDispList[decDispId].frameHeight = 0;
        decDispList[decDispId].frameWidth = 0;
        decDispList[decDispId].frameRate = 0;

        decDispList[decDispId].noOfReferanceFrame = 0;
        decDispList[decDispId].iFrameRecv = FALSE;

        GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);

        windowId = getWindowId(decoderId);

        /* Delete Decoder Instant */
        GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
        if ((frameHeight > 0) && (frameWidth > 0))
        {
            deInitDecoderResources(decoderId, windowId);
        }

        decDispLib.ch2WinMap[windowId] = INVALID_DEC_DISP_ID;
        GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

        removeDecoderUsage(frameHeight, frameWidth, frameRate);

        if (audioStream.audioChannel == decoderId)
        {
            ExcludeAudio(decoderId);
        }

        GUI_LIB_MUTEX_LOCK(decDispListMutex);

        decDispList[decDispId].status = FREE;

        GUI_LIB_MUTEX_UNLOCK(decDispListMutex);

        DPRINT(GUI_LIB,"Channel ID stops :[decoderID=%d] [windId=%d] [frameRate: %d]", decDispId, windowId,frameRate);
    }
    else
    {
        GUI_LIB_MUTEX_UNLOCK(decDispListMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is to validate decoder from sync Pb.
 * @param   decId
 * @return
 */
BOOL ValidateDecoderSyncPb(INT8 decId)
{
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;
    HI_S32      s32Ret = HI_FAILURE;
    UINT8       tIndex = 0;
    UINT8       tMaxCh = 0;

	switch(decDispLib.displayInfo.windowLayout)
	{
        default:
            return (SUCCESS);

        case WIND_LAYOUT_1X1_1CH:
        case WIND_LAYOUT_1X1_PLAYBACK:
            tMaxCh = 1;
            break;

        case WIND_LAYOUT_2X2_4CH:
        case WIND_LAYOUT_2X2_PLAYBACK:
            tMaxCh = 4;
            break;

        case WIND_LAYOUT_3X3_9CH:
        case WIND_LAYOUT_3X3_PLAYBACK:
            tMaxCh = 9;
            break;
        case WIND_LAYOUT_4X4_16CH:
        case WIND_LAYOUT_4X4_PLAYBACK:
            tMaxCh = 16;
            break;
	}

	for(tIndex = 0; tIndex < tMaxCh ; tIndex++)
	{
		stDestChn.enModId  = HI_ID_VOU;
		stDestChn.s32DevId = VO_LAYER_ID;
		stDestChn.s32ChnId = tIndex;

		/* Obtains the information about a bound data source */
		if((s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn)) == HI_SUCCESS)
		{
			/* Match DecoderId which is visible in current layout by
			 * checking the associated VO channel */
			if(decId == stSrcChn.s32DevId)
			{
				return (SUCCESS);
			}
		}
	}
	return (FAIL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function use to store frame into decoder buffer for decoding and display. Defines
 *          the attributes of a VDEC channel , Starts to receive the streams sent  by the user,
 *          Creates a  VPSS group, Sets the parameters of a VPSS group, Enables a VPSS channel,
 *          Start a VPSS group
 * @param   decDispId
 * @param   frameInfo
 * @param   errorCode
 * @return
 */
BOOL DecodeDispFrame(DEC_DISP_PLAY_ID decDispId, FRAME_INFO_t * frameInfo,DECODER_ERROR_e* errorCode)
{
    HI_S32                      s32Ret = HI_SUCCESS;
    BOOL                        frameFeedFlg = TRUE;
    BOOL                        iFrameRecvF = FALSE;
    UINT8                       decoderId = MAX_DEC_DISP_CHN;
    UINT8                       windowId, winId;
    UINT16                      frameWidth, frameHeight, noOfReferanceFrame, frameRate;
    PAYLOAD_TYPE_E              vidCodec;
    VDEC_CHN_ATTR_S             stVdecChnAttr;
    VDEC_STREAM_S               stStream;
    MPP_CHN_S                   stSrcChn;
    MPP_CHN_S                   stDestChn;
    SIZE_S                      resolution;
    DECODER_CREATION_STATE_e    currState = MAX_STATE;
    UINT32                      tmp, tmpFrameLeght,tmp2;
    INT32                       wrIndex;
    VIDEO_DISPLAY_MODE_E        enDispMode;
    VO_CHN_ATTR_S               voChnAttr;
    float                       ratioHeight, ratioWidth;

    if (errorCode == NULL)
    {
        EPRINT(GUI_LIB,"[decDispId=%d] Error: Invalid argument, nocapcity is null", decDispId);
        return FAIL;
    }

    if (frameInfo->frameSize == 0)
    {
        EPRINT(GUI_LIB,"[decDispId=%d] Error: Frame Size Zero", decDispId);
        *errorCode = DEC_ERROR_FRAME_SIZE;
        return FAIL;
    }

    if (frameInfo->mediaType == STREAM_TYPE_VIDEO)
    {
        if (frameInfo->codecType >= MAX_VIDEO_CODEC)
        {
            EPRINT(GUI_LIB,"[decDispId=%d] Error: Invalid Codec [codec=%d]", decDispId, frameInfo->codecType);
            *errorCode = DEC_ERROR_INVALID_CODEC;
            return FAIL;
        }

        if (decDispId >= MAX_DEC_DISP_CHN)
        {
            EPRINT(GUI_LIB,"[decDispId=%d] Error: Invalid decDispId ", decDispId);
            *errorCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        //check for codec change
        GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);
        iFrameRecvF = decDispList[decDispId].iFrameRecv;
        decoderId = decDispList[decDispId].decoderId;
        if (decoderId >= MAX_DEC_DISP_CHN)
        {
            /* Update decoder capacity and make frame resolution and framerate to 0. So next time not delete it */
            removeDecoderUsage(decDispList[decDispId].frameHeight, decDispList[decDispId].frameWidth, decDispList[decDispId].frameRate);
            decDispList[decDispId].frameHeight = 0;
            decDispList[decDispId].frameWidth = 0;
            decDispList[decDispId].frameRate = 0;
            GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
            EPRINT(GUI_LIB,"[DecoderId=%d] Error: Invalid Decoder Id", decoderId);
            *errorCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        if((decDispList[decDispId].vidCodec != hiVidCodecMap[frameInfo->codecType])
                || (decDispList[decDispId].noOfReferanceFrame != frameInfo->noOfReferanceFrame)
                || (decDispList[decDispId].frameHeight != frameInfo->frameHeight)
                || (decDispList[decDispId].frameWidth != frameInfo->frameWidth)
                || (decDispList[decDispId].frameRate != frameInfo->frameRate))
        {
            windowId = getWindowId(decoderId);
            frameWidth = decDispList[decDispId].frameWidth;
            frameHeight = decDispList[decDispId].frameHeight;
            frameRate = decDispList[decDispId].frameRate;
            vidCodec = decDispList[decDispId].vidCodec;
            noOfReferanceFrame = decDispList[decDispId].noOfReferanceFrame;
            GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);

            DPRINT(GUI_LIB, "frame parameter updated: [decoderId=%d], [resolution=%dx%d->%dx%d], [reference=%d->%d], [fps=%d->%d]",
                   decoderId, frameWidth, frameHeight, frameInfo->frameWidth, frameInfo->frameHeight,
                   noOfReferanceFrame, frameInfo->noOfReferanceFrame, frameRate, frameInfo->frameRate);

            if (FAIL == addDecoderUsage(decoderId, frameHeight, frameWidth, frameInfo->frameHeight, frameInfo->frameWidth, frameRate, frameInfo->frameRate))
            {
                // Delete Decoder Instant
                EPRINT(GUI_LIB, "no more decoding capacity: [decoderId=%d]", decoderId);
                if (((frameHeight > 0) && (frameWidth > 0)) || (noOfReferanceFrame > 0))
                {
                    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
                    deInitDecoderResources(decoderId, windowId);
                    decDispLib.ch2WinMap[windowId] = decoderId;
                    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);
                }

                /* Update decoder capacity and make frame resolution and framerate to 0. So next time not delete it */
                removeDecoderUsage(frameHeight, frameWidth, frameRate);
                GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);
                decDispList[decDispId].frameHeight = 0;
                decDispList[decDispId].frameWidth = 0;
                decDispList[decDispId].frameRate  = 0;
                GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
                *errorCode = DEC_ERROR_NO_CAPACITY;
                return FAIL;
            }

            if ((vidCodec != hiVidCodecMap[frameInfo->codecType]) || (noOfReferanceFrame != frameInfo->noOfReferanceFrame)
                    || (frameHeight != frameInfo->frameHeight) || (frameWidth != frameInfo->frameWidth))
            {
                // Delete Decoder Instant
                if (((frameHeight > 0) && (frameWidth > 0)) || (noOfReferanceFrame > 0))
                {
                    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
                    deInitDecoderResources(decoderId, windowId);
                    decDispLib.ch2WinMap[windowId] = decoderId;
                    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);
                }

                noOfReferanceFrame = (frameInfo->noOfReferanceFrame);
                stVdecChnAttr.enType       = hiVidCodecMap[frameInfo->codecType]; /* video type to be decoded */
                stVdecChnAttr.u32BufSize   = (3 * frameInfo->frameWidth * frameInfo->frameHeight) >> 1;/* stream buf size(Byte) */
                stVdecChnAttr.u32Priority  = 5;/* priority */
                stVdecChnAttr.u32PicWidth  = frameInfo->frameWidth;/* max pic width */
                stVdecChnAttr.u32PicHeight = frameInfo->frameHeight;/* max pic height */

                switch(stVdecChnAttr.enType)
                {
                    case PT_MP4VIDEO:
                        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;/*ref pic num [1,16]*/
                        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;/*send by stream or by frame*/

                        /* specifies whether temporal motion vector predictors can be used for inter prediction*/
                        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
                        break;

                    case PT_H264:
                        /* enMode should be FRAME only.. in case of enMode as STREAM MODE live stream is not smooth. */
                        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = noOfReferanceFrame;/*ref pic num [1,16]*/
                        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;/*send by stream or by frame*/
                        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
                        break;

                    case PT_H265:
                        /* enMode should be VIDEO_MODE_STREAM only..in case of enMode as
                         * VIDEO_MODE_FRAME live stream is not smooth, check decoder logs. */
                        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;/*ref pic num [1,16]*/
                        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_STREAM;/*send by stream or by frame*/

                        /* specifies whether temporal motion vector predictors can be used for inter prediction*/
                        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 1;
                        break;

                    case PT_JPEG:
                    case PT_MJPEG:
                        stVdecChnAttr.stVdecJpegAttr.enMode = VIDEO_MODE_FRAME;/*send by stream or by frame*/
                        /* jpeg format select ,may be YUV420,YUV400,YUV422,YUV444*/
                        stVdecChnAttr.stVdecJpegAttr.enJpegFormat = JPG_COLOR_FMT_YCBCR420;
                        break;

                    default:
                        EPRINT(GUI_LIB, "invld video codec type: [codec=%d]", frameInfo->codecType);
                        *errorCode = DEC_ERROR_INVALID_ARG;
                        return FAIL;
                }

                GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
                resolution.u32Width = frameInfo->frameWidth;
                resolution.u32Height = frameInfo->frameHeight;
                stDestChn.enModId = HI_ID_VOU;
                stDestChn.s32DevId = VO_LAYER_ID;
                stDestChn.s32ChnId = windowId;

                enDispMode = VIDEO_DISPLAY_MODE_PREVIEW;

                /* Obtains the information about a bound data source.*/
                if((s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn)) == HI_SUCCESS)
                {
                    if((s32Ret = unBindVpssVo(VO_LAYER_ID, windowId, stSrcChn.s32DevId, VPSS_CHN0)) != HI_SUCCESS)
                    {
                        API_FAIL_DEBUG("unBindVpssVo",s32Ret);
                    }
                }

                /*Disables a specified VO channel*/
                if((s32Ret = HI_MPI_VO_DisableChn(VO_LAYER_ID, windowId)) != HI_SUCCESS)
                {
                    API_FAIL_DEBUG("HI_MPI_VO_DisableChn", s32Ret);
                }

                BOOL retVal = SUCCESS;
                do
                {
                    if((s32Ret = createDecChn(decoderId, &stVdecChnAttr)) != HI_SUCCESS)
                    {
                        if (s32Ret == HI_ERR_VDEC_EXIST)
                        {
                            currState = DECODER_CREATION;
                        }
                        API_FAIL_DEBUG("createDecChn",s32Ret);
                        retVal = FAIL;
                        break;
                    }

                    /*Enables a specified VO channel*/
                    if((s32Ret = HI_MPI_VO_EnableChn(VO_LAYER_ID, windowId)) != HI_SUCCESS)
                    {
                        currState = VO_CHN_ENABLE;
                        API_FAIL_DEBUG("HI_MPI_VO_EnableChn",s32Ret);
                        retVal = FAIL;
                        break;
                    }

                    if((s32Ret = createVpssGrp(decoderId, 1, &resolution)) != HI_SUCCESS)
                    {
                        currState = VPSS_CREATION;
                        API_FAIL_DEBUG("createVpssGrp",s32Ret);
                        retVal = FAIL;
                        break;
                    }

                    if((s32Ret = bindVdecVpss(decoderId, decoderId)) != HI_SUCCESS)
                    {
                        currState = BIND_VDEC_VPSS;
                        API_FAIL_DEBUG("bindVdecVpss",s32Ret);
                        retVal = FAIL;
                        break;
                    }

                    if((s32Ret = bindVpssVo(VO_LAYER_ID, windowId, decoderId, VPSS_CHN0)) != HI_SUCCESS)
                    {
                        currState = BIND_VPSS_VO;
                        API_FAIL_DEBUG("bindVpssVo",s32Ret);
                        retVal = FAIL;
                        break;
                    }

                    /* Sets the display mode*/
                    if((s32Ret = HI_MPI_VDEC_SetDisplayMode(decoderId, enDispMode)) != HI_SUCCESS)
                    {
                        API_FAIL_DEBUG("HI_MPI_VDEC_SetDisplayMode", s32Ret);
                    }

                }while(0);

                GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

                if (retVal == FAIL)
                {
                    GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);
                    clearPrevDecResources(currState, decoderId, windowId);
                    decDispList[decDispId].frameHeight = 0;
                    decDispList[decDispId].frameWidth = 0;
                    decDispList[decDispId].frameRate = 0;
                    GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
                    removeDecoderUsage(frameInfo->frameHeight, frameInfo->frameWidth, frameInfo->frameRate);
                    *errorCode = DEC_ERROR_CREATION_FAILED;
                    return FAIL;
                }
            }

            GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);
            decDispList[decDispId].vidCodec = hiVidCodecMap[frameInfo->codecType];
            decDispList[decDispId].frameHeight = frameInfo->frameHeight;
            decDispList[decDispId].frameWidth = frameInfo->frameWidth;
            decDispList[decDispId].frameRate  = frameInfo->frameRate;
            decDispList[decDispId].noOfReferanceFrame = frameInfo->noOfReferanceFrame;
            decDispList[decDispId].iFrameRecv = FALSE;
            GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
        }
        else
        {
            GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
        }

        if ((frameInfo->codecType == VIDEO_H264) || (frameInfo->codecType == VIDEO_MPEG4) || ((frameInfo->codecType == VIDEO_H265)))
        {
            if (frameInfo->frameType < MAX_ITYPE)
            {
                if(iFrameRecvF == FALSE)
                {
                    if (frameInfo->frameType == I_FRAME)
                    {
                        GUI_LIB_MUTEX_LOCK(decDispList[decDispId].decDispParamMutex);
                        decDispList[decDispId].iFrameRecv = TRUE;
                        GUI_LIB_MUTEX_UNLOCK(decDispList[decDispId].decDispParamMutex);
                    }
                    else
                    {
                        frameFeedFlg = FALSE;
                    }
                }
            }
            else
            {
                frameFeedFlg = FALSE;
            }
        }

        winId = getWindowId(decoderId);
        if (winId != INVALID_DEC_DISP_ID)
        {
            memset(&voChnAttr, 0, sizeof(VO_CHN_ATTR_S));
            /*Obtains the attributes of a specified VO channel. */
            s32Ret = HI_MPI_VO_GetChnAttr(VO_LAYER_ID, winId, &voChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VO_GetChnAttr", s32Ret);
                *errorCode = DEC_ERROR_VO;
                return FAIL;
            }

            if (frameInfo->frameWidth > voChnAttr.stRect.u32Width)
            {
                ratioWidth = ((float)frameInfo->frameWidth) /(float) voChnAttr.stRect.u32Width;
            }
            else
            {
                ratioWidth = (float)voChnAttr.stRect.u32Width / ((float)frameInfo->frameWidth);
            }

            if (frameInfo->frameHeight > voChnAttr.stRect.u32Height)
            {
                ratioHeight = ((float)frameInfo->frameHeight) / (float)voChnAttr.stRect.u32Height;
            }
            else
            {
                ratioHeight = (float)voChnAttr.stRect.u32Height / ((float)frameInfo->frameHeight);
            }

            if ((ratioWidth > MAX_VPSS_SCALE_SUPPORTED) || (ratioHeight > MAX_VPSS_SCALE_SUPPORTED))
            {
                /* As In Hi3536 The VPSS supports at most 16x zoom in and 15x zoom out.
                The width of the VPSS output picture after zoom-in cannot exceed 4096. */
                EPRINT(GUI_LIB,"VPSS Unable to zoom [ratioWidth=%f] [ratioHeight=%f]", ratioWidth, ratioHeight);
                *errorCode = DEC_ERROR_VPSS_SCALING;
                return FAIL;
            }
        }

        if (frameFeedFlg == TRUE)
        {
            //u64PTS =0 then decoder will control FPS
            stStream.u64PTS  = (((HI_U64)(frameInfo->frameTimeSec)) * 1000 * 1000) + (((HI_U64)(frameInfo->frameTimeMSec)) * 1000); /* time stamp */
            stStream.pu8Addr = (HI_U8*)frameInfo->framePayload;/* stream address */
            stStream.u32Len  = frameInfo->frameSize;/* stream len */
            stStream.bEndOfFrame  = HI_TRUE;/* is the end of a frame */
            stStream.bEndOfStream = HI_FALSE;/* is the end of all stream */
            /* SEND_STREAM_MODE: -1 is block,0 is no block,other positive number is timeout. Sends streams to a VDEC channel. */
            s32Ret = HI_MPI_VDEC_SendStream(decoderId, &stStream, SEND_STREAM_MODE);
            if (s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VDEC_SendStream",s32Ret);
                *errorCode = DEC_ERROR_VDEC;
                return FAIL;
            }
        }
    }
    else if(frameInfo->mediaType == STREAM_TYPE_AUDIO)
    {
        if (frameInfo->codecType >= MAX_AUDIO_CODEC)
        {
            EPRINT(GUI_LIB,"[codec=%d] Error: Invalid Audio Codec", frameInfo->codecType);
            *errorCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        if ((frameInfo->codecType != AUDIO_G711_ULAW) && (frameInfo->codecType != AUDIO_G711_ALAW))
        {
            /* We support only alaw and ulaw codec in local play */
            return SUCCESS;
        }

        if ((decDispId >= MAX_DEC_DISP_CHN) && (decDispId != AUDIO_OUT_DEC_CHNL_ID))
        {
            EPRINT(GUI_LIB,"[decoder=%d] Invalid Audio Decode Id ", decDispId);
            *errorCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
        if (audioStream.audioChannel != decDispId)
        {
            GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
            return SUCCESS;
        }

        if (audioOutInitStatus == FALSE)
        {
            setEnPayloadType(frameInfo->codecType);
            startAudioOut();
            audioOutInitStatus = TRUE;
        }
        GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);

        wrIndex = audioStream.wrPtr;
        tmpFrameLeght = (frameInfo->frameSize);
        tmp = (wrIndex + tmpFrameLeght);

        if (tmp > MAX_AUDIO_BUFFER_SIZE)
        {
            // If at End of audioBuffer, remaning size less than Frame Size
            tmp2 = (MAX_AUDIO_BUFFER_SIZE - wrIndex);
            memcpy((audioBuffer + wrIndex), (frameInfo->framePayload), tmp2);
            wrIndex = 0;
            tmp = (tmpFrameLeght - tmp2);
            memcpy((audioBuffer + wrIndex), ((frameInfo->framePayload) + tmp2), tmp);
            wrIndex = tmp;
        }
        else
        {
            // Write frame to AudioBuffer
            memcpy((audioBuffer + wrIndex), (frameInfo->framePayload), frameInfo->frameSize);
            wrIndex += tmpFrameLeght;
        }

        // Upadte Write Index and send signal to liveAudioPlay thread as thread can be in wait
        GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
        audioStream.wrPtr = wrIndex;
        pthread_cond_signal(&audioStream.streamSignal);
        GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   changes two channel location on current window.
 * @param   windId1
 * @param   windId2
 * @param   dispDevId
 * @return
 */
BOOL SwapWindChannel(UINT8 windId1, UINT8 windId2, DISPLAY_DEV_e dispDevId)
{
    BOOL        retVal = FAIL;
    UINT8       tempDec;
    HI_S32      s32Ret = HI_SUCCESS;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    if ((windId2 >= MAX_WINDOWS) || (windId1 >= MAX_WINDOWS))
    {
        return FAIL;
    }

    DPRINT(GUI_LIB, "window swap: [window1=%d], [window2=%d]", windId1, windId2);
    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    do
    {
        if(decDispLib.ch2WinMap[windId1] < INVALID_DEC_DISP_ID)
        {
            /* Disables a specified VO channel*/
            s32Ret = HI_MPI_VO_DisableChn(VO_LAYER_ID, windId1);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VO_DisableChn",s32Ret);
                break;
            }

            s32Ret = unBindVpssVo(VO_LAYER_ID, windId1, decDispLib.ch2WinMap[windId1], VPSS_CHN0);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("unBindVpssVo",s32Ret);
                break;
            }
        }

        if(decDispLib.ch2WinMap[windId2] < INVALID_DEC_DISP_ID)
        {
            /* Disables a specified VO channel*/
            s32Ret = HI_MPI_VO_DisableChn(VO_LAYER_ID, windId2);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VO_DisableChn",s32Ret);
                break;
            }

            s32Ret = unBindVpssVo(VO_LAYER_ID, windId2, decDispLib.ch2WinMap[windId2], VPSS_CHN0);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("unBindVpssVo",s32Ret);
                break;
            }
        }

        tempDec = decDispLib.ch2WinMap[windId1];
        decDispLib.ch2WinMap[windId1] = decDispLib.ch2WinMap[windId2];
        decDispLib.ch2WinMap[windId2] = tempDec;

        if(decDispLib.ch2WinMap[windId1] < INVALID_DEC_DISP_ID)
        {
            stDestChn.enModId = HI_ID_VOU;
            stDestChn.s32DevId = VO_LAYER_ID;
            stDestChn.s32ChnId = windId1;

            /*Obtains the information about a bound data source.*/
            if((s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn)) == HI_SUCCESS)
            {
                if((s32Ret = unBindVpssVo(VO_LAYER_ID, windId1, stSrcChn.s32DevId,VPSS_CHN0)) != HI_SUCCESS)
                {
                    API_FAIL_DEBUG("unBindVpssVo",s32Ret);
                }
            }

            /* Enables a specified VO channel*/
            s32Ret = HI_MPI_VO_EnableChn(VO_LAYER_ID, windId1);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VO_EnableChn",s32Ret);
                break;
            }

            s32Ret = bindVpssVo(VO_LAYER_ID, windId1, decDispLib.ch2WinMap[windId1], VPSS_CHN0);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("bindVpssVo",s32Ret);
                break;
            }
        }

        if(decDispLib.ch2WinMap[windId2] < INVALID_DEC_DISP_ID)
        {
            stDestChn.enModId = HI_ID_VOU;
            stDestChn.s32DevId = VO_LAYER_ID;
            stDestChn.s32ChnId = windId2;

            /*Obtains the information about a bound data source.*/
            if((s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn)) == HI_SUCCESS)
            {
                if((s32Ret = unBindVpssVo(VO_LAYER_ID, windId2, stSrcChn.s32DevId, VPSS_CHN0)) != HI_SUCCESS)
                {
                    API_FAIL_DEBUG("unBindVpssVo",s32Ret);
                }
            }

            /* Enables a specified VO channel*/
            s32Ret = HI_MPI_VO_EnableChn(VO_LAYER_ID, windId2);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VO_EnableChn",s32Ret);
                break;
            }

            s32Ret = bindVpssVo(VO_LAYER_ID, windId2, decDispLib.ch2WinMap[windId2], VPSS_CHN0);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("bindVpssVo",s32Ret);
                break;
            }
        }

        retVal = SUCCESS;

    }while(0);

    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UpdateChannelToWindowMap
 * @param   dispDevId
 * @param   offset
 * @param   upOrDownDir
 * @param   selectedWindow
 * @param   layoutIndex
 * @return
 */
BOOL UpdateChannelToWindowMap(DISPLAY_DEV_e dispDevId, UINT8 offset, WINDOW_SHIFT_DIR_e upOrDownDir,
                              UINT8 selectedWindow, WIND_LAYOUT_ID_e layoutIndex)
{
    UINT8   winCnt=0, totalWinOnLayout = 0;
    UINT8   tmpShift = 0;
    UINT8   newChnMap[MAX_WINDOWS];
    HI_S32  s32Ret = HI_SUCCESS;

    if((dispDevId >= MAX_DISPLAY_DEV) || (layoutIndex >= WIND_LAYOUT_MAX))
    {
        EPRINT(GUI_LIB,"Invalid parameters [dispDevId=%d][layoutIndex=%d]",dispDevId,layoutIndex);
        return FAIL;
    }

    DPRINT(GUI_LIB, "window chnl map changed: [layout=%d], [dir=%s], [offset=%d], [selectedWindow=%d]",
           layoutIndex, upOrDownDir ? "DOWN" : "UP", offset, selectedWindow);
    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    for(winCnt = 0; winCnt < MAX_WINDOWS; winCnt++)
    {
        /* Disables a specified VO channel*/
        s32Ret = HI_MPI_VO_DisableChn(VO_LAYER_ID, winCnt);
        if(s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VO_DisableChn",s32Ret);
        }
        else
        {
            s32Ret = unBindVpssVo(VO_LAYER_ID,(VO_CHN)winCnt, decDispLib.ch2WinMap[winCnt], VPSS_CHN0);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("unBindVpssVo",s32Ret);
            }
        }
    }

    // Generate Mosaic layout for display device
    s32Ret = startVoChn(VO_LAYER_ID, layoutIndex);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("startVoChn",s32Ret);
    }

    if(upOrDownDir == CHANNEL_UP_SHIFT)
    {
        for(winCnt = 0; winCnt < MAX_WINDOWS; winCnt++)
        {
            if((winCnt + offset) < MAX_WINDOWS)
            {
                newChnMap[winCnt] = decDispLib.ch2WinMap[winCnt + offset];
            }
            else
            {
                newChnMap[winCnt] = decDispLib.ch2WinMap[tmpShift++];
            }
        }
    }
    else
    {
        tmpShift = (MAX_WINDOWS - offset);
        for(winCnt = 0; winCnt < MAX_WINDOWS; winCnt++)
        {
            if(winCnt < offset)
            {
                newChnMap[winCnt] = decDispLib.ch2WinMap[tmpShift++];
            }
            else
            {
                newChnMap[winCnt] = decDispLib.ch2WinMap[winCnt - offset];
            }
        }
    }

    totalWinOnLayout =  getTotalWindOnLayout(layoutIndex);

    for(winCnt = 0; winCnt < MAX_WINDOWS; winCnt++)
    {
        if(winCnt < totalWinOnLayout)
        {
            decDispLib.ch2WinMap[winCnt] = newChnMap[winCnt];

            if(newChnMap[winCnt] < INVALID_DEC_DISP_ID)
            {
                /* Enables a specified VO channel*/
                if((s32Ret = HI_MPI_VO_EnableChn(VO_LAYER_ID, winCnt)) != HI_SUCCESS)
                {
                    API_FAIL_DEBUG("HI_MPI_VO_EnableChn",s32Ret);
                }
                else if((s32Ret = bindVpssVo(VO_LAYER_ID, winCnt, newChnMap[winCnt], VPSS_CHN0)) != HI_SUCCESS)
                {
                    API_FAIL_DEBUG("bindVpssVo",s32Ret);

                }
            }
        }
        else
        {
            decDispLib.ch2WinMap[winCnt] = newChnMap[winCnt];
        }
    }
    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

    decDispLib.displayInfo.windowLayout = layoutIndex;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function changes the layout mode for given display channel and display channel
 *          which are fit into this window. Other channel are not display.
 * @param   dispDevId
 * @param   windLayoutId
 * @param   windId
 * @return
 */
BOOL ChangeWindowLayout(DISPLAY_DEV_e dispDevId, WIND_LAYOUT_ID_e windLayoutId, UINT8 selectedWindow)
{
    INT32 	pageNo = HI_FAILURE;
    UINT8	currWindow, windowId, startWindow;
    UINT8   totalWinOnLayout = 0;
    UINT8   newChnMap[MAX_WINDOWS];
    HI_S32 	s32Ret = HI_SUCCESS;

    if((dispDevId >= MAX_DISPLAY_DEV) || (windLayoutId >= WIND_LAYOUT_MAX) || (windLayoutId < WIND_LAYOUT_1X1_1CH) || (selectedWindow >= INVALID_DEC_DISP_ID))
    {
        EPRINT(GUI_LIB, "invld input params: [dispDevId=%d], [windLayoutId=%d], [windowId=%d]", dispDevId, windLayoutId, selectedWindow);
        return FAIL;
    }

    decDispLib.displayInfo.windowLayout = windLayoutId;
    pageNo = getPageNo(dispDevId, selectedWindow);
    totalWinOnLayout = getTotalWindOnLayout(windLayoutId);
    startWindow = (pageNo * totalWinOnLayout);
    DPRINT(GUI_LIB, "change layout: [windowId=%d], [windLayoutId=%d], [pageNo=%d], [totalWinOnLayout=%d]", selectedWindow, windLayoutId, pageNo, totalWinOnLayout);

    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    for(windowId = 0; windowId < MAX_WINDOWS; windowId++)
    {
        currWindow = (windowId + startWindow);
        if(currWindow < MAX_WINDOWS)
        {
            newChnMap[windowId] = decDispLib.ch2WinMap[currWindow];
        }
        else
        {
            newChnMap[windowId] = INVALID_DEC_DISP_ID;
        }

        /* Disables a specified VO channel*/
        s32Ret = HI_MPI_VO_DisableChn(VO_LAYER_ID, windowId);
        if(s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_VO_DisableChn",s32Ret);
        }

        s32Ret = unBindVpssVo(VO_LAYER_ID,(VO_CHN)windowId, decDispLib.ch2WinMap[windowId], VPSS_CHN0);
        if(s32Ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("unBindVpssVo",s32Ret);
        }
    }

    // Generate Mosaic layout for display device
    s32Ret = startVoChn(VO_LAYER_ID, windLayoutId);
    if (s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("startVoChn",s32Ret);
        GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);
        return FAIL;
    }

    totalWinOnLayout = getTotalWindOnLayout(windLayoutId);

    for(windowId = 0; windowId < MAX_WINDOWS; windowId++)
    {
        if((windowId < totalWinOnLayout) && (newChnMap[windowId] < INVALID_DEC_DISP_ID))
        {
            /* Enables a specified VO channel*/
            s32Ret = HI_MPI_VO_EnableChn(VO_LAYER_ID, windowId);
            if(s32Ret != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_VO_EnableChn",s32Ret);
            }
            else
            {
                s32Ret = bindVpssVo(VO_LAYER_ID, windowId, newChnMap[windowId], VPSS_CHN0);
                if(s32Ret != HI_SUCCESS)
                {
                    API_FAIL_DEBUG("bindVpssVo",s32Ret);
                }
            }
        }
    }
    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function start vo dev, vo layer, open and set resolution for frame buffer and start hdmi.
 * @param   vo_resolution
 * @return
 */
static BOOL startVoAndHdmi(VO_INTF_SYNC_E vo_resolution)
{
    HI_S32  s32ret = HI_FAILURE;
    UINT32  width = 0, height = 0, frameFq = 0;

    s32ret = startVoDev(VO_DEVICE_ID, vo_resolution);
    if(HI_SUCCESS != s32ret)
    {
        API_FAIL_DEBUG("startVoDev",s32ret);
        return FAIL;
    }

    s32ret =startHdmi(vo_resolution);
    if(HI_SUCCESS != s32ret)
    {
        API_FAIL_DEBUG("startHdmi",s32ret);
        stopVoDev(VO_DEVICE_ID);
        return FAIL;
    }

    s32ret = startVoLayer(VO_LAYER_ID, vo_resolution);
    if(HI_SUCCESS != s32ret)
    {
        API_FAIL_DEBUG("startVoLayer",s32ret);
        stopVoDev(VO_DEVICE_ID);
        return FAIL;
    }

	s32ret = getVoWH(vo_resolution, &width, &height, &frameFq);
    if(HI_SUCCESS != s32ret)
    {
		API_FAIL_DEBUG("getVoWH",s32ret);
		stopVoLayer(VO_LAYER_ID);
		stopVoDev(VO_DEVICE_ID);
        return FAIL;
	}

    //open and set resolution for frame buffer
    s32ret = getFbInfo(width, height);
    if(HI_SUCCESS != s32ret)
    {
        API_FAIL_DEBUG("getFbInfo",s32ret);
        stopVoLayer(VO_LAYER_ID);
        stopVoDev(VO_DEVICE_ID);
        return FAIL;
    }

	s32ret = startVoChn(VO_LAYER_ID, decDispLib.displayInfo.windowLayout);
    if(HI_SUCCESS != s32ret)
    {
        API_FAIL_DEBUG("startVoChn",s32ret);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   changes resolution of give display device.
 * @param   dispDevId
 * @param   dispResolution
 * @return
 */
static BOOL setDispResolution(DISPLAY_DEV_e dispDevId, DISPLAY_RESOLUTION_e dispResolution)
{
	HI_S32						s32ret;
	VO_INTF_SYNC_E				vo_resolution = VO_OUTPUT_BUTT;
	HI_HDMI_SINK_CAPABILITY_S	stSinkCap;
	HI_S32						supportedRes = 0;
	HI_S32						loop = 0;
    DISPLAY_RESOLUTION_e        best_res;

	/* Get Display Capabilities */
	memset(&stSinkCap, 0, sizeof(HI_HDMI_SINK_CAPABILITY_S));
	s32ret = HI_MPI_HDMI_GetSinkCapability(HI_HDMI_ID_0, &stSinkCap);
	if(s32ret != HI_SUCCESS)
	{
		API_FAIL_DEBUG("HI_MPI_HDMI_GetSinkCapability",s32ret);
		return FAIL;
	}

	DPRINT(GUI_LIB,"[bConnected=%d] [bSupportHdmi=%d] [bIsSinkPowerOn=%d] [bIsRealEDID=%d] [enNativeVideoFormat=%d]",
		   stSinkCap.bConnected, stSinkCap.bSupportHdmi, stSinkCap.bIsSinkPowerOn, stSinkCap.bIsRealEDID, stSinkCap.enNativeVideoFormat);

	/* Do not set Resolution in case of HDMI not supported/Display not connected */
	if( (!stSinkCap.bConnected) || (!stSinkCap.bSupportHdmi) || (!stSinkCap.bIsRealEDID) )
	{
		return FAIL;
	}

	for(loop = 0;loop < HI_HDMI_VIDEO_FMT_BUTT;loop++)
	{
		if(HI_TRUE == stSinkCap.bVideoFmtSupported[loop])
		{
			DPRINT(GUI_LIB,"%s", hiHdmiVidFormat[loop]);
		}
	}

	/* Look for 720P */
	for(loop = HI_HDMI_VIDEO_FMT_720P_60; loop <= HI_HDMI_VIDEO_FMT_720P_50;loop++)
	{
		if(HI_TRUE == stSinkCap.bVideoFmtSupported[loop])
		{
            supportedRes |= (1 << DISPLAY_RESOLUTION_720P);
			break;
		}
	}
	/* Look for 1080P */
	for(loop = HI_HDMI_VIDEO_FMT_1080P_60; loop <= HI_HDMI_VIDEO_FMT_1080P_24;loop++)
	{
		if(HI_TRUE == stSinkCap.bVideoFmtSupported[loop])
		{
            supportedRes |= (1 << DISPLAY_RESOLUTION_1080P);
			break;
		}
	}
	/* Look for 2160P */
	for(loop = HI_HDMI_VIDEO_FMT_3840X2160P_24; loop <= HI_HDMI_VIDEO_FMT_3840X2160P_60;loop++)
	{
		if(HI_TRUE == stSinkCap.bVideoFmtSupported[loop])
		{
            supportedRes |= (1 << DISPLAY_RESOLUTION_2160P);
			break;
		}
	}

	/* Find Best match from supported Display resolutions */
	best_res = dispResolution;
    for(loop = 0; loop < DISPLAY_RESOLUTION_MAX; loop++)
	{
		if((supportedRes >> loop) & 0x1)
		{
            best_res = (DISPLAY_RESOLUTION_e)loop;
		}
	}

	if (best_res != dispResolution)
	{
		EPRINT(GUI_LIB, "Difference in Display resolution, saved %d supported %d", dispResolution, best_res);
		/*  Use supported Display resolution when supported resolution is lower than configured Display resolution */
		if(dispResolution > best_res)
		{
			dispResolution = best_res;
		}
	}

	/* Set vo_resolution based on supported Display Resolution */
	switch(dispResolution)
    {
        case DISPLAY_RESOLUTION_2160P:
        {
            if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_3840X2160P_30])
            {
                vo_resolution = VO_OUTPUT_3840x2160_30;
            }
            else if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_3840X2160P_25])
            {
                vo_resolution = VO_OUTPUT_3840x2160_25;
            }
        }
            /* Fall Through to check 1080p/720p Display Resolution */
        case DISPLAY_RESOLUTION_1080P:
        default:
        {
            if(VO_OUTPUT_BUTT != vo_resolution)
            {
                break;
            }
            if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_1080P_60])
            {
                vo_resolution = VO_OUTPUT_1080P60;
            }
            else if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_1080P_50])
            {
                vo_resolution = VO_OUTPUT_1080P50;
            }
            else if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_1080P_30])
            {
                vo_resolution = VO_OUTPUT_1080P30;
            }
            else if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_1080P_25])
            {
                vo_resolution = VO_OUTPUT_1080P25;
            }
            else if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_1080P_24])
            {
                vo_resolution = VO_OUTPUT_1080P24;
            }
        }

        /* Fall Through to check 720p Display Resolution */
        case DISPLAY_RESOLUTION_720P:
        {
            if(VO_OUTPUT_BUTT != vo_resolution)
            {
                break;
            }
            if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_720P_60])
            {
                vo_resolution = VO_OUTPUT_720P60;
            }
            else if(HI_TRUE == stSinkCap.bVideoFmtSupported[HI_HDMI_VIDEO_FMT_720P_50])
            {
                vo_resolution = VO_OUTPUT_720P50;
            }
        }
        break;
    }

	/* If vo_resolution still not set, then
	 * Initialize Display with configured resolution(Using Default Refresh frequencies) */
	if(VO_OUTPUT_BUTT == vo_resolution)
	{
        switch(dispResolution)
        {
            case DISPLAY_RESOLUTION_720P:
                vo_resolution = VO_OUTPUT_720P60;
                break;

            case DISPLAY_RESOLUTION_2160P:
                vo_resolution = VO_OUTPUT_3840x2160_30;
                break;

            default:
            case DISPLAY_RESOLUTION_1080P:
                vo_resolution = VO_OUTPUT_1080P60;
                break;
		}
	}

	/* Note: When vo_resolution matched with gCurr_vo_resolution, No need to proceed
     * 1. For Display Resolution 1080P, Live Stream gets blank on HDMI cable re-connect If we set same resoluton again
     * 2. For Display Resolution 2160P, Black screen gets displayed on HDMI cable re-connect If we do not set same resoluton again */
    if((DISPLAY_RESOLUTION_2160P != dispResolution) && (vo_resolution == gCurr_vo_resolution))
	{
		DPRINT(GUI_LIB,"vo_resolution[%d] already set", vo_resolution);
		return  SUCCESS;
	}

	if(frameBufFd != -1)
	{
		close(frameBufFd);
		frameBufFd = -1;
	}

	stopVoChn(VO_LAYER_ID);
	stopVoLayer(VO_LAYER_ID);
	stopVoDev(VO_DEVICE_ID);

	decDispLib.displayInfo.dispResolution = dispResolution;
    if(FAIL == startVoAndHdmi(vo_resolution))
	{
        EPRINT(GUI_LIB,"startVoAndHdmi failed");
		return  FAIL;
	}

    DPRINT(GUI_LIB,"Set Display Resolution Success[dispDevId=%d][dispResolution=%d][vo_resolution=%d]", dispDevId, dispResolution, vo_resolution);
	gCurr_vo_resolution = vo_resolution;
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   removes audio into running streaming.
 * @param   decDispId
 * @return
 */
BOOL ExcludeAudio(UINT8 decDispId)
{
    if ((decDispId >= MAX_DEC_DISP_CHN) && (decDispId != AUDIO_OUT_DEC_CHNL_ID))
    {
        EPRINT(GUI_LIB,"Invalid input parameters [decDispId=%d]",decDispId);
        return FAIL;
    }

    GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
    if(audioStream.audioChannel == decDispId)
    {
        audioStream.audioChannel = INVALID_DEC_DISP_PLAY_ID;
        audioStream.terAudioThread = TRUE;
        bzero(audioBuffer,MAX_AUDIO_BUFFER_SIZE);
    }
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
    GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
    pthread_cond_signal(&audioStream.streamSignal);
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   adds audio into running streaming.
 * @param   decDispId
 * @return
 */
BOOL IncludeAudio(UINT8 decDispId)
{
    BOOL	   			retVal = FAIL;
    pthread_t  			audiodecode;
    pthread_attr_t		audioAttr;

    if ((decDispId >= MAX_DEC_DISP_CHN) && (decDispId != AUDIO_OUT_DEC_CHNL_ID))
    {
        EPRINT(GUI_LIB,"Invalid input parameters [decDispId=%d]",decDispId);
        return FAIL;
    }

    GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
    audioStream.audioChannel = decDispId;
    if(audioStream.threadRunningFlag == TRUE)
    {
        GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
        return FAIL;
    }

    audioStream.threadRunningFlag = TRUE;
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
    pthread_attr_init(&audioAttr);
    pthread_attr_setdetachstate(&audioAttr, PTHREAD_CREATE_DETACHED);
    // Create live Audio play thread
    if (pthread_create(&audiodecode, &audioAttr, liveAudioPlay, NULL) != 0)
    {
        GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
        audioStream.threadRunningFlag = FALSE;
        GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
        EPRINT(GUI_LIB,"Failed to Create liveAudioPlay thread");
    }
    else
    {
        DPRINT(GUI_LIB,"live audio play thread created");
        retVal = SUCCESS;
    }

    //Destroy thread
    pthread_attr_destroy(&audioAttr);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   change the state of audio output Device
 * @param   muteState
 * @return
 */
BOOL SetAudioMuteUnMute(AUDIO_STATE_e muteState)
{
    BOOL      		retVal = SUCCESS;
    HI_BOOL   		setMuteState;
    HI_S32 	  		s32ret;
    AUDIO_DEVICE_e 	audioDevCnt = AUDIO_AO_DEV;

    if (curAudioOutState != muteState)
    {
        curAudioOutState = muteState;
    }

    if (audioStream.audioChannel == INVALID_DEC_DISP_PLAY_ID)
    {
        return SUCCESS;
    }

    setMuteState = (muteState == AUDIO_OFF) ? HI_TRUE : HI_FALSE;
    for(audioDevCnt = AUDIO_AO_DEV; audioDevCnt < MAX_AUDIO_DEVICE; audioDevCnt++)
    {
        s32ret = HI_MPI_AO_SetMute((AUDIO_DEV)audioDevCnt, setMuteState, NULL);
        if(s32ret != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_AO_SetMute",s32ret);
            retVal = FAIL;
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   change the volume level of audio output Deveice.
 * @param   volumeLevel
 * @return
 */
BOOL SetAudioVolume(UINT8 volumeLevel)
{
    BOOL      		retVal = SUCCESS;
    HI_S32          s32ret;
    AUDIO_DEVICE_e 	audioDevCnt = AUDIO_AO_DEV;
    HI_S32          s32Volume;

    if(volumeLevel > VOLUME_LEVEL_MAX)
    {
        EPRINT(GUI_LIB,"Invalid Volume arg [%d], should be in [0 - %d]",volumeLevel,VOLUME_LEVEL_MAX);
        return SUCCESS;
    }

    curAudioOutVol = volumeLevel;
    if(audioStream.audioChannel == INVALID_DEC_DISP_PLAY_ID)
    {
        return SUCCESS;
    }

    if(volumeLevel == 0)
    {
        s32Volume = AUDIO_DEF_VOL_DB;
    }
    else
    {
        s32Volume = AUDIO_REF_VOL_MIN_DB + (volumeLevel * AUDIO_DB_VOL_MULTIPLIER);
    }

    for(audioDevCnt = AUDIO_AO_DEV; audioDevCnt < MAX_AUDIO_DEVICE; audioDevCnt++)
    {
        s32ret = HI_MPI_AO_SetVolume((AUDIO_DEV)audioDevCnt, s32Volume);
        if(HI_SUCCESS != s32ret)
        {
            API_FAIL_DEBUG("HI_MPI_AO_SetVolume",s32ret);
            retVal = FAIL;
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   return audio channel Device to ON/OFF.
 * @return
 */
UINT8 GetCurrentAudioChannel(void)
{
    GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
    UINT8 audioChannel = audioStream.audioChannel;
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
    return audioChannel;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to zoom in and also zoom out to particular channel in window layout.
 *          For zoom out user need to set all parameter of cropParam
 * @param   windId
 * @param   cropParam
 * @param   zoomType
 * @return
 */
BOOL SetCropParam(UINT8 windId, CROP_PARAM_t * cropParam,ZOOM_TYPE_e zoomType)
{
    UINT8               decChnNo;
    UINT32              width;
    UINT32              height;
    HI_S32              s32Ret;
    VPSS_CROP_INFO_S    pstCropInfo;
    VALID_SCREEN_INFO_t validScreen;

    if (windId >= MAX_WINDOWS)
    {
        return FAIL;
    }

    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    decChnNo = decDispLib.ch2WinMap[windId];
    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

    if (decChnNo >= MAX_DEC_DISP_CHN)
    {
        EPRINT(GUI_LIB,"Zooming [windId=%d] Error:Invalid [decChnNo=%d]",windId,decChnNo);
        return FAIL;
    }

    memset(&pstCropInfo, 0, sizeof(VPSS_CROP_INFO_S));
    pstCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    if(zoomType == ZOOM_IN)
    {
        // Finding frame frameHeight & frameWidth
        GUI_LIB_MUTEX_LOCK(decDispList[decChnNo].decDispParamMutex);
        height = decDispList[decChnNo].frameHeight;
        width = decDispList[decChnNo].frameWidth;
        GUI_LIB_MUTEX_UNLOCK(decDispList[decChnNo].decDispParamMutex);

        // fill Zoom Attributes
        pstCropInfo.bEnable = HI_TRUE;
        GetValidScreenInfo(MAIN_VIDEO_DISPLAY,&validScreen,windId);
        pstCropInfo.stCropRect.s32X     = ALIGN_BACK(((cropParam->startXPos * width) / validScreen.actualWidth) , 4);
        pstCropInfo.stCropRect.s32Y     = ALIGN_BACK(((cropParam->startYPos * height) / validScreen.actualHeight), 4);
        pstCropInfo.stCropRect.u32Width = ALIGN_BACK(((cropParam->width * width) / validScreen.actualWidth), 8);
        pstCropInfo.stCropRect.u32Height= ALIGN_BACK(((cropParam->height * height) / validScreen.actualHeight), 8);

        if(pstCropInfo.stCropRect.u32Width <= (2 * VPSS_MIN_WIDTH))
        {
            pstCropInfo.stCropRect.u32Width = ALIGN_BACK((2 * VPSS_MIN_WIDTH), 8);
        }

        if(pstCropInfo.stCropRect.u32Height <= (2 * VPSS_MIN_HEIGHT))
        {
            pstCropInfo.stCropRect.u32Height = ALIGN_BACK((2 * VPSS_MIN_HEIGHT), 8);
        }

        if(pstCropInfo.stCropRect.u32Width > VPSS_MAX_WIDTH)
        {
            pstCropInfo.stCropRect.u32Width = ALIGN_BACK((VPSS_MAX_WIDTH), 8);
        }

        if(pstCropInfo.stCropRect.u32Height >  VPSS_MAX_HEIGHT)
        {
            pstCropInfo.stCropRect.u32Height = ALIGN_BACK((VPSS_MAX_HEIGHT), 8);
        }
    }
    else
    {
        pstCropInfo.bEnable = HI_FALSE; /*CROP enable*/
        pstCropInfo.stCropRect.s32X = 0;
        pstCropInfo.stCropRect.s32Y = 0;
        pstCropInfo.stCropRect.u32Width = 0;
        pstCropInfo.stCropRect.u32Height = 0;
    }

    /* Sets the crop attributes of the VPSS*/
    s32Ret = HI_MPI_VPSS_SetGrpCrop(decChnNo, &pstCropInfo);
    if(s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VPSS_SetGrpCrop",s32Ret);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sets the effect of the output pictures from a device.
 * @param   devId
 * @param   displayParam
 * @param   value
 * @return
 */
BOOL SetDisplayParameter(PHYSICAL_DISPLAY_TYPE_e devId, DISPLAY_SCREEN_PARAM_TYPE_e displayParam, UINT32 value)
{
    VO_HDMI_PARAM_S pstHdmiParam;
    UINT32          compValue;
    HI_S32          s32Ret;

    if ((devId >= PHYSICAL_DISPLAY_TYPE_MAX) || (displayParam >= DISPLAY_SCREEN_PARAM_MAX) || (value > 100))
    {
        return FAIL;
    }

    /* Obtains the HDMI output picture effect.*/
    s32Ret = HI_MPI_VO_GetHdmiParam(DEVICE_DHD0, &pstHdmiParam);
    if(s32Ret!= HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VO_GetHdmiParam",s32Ret);
        return FAIL;
    }

    switch(displayParam)
    {
        case DISPLAY_BRIGHTNESS:
             /* luminance:   0 ~ 100 default: 50 */
            pstHdmiParam.stCSC.u32Luma = value;
            break;

        case DISPLAY_CONTRAST:
            /* contrast :   0 ~ 100 default: 50 */
            compValue = REF_MIN_CONTRAST + (value * CONTRAST_MULTIPLIER);
            pstHdmiParam.stCSC.u32Contrast = compValue;
            break;

        case DISPLAY_SATURATION:
            /* saturation:  0 ~ 100 default: 50 */
            pstHdmiParam.stCSC.u32Saturation = value;
            break;

        case DISPLAY_HUE:
            /* hue      :   0 ~ 100 default: 50 */
            pstHdmiParam.stCSC.u32Hue = value;
            break;

        default:
            break;
    }

    /*Sets the HDMI output picture effect. */
    s32Ret = HI_MPI_VO_SetHdmiParam(DEVICE_DHD0, &pstHdmiParam);
    if(s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VO_SetHdmiParam",s32Ret);
        return FAIL;
    }

    DPRINT(GUI_LIB,"New HDMI Param :[luma=%d] [Contrast=%d] [Saturation=%d] [Hue=%d]",
           pstHdmiParam.stCSC.u32Luma, pstHdmiParam.stCSC.u32Contrast, pstHdmiParam.stCSC.u32Saturation, pstHdmiParam.stCSC.u32Hue);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives the Current Page Number for particular display Device
 * @param   displayId
 * @param   windId
 * @return
 */
static INT32 getPageNo(DISPLAY_DEV_e displayId, UINT8 windId)
{
    return (windId / maxWindPerLayout[decDispLib.displayInfo .windowLayout]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief deInitDecoderResources delete the resources created on decoder creation
 * @param decoderId
 * @param windowId
 * @return
 */
static void deInitDecoderResources(UINT8 decoderId, UINT8 windowId)
{
    if (windowId < MAX_WINDOWS)
    {
        UpdateDecoderStatusOnVideoLoss(decoderId, MAIN_VIDEO_DISPLAY, windowId);
        unlinkChn(windowId);
    }

    unBindVdecVpss((VDEC_CHN)decoderId, (VPSS_GRP)decoderId);
    deleteVpssGrp((VPSS_GRP)decoderId, 1);
    deleteDecChn((VDEC_CHN)decoderId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   delete the resources created on decoder creation
 * @param   currState
 * @param   decoderId
 * @param   windowId
 * @return
 */
static HI_S32 clearPrevDecResources(DECODER_CREATION_STATE_e  currState, UINT8 decoderId,UINT8 windowId)
{
    switch(currState)
    {
        case BIND_VPSS_VO:
            unBindVdecVpss((VDEC_CHN)decoderId, (VPSS_GRP)decoderId);

            /* FALLS THROUGH */
        case BIND_VDEC_VPSS:
            deleteVpssGrp((VPSS_GRP)decoderId, 1);

            /* FALLS THROUGH */
        case VPSS_CREATION:
            /* Disables a specified VO channel*/
            HI_MPI_VO_DisableChn(VO_LAYER_ID, windowId);

            /* FALLS THROUGH */
        case VO_CHN_ENABLE:
            /* Disables a specified VO channel*/
            HI_MPI_VO_DisableChn(VO_LAYER_ID, windowId);
            deleteDecChn((VDEC_CHN)decoderId);
            break;

        case DECODER_CREATION:
            deInitDecoderResources(decoderId, getWindowId(decoderId));
            break;

        default:
            break;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getWindowId This function delete the resources created on decoder creation
 * @param decoderId
 * @return
 */
static UINT8 getWindowId(UINT8 decoderId)
{
    UINT8 winCnt,windowId = INVALID_DEC_DISP_ID;

    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    for(winCnt = 0; winCnt < MAX_WINDOWS; winCnt++)
    {
        if (decDispLib.ch2WinMap[winCnt] == decoderId)
        {
            windowId = winCnt;
            break;
        }
    }
    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);
    return windowId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   update the link between given channel and associated decoder.
 * @param   windId
 * @return
 */
static HI_S32 updateChn2DecLink(UINT8 windId)
{
    HI_S32 status = HI_FAILURE;

    GUI_LIB_MUTEX_LOCK(decDispLib.ch2WindMapMutex);
    if(decDispLib.ch2WinMap[windId] != INVALID_DEC_DISP_ID)
    {
        status = unlinkChn(windId);
    }
    GUI_LIB_MUTEX_UNLOCK(decDispLib.ch2WindMapMutex);

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief unlinkChn delete the link between given channel and associated decoder.
 * @param windId
 * @return
 */
static HI_S32 unlinkChn(UINT8 windId)
{
    HI_S32 s32Ret;

    /* Disables a specified VO channel*/
    s32Ret = HI_MPI_VO_DisableChn(VO_LAYER_ID, windId);
    if(s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_VO_DisableChn",s32Ret);
    }

    /* PARASOFT: Global variable synchronization maintained externally using mutex lock in caller scope */
    s32Ret = unBindVpssVo(VO_LAYER_ID, windId, decDispLib.ch2WinMap[windId], VPSS_CHN0);
    if(s32Ret != HI_SUCCESS)
    {
        API_FAIL_DEBUG("unBindVpssVo",s32Ret);
        EPRINT(GUI_LIB,"Failed to unlink channel [error=%#x][windId=%d]",s32Ret,windId);
    }

    /* PARASOFT: Global variable synchronization maintained externally using mutex lock in caller scope */
    decDispLib.ch2WinMap[windId] = INVALID_DEC_DISP_ID;
    return s32Ret;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread function which received encoded audio data decode and send audio data to audio output device
 * @return
 */
static void *liveAudioPlay(void *pThArg)
{
    UINT32          wrIndex = 0;
    AUDIO_STREAM_S  stAudioStream;
    HI_S32          retval = HI_SUCCESS;
    HI_BOOL         readFrame = HI_FALSE;

    audioStream.terAudioThread = FALSE;
    GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
    audioStream.rdPtr = audioStream.wrPtr;
    wrIndex = audioStream.wrPtr;
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);

    stAudioStream.pStream = (HI_U8*)malloc(sizeof(HI_U8) * MAX_AUDIO_STREAM_LEN);
    if (NULL == stAudioStream.pStream)
    {
        EPRINT(GUI_LIB,"pu8AudioStream malloc failed");

        GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
        audioStream.threadRunningFlag = FALSE;
        if (audioOutInitStatus == TRUE)
        {
            stopAudioOut();
            audioOutInitStatus = FALSE;
        }
        GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);

        // If Memory allocation Failed then EXIT Thread
        goto AUD_TH_EXIT;
    }

    while(1)
    {
        // Data in audioBuffer is greater or Equal_to Read length or not
        if(wrIndex > audioStream.rdPtr)
        {
            readFrame = ((wrIndex - audioStream.rdPtr) >= DEFAULT_AUDIO_PLAY_FRAME_LEN)?TRUE:FALSE;
        }
        else if(audioStream.rdPtr > wrIndex)
        {
            readFrame = (((MAX_AUDIO_BUFFER_SIZE - audioStream.rdPtr) + wrIndex) >= DEFAULT_AUDIO_PLAY_FRAME_LEN)?TRUE:FALSE;
        }

        if ((((MAX_AUDIO_BUFFER_SIZE - (wrIndex - audioStream.rdPtr)) % MAX_AUDIO_BUFFER_SIZE) > DEFAULT_AUDIO_PLAY_FRAME_LEN) && (readFrame))

        {
            if ((audioOutInitStatus == TRUE) && readFrame)
            {
                // Header
                *(stAudioStream.pStream) = 0x0;
                *(stAudioStream.pStream + 1) = 0x1;
                *(stAudioStream.pStream + 2) = 0xA0;
                *(stAudioStream.pStream + 3) = 0x0;

                memcpy((stAudioStream.pStream) + 4, audioBuffer + audioStream.rdPtr, DEFAULT_AUDIO_PLAY_FRAME_LEN);

                stAudioStream.u32Len = DEFAULT_AUDIO_PLAY_FRAME_LEN + 4;

                if(curAudioOutState == AUDIO_ON)
                {
                    retval = HI_MPI_ADEC_SendStream(0, &stAudioStream, HI_TRUE);
                    if (HI_SUCCESS != retval)
                    {
                        API_FAIL_DEBUG("HI_MPI_ADEC_SendStream",retval);
                        break;
                    }
                }

                audioStream.rdPtr += DEFAULT_AUDIO_PLAY_FRAME_LEN;
                audioStream.rdPtr %= MAX_AUDIO_BUFFER_SIZE;
            }
        }
        else
        {
            // put thread in wait if Buffer is empty or Data less than Read length
            GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
            pthread_cond_wait(&audioStream.streamSignal,&audioStream.audioTaskMutex);
            GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
        }

        GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);

        if (audioStream.terAudioThread == TRUE)
        {
            audioStream.threadRunningFlag = FALSE;
            if (audioOutInitStatus == TRUE)
            {
                // DeInit Audio Config & Exit Thread
                stopAudioOut();
                audioOutInitStatus = FALSE;
            }
            GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
            break;
        }
        else
        {
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            if (audioOutInitStatus == TRUE)
            {
                wrIndex = audioStream.wrPtr;
            }
            GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
        }
    }

    /* Free allocated memory */
    free(stAudioStream.pStream);

AUD_TH_EXIT:
    DPRINT(GUI_LIB,"Audio Thread Exit");
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used for unbind Adec channel from Ao channel.
 * @param   AoDev
 * @param   AoChn
 * @param   AdChn
 * @return
 */
static HI_S32 unbindAoAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
    HI_S32      retVal = HI_SUCCESS;
    MPP_CHN_S   stSrcChn,stDestChn;

    stSrcChn.enModId = HI_ID_ADEC;
    stSrcChn.s32ChnId = AdChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;
    /*Unbind a data source from a data receiver. */
    retVal = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_UnBind",retVal);
    }
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to desable Ao channel.
 * @param   AoDevId
 * @param   s32AoChnCnt
 * @param   bResampleEn
 * @param   bVqeEn
 * @return
 */
static HI_S32 stopAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt, HI_BOOL bResampleEn, HI_BOOL bVqeEn)
{
    HI_S32 i;
    HI_S32 retVal;

    for (i = 0; i < s32AoChnCnt; i++)
    {
        if (HI_TRUE == bResampleEn)
        {
            retVal = HI_MPI_AO_DisableReSmp(AoDevId, i);
            if (HI_SUCCESS != retVal)
            {
                API_FAIL_DEBUG("HI_MPI_AO_DisableReSmp",retVal);
                return retVal;
            }
        }

        if (HI_TRUE == bVqeEn)
        {
            retVal = HI_MPI_AO_DisableVqe(AoDevId, i);
            if (HI_SUCCESS != retVal)
            {
                API_FAIL_DEBUG("HI_MPI_AO_DisableVqe",retVal);
                return retVal;
            }
        }

        retVal = HI_MPI_AO_DisableChn(AoDevId, i);
        if (HI_SUCCESS != retVal)
        {
            API_FAIL_DEBUG("HI_MPI_AO_DisableChn",retVal);
            return retVal;
        }
    }

    retVal = HI_MPI_AO_Disable(AoDevId);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_AO_Disable",retVal);
        return retVal;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to destroy Adec channel.
 * @param   AdChn
 * @return
 */
static HI_S32 stopAudioDec(ADEC_CHN AdChn)
{
    HI_S32 retVal;

    retVal = HI_MPI_ADEC_DestroyChn(AdChn);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_ADEC_DestroyChn",retVal);
        return retVal;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief bindAoAdec This function is used for bind Adec channel with Ao channel.
 * @param AoDev
 * @param AoChn
 * @param AdChn
 * @return
 */
static HI_S32 bindAoAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
    MPP_CHN_S   stSrcChn,stDestChn;
    HI_S32      retVal = HI_SUCCESS;

    stSrcChn.enModId = HI_ID_ADEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = AdChn;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;

    /*Binds a data source and a data receiver. */
    retVal = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_SYS_Bind",retVal);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to enable Ao channel and Ao Device.
 * @param   AoDevId
 * @param   s32AoChnCnt
 * @param   pstAioAttr
 * @param   enInSampleRate
 * @param   bResampleEn
 * @param   pstAoVqeAttr
 * @param   u32AoVqeType
 * @return
 */
static HI_S32 startAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt, AIO_ATTR_S* pstAioAttr, AUDIO_SAMPLE_RATE_E enInSampleRate,
                      HI_BOOL bResampleEn, HI_VOID* pstAoVqeAttr, HI_U32 u32AoVqeType)
{
    HI_S32 audioCh;
    HI_S32 retVal;

    if (pstAioAttr->u32ClkChnCnt == 0)
    {
        pstAioAttr->u32ClkChnCnt = pstAioAttr->u32ChnCnt;
    }

    retVal = HI_MPI_AO_SetPubAttr(AoDevId, pstAioAttr);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_AO_SetPubAttr",retVal);
        return HI_FAILURE;
    }

    retVal = HI_MPI_AO_Enable(AoDevId);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_AO_Enable",retVal);
        return HI_FAILURE;
    }

    for (audioCh = 0; audioCh < s32AoChnCnt; audioCh++)
    {
        retVal = HI_MPI_AO_EnableChn(AoDevId, audioCh);
        if (HI_SUCCESS != retVal)
        {
            API_FAIL_DEBUG("HI_MPI_AO_EnableChn",retVal);
            return HI_FAILURE;
        }

        if (HI_TRUE == bResampleEn)
        {
            retVal = HI_MPI_AO_DisableReSmp(AoDevId, audioCh);
            retVal |= HI_MPI_AO_EnableReSmp(AoDevId, audioCh, enInSampleRate);
            if (HI_SUCCESS != retVal)
            {
                API_FAIL_DEBUG("HI_MPI_AO_EnableReSmp",retVal);
                return HI_FAILURE;
            }
        }

        if (NULL == pstAoVqeAttr)
        {
            continue;
        }

        HI_BOOL bAoVqe = HI_TRUE;
        switch (u32AoVqeType)
        {
            case 0:
                retVal = HI_SUCCESS;
                bAoVqe = HI_FALSE;
                break;

            case 1:
                retVal = HI_MPI_AO_SetVqeAttr(AoDevId, audioCh, (AO_VQE_CONFIG_S *)pstAoVqeAttr);
                break;

            default:
                retVal = HI_FAILURE;
                break;
        }

        if (retVal)
        {
            API_FAIL_DEBUG("HI_MPI_AO_SetVqeAttr",retVal);
            return retVal;
        }

        if (bAoVqe)
        {
            retVal = HI_MPI_AO_EnableVqe(AoDevId, audioCh);
            if (retVal)
            {
                API_FAIL_DEBUG("HI_MPI_AO_EnableVqe",retVal);
                return retVal;
            }
        }
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to create Adec channel.
 * @param   AdChn
 * @param   enType
 * @return
 */
static HI_S32 startAudioDec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
    HI_S32              retVal = HI_SUCCESS;
    ADEC_CHN_ATTR_S     stAdecAttr;
    ADEC_ATTR_ADPCM_S   stAdpcm;
    ADEC_ATTR_G711_S    stAdecG711;
    ADEC_ATTR_G726_S    stAdecG726;
    ADEC_ATTR_LPCM_S    stAdecLpcm;

    stAdecAttr.enType = enType;
    stAdecAttr.u32BufSize = 20;
    stAdecAttr.enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */

    if (PT_ADPCMA == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdpcm;
        stAdpcm.enADPCMType = ADPCM_TYPE_DVI4 ;
    }
    else if (PT_G711A == stAdecAttr.enType || PT_G711U == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG711;
    }
    else if (PT_G726 == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG726;
        stAdecG726.enG726bps = MEDIA_G726_40K ;
    }
    else if (PT_LPCM == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecLpcm;
        stAdecAttr.enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
    }
    else
    {
        EPRINT(GUI_LIB,"invalid aenc payload type:%d",stAdecAttr.enType);
        return HI_FAILURE;
    }

    /* create adec chn*/
    retVal = HI_MPI_ADEC_CreateChn(AdChn, &stAdecAttr);
    if (HI_SUCCESS != retVal)
    {
        API_FAIL_DEBUG("HI_MPI_ADEC_CreateChn",retVal);
        return retVal;
    }
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   audioCfgAcodec
 * @param   enSample
 * @return
 */
static HI_S32 audioCfgAcodec(AUDIO_SAMPLE_RATE_E enSample)
{
    HI_S32  fdAcodec = -1;
    HI_S32  ret = HI_SUCCESS;
    UINT32  i2s_fs_sel = 0;
    UINT32  mixer_mic_ctrl = ACODEC_MIXER_LINEIN;
    UINT32  output_ctrl = ACODEC_LINEOUTD_NONE;
    UINT32  gain_mic = 0;
    UINT32  mute = 0;
    INT32   pd = 0;

    fdAcodec = open(ACODEC_FILE, O_RDWR | O_CLOEXEC);
    if (fdAcodec < 0)
    {
        EPRINT(GUI_LIB,"can't open Acodec,%s",ACODEC_FILE);
        return HI_FAILURE;
    }

    //Restores the audio CODEC to default settings. reset the audio code to the default config.
    if(ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
    {
        EPRINT(GUI_LIB,"Reset audio codec error");
    }

    if ((AUDIO_SAMPLE_RATE_8000 == enSample) || (AUDIO_SAMPLE_RATE_11025 == enSample) || (AUDIO_SAMPLE_RATE_12000 == enSample))
    {
        i2s_fs_sel = 0x18;
    }
    else if ((AUDIO_SAMPLE_RATE_16000 == enSample) || (AUDIO_SAMPLE_RATE_22050 == enSample) || (AUDIO_SAMPLE_RATE_24000 == enSample))
    {
        i2s_fs_sel = 0x19;
    }
    else if ((AUDIO_SAMPLE_RATE_32000 == enSample) || (AUDIO_SAMPLE_RATE_44100 == enSample) || (AUDIO_SAMPLE_RATE_48000 == enSample))
    {
        i2s_fs_sel = 0x1a;
    }
    else
    {
        EPRINT(GUI_LIB,"not support enSample:%d",enSample);
        close(fdAcodec);
        return HI_FAILURE;
    }

    /*ACODEC_FS_E*/
    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel))
    {
        EPRINT(GUI_LIB,"set acodec sample rate failed");
        close(fdAcodec);
        return HI_FAILURE;
    }

    switch (1)//g_InnerCodecInput /* 0-micin 1-linein 2-micind 3-lineind */
    {
        case 0:
        {
            /*select the micpga's input, micin linein, or differential input(ACODEC_MIXER_E)*/
            mixer_mic_ctrl = ACODEC_MIXER_MICIN;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

            /* set volume plus (0~0x1f,default 0). analog part input volume control(left channel 0~0x1f)*/
            gain_mic = 0x10;
            if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &gain_mic))
            {
                EPRINT(GUI_LIB,"set acodec micin volume failed");
                ret = HI_FAILURE;
                break;
            }

            /*analog part input volume control(right channel 0~0x1f)*/
            if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &gain_mic))
            {
                EPRINT(GUI_LIB,"set acodec micin volume failed");
                ret = HI_FAILURE;
                break;
            }
        }
        break;

        case 1:
        {
            mixer_mic_ctrl = ACODEC_MIXER_LINEIN;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

            mute = 0;
            if (ioctl(fdAcodec, ACODEC_SET_MICL_MUTE, &mute))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

            if (ioctl(fdAcodec, ACODEC_SET_MICR_MUTE, &mute))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

            /* set volume plus (0~0x1f,default 0). analog part input volume control(left channel 0~0x1f)*/
            gain_mic = 0x00; /* set defailt gain */
            if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &gain_mic))
            {
                EPRINT(GUI_LIB,"set acodec micin volume failed");
                ret = HI_FAILURE;
                break;
            }

            /*analog part input volume control(right channel 0~0x1f)*/
            if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &gain_mic))
            {
                EPRINT(GUI_LIB,"set acodec micin volume failed");
                ret = HI_FAILURE;
                break;
            }
        }
        break;

        case 2:
        {
            mixer_mic_ctrl = ACODEC_MIXER_MICIN_D;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
            {
                EPRINT(GUI_LIB," set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }
        }
        break;

        case 3:
        {
            mixer_mic_ctrl = ACODEC_MIXER_LINEIN_D;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }
        }
        break;

        default:
        {
            EPRINT(GUI_LIB," acodec input mod wrong!");
            ret = HI_FAILURE;
        }
        break;
    }

    if (HI_SUCCESS != ret)
    {
        EPRINT(GUI_LIB,"AUDIO_CfgAcodec failed, not config the right codec");
        return HI_FALSE;
    }

    switch (0)//g_InnerCodecOutput /* 0-lineout 1-leftd 2-rightd */
    {
        case 0:
        {
            /*select the differential output source, left channel or right channel*/
            output_ctrl = ACODEC_LINEOUTD_NONE;
            if (ioctl(fdAcodec, ACODEC_SET_DAC_LINEOUTD, &output_ctrl))
            {
                EPRINT(GUI_LIB," set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

            /*Output mute control(differential output), 1:mute, 0:unmute*/
            mute = 0;
            if (ioctl(fdAcodec, ACODEC_SET_DACD_MUTE, &mute))
            {
                EPRINT(GUI_LIB," set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }
        }
        break;

        case 1:
        {
            output_ctrl = ACODEC_LINEOUTD_LEFT;
            if (ioctl(fdAcodec, ACODEC_SET_DAC_LINEOUTD, &output_ctrl))
            {
                EPRINT(GUI_LIB," set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }
            mute = 0;
            if (ioctl(fdAcodec, ACODEC_SET_DACD_MUTE, &mute))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

            pd = 0;
            /*set adcr power, 0: power up, 1: power down*/
            ioctl(fdAcodec, ACODEC_SET_PD_LINEOUTD, &pd);

        }
        break;

        case 2:
        {
            output_ctrl = ACODEC_LINEOUTD_RIGHT;
            if (ioctl(fdAcodec, ACODEC_SET_DAC_LINEOUTD, &output_ctrl))
            {
                EPRINT(GUI_LIB,"set acodec micin failed");
                ret = HI_FAILURE;
                break;
            }

        }
        break;

        default:
        {
            EPRINT(GUI_LIB,"acodec input mod wrong!");
            ret = HI_FAILURE;
        }
        break;
    }

    close(fdAcodec);

    if (HI_SUCCESS != ret)
    {
        EPRINT(GUI_LIB,"AUDIO_CfgAcodec failed, not config the right codec");
        return HI_FALSE;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function set EnPayloadType.
 * @param frameCodec
 * @return
 */
static void setEnPayloadType(STREAM_CODEC_TYPE_e frameCodec)
{
    switch(frameCodec)
    {
        case AUDIO_G711_ULAW:
            gs_enPayloadType = PT_G711U;
            break;

        case AUDIO_G711_ALAW:
            gs_enPayloadType = PT_G711A;
            break;

        default:
            break;
    }
}


//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function delete the resources created start audioout
 * @param   currState
 * @param   AoDev
 * @param   AoChn
 * @param   AdChn
 * @return
 */
static BOOL FreeAudResources(AUDIO_OUT_STATE_e currState, AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
    BOOL retVal = SUCCESS;

    switch(currState)
    {
        case AUD_AO_START:
            stopAo(AoDev, stAioAttr.u32ChnCnt, gs_bAioReSample[AoDev], HI_FALSE);
            break;

        case AUD_BIND:
            stopAudioDec(AdChn);
            stopAo(AoDev, stAioAttr.u32ChnCnt, gs_bAioReSample[AoDev], HI_FALSE);
            break;

        default:
            break;
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize the audio stream parameter.
 */
static void initAudioStream(void)
{
    audioStream.wrPtr = 0;
    audioStream.rdPtr = 0;
    audioStream.threadRunningFlag = FALSE;
    audioStream.audioChannel = INVALID_DEC_DISP_PLAY_ID;
    GUI_LIB_MUTEX_INIT(audioStream.audioTaskMutex, NULL);
    GUI_LIB_MUTEX_LOCK(audioStream.audioTaskMutex);
    pthread_cond_init(&audioStream.streamSignal, NULL);
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioTaskMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize audio configuration
 * @return
 */
static HI_S32 startAudioOut(HI_VOID)
{
    HI_S32                  retVal = HI_SUCCESS;
    AO_CHN                  AoChn = 0;
    ADEC_CHN                AdChn = 0;
    AUDIO_OUT_STATE_e       currState = MAX_AUD_STATE;
    AUDIO_DEVICE_e          audioDevCnt = AUDIO_AO_DEV;
    AUDIO_RESAMPLE_ATTR_S   stAoReSampleAttr;

    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;/* sample rate */
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16; /* bitwidth */
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER; /* master or slave mode */
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;/* mono or steror */
    stAioAttr.u32EXFlag      = 1;/* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit) */
    stAioAttr.u32FrmNum      = AUDIO_FRAME_BUFF_CNT_MAX;/* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    stAioAttr.u32PtNumPerFrm = DEFAULT_AUDIO_PLAY_FRAME_LEN;/* point num per frame (80/160/240/320/480/1024/2048) (ADPCM IMA should add 1 point, AMR only support 160) */
    stAioAttr.u32ChnCnt      = 1; /* channel number, valid value:1/2/4/8 */
    stAioAttr.u32ClkChnCnt   = 2; /* channel number on FS, valid value:1/2/4/8/16*/
    stAioAttr.u32ClkSel      = 0; /* 0: AI and AO clock is separate 1: AI and AO clock is inseparate, AI use AO's clock */
    gs_bAioReSample[0] = HI_FALSE;
    gs_bAioReSample[1] = HI_TRUE;
    gs_pstAoReSmpAttr = NULL;

    do
    {
        retVal = audioCfgAcodec(stAioAttr.enSamplerate);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("audioCfgAcodec",retVal);
            break;
        }

        retVal = startAudioDec(AdChn, gs_enPayloadType);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("startAudioDec",retVal);
            break;
        }

        stAoReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_BUTT;
        gs_pstAoReSmpAttr = &stAoReSampleAttr;
        for (audioDevCnt = AUDIO_AO_DEV;audioDevCnt < MAX_AUDIO_DEVICE; audioDevCnt++)
        {
            if(audioDevCnt == AUDIO_HDMI_AO_DEV)
            {
                stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
                stAoReSampleAttr.u32InPointNum  = DEFAULT_AUDIO_PLAY_FRAME_LEN;
                stAoReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_8000;
                stAoReSampleAttr.enOutSampleRate = AUDIO_SAMPLE_RATE_48000;
                gs_pstAoReSmpAttr = &stAoReSampleAttr;
            }

            retVal = startAo((AUDIO_DEV)audioDevCnt, stAioAttr.u32ChnCnt, &stAioAttr, gs_pstAoReSmpAttr->enInSampleRate, gs_bAioReSample[audioDevCnt], NULL, 0);
            if (retVal != HI_SUCCESS)
            {
                currState = AUD_AO_START;
                API_FAIL_DEBUG("startAo",retVal);
                break;
            }

            retVal = bindAoAdec(audioDevCnt, AoChn, AdChn);
            if (retVal != HI_SUCCESS)
            {
                currState = AUD_BIND;
                API_FAIL_DEBUG("bindAoAdec",retVal);
                break;
            }
        }

        SetAudioVolume(curAudioOutVol);

    }while(0);

    if ((retVal != HI_SUCCESS) && (currState != MAX_AUD_STATE))
    {
        FreeAudResources(currState, audioDevCnt, AoChn, AdChn);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will Deinitialize audio configuration
 * @return
 */
static HI_S32 stopAudioOut(HI_VOID)
{
    AO_CHN          AoChn = 0;
    ADEC_CHN        AdChn = 0;
    HI_S32          retVal = HI_SUCCESS;
    AUDIO_DEVICE_e  audioDevCnt = AUDIO_AO_DEV;

    retVal = stopAudioDec(AdChn);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("stopAudioDec",retVal);
        retVal = HI_FAILURE;
    }

    for(audioDevCnt = AUDIO_AO_DEV; audioDevCnt < MAX_AUDIO_DEVICE; audioDevCnt++)
    {
        retVal = stopAo((AUDIO_DEV)audioDevCnt, stAioAttr.u32ChnCnt, gs_bAioReSample[audioDevCnt], HI_FALSE);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("stopAo",retVal);
            break;
        }

        retVal = unbindAoAdec(audioDevCnt, AoChn, AdChn);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("unbindAoAdec",retVal);
            break;
        }
    }

    return retVal;
}


//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to enable Ai channel and Ai Device.
 * @param   AiDevId
 * @param   s32AiChnCnt
 * @param   pstAioAttr
 * @param   enOutSampleRate
 * @param   bResampleEn
 * @param   pstAiVqeAttr
 * @param   u32AiVqeType
 * @return
 */
static HI_S32 startAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt, AIO_ATTR_S* pstAioAttr,
                      AUDIO_SAMPLE_RATE_E enOutSampleRate, HI_BOOL bResampleEn, HI_VOID* pstAiVqeAttr, HI_U32 u32AiVqeType)
{
    HI_S32 i;
    HI_S32 retVal;

    if (pstAioAttr->u32ClkChnCnt == 0)
    {
        pstAioAttr->u32ClkChnCnt = pstAioAttr->u32ChnCnt;
    }

    retVal = HI_MPI_AI_SetPubAttr(AiDevId, pstAioAttr);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AI_SetPubAttr",retVal);
        return retVal;
    }

    retVal = HI_MPI_AI_Enable(AiDevId);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AI_Enable",retVal);
        return retVal;
    }

    for (i = 0; i < s32AiChnCnt; i++)
    {
        retVal = HI_MPI_AI_EnableChn(AiDevId, i);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_AI_EnableChn",retVal);
            return retVal;
        }

        if (HI_TRUE == bResampleEn)
        {
            retVal = HI_MPI_AI_EnableReSmp(AiDevId, i, enOutSampleRate);
            if (retVal != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_AI_EnableReSmp",retVal);
                return retVal;
            }
        }

        if (NULL == pstAiVqeAttr)
        {
            continue;
        }

        HI_BOOL bAiVqe = HI_TRUE;
        switch (u32AiVqeType)
        {
            case 0:
                retVal = HI_SUCCESS;
                bAiVqe = HI_FALSE;
                break;

            case 1:
                retVal = HI_MPI_AI_SetVqeAttr(AiDevId, i, TEST_AUDIO_HDMI_AO_DEV, i, (AI_VQE_CONFIG_S *)pstAiVqeAttr);
                break;

            default:
                retVal = HI_FAILURE;
                break;
        }

        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_AI_SetVqeAttr",retVal);
            return retVal;
        }

        if (bAiVqe)
        {
            retVal = HI_MPI_AI_EnableVqe(AiDevId, i);
            if (retVal != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_AI_EnableVqe",retVal);
                return retVal;
            }
        }
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used for bind Audio in with audio out device.
 * @param   AiDev
 * @param   AiChn
 * @param   AoDev
 * @param   AoChn
 * @return
 */
static HI_S32 bindAoAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn)
{
    MPP_CHN_S stSrcChn,stDestChn;
    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32ChnId = AiChn;
    stSrcChn.s32DevId = AiDev;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;
    /*Binds a data source and a data receiver. */
    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used for unbind Audio in with audio out device.
 * @param   AiDev
 * @param   AiChn
 * @param   AoDev
 * @param   AoChn
 * @return
 */
static HI_S32 unbindAoAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn)
{
    MPP_CHN_S stSrcChn,stDestChn;
    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32ChnId = AiChn;
    stSrcChn.s32DevId = AiDev;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;
    /*Unbind a data source from a data receiver. */
    return HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to disable Ai channel and Ai Device.
 * @param   AiDevId
 * @param   s32AiChnCnt
 * @param   bResampleEn
 * @param   bVqeEn
 * @return
 */
HI_S32 stopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt, HI_BOOL bResampleEn, HI_BOOL bVqeEn)
{
    HI_S32 i;
    HI_S32 retVal;

    for (i = 0; i < s32AiChnCnt; i++)
    {
        if (HI_TRUE == bResampleEn)
        {
            retVal = HI_MPI_AI_DisableReSmp(AiDevId, i);
            if (retVal != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_AI_DisableReSmp",retVal);
                return retVal;
            }
        }

        if (HI_TRUE == bVqeEn)
        {
            retVal = HI_MPI_AI_DisableVqe(AiDevId, i);
            if (retVal != HI_SUCCESS)
            {
                API_FAIL_DEBUG("HI_MPI_AI_DisableVqe",retVal);
                return retVal;
            }
        }

        retVal = HI_MPI_AI_DisableChn(AiDevId, i);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_AI_DisableChn",retVal);
            return retVal;
        }
    }

    retVal = HI_MPI_AI_Disable(AiDevId);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AI_Disable",retVal);
        return retVal;
    }

    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize audio configuration for Test Application
 * @param   audDev
 * @return
 */
BOOL StartAudioOutTest(AUDIO_TEST_DEVICE_e audDev)
{
    HI_S32                  retVal  = HI_SUCCESS;
    HI_S32                  s32AiChnCnt;
    AUDIO_DEV               AiDev   = TEST_AUDIO_AI_DEV;
    AI_CHN                  AiChn   = 0;
    AO_CHN                  AoChn   = 0;
    AUDIO_TEST_DEVICE_e     audioDevCnt ,startAudDevCnt,stopAudDevCnt;
    AUDIO_RESAMPLE_ATTR_S   stTestAoReSampleAttr;

    stTestAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;/* sample rate */
    stTestAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;/* bitwidth */
    stTestAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;/* master or slave mode */
    stTestAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;/* mono or steror */
    stTestAioAttr.u32EXFlag      = 1;/* expand 8bit to 16bit */
    stTestAioAttr.u32FrmNum      = AUDIO_FRAME_BUFF_CNT_MAX;/* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    stTestAioAttr.u32PtNumPerFrm = DEFAULT_AUDIO_PLAY_FRAME_LEN;/* point num per frame (80/160/240/320/480/1024/2048) (ADPCM IMA should add 1 point, AMR only support 160) */
    stTestAioAttr.u32ChnCnt      = 1;
    stTestAioAttr.u32ClkChnCnt   = 2;
    stTestAioAttr.u32ClkSel      = 0; /* 0: AI and AO clock is separate 1: AI and AO clock is inseparate, AI use AO's clock */
    gs_pstTestAoReSmpAttr = NULL;

    switch(audDev)
    {
        case AUDIO_TEST_AO_DEV:
            startAudDevCnt = AUDIO_TEST_AO_DEV;
            stopAudDevCnt = AUDIO_TEST_HDMI_AO_DEV;
            break;

        case AUDIO_TEST_HDMI_AO_DEV:
            startAudDevCnt = AUDIO_TEST_HDMI_AO_DEV;
            stopAudDevCnt = AUDIO_TEST_BOTH_AO_DEV;
            break;

        case AUDIO_TEST_BOTH_AO_DEV:
            startAudDevCnt = AUDIO_TEST_AO_DEV;
            stopAudDevCnt = AUDIO_TEST_BOTH_AO_DEV;
            break;

        default:
            return FAIL;
    }

    retVal = audioCfgAcodec(stTestAioAttr.enSamplerate);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("audioCfgAcodec",retVal);
        return FAIL;
    }

    stTestAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_48000;/* sample rate */
    stTestAoReSampleAttr.u32InPointNum = DEFAULT_AUDIO_PLAY_FRAME_LEN;/* input point number of frame */
    stTestAoReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_8000;/* input sample rate */
    stTestAoReSampleAttr.enOutSampleRate = AUDIO_SAMPLE_RATE_48000;/* output sample rate */
    gs_pstTestAoReSmpAttr = &stTestAoReSampleAttr;
    s32AiChnCnt = stTestAioAttr.u32ChnCnt;

    retVal = startAi(AiDev, s32AiChnCnt, &stTestAioAttr, AUDIO_SAMPLE_RATE_BUTT, HI_FALSE, NULL, 0);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("startAi",retVal);
        return FAIL;
    }

    for(audioDevCnt = startAudDevCnt; audioDevCnt < stopAudDevCnt; audioDevCnt++)
    {
        retVal = startAo((AUDIO_DEV)audioDevCnt, stTestAioAttr.u32ChnCnt, &stTestAioAttr, gs_pstTestAoReSmpAttr->enInSampleRate, HI_TRUE, NULL, 0);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("startAo",retVal);
            break;
        }

        retVal = bindAoAi(AiDev, AiChn, (AUDIO_DEV)audioDevCnt, AoChn);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("bindAoAi",retVal);
            break;
        }

        retVal = HI_MPI_AO_SetVolume((AUDIO_DEV)audioDevCnt, 6);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("HI_MPI_AO_SetVolume",retVal);
        }
    }

    if (retVal != HI_SUCCESS)
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will DeInitialize audio configuration done for Test Application
 * @param   audDev
 * @return
 */
BOOL StopAudioOutTest(AUDIO_TEST_DEVICE_e audDev)
{
    AUDIO_DEV           AiDev = TEST_AUDIO_AI_DEV;
    AI_CHN              AiChn = 0;
    AO_CHN              AoChn = 0;
    HI_S32              retVal, s32AiChnCnt, s32AoChnCnt;
    AUDIO_TEST_DEVICE_e audioDevCnt ,startAudDevCnt,stopAudDevCnt;

    switch(audDev)
    {
        case AUDIO_TEST_AO_DEV:
            startAudDevCnt = AUDIO_TEST_AO_DEV;
            stopAudDevCnt = AUDIO_TEST_HDMI_AO_DEV;
            break;

        case AUDIO_TEST_HDMI_AO_DEV:
            startAudDevCnt = AUDIO_TEST_HDMI_AO_DEV;
            stopAudDevCnt = AUDIO_TEST_BOTH_AO_DEV;
            break;

        case AUDIO_TEST_BOTH_AO_DEV:
            startAudDevCnt = AUDIO_TEST_AO_DEV;
            stopAudDevCnt = AUDIO_TEST_BOTH_AO_DEV;
            break;

        default:
            return FAIL;
    }

    for(audioDevCnt = startAudDevCnt; audioDevCnt < stopAudDevCnt; audioDevCnt++)
    {
        retVal = unbindAoAi(AiDev, AiChn,(AUDIO_DEV) audioDevCnt, AoChn);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("unbindAoAi",retVal);
        }
    }

    s32AiChnCnt = stTestAioAttr.u32ChnCnt;
    retVal = stopAi(AiDev, s32AiChnCnt, HI_FALSE, HI_FALSE);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("stopAi",retVal);
    }

    s32AoChnCnt = stTestAioAttr.u32ChnCnt;
    for(audioDevCnt = startAudDevCnt; audioDevCnt < stopAudDevCnt; audioDevCnt++)
    {
        retVal = stopAo((AUDIO_DEV)audioDevCnt, s32AoChnCnt, HI_TRUE, HI_FALSE);
        if (retVal != HI_SUCCESS)
        {
            API_FAIL_DEBUG("stopAo",retVal);
        }
    }

    if (retVal != HI_SUCCESS)
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will set TV adjust offset value.
 * @param   offset
 * @return
 */
void SetTVAdjustOffset(UINT32 offset)
{
    DPRINT(GUI_LIB,"Setting TV Adjust:[old=%d][new=%d]",tvAdjustOffset,offset);
    tvAdjustOffset = offset;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetMaxDecodingCapacity
 * @return
 */
UINT32 GetMaxDecodingCapacity(void)
{
    return MAX_DEC_CAPABILITY;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Configure audio codec, start Ai and Aenc.
 * @return
 */
BOOL StartAudioIn(void)
{
    HI_S32          retVal = HI_FAILURE;
    AIO_ATTR_S      AioAttr;
    AUDIO_DEV       AiDev = AUDIO_AI_DEV;
    AI_CHN          AiChn = AUDIO_AI_CHN;
    AI_CHN_PARAM_S  AiChnPara;

    DPRINT(GUI_LIB, "AudioIn: request to start audio in");

    /* set audio in-out attributes */
    AioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    AioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    AioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    AioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    AioAttr.u32EXFlag      = 1;     /* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit) */
    AioAttr.u32FrmNum      = AUDIO_FRAME_BUFF_CNT_MAX;    /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    AioAttr.u32PtNumPerFrm = DEFAULT_AUDIO_PLAY_FRAME_LEN; /* point num per frame (80/160/240/320/480/1024/2048) */
    AioAttr.u32ChnCnt      = AIN_CHN_COUNT;
    AioAttr.u32ClkChnCnt   = 2;
    AioAttr.u32ClkSel      = 0;

    /* config audio codec. NOTE: if audio out is going on codec configuration will be reset */
    retVal = audioCfgAcodec(AioAttr.enSamplerate);
    if (retVal != HI_SUCCESS)
    {
        EPRINT(GUI_LIB, "AudioIn: failed while configuring audio codec: [err= %x]", retVal);
        return FAIL;
    }

    do
    {
        if (audioInRunStatus != FALSE)
        {
            /* stop audio in first */
            EPRINT(GUI_LIB, "AudioIn: audio in is already on");
            StopAudioIn();
        }

        /* start Ai(audio input) */
        retVal = startAi(AiDev, AioAttr.u32ChnCnt, &AioAttr, AUDIO_SAMPLE_RATE_BUTT, HI_FALSE, NULL, 0);
        if (retVal != HI_SUCCESS)
        {
            EPRINT(GUI_LIB, "failed while starting Ai: [err= %x]", retVal);
            break;
        }

        /* start Aenc(audio encoder) */
        retVal = startAenc(AUDIO_AENC_CHN, DEFAULT_AUDIO_PLAY_FRAME_LEN, gs_enPayloadType_Ai);
        if (retVal != HI_SUCCESS)
        {
            EPRINT(GUI_LIB, "failed while starting Ai: [err= %x]", retVal);
            break;
        }

        /* set Ai channel  attributes */
        retVal = HI_MPI_AI_GetChnParam(AiDev, AiChn, &AiChnPara);
        if (HI_SUCCESS != retVal)
        {
            API_FAIL_DEBUG("HI_MPI_AI_GetChnParam", retVal)
            break;
        }

        AiChnPara.u32UsrFrmDepth = AUDIO_FRAME_BUFF_CNT_MAX; /* depth of the buffer for storing audio frames */

        retVal = HI_MPI_AI_SetChnParam(AiDev, AiChn, &AiChnPara);
        if (HI_SUCCESS != retVal)
        {
            API_FAIL_DEBUG("HI_MPI_AI_SetChnParam", retVal)
            break;
        }

        /* get audio fd to read audio data */
        audioInFd = HI_MPI_AI_GetFd(AiDev, AiChn);
        if (audioInFd < 0)
        {
            EPRINT(GUI_LIB, "AudioIn: invalid audio channel fd: [fd= %d]", audioInFd);
            break;
        }

        /* set audio in run  status TRUE */
        audioInRunStatus = TRUE;
        retVal = HI_SUCCESS;

    }while(0);

    if (retVal != HI_SUCCESS)
    {
        /* stop audio in and free resources */
        StopAudioIn();
        return FAIL;
    }

    DPRINT(GUI_LIB, "AudioIn: audio in started successfully!");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop Ai and Aenc channels, free resources. Set run status to FALSE
 */
void StopAudioIn(void)
{
    HI_S32  retVal = HI_SUCCESS;

    if ((retVal = stopAenc(AUDIO_AENC_CHN)) != HI_SUCCESS)
    {
        EPRINT(GUI_LIB, "AudioIn: error in stop Aenc: [err= %x]", retVal);
    }

    if ((retVal = stopAi(AUDIO_AI_DEV, AIN_CHN_COUNT, HI_FALSE, HI_FALSE)) != HI_SUCCESS)
    {
        EPRINT(GUI_LIB, "AudioIn: error in stop Ai: [err= %x]", retVal);
    }

    /* set audio in run status to FALSE */
    audioInRunStatus = FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create Aenc Channel
 * @param   AencChn
 * @param   u32AencPtNumPerFrm
 * @param   enType
 * @return
 */
static HI_S32 startAenc(HI_S32 AencChn, HI_U32 u32AencPtNumPerFrm, PAYLOAD_TYPE_E enType)
{
    HI_S32              s32Ret;
    AENC_CHN_ATTR_S     stAencAttr;
    AENC_ATTR_G711_S    stAencG711;

    if (PT_G711U != enType)
    {
        EPRINT(GUI_LIB, "AudioIn: invalid encoder payload type: [enType= %d]", enType);
        return HI_FAILURE;
    }

    /* set AENC chn attr */
    stAencAttr.enType = enType;
    stAencAttr.u32BufSize = AUDIO_FRAME_BUFF_CNT_MAX;
    stAencAttr.u32PtNumPerFrm = u32AencPtNumPerFrm;
    stAencAttr.pValue = &stAencG711;

    /* create aenc chn*/
    s32Ret = HI_MPI_AENC_CreateChn(AencChn, &stAencAttr);
    if (HI_SUCCESS != s32Ret)
    {
        API_FAIL_DEBUG("HI_MPI_AENC_CreateChn", s32Ret)
        return s32Ret;
    }

    DPRINT(GUI_LIB, "AudioIn: Aenc init success");
    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Destroy Aenc channel
 * @param   AencChnCnt
 * @return
 */
static HI_S32 stopAenc(HI_S32 AencChnCnt)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_AENC_DestroyChn(AencChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        API_FAIL_DEBUG("HI_MPI_AENC_DestroyChn", s32Ret)
        return s32Ret;
    }

    DPRINT(GUI_LIB, "AudioIn: Aenc deinit success");
    return HI_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get next audio frame from Ai-> send it to Aenc-> Get stream from Aenc-> Copy in output buffer
 * @param   pFrameBuf - pointer to output audio stream buffer
 * @param   frameLen - frame length
 * @return
 */
BOOL GetNextAudioInFrame(CHARPTR pFrameBuf, INT32PTR frameLen)
{
    fd_set          readFd;
    INT32           selectStat;
    struct timeval 	rcvFrameTimeout;

    HI_S32          retVal = HI_SUCCESS;
    AUDIO_DEV       AiDev = AUDIO_AI_DEV;
    AI_CHN          AiChn = AUDIO_AI_CHN;
    AENC_CHN        AencChn = AUDIO_AENC_CHN;
    AUDIO_FRAME_S   AiFrame;
    AEC_FRAME_S     AencFrame;
    AUDIO_STREAM_S  AiStream;

    if(audioInFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(GUI_LIB, "AudioIn: invalid audio channel fd: [fd= %d]", audioInFd);
        return FAIL;
    }

    /* Set receive length to zero */
    *frameLen = 0;

    /* Set required timeout */
    rcvFrameTimeout.tv_sec = 1;
    rcvFrameTimeout.tv_usec = 0;

    /* Receive entire message upto timeout */
    FD_ZERO(&readFd);
    FD_SET(audioInFd, &readFd);

    selectStat = select((audioInFd + 1), &readFd, NULL, NULL, &rcvFrameTimeout);
    if(selectStat == NILL)
    {
        /* Select failed */
        EPRINT(GUI_LIB, "AudioIn: select fail: [fd=%d], [err=%s]", audioInFd, strerror(errno));
        return FAIL;
    }
    else if(selectStat == STATUS_OK)
    {
        /* Timeout occurred */
        EPRINT(GUI_LIB, "AudioIn: frame recv timeout : [fd=%d], [err=%s]", audioInFd, strerror(errno));
        return FAIL;
    }

    if(FD_ISSET(audioInFd, &readFd) != TRUE)
    {
        EPRINT(GUI_LIB, "AudioIn: fd is not set: [fd=%d], [err=%s]", audioInFd, strerror(errno));
        return FAIL;
    }

    /* get frame from Ai */
    retVal = HI_MPI_AI_GetFrame(AiDev, AiChn, &AiFrame, &AencFrame, 0);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AI_GetFrame", retVal);
        return FAIL;
    }

    /* send frame to Aenc */
    retVal = HI_MPI_AENC_SendFrame(AencChn, &AiFrame, &AencFrame);
    if(retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AENC_SendFrame", retVal);
        return FAIL;
    }

    /* release frame from Ai */
    retVal = HI_MPI_AI_ReleaseFrame(AiDev, AiChn, &AiFrame, &AencFrame);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AI_ReleaseFrame", retVal);
        return FAIL;
    }

    /* get stream from Aenc */
    retVal = HI_MPI_AENC_GetStream(AencChn, &AiStream, 0);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AENC_GetStream", retVal);
        return FAIL;
    }

    /* copy stream to output buffer. NOTE: skip 4 bytes for encoder header */
    memcpy(pFrameBuf, (AiStream.pStream+4), (AiStream.u32Len-4));
    *frameLen = (AiStream.u32Len-4);

    /* release stream from Aenc */
    retVal = HI_MPI_AENC_ReleaseStream(AencChn, &AiStream);
    if (retVal != HI_SUCCESS)
    {
        API_FAIL_DEBUG("HI_MPI_AENC_GetStream", retVal);
        return FAIL;
    }

    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
