#ifndef RTSP_CLIENT_H
#define RTSP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		RtspClient.h
@brief      RTSP client streaming application who communicates with NVR server application and
            provides interface for Live555 client
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Live555Interface.h"
#include "RtspClientInterface.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL AllocateRtpPort(UINT16PTR rtpPortPtr, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
void DeallocRtpPort(UINT16PTR rtpPortPtr, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
BOOL CreateSharedMemory(RTSP_HANDLE mediaHandle, INT32PTR shmFd, UINT8PTR *shmBaseAddr, STREAM_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
BOOL DestroySharedMemory(RTSP_HANDLE mediaHandle, INT32PTR shmFd, UINT8PTR *shmBaseAddr, STREAM_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif //RTSP_CLIENT_H
