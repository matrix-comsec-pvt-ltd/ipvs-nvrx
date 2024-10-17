#ifndef CLIENTMEDIASTREAMER_H_
#define CLIENTMEDIASTREAMER_H_
// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		ClientMediaStreamer.h
@brief      This module used to play audio from any remote client to either device (local client)
            or camera(which supports two way audio).
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* Application Includes */
#include "CameraInterface.h"

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef struct
{
    CLIENT_AUDIO_DESTINATION_e destination;         // where to send audio either device(0) or camera(1)
    UINT8                      cameraIndex;         // 0 on destination=0 and camera index(1-64) on dest=1
    UINT8                      sessionIndex;        // Session index of receive audio request
    NET_CMD_STATUS_e           streamerStatusCode;  // status code tells that why audio was stopped
    INT32                      clientSockFd;        // used to send response
    CLIENT_CB_TYPE_e           clientCbType;        // used to send response
    CAMERA_TYPE_e              camType;             // Camera Type
    TCP_HANDLE                 tcpHandle;
} CLIENT_AUD_INFO_t;

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
void InitClientMediaStreamer(void);
//-------------------------------------------------------------------------------------------------
void DeinitClientMediaStreamer(void);
//-------------------------------------------------------------------------------------------------
void TerminateClientMediaStreamer(UINT8 cameraIndex, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
BOOL IsAudioTransferInProcess(void);
//-------------------------------------------------------------------------------------------------
BOOL StartClientAudioTransfer(CLIENT_AUD_INFO_t *clientAudRxInfo);
//-------------------------------------------------------------------------------------------------
void StopClientAudioReception(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
void UpdateAudioSendFd(INT32 connFd);
//-------------------------------------------------------------------------------------------------
void StopClientAudioTransmission(NET_CMD_STATUS_e stopReasonCode);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ProcessClientAudioToCamera(CLIENT_AUD_INFO_t *clientAudRxInfo);
//-------------------------------------------------------------------------------------------------
void ProcTxAudioToCameraSetupFailure(NET_CMD_STATUS_e statusCode);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @END OF FILE
// #################################################################################################
#endif /* CLIENTMEDIASTREAMER_H_ */
