#if !defined P2P_PROCESS_UTILS_H
#define P2P_PROCESS_UTILS_H

//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pProcessUtils.h
@brief      File containing the prototype of helper functions for P2P process
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "P2pProcess.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/** System timer function pointer */
typedef void (*SYS_TIMER_CB)(UINT32 appData1, UINT32 appData2, BOOL isTimerExpired);

/* System timer link list node information */
typedef struct SYS_TIMER_t
{
    /** Time interval in millisec at which tick will be provided to systimer module */
    UINT32 timerTickInMs;

    /** Number of timer ticks to provide required timer */
    UINT32 reloadTimerTicks;

    /** Timer tick count to be wait for timer expire */
    UINT32 ticks;

    /** Application data */
    UINT32 appData1;

    /** Application data */
    UINT32 appData2;

    /** Timer loop count */
    INT32 loopCnt;

    /** Timer expiry callback routine */
    SYS_TIMER_CB callBack;

    /** Reference of next node in the list */
    struct SYS_TIMER_t *pNext;

} SYS_TIMER_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void SysTimerInit(SYS_TIMER_t **pHead);
//-------------------------------------------------------------------------------------------------
BOOL SysTimerConfigure(SYS_TIMER_t *pTmr, UINT32 timerTickInMs, SYS_TIMER_CB callBack);
//-------------------------------------------------------------------------------------------------
BOOL SysTimerStart(SYS_TIMER_t **pHead, SYS_TIMER_t *pTmr, UINT32 appData1, UINT32 appData2, UINT32 intervalInMs, UINT16 loopCnt);
//-------------------------------------------------------------------------------------------------
void SysTimerStop(SYS_TIMER_t **pHead, SYS_TIMER_t *pTmr);
//-------------------------------------------------------------------------------------------------
void SysTimerTick(SYS_TIMER_t **pHead);
//-------------------------------------------------------------------------------------------------
void WaitNextTick(UINT8 clientIdx, UINT64 *pRefTime, UINT64 tickInNs, UINT32 overflowTime);
//-------------------------------------------------------------------------------------------------
UINT16 PrepareJsonMsg(P2P_MSG_INFO_t *pJsonInfo, CHAR *pJsonMsg, UINT16 jsonBufLen);
//-------------------------------------------------------------------------------------------------
BOOL DecodeJsonMsg(CHAR *pJsonMsg, P2P_MSG_INFO_t *pJsonInfo);
//-------------------------------------------------------------------------------------------------
void GetP2pTimeStamp(CHAR *pTsBuff, UINT8 buffLen);
//-------------------------------------------------------------------------------------------------
BOOL OpenP2pSocket(BOOL isUdpSock, IP_ADDR_INFO_t *pLocalAddr, INT32 *pSockFd);
//-------------------------------------------------------------------------------------------------
BOOL ConnectP2pSocket(INT32 sockFd, IP_ADDR_INFO_t *pRemoteAddr);
//-------------------------------------------------------------------------------------------------
BOOL IsP2pConnectionEstablished(INT32 sockFd);
//-------------------------------------------------------------------------------------------------
BOOL SendP2pMsg(INT32 sockFd, CHAR *pMsgBuff, UINT32 msgLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
BOOL RecvP2pMsg(INT32 sockFd, CHAR *pMsgBuff, UINT32 buffSize, UINT32 *pMsgLen);
//-------------------------------------------------------------------------------------------------
INT32 GetAvailableBytesOnSocket(INT32 sockFd);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
