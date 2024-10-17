#if !defined LIVE555_INTERFACE_H
#define LIVE555_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Live555Interface.h
@brief      This module provides functionality of retrieving media (Audio/Video) from IP camera
            using RTSP protocol. Other modules can request media from IP camera by specifying URL
            and one of the supported transport mechanism. It supports RTSP/RTP, TCP interleaved
            and HTTP tunneling as transport. Once media starts flowing, this module passes media
            to requesting module through registered call-back function.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "VideoParser.h"
#include "RtspClientInterface.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
// This callback pass by user for media data
typedef void (*RTSP_CB) (MediaFrameResp_e, RTSP_HANDLE, MEDIA_FRAME_INFO_t*, UINT8, UINT32, UINT32);

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitRtspClient(void);
//-------------------------------------------------------------------------------------------------
void DeinitRtspClient(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartRtspMedia(RTSP_HANDLE mediaHandle, RtspStreamInfo_t *rtspInfo, RTSP_CB callBack);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopRtspMedia(RTSP_HANDLE mediaHandle);
//-------------------------------------------------------------------------------------------------
void UpdateRtpPort(UINT32 rtpPortStart);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif  // LIVE555_INTERFACE_H
