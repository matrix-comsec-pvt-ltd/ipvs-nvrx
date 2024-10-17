//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DecDispLib.c
@brief		This file describes interface to rockchip hardware decoder module.
*/
//#################################################################################################
// @DETAILS
//#################################################################################################
/* Library developed based on:
 * RK3568 --> NVR Linux Lite SDK (v1.3.1) and RKMPI SDK (v1.5.1)
 * RK3588 --> NVR Linux Lite SDK (v1.5.0) and RKMPI SDK (v1.9.1) (No patches added)
 *
 * Below patches added on top of the RKMPI SDK (v1.5.1)
 * 1. librockit and header files provided as a part of ticket resolution (#351215):
 *    File Name: rockit_release_rk356x_2205121538.tar.gz
 *    File Path: https://redmine.rock-chips.com/issues/351215#note-9
 *
 * 2. librockchip_mpp provided as a part of ticket resolution (#344178):
 *    File Name: librockchip_mpp.so.1
 *    File Path: https://redmine.rock-chips.com/issues/344178#note-11
 *
 * 3. libgraphic_lsf provided as a part of ticket resolution (#346065):
 *    File Name: libgraphic_lsf.so
 *    File Path: https://redmine.rock-chips.com/issues/346065#note-38
 *
 * Usefull commands:
 * GPU Utilization  : cat /sys/devices/platform/fde60000.gpu/utilisation
 * DDR Frequency    : cat /sys/class/devfreq/dmc/cur_freq
 *
 * Rockchip has given patch for QT linuxfb (we have modified as per our requirement) to improve
 * performance with 4K display. Earlier we were using eglfs_kms platform.
 *
 * RKMPI: Rockchip Media Process Interface
 * VDEC: Video decoder module --> Responsible for video stream decoding and passing to next module
 * VO: Video output module --> Responsible for rendering video and GUI on the display
 * VPSS: Video processing subsystem --> Responsible for intermediate video alteration (e.g crop function)
 *
 * dispChnlId: Logical identity of decoding library for GUI application to perform video decoding and display
 * decoderId: Physical decoder id (VDEC channel) of RKMPI
 * windowId: Logical display separation id (VO channel) of RKMPI
 *
 * In hisilicon, dispChnlId and decoderId are same (72 channels). we allocate dispChnlId randomly in
 * round-robin fashion. With this design, we were facing issues in page sequencing. Sometimes We were
 * not getting free dispChnlId during transition of page or getting decoder usage/capacity error.
 *
 * In rockchip, It supports 64 decoders but once we create decoder, it also creates few persistent
 * threads. To avoid this we have to restrict decoderId to 36 same as windowId.
 * dispChnlId = 72 channels
 * decoderId = 36 channels
 * windowId = 36 channels
 *
 * For better user experience, We have bound dispChnlId and windowId. Every windowId will have two dispChnlId.
 * e.g. Following is the initially binding between windowId and dispChnlId:
 * windowId = 0 will have dispChnlId = 0 and dispChnlId = 36
 * windowId = 1 will have dispChnlId = 1 and dispChnlId = 37 so on.
 *
 * If previous we have dispChnlId = 0 allocated for windowId = 0 then now we will allocate dispChnlId = 36 to windowId = 0
 * At a time, we need to decode and display frame for window = 0 either from dispChnlId = 0 or from dispChnlId = 36
 * decoderId will be derived from dispChnlId (dispChnlId % max decoderId 36).
 * So, decoderId = 0 for dispChnlId = 0 and dispChnlId = 36
 *
 * For window swapping type of cases, We will swap dispChnlIds of windowIds.
 * e.g. If we want to swap window 0 and 1 then
 * Before swapping:
 * windowId = 0 will have decoderId = 0, dispChnlId = 0 and dispChnlId = 36
 * windowId = 1 will have decoderId = 1, dispChnlId = 1 and dispChnlId = 37
 * After swapping:
 * windowId = 0 will have decoderId = 1, dispChnlId = 1 and dispChnlId = 37
 * windowId = 1 will have decoderId = 0, dispChnlId = 0 and dispChnlId = 36
 *
 * For audio play, Earlier we connect adec 0 with ao 0 and ao 1 (tee kind of connection. same as hisilicon).
 * In this case, we were facing audio related issues (audio cut, audio sync between ao 0 and ao 1 etc.)
 * Now we changed the connections, we will connect adec 0 to ao 0 and adec 1 to ao 1. and we will send
 * same audio frame twice to adec 0 and adec 1.
 */
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>

/* Application Includes */
#include "DecDispLib.h"

/* Rockchip Decoder Library Includes */
#include "rk_mpi_sys.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_vpss.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_ai.h"
#include "rk_mpi_ao.h"
#include "rk_mpi_adec.h"
#include "rk_mpi_aenc.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_cal.h"

/* Other Library Includes */
#include "xf86drm.h"
#include "xf86drmMode.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define HDMI1_DEVICE_ID                 0
#define VPSS_PER_GRP_CHNL_NUM           VPSS_CHN0
#define VO_LAYER_ID                     0
#define VO_DEVICE_ID                    0
#define AUDIO_AO_CHNL_ID                0
#define AUDIO_AI_DEV_ID                 0
#define AUDIO_AI_CHNL_ID                0
#define AUDIO_AENC_CHNL_ID              0
#define AUDIO_FRAME_RECV_TIMEOUT_MS     1000
#define AUDIO_IN_FRAME_SIZE_MAX         1024
#define AUDIO_OUT_FRAME_SIZE_MAX        320
#define AUDIO_OUT_BUFF_CNT_MAX          8

/* It is recommended to set 4 times of FPS time: e.g 160ms for 25FPS but we need higher timeout for fast playback */
#define VDEC_STREAM_SEND_TIMEOUT_MS     1000    /* 1000ms */

/* In lower resolution, with vdec compression, some of the video porstion cut from bottom and right side.
 * Rockchip has recommended to compress vdec after HD resolution to avoid this situation.
 * It takes more CPU with 36D1 and 32D1+4HD without vdec compression.
 * Hence we have derived below value after checking vdec and video performance. */
#define VDEC_NO_COMPRESS_START_MAX      (200000)

/* Docoder resources */
#if defined(RK3568_NVRL)
#define DECODER_CAPABILITY_MAX          (500 * 1000 * 1000)     /* 500 Mega Pixel */
#define	DEC_DISP_CHN_MAX                72
#define DECODER_WINDOW_MAX              36
#define DEFAULT_WINDOW_LAYOUT           WIND_LAYOUT_6x6_36CH
#else
#define DECODER_CAPABILITY_MAX          (996 * 1000 * 1000)     /* 996 Mega Pixel */
#define	DEC_DISP_CHN_MAX                128
#define DECODER_WINDOW_MAX              64
#define DEFAULT_WINDOW_LAYOUT           WIND_LAYOUT_8x8_64CH
#endif
#define GET_DECODER_ID(dispChnlId)      (dispChnlId % DECODER_WINDOW_MAX)
#define GET_DISP_CHNL_ID(dispChnlId)    ((dispChnlId < DECODER_WINDOW_MAX) ? (dispChnlId + DECODER_WINDOW_MAX) : (dispChnlId - DECODER_WINDOW_MAX))

/* Get best value after comparision */
#define GET_VPSS_VALUE(value, min, max) value = (value <= min) ? ALIGN_BACK((2 * min), 8) : ((value > max) ? ALIGN_BACK(max, 8) : value)

/* Transform vo resolution from 4K to FHD for UI */
#define IS_UI_VO_TO_RESTRICT(voRes)     (voRes > VO_OUTPUT_1080P60)

/* macro to print API name in debug on failure */
#define API_FAIL_DEBUG(mpiName, s32Ret) EPRINT(GUI_LIB, "%s failed: [s32Ret=0x%#x]", mpiName, s32Ret);

//#################################################################################################
// @DATA TYPE
//#################################################################################################
/* Enum Contain list of Audio Device */
typedef enum
{
    AUDIO_OUT_DEVICE_HDMI = 0,
    AUDIO_OUT_DEVICE_NVR,
    AUDIO_OUT_DEVICE_MAX
}AUDIO_DEVICE_e;

typedef enum
{
    /* Available for allocation */
    DISP_CHNL_STS_READY = 0,

    /* Channel is allocated for use */
    DISP_CHNL_STS_ACTIVE,

    /* Channel is inactive and not usable for live-view. Ready for free */
    DISP_CHNL_STS_INACTIVE,

    /* Channel is inprogress to free resource and will be ready soon */
    DISP_CHNL_STS_READY_WAIT
}DISP_CHNL_STS_e;

typedef struct
{
    DISPLAY_RESOLUTION_e    dispResolution;
    DISPLAY_RESOLUTION_e    configResolution;
    WIND_LAYOUT_ID_e        windowLayout;
}DISPLAY_INFO_t;

// structure which contains information of decoder as well as display instance
typedef struct
{
    DISP_CHNL_STS_e     chnlSts;
    BOOL                iFrameRecv;
    UINT16              noOfReferanceFrame;
    STREAM_CODEC_TYPE_e vidCodec;
    UINT16              frameWidth;
    UINT16              frameHeight;
    UINT16              frameRate;          //for configured fps of camera or playback
    pthread_mutex_t     decDispParamMutex;
}DEC_DISP_LIST_t;

typedef struct
{
    BOOL                status;
    UINT8               dispChnlId;
}DISP_CHNL_WIND_MAP_t;

typedef struct
{
    UINT32              startX[DECODER_WINDOW_MAX];
    UINT32              startY[DECODER_WINDOW_MAX];
    UINT32              winHeight[DECODER_WINDOW_MAX];
    UINT32              winWidth[DECODER_WINDOW_MAX];
    UINT32              totalNumberWin;
}WIN_GEOMETRY_PARAM_t;

// structure which contains information of Audio buffer
typedef struct
{
    BOOL                isAudioInInit;
    BOOL                isAudioOutInit;
    UINT8               audioOutChnl;
    UINT8               audioOutVolume;
    AUDIO_STATE_e       audioOutState;
    pthread_mutex_t     audioOutChnlMutex;
}AUDIO_STREAM_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static DEC_DISP_LIST_t		decDispList[DEC_DISP_CHN_MAX];
static pthread_mutex_t		dispChnlWindMapMutex = PTHREAD_MUTEX_INITIALIZER;
static DISP_CHNL_WIND_MAP_t dispChnlWindMap[DECODER_WINDOW_MAX];
static DISPLAY_INFO_t       displayInfo;
static UINT32               tvAdjustOffset = 0;
static AUDIO_STREAM_t       audioStream;
static pthread_mutex_t      currentUsageMutex = PTHREAD_MUTEX_INITIALIZER;
static UINT32               decoderCurrentUsage = 0;
static VO_INTF_SYNC_E       currVoResolution = VO_OUTPUT_BUTT;
static BOOL                 reinitHdmiParamF = FALSE;
static BOOL                 hdmiThreadRunStatusF;
static pthread_cond_t 		hdmiHandlerSignal;
static pthread_mutex_t		hdmiHandlerLock;

static const RK_CODEC_ID_E rkVideoCodecMap[MAX_VIDEO_CODEC] =
{
    RK_VIDEO_ID_Unused,
    RK_VIDEO_ID_MJPEG,
    RK_VIDEO_ID_AVC,
    RK_VIDEO_ID_MPEG4,
    RK_VIDEO_ID_Unused,
    RK_VIDEO_ID_HEVC
};

static const RK_CODEC_ID_E rkAudioCodecMap[MAX_AUDIO_CODEC] =
{
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_PCM_MULAW,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_Unused,
    RK_AUDIO_ID_PCM_ALAW
};

/* For dividing total height and width for single window geometry */
static const UINT8 winMultiple[WIND_LAYOUT_MAX] = {1, 2, 3, 4, 4, 3, 4, 5, 4, 4, 5, 5, 4, 5, 6, 8, 1, 2, 3, 4};

