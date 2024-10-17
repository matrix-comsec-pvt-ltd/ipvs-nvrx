//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pInterface.c
@brief      File containing the defination of interface level functions for other module who wants
            access P2P module service
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "P2pInterface.h"
#include "P2pClientComm.h"
#include "DebugLog.h"
#include "SysTimer.h"
#include "CommonApi.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Delay time to start P2P process on bootup. Provide some time to system */
#define P2P_START_ON_BOOT_TMR           15  /* Seconds */

/* Timer to start P2P on config change */
#define P2P_START_ON_CNFG_CHANGE_TMR    2   /* Seconds */

/* P2P process thread stack size */
#define P2P_PROC_THREAD_STACK_SIZE      (1 * MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/* It stores information related to P2P interface */
typedef struct
{
    BOOL            isThreadActive;
    TIMER_HANDLE    timerHandle;
    pthread_t       threadId;
    pthread_mutex_t lock;

}P2P_INTERFACE_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static P2P_INTERFACE_INFO_t p2pIfaceInfo =
{
    .isThreadActive = FALSE,
    .timerHandle = INVALID_TIMER_HANDLE,
    .threadId = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL startP2pProcessTimer(UINT8 secCnt);
//-------------------------------------------------------------------------------------------------
static void stopP2pProcessTimer(void);
//-------------------------------------------------------------------------------------------------
static void startP2pProcess(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void stopP2pProcess(void);
//-------------------------------------------------------------------------------------------------
static void restartP2pProcess(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of P2P module
 */
void InitP2pModule(void)
{
    P2P_CONFIG_t p2pConfig;

    /* Init P2P process info */
    InitP2pProcInfo();

    /* Get P2P configuration */
    ReadP2PConfig(&p2pConfig);

    /* Check P2P is enabled or not */
    if (DISABLE == p2pConfig.p2pEnable)
    {
        /* P2P is disabled */
        return;
    }

    /* P2P is enabled. Hence start P2P process */
    if (FAIL == startP2pProcessTimer(P2P_START_ON_BOOT_TMR))
    {
        EPRINT(P2P_MODULE, "failed to start p2p process");
        return;
    }

    /* P2P process started */
    DPRINT(P2P_MODULE, "p2p process started on bootup..!!");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Exit P2P module
 */
void DeInitP2pModule(void)
{
    /* Stop all P2P related process */
    stopP2pProcess();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P configuration change notification
 * @param   newP2pConfig - New configuration copy
 * @param   oldP2pConfig - Old configuration copy
 */
void P2pConfigNotify(P2P_CONFIG_t newP2PConfig, P2P_CONFIG_t *oldP2PConfig)
{
    if (newP2PConfig.p2pEnable == oldP2PConfig->p2pEnable)
    {
        /* No change in configuration */
        return;
    }

    /* If P2P config changed to enabled */
    if (ENABLE == newP2PConfig.p2pEnable)
    {
        /* Start P2P process */
        startP2pProcessTimer(P2P_START_ON_CNFG_CHANGE_TMR);
    }
    else
    {
        /* Stop P2P process timer */
        stopP2pProcessTimer();

        /* Terminate P2P process */
        stopP2pProcess();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P status
 * @return  Returns TRUE if status connected otherwise FALSE
 */
BOOL GetP2pStatus(void)
{
    P2P_CONFIG_t p2pConfig;

    /* Get P2P config and if it is disabled then send status disabled */
    ReadP2PConfig(&p2pConfig);
    if (DISABLE == p2pConfig.p2pEnable)
    {
        /* P2P is disabled */
        return FALSE;
    }

    /* Get P2P process status and if p2p it is connnected then return TRUE, FALSE otherwise */
    return (GetP2pProcessStatus() == P2P_STATUS_CONNECTED) ? TRUE : FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update P2P process on LAN config change if applicable
 * @param   lanNo
 */
void P2pLanConfigUpdate(LAN_CONFIG_ID_e lanNo)
{
    /* Check P2P process is running or not */
    if (FALSE == GetP2pProcessThreadRunStatus())
    {
        /* P2P is not running */
        return;
    }

    if (lanNo == LAN1_PORT)
    {
        /* Check P2P is running on LAN1 */
        if (GetP2pNetworkPort() == NETWORK_PORT_LAN1)
        {
            /* Restart P2P as LAN1 config changed */
            DPRINT(P2P_MODULE, "restart p2p due to lan1 config change");
            restartP2pProcess();
        }
    }
    else if (lanNo == LAN2_PORT)
    {
        /* Check P2P is running on LAN2 */
        if (GetP2pNetworkPort() == NETWORK_PORT_LAN2)
        {
            /* Restart P2P as LAN2 config changed */
            DPRINT(P2P_MODULE, "restart p2p due to lan2 config change");
            restartP2pProcess();
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update P2P process on mobile broadband config change if applicable
 */
void P2pMobileBroadbandCfgUpdate(void)
{
    /* Check P2P process is running or not */
    if (FALSE == GetP2pProcessThreadRunStatus())
    {
        /* P2P is not running */
        return;
    }

    /* Check P2P is running on Mobile Broadband */
    if (GetP2pNetworkPort() == NETWORK_PORT_USB_MODEM)
    {
        /* Restart P2P as mobile broadband config changed */
        DPRINT(P2P_MODULE, "restart p2p due to mobile broadband config change");
        restartP2pProcess();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update P2P process on default route config change if applicable
 * @param   newCopy
 * @param   oldCopy
 */
void P2pUpdateDefaultRoute(STATIC_ROUTING_CONFIG_t newCopy, STATIC_ROUTING_CONFIG_t *oldCopy)
{
    /* Check P2P process is running or not */
    if (FALSE == GetP2pProcessThreadRunStatus())
    {
        /* P2P is not running */
        return;
    }

    /* Check default port is changed or not */
    if (newCopy.defaultPort != oldCopy->defaultPort)
    {
        /* Restart P2P as dafault route config changed */
        DPRINT(P2P_MODULE, "restart p2p due to dafault route config change");
        restartP2pProcess();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Change in data-time configuration
 * @param   newCopy
 * @param   oldCopy
 */
void P2pDateTimeCfgUpdate(DATE_TIME_CONFIG_t newCopy, DATE_TIME_CONFIG_t *oldCopy)
{
    /* Check P2P process is running or not */
    if (FALSE == GetP2pProcessThreadRunStatus())
    {
        /* P2P is not running */
        return;
    }

    /* Check timezone is changed or not */
    if (newCopy.timezone != oldCopy->timezone)
    {
        /* Restart P2P as timezone config changed */
        DPRINT(P2P_MODULE, "restart p2p due to timezone config change");
        restartP2pProcess();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send client command response data (Equivalent to SendToSocket)
 * @param   localMsgUid
 * @param   pSendMsg
 * @param   sendMsgLen
 * @param   timeout
 * @return  TRUE on success; FALSE otherwise
 */
BOOL P2pCmdSendCallback(INT32 connFd, UINT8 *pSendBuff, UINT32 sendLen, UINT32 timeout)
{
    /* Provide to P2P communication thread */
    return SendP2pClientControlData(connFd, pSendBuff, sendLen, timeout);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send client message data (Equivalent to SendToSocket)
 * @param   localMsgUid
 * @param   pSendMsg
 * @param   sendMsgLen
 * @param   timeout
 * @return  TRUE on success; FALSE otherwise
 */
BOOL P2pDataSendCallback(INT32 connFd, UINT8 *pSendBuff, UINT32 sendLen, UINT32 timeout)
{
    /* Provide to P2P client communication module */
    return SendP2pClientFrameData(connFd, pSendBuff, sendLen, timeout);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P Client Message Close Callback (Equivalent to CloseSocket)
 * @param   pConnFd
 */
void P2pCloseConnCallback(INT32 *pConnFd)
{
    /* Provide to P2P client communication module */
    CloseP2pClientConn(pConnFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P client data tranfer fd.
 * @param   fdType
 * @param   localMsgUid
 * @param   pDataRecvFd
 * @return  TRUE on success; FALSE otherwise
 */
BOOL GetP2pDataXferFd(P2P_CLIENT_FD_TYPE_e fdType, INT32 localMsgUid, INT32 *pDataRecvFd)
{
    /* Provide to P2P client communication module */
    return GetP2pClientDataXferFd(fdType, localMsgUid, pDataRecvFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Close P2P data transfer fd.
 * @param   fdType
 */
void CloseP2pDataXferFd(P2P_CLIENT_FD_TYPE_e fdType)
{
    /* Provide to P2P client communication module */
    CloseP2pClientDataXferFd(fdType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P process timer and on timeout start P2P related work
 * @param   secCnt - Delay in seconds
 */
static BOOL startP2pProcessTimer(UINT8 secCnt)
{
#if !defined(OEM_JCI)
    TIMER_INFO_t timerInfo;

    timerInfo.data = 0;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(secCnt);
    timerInfo.funcPtr = startP2pProcess;

    if (p2pIfaceInfo.timerHandle == INVALID_TIMER_HANDLE)
    {
        /* Start a new timer */
        return StartTimer(timerInfo, &p2pIfaceInfo.timerHandle);
    }
    else
    {
        /* Timer already running. Hence reload it */
        return ReloadTimer(p2pIfaceInfo.timerHandle, timerInfo.count);
    }
#else
    return FAIL;
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P process timer
 */
static void stopP2pProcessTimer(void)
{
    /* Delete the timer if running */
    DeleteTimer(&p2pIfaceInfo.timerHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P related process
 * @param   data - User Data
 */
static void startP2pProcess(UINT32 data)
{
    /* Delete P2P process timer */
    stopP2pProcessTimer();

    /* Lock P2P process */
    MUTEX_LOCK(p2pIfaceInfo.lock);

    /* Check P2P process is runing or not */
    if (TRUE == p2pIfaceInfo.isThreadActive)
    {
        /* Unlock P2P process */
        MUTEX_UNLOCK(p2pIfaceInfo.lock);

        /* Restart P2P process */
        stopP2pProcess();

        /* Lock P2P process */
        MUTEX_LOCK(p2pIfaceInfo.lock);
    }

    /* Start P2P process thread */
    SetP2pProcessThreadRunStatus(TRUE);
    if (FALSE == Utils_CreateThread(&p2pIfaceInfo.threadId, P2pProcessThreadRoutine, NULL, JOINABLE_THREAD, P2P_PROC_THREAD_STACK_SIZE))
    {
        /* Failed to create thread */
        EPRINT(P2P_MODULE, "failed to create p2p process thread");
        SetP2pProcessThreadRunStatus(FALSE);
    }
    else
    {
        /* P2P process thread started */
        DPRINT(P2P_MODULE, "p2p process thread started..!!");
        p2pIfaceInfo.isThreadActive = TRUE;
    }

    /* Unlock P2P process */
    MUTEX_UNLOCK(p2pIfaceInfo.lock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P related process
 */
static void stopP2pProcess(void)
{
    /* Lock P2P process */
    MUTEX_LOCK(p2pIfaceInfo.lock);

    /* If thread active then stop it */
    if (TRUE == p2pIfaceInfo.isThreadActive)
    {
        /* Stop P2P process thread */
        SetP2pProcessThreadRunStatus(FALSE);

        #if !defined(OEM_JCI)
        /* Wait to thread exit */
        pthread_join(p2pIfaceInfo.threadId, NULL);
        #endif
        p2pIfaceInfo.isThreadActive = FALSE;
    }

    /* Unlock P2P process */
    MUTEX_UNLOCK(p2pIfaceInfo.lock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Restart P2P process
 */
static void restartP2pProcess(void)
{
    /* Stop P2P process timer */
    stopP2pProcessTimer();

    /* Terminate P2P process */
    stopP2pProcess();

    /* Start P2P process */
    startP2pProcessTimer(P2P_START_ON_CNFG_CHANGE_TMR);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
