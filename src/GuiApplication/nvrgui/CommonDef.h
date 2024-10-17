#if !defined COMMANDEF_H
#define COMMANDEF_H
///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : NVR ( Network Video Recorder with local GUI)
//   Owner        : Meet Modia
//   File         : CommonDef.h
//   Description  : This file describes API and data structures to used the full
// 					functionality of decode and display module.
//
/////////////////////////////////////////////////////////////////////////////

#include "../DecoderLib/include/MxTypedef.h"

#define	INVALID_DEC_DISP_PLAY_ID	(255) // Change
#define DEFAULT_FRAME_RATE          (30)

// It contains information of total number of display device.
typedef enum
{
    MAIN_VIDEO_DISPLAY = 0,
    SPOT_VIDEO_DISPLAY,
    MAX_DISPLAY_DEV
}DISPLAY_DEV_e;

typedef enum
{
    NTSC_VIDEO_STANDARD = 0,
    PAL_VIDEO_STANDARD,
    MAX_VIDEO_STANDARD
}VIDEO_STANDARD_e;

typedef enum
{
    HDMI = 0,
    VGA,
    PHYSICAL_DISPLAY_TYPE_MAX
}PHYSICAL_DISPLAY_TYPE_e;

typedef enum
{
    DISPLAY_RESOLUTION_720P = 0,
    DISPLAY_RESOLUTION_1080P,
    DISPLAY_RESOLUTION_2160P,
    DISPLAY_RESOLUTION_MAX
}DISPLAY_RESOLUTION_e;

typedef enum
{
    LIVE_VIEW_TYPE_SMOOTH = 0,
    LIVE_VIEW_TYPE_REAL_TIME,
    LIVE_VIEW_TYPE_MAX
}LIVE_VIEW_TYPE_e;

typedef enum
{
    STREAM_TYPE_VIDEO = 0,
    STREAM_TYPE_AUDIO,
    MAX_STREAM_TYPE
}STREAM_TYPE_e;

typedef enum
{
    VBR,
    CBR,
    MAX_BITRATE_MODE
}BITRATE_MODE_e;

typedef enum
{
    OSD_NONE,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    MAX_OSD_POSITION
}OSD_POSITION_e;

typedef enum
{
    PHYSICAL_DISPLAY_BRIGHTNESS = 0,
    PHYSICAL_DISPLAY_CONTRAST,
    PHYSICAL_DISPLAY_SATURATION,
    PHYSICAL_DISPLAY_HUE,
    PHYSICAL_DISPLAY_SCREEN_PARAM_MAX
}PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e;

typedef enum
{
    VIDEO_CODEC_NONE = 0,
    VIDEO_MJPG,						// Motion JPEG encoding
    VIDEO_H264,						// H264 encoding
    VIDEO_MPEG4,					// MPEG4 encoding
    VIDEO_CODEC_UNUSED,             // Unused stream codec
    VIDEO_H265,                     // H265 encoding
    MAX_VIDEO_CODEC,

    AUDIO_CODEC_NONE = 0,
    AUDIO_G711_ULAW,
    AUDIO_G726_8,
    AUDIO_G726_16,
    AUDIO_G726_24,
    AUDIO_G726_32,
    AUDIO_G726_40,
    AUDIO_AAC,
    AUDIO_PCM_L,
    AUDIO_PCM_B,
    AUDIO_G711_ALAW,
    MAX_AUDIO_CODEC,

    MAX_CODEC_TYPE = 0xFF
}STREAM_CODEC_TYPE_e;


typedef enum
{
    I_FRAME,
    P_FRAME,
    B_FRAME,
    MAX_ITYPE
}FRAME_TYPE_e;

// This data structure describes frame information.
typedef struct
{
    CHARPTR					framePayload;
    UINT32					frameSize;
    STREAM_TYPE_e			mediaType;
    STREAM_CODEC_TYPE_e		codecType;
    FRAME_TYPE_e			frameType;
    UINT32					frameTimeSec;
    UINT16					frameTimeMSec;
    UINT16					noOfReferanceFrame;
    UINT16					frameWidth;
    UINT16					frameHeight;
    UINT16                  frameRate;
}FRAME_INFO_t;

#endif