/* Table containing the maximum No. of window on layout & maximum No. of instant possible on Sequence */
static const UINT8 maxWindPerLayout[WIND_LAYOUT_MAX] = {1, 4, 6, 7, 8, 9, 10, 10, 13, 13, 13, 14, 16, 25, 36, 64, 1, 4, 9, 16};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void resetDecoderDispList(DEC_DISP_PLAY_ID dispChnlId);
//-------------------------------------------------------------------------------------------------
static BOOL isVoSinkConnected(void);
//-------------------------------------------------------------------------------------------------
static void negotiateHdmiParam(void);
//-------------------------------------------------------------------------------------------------
static void *hdmiDetectionHandler(void *data);
//-------------------------------------------------------------------------------------------------
static void hdmiEventHandler(RK_VOID *pPrivateData);
//-------------------------------------------------------------------------------------------------
static BOOL isDisplayConnected(void);
//-------------------------------------------------------------------------------------------------
static BOOL getHdmiCapability(BOOL needOnlyConnSts, BOOL *pIsHdmiConnected, BOOL *pVoDispRes);
//-------------------------------------------------------------------------------------------------
static BOOL addDecoderUsage(DEC_DISP_PLAY_ID dispChnlId, UINT16 oldHeight, UINT16 oldWidth, UINT16 oldFps, UINT16 newHeight, UINT16 newWidth, UINT16 newFps);
//-------------------------------------------------------------------------------------------------
static void removeDecoderUsage(DEC_DISP_PLAY_ID dispChnlId, UINT16 height, UINT16 width, UINT16 fps);
//-------------------------------------------------------------------------------------------------
static UINT8 getDecoderId(UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static UINT8 getWindowId(DEC_DISP_PLAY_ID dispChnlId);
//-------------------------------------------------------------------------------------------------
static DEC_DISP_PLAY_ID getDispChnlId(UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static BOOL generateWindowGeometry(WIND_LAYOUT_ID_e windLayoutId, WIN_GEOMETRY_PARAM_t *pWinGeometry);
//-------------------------------------------------------------------------------------------------
static void deInitDecoderResources(UINT8 decoderId, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static BOOL initRkMpp(void);
//-------------------------------------------------------------------------------------------------
static BOOL exitRkMpp(void);
//-------------------------------------------------------------------------------------------------
static BOOL initVoModule(VO_INTF_SYNC_E voResolution);
//-------------------------------------------------------------------------------------------------
static RK_S32 createVdecChnl(UINT8 decoderId, VDEC_CHN_ATTR_S *pStVdecChnAttr, VDEC_PIC_BUF_ATTR_S *pStVdecPicBufAttr);
//-------------------------------------------------------------------------------------------------
static RK_S32 deleteVdecChnl(UINT8 decoderId);
//-------------------------------------------------------------------------------------------------
static RK_S32 createVpssGrp(UINT8 VpssGrp, VDEC_CHN_ATTR_S *pStVdecChnAttr);
//-------------------------------------------------------------------------------------------------
static RK_S32 deleteVpssGrp(UINT8 VpssGrp);
//-------------------------------------------------------------------------------------------------
static RK_S32 createVoChnlLayout(WIND_LAYOUT_ID_e windLayoutId);
//-------------------------------------------------------------------------------------------------
static RK_S32 startVoDev(VO_INTF_SYNC_E voResolution);
//-------------------------------------------------------------------------------------------------
static RK_S32 stopVoDev(void);
//-------------------------------------------------------------------------------------------------
static RK_S32 startVoLayer(VO_INTF_SYNC_E voResolution);
//-------------------------------------------------------------------------------------------------
static RK_S32 stopVoLayer(void);
//-------------------------------------------------------------------------------------------------
static RK_S32 bindVdec2Vpss(UINT8 decoderId, UINT8 VpssGrp);
//-------------------------------------------------------------------------------------------------
static RK_S32 unbindVdec2Vpss(UINT8 decoderId, UINT8 VpssGrp);
//-------------------------------------------------------------------------------------------------
static RK_S32 bindVpss2Vo(UINT8 VpssGrp, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static RK_S32 unbindVpss2Vo(UINT8 VpssGrp, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static RK_S32 bindVdec2Vo(UINT8 decoderId, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static RK_S32 unbindVdec2Vo(UINT8 decoderId, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
static BOOL getLiveViewScreenInfo(BOOL isUiScreenInfoReq, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId);
//-------------------------------------------------------------------------------------------------
static BOOL getSyncPlaybackScreenInfo(BOOL isUiScreenInfoReq, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId);
//-------------------------------------------------------------------------------------------------
static void getDisplayWidthHeight(VO_INTF_SYNC_E voResolution, UINT32 *pDispWidth, UINT32 *pDispHeight, UINT32 *pFrameRate);
//-------------------------------------------------------------------------------------------------
static VO_INTF_SYNC_E getNegotiatedVoRkRes(DISPLAY_RESOLUTION_e *pDispResolution);
//-------------------------------------------------------------------------------------------------
static RK_S32 startAudioOut(STREAM_CODEC_TYPE_e audioCodec);
//-------------------------------------------------------------------------------------------------
static RK_S32 stopAudioOut(void);
//-------------------------------------------------------------------------------------------------
static RK_S32 createAdecChnl(AUDIO_DEVICE_e adecChnId, STREAM_CODEC_TYPE_e audioCodec, UINT32 samplingRate, UINT8 channels);
//-------------------------------------------------------------------------------------------------
static RK_S32 deleteAdecChnl(AUDIO_DEVICE_e adecChnId);
//-------------------------------------------------------------------------------------------------
static RK_S32 enableAoChnl(AUDIO_DEVICE_e aoDevId, const AIO_ATTR_S *pStAoAttr);
//-------------------------------------------------------------------------------------------------
static RK_S32 disableAoChnl(AUDIO_DEVICE_e aoDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 bindAdec2Ao(AUDIO_DEVICE_e adecChnId, AUDIO_DEVICE_e aoDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 unbindAdec2Ao(AUDIO_DEVICE_e adecChnId, AUDIO_DEVICE_e aoDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 createAencChnl(void);
//-------------------------------------------------------------------------------------------------
static RK_S32 deleteAencChnl(void);
//-------------------------------------------------------------------------------------------------
static RK_S32 enableAiChnl(UINT8 aiDevId, const AIO_ATTR_S *pStAoAttr);
//-------------------------------------------------------------------------------------------------
static RK_S32 disableAiChnl(UINT8 aiDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 bindAi2Aenc(UINT8 aiDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 unbindAi2Aenc(UINT8 aiDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 bindAi2Ao(UINT8 aiDevId, AUDIO_DEVICE_e aoDevId);
//-------------------------------------------------------------------------------------------------
static RK_S32 unbindAi2Ao(UINT8 aiDevId, AUDIO_DEVICE_e aoDevId);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize display device with default layout and resolution.
 * @param   resolution
 * @return  TRUE on success; FALSE otherwise
 */
BOOL InitDecodeDisplay(DISPLAY_RESOLUTION_e resolution)
{
    UINT8       debugPrintCnt = 0;
    UINT16      chnlIdx;
    pthread_t   threadId;

    while (FAIL == isDisplayConnected())
    {
        /* Avoid frequent debugs */
        if (++debugPrintCnt >= 30)
        {
            debugPrintCnt = 0;
            WPRINT(GUI_LIB, "hdmi display not connected, waiting for display..!!!");
        }

        /* Do not check freqently */
        sleep(1);
    }

    /* Store parameter for future use */
    displayInfo.dispResolution = resolution;
    displayInfo.configResolution = resolution;
    displayInfo.windowLayout = DEFAULT_WINDOW_LAYOUT;

    /* Initialization of Rockchip MPP */
    initRkMpp();

    /* Get negotiated VO channel resolution based on configured resolution */
    currVoResolution = getNegotiatedVoRkRes(&displayInfo.dispResolution);

    /* Create VO channel with neogitated resolution */
    if (FAIL == initVoModule(currVoResolution))
    {
        /* Failed to init VO module */
        EPRINT(GUI_LIB, "failed to init vo module");
        return FAIL;
    }

    for (chnlIdx = 0; chnlIdx < DEC_DISP_CHN_MAX; chnlIdx++)
    {
        //Initially setting default parameter
        GUI_LIB_MUTEX_INIT(decDispList[chnlIdx].decDispParamMutex, NULL);
        decDispList[chnlIdx].chnlSts = DISP_CHNL_STS_READY;
        resetDecoderDispList(chnlIdx);
    }

    for (chnlIdx = 0; chnlIdx < DECODER_WINDOW_MAX; chnlIdx++)
    {
        /* Reset param with default */
        dispChnlWindMap[chnlIdx].status = FREE;
        dispChnlWindMap[chnlIdx].dispChnlId = GET_DISP_CHNL_ID(chnlIdx);
    }

    /* Init audio stream parameters */
    audioStream.isAudioInInit = FALSE;
    audioStream.isAudioOutInit = FALSE;
    audioStream.audioOutChnl = INVALID_DEC_DISP_PLAY_ID;
    audioStream.audioOutVolume = 40;
    audioStream.audioOutState = MAX_AUDIO_STATE;
    GUI_LIB_MUTEX_INIT(audioStream.audioOutChnlMutex, NULL);

    /* Init HDMI thread variables */
    hdmiThreadRunStatusF = TRUE;
    GUI_LIB_MUTEX_INIT(hdmiHandlerLock, NULL);
    pthread_cond_init(&hdmiHandlerSignal, NULL);

    /* Initialise thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (512 * KILO_BYTE));

    /* Create HDMI detection handler thread */
    pthread_create(&threadId, &attr, &hdmiDetectionHandler, NULL);

    /* Destroy thread attribute */
    pthread_attr_destroy(&attr);

    DPRINT(GUI_LIB, "decoder library initialized: [disp_resolution=%d], [vo_resolution=%d]", resolution, currVoResolution);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function close the decode and display link as well as destroy all the MPP resources
 */
void DeInitDecDispLib(void)
{
    UINT8 windowId;

    /* GUI library deinitialization */
    DPRINT(GUI_LIB, "decoder library deinitialization");

    /* Exit HDMI detection handler thread to send signal */
    hdmiThreadRunStatusF = FALSE;
    GUI_LIB_MUTEX_LOCK(hdmiHandlerLock);
    pthread_cond_signal(&hdmiHandlerSignal);
    GUI_LIB_MUTEX_UNLOCK(hdmiHandlerLock);

    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
    for (windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        deInitDecoderResources(DECODER_WINDOW_MAX, windowId);
    }
    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);

    /* Stop the VO Layer */
    stopVoLayer();

    /* Stop the VO device */
    stopVoDev();

    /* Deinitializes the rkmpp system */
    exitRkMpp();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This callback will trigger on HDMI status change (connect --> disconnect and vice-versa)
 * @param   pHdmiCallBack
 */
void HdmiRegCallBack(HDMI_CALLBACK pHdmiCallBack)
{
    /* Nothing to do for rockchip */
}

/**
 * @brief   Check whether HDMI display connected with or not with NVR
 * @return  TRUE if display connected with NVR otherwise FALSE
 */
BOOL isDisplayConnected(void)
{
    BOOL isHdmiConnected = FAIL;

    if (FAIL == getHdmiCapability(TRUE, &isHdmiConnected, NULL))
    {
        EPRINT(GUI_LIB, "fail to get hdmi connection status");
        return FAIL;
    }

    /* We got the display connection status */
    return isHdmiConnected;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCurrentMainResolution
 * @param   pResolution
 * @param   pDispWidth
 * @param   pDispHeight
 * @param   pFrameRate
 * @return  BOOL
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

    *pResolution = displayInfo.dispResolution;
    getDisplayWidthHeight(currVoResolution, pDispWidth, pDispHeight, pFrameRate);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   calculate the actual screen height width in multiple of 8.
 * @param   dispDevId
 * @param   pScreenInfo
 * @param   windLayoutId
 * @return
 */
BOOL GetValidScreenInfoPlayBack(DISPLAY_DEV_e dispDevId, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId)
{
    return getSyncPlaybackScreenInfo(TRUE, pScreenInfo, windLayoutId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   calculate the actual screen height width in multiple of 8.
 * @param   dispDevId
 * @param   pScreenInfo
 * @param   windLayoutId
 * @return
 */
BOOL GetValidScreenInfo(DISPLAY_DEV_e dispDevId, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId)
{
    return getLiveViewScreenInfo(TRUE, pScreenInfo, windLayoutId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   change the state of any channel on any display device .
 * @param   dispChnlId
 * @param   dispDevId
 * @param   windowId
 * @return
 */
BOOL UpdateDecoderStatusOnVideoLoss(DEC_DISP_PLAY_ID dispChnlId, DISPLAY_DEV_e dispDevId, UINT8 windowId)
{
    if ((dispDevId >= MAX_DISPLAY_DEV) || (dispChnlId >= DEC_DISP_CHN_MAX))
    {
        EPRINT(GUI_LIB, "invld input param: [dispDevId=%d], [dispChnlId=%d], [windowId=%d]", dispDevId, dispChnlId, windowId);
        return FAIL;
    }

    /* Lock and update the status */
    GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
    decDispList[dispChnlId].iFrameRecv = FALSE;
    GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function find the free display channel. The display channel is a logical entity.
 *          This function starts rendering on given display device and window number.
 * @param   dispDevId
 * @param   windowId
 * @param   pDispChnlId
 * @return
 */
BOOL StartChannelView(DISPLAY_DEV_e dispDevId, UINT8 windowId, DEC_DISP_PLAY_ID *pDispChnlId)
{
    UINT8               decoderId;
    DEC_DISP_PLAY_ID    dispChnlId;

    /* Validate input params */
    if (pDispChnlId == NULL)
    {
        EPRINT(GUI_LIB, "null pDispChnlId ptr found: [windowId=%d]", windowId);
        return FAIL;
    }

    /* Validate input params */
    if ((dispDevId >= MAX_DISPLAY_DEV) || (windowId >= DECODER_WINDOW_MAX))
    {
        EPRINT(GUI_LIB, "invld input params found: [dispDevId=%d], [windowId=%d]", dispDevId, windowId);
        *pDispChnlId = INVALID_DEC_DISP_ID;
        return FAIL;
    }

    /* Get deocder if bound with given window */
    decoderId = getDecoderId(windowId);
    if (decoderId < DECODER_WINDOW_MAX)
    {
        /* Get display channel from window */
        GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
        dispChnlId = dispChnlWindMap[windowId].dispChnlId;

        /* Sanity check, Ideally it should not happen */
        if (decoderId != GET_DECODER_ID(dispChnlId))
        {
            WPRINT(GUI_LIB, "older decoder mismatch: [dispChnlId=%d], [windowId=%d], [decoderId=%d]", dispChnlId, windowId, decoderId);
        }

        /* Mark this display channel as not active for live-view */
        GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
        if (decDispList[dispChnlId].chnlSts == DISP_CHNL_STS_ACTIVE)
        {
            decDispList[dispChnlId].chnlSts = DISP_CHNL_STS_INACTIVE;
        }
        GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);

        /* Get and store new display channel information */
        dispChnlId = GET_DISP_CHNL_ID(dispChnlId);
        dispChnlWindMap[windowId].dispChnlId = dispChnlId;
        dispChnlWindMap[windowId].status = BUSY;

        /* Free occupied decoder resources and update usage */
        deInitDecoderResources(decoderId, windowId);
        GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
    }
    else
    {
        /* Display channel is not bound with any window. Get and store new display channel information */
        GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
        dispChnlId = GET_DISP_CHNL_ID(dispChnlWindMap[windowId].dispChnlId);
        dispChnlWindMap[windowId].dispChnlId = dispChnlId;
        dispChnlWindMap[windowId].status = BUSY;
        GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
    }

    /* Occupy display channel for given window if available */
    GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
    if (decDispList[dispChnlId].chnlSts != DISP_CHNL_STS_READY)
    {
        /* Free display channel not found. Ideally it should not happen */
        GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
        EPRINT(GUI_LIB, "free display channel not found: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, windowId, decoderId);
        *pDispChnlId = INVALID_DEC_DISP_ID;
        return FAIL;
    }

    /* Mark this channel as occupied */
    decDispList[dispChnlId].chnlSts = DISP_CHNL_STS_ACTIVE;
    GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);

    /* Return acquired display channel */
    *pDispChnlId = dispChnlId;
    DPRINT(GUI_LIB, "display channel assigned: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops video for given display channel. This function stops rendering on
 *          given display device and window number And also stops decoding channel.
 * @param   dispChnlId
 * @return
 */
BOOL StopChannelView(DEC_DISP_PLAY_ID dispChnlId)
{
    UINT8   decoderId;
    UINT8   windowId;
    UINT16  frameHeight;
    UINT16  frameWidth;
    UINT16  frameRate;

    if (dispChnlId >= DEC_DISP_CHN_MAX)
    {
        EPRINT(GUI_LIB, "invld display channel: [dispChnlId=%d]", dispChnlId);
        return FAIL;
    }

    /* Nothing to do if display channel is already freed or in-progress */
    GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
    if ((decDispList[dispChnlId].chnlSts == DISP_CHNL_STS_READY) || (decDispList[dispChnlId].chnlSts == DISP_CHNL_STS_READY_WAIT))
    {
        GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
        return SUCCESS;
    }

    /* Reset display channel params and release related resources */
    decDispList[dispChnlId].chnlSts = DISP_CHNL_STS_READY_WAIT;
    frameHeight = decDispList[dispChnlId].frameHeight;
    frameWidth = decDispList[dispChnlId].frameWidth;
    frameRate = decDispList[dispChnlId].frameRate;
    resetDecoderDispList(dispChnlId);
    GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);

    /* Get decoder and window from display channel */
    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
    windowId = getWindowId(dispChnlId);
    decoderId = GET_DECODER_ID(dispChnlId);
    if (windowId < DECODER_WINDOW_MAX)
    {
        dispChnlWindMap[windowId].status = FREE;
        deInitDecoderResources(decoderId, windowId);
    }
    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);

    /* Update decoder resource usage */
    removeDecoderUsage(dispChnlId, frameHeight, frameWidth, frameRate);

    /* Remove audio if request is for audio display channel */
    if (audioStream.audioOutChnl == dispChnlId)
    {
        ExcludeAudio(dispChnlId);
    }

    GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
    decDispList[dispChnlId].chnlSts = DISP_CHNL_STS_READY;
    GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
    DPRINT(GUI_LIB, "decoding channel released: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is to validate decoder from sync Pb.
 * @param   dispChnlId
 * @return  Returns TRUE if given display channel is bound with any window else returns FALSE
 */
BOOL ValidateDecoderSyncPb(INT8 dispChnlId)
{
    UINT8 windowId;

    switch(displayInfo.windowLayout)
    {
        case WIND_LAYOUT_1X1_1CH:
        case WIND_LAYOUT_1X1_PLAYBACK:
        case WIND_LAYOUT_2X2_4CH:
        case WIND_LAYOUT_2X2_PLAYBACK:
        case WIND_LAYOUT_3X3_9CH:
        case WIND_LAYOUT_3X3_PLAYBACK:
        case WIND_LAYOUT_4X4_16CH:
        case WIND_LAYOUT_4X4_PLAYBACK:
            break;

        default:
            return SUCCESS;
    }

    for (windowId = 0; windowId < maxWindPerLayout[displayInfo.windowLayout]; windowId++)
    {
        /* Match decoder channel which is visible in current layout by checking the association with window */
        if (GET_DECODER_ID(dispChnlId) == getDecoderId(windowId))
        {
            return SUCCESS;
        }
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function use to store frame into decoder buffer for decoding and display. Defines
 *          the attributes of a VDEC channel , Starts to receive the streams sent  by the user,
 * @param   dispChnlId
 * @param   pFrameInfo
 * @param   pDecErrCode
 * @return
 */
BOOL DecodeDispFrame(DEC_DISP_PLAY_ID dispChnlId, FRAME_INFO_t *pFrameInfo, DECODER_ERROR_e *pDecErrCode)
{
    RK_S32          s32Ret;
    BOOL            iFrameRecvF = FALSE;
    UINT8           decoderId;
    UINT8           windowId = DECODER_WINDOW_MAX;
    MB_EXT_CONFIG_S stMbExtConfig;

    /* Validate frame size to display */
    if ((pFrameInfo->frameSize == 0) || (pFrameInfo->mediaType >= MAX_STREAM_TYPE))
    {
        EPRINT(GUI_LIB, "invld frame data: [dispChnlId=%d], [frameSize=%d], [mediaType=%d]", dispChnlId, pFrameInfo->frameSize, pFrameInfo->mediaType);
        *pDecErrCode = DEC_ERROR_INVALID_ARG;
        return FAIL;
    }

    if (pFrameInfo->mediaType == STREAM_TYPE_VIDEO)
    {
        if (dispChnlId >= DEC_DISP_CHN_MAX)
        {
            EPRINT(GUI_LIB, "invld display channel found: [dispChnlId=%d]", dispChnlId);
            *pDecErrCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        if ((pFrameInfo->codecType >= MAX_VIDEO_CODEC) || (rkVideoCodecMap[pFrameInfo->codecType] == RK_VIDEO_ID_Unused))
        {
            EPRINT(GUI_LIB, "invld video codec found: [dispChnlId=%d], [codec=%d]", dispChnlId, pFrameInfo->codecType);
            *pDecErrCode = DEC_ERROR_INVALID_CODEC;
            return FAIL;
        }

        /* Lock the display channel param */
        GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
        if (decDispList[dispChnlId].chnlSts != DISP_CHNL_STS_ACTIVE)
        {
            /* Unlock the display channel param */
            GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
            EPRINT(GUI_LIB, "display channel is not active: [dispChnlId=%d]", dispChnlId);
            *pDecErrCode = DEC_ERROR_NO_FREE_DEC_ID;
            return FAIL;
        }

        /* Get params in locally */
        iFrameRecvF = decDispList[dispChnlId].iFrameRecv;
        decoderId = GET_DECODER_ID(dispChnlId);

        /* Is frame param changed? */
        if((decDispList[dispChnlId].vidCodec != pFrameInfo->codecType)
                || (decDispList[dispChnlId].noOfReferanceFrame != pFrameInfo->noOfReferanceFrame)
                || (decDispList[dispChnlId].frameHeight != pFrameInfo->frameHeight)
                || (decDispList[dispChnlId].frameWidth != pFrameInfo->frameWidth)
                || (decDispList[dispChnlId].frameRate != pFrameInfo->frameRate))
        {
            UINT16 frameWidth = decDispList[dispChnlId].frameWidth;
            UINT16 frameHeight = decDispList[dispChnlId].frameHeight;
            UINT16 frameRate = decDispList[dispChnlId].frameRate;
            STREAM_CODEC_TYPE_e vidCodec = decDispList[dispChnlId].vidCodec;
            UINT16 noOfReferanceFrame = decDispList[dispChnlId].noOfReferanceFrame;
            GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);

            /* Get window from display channel */
            GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
            windowId = getWindowId(dispChnlId);
            GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
            if (windowId >= DECODER_WINDOW_MAX)
            {
                WPRINT(GUI_LIB, "window is not allocated: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
                return SUCCESS;
            }

            DPRINT(GUI_LIB, "frame parameter updated: [dispChnlId=%d], [decoderId=%d], [windowId=%d], [resolution=%dx%d->%dx%d], [reference=%d->%d], [fps=%d->%d]",
                   dispChnlId, decoderId, windowId, frameWidth, frameHeight, pFrameInfo->frameWidth, pFrameInfo->frameHeight,
                   noOfReferanceFrame, pFrameInfo->noOfReferanceFrame, frameRate, pFrameInfo->frameRate);

            /* Add current video usage in decoder usage */
            if (FAIL == addDecoderUsage(dispChnlId, frameHeight, frameWidth, frameRate, pFrameInfo->frameHeight, pFrameInfo->frameWidth, pFrameInfo->frameRate))
            {
                EPRINT(GUI_LIB, "no more decoding capacity: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
                if (((frameHeight > 0) && (frameWidth > 0)) || (noOfReferanceFrame > 0))
                {
                    /* Release decoder and window resources */
                    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
                    deInitDecoderResources(decoderId, windowId);
                    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
                }

                /* Update decoder capacity and make frame resolution and framerate to 0. So next time not delete it */
                removeDecoderUsage(dispChnlId, frameHeight, frameWidth, frameRate);
                GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
                resetDecoderDispList(dispChnlId);
                GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
                *pDecErrCode = DEC_ERROR_NO_CAPACITY;
                return FAIL;
            }

            /* Change in core video params. Hence we have to re-create decoder */
            if ((vidCodec != pFrameInfo->codecType) || (noOfReferanceFrame != pFrameInfo->noOfReferanceFrame)
                    || (frameHeight != pFrameInfo->frameHeight) || (frameWidth != pFrameInfo->frameWidth))
            {
                /* Delete Decoder Instant */
                if (((frameHeight > 0) && (frameWidth > 0)) || (noOfReferanceFrame > 0))
                {
                    /* Release decoder and window resources */
                    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
                    deInitDecoderResources(decoderId, windowId);
                    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
                }

                /* Get picture buffer size */
                MB_PIC_CAL_S        stMbPicCalResult = {0};
                VDEC_PIC_BUF_ATTR_S stVdecPicBufAttr = {0};
                stVdecPicBufAttr.enCodecType = rkVideoCodecMap[pFrameInfo->codecType];
                stVdecPicBufAttr.stPicBufAttr.u32Width = pFrameInfo->frameWidth;
                stVdecPicBufAttr.stPicBufAttr.u32Height = pFrameInfo->frameHeight;
                stVdecPicBufAttr.stPicBufAttr.enPixelFormat = RK_FMT_YUV420SP;

                /* Compression is not allowed in MJPEG. As per rockchip recommendation, Do not compress the video hd and lower resolution video  */
                if ((pFrameInfo->codecType != VIDEO_MJPG) && ((pFrameInfo->frameWidth * pFrameInfo->frameHeight) > VDEC_NO_COMPRESS_START_MAX))
                {
                    /* It will compress the decoded video */
                    stVdecPicBufAttr.stPicBufAttr.enCompMode = COMPRESS_AFBC_16x16;
                }
                else
                {
                    /* It will not compress the decoded video */
                    stVdecPicBufAttr.stPicBufAttr.enCompMode = COMPRESS_MODE_NONE;
                }

                s32Ret = RK_MPI_CAL_VDEC_GetPicBufferSize(&stVdecPicBufAttr, &stMbPicCalResult);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "fail to get picture buffer size: [dispChnlId=%d], [decoderId=%d], [windowId=%d], [s32Ret=0x%x]", dispChnlId, decoderId, windowId, s32Ret);
                    return s32Ret;
                }

                /* Set decoding channel params */
                VDEC_CHN_ATTR_S stVdecChnAttr = {0};
                stVdecChnAttr.enType = rkVideoCodecMap[pFrameInfo->codecType];  /* video type to be decoded */
                stVdecChnAttr.u32PicWidth = pFrameInfo->frameWidth;             /* max pic width */
                stVdecChnAttr.u32PicHeight = pFrameInfo->frameHeight;           /* max pic height */
                stVdecChnAttr.u32FrameBufCnt = 8;                               /* frame buffer pool count */
                stVdecChnAttr.u32StreamBufCnt = 8;                              /* stream buffer pool count */
                stVdecChnAttr.u32FrameBufSize = stMbPicCalResult.u32MBSize;     /* if decode 10bit stream, need to specify the u32FrameBufSize, other conditions can be set to 0, calculated internally */
                stVdecChnAttr.enMode = VIDEO_MODE_FRAME;                        /* Send by stream or by frame */

                /* As per the recommendation in FAQ doc */
                stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = (pFrameInfo->codecType == VIDEO_H265) ? RK_TRUE : RK_FALSE;

                /* Lock for decoder window creation task */
                GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);

                /* Create a VDEC channel with specified video parameters */
                s32Ret = createVdecChnl(decoderId, &stVdecChnAttr, &stVdecPicBufAttr);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "createVdecChnl failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d], [s32Ret=0x%x]", dispChnlId, decoderId, windowId, s32Ret);
                    if (s32Ret == RK_ERR_VDEC_EXIST)
                    {
                        /* Free all decoder related resources */
                        deInitDecoderResources(decoderId, DECODER_WINDOW_MAX);
                    }
                }
                else
                {
                    /* Bind VDEC and VO */
                    s32Ret = bindVdec2Vo(decoderId, windowId);
                    if (s32Ret != RK_SUCCESS)
                    {
                        /* Disables a specified VO channel */
                        RK_MPI_VO_DisableChn(VO_LAYER_ID, windowId);

                        /* Delete VDEC channel */
                        deleteVdecChnl(decoderId);
                        EPRINT(GUI_LIB, "bindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
                    }
                }

                /* Unlock the display channel window map */
                GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);

                /* Is error found in decoder window creation? */
                if (s32Ret != RK_SUCCESS)
                {
                    GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
                    resetDecoderDispList(dispChnlId);
                    GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
                    removeDecoderUsage(dispChnlId, pFrameInfo->frameHeight, pFrameInfo->frameWidth,pFrameInfo->frameRate);
                    *pDecErrCode = DEC_ERROR_CREATION_FAILED;
                    return FAIL;
                }
            }

            DPRINT(GUI_LIB, "decoder created: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);

            /* Set video frame params for future reference */
            GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
            decDispList[dispChnlId].vidCodec = pFrameInfo->codecType;
            decDispList[dispChnlId].frameHeight = pFrameInfo->frameHeight;
            decDispList[dispChnlId].frameWidth = pFrameInfo->frameWidth;
            decDispList[dispChnlId].frameRate  = pFrameInfo->frameRate;
            decDispList[dispChnlId].noOfReferanceFrame = pFrameInfo->noOfReferanceFrame;
            decDispList[dispChnlId].iFrameRecv = iFrameRecvF = FALSE;
            GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
        }
        else
        {
            /* Unlock the display channel param */
            GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
        }

        /* Discard invalid frame */
        if (pFrameInfo->frameType >= MAX_ITYPE)
        {
            /* This is not a valid frame */
            return SUCCESS;
        }

        /* Is I-frame received? */
        if (iFrameRecvF == FALSE)
        {
            /* Wait for I-frame */
            if (pFrameInfo->frameType != I_FRAME)
            {
                /* We are waiting for first I-frame */
                return SUCCESS;
            }

            /* This is first I-frame after decoder creation */
            GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
            decDispList[dispChnlId].iFrameRecv = TRUE;
            GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
        }

        /* Lock the display channel param */
        GUI_LIB_MUTEX_LOCK(decDispList[dispChnlId].decDispParamMutex);
        if (decDispList[dispChnlId].chnlSts != DISP_CHNL_STS_ACTIVE)
        {
            /* Unlock the display channel param */
            GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);
            return SUCCESS;
        }

        /* Unlock the display channel param */
        GUI_LIB_MUTEX_UNLOCK(decDispList[dispChnlId].decDispParamMutex);

        VDEC_STREAM_S stVdecStream;

        /* Create a media block */
        stMbExtConfig.pFreeCB = RK_NULL;
        stMbExtConfig.pOpaque = RK_NULL;
        stMbExtConfig.pu8VirAddr = (RK_U8*)pFrameInfo->framePayload;    /* stream address */
        stMbExtConfig.u64Size = pFrameInfo->frameSize;                  /* stream len */
        RK_MPI_SYS_CreateMB(&stVdecStream.pMbBlk, &stMbExtConfig);

        /* Set VDEC stream params */
        stVdecStream.u64PTS = (((RK_U64)(pFrameInfo->frameTimeSec)) * 1000 * 1000) + (((RK_U64)(pFrameInfo->frameTimeMSec)) * 1000);
        stVdecStream.u32Len = pFrameInfo->frameSize;    /* stream len */
        stVdecStream.bEndOfFrame = RK_TRUE;             /* is the end of a frame */
        stVdecStream.bEndOfStream = RK_FALSE;           /* is the end of all stream */
        stVdecStream.bBypassMbBlk = RK_FALSE;           /* Decoder will create own copy of data */

        /* Send frame to decoder in stream mode */
        s32Ret = RK_MPI_VDEC_SendStream(decoderId, &stVdecStream, VDEC_STREAM_SEND_TIMEOUT_MS);

        /* Free memory block */
        RK_MPI_MB_ReleaseMB(stVdecStream.pMbBlk);

        /* Is error found in send stream? */
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "RK_MPI_VDEC_SendStream failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d], [s32Ret=0x%x]", dispChnlId, decoderId, windowId, s32Ret);
            *pDecErrCode = DEC_ERROR_VDEC;
            return FAIL;
        }
    }
    else
    {
        if (pFrameInfo->codecType >= MAX_AUDIO_CODEC)
        {
            EPRINT(GUI_LIB, "invld audio codec found: [dispChnlId=%d], [codec=%d]", dispChnlId, pFrameInfo->codecType);
            *pDecErrCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        /* Validate supported audio codec type */
        if (RK_AUDIO_ID_Unused == rkAudioCodecMap[pFrameInfo->codecType])
        {
            /* We support only alaw and ulaw codec in local play */
            return SUCCESS;
        }

        /* We have fixed audio out client decoder channel */
        if ((dispChnlId >= DEC_DISP_CHN_MAX) && (dispChnlId != AUDIO_OUT_DEC_CHNL_ID))
        {
            EPRINT(GUI_LIB,"invld audio decoder channel: [dispChnlId=%d]", dispChnlId);
            *pDecErrCode = DEC_ERROR_INVALID_ARG;
            return FAIL;
        }

        /* Lock audio channel and verify decoder channel */
        GUI_LIB_MUTEX_LOCK(audioStream.audioOutChnlMutex);
        if (audioStream.audioOutChnl != dispChnlId)
        {
            GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);
            return SUCCESS;
        }

        /* Create ADEC and AO for audio out */
        if (FALSE == audioStream.isAudioOutInit)
        {
            /* Consider audio is started otherwise volume will not get updated */
            audioStream.isAudioOutInit = TRUE;

            /* Start audio out as it not started yet */
            s32Ret = startAudioOut(pFrameInfo->codecType);
            if (s32Ret != RK_SUCCESS)
            {
                /* Failed to start audio out */
                audioStream.isAudioOutInit = FALSE;
                GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);
                EPRINT(GUI_LIB, "fail to start audio out: [dispChnlId=%d]", dispChnlId);
                *pDecErrCode = DEC_ERROR_AUDIO;
                return FAIL;
            }

            /* Init audio frame play parmas */
            audioStream.audioOutState = AUDIO_ON;
        }

        /* Unlock the audio out mutex */
        GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);

        RK_U8           adecChnId;
        AUDIO_STREAM_S  stAdecStream;

        /* Create a media block */
        stMbExtConfig.pFreeCB = RK_NULL;
        stMbExtConfig.pOpaque = RK_NULL;
        stMbExtConfig.pu8VirAddr = (RK_U8*)pFrameInfo->framePayload;    /* stream address */
        stMbExtConfig.u64Size = pFrameInfo->frameSize;                  /* stream len */
        RK_MPI_SYS_CreateMB(&stAdecStream.pMbBlk, &stMbExtConfig);

        /* Set audio stream params */
        stAdecStream.u32Len = pFrameInfo->frameSize;
        stAdecStream.u64TimeStamp = 0;
        stAdecStream.u32Seq = 0;                /* frame seq, if stream is not a valid frame, u32Seq is 0 */
        stAdecStream.bBypassMbBlk = RK_FALSE;   /* Decoder will create own copy of data */

        /* Send same audio stream to both the devices. If we bind same ADEC 0 with AO 0 and AO 1, in this case,
         * observes the audio cut issue and audio sync issue between both the devices */
        for (adecChnId = 0; adecChnId < AUDIO_OUT_DEVICE_MAX; adecChnId++)
        {
            s32Ret = RK_MPI_ADEC_SendStream(adecChnId, &stAdecStream, RK_FALSE);
            if (s32Ret != RK_SUCCESS)
            {
                /* Check error status code */
                if (s32Ret != RK_ERR_ADEC_BUF_FULL)
                {
                    /* Error found in send audio stream */
                    break;
                }

                /* If adec buffer full then discard this frame and ignore the error */
                EPRINT(GUI_LIB, "adec buffer full, frame dropped: [dispChnlId=%d], [adecChnId=%d]", dispChnlId, adecChnId);
            }
        }

        /* Free memory block */
        RK_MPI_MB_ReleaseMB(stAdecStream.pMbBlk);

        /* Is error found in send stream? */
        if (s32Ret != RK_SUCCESS)
        {
            /* Unrecoverable error found, restart the audio */
            if (s32Ret != RK_ERR_ADEC_BUF_FULL)
            {
                /* On failure, stop audio out to start it again */
                EPRINT(GUI_LIB, "RK_MPI_ADEC_SendStream failed: [adecChnId=%d], [s32Ret=0x%x]", adecChnId, s32Ret);
                GUI_LIB_MUTEX_LOCK(audioStream.audioOutChnlMutex);
                stopAudioOut();
                audioStream.isAudioOutInit = FALSE;
                GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);
            }
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Swap two channel location on current window.
 * @param   windowId1
 * @param   windowId2
 * @param   dispDevId
 * @return
 */
BOOL SwapWindChannel(UINT8 windowId1, UINT8 windowId2, DISPLAY_DEV_e dispDevId)
{
    RK_S32                  s32Ret = RK_SUCCESS;
    UINT8                   decoderId1 = DECODER_WINDOW_MAX, decoderId2 = DECODER_WINDOW_MAX;
    DEC_DISP_PLAY_ID        dispChnlId;
    DISP_CHNL_WIND_MAP_t    swapDispChnlMap;

    if ((windowId2 >= DECODER_WINDOW_MAX) || (windowId1 >= DECODER_WINDOW_MAX))
    {
        return FAIL;
    }

    /* Lock for display channel and window map update */
    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
    do
    {
        dispChnlId = getDispChnlId(windowId1);
        if (dispChnlId < DEC_DISP_CHN_MAX)
        {
            decoderId1 = getDecoderId(windowId1);
            if (decoderId1 < DECODER_WINDOW_MAX)
            {
                if (decoderId1 != GET_DECODER_ID(dispChnlId))
                {
                    WPRINT(GUI_LIB, "decoder mismatch: [dispChnlId=%d], [decoderId=%d], [windowId1=%d]", dispChnlId, decoderId1, windowId1);
                }

                s32Ret = unbindVdec2Vo(decoderId1, windowId1);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "unbindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId1=%d]", dispChnlId, decoderId1, windowId1);
                    break;
                }
            }
        }

        dispChnlId = getDispChnlId(windowId2);
        if (dispChnlId < DEC_DISP_CHN_MAX)
        {
            decoderId2 = getDecoderId(windowId2);
            if (decoderId2 < DECODER_WINDOW_MAX)
            {
                if (decoderId2 != GET_DECODER_ID(dispChnlId))
                {
                    WPRINT(GUI_LIB, "decoder mismatch: [dispChnlId=%d], [decoderId=%d], [windowId2=%d]", dispChnlId, decoderId2, windowId2);
                }

                s32Ret = unbindVdec2Vo(decoderId2, windowId2);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "unbindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId2=%d]", dispChnlId, decoderId2, windowId2);
                    break;
                }
            }
        }

        /* Swap window in display channel mapping */
        swapDispChnlMap = dispChnlWindMap[windowId1];
        dispChnlWindMap[windowId1] = dispChnlWindMap[windowId2];
        dispChnlWindMap[windowId2] = swapDispChnlMap;

        dispChnlId = getDispChnlId(windowId1);
        if (dispChnlId < DEC_DISP_CHN_MAX)
        {
            if (decoderId2 < DECODER_WINDOW_MAX)
            {
                /* Bind VDEC and VO */
                s32Ret = bindVdec2Vo(decoderId2, windowId1);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "bindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId1=%d]", dispChnlId, decoderId2, windowId1);
                    break;
                }
            }
        }

        dispChnlId = getDispChnlId(windowId2);
        if (dispChnlId < DEC_DISP_CHN_MAX)
        {
            if (decoderId1 < DECODER_WINDOW_MAX)
            {
                /* Bind VDEC and VO */
                s32Ret = bindVdec2Vo(decoderId1, windowId2);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "bindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId2=%d]", dispChnlId, decoderId1, windowId2);
                    break;
                }
            }
        }
    }while(0);

    /* Unlock after display channel and window map update */
    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
    return (s32Ret == RK_SUCCESS) ? SUCCESS : FAIL;
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
BOOL UpdateChannelToWindowMap(DISPLAY_DEV_e dispDevId, UINT8 offset, WINDOW_SHIFT_DIR_e upOrDownDir, UINT8 selectedWindow, WIND_LAYOUT_ID_e layoutIndex)
{
    RK_S32                  s32Ret = RK_SUCCESS;
    UINT8                   windowId, totalWinOnLayout;
    UINT8                   decoderId;
    UINT8                   tmpShift = 0;
    DEC_DISP_PLAY_ID        dispChnlId;
    DISP_CHNL_WIND_MAP_t    newChnMap[DECODER_WINDOW_MAX];

    if ((dispDevId >= MAX_DISPLAY_DEV) || (layoutIndex >= WIND_LAYOUT_MAX))
    {
        EPRINT(GUI_LIB, "invld input params: [dispDevId=%d], [layoutIndex=%d]", dispDevId, layoutIndex);
        return FAIL;
    }

    /* Lock for display channel and window map update */
    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
    for (windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        /* Obtains the information about a bound data source */
        decoderId = getDecoderId(windowId);
        if (decoderId < DECODER_WINDOW_MAX)
        {
            dispChnlId = getDispChnlId(windowId);
            if ((dispChnlId >= DEC_DISP_CHN_MAX) || (decoderId != GET_DECODER_ID(dispChnlId)))
            {
                WPRINT(GUI_LIB, "decoder mismatch: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
            }

            s32Ret = unbindVdec2Vo(decoderId, windowId);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "unbindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
            }
        }
    }

    /* Generate Mosaic layout for display device */
    s32Ret = createVoChnlLayout(layoutIndex);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "createVoChnlLayout failed: [layoutIndex=%d]", layoutIndex);
    }

    if (upOrDownDir == CHANNEL_UP_SHIFT)
    {
        for (windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
        {
            if ((windowId + offset) < DECODER_WINDOW_MAX)
            {
                newChnMap[windowId] = dispChnlWindMap[windowId + offset];
            }
            else
            {
                newChnMap[windowId] = dispChnlWindMap[tmpShift++];
            }
        }
    }
    else
    {
        tmpShift = (DECODER_WINDOW_MAX - offset);
        for (windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
        {
            if (windowId < offset)
            {
                newChnMap[windowId] = dispChnlWindMap[tmpShift++];
            }
            else
            {
                newChnMap[windowId] = dispChnlWindMap[windowId - offset];
            }
        }
    }

    totalWinOnLayout =  maxWindPerLayout[layoutIndex];
    for(windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        dispChnlWindMap[windowId] = newChnMap[windowId];
        if (windowId < totalWinOnLayout)
        {
            dispChnlId = getDispChnlId(windowId);
            if (dispChnlId < DEC_DISP_CHN_MAX)
            {
                /* Get decoder from display channel */
                decoderId = GET_DECODER_ID(dispChnlId);

                /* Bind VDEC and VO */
                s32Ret = bindVdec2Vo(decoderId, windowId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "bindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
                }
            }
        }
    }

    /* Unlock after display channel and window map update */
    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
    displayInfo.windowLayout = layoutIndex;
    DPRINT(GUI_LIB, "change in layout: [windowId=%d], [windLayoutId=%d]", selectedWindow, layoutIndex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function changes the layout mode for given display channel and display channel
 *          which are fit into this window. Other channel are not display.
 * @param   dispDevId
 * @param   windLayoutId
 * @param   selectedWindow
 * @return
 */
BOOL ChangeWindowLayout(DISPLAY_DEV_e dispDevId, WIND_LAYOUT_ID_e windLayoutId, UINT8 selectedWindow)
{
    RK_S32                  s32Ret = RK_SUCCESS;
    INT8                    currWindow, windowId, startWindow;
    UINT8                   totalWinOnLayout;
    UINT8                   decoderId;
    UINT8                   pageNo;
    DEC_DISP_PLAY_ID        dispChnlId;
    DISP_CHNL_WIND_MAP_t    newChnMap[DECODER_WINDOW_MAX];

    if ((dispDevId >= MAX_DISPLAY_DEV) || (windLayoutId >= WIND_LAYOUT_MAX) || (selectedWindow >= DECODER_WINDOW_MAX))
    {
        EPRINT(GUI_LIB, "invld input params: [dispDevId=%d], [windLayoutId=%d], [windowId=%d]", dispDevId, windLayoutId, selectedWindow);
        return FAIL;
    }

    displayInfo.windowLayout = windLayoutId;
    pageNo = selectedWindow/maxWindPerLayout[windLayoutId];
    totalWinOnLayout = maxWindPerLayout[windLayoutId];
    startWindow = (pageNo * totalWinOnLayout);
    DPRINT(GUI_LIB, "change layout: [windowId=%d], [windLayoutId=%d], [pageNo=%d], [totalWinOnLayout=%d]", selectedWindow, windLayoutId, pageNo, totalWinOnLayout);

    /* Lock for display channel and window map update */
    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
    for(windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        currWindow = (windowId + startWindow);
        if(currWindow < DECODER_WINDOW_MAX)
        {
            /* Display this window if applicable as per new layout */
            newChnMap[windowId] = dispChnlWindMap[currWindow];
        }
        else
        {
            /* Ignore this window for new layout */
            newChnMap[windowId].status = FREE;
        }

        /* Obtains the information about a bound data source */
        decoderId = getDecoderId(windowId);
        if (decoderId < DECODER_WINDOW_MAX)
        {
            dispChnlId = getDispChnlId(windowId);
            if ((dispChnlId >= DEC_DISP_CHN_MAX) || (decoderId != GET_DECODER_ID(dispChnlId)))
            {
                WPRINT(GUI_LIB, "decoder mismatch: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
            }

            s32Ret = unbindVdec2Vo(decoderId, windowId);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "unbindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
            }
        }
    }

    /* Generate Mosaic layout for display device */
    s32Ret = createVoChnlLayout(windLayoutId);
    if (s32Ret != RK_SUCCESS)
    {
        /* Unlock after display channel and window map update fail */
        GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
        EPRINT(GUI_LIB, "createVoChnlLayout failed: [windowId=%d], [windLayoutId=%d]", selectedWindow, windLayoutId);
        return FAIL;
    }

    for(windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        /* It is temporary updation of binding, hence no need to update in our database */
        if ((windowId < totalWinOnLayout) && (newChnMap[windowId].status == BUSY))
        {
            /* Get display channel to derive decoder */
            dispChnlId = newChnMap[windowId].dispChnlId;

            /* Get decoder from display channel */
            decoderId = GET_DECODER_ID(dispChnlId);

            /* Bind VDEC and VO */
            s32Ret = bindVdec2Vo(decoderId, windowId);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "bindVdec2Vo failed: [dispChnlId=%d], [decoderId=%d], [windowId=%d]", dispChnlId, decoderId, windowId);
            }
        }
    }

    /* Unlock after display channel and window map update */
    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   resetDecoderDispList
 * @param   dispChnlId
 */
static void resetDecoderDispList(DEC_DISP_PLAY_ID dispChnlId)
{
    decDispList[dispChnlId].vidCodec = VIDEO_CODEC_NONE;
    decDispList[dispChnlId].frameHeight = 0;
    decDispList[dispChnlId].frameWidth = 0;
    decDispList[dispChnlId].frameRate = 0;
    decDispList[dispChnlId].iFrameRecv = FALSE;
    decDispList[dispChnlId].noOfReferanceFrame = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   removes audio into running streaming.
 * @param   dispChnlId
 * @return
 */
BOOL ExcludeAudio(UINT8 dispChnlId)
{
    /* Validate input params */
    if ((dispChnlId >= DEC_DISP_CHN_MAX) && (dispChnlId != AUDIO_OUT_DEC_CHNL_ID))
    {
        EPRINT(GUI_LIB, "invld input params: [dispChnlId=%d]", dispChnlId);
        return FAIL;
    }

    /* Stop audio if matched with current channel */
    GUI_LIB_MUTEX_LOCK(audioStream.audioOutChnlMutex);
    if (audioStream.audioOutChnl == dispChnlId)
    {
        if (TRUE == audioStream.isAudioOutInit)
        {
            /* Stop previous audio out */
            stopAudioOut();
            audioStream.isAudioOutInit = FALSE;
        }
        audioStream.audioOutChnl = INVALID_DEC_DISP_PLAY_ID;
    }
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);
    DPRINT(GUI_LIB, "stop audio out request received: [dispChnlId=%d]", dispChnlId);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   adds audio into running streaming.
 * @param   dispChnlId
 * @return
 */
BOOL IncludeAudio(UINT8 dispChnlId)
{
    /* Validate input params */
    if ((dispChnlId >= DEC_DISP_CHN_MAX) && (dispChnlId != AUDIO_OUT_DEC_CHNL_ID))
    {
        EPRINT(GUI_LIB, "invld input params: [dispChnlId=%d]", dispChnlId);
        return FAIL;
    }

    /* Stop previous audio before starting new one, if older one is already on */
    GUI_LIB_MUTEX_LOCK(audioStream.audioOutChnlMutex);
    if ((audioStream.audioOutChnl != INVALID_DEC_DISP_PLAY_ID) && (TRUE == audioStream.isAudioOutInit))
    {
        /* Stop previous audio out */
        stopAudioOut();
        audioStream.isAudioOutInit = FALSE;
        WPRINT(GUI_LIB, "audio out was already running: [dispChnlId=%d]", audioStream.audioOutChnl);
    }

    /* Audio related process will be started on first frame */
    audioStream.audioOutChnl = dispChnlId;
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);
    DPRINT(GUI_LIB, "start audio out request received: [dispChnlId=%d]", dispChnlId);
    return SUCCESS;
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
    RK_S32 	  		s32Ret;
    AUDIO_DEVICE_e 	aoDevId;
    AUDIO_FADE_S    audioFade = {0};

    /* Store current mute state */
    audioStream.audioOutState = muteState;

    /* Display channel should be audio channel */
    if ((audioStream.audioOutChnl == INVALID_DEC_DISP_PLAY_ID) || (FALSE == audioStream.isAudioOutInit))
    {
        return SUCCESS;
    }

    for (aoDevId = 0; aoDevId < AUDIO_OUT_DEVICE_MAX; aoDevId++)
    {
        s32Ret = RK_MPI_AO_SetMute(aoDevId, (muteState == AUDIO_OFF) ? RK_TRUE : RK_FALSE, &audioFade);
        if (s32Ret != RK_SUCCESS)
        {
            API_FAIL_DEBUG("RK_MPI_AO_SetMute", s32Ret);
            retVal = FAIL;
        }
    }

    DPRINT(GUI_LIB, "audio out state changed: [state=%s]", (muteState == AUDIO_OFF) ? "mute" : "unmute");
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
    RK_S32          s32Ret;
    AUDIO_DEVICE_e 	aoDevId;

    if (volumeLevel > VOLUME_LEVEL_MAX)
    {
        EPRINT(GUI_LIB, "invld volume level, range 0 to 100: [level=%d]", volumeLevel);
        return SUCCESS;
    }

    audioStream.audioOutVolume = volumeLevel;
    if ((audioStream.audioOutChnl == INVALID_DEC_DISP_PLAY_ID) || (FALSE == audioStream.isAudioOutInit))
    {
        return SUCCESS;
    }

    for (aoDevId = 0; aoDevId < AUDIO_OUT_DEVICE_MAX; aoDevId++)
    {
        s32Ret = RK_MPI_AO_SetVolume(aoDevId, volumeLevel);
        if (s32Ret != RK_SUCCESS)
        {
            API_FAIL_DEBUG("RK_MPI_AO_SetVolume", s32Ret);
            retVal = FAIL;
        }
    }

    DPRINT(GUI_LIB, "audio out volume changed: [level=%d]", volumeLevel);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get audio out channel
 * @return  return audio channel Device to ON/OFF.
 */
UINT8 GetCurrentAudioChannel(void)
{
    GUI_LIB_MUTEX_LOCK(audioStream.audioOutChnlMutex);
    UINT8 audioChannel = audioStream.audioOutChnl;
    GUI_LIB_MUTEX_UNLOCK(audioStream.audioOutChnlMutex);
    return audioChannel;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to zoom in and also zoom out to particular channel in window layout.
 *          For zoom out user need to set all parameter of cropParam
 * @param   windowId
 * @param   pCropParam
 * @param   zoomType
 * @return
 */
BOOL SetCropParam(UINT8 windowId, CROP_PARAM_t *pCropParam, ZOOM_TYPE_e zoomType)
{
    RK_S32              s32Ret;
    UINT8               decoderId;
    VPSS_CROP_INFO_S    stCropInfo = {0};
    VALID_SCREEN_INFO_t screenInfo;
    MPP_CHN_S           stSrcChn;
    MPP_CHN_S           stDestChn;

    /* Validate input parameters */
    if (windowId >= DECODER_WINDOW_MAX)
    {
        return FAIL;
    }

    /* Lock for display channel and window map update */
    GUI_LIB_MUTEX_LOCK(dispChnlWindMapMutex);
    do
    {
        /* We do crop in 1x1 layout only. So that, Here window should be 0 but application provides selected
         * window in sync playback and selected window may not displaying currently on VO. Due to that,
         * sometimes not able to derive decoder and it throws error.
         * example: In 2x2 sync platback, we need crop on 3rd window then application provides window = 3 in
         * 1x1 layout during crop but in 1x1 layout, window must be 0 */
        windowId = 0;

        /* Get decoder from window */
        decoderId = getDecoderId(windowId);
        if (decoderId >= DECODER_WINDOW_MAX)
        {
            EPRINT(GUI_LIB, "invld decoder: [decoderId=%d], [windowId=%d]", decoderId, windowId);
            break;
        }

        stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
        if (zoomType == ZOOM_IN)
        {
            VDEC_CHN_ATTR_S stVdecChnAttr;

            /* Get decoder channel attributes */
            s32Ret = RK_MPI_VDEC_GetChnAttr(decoderId, &stVdecChnAttr);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "RK_MPI_VDEC_GetChnAttr failed: [decoderId=%d], [windowId=%d], [s32Ret=0x%x]", decoderId, windowId, s32Ret);
                break;
            }

            /* Fill Zoom Attributes */
            stCropInfo.bEnable = RK_TRUE;
            getLiveViewScreenInfo(TRUE, &screenInfo, WIND_LAYOUT_1X1_1CH);
            stCropInfo.stCropRect.s32X = ALIGN_BACK(((pCropParam->startXPos * stVdecChnAttr.u32PicWidth) / screenInfo.actualWidth), 4);
            stCropInfo.stCropRect.s32Y = ALIGN_BACK(((pCropParam->startYPos * stVdecChnAttr.u32PicHeight) / screenInfo.actualHeight), 4);
            stCropInfo.stCropRect.u32Width = ALIGN_BACK(((pCropParam->width * stVdecChnAttr.u32PicWidth) / screenInfo.actualWidth), 8);
            stCropInfo.stCropRect.u32Height = ALIGN_BACK(((pCropParam->height * stVdecChnAttr.u32PicHeight) / screenInfo.actualHeight), 8);

            /* Get best suitable VPSS resolution height and width */
            GET_VPSS_VALUE(stCropInfo.stCropRect.u32Width, (2 * VPSS_MIN_IMAGE_WIDTH), VPSS_MAX_IMAGE_WIDTH);
            GET_VPSS_VALUE(stCropInfo.stCropRect.u32Height, (2 * VPSS_MIN_IMAGE_HEIGHT), VPSS_MAX_IMAGE_HEIGHT);

            /* Obtains the information about a bound data source */
            stDestChn.enModId = RK_ID_VO;
            stDestChn.s32DevId = VO_LAYER_ID;
            stDestChn.s32ChnId = windowId;
            s32Ret = RK_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "RK_MPI_SYS_GetBindbyDest failed: [decoderId=%d], [windowId=%d], [s32Ret=%x]", decoderId, windowId, s32Ret);
                break;
            }

            /* If VO chnl is currently bind to VDEC, then create VPSS Group, VPSS Chnl and bind VPSS to VO */
            if(stSrcChn.enModId == RK_ID_VDEC)
            {
                /* Unbind VDEC and VO */
                s32Ret = unbindVdec2Vo(decoderId, windowId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "unbindVdec2Vo failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }

                /* Create VPSS group for video processing */
                s32Ret = createVpssGrp(decoderId, &stVdecChnAttr);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "createVpssGrp failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }

                /* Bind VDEC and VPSS */
                s32Ret = bindVdec2Vpss(decoderId, decoderId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "bindVdec2Vpss failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }

                /* Bind VPSS and VO */
                s32Ret = bindVpss2Vo(decoderId, windowId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "bindVpss2Vo failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }
            }

            /* Sets the crop attributes of the VPSS */
            s32Ret = RK_MPI_VPSS_SetChnCrop(decoderId, VPSS_PER_GRP_CHNL_NUM, &stCropInfo);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "RK_MPI_VPSS_SetGrpCrop failed: [decoderId=%d], [windowId=%d], [s32Ret=%x]", decoderId, windowId, s32Ret);
                break;
            }
        }
        else
        {
             /* Disable crop */
            stCropInfo.bEnable = RK_FALSE;
            stCropInfo.stCropRect.s32X = 0;
            stCropInfo.stCropRect.s32Y = 0;
            stCropInfo.stCropRect.u32Width = 0;
            stCropInfo.stCropRect.u32Height = 0;

            /* Sets the crop attributes of the VPSS */
            s32Ret = RK_MPI_VPSS_SetGrpCrop(decoderId, &stCropInfo);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "RK_MPI_VPSS_SetGrpCrop failed: [decoderId=%d], [windowId=%d], [s32Ret=%x]", decoderId, windowId, s32Ret);
                break;
            }

            /* Obtains the information about a bound data source */
            stDestChn.enModId = RK_ID_VO;
            stDestChn.s32DevId = VO_LAYER_ID;
            stDestChn.s32ChnId = windowId;
            s32Ret = RK_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "RK_MPI_SYS_GetBindbyDest failed: [decoderId=%d], [windowId=%d], [s32Ret=%x]", decoderId, windowId, s32Ret);
                break;
            }

            /* If VO chnl is currently bind to VPSS, then delete VPSS Group, VPSS Chnl and bind VDEC to VO */
            if(stSrcChn.enModId == RK_ID_VPSS)
            {
                /* Remove VPSS to VO link */
                s32Ret = unbindVpss2Vo(decoderId, windowId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "unbindVpss2Vo failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }

                /* Unbind VDEC and VPSS */
                s32Ret = unbindVdec2Vpss(decoderId, decoderId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "unbindVdec2Vpss failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }

                /* Delete VPSS group for video processing */
                s32Ret = deleteVpssGrp(decoderId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "createVpssGrp failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }

                /* Bind VDEC and VO */
                s32Ret = bindVdec2Vo(decoderId, windowId);
                if (s32Ret != RK_SUCCESS)
                {
                    EPRINT(GUI_LIB, "bindVdec2Vo failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
                    break;
                }
            }

            /* Unlock after display channel and window map update */
            GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
            return SUCCESS;
        }
    }while(0);

    /* Unlock after display channel and window map update */
    GUI_LIB_MUTEX_UNLOCK(dispChnlWindMapMutex);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sets the display picture parameters
 * @param   devId
 * @param   displayParam
 * @param   value
 * @return
 * @note    Range 0 ~ 100 and default value is 50
 */
BOOL SetDisplayParameter(PHYSICAL_DISPLAY_TYPE_e devId, DISPLAY_SCREEN_PARAM_TYPE_e displayParam, UINT32 value)
{
    RK_S32      s32Ret;
    VO_CSC_S    pstParam = {0};

    if ((devId >= PHYSICAL_DISPLAY_TYPE_MAX) || (displayParam >= DISPLAY_SCREEN_PARAM_MAX) || (value > 100))
    {
        EPRINT(GUI_LIB, "invld input param: [devId=%d], [displayParam=%d], [value=%d]", devId, displayParam, value);
        return FAIL;
    }

    /* Obtains the HDMI output picture effect.*/
    s32Ret = RK_MPI_VO_GetPostProcessParam(VO_DEVICE_ID, &pstParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_GetPostProcessParam", s32Ret);
        return FAIL;
    }

    switch(displayParam)
    {
        case DISPLAY_BRIGHTNESS:
        {
            /* Nothing to do if no change in brightness */
            if (pstParam.u32Luma == value)
            {
                DPRINT(GUI_LIB, "no change in brightness: [value=%d]", value);
                return SUCCESS;
            }
            pstParam.u32Luma = value;
        }
        break;

        case DISPLAY_CONTRAST:
        {
            /* Nothing to do if no change in contrast */
            /* Allow actual contrast between 20 ~ 100 to avoid complete blackout on lower contrast */
            value = 20 + (value * ((float)(100 - 20)/(float)100));
            if (pstParam.u32Contrast == value)
            {
                DPRINT(GUI_LIB, "no change in contrast: [value=%d]", value);
                return SUCCESS;
            }
            pstParam.u32Contrast = value;
        }
        break;

        case DISPLAY_SATURATION:
        {
            /* Nothing to do if no change in saturation */
            if (pstParam.u32Satuature == value)
            {
                DPRINT(GUI_LIB, "no change in saturation: [value=%d]", value);
                return SUCCESS;
            }
            pstParam.u32Satuature = value;
        }
        break;

        case DISPLAY_HUE:
        {
            /* Nothing to do if no change in hue */
            if (pstParam.u32Hue == value)
            {
                DPRINT(GUI_LIB, "no change in hue: [value=%d]", value);
                return SUCCESS;
            }
            pstParam.u32Hue = value;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        return FAIL;
    }

    /* It is added due to false hdmi color format getting set on platform if display is disconnected.
     * If display is not connected then reinit hdmi after it gets connected again. */
    if (FALSE == isVoSinkConnected())
    {
        reinitHdmiParamF = TRUE;
    }

    /* Sets the HDMI output picture effect */
    s32Ret = RK_MPI_VO_SetPostProcessParam(VO_DEVICE_ID, &pstParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetPostProcessParam", s32Ret);
        return FAIL;
    }

    /* If display is not connected then reinit hdmi after it gets connected again */
    if (FALSE == isVoSinkConnected())
    {
        reinitHdmiParamF = TRUE;
    }

    DPRINT(GUI_LIB, "new hdmi param: [luma=%d], [contrast=%d], [saturation=%d], [hue=%d]",
           pstParam.u32Luma, pstParam.u32Contrast, pstParam.u32Satuature, pstParam.u32Hue);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize audio configuration for Test Application
 * @param   audDev
 * @return
 */
BOOL StartAudioOutTest(AUDIO_TEST_DEVICE_e audDev)
{
    RK_S32          s32Ret;
    AIO_ATTR_S      stAioAttr;
    AUDIO_DEVICE_e  aoDevId, startAoDevId, endAoDevId;

    switch(audDev)
    {
        case AUDIO_TEST_AO_DEV:
        {
            DPRINT(GUI_LIB, "audio out start on onboard jack");
            startAoDevId = AUDIO_OUT_DEVICE_NVR;
            endAoDevId = AUDIO_OUT_DEVICE_MAX;
        }
        break;

        case AUDIO_TEST_HDMI_AO_DEV:
        {
            DPRINT(GUI_LIB, "audio out start on hdmi display");
            startAoDevId = AUDIO_OUT_DEVICE_HDMI;
            endAoDevId = AUDIO_OUT_DEVICE_NVR;
        }
        break;

        case AUDIO_TEST_BOTH_AO_DEV:
        {
            DPRINT(GUI_LIB, "audio out start on onboard jack and hdmi display");
            startAoDevId = AUDIO_OUT_DEVICE_HDMI;
            endAoDevId = AUDIO_OUT_DEVICE_MAX;
        }
        break;

        default:
        {
            /* Invalid output device */
        }
        return FAIL;
    }

    memset(&stAioAttr, 0, sizeof(stAioAttr));
    stAioAttr.soundCard.sampleRate = AUDIO_SAMPLE_RATE_48000;   /* output sample rate */
    stAioAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;          /* output bitwidth */
    stAioAttr.soundCard.channels = 2;                           /* output device channels */
    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;            /* input data sample rate */
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;                  /* bitwidth */
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;              /* mono or stereo */
    stAioAttr.u32EXFlag = 1;                                    /* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit) */
    stAioAttr.u32FrmNum = 1;                                    /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    stAioAttr.u32PtNumPerFrm = AUDIO_OUT_FRAME_SIZE_MAX;        /* point num per frame (80/160/240/320/480/1024/2048) */
    stAioAttr.u32ChnCnt = 1;                                    /* channel number, valid value:1/2/4/8 */

    /* Enable input audio channel */
    s32Ret = enableAiChnl(AUDIO_AI_DEV_ID, &stAioAttr);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to enable ai channel");
        return FAIL;
    }

    memset(&stAioAttr, 0, sizeof(stAioAttr));
    stAioAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;          /* output bitwidth */
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;                  /* bitwidth */
    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;            /* input data sample rate */
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;              /* mono or stereo */
    stAioAttr.u32EXFlag = 1;                                    /* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit) */
    stAioAttr.u32FrmNum = 1;                                    /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    stAioAttr.u32PtNumPerFrm = AUDIO_OUT_FRAME_SIZE_MAX;        /* point num per frame (80/160/240/320/480/1024/2048) */
    stAioAttr.u32ChnCnt = 1;                                    /* channel number, valid value:1/2/4/8 */

    for (aoDevId = startAoDevId; aoDevId < endAoDevId; aoDevId++)
    {
        /* Set AO parameters */
        stAioAttr.soundCard.channels = (aoDevId == AUDIO_OUT_DEVICE_NVR) ? 1 : 2;   /* output device channels */

        /* Enable audio channel */
        s32Ret = enableAoChnl(aoDevId, &stAioAttr);
        if (s32Ret != RK_SUCCESS)
        {
            API_FAIL_DEBUG("enableAoChnl failed", s32Ret);
            break;
        }

        /* Bind ai to ao */
        s32Ret = bindAi2Ao(AUDIO_AI_DEV_ID, aoDevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "bindAi2Ao failed: [aiDevId=%d], [aoDevId=%d]", AUDIO_AI_DEV_ID, aoDevId);
            break;
        }

        s32Ret = RK_MPI_AO_SetVolume(aoDevId, audioStream.audioOutVolume);
        if (s32Ret != RK_SUCCESS)
        {
            API_FAIL_DEBUG("RK_MPI_AO_SetVolume", s32Ret);
        }
    }

    /* Check error occurred?? */
    if (s32Ret != RK_SUCCESS)
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
    RK_S32          s32Ret;
    AUDIO_DEVICE_e  aoDevId, startAoDevId, endAoDevId;

    switch(audDev)
    {
        case AUDIO_TEST_AO_DEV:
        {
            DPRINT(GUI_LIB, "audio out stop on onboard jack");
            startAoDevId = AUDIO_OUT_DEVICE_NVR;
            endAoDevId = AUDIO_OUT_DEVICE_MAX;
        }
        break;

        case AUDIO_TEST_HDMI_AO_DEV:
        {
            DPRINT(GUI_LIB, "audio out stop on hdmi display");
            startAoDevId = AUDIO_OUT_DEVICE_HDMI;
            endAoDevId = AUDIO_OUT_DEVICE_NVR;
        }
        break;

        case AUDIO_TEST_BOTH_AO_DEV:
        {
            DPRINT(GUI_LIB, "audio out stop on onboard jack & hdmi display");
            startAoDevId = AUDIO_OUT_DEVICE_HDMI;
            endAoDevId = AUDIO_OUT_DEVICE_MAX;
        }
        break;

        default:
        {
            /* Invalid output device */
        }
        return FAIL;
    }

    for (aoDevId = startAoDevId; aoDevId < endAoDevId; aoDevId++)
    {
        /* Unbind ai to ao */
        s32Ret = unbindAi2Ao(AUDIO_AI_DEV_ID, aoDevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "unbindAi2Ao failed: [aiDevId=%d], [aoDevId=%d]", AUDIO_AI_DEV_ID, aoDevId);
        }
    }

    s32Ret = disableAiChnl(AUDIO_AI_DEV_ID);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "disableAiChnl failed: [aiDevId=%d]", AUDIO_AI_DEV_ID);
    }

    for (aoDevId = startAoDevId; aoDevId < endAoDevId; aoDevId++)
    {
        /* Disable ao channel */
        s32Ret = disableAoChnl(aoDevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "disableAoChnl failed: [aoDevId=%d]", aoDevId);
        }
    }

    /* Check error occurred?? */
    if (s32Ret != RK_SUCCESS)
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
    DPRINT(GUI_LIB, "setting tv adjust offset: [old=%d], [new=%d]", tvAdjustOffset, offset);
    tvAdjustOffset = offset;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetMaxDecodingCapacity
 * @return
 */
UINT32 GetMaxDecodingCapacity(void)
{
    return DECODER_CAPABILITY_MAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Configure audio codec, start Ai and Aenc.
 * @return
 */
BOOL StartAudioIn(void)
{
    RK_S32      s32Ret;
    AIO_ATTR_S  stAiAttr = {0};

    if (TRUE == audioStream.isAudioInInit)
    {
        /* Stop audio-in first */
        EPRINT(GUI_LIB, "audio-in is already on");
        StopAudioIn();
    }

    DPRINT(GUI_LIB, "start audio-in request received");
    stAiAttr.soundCard.sampleRate = AUDIO_SAMPLE_RATE_48000;    /* output sample rate */
    stAiAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;           /* output bitwidth */
    stAiAttr.soundCard.channels = 2;                            /* output device channels */
    stAiAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;             /* input data sample rate */
    stAiAttr.enBitwidth = AUDIO_BIT_WIDTH_16;                   /* bitwidth */
    stAiAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;               /* mono or stereo */
    stAiAttr.u32FrmNum = AUDIO_FRAME_BUFF_CNT_MAX;              /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    stAiAttr.u32PtNumPerFrm = AUDIO_IN_FRAME_SIZE_MAX;          /* point num per frame (80/160/240/320/480/1024/2048) */
    stAiAttr.u32ChnCnt = 1;                                     /* channel number, valid value:1/2/4/8 */

    /* Enable input audio channel */
    s32Ret = enableAiChnl(AUDIO_AI_DEV_ID, &stAiAttr);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to enable ai channel");
        return FAIL;
    }

    /* Create AENC channel */
    s32Ret = createAencChnl();
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to create aenc channel");
        disableAiChnl(AUDIO_AI_DEV_ID);
        return FAIL;
    }

    /* Bind ai to aenc */
    s32Ret = bindAi2Aenc(AUDIO_AI_DEV_ID);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to bind ai2aenc channel");
        disableAiChnl(AUDIO_AI_DEV_ID);
        deleteAencChnl();
        return FAIL;
    }

    /* Audio in graph created */
    DPRINT(GUI_LIB, "audio-in started successfully!");
    audioStream.isAudioInInit = TRUE;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop Ai and Aenc channels, free resources. Set run status to FALSE
 */
void StopAudioIn(void)
{
    RK_S32 s32Ret = unbindAi2Aenc(AUDIO_AI_DEV_ID);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to unbind ai2aenc channel");
    }

    s32Ret = deleteAencChnl();
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to delete aenc channel");
    }

    s32Ret = disableAiChnl(AUDIO_AI_DEV_ID);
    if (s32Ret != RK_SUCCESS)
    {
        EPRINT(GUI_LIB, "failed to disable ai channel");
    }

    /* Audio-in disabled */
    audioStream.isAudioInInit = FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get next audio frame from Ai-> send it to Aenc-> Get stream from Aenc-> Copy in output buffer
 * @param   pFrameBuf - pointer to output audio stream buffer
 * @param   pFrameLen - frame length
 * @return
 */
BOOL GetNextAudioInFrame(CHARPTR pFrameBuf, INT32PTR pFrameLen)
{
    RK_S32          s32Ret;
    RK_VOID         *pFrame;
    AUDIO_STREAM_S  streamInfo;

    /* Get encoded audio stream from encoder */
    s32Ret = RK_MPI_AENC_GetStream(AUDIO_AENC_CHNL_ID, &streamInfo, AUDIO_FRAME_RECV_TIMEOUT_MS);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AENC_GetStream", s32Ret);
        return FAIL;
    }

    /* Get audio stream address */
    pFrame = RK_MPI_MB_Handle2VirAddr(streamInfo.pMbBlk);
    if (pFrame == NULL)
    {
        EPRINT(GUI_LIB, "failed to get audio-in frame");
        return FAIL;
    }

    /* Validate audio frame length */
    if ((streamInfo.u32Len == 0) || (streamInfo.u32Len > *pFrameLen))
    {
        EPRINT(GUI_LIB, "no data or more data available in audio-in frame: [frameLen=%d]", streamInfo.u32Len);
        RK_MPI_AENC_ReleaseStream(AUDIO_AENC_CHNL_ID, &streamInfo);
        return FAIL;
    }

    /* Copy audio frame data and set frame length */
    memcpy(pFrameBuf, pFrame, streamInfo.u32Len);
    *pFrameLen = streamInfo.u32Len;

    /* Release audio stream */
    s32Ret = RK_MPI_AENC_ReleaseStream(AUDIO_AENC_CHNL_ID, &streamInfo);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AENC_ReleaseStream", s32Ret);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get connection status of VO sink
 * @return  TRUE if display connected else FALSE
 */
static BOOL isVoSinkConnected(void)
{
    RK_S32                  s32Ret;
    VO_SINK_CAPABILITY_S    stVoSinkCap;

    /* Get VO sink capability */
    s32Ret = RK_MPI_VO_GetSinkCapability(VO_INTF_HDMI, HDMI1_DEVICE_ID, &stVoSinkCap);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_GetSinkCapability", s32Ret);
        return FALSE;
    }

    /* Return display connection status */
    return stVoSinkCap.bConnected;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Renegotiate HDMI parameters
 */
static void negotiateHdmiParam(void)
{
#if defined(RK3588_NVRH) /* Set display color space at bootup and on hdmi connect (As per rockchip's recommendation) */
    RK_S32          s32Ret;
    VO_HDMI_PARAM_S stHdmiParam;

    /* Get HDMI display color format parameters */
    s32Ret = RK_MPI_VO_GetHdmiParam(VO_INTF_HDMI, HDMI1_DEVICE_ID, &stHdmiParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_GetHdmiParam", s32Ret);
        return;
    }

    /* Set HDMI display color format parameters as RGB */
    stHdmiParam.enHdmiMode = VO_HDMI_MODE_HDMI;
    stHdmiParam.enColorFmt = VO_HDMI_COLOR_FORMAT_RGB;
    stHdmiParam.enQuantRange = VO_HDMI_QUANT_RANGE_FULL;
    s32Ret = RK_MPI_VO_SetHdmiParam(VO_INTF_HDMI, HDMI1_DEVICE_ID, &stHdmiParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetHdmiParam", s32Ret);
        return;
    }

    /* Provide some time for negotiation */
    sleep(1);

    /* Get HDMI display color format parameters */
    s32Ret = RK_MPI_VO_GetHdmiParam(VO_INTF_HDMI, HDMI1_DEVICE_ID, &stHdmiParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_GetHdmiParam", s32Ret);
        return;
    }

    /* Set HDMI display color format parameters as AUTO */
    stHdmiParam.enColorFmt = VO_HDMI_COLOR_FORMT_AUTO;
    s32Ret = RK_MPI_VO_SetHdmiParam(VO_INTF_HDMI, HDMI1_DEVICE_ID, &stHdmiParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetHdmiParam", s32Ret);
        return;
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Detection Of HDMI
 * @param   data
 */
static void *hdmiDetectionHandler(void *data)
{
    VO_INTF_SYNC_E          voResolution = VO_OUTPUT_BUTT;
    DISPLAY_RESOLUTION_e    displayResolution = DISPLAY_RESOLUTION_MAX;

    /* Set HDMI thread name */
    prctl(PR_SET_NAME, "HDMI_DETECT", 0, 0, 0);
    DPRINT(GUI_LIB, "hdmi detection handler thread started");

    while(TRUE)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        /* Wait for signal to wakeup */
        GUI_LIB_MUTEX_LOCK(hdmiHandlerLock);
        pthread_cond_wait(&hdmiHandlerSignal, &hdmiHandlerLock);
        GUI_LIB_MUTEX_UNLOCK(hdmiHandlerLock);

        /* Do we have to exit from thread? */
        if (FALSE == hdmiThreadRunStatusF)
        {
            /* Exit from the thread */
            break;
        }

        /* Is Display connected? */
        if (FALSE == isVoSinkConnected())
        {
            DPRINT(GUI_LIB, "hdmi display is not connected");
            continue;
        }

        /* Is reinit needed? */
        if (reinitHdmiParamF)
        {
            /* Re-negotiate HDMI parameters with display */
            negotiateHdmiParam();

            /* We have reinited display params */
            reinitHdmiParamF = FALSE;
        }

        /* Get negotiated VO channel resolution based on configured resolution */
        displayResolution = displayInfo.configResolution;
        voResolution = getNegotiatedVoRkRes(&displayResolution);

        /* If output resolution is changed then relaunch the application */
        if (voResolution != currVoResolution)
        {
            EPRINT(GUI_LIB, "change in display resolution: [voRes=%d --> %d]", currVoResolution, voResolution);
            kill(getpid(), SIGTERM);
        }
    }

    /* Exit from the thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   hdmiEventHandler
 * @param   pPrivateData
 */
static void hdmiEventHandler(RK_VOID *pPrivateData)
{
    BOOL hdmiStatus = isVoSinkConnected();

    DPRINT(GUI_LIB, "hdmi status changed: %s", hdmiStatus ? "connected" : "disconnected");

    /* Send signal to hdmi handler thread if display connected and reinit is required */
    if (hdmiStatus)
    {
        /* Send signal to hdmi handler thread to renegotiate params */
        GUI_LIB_MUTEX_LOCK(hdmiHandlerLock);
        pthread_cond_signal(&hdmiHandlerSignal);
        GUI_LIB_MUTEX_UNLOCK(hdmiHandlerLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get HDMI display capabilities.
 * @param   needOnlyConnSts
 * @param   pIsHdmiConnected
 * @param   pVoDispRes
 * @return
 */
static BOOL getHdmiCapability(BOOL needOnlyConnSts, BOOL *pIsHdmiConnected, BOOL *pVoDispRes)
{
    BOOL                retVal = FAIL;
    INT32               drmFd;
    INT32               resCnt;
    INT32               mode;
    drmModeRes          *resources = NULL;
    drmModeConnector    *connector = NULL;
    drmModeModeInfo     *modeInfo;

    /* Open DRM device */
    drmFd = open("/dev/dri/card0", O_RDONLY | O_CLOEXEC);
    if (drmFd < 0)
    {
        EPRINT(GUI_LIB, "failed to open drm");
        return FAIL;
    }

    /* Get DRM resources */
    resources = drmModeGetResources(drmFd);
    if (NULL == resources)
    {
        EPRINT(GUI_LIB, "failed to get drm resource");
        close(drmFd);
        return FAIL;
    }

    /* Check for required display connector (e.g. HDMIA) */
    for (resCnt = 0; resCnt < resources->count_connectors; resCnt++)
    {
        /* Get DRM connector */
        connector = drmModeGetConnector(drmFd, resources->connectors[resCnt]);
        if (NULL == connector)
        {
            EPRINT(GUI_LIB, "failed to get drm connector: [resCnt=%d]", resCnt);
            continue;
        }

        /* Check connector type. We need HDMIA */
        if (connector->connector_type != DRM_MODE_CONNECTOR_HDMIA)
        {
            /* Not required connector */
            drmModeFreeConnector(connector);
            connector = NULL;
            continue;
        }

        /* Check connection state. It should not be unknown */
        if (connector->connection == DRM_MODE_UNKNOWNCONNECTION)
        {
            EPRINT(GUI_LIB, "unknown hdmi state found: [connector_id=%d], [resCnt=%d]", connector->connector_id, resCnt);
            break;
        }

        /* Set current HDMI connection status */
        *pIsHdmiConnected = (connector->connection == DRM_MODE_CONNECTED) ? TRUE : FALSE;

        /* Do we need only HDMI connection status? */
        if (TRUE == needOnlyConnSts)
        {
            /* We got connection status */
            retVal = SUCCESS;
            break;
        }

        /* Check connection modes count */
        if (connector->count_modes == 0)
        {
            EPRINT(GUI_LIB, "valid connector mode not found: [connector_id=%d], [resCnt=%d]", connector->connector_id, resCnt);
            break;
        }

        /* Check all connection modes */
        retVal = FAIL;
        for (mode = 0; mode < connector->count_modes; mode++)
        {
            modeInfo = &connector->modes[mode];
            DPRINT(GUI_LIB, "supported display resolution: %dx%d@%d", modeInfo->hdisplay, modeInfo->vdisplay, modeInfo->vrefresh);

            /* Check for 720p, 1080p & 4K resolution */
            if ((modeInfo->hdisplay == 1280) && (modeInfo->vdisplay == 720))
            {
                /* This is standard hd resolution (1280x720) */
                retVal = SUCCESS;
                if (modeInfo->vrefresh == 50)
                {
                    pVoDispRes[VO_OUTPUT_720P50] = TRUE;
                }
                else if (modeInfo->vrefresh == 60)
                {
                    pVoDispRes[VO_OUTPUT_720P60] = TRUE;
                }
            }
            else if ((modeInfo->hdisplay == 1920) && (modeInfo->vdisplay == 1080))
            {
                /* This is standard full hd resolution (1920x1080) */
                retVal = SUCCESS;
                if (modeInfo->vrefresh == 24)
                {
                    pVoDispRes[VO_OUTPUT_1080P24] = TRUE;
                }
                else if (modeInfo->vrefresh == 25)
                {
                    pVoDispRes[VO_OUTPUT_1080P25] = TRUE;
                }
                else if (modeInfo->vrefresh == 30)
                {
                    pVoDispRes[VO_OUTPUT_1080P30] = TRUE;
                }
                else if (modeInfo->vrefresh == 50)
                {
                    pVoDispRes[VO_OUTPUT_1080P50] = TRUE;
                }
                else if (modeInfo->vrefresh == 60)
                {
                    pVoDispRes[VO_OUTPUT_1080P60] = TRUE;
                }
            }
            else if ((modeInfo->hdisplay == 3840) && (modeInfo->vdisplay == 2160))
            {
                /* This is standard 4k resolution (3840x2160) */
                retVal = SUCCESS;
                if (modeInfo->vrefresh == 24)
                {
                    pVoDispRes[VO_OUTPUT_3840x2160_24] = TRUE;
                }
                else if (modeInfo->vrefresh == 25)
                {
                    pVoDispRes[VO_OUTPUT_3840x2160_25] = TRUE;
                }
                else if (modeInfo->vrefresh == 30)
                {
                    pVoDispRes[VO_OUTPUT_3840x2160_30] = TRUE;
                }
                #if defined(RK3588_NVRH) /* As per rockchip's recommendation for RK3568, don't use 4K @ 60Hz for performance */
                else if (modeInfo->vrefresh == 50)
                {
                    pVoDispRes[VO_OUTPUT_3840x2160_50] = TRUE;
                }
                else if (modeInfo->vrefresh == 60)
                {
                    pVoDispRes[VO_OUTPUT_3840x2160_60] = TRUE;
                }
                #endif
            }
        }

        /* Required connector found */
        break;
    }

    /* Free DRM connector */
    if (connector)
    {
        drmModeFreeConnector(connector);
    }

    /* Free DRM resources */
    if (resources)
    {
        drmModeFreeResources(resources);
    }

    /* Close drm device */
    if (drmFd != -1)
    {
        close(drmFd);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add new resolution changes decoder usage.
 * @param   dispChnlId
 * @param   oldHeight
 * @param   oldWidth
 * @param   oldFps
 * @param   newHeight
 * @param   newWidth
 * @param   newFps
 * @return  Return SUCCESS if capability available and add in usage else return FAIL
 */
static BOOL addDecoderUsage(DEC_DISP_PLAY_ID dispChnlId, UINT16 oldHeight, UINT16 oldWidth, UINT16 oldFps, UINT16 newHeight, UINT16 newWidth, UINT16 newFps)
{
    UINT32	usedDecCap;
    UINT32	oldChnlCap = (oldWidth * oldHeight) * oldFps;
    UINT32  newChnlCap = (newWidth * newHeight) * newFps;

    /* Lock for decoder usage update */
    GUI_LIB_MUTEX_LOCK(currentUsageMutex);
    if (oldChnlCap > decoderCurrentUsage)
    {
        /* Unlock decoder usage on failure */
        usedDecCap = decoderCurrentUsage;
        GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
        EPRINT(GUI_LIB, "error in decoding capacity: [dispChnlId=%d], [acquired=%u], [used=%u], [free=%u]",
               dispChnlId, oldChnlCap, usedDecCap, (DECODER_CAPABILITY_MAX - usedDecCap));
        return FAIL;
    }

    /* Derived final decoding capacity */
    usedDecCap = decoderCurrentUsage - oldChnlCap + newChnlCap;

    /* Is decoding capacity available? */
    if (usedDecCap > DECODER_CAPABILITY_MAX)
    {
        /* No more decoding capacity available */
        usedDecCap = decoderCurrentUsage;
        GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
        EPRINT(GUI_LIB, "no decoding capacity: [dispChnlId=%d], [used=%u], [free=%u], [required=%u], [resolution=%dx%d@%d]",
               dispChnlId, usedDecCap, (DECODER_CAPABILITY_MAX - usedDecCap), newChnlCap, newWidth, newHeight, newFps);
        return FAIL;
    }

    /* Set final decoding usage after acquiring new capacity */
    decoderCurrentUsage = usedDecCap;
    GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
    DPRINT(GUI_LIB, "add decoding capacity: [dispChnlId=%d], [acquired=%u], [used=%u], [free=%u]",
           dispChnlId, newChnlCap, usedDecCap, (DECODER_CAPABILITY_MAX - usedDecCap));
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove the current decoder usage of NVR system.
 * @param   dispChnlId
 * @param   height
 * @param   width
 * @param   fps
 */
static void removeDecoderUsage(DEC_DISP_PLAY_ID dispChnlId, UINT16 height, UINT16 width, UINT16 fps)
{
    UINT32  chnlCap;
    UINT32	usedDecCap;

    /* Nothing to if input params are 0 */
    if ((width == 0) || (height == 0) || (fps == 0))
    {
        return;
    }

    /* Derive channel decoding capacity */
    chnlCap = width * height * fps;

    /* Lock for decoder usage update */
    GUI_LIB_MUTEX_LOCK(currentUsageMutex);

    /* Update final decoding capacity by removing channel capacity */
    usedDecCap = decoderCurrentUsage = (chnlCap > decoderCurrentUsage) ? 0 : (decoderCurrentUsage - chnlCap);

    /* Unlock after decoder usage update */
    GUI_LIB_MUTEX_UNLOCK(currentUsageMutex);
    DPRINT(GUI_LIB, "remove decoding capacity:  [dispChnlId=%d], [released=%u], [used=%u], [free=%u], [resolution=%dx%d@%d]",
           dispChnlId, chnlCap, usedDecCap, (DECODER_CAPABILITY_MAX - usedDecCap), width, height, fps);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It proivdes decoder from window
 * @param   windowId
 * @return
 */
static UINT8 getDecoderId(UINT8 windowId)
{
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;
    RK_S32      s32Ret;

    stDestChn.enModId = RK_ID_VO;
    stDestChn.s32DevId = VO_LAYER_ID;
    stDestChn.s32ChnId = windowId;

    /* Obtains the information about a bound data source */
    s32Ret = RK_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
    if (s32Ret != RK_SUCCESS)
    {
        if (s32Ret != RK_ERR_SYS_ILLEGAL_PARAM)
        {
            API_FAIL_DEBUG("RK_MPI_SYS_GetBindbyDest", s32Ret);
        }

        return DECODER_WINDOW_MAX;
    }

    /* If window is bound with VPSS then provide device id else provide channel id (bound with decoder) */
    return (stSrcChn.enModId == RK_ID_VPSS) ? stSrcChn.s32DevId : stSrcChn.s32ChnId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It proivdes window from display channel
 * @param   dispChnlId
 * @return
 */
static UINT8 getWindowId(DEC_DISP_PLAY_ID dispChnlId)
{
    UINT8 windowId;

    for(windowId = 0; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        /* Is this window free? */
        if (dispChnlWindMap[windowId].status == FREE)
        {
            /* Do not consider this window as it is free */
            continue;
        }

        /* Is this required window? */
        if (dispChnlWindMap[windowId].dispChnlId == dispChnlId)
        {
            /* Required window found */
            break;
        }
    }

    return windowId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get display channel from window
 * @param   windowId
 * @return  Window mapped display channel
 */
static DEC_DISP_PLAY_ID getDispChnlId(UINT8 windowId)
{
    /* Is this channel free? */
    if (FREE == dispChnlWindMap[windowId].status)
    {
        /* Display channel is not in use for this window */
        return INVALID_DEC_DISP_ID;
    }

    /* Get window mapped display channel */
    return dispChnlWindMap[windowId].dispChnlId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function Sets the the attributes of a VO channel According to selected  WINDOW_LAYOUT.
 * @param   windLayoutId
 * @param   pWinGeometry
 * @return
 */
static BOOL generateWindowGeometry(WIND_LAYOUT_ID_e windLayoutId, WIN_GEOMETRY_PARAM_t *pWinGeometry)
{
    UINT32              windowId = 0, colMax = 0, rowMax = 0;
    UINT32              row = 0, col = 0, widthAlign = 2, heightAlign = 2;
    VALID_SCREEN_INFO_t screenInfo = {0};

    /* Validate input params */
    if (windLayoutId >= WIND_LAYOUT_MAX)
    {
        DPRINT(GUI_LIB, "invld window layout: [windLayoutId=%d]", windLayoutId);
        return FALSE;
    }

    /* Get get sceen info based on live view and sync playback window layout */
    if (windLayoutId >= WIND_LAYOUT_1X1_PLAYBACK)
    {
        getSyncPlaybackScreenInfo(FALSE, &screenInfo, windLayoutId);
    }
    else
    {
        getLiveViewScreenInfo(FALSE, &screenInfo, windLayoutId);
    }

    /* Get total number of windows (tiles) */
    pWinGeometry->totalNumberWin = maxWindPerLayout[windLayoutId];
    DPRINT(GUI_LIB, "[actualWidth=%d], [actualHeight=%d], [actualStartX=%d], [actualStartY=%d], [windLayout=%d], [totalWindow=%d]",
           screenInfo.actualWidth, screenInfo.actualHeight, screenInfo.actualStartX, screenInfo.actualStartY, windLayoutId, pWinGeometry->totalNumberWin);

    switch (windLayoutId)
    {
        case WIND_LAYOUT_1X1_1CH:
        case WIND_LAYOUT_2X2_4CH:
        case WIND_LAYOUT_3X3_9CH:
        case WIND_LAYOUT_4X4_16CH:
        case WIND_LAYOUT_5x5_25CH:
        case WIND_LAYOUT_6x6_36CH:
        case WIND_LAYOUT_8x8_64CH:
        {
            rowMax = colMax = winMultiple[windLayoutId];
            for(row = 0; row < rowMax; row++)
            {
                for(col = 0; col < colMax; col++)
                {
                    windowId = (row * colMax) + col;
                    pWinGeometry->winWidth[windowId]  = screenInfo.actualWidth/colMax;
                    pWinGeometry->winHeight[windowId] = screenInfo.actualHeight/rowMax;
                    pWinGeometry->startX[windowId] = pWinGeometry->winWidth[windowId]*col + screenInfo.actualStartX;
                    pWinGeometry->startY[windowId] = pWinGeometry->winHeight[windowId]*row + screenInfo.actualStartY;
                }
            }
        }
        break;

        case WIND_LAYOUT_1X5_6CH:
        {
            rowMax = 2;
            colMax = 3;
            pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth*rowMax)/colMax, widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight*rowMax)/colMax, heightAlign);
            pWinGeometry->startX[windowId] = screenInfo.actualStartX;
            pWinGeometry->startY[windowId] = screenInfo.actualStartY;

            for(row = 0; row < rowMax; row++)
            {
                windowId++;
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(pWinGeometry->winWidth[0]/2, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(pWinGeometry->winHeight[0]/2, heightAlign);
                pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[0] + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[windowId]*row + screenInfo.actualStartY);
            }

            for(col = 0; col < colMax; col++)
            {
                windowId++;
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(pWinGeometry->winWidth[0]/2, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(pWinGeometry->winHeight[0]/2, heightAlign);
                pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[windowId]*col + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[0] + screenInfo.actualStartY);
            }
        }
        break;

        case WIND_LAYOUT_1X7_8CH:
        {
            pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/4)*3, widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/4)*3, heightAlign);
            pWinGeometry->startX[windowId] = screenInfo.actualStartX;
            pWinGeometry->startY[windowId] = screenInfo.actualStartY;

            for(row = 0; row < 3; row++)
            {
                windowId++;
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/4, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(pWinGeometry->winHeight[0]/3, heightAlign);
                pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[0] + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[windowId]*row + screenInfo.actualStartY);
            }

            for(col = 0; col < 4; col++)
            {
                windowId++;
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/4, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(screenInfo.actualHeight/4, heightAlign);
                pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId]*col) + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[0] + screenInfo.actualStartY);
            }
        }
        break;

        case WIND_LAYOUT_1X9_10CH:
        {
            pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/5)*4, widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/5)*4, heightAlign);
            pWinGeometry->startX[windowId] = screenInfo.actualStartX;
            pWinGeometry->startY[windowId] = screenInfo.actualStartY;

            for(row = 0; row < 4; row++)
            {
                windowId++;
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/5, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(pWinGeometry->winHeight[0]/4, heightAlign);
                pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[0] + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId]*row) + screenInfo.actualStartY);
            }

            for(col = 0; col < 5; col++)
            {
                windowId++;
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/5, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(screenInfo.actualHeight/5, heightAlign);
                pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId]*col) + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[0] + screenInfo.actualStartY);
            }
        }
        break;

        case WIND_LAYOUT_3X4_7CH:
        {
            for(col = 0; col < 2; col++)
            {
                for(row = 0; row < 2; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/2), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/2), heightAlign);
                    pWinGeometry->startX[windowId] = (((screenInfo.actualWidth/2) * row) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = (((screenInfo.actualHeight/2) * col) + screenInfo.actualStartY);
                    windowId++;
                }
            }

            windowId--;
            rowMax = colMax = 4;
            for(col = 0; col < 2; col++)
            {
                for(row = 0; row < 2; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/colMax), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/rowMax), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId] * (row + 2)) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId] * (col + 2)) + screenInfo.actualStartY);
                    windowId++;
                }
            }
        }
        break;

        case WIND_LAYOUT_2X8_10CH:
        {
            for(row = 0; row < 2; row++)
            {
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/2, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(screenInfo.actualHeight/2, heightAlign);
                pWinGeometry->startX[windowId] = (((screenInfo.actualWidth/2) * row) + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = screenInfo.actualStartY;
                windowId++;
            }

            rowMax = colMax = 4;
            for(col = 0; col < 2; col++)
            {
                for(row = 0; row < rowMax; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/colMax), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/rowMax), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId] * row) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId]*(col + 2)) + screenInfo.actualStartY);
                    windowId++;
                }
            }
        }
        break;

        case WIND_LAYOUT_1X12_13CH:
        {
            pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth /2), widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/2), heightAlign);
            pWinGeometry->startX[windowId] = screenInfo.actualStartX;
            pWinGeometry->startY[windowId] = screenInfo.actualStartY;

            rowMax = colMax = 2;
            for(col = 0; col < colMax; col++)
            {
                for(row = 0; row < rowMax; row++)
                {
                    windowId++;
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/4), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/4), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId] * (row + 2)) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId] * col) + screenInfo.actualStartY);
                }
            }

            rowMax = colMax = 4;
            for(col = 0; col < 2; col++)
            {
                for(row = 0; row < rowMax; row++)
                {
                    windowId++;
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/colMax), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/rowMax), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId] * row) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId]*(col + 2)) + screenInfo.actualStartY);
                }
            }
        }
        break;

        case WIND_LAYOUT_1CX12_13CH:
        {
            windowId = 5;
            pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/2), widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/2), heightAlign);
            pWinGeometry->startX[windowId] = ((screenInfo.actualWidth/4) + screenInfo.actualStartX);
            pWinGeometry->startY[windowId] = ((screenInfo.actualHeight/4) + screenInfo.actualStartY);

            windowId = 0;
            rowMax = colMax = 4;
            for(col = 0; col < 1; col++)
            {
                for(row = 0; row < rowMax; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/rowMax), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/colMax), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId] * row) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId] * col) + screenInfo.actualStartY);
                    windowId++;
                }
            }

            for(col = 0; col < 2; col++)
            {
                for(row = 0; row < 2; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/4, widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK(screenInfo.actualHeight/4, heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId]*(row * 3)) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId]*(col+1)) + screenInfo.actualStartY);
                    // because we need to skip 5 number channel
                    windowId++;
                    if(windowId == 5)
                    {
                        windowId++;
                    }
                }
            }

            for(col = 0; col < 1; col++)
            {
                for(row = 0; row < rowMax; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/rowMax), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/colMax), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId] * row) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = ((pWinGeometry->winHeight[windowId] * 3) + screenInfo.actualStartY);
                    windowId++;
                }
            }
        }
        break;

        case WIND_LAYOUT_4X9_13CH:
        {
            for(col = 0; col < 2; col++)
            {
                for(row = 0; row < 2; row++)
                {
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK(((screenInfo.actualWidth/5)*2), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK(((screenInfo.actualHeight/5)*2), heightAlign);
                    pWinGeometry->startX[windowId] = ((pWinGeometry->winWidth[windowId]*row) + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[windowId]*col + screenInfo.actualStartY);
                    windowId++;
                    if(windowId == 2)
                    {
                        windowId = 4;
                    }
                }
            }

            windowId = 2;
            for(row = 0; row < 4; row++)
            {
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/5, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/5), heightAlign);
                pWinGeometry->startX[windowId] = (((screenInfo.actualWidth/5)*4) + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[windowId]*row + screenInfo.actualStartY) ;
                windowId++;
                if(windowId == 4)
                {
                    windowId = 6;
                }
            }

            for(col = 0; col < 5; col++)
            {
                pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/5, widthAlign);
                pWinGeometry->winHeight[windowId] = ALIGN_BACK(screenInfo.actualHeight/5, heightAlign);
                pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[windowId]*col + screenInfo.actualStartX);
                pWinGeometry->startY[windowId] = (((screenInfo.actualHeight/5)*4) + screenInfo.actualStartY);
                windowId++;
            }
        }
        break;

        case WIND_LAYOUT_2X12_14CH:
        {
            pWinGeometry->winWidth[windowId] = ALIGN_BACK(((screenInfo.actualWidth/5)*3), widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK(((screenInfo.actualHeight/5)*3), heightAlign);
            pWinGeometry->startX[windowId] = screenInfo.actualStartX;
            pWinGeometry->startY[windowId]  = screenInfo.actualStartY;
            windowId++;
            pWinGeometry->winWidth[windowId] = ALIGN_BACK(((screenInfo.actualWidth/5)*2), widthAlign);
            pWinGeometry->winHeight[windowId] = ALIGN_BACK(((screenInfo.actualHeight/5)*2), heightAlign);
            pWinGeometry->startX[windowId] = (((screenInfo.actualWidth/5)*3) + screenInfo.actualStartX);
            pWinGeometry->startY[windowId]  = screenInfo.actualStartY;

            row = 3;
            rowMax = colMax = 5;
            for(col = 2; col < colMax; col++)
            {
                for(; row < rowMax; row++)
                {
                    windowId++;
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK((screenInfo.actualWidth/rowMax), widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK((screenInfo.actualHeight/colMax), heightAlign);
                    pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[windowId]*row + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId]  = (pWinGeometry->winHeight[windowId]*col + screenInfo.actualStartY);
                }
                row = 0;
            }
        }
        break;

        case WIND_LAYOUT_1X1_PLAYBACK:
        {
            pWinGeometry->startX[windowId] = screenInfo.actualStartX;
            pWinGeometry->startY[windowId] = screenInfo.actualStartY;
            pWinGeometry->winWidth[windowId] = screenInfo.actualWidth;
            pWinGeometry->winHeight[windowId] = screenInfo.actualHeight;
        }
        break;

        case WIND_LAYOUT_2X2_PLAYBACK:
        case WIND_LAYOUT_3X3_PLAYBACK:
        case WIND_LAYOUT_4X4_PLAYBACK:
        {
            if (screenInfo.actualWidth == 1024)
            {
                screenInfo.actualWidth = 848;
            }

            rowMax = colMax = winMultiple[windLayoutId];
            for(row = 0; row < rowMax; row++)
            {
                for(col = 0; col < colMax; col++)
                {
                    windowId = row*colMax + col;
                    pWinGeometry->winWidth[windowId] = ALIGN_BACK(screenInfo.actualWidth/colMax, widthAlign);
                    pWinGeometry->winHeight[windowId] = ALIGN_BACK(screenInfo.actualHeight/rowMax, heightAlign);
                    pWinGeometry->startX[windowId] = (pWinGeometry->winWidth[windowId]*col + screenInfo.actualStartX);
                    pWinGeometry->startY[windowId] = (pWinGeometry->winHeight[windowId]*row + screenInfo.actualStartY);
                }
            }
        }
        break;

        default:
        {
            EPRINT(GUI_LIB, "invld window layout: [windLayoutId=%d]", windLayoutId);
        }
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete the decoder and related resources
 * @param   decoderId
 * @param   windowId
 * @return
 */
static void deInitDecoderResources(UINT8 decoderId, UINT8 windowId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stDestChn.enModId = RK_ID_VO;
    stDestChn.s32DevId = VO_LAYER_ID;
    stDestChn.s32ChnId = windowId;

    /* Obtains the information about a bound data source */
    s32Ret = RK_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
    if (s32Ret != RK_SUCCESS)
    {
        if (deleteVdecChnl(decoderId) != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "deleteVdecChnl failed: [decoderId=%d], [windowId=%d]", decoderId, windowId);
        }

        if (s32Ret != RK_ERR_SYS_ILLEGAL_PARAM)
        {
            EPRINT(GUI_LIB, "RK_MPI_SYS_GetBindbyDest failed: [decoderId=%d], [windowId=%d], [s32Ret=0x%x]", decoderId, windowId, s32Ret);
        }
        return;
    }

    /* If VO chnl is currently bind to VPSS, then delete VPSS Group, VPSS Chnl and VDEC Chnl */
    if (stSrcChn.enModId == RK_ID_VPSS)
    {
        /* Validate decoder */
        if (decoderId != stSrcChn.s32DevId)
        {
            WPRINT(GUI_LIB, "decoder mismatched: [required=%d], [actual=%d], [windowId=%d]", decoderId, stSrcChn.s32DevId, windowId);
        }

        /* Remove VPSS to VO link */
        s32Ret = unbindVpss2Vo(stSrcChn.s32DevId, windowId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "unbindVpss2Vo failed: [decoderId=%d], [windowId=%d]", stSrcChn.s32DevId, windowId);
        }

        /* Unbind VDEC and VPSS */
        s32Ret = unbindVdec2Vpss(stSrcChn.s32DevId, stSrcChn.s32DevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "unbindVdec2Vpss failed: [decoderId=%d], [windowId=%d]", stSrcChn.s32DevId, windowId);
        }

        /* Delete VPSS group for video processing */
        s32Ret = deleteVpssGrp(stSrcChn.s32DevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "createVpssGrp failed: [decoderId=%d], [windowId=%d]", stSrcChn.s32DevId, windowId);
        }

        /* Delete VDEC Chnl */
        if (deleteVdecChnl(stSrcChn.s32DevId) != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "deleteVdecChnl failed: [decoderId=%d], [windowId=%d]", stSrcChn.s32DevId, windowId);
        }
    }
    else
    {
        /* Validate decoder */
        if (decoderId != stSrcChn.s32ChnId)
        {
            WPRINT(GUI_LIB, "decoder mismatched: [required=%d], [actual=%d], [windowId=%d]", decoderId, stSrcChn.s32ChnId, windowId);
        }

        /* Unbind VDEC and VO */
        s32Ret = unbindVdec2Vo(stSrcChn.s32ChnId, windowId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "unbindVdec2Vo failed: [decoderId=%d], [windowId=%d]", stSrcChn.s32ChnId, windowId);
        }

        /* Delete VDEC Chnl */
        if (deleteVdecChnl(stSrcChn.s32ChnId) != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "deleteVdecChnl failed: [decoderId=%d], [windowId=%d]", stSrcChn.s32ChnId, windowId);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of RK MPP
 * @return  SUCCESS/FAIL
 * @note    Memory allocation default mode is module (MB_SOURCE_MODULE)
 */
