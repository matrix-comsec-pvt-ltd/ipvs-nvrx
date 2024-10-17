#ifndef LIVEMEDIASTREAMER_H
#define LIVEMEDIASTREAMER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		LiveMediaStreamer.h
@brief      This is header file for livemedia stream module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigApi.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitLiveMediaStream(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e AddLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIndex, INT32 connId,
                                    CLIENT_CB_TYPE_e clientCbType, UINT8 reqFrameType, UINT8 reqFrameTypeForMPJEG, UINT8 reqfps);
//-------------------------------------------------------------------------------------------------
BOOL RemoveLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIndex);
//-------------------------------------------------------------------------------------------------
BOOL ChangeLiveMediaAudioState(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIndex, BOOL state);
//-------------------------------------------------------------------------------------------------
BOOL ChangeLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIndex, INT32 connFd,
                           CLIENT_CB_TYPE_e clientCbType, UINT8 reqFrameType, UINT8 reqFrameTypeForMPJEG,UINT8 reqfps);
//-------------------------------------------------------------------------------------------------
UINT32 GetLiveStreamCnt(void);
//-------------------------------------------------------------------------------------------------
void CamMotionNotify(UINT8 camIndex, BOOL evtState);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* LIVEMEDIASTREAMER_H */
