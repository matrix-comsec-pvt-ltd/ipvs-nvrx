#if !defined UTILS_COMM_H
#define UTILS_COMM_H

#ifdef __cplusplus
extern "C" {
#endif
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		UtilCommon.h
@brief      File containing the prototype of different common functions for all applications
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <poll.h>

/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DEFINES
//#################################################################################################

/* Initialize poll fd structure */
#define INIT_POLL_FD(pollFd)      {pollFd.fd = INVALID_FILE_FD, pollFd.events = 0, pollFd.revents = 0;}

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
STREAM_CODEC_TYPE_e GetAudioCodec(CHARPTR codecString);
//-------------------------------------------------------------------------------------------------
STREAM_CODEC_TYPE_e GetVideoCodec(CHARPTR codecString);
//-------------------------------------------------------------------------------------------------
INT32 SendToClient(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeoutSec);
//-------------------------------------------------------------------------------------------------
BOOL SendToSocket(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeoutSec);
//-------------------------------------------------------------------------------------------------
UINT8 RecvMessage(INT32 connFd, CHARPTR rcvMsg, UINT32PTR rcvLen, UINT8 startOfData, UINT8 endOfData, UINT32 maxData, UINT32 timeoutSec);
//-------------------------------------------------------------------------------------------------
BOOL RecvFrame(INT32 connFd, CHARPTR rcvMsg, UINT32PTR rcvLen,UINT32 maxData, UINT32 timeoutSec, UINT32 timeoutUs);
//-------------------------------------------------------------------------------------------------
UINT8 GetSocketPollEvent(INT32 connFd, INT16 pollEvent, INT32 timeoutMs, INT16 *recvEvent);
//-------------------------------------------------------------------------------------------------
UINT64 GetRemainingPollTime(UINT64 *prevTimeMs, UINT64 timeoutMs);
//-------------------------------------------------------------------------------------------------
void CloseSocket(INT32PTR connFd);
//-------------------------------------------------------------------------------------------------
void CloseFileFd(INT32PTR fileFd);
//-------------------------------------------------------------------------------------------------
BOOL CheckSocketFdState(INT32 sockFd);
//-------------------------------------------------------------------------------------------------
BOOL Utils_OpenSharedMemory(CHARPTR fileName, size_t sizeInBytes, INT32PTR shmFd, UINT8PTR *shmBaseAddr, BOOL readOnly);
//-------------------------------------------------------------------------------------------------
void Utils_DestroySharedMemory(CHARPTR fileName, size_t sizeInBytes, INT32PTR shmFd, UINT8PTR *shmBaseAddr);
//-------------------------------------------------------------------------------------------------
UINT8 RecvUnixSockSeqPkt(INT32 connFd, void *pData, UINT32 dataLenMax, UINT16 timeoutSec, UINT32 *pReadBytes);
//-------------------------------------------------------------------------------------------------
BOOL SetSockFdOption(INT32 sockFd);
//-------------------------------------------------------------------------------------------------
void PrepareIpAddressForUrl(const CHAR *ipAddr, CHAR *ipAddrForUrl);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif
