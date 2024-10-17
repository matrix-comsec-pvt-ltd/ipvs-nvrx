#ifndef NVR_DECODE_DISPLAY_LIB_H
#define NVR_DECODE_DISPLAY_LIB_H

#ifdef __cplusplus
extern "C" {
#endif
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DecDispLib.h
@brief		This file describes interface to hardware decoder module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "../../nvrgui/CommonDef.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Initialization of mutex */
#define GUI_LIB_MUTEX_INIT(x, attr)     pthread_mutex_init(&x, attr);

/* Define to acquire mutex lock */
#define GUI_LIB_MUTEX_LOCK(x)           pthread_mutex_lock(&x)

/* Define to release mutex lock */
#define GUI_LIB_MUTEX_UNLOCK(x)         pthread_mutex_unlock(&x)

/* Geometry starting points */
#define LIVE_VIEW_STARTX                (24)
#define LIVE_VIEW_STARTY                (12)

/* Live view screen height and width offset */
#define LIVE_VIEW_WIDTH_OFFSET          (2 * (LIVE_VIEW_STARTX))
#define LIVE_VIEW_HEIGHT_OFFSET         (2 * LIVE_VIEW_STARTY)

/* Sync Playback Timeline */
#define SYNCPB_TIMELINE_HEIGHT_1080P    104
#define SYNCPB_TIMELINE_WIDTH_1080P     320

/* Display related misc defines */
#define	INVALID_DEC_DISP_ID             255
#define AUDIO_OUT_DEC_CHNL_ID           129
#define ALIGN_BACK(x, a)                ((a) * ((x) / (a)))

#define AUDIO_FRAME_BUFF_CNT_MAX        30
#define	DEFAULT_AUDIO_PLAY_FRAME_LEN    320
#define MAX_AUDIO_BUFFER_SIZE           (1040*1040)
#define	VOLUME_LEVEL_MAX                100     /* Range: 0 to 100 */
#define AUDIO_DEF_VOL_DB                -110
#define AUDIO_REF_VOL_MIN_DB            -50
#define AUDIO_REF_VOL_MAX_DB            +6
#define AUDIO_DB_VOL_MULTIPLIER         ((float)(AUDIO_REF_VOL_MAX_DB - (AUDIO_REF_VOL_MIN_DB))/(float)VOLUME_LEVEL_MAX)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/* decoder display id */
typedef UINT8 DEC_DISP_PLAY_ID;

typedef enum
{
    AUDIO_OFF,
    AUDIO_ON,
    MAX_AUDIO_STATE

}AUDIO_STATE_e;

typedef enum
{
    LIVE_VIDEO_TYPE,
    PLACKBACK_VIDEO_TYPE,
    MAX_VIDEO_TYPE

}VIDEO_TYPE_e;

typedef enum
{
    CHANNEL_UP_SHIFT,
    CHANNEL_DOWN_SHIFT,
    MAX_CHANNEL_SHIFT

}WINDOW_SHIFT_DIR_e;

typedef enum
{
    WIND_LAYOUT_1X1_1CH     = 0,
    WIND_LAYOUT_2X2_4CH     = 1,
    WIND_LAYOUT_1X5_6CH     = 2,
    WIND_LAYOUT_3X4_7CH     = 3,
    WIND_LAYOUT_1X7_8CH     = 4,
    WIND_LAYOUT_3X3_9CH     = 5,
    WIND_LAYOUT_2X8_10CH    = 6,
    WIND_LAYOUT_1X9_10CH    = 7,
    WIND_LAYOUT_1X12_13CH   = 8,
    WIND_LAYOUT_1CX12_13CH  = 9,
    WIND_LAYOUT_4X9_13CH    = 10,
    WIND_LAYOUT_2X12_14CH   = 11,
    WIND_LAYOUT_4X4_16CH    = 12,
    WIND_LAYOUT_5x5_25CH    = 13,
    WIND_LAYOUT_6x6_36CH    = 14,
    WIND_LAYOUT_8x8_64CH    = 15,
    WIND_LAYOUT_1X1_PLAYBACK= 16,
    WIND_LAYOUT_2X2_PLAYBACK= 17,
    WIND_LAYOUT_3X3_PLAYBACK= 18,
    WIND_LAYOUT_4X4_PLAYBACK= 19,
    WIND_LAYOUT_MAX

}WIND_LAYOUT_ID_e;

typedef enum
{
    ZOOM_IN,
    ZOOM_OUT,

}ZOOM_TYPE_e;

typedef enum
{
    DISPLAY_BRIGHTNESS = 0,
    DISPLAY_CONTRAST,
    DISPLAY_SATURATION,
    DISPLAY_HUE,
    DISPLAY_SCREEN_PARAM_MAX

}DISPLAY_SCREEN_PARAM_TYPE_e;

typedef struct
{
    UINT32	actualWidth;
    UINT32	actualHeight;
    UINT32	actualStartX;
    UINT32	actualStartY;

}VALID_SCREEN_INFO_t;

typedef enum
{
    AUDIO_TEST_AO_DEV       = 0,
    AUDIO_TEST_HDMI_AO_DEV  = 1,
    AUDIO_TEST_BOTH_AO_DEV  = 2,
    MAX_AUDIO_TEST_DEVICE

}AUDIO_TEST_DEVICE_e;

/* Add or Update error code here to define common possible errors */
typedef enum
{
    DEC_ERROR_NO_CAPACITY = 0,	// No decoding capacity
    DEC_ERROR_INVALID_ARG,      // Invalid argument
    DEC_ERROR_FRAME_SIZE,       // Invalid frame Len eg, size=0
    DEC_ERROR_INVALID_CODEC,    // Invalid or unsupported codec
    DEC_ERROR_VO,               // VO module Error
    DEC_ERROR_VPSS,             // VPSS module Error
    DEC_ERROR_VDEC,             // VDEC module Error
    DEC_ERROR_HDMI,             // HDMI module Error
    DEC_ERROR_SYS,              // SYS Module Error
    DEC_ERROR_AUDIO,            // Audio Module Error
    DEC_ERROR_VPSS_SCALING,     // VPSS module Failed to scale
    DEC_ERROR_NO_FREE_DEC_ID,   // No free decoding session
    DEC_ERROR_CREATION_FAILED,  // Failed to create decoder
    MAX_DEC_ERROR

}DECODER_ERROR_e;

// This data structure used to zoom in and zoom out for window.
typedef struct
{
    UINT32		startXPos;
    UINT32		startYPos;
    UINT32		width;
    UINT32		height;

}CROP_PARAM_t;

//-------------------------------------------------------------------------------------------------
typedef void (*HDMI_CALLBACK)(BOOL isHdmiInfoShow);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL StartChannelView(DISPLAY_DEV_e dispDevId, UINT8 windowId, DEC_DISP_PLAY_ID *pDispChnlId);
//-------------------------------------------------------------------------------------------------
BOOL StopChannelView(DEC_DISP_PLAY_ID dispChnlId);
//-------------------------------------------------------------------------------------------------
BOOL SwapWindChannel(UINT8 windowId1, UINT8 windowId2, DISPLAY_DEV_e dispDevId);
//-------------------------------------------------------------------------------------------------
BOOL DecodeDispFrame(DEC_DISP_PLAY_ID dispChnlId, FRAME_INFO_t *pFrameInfo, DECODER_ERROR_e *pDecErrCode);
//-------------------------------------------------------------------------------------------------
BOOL GetValidScreenInfoPlayBack(DISPLAY_DEV_e dispDevId, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId);
//-------------------------------------------------------------------------------------------------
BOOL GetValidScreenInfo(DISPLAY_DEV_e dispDevId, VALID_SCREEN_INFO_t *pScreenInfo, WIND_LAYOUT_ID_e windLayoutId);
//-------------------------------------------------------------------------------------------------
BOOL UpdateDecoderStatusOnVideoLoss(DEC_DISP_PLAY_ID dispChnlId, DISPLAY_DEV_e dispDevId, UINT8 windowId);
//-------------------------------------------------------------------------------------------------
BOOL SetCropParam(UINT8 windowId, CROP_PARAM_t *pCropParam, ZOOM_TYPE_e zoomType);
//-------------------------------------------------------------------------------------------------
BOOL InitDecodeDisplay(DISPLAY_RESOLUTION_e resolution);
//-------------------------------------------------------------------------------------------------
void DeInitDecDispLib(void);
//-------------------------------------------------------------------------------------------------
BOOL UpdateChannelToWindowMap(DISPLAY_DEV_e dispDevId, UINT8 offset, WINDOW_SHIFT_DIR_e upOrDownDir, UINT8 selectedWindow, WIND_LAYOUT_ID_e layoutIndex);
//-------------------------------------------------------------------------------------------------
BOOL ExcludeAudio(UINT8 dispChnlId);
//-------------------------------------------------------------------------------------------------
BOOL IncludeAudio(UINT8 dispChnlId);
//-------------------------------------------------------------------------------------------------
BOOL SetAudioMuteUnMute(AUDIO_STATE_e muteState);
//-------------------------------------------------------------------------------------------------
BOOL SetAudioVolume(UINT8 volumeLevel);
//-------------------------------------------------------------------------------------------------
UINT8 GetCurrentAudioChannel(void);
//-------------------------------------------------------------------------------------------------
void GetCurrentMainResolution(DISPLAY_RESOLUTION_e *pResolution, UINT32 *pDispWidth, UINT32 *pDispHeight, UINT32 *pFrameRate);
//-------------------------------------------------------------------------------------------------
BOOL SetDisplayParameter(PHYSICAL_DISPLAY_TYPE_e devId, DISPLAY_SCREEN_PARAM_TYPE_e displayParam, UINT32 value);
//-------------------------------------------------------------------------------------------------
BOOL ChangeWindowLayout(DISPLAY_DEV_e dispDevId, WIND_LAYOUT_ID_e windLayoutId, UINT8 selectedWindow);
//-------------------------------------------------------------------------------------------------
BOOL StartAudioOutTest(AUDIO_TEST_DEVICE_e audDev);
//-------------------------------------------------------------------------------------------------
BOOL StopAudioOutTest(AUDIO_TEST_DEVICE_e audDev);
//-------------------------------------------------------------------------------------------------
void SetTVAdjustOffset(UINT32 offset);
//-------------------------------------------------------------------------------------------------
BOOL ValidateDecoderSyncPb(INT8 dispChnlId);
//-------------------------------------------------------------------------------------------------
void HdmiRegCallBack(HDMI_CALLBACK pHdmiCallBack);
//-------------------------------------------------------------------------------------------------
UINT32 GetMaxDecodingCapacity(void);
//-------------------------------------------------------------------------------------------------
BOOL StartAudioIn(void);
//-------------------------------------------------------------------------------------------------
void StopAudioIn(void);
//-------------------------------------------------------------------------------------------------
BOOL GetNextAudioInFrame(CHARPTR pFrameBuf, INT32PTR pFrameLen);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif // NVR_DECODE_DISPLAY_LIB_H
