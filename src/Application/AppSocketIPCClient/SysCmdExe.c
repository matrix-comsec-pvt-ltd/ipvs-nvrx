//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SysCmdExe.c
@brief      This file provides the facility of system command execution to reduce the processing
            load of main application.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Simultaneous max system command execution */
#define SYS_CMD_PERFORM_CNT_MAX     5
#define SYS_CMD_THREAD_STACK_SZ     (200 * KILO_BYTE)

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static INT32                sysCmdRxMsgQId = INVALID_FILE_FD;
static INT32                sysCmdTxMsgQId = INVALID_FILE_FD;
static SYS_CMD_MSG_INFO_t   sysCmdMsgInfo[SYS_CMD_PERFORM_CNT_MAX];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR performSysCmd(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void sendSysCmdExeResp(long sysCmdId, BOOL status);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Responsible for system command execution
 * @param   argc
 * @param   argv
 * @return
 */
INT32 main(void)
{
    UINT8               sysCmdCnt = 0;
    SYS_CMD_MSG_INFO_t 	sysCmdMsg;

    /* Init debug module */
    InitDebugLog();

    /* Create the message queue for system command execution */
    sysCmdRxMsgQId = msgget(SYS_CMD_EXE_REQ_IPC_MSG_ID, 0666);
    if (sysCmdRxMsgQId == INVALID_FILE_FD)
    {
        /* Fail to create system command ipc message queue */
        EPRINT(SYS_LOG, "fail to get system cmd rx msg queue id: [err=%s]", STR_ERR);
        return -1;
    }

    /* Create the message queue for system command execution */
    sysCmdTxMsgQId = msgget(SYS_CMD_EXE_RESP_IPC_MSG_ID, 0666);
    if (sysCmdTxMsgQId == INVALID_FILE_FD)
    {
        /* Fail to create system command ipc message queue */
        EPRINT(SYS_LOG, "fail to get system cmd tx msg queue id: [err=%s]", STR_ERR);
        return -1;
    }

    /* System command exe ipc message queue created successfully */
    DPRINT(SYS_LOG, "system cmd msg queue created successfully");

    while(TRUE)
    {
        /* Blocking call to get message from queue */
        if ((msgrcv(sysCmdRxMsgQId, (void*)&sysCmdMsg, SYS_CMD_MSG_LEN_MAX, 0, 0)) == -1)
        {
            /* Fail receive message from nvr app */
            EPRINT(SYS_LOG, "fail to recv msg from nvr app: [err=%s]", STR_ERR);
            usleep(100000);
            continue;
        }

        /* Check command type. If it special control command then take necessary action directly */
        if (sysCmdMsg.cmdId == SYS_EXE_CTRL_CMD_ID_DEBUG_CNFG)
        {
            /* Read debug configuration */
            ReadDebugConfigFile();
            sendSysCmdExeResp(sysCmdMsg.cmdId, SUCCESS);
            DPRINT(SYS_LOG, "debug config updated successfully");
            continue;
        }

        /* Create the thread to execute system command */
        sysCmdMsgInfo[sysCmdCnt] = sysCmdMsg;
        if (FAIL == Utils_CreateThread(NULL, performSysCmd, &sysCmdMsgInfo[sysCmdCnt], DETACHED_THREAD, SYS_CMD_THREAD_STACK_SZ))
        {
            EPRINT(SYS_LOG, "fail to create the sys cmd thread: [sysCmdCnt=%d], [cmdId=%ld], [cmd=%s]", sysCmdCnt, sysCmdMsg.cmdId, sysCmdMsg.cmdData);
            sendSysCmdExeResp(sysCmdMsg.cmdId, FAIL);
            continue;
        }

        /* It is just store system command in global variable for thread */
        sysCmdCnt++;
        if (sysCmdCnt >= SYS_CMD_PERFORM_CNT_MAX)
        {
            /* Rollover the system command info counter */
            sysCmdCnt = 0;
        }
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Execute system command
 * @param   threadArg
 * @return
 */
static VOIDPTR performSysCmd(VOIDPTR threadArg)
{
    SYS_CMD_MSG_INFO_t  sysCmdMsg;
    static UINT16       threadCnt = 0;

    /* Get the global copy of msg info to local, after that global copy can be used for other message */
    memcpy(&sysCmdMsg, (SYS_CMD_MSG_INFO_t *)threadArg, sizeof(SYS_CMD_MSG_INFO_t));

    /* Derive thread counter for identification */
    threadCnt++;
    if (threadCnt == 0)
    {
        /* It is just sync with cmdId for troubleshooting */
        threadCnt++;
    }

    /* Set thread name */
    THREAD_START_INDEX2("SYS_CMD", threadCnt);

    /* Execute system command */
    sendSysCmdExeResp(sysCmdMsg.cmdId, Utils_ExeCmd(sysCmdMsg.cmdData));
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send system command execution response
 * @param   sysCmdId
 * @param   status
 */
static void sendSysCmdExeResp(long sysCmdId, BOOL status)
{
    UINT8               dataCnt = 0;
    SYS_CMD_MSG_INFO_t  sysCmdMsg;

    /* Prepare response message */
    sysCmdMsg.cmdId = sysCmdId;
    sysCmdMsg.cmdData[dataCnt++] = status;

    /* Send response to nvr application */
    if ((msgsnd(sysCmdTxMsgQId, (void*)&sysCmdMsg, dataCnt, IPC_NOWAIT)) == -1)
    {
        EPRINT(SYS_LOG, "fail to send msg to nvr app: [cmdId=%ld], [err=%s]", sysCmdId, STR_ERR);
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
