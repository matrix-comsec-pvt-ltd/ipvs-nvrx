//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Utils.h
@brief      File containing the prototype of different utils functions for all modules
*/

//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

/* Application Includes */
#include "Utils.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define PROC_PATH   "/proc/"

//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//*****************************************************************************
//GetPidOfProcess
//Param
//		Input:		CHARPTR processName
//		Output:		NONE
//		Return:		INT32
//
//Description
//		This function returns the pid number of given process. If process is not
//		exists then it returns NILL
//*****************************************************************************
INT32 GetPidOfProcess(CHARPTR processName)
{
    INT32			ret, loop;
    INT32			pid = NILL;
    INT64			tempPid;
    DIR				* dirPtr = NULL;
    FILE			* filePtr = NULL;
    CHAR			filePath[FILENAME_MAX];
    CHAR			cmdlineBuf[FILENAME_MAX];
    CHARPTR 		procStr = NULL;
    struct	dirent	* dirEntry = NULL;
    struct	stat	fileStat;
    UINT16          outLen = 0;

    dirPtr = opendir(PROC_PATH);
    if(dirPtr == NULL)
    {
        return NILL;
    }

    while ((dirEntry = readdir(dirPtr)) != NULL)
    {
        /* Folder must have numeric number to proceed further */
        tempPid = atol(dirEntry->d_name);
        if (tempPid <= 0)
        {
            continue;
        }

        outLen = snprintf(filePath, FILENAME_MAX, "%s%s", PROC_PATH, dirEntry->d_name);
        if(outLen > FILENAME_MAX)
        {
            EPRINT(UTILS, "Length is greater than buffer : %d", outLen);
            outLen = FILENAME_MAX;
        }

        memset(&fileStat, 0, sizeof(struct stat));
        if(stat(filePath, &fileStat) < STATUS_OK)
        {
            continue;
        }

        if(FAIL == S_ISDIR(fileStat.st_mode))
        {
            continue;
        }

        snprintf(filePath + outLen, FILENAME_MAX - outLen, "/cmdline");
        filePtr = fopen(filePath, "r");
        if(filePtr == NULL)
        {
            continue;
        }

        memset(cmdlineBuf, 0, sizeof(cmdlineBuf));
        ret = fread(cmdlineBuf, sizeof(CHAR), (FILENAME_MAX - 1), filePtr);
        fclose(filePtr);

        for(loop = 0; (ret > 0) && (loop < (ret - 1)); loop++)
        {
            if(cmdlineBuf[loop] == '\0')
            {
                cmdlineBuf[loop] = ' ';
            }
        }

        procStr = strstr(cmdlineBuf, processName);
        if(procStr == NULL)
        {
            continue;
        }

        if (strncmp(procStr, processName, strlen(processName)) != STATUS_OK)
        {
            continue;
        }

        pid = (INT32)tempPid;
        DPRINT(UTILS, "Process [%s] ID [%d]", processName, pid);
        break;
    }

    closedir(dirPtr);
    return pid;
}

//*****************************************************************************
//KillProcess
//Param
//		Input:		CHARPTR processName, signal type
//		Output:		NONE
//		Return:		NONE
//
//Description
//		This function kills the given process.
//*****************************************************************************
void KillProcess(CHARPTR processName, SEND_PROC_SIG_e sigType)
{
    DPRINT(UTILS, "Kill Process [%s]", processName);
    KillProcessId(GetPidOfProcess(processName), sigType);
}

//*****************************************************************************
//KillProcessId
//Param
//		Input:		UINT32 	processId, signal type
//		Output:		NONE
//		Return:		NONE
//
//Description
//		This function kills the given process from id.
//*****************************************************************************
BOOL KillProcessId(INT32 processId, SEND_PROC_SIG_e sigType)
{
    INT32 signalType;

    if (processId == NILL)
    {
        return FAIL;
    }

    switch(sigType)
    {
        case INT_SIG:
            signalType = SIGINT;
            break;

        case KILL_SIG:
            signalType = SIGKILL;
            break;

        case SIG_USR:
            signalType = SIGUSR2;
            break;

        default:
            return FAIL;
    }

    if(kill(processId, signalType) != STATUS_OK)
    {
        return FAIL;
    }

    DPRINT(UTILS, "Kill Process ID [%d]", processId);
    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