static BOOL initRkMpp(void)
{
    RK_S32              s32Ret;
    MOD_ID_E            enModId;
    LOG_LEVEL_CONF_S    stLogConf;

    /* Set debug levels of each module */
    for (enModId = 0; enModId < RK_ID_BUTT; enModId++)
    {
        /* Set RKMPI debug level */
        stLogConf.enModId = enModId;
        stLogConf.s32Level = 0;
        stLogConf.cModName[0] = '\0';
        RK_MPI_LOG_SetLevelConf(&stLogConf);
    }

    /* Initialization of Rockchip MPP */
    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Init", s32Ret);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   De-initialization of RK MPP
 * @return  SUCCESS/FAIL
 */
static BOOL exitRkMpp(void)
{
    /* De-initialization of Rockchip MPP */
    RK_S32 s32Ret = RK_MPI_SYS_Exit();
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Exit", s32Ret);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function start VO device, VO layer and VO channel
 * @param   voResolution
 * @return
 */
static BOOL initVoModule(VO_INTF_SYNC_E voResolution)
{
    RK_S32 s32Ret;

    if (voResolution >= VO_OUTPUT_BUTT)
    {
        EPRINT(GUI_LIB, "invld resolution type: [voResolution=%d]", voResolution);
        return FAIL;
    }

    s32Ret = startVoDev(voResolution);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("startVoDev", s32Ret);
        return FAIL;
    }

    s32Ret = startVoLayer(voResolution);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("startVoLayer", s32Ret);
        stopVoDev();
        return FAIL;
    }

    s32Ret = createVoChnlLayout(displayInfo.windowLayout);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("createVoChnlLayout", s32Ret);
        stopVoDev();
        stopVoLayer();
        return FAIL;
    }

    currVoResolution = voResolution;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   create decoder channel
 * @param   decoderId
 * @param   pStVdecChnAttr
 * @param   pStVdecPicBufAttr
 * @return
 */
static RK_S32 createVdecChnl(UINT8 decoderId, VDEC_CHN_ATTR_S *pStVdecChnAttr, VDEC_PIC_BUF_ATTR_S *pStVdecPicBufAttr)
{
    RK_S32              s32Ret;
    VDEC_CHN_PARAM_S    stVdecChnParam = {0};

    /* Validate input params */
    if (decoderId >= DECODER_WINDOW_MAX)
    {
        EPRINT(GUI_LIB, "invld decoder: [decoderId=%d]", decoderId);
        return RK_ERR_VDEC_INVALID_CHNID;
    }

    /*  Creates a VDEC channel. */
    s32Ret = RK_MPI_VDEC_CreateChn(decoderId, pStVdecChnAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VDEC_CreateChn", s32Ret);
        return s32Ret;
    }

    /*  Update VDEC channel param */
    s32Ret = RK_MPI_VDEC_GetChnParam(decoderId, &stVdecChnParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VDEC_GetChnParam", s32Ret);
        return s32Ret;
    }

    /* Disable compression for MJPEG as it doesn't support it */
    stVdecChnParam.stVdecVideoParam.enCompressMode = pStVdecPicBufAttr->stPicBufAttr.enCompMode;
    s32Ret = RK_MPI_VDEC_SetChnParam(decoderId, &stVdecChnParam);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VDEC_SetChnParam", s32Ret);
        return s32Ret;
    }

    /* Starts to receive the streams sent by the user*/
    s32Ret = RK_MPI_VDEC_StartRecvStream(decoderId);
    if (s32Ret != RK_SUCCESS)
    {
        /* Destroys a VDEC channel. */
        RK_MPI_VDEC_DestroyChn(decoderId);
        API_FAIL_DEBUG("RK_MPI_VDEC_StartRecvStream", s32Ret);
        return s32Ret;
    }

    /* Sets the display mode */
    s32Ret = RK_MPI_VDEC_SetDisplayMode(decoderId, VIDEO_DISPLAY_MODE_PREVIEW);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VDEC_SetDisplayMode", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete decoder channel
 * @param   decoderId
 * @return
 */
static RK_S32 deleteVdecChnl(UINT8 decoderId)
{
    RK_S32 s32Ret;

    if (decoderId >= DECODER_WINDOW_MAX)
    {
        return RK_SUCCESS;
    }

    /* Stops receiving the streams sent by the user. */
    s32Ret = RK_MPI_VDEC_StopRecvStream(decoderId);
    if (s32Ret != RK_SUCCESS)
    {
        /* Is decoder exist and error found? */
        if (s32Ret == RK_ERR_VDEC_UNEXIST)
        {
            /* Decoder doesn't exist. Avoid to log error */
            return RK_SUCCESS;
        }

        API_FAIL_DEBUG("RK_MPI_VDEC_StopRecvStream", s32Ret);
        return s32Ret;
    }

    /* Destroys a VDEC channel */
    s32Ret = RK_MPI_VDEC_DestroyChn(decoderId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VDEC_DestroyChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Creates a VPSS group, Sets the parameters of a VPSS channel attributes then Enables a
 *          VPSS channel and Start a VPSS group.
 * @param   VpssGrp
 * @param   pStVdecChnAttr
 * @return
 */
static RK_S32 createVpssGrp(UINT8 VpssGrp, VDEC_CHN_ATTR_S *pStVdecChnAttr)
{
    VPSS_GRP_ATTR_S     stGrpAttr = {0};
    VPSS_CHN_ATTR_S     stChnAttr = {0};
    RK_S32              s32Ret;

    /* Set VPSS group attributes */
    stGrpAttr.u32MaxW = VPSS_MAX_IMAGE_WIDTH;
    stGrpAttr.u32MaxH = VPSS_MAX_IMAGE_HEIGHT;
    stGrpAttr.enPixelFormat = RK_FMT_YUV420SP;
    stGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stGrpAttr.enCompressMode = COMPRESS_AFBC_16x16;

    /* Create VPSS group */
    s32Ret = RK_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_CreateGrp", s32Ret);
        return s32Ret;
    }

    /* Enable VPSS channel with frame */
    stChnAttr.enChnMode = VPSS_CHN_MODE_USER;
    stChnAttr.u32Width = pStVdecChnAttr->u32PicWidth;
    stChnAttr.u32Height = pStVdecChnAttr->u32PicHeight;
    stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
    stChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    stChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stChnAttr.stFrameRate.s32DstFrameRate = -1;

    /* Sets VPSS channel attributes. We have only one channel per group (0th) */
    s32Ret = RK_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PER_GRP_CHNL_NUM, &stChnAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_SetChnAttr", s32Ret);
        return s32Ret;
    }

    /* Enables a VPSS channel */
    s32Ret = RK_MPI_VPSS_EnableChn(VpssGrp, VPSS_PER_GRP_CHNL_NUM);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_EnableChn", s32Ret);
        return s32Ret;
    }

    /* Start a VPSS group */
    s32Ret = RK_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_StartGrp", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disables a VPSS group, Disables a VPSS channel and Destroy it.
 * @param   VpssGrp
 * @return
 */
static RK_S32 deleteVpssGrp(UINT8 VpssGrp)
{
    RK_S32 s32Ret;

    /* Disables a VPSS group */
    s32Ret = RK_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_StopGrp", s32Ret);
        return s32Ret;
    }

    /* Disables a VPSS channel */
    s32Ret = RK_MPI_VPSS_DisableChn(VpssGrp, VPSS_PER_GRP_CHNL_NUM);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_DisableChn", s32Ret);
        return s32Ret;
    }

    /* Destroys a VPSS group */
    s32Ret = RK_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VPSS_DestroyGrp", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sets the the attributes of a VO channels according to selected layout
 * @param   windLayoutId
 * @return
 */
static RK_S32 createVoChnlLayout(WIND_LAYOUT_ID_e windLayoutId)
{
    RK_S32                  windowId;
    RK_S32                  s32Ret;
    VO_CHN_ATTR_S           stChnAttr = {0};
    WIN_GEOMETRY_PARAM_t    winGeometry = {0};

    /* Get the geometry parameters */
    if (FAIL == generateWindowGeometry(windLayoutId, &winGeometry))
    {
        EPRINT(GUI_LIB, "failed to set geometry: [windLayoutId=%d]", windLayoutId);
        return RK_FAILURE;
    }

    for (windowId = 0; windowId < winGeometry.totalNumberWin; windowId++)
    {
        stChnAttr.stRect.s32X = winGeometry.startX[windowId];
        stChnAttr.stRect.s32Y = winGeometry.startY[windowId];
        stChnAttr.stRect.u32Width = winGeometry.winWidth[windowId];
        stChnAttr.stRect.u32Height = winGeometry.winHeight[windowId];
        stChnAttr.u32Priority = 1;
        stChnAttr.bDeflicker = RK_FALSE;

        /* Sets the attributes of a specified VO channel */
        s32Ret = RK_MPI_VO_SetChnAttr(VO_LAYER_ID, windowId, &stChnAttr);
        if (s32Ret != RK_SUCCESS)
        {
            API_FAIL_DEBUG("RK_MPI_VO_SetChnAttr", s32Ret);
            return s32Ret;
        }
    }

    /* Set invalid channel attributes */
    stChnAttr.stRect.s32X = -1;
    stChnAttr.stRect.s32Y = -1;
    stChnAttr.stRect.u32Width = 0;
    stChnAttr.stRect.u32Height = 0;
    stChnAttr.u32Priority = 1;
    stChnAttr.bDeflicker = RK_FALSE;

    /* Set all other window as invalid */
    for (; windowId < DECODER_WINDOW_MAX; windowId++)
    {
        /* Sets the attributes of a specified VO channel */
        s32Ret = RK_MPI_VO_SetChnAttr(VO_LAYER_ID, windowId, &stChnAttr);
        if (s32Ret != RK_SUCCESS)
        {
            API_FAIL_DEBUG("RK_MPI_VO_SetChnAttr", s32Ret);
        }
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start VO device
 * @param   voResolution
 * @return
 */
static RK_S32 startVoDev(VO_INTF_SYNC_E voResolution)
{
    RK_S32          s32Ret;
    VO_PUB_ATTR_S   stVoPubAttr = {0};

    /* Obtains the public attributes of a VO device */
    s32Ret = RK_MPI_VO_GetPubAttr(VO_DEVICE_ID, &stVoPubAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_GetPubAttr", s32Ret);
        return s32Ret;
    }

    /* Set VO public parameters */
    stVoPubAttr.u32BgColor = 0;
    stVoPubAttr.enIntfType = VO_INTF_HDMI;
    stVoPubAttr.enIntfSync = voResolution;

    /* Sets the public attributes of a VO device. */
    s32Ret = RK_MPI_VO_SetPubAttr(VO_DEVICE_ID, &stVoPubAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetPubAttr", s32Ret);
        return s32Ret;
    }

    /* With few displays (e.g. VU TV 55-QDV/C8Z1), HDMI negotiation takes time and if we enable VO immediately,
     * display becomes pink, blue or green. As per recommended by rockchip, provide delay for negotiation. */
    sleep(1);

    /* Enables a VO device. */
    s32Ret = RK_MPI_VO_Enable(VO_DEVICE_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_Enable", s32Ret);
        return s32Ret;
    }

    /* Register HDMI connection detection callback */
    RK_VO_CALLBACK_FUNC_S hotplugHdmiEventHandler;
    hotplugHdmiEventHandler.pfnEventCallback = hdmiEventHandler;
    hotplugHdmiEventHandler.pPrivateData = NULL;
    s32Ret = RK_MPI_VO_RegCallbackFunc(VO_INTF_HDMI, HDMI1_DEVICE_ID, &hotplugHdmiEventHandler);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_RegCallbackFunc", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disables a video Device.
 * @param   VoDev
 * @return
 */
static RK_S32 stopVoDev(void)
{
    RK_S32 s32Ret;

    /* Disables a VO device */
    s32Ret = RK_MPI_VO_Disable(VO_DEVICE_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_Disable", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sets the attributes of a video layer and Enables a video layer.
 * @param   voResolution
 * @return
 */
static RK_S32 startVoLayer(VO_INTF_SYNC_E voResolution)
{
    RK_S32                  s32Ret;
    UINT8                   displayParam;
    UINT32                  screenWidth, screenHeight, frameRate;
    VO_VIDEO_LAYER_ATTR_S   stVoLayerAttr;
    VO_CSC_S                stVideoCsc = {0};

    s32Ret = RK_MPI_VO_SetLayerDispBufLen(VO_LAYER_ID, 4);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetLayerDispBufLen", s32Ret);
        return s32Ret;
    }

    /* Obtains the attributes of the VO layer */
    s32Ret = RK_MPI_VO_GetLayerAttr(VO_LAYER_ID, &stVoLayerAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_GetVideoLayerAttr", s32Ret);
        return s32Ret;
    }

    /* Get output display width, height and framerate */
    getDisplayWidthHeight(voResolution, &screenWidth, &screenHeight, &frameRate);
    stVoLayerAttr.stDispRect.u32Width = screenWidth;
    stVoLayerAttr.stDispRect.u32Height = screenHeight;
    stVoLayerAttr.stImageSize.u32Width = screenWidth;
    stVoLayerAttr.stImageSize.u32Height = screenHeight;
    stVoLayerAttr.u32DispFrmRt = 30;
    stVoLayerAttr.enPixFormat = RK_FMT_RGBA8888;
    #if defined(RK3568_NVRL)
    stVoLayerAttr.bDoubleFrame = RK_TRUE;
    #else
    stVoLayerAttr.bBypassFrame = RK_FALSE;
    #endif
    stVoLayerAttr.enCompressMode = COMPRESS_AFBC_16x16;

    /* Sets the attributes of a video layer */
    s32Ret = RK_MPI_VO_SetLayerAttr(VO_LAYER_ID, &stVoLayerAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetVideoLayerAttr", s32Ret);
        return s32Ret;
    }

    /* Enable the VO layer */
    s32Ret = RK_MPI_VO_EnableLayer(VO_LAYER_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_EnableVideoLayer", s32Ret);
        return s32Ret;
    }

    /* Set display setting params */
    stVideoCsc.enCscMatrix = VO_CSC_MATRIX_IDENTITY;
    stVideoCsc.u32Contrast =  50;
    stVideoCsc.u32Hue = 50;
    stVideoCsc.u32Luma = 50;
    stVideoCsc.u32Satuature = 50;
    s32Ret = RK_MPI_VO_SetLayerCSC(VO_LAYER_ID, &stVideoCsc);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_SetLayerCSC", s32Ret);
        return s32Ret;
    }

    /* Set display setting params */
    for (displayParam = 0; displayParam < DISPLAY_SCREEN_PARAM_MAX; displayParam++)
    {
		/* Set default value of all params */
        SetDisplayParameter(HDMI, displayParam, 50);
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disables a video layer.
 * @param   VoLayer
 * @return
 */
static RK_S32 stopVoLayer(void)
{
    /* Disables a video layer */
    RK_S32 s32Ret = RK_MPI_VO_DisableLayer(VO_LAYER_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_Disable", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the binding between VDEC and VPSS.
 * @param   decoderId
 * @param   VpssGrp
 * @return
 */
static RK_S32 bindVdec2Vpss(UINT8 decoderId, UINT8 VpssGrp)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stSrcChn.enModId = RK_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = decoderId;

    stDestChn.enModId = RK_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = VPSS_PER_GRP_CHNL_NUM;

    /* Binds a data source and a data receiver */
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Bind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete the binding between VDEC and VPSS.
 * @param   decoderId
 * @param   VpssGrp
 * @return
 */
static RK_S32 unbindVdec2Vpss(UINT8 decoderId, UINT8 VpssGrp)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stSrcChn.enModId = RK_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = decoderId;

    stDestChn.enModId = RK_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = VPSS_PER_GRP_CHNL_NUM;

    /* Unbind a data source and a data receiver */
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_UnBind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the binding between VPSS and VO.
 * @param   VpssGrp
 * @param   windowId
 * @return
 */
static RK_S32 bindVpss2Vo(UINT8 VpssGrp, UINT8 windowId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    /* Enables a specified VO channel */
    s32Ret = RK_MPI_VO_EnableChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_EnableChn", s32Ret);
        return s32Ret;
    }

    stSrcChn.enModId = RK_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VPSS_PER_GRP_CHNL_NUM;

    stDestChn.enModId = RK_ID_VO;
    stDestChn.s32DevId = VO_LAYER_ID;
    stDestChn.s32ChnId = windowId;

    /* Binds a data source and a data receiver */
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Bind", s32Ret);
        return s32Ret;
    }

    /* Show a specified channel. */
    s32Ret = RK_MPI_VO_ShowChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_ShowChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete the binding between VPSS and VO.
 * @param   windowId
 * @return
 */
static RK_S32 unbindVpss2Vo(UINT8 VpssGrp, UINT8 windowId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stSrcChn.enModId = RK_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VPSS_PER_GRP_CHNL_NUM;

    stDestChn.enModId = RK_ID_VO;
    stDestChn.s32DevId = VO_LAYER_ID;
    stDestChn.s32ChnId = windowId;

    /* Hides a specified channel. */
    s32Ret = RK_MPI_VO_HideChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_HideChn", s32Ret);
    }

    /* Unbind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_UnBind", s32Ret);
    }

    /* Disables a specified VO channel */
    s32Ret = RK_MPI_VO_DisableChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_DisableChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the binding between decoder and VO.
 * @param   decoderId
 * @param   windowId
 * @return
 */
static RK_S32 bindVdec2Vo(UINT8 decoderId, UINT8 windowId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    /* Enables a specified VO channel */
    s32Ret = RK_MPI_VO_EnableChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_EnableChn", s32Ret);
        return s32Ret;
    }

    stSrcChn.enModId = RK_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = decoderId;

    stDestChn.enModId = RK_ID_VO;
    stDestChn.s32DevId = VO_LAYER_ID;
    stDestChn.s32ChnId = windowId;

    /* Binds a data source and a data receiver */
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Bind", s32Ret);
        return s32Ret;
    }

    /* Show a specified channel. */
    s32Ret = RK_MPI_VO_ShowChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_ShowChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete the binding between decoder and VO.
 * @param   decoderId
 * @param   windowId
 * @return
 */
static RK_S32 unbindVdec2Vo(UINT8 decoderId, UINT8 windowId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   stSrcChn;
    MPP_CHN_S   stDestChn;

    stSrcChn.enModId = RK_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = decoderId;

    stDestChn.enModId = RK_ID_VO;
    stDestChn.s32DevId = VO_LAYER_ID;
    stDestChn.s32ChnId = windowId;

    /* Hides a specified channel. */
    s32Ret = RK_MPI_VO_HideChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_HideChn", s32Ret);
    }

    /* Unbind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_UnBind", s32Ret);
    }

    /* Disables a specified VO channel */
    s32Ret = RK_MPI_VO_DisableChn(VO_LAYER_ID, windowId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_VO_DisableChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Calculate the actual screen height width in multiple of 8 for live-view window.
 * @param   isUiScreenInfoReq: TRUE - Request from UI, FALSE - Internal Library Request
 * @param   pScreenInfo
 * @param   windLayoutId
 * @return
 */
static BOOL getLiveViewScreenInfo(BOOL isUiScreenInfoReq, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId)
{
    UINT32  width, height;
    UINT32  screenWidth, screenHeight, frameRate;
    UINT32  multiple;

    /* Validate input params */
    if (NULL == pScreenInfo)
    {
        EPRINT(GUI_LIB, "null ptr found: validScreen");
        return FAIL;
    }

    /* Get display height and width */
    getDisplayWidthHeight(currVoResolution, &screenWidth, &screenHeight, &frameRate);

    /* Validate window layout */
    if ((windLayoutId < WIND_LAYOUT_1X1_1CH) || (windLayoutId >= WIND_LAYOUT_MAX))
    {
        EPRINT(GUI_LIB, "invld window layout found. fallback to default 4X4_16CH: [windLayoutId=%d]", windLayoutId);
        windLayoutId = WIND_LAYOUT_4X4_16CH;
    }

    /* Get division/multiplication factor for layout calculation */
    multiple = winMultiple[windLayoutId];

    /* If the TV offset changes, the screenWidth and screenHeight value changes accordingly */
    pScreenInfo->actualStartX = LIVE_VIEW_STARTX * tvAdjustOffset;
    pScreenInfo->actualStartY = LIVE_VIEW_STARTY * tvAdjustOffset;
    pScreenInfo->actualWidth = screenWidth - (LIVE_VIEW_WIDTH_OFFSET * tvAdjustOffset);
    pScreenInfo->actualHeight = screenHeight - (LIVE_VIEW_HEIGHT_OFFSET * tvAdjustOffset);

    /* Divide all windows in equal width */
    width = (pScreenInfo->actualWidth / multiple);
    pScreenInfo->actualWidth = width * multiple;

    /* Divide all windows in equal height */
    height = (pScreenInfo->actualHeight / multiple);
    pScreenInfo->actualHeight = height * multiple;

    /* Update x-y cordinates and height-width. It will be divided by 2 when Video layer is 4K and UI is FHD */
    if ((TRUE == isUiScreenInfoReq) && (IS_UI_VO_TO_RESTRICT(currVoResolution)))
    {
        /* Transform vo resolution from 4K to FHD for UI. For other VO resolution, no change required.
         * We have to downscale UI less because Video layer have 4K resoluion but UI have only FHD */
        pScreenInfo->actualStartX /= 2;
        pScreenInfo->actualStartY /= 2;
        pScreenInfo->actualWidth /= 2;
        pScreenInfo->actualHeight /= 2;
    }

    DPRINT(GUI_LIB, "[dispResolution=%d], [windLayoutId=%d], [tvAdjustOffset=%d], [actualStartX=%u], [actualStartY=%u], [actualWidth=%u], [actualHeight=%u]",
           displayInfo.dispResolution, windLayoutId, tvAdjustOffset, pScreenInfo->actualStartX, pScreenInfo->actualStartY, pScreenInfo->actualWidth, pScreenInfo->actualHeight);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Calculate the actual screen height width in multiple of 8 for sync playback window.
 * @param   isUiScreenInfoReq: TRUE - Request from UI, FALSE - Internal Library Request
 * @param   pScreenInfo
 * @param   windLayoutId
 * @return
 */
static BOOL getSyncPlaybackScreenInfo(BOOL isUiScreenInfoReq, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId)
{
    UINT32  width, height;
    UINT32  actualWidth, actualHeight;
    UINT32  screenWidth, screenHeight, frameRate;
    UINT32  multiple;

    /* Validate input params */
    if (NULL == pScreenInfo)
    {
        EPRINT(GUI_LIB, "null ptr found: validScreen");
        return FAIL;
    }

    /* Get display height and width */
    getDisplayWidthHeight(currVoResolution, &screenWidth, &screenHeight, &frameRate);

    /* Validate window layout */
    if (windLayoutId >= WIND_LAYOUT_MAX)
    {
        EPRINT(GUI_LIB, "invld window layout found. fallback to default: [windLayoutId=%d]", windLayoutId);
        windLayoutId = WIND_LAYOUT_2X2_4CH;
    }

    /* Get division/multiplication factor for layout calculation */
    multiple = winMultiple[windLayoutId];

    /* If the TV offset changes, the screenWidth and screenHeight value changes accordingly */
    screenWidth = screenWidth - (LIVE_VIEW_WIDTH_OFFSET * tvAdjustOffset);
    screenHeight = screenHeight - (LIVE_VIEW_HEIGHT_OFFSET * tvAdjustOffset) + (tvAdjustOffset);
    // Additional (tvAdjustOffset) offset is to lessen space between layout and sync PB toolbar

    /* As the timeline is considered for 1080p resolution, hence for other resolution screenHeight and screenWidth divided by 1080p height and width resp.*/
    actualWidth = screenWidth - (SYNCPB_TIMELINE_WIDTH_1080P * screenWidth/1920);
    actualHeight = screenHeight - (SYNCPB_TIMELINE_HEIGHT_1080P * screenHeight/1080);

    pScreenInfo->actualStartX = LIVE_VIEW_STARTX * tvAdjustOffset;
    pScreenInfo->actualStartY = LIVE_VIEW_STARTY * tvAdjustOffset;
    pScreenInfo->actualWidth =  actualWidth;
    pScreenInfo->actualHeight = actualHeight;

    /* width = (validScreen->actualHeight % multiple); */
    if ((pScreenInfo->actualWidth % 8) != 0)
    {
        pScreenInfo->actualWidth -= (pScreenInfo->actualWidth % 8);
    }

    width = (pScreenInfo->actualWidth / multiple);
    if (width % 2 != 0)
    {
        width -= 1;
    }
    pScreenInfo->actualWidth = width * multiple;

    if ((pScreenInfo->actualHeight % 8) != 0)
    {
        pScreenInfo->actualHeight -= (pScreenInfo->actualHeight % 8);
    }

    height = (pScreenInfo->actualHeight / multiple);
    if (height % 2 != 0)
    {
        height -= 1;
    }
    pScreenInfo->actualHeight = height * multiple;

    /* Update x-y cordinates and height-width. It will be divided by 2 when Video layer is 4K and UI is FHD */
    if ((TRUE == isUiScreenInfoReq) && (IS_UI_VO_TO_RESTRICT(currVoResolution)))
    {
        /* Transform vo resolution from 4K to FHD for UI. For other VO resolution, no change required.
         * We have to downscale UI less because Video layer have 4K resoluion but UI have only FHD */
        pScreenInfo->actualStartX /= 2;
        pScreenInfo->actualStartY /= 2;
        pScreenInfo->actualWidth /= 2;
        pScreenInfo->actualHeight /= 2;
    }

    DPRINT(GUI_LIB, "[dispResolution=%d], [windLayoutId=%d], [tvAdjustOffset=%d], [actualStartX=%u], [actualStartY=%u], [actualWidth=%u], [actualHeight=%u]",
           displayInfo.dispResolution, windLayoutId, tvAdjustOffset, pScreenInfo->actualStartX, pScreenInfo->actualStartY, pScreenInfo->actualWidth, pScreenInfo->actualHeight);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get display width and height from resolution
 * @param   voResolution
 * @param   pDispWidth
 * @param   pDispHeight
 * @param   pFrameRate
 */
static void getDisplayWidthHeight(VO_INTF_SYNC_E voResolution, UINT32 *pDispWidth, UINT32 *pDispHeight, UINT32 *pFrameRate)
{
    switch (voResolution)
    {
        case VO_OUTPUT_720P50:
        {
            *pDispWidth = 1280;
            *pDispHeight = 720;
            *pFrameRate = 50;
        }
        break;

        case VO_OUTPUT_720P60:
        {
            *pDispWidth = 1280;
            *pDispHeight = 720;
            *pFrameRate = 60;
        }
        break;

        case VO_OUTPUT_1080P24:
        {
            *pDispWidth = 1920;
            *pDispHeight = 1080;
            *pFrameRate = 24;
        }
        break;

        case VO_OUTPUT_1080P25:
        {
            *pDispWidth = 1920;
            *pDispHeight = 1080;
            *pFrameRate = 25;
        }
        break;

        case VO_OUTPUT_1080P30:
        {
            *pDispWidth = 1920;
            *pDispHeight = 1080;
            *pFrameRate = 30;
        }
        break;

        case VO_OUTPUT_1080P50:
        {
            *pDispWidth = 1920;
            *pDispHeight = 1080;
            *pFrameRate = 50;
        }
        break;

        case VO_OUTPUT_1080P60:
        {
            *pDispWidth = 1920;
            *pDispHeight = 1080;
            *pFrameRate = 60;
        }
        break;

        case VO_OUTPUT_3840x2160_24:
        {
            *pDispWidth = 3840;
            *pDispHeight = 2160;
            *pFrameRate = 24;
        }
        break;

        case VO_OUTPUT_3840x2160_25:
        {
            *pDispWidth = 3840;
            *pDispHeight = 2160;
            *pFrameRate = 25;
        }
        break;

        case VO_OUTPUT_3840x2160_30:
        {
            *pDispWidth = 3840;
            *pDispHeight = 2160;
            *pFrameRate = 30;
        }
        break;

        case VO_OUTPUT_3840x2160_50:
        {
            *pDispWidth = 3840;
            *pDispHeight = 2160;
            *pFrameRate = 50;
        }
        break;

        case VO_OUTPUT_3840x2160_60:
        {
            *pDispWidth = 3840;
            *pDispHeight = 2160;
            *pFrameRate = 60;
        }
        break;

        default:
        {
            *pDispWidth = *pDispHeight = *pFrameRate = 0;
            EPRINT(GUI_LIB, "invld resolution type: [voResolution=%d]", voResolution);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the rockchip VO resolution type from matrix's resolution
 * @param   pDispResolution
 * @return  Rockchip resolution type
 */
static VO_INTF_SYNC_E getNegotiatedVoRkRes(DISPLAY_RESOLUTION_e *pDispResolution)
{
    BOOL                    isHdmiConnected = FALSE;
    BOOL                    voDispRes[VO_OUTPUT_BUTT] = {FALSE};
    VO_INTF_SYNC_E          voResolution = VO_OUTPUT_BUTT;
    DISPLAY_RESOLUTION_e    dispResolution;

    /* Get HDMI display capabilities */
    if (FAIL == getHdmiCapability(FALSE, &isHdmiConnected, voDispRes))
    {
        EPRINT(GUI_LIB, "failed to get hdmi capabilities");
        return voResolution;
    }

    /* Set voResolution based on supported Display Resolution */
    switch(*pDispResolution)
    {
        case DISPLAY_RESOLUTION_2160P:
        {
            dispResolution = DISPLAY_RESOLUTION_2160P;
            if (TRUE == voDispRes[VO_OUTPUT_3840x2160_60])
            {
                voResolution = VO_OUTPUT_3840x2160_60;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_3840x2160_50])
            {
                voResolution = VO_OUTPUT_3840x2160_50;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_3840x2160_30])
            {
                voResolution = VO_OUTPUT_3840x2160_30;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_3840x2160_25])
            {
                voResolution = VO_OUTPUT_3840x2160_25;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_3840x2160_24])
            {
                voResolution = VO_OUTPUT_3840x2160_24;
                break;
            }
        }

       /* Fall Through to check 1080p/720p Display Resolution */
        case DISPLAY_RESOLUTION_1080P:
        default:
        {
            dispResolution = DISPLAY_RESOLUTION_1080P;
            if (TRUE == voDispRes[VO_OUTPUT_1080P60])
            {
                voResolution = VO_OUTPUT_1080P60;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_1080P50])
            {
                voResolution = VO_OUTPUT_1080P50;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_1080P30])
            {
                voResolution = VO_OUTPUT_1080P30;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_1080P25])
            {
                voResolution = VO_OUTPUT_1080P25;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_1080P24])
            {
                voResolution = VO_OUTPUT_1080P24;
                break;
            }
        }

        /* Fall Through to check 720p Display Resolution */
        case DISPLAY_RESOLUTION_720P:
        {
            dispResolution = DISPLAY_RESOLUTION_720P;
            if (TRUE == voDispRes[VO_OUTPUT_720P60])
            {
                voResolution = VO_OUTPUT_720P60;
                break;
            }

            if (TRUE == voDispRes[VO_OUTPUT_720P50])
            {
                voResolution = VO_OUTPUT_720P50;
                break;
            }
        }
        break;
    }

    /* If voResolution still not set, then Initialize Display with configured resolution(Using Default Refresh frequencies) */
    if (VO_OUTPUT_BUTT == voResolution)
    {
        switch(*pDispResolution)
        {
            case DISPLAY_RESOLUTION_720P:
                voResolution = VO_OUTPUT_720P60;
                break;

            case DISPLAY_RESOLUTION_2160P:
                voResolution = VO_OUTPUT_3840x2160_30;
                break;

            default:
            case DISPLAY_RESOLUTION_1080P:
                voResolution = VO_OUTPUT_1080P60;
                break;
        }
    }
    else
    {
        *pDispResolution = dispResolution;
    }

    return voResolution;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start audio output
 * @param   audioCodec
 * @return
 */
static RK_S32 startAudioOut(STREAM_CODEC_TYPE_e audioCodec)
{
    RK_S32          s32Ret;
    AUDIO_DEVICE_e  aoDevId;
    AIO_ATTR_S      stAoAttr = {0};

    stAoAttr.soundCard.sampleRate = AUDIO_SAMPLE_RATE_48000;    /* output device sample rate */
    stAoAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;           /* output bitwidth */
    stAoAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;             /* input data sample rate */
    stAoAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;               /* mono or stereo */
    stAoAttr.u32ChnCnt = 1;                                     /* channel number, valid value:1/2/4/8 */
    stAoAttr.enBitwidth = AUDIO_BIT_WIDTH_16;                   /* bitwidth */
    stAoAttr.u32FrmNum = AUDIO_OUT_BUFF_CNT_MAX;                /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    stAoAttr.u32PtNumPerFrm = AUDIO_OUT_FRAME_SIZE_MAX;         /* point num per frame (80/160/240/320/480/1024/2048) (ADPCM IMA should add 1 point, AMR only support 160) */

    do
    {
        for (aoDevId = 0; aoDevId < AUDIO_OUT_DEVICE_MAX; aoDevId++)
        {
            /* Create ADEC channel */
            s32Ret = createAdecChnl(aoDevId, audioCodec, stAoAttr.enSamplerate, stAoAttr.u32ChnCnt);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "createAdecChnl failed: [adecChnId=%d], [audioCodec=%d]", aoDevId, audioCodec);
                break;
            }

            /* Set AO parameters */
            stAoAttr.soundCard.channels = (aoDevId == AUDIO_OUT_DEVICE_NVR) ? 1 : 2;    /* output device channels */

            /* Enable audio channel */
            s32Ret = enableAoChnl(aoDevId, &stAoAttr);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "enableAoChnl failed: [aoDevId=%d], [audioCodec=%d]", aoDevId, audioCodec);
                break;
            }

            /* Bind adec to ao */
            s32Ret = bindAdec2Ao(aoDevId, aoDevId);
            if (s32Ret != RK_SUCCESS)
            {
                EPRINT(GUI_LIB, "bindAdec2Ao failed: [aoDevId=%d], [audioCodec=%d]", aoDevId, audioCodec);
                break;
            }
        }

        /* Check error occurred?? */
        if (s32Ret != RK_SUCCESS)
        {
            break;
        }

        /* Set Audio out volume level */
        SetAudioVolume(audioStream.audioOutVolume);

        /* Audio out graph created */
        DPRINT(GUI_LIB, "audio started: [audioCodec=%d]", audioCodec);
        return RK_SUCCESS;

    } while(0);

    /* Audio out not graph created */
    stopAudioOut();
    return RK_FAILURE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop audio output
 * @return
 */
static RK_S32 stopAudioOut(void)
{
    RK_S32          s32Ret;
    AUDIO_DEVICE_e  aoDevId;

    DPRINT(GUI_LIB, "waiting to exit from stop audio");
    for (aoDevId = 0; aoDevId < AUDIO_OUT_DEVICE_MAX; aoDevId++)
    {
        /* Wait for the adec to end */
        RK_MPI_ADEC_SendEndOfStream(aoDevId, RK_FALSE);

        s32Ret = unbindAdec2Ao(aoDevId, aoDevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "unbindAdec2Ao failed: [aoDevId=%d]", aoDevId);
        }

        s32Ret = deleteAdecChnl(aoDevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "deleteAdecChnl failed: [aoDevId=%d]", aoDevId);
        }

        s32Ret = disableAoChnl(aoDevId);
        if (s32Ret != RK_SUCCESS)
        {
            EPRINT(GUI_LIB, "disableAoChnl failed: [aoDevId=%d]", aoDevId);
        }
    }

    DPRINT(GUI_LIB, "audio play stopped..!!!");
    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create ADEC channel
 * @param   adecChnId
 * @param   audioCodec
 * @param   samplingRate
 * @param   channels
 * @return
 */
static RK_S32 createAdecChnl(AUDIO_DEVICE_e adecChnId, STREAM_CODEC_TYPE_e audioCodec, UINT32 samplingRate, UINT8 channels)
{
    RK_S32          s32Ret;
    ADEC_CHN_ATTR_S stAdecAttr = {0};

    /* Set decoder audio parameters */
    stAdecAttr.enType = rkAudioCodecMap[audioCodec];
    stAdecAttr.enMode = ADEC_MODE_STREAM;
    stAdecAttr.u32BufCount = AUDIO_OUT_BUFF_CNT_MAX;
    stAdecAttr.u32BufSize = (AUDIO_OUT_FRAME_SIZE_MAX * AUDIO_OUT_BUFF_CNT_MAX);
    stAdecAttr.stCodecAttr.enType = stAdecAttr.enType;
    stAdecAttr.stCodecAttr.u32Channels = channels;
    stAdecAttr.stCodecAttr.u32SampleRate = samplingRate;
    #if defined(RK3568_NVRL)
    stAdecAttr.stCodecAttr.u32BitPerCodedSample = 8;
    #else
    stAdecAttr.stCodecAttr.u32Bitrate = samplingRate * 8;
    #endif

    /* Create decoder channel */
    s32Ret = RK_MPI_ADEC_CreateChn(adecChnId, &stAdecAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_ADEC_CreateChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete ADEC channel
 * @param   adecChnId
 * @return
 */
static RK_S32 deleteAdecChnl(AUDIO_DEVICE_e adecChnId)
{
    RK_S32 s32Ret;

    /* Delete decoder channel */
    s32Ret = RK_MPI_ADEC_DestroyChn(adecChnId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_ADEC_DestroyChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Enable AO channel to start audio
 * @param   aoDevId
 * @param   pStAoAttr
 * @return
 */
static RK_S32 enableAoChnl(AUDIO_DEVICE_e aoDevId, const AIO_ATTR_S *pStAoAttr)
{
    RK_S32 s32Ret;

    s32Ret = RK_MPI_AO_SetPubAttr(aoDevId, pStAoAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_SetPubAttr", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_AO_Enable(aoDevId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_Enable", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_AO_EnableChn(aoDevId, AUDIO_AO_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_EnableChn", s32Ret);
        return s32Ret;
    }

    /* It is recommended to use resampling module */
    s32Ret = RK_MPI_AO_EnableReSmp(aoDevId, AUDIO_AO_CHNL_ID, pStAoAttr->enSamplerate);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_EnableReSmp", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disable AO channel to stop audio
 * @param   aoDevId
 * @return
 */
static RK_S32 disableAoChnl(AUDIO_DEVICE_e aoDevId)
{
    RK_S32 s32Ret;

    s32Ret = RK_MPI_AO_DisableReSmp(aoDevId, AUDIO_AO_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_DisableReSmp", s32Ret);
    }

    s32Ret = RK_MPI_AO_DisableChn(aoDevId, AUDIO_AO_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_DisableChn", s32Ret);
    }

    s32Ret = RK_MPI_AO_Disable(aoDevId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AO_Disable", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the binding between ADEC and AO
 * @param   adecChnId
 * @param   aoDevId
 * @return  Status
 * @note    We bind ADEC 0 --> AO DEV 0 and ADEC 1 --> AO DEV 1
 */
static RK_S32 bindAdec2Ao(AUDIO_DEVICE_e adecChnId, AUDIO_DEVICE_e aoDevId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   srcChnl;
    MPP_CHN_S   dstChnl;

    /* Set binding parameters */
    srcChnl.enModId = RK_ID_ADEC;
    srcChnl.s32DevId = 0;
    srcChnl.s32ChnId = adecChnId;

    dstChnl.enModId = RK_ID_AO;
    dstChnl.s32DevId = aoDevId;
    dstChnl.s32ChnId = AUDIO_AO_CHNL_ID;

    /* Bind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_Bind(&srcChnl, &dstChnl);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Bind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove the binding between ADEC and AO
 * @param   adecChnId
 * @param   aoDevId
 * @return
 */
static RK_S32 unbindAdec2Ao(AUDIO_DEVICE_e adecChnId, AUDIO_DEVICE_e aoDevId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   srcChnl;
    MPP_CHN_S   dstChnl;

    /* Set binding parameters */
    srcChnl.enModId = RK_ID_ADEC;
    srcChnl.s32DevId = 0;
    srcChnl.s32ChnId = adecChnId;

    dstChnl.enModId = RK_ID_AO;
    dstChnl.s32DevId = aoDevId;
    dstChnl.s32ChnId = AUDIO_AO_CHNL_ID;

    /* Bind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_UnBind(&srcChnl, &dstChnl);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_UnBind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create AENC channel
 * @return
 */
static RK_S32 createAencChnl(void)
{
    RK_S32          s32Ret;
    AENC_CHN_ATTR_S stAencAttr = {0};

    /* Set encoder audio parameters */
    stAencAttr.enType = RK_AUDIO_ID_PCM_MULAW;
    stAencAttr.stCodecAttr.enType = stAencAttr.enType;
    stAencAttr.stCodecAttr.u32Channels = 1;
    stAencAttr.stCodecAttr.u32SampleRate = AUDIO_SAMPLE_RATE_8000;
    stAencAttr.stCodecAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAencAttr.u32BufCount = AUDIO_FRAME_BUFF_CNT_MAX;

    /* Create encoder channel */
    s32Ret = RK_MPI_AENC_CreateChn(AUDIO_AENC_CHNL_ID, &stAencAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AENC_CreateChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete AENC channel
 * @return
 */
static RK_S32 deleteAencChnl(void)
{
    RK_S32 s32Ret;

    /* Delete encoder channel */
    s32Ret = RK_MPI_AENC_DestroyChn(AUDIO_AENC_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AENC_DestroyChn", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create AI channel
 * @param   aiDevId
 * @param   pStAoAttr
 * @return
 */
static RK_S32 enableAiChnl(UINT8 aiDevId, const AIO_ATTR_S *pStAoAttr)
{
    RK_S32          s32Ret;
    AI_CHN_PARAM_S  stAiChnPara;

    s32Ret = RK_MPI_AI_SetPubAttr(aiDevId, pStAoAttr);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_SetPubAttr", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_AI_Enable(aiDevId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_Enable", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_AI_EnableChn(aiDevId, AUDIO_AI_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_EnableChn", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_AI_EnableReSmp(aiDevId, AUDIO_AI_CHNL_ID, pStAoAttr->enSamplerate);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_EnableReSmp", s32Ret);
        return s32Ret;
    }

    /* set Ai channel attributes */
    s32Ret = RK_MPI_AI_GetChnParam(aiDevId, AUDIO_AI_CHNL_ID, &stAiChnPara);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_GetChnParam", s32Ret);
        disableAiChnl(AUDIO_AI_DEV_ID);
        return s32Ret;
    }

    stAiChnPara.u32UsrFrmDepth = AUDIO_FRAME_BUFF_CNT_MAX; /* depth of the buffer for storing audio frames */
    s32Ret = RK_MPI_AI_SetChnParam(aiDevId, AUDIO_AI_CHNL_ID, &stAiChnPara);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_SetChnParam", s32Ret);
        disableAiChnl(AUDIO_AI_DEV_ID);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disable AI channel to stop audio
 * @param   aiDevId
 * @return
 */
static RK_S32 disableAiChnl(UINT8 aiDevId)
{
    RK_S32 s32Ret;

    s32Ret = RK_MPI_AI_DisableReSmp(aiDevId, AUDIO_AI_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_EnableReSmp", s32Ret);
    }

    s32Ret = RK_MPI_AI_DisableChn(aiDevId, AUDIO_AI_CHNL_ID);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_DisableChn", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_AI_Disable(aiDevId);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_AI_Disable", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the binding between AI and AENC
 * @param   aiDevId
 * @return
 */
static RK_S32 bindAi2Aenc(UINT8 aiDevId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   srcChnl;
    MPP_CHN_S   dstChnl;

    /* Set binding parameters */
    srcChnl.enModId = RK_ID_AI;
    srcChnl.s32DevId = aiDevId;
    srcChnl.s32ChnId = AUDIO_AI_CHNL_ID;

    dstChnl.enModId = RK_ID_AENC;
    dstChnl.s32DevId = 0;
    dstChnl.s32ChnId = AUDIO_AI_CHNL_ID;

    /* Bind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_Bind(&srcChnl, &dstChnl);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Bind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove the binding between AI and AENC
 * @param   aiDevId
 * @return
 */
static RK_S32 unbindAi2Aenc(UINT8 aiDevId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   srcChnl;
    MPP_CHN_S   dstChnl;

    /* Set binding parameters */
    srcChnl.enModId = RK_ID_AI;
    srcChnl.s32DevId = aiDevId;
    srcChnl.s32ChnId = AUDIO_AI_CHNL_ID;

    dstChnl.enModId = RK_ID_AENC;
    dstChnl.s32DevId = 0;
    dstChnl.s32ChnId = AUDIO_AI_CHNL_ID;

    /* Bind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_UnBind(&srcChnl, &dstChnl);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_UnBind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the binding between AI and AO
 * @param   aiDevId
 * @param   aoDevId
 * @return
 */
static RK_S32 bindAi2Ao(UINT8 aiDevId, AUDIO_DEVICE_e aoDevId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   srcChnl;
    MPP_CHN_S   dstChnl;

    /* Set binding parameters */
    srcChnl.enModId = RK_ID_AI;
    srcChnl.s32DevId = aiDevId;
    srcChnl.s32ChnId = AUDIO_AI_CHNL_ID;

    dstChnl.enModId = RK_ID_AO;
    dstChnl.s32DevId = aoDevId;
    dstChnl.s32ChnId = AUDIO_AO_CHNL_ID;

    /* Bind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_Bind(&srcChnl, &dstChnl);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_Bind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove the binding between AI and AO
 * @param   aiDevId
 * @param   aoDevId
 * @return
 */
static RK_S32 unbindAi2Ao(UINT8 aiDevId, AUDIO_DEVICE_e aoDevId)
{
    RK_S32      s32Ret;
    MPP_CHN_S   srcChnl;
    MPP_CHN_S   dstChnl;

    /* Set binding parameters */
    srcChnl.enModId = RK_ID_AI;
    srcChnl.s32DevId = aiDevId;
    srcChnl.s32ChnId = AUDIO_AI_CHNL_ID;

    dstChnl.enModId = RK_ID_AO;
    dstChnl.s32DevId = aoDevId;
    dstChnl.s32ChnId = AUDIO_AO_CHNL_ID;

    /* Bind specified RK MPI modules */
    s32Ret = RK_MPI_SYS_UnBind(&srcChnl, &dstChnl);
    if (s32Ret != RK_SUCCESS)
    {
        API_FAIL_DEBUG("RK_MPI_SYS_UnBind", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
