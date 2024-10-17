//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		CommonApi.c
@brief      File containing the defination of different common functions for all applications
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>

/* Application Includes */
#include "CommonApi.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Define boolen for all applications if not defined */
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create posix thread
 * @param   rThreadId - Pointer to thread ID
 * @param   rRoutine - Thread routine function
 * @param   rData - Data for thread routine
 * @param   threadType - Is thread detachable or joinable
 * @param   stackSize - stack size of thread (default stack size if value is 0)
 * @return  Return TRUE if thread is created successfully else return FALSE
 */
BOOL Utils_CreateThread(pthread_t *rThreadId, void *(*rRoutine)(void *), void *rData, BOOL threadType, UINT32 stackSizeBytes)
{
    BOOL            tRetSts = FALSE;
    INT32           threadState;
    pthread_attr_t  tAttr;
    pthread_t       threadId;
    INT32           errNo;


    /* Initialize thread attributes object */
    if (0 != (errNo = pthread_attr_init(&tAttr)))
    {
        EPRINT(UTILS, "pthread_attr_init failed: [err=%s]", strerror(errNo));
        return (FALSE);
    }

    do
    {
        /* Set thread detach state as per requirement */
        if (threadType == JOINABLE_THREAD)
        {
            /* Set thread joinable */
            threadState = PTHREAD_CREATE_JOINABLE;
        }
        else
        {
            /* Set thread detachable */
            threadState = PTHREAD_CREATE_DETACHED;
        }

        /* Set Detach State to Detached */
        if (0 != (errNo = pthread_attr_setdetachstate(&tAttr, threadState)))
        {
            EPRINT(UTILS, "pthread_attr_setdetachstate failed: [err=%s]", strerror(errNo));
            break;
        }

        /* set stack size if provided */
        if (0 != stackSizeBytes)
        {
            if (0 != (errNo = pthread_attr_setstacksize(&tAttr, stackSizeBytes)))
            {
                EPRINT(UTILS, "pthread_attr_setstacksize failed: [err=%s]", strerror(errNo));
                break;
            }
        }

        /* If thread id is not provided then use local one */
        if (NULL == rThreadId)
        {
            rThreadId = &threadId;
        }

        /* Create thread that will immediately start the routine */
        if (0 != (errNo = pthread_create(rThreadId, &tAttr, rRoutine, rData)))
        {
            EPRINT(UTILS, "pthread_create failed: [err=%s]", strerror(errNo));
            break;
        }

        /* Thread created successfully */
        tRetSts = TRUE;

    } while (0);

    /* Destroy thread attributes object */
    if (0 != (errNo = pthread_attr_destroy(&tAttr)))
    {
        EPRINT(UTILS, "pthread_attr_destroy failed: [err=%s]", strerror(errNo));
    }

    /* Return thread creation status */
    return (tRetSts);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides wrapper for system command. We have used popen to execute command from
 *          application. It will open pipe for executing command.
 * @param   rCmd - Pointer to system command
 * @return  Returns TRUE if command executed successfully else returns FALSE
 */
BOOL Utils_ExeCmd(const CHARPTR rCmd)
{
    INT32   status;
    FILE    *pPipeFd;

    /* Validate Pointers */
    if (NULL == rCmd)
    {
        /* Invalid Parameters */
        EPRINT(UTILS, "null cmd pointer for execution");
        return FALSE;
    }

    /* Open Pipe to system call */
    pPipeFd = popen(rCmd, "we");
    if(pPipeFd == NULL)
    {
        /* Failed to Open Pipe */
        EPRINT(UTILS, "fail to open pipe: [cmd=%s], [err=%s]", rCmd, strerror(errno));
        return FALSE;
    }

    /* Get and Check Status of Pipe Close */
    status = pclose(pPipeFd);
    if (-1 == status)
    {
        /* Pipe failed to close */
        EPRINT(UTILS, "fail to close pipe: [cmd=%s], [err=%s]", rCmd, strerror(errno));
        return FALSE;
    }

    /* Check child status */
    if ((1 != WIFEXITED(status)) || (0 != WEXITSTATUS(status)))
    {
        EPRINT(UTILS, "fail to execute cmd: [cmd=%s], [status=%d], [WEXITSTATUS=%d], [WIFEXITED=%d]", rCmd, status, WEXITSTATUS(status), WIFEXITED(status));
        return FALSE;
    }

    /* Command executed successfully */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides wrapper for write api. We have taken care of write to retry when
 *          a write is return with an error EINTR.
 * @param   iFd        - File descriptor
 * @param   iBuffer    - Buffer from which data is written
 * @param   iCount     - No. of bytes to be written
 * @param   pErrorCode - Error code will be written in case of write fails else 0 is return.
 * @return  Returns the number of bytes written in the file.
 */
ssize_t Utils_Write(INT32 iFd, const void *iBuffer, size_t iCount, UINT32PTR pErrorCode)
{
    ssize_t tStatus = 0;

    /* Validate Parameters */
    if((INVALID_FILE_FD == iFd) || (NULL == iBuffer))
    {
        /* Invalid Parameters */
        EPRINT(UTILS, "invld input param: [fd=%d], [buffer=%p]", iFd, iBuffer);
        return tStatus;
    }

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE)

    tStatus = write(iFd, iBuffer, iCount);

    /* Write failed */
    if(-1 == tStatus)
    {
        /* Write failed due to an interrupt then try again */
        if(EINTR == errno)
        {
            tStatus = 0;
        }
        else
        {
           SET_ERROR_NUMBER(pErrorCode, errno)
           return tStatus;
        }
    }

    /* failed to write the data in the first attempt then try again */
    if(tStatus != (ssize_t)iCount)
    {
        ssize_t tWriteCount = (iCount - tStatus);

        tStatus = write(iFd, ((const CHARPTR)iBuffer + tStatus), tWriteCount);

        if(-1 == tStatus)
        {
            SET_ERROR_NUMBER(pErrorCode, errno)
        }
        else if(tStatus != tWriteCount)
        {
            /* Update total write bytes */
            tStatus = (iCount - tWriteCount) + tStatus;
        }
        else
        {
            tStatus = iCount;
        }
    }

    return tStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides wrapper for pwrite api. We have taken care of pwrite to retry when
 *          a pwrite is return with an error EINTR.
 * @param   iFd     - File descriptor
 * @param   iBuffer - Buffer from which data is written
 * @param   iCount  - No. of bytes to be written
 * @param   iOffset - Offset
 * @param   pErrorCode - Error code will be set in case of pwrite fails.
 * @return  Returns the number of bytes written in the file.
 */
ssize_t Utils_pWrite(INT32 iFd, const void *iBuffer, size_t iCount, off_t iOffset,  UINT32PTR pErrorCode)
{
    ssize_t tStatus = 0;

    /* Validate Parameters */
    if((INVALID_FILE_FD == iFd) || (NULL == iBuffer))
    {
        /* Invalid Parameters */
        EPRINT(UTILS, "invld input param: [fd=%d], [buffer=%p]", iFd, iBuffer);
        return tStatus;
    }

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE)

    tStatus = pwrite(iFd, iBuffer, iCount, iOffset);

    /* Write failed */
    if(-1 == tStatus)
    {
        /* Write failed due to an interrupt then try again */
        if(EINTR == errno)
        {
            tStatus = 0;
        }
        else
        {
           SET_ERROR_NUMBER(pErrorCode, errno)
           EPRINT(UTILS, "fail to write: [err=%s]", strerror(errno));
           return tStatus;
        }
    }

    /* failed to write the data in the first attempt then try again */
    if(tStatus != (ssize_t)iCount)
    {
        ssize_t tWriteCount = (iCount - tStatus);

        tStatus = pwrite(iFd, ((const CHARPTR)iBuffer + tStatus), tWriteCount, (iOffset + tStatus));

        if(-1 == tStatus)
        {
            SET_ERROR_NUMBER(pErrorCode, errno)
        }
        else if(tStatus != tWriteCount)
        {
            tStatus = (iCount - tWriteCount) + tStatus;
        }
        else
        {
            tStatus = iCount;
        }
    }
    return tStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides wrapper for mkdir api.
 * @param   pPath - Path of directory which needs to be created
 * @param   iMode - Directory mode
 * @param   pErrroCode - Error code optional, only meaningful when return value is -1.
 * @return  Returns 0 on success else -1 is returned.
 */
INT32 Utils_Mkdir(const CHAR *pPath, mode_t iMode, UINT32PTR pErrorCode)
{
    INT32 tStatus = 0;

    /* Validate Parameters */
    if(NULL == pPath)
    {
       return tStatus;
    }

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE)

    tStatus = mkdir(pPath, iMode);

    if(-1 == tStatus)
    {
        SET_ERROR_NUMBER(pErrorCode, errno)
        EPRINT(UTILS, "fail to create dir: [dir=%s], [err=%s]", pPath, strerror(errno));
    }

    return tStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides wrapper for read api.
 * @param   iFd - File descriptor
 * @param   pBuffer - Buffer in which data is filled after reading
 * @param   iBytes - No. of bytes to read
 * @param   pErrroCode - Error code optional, only meaningful when return value is -1.
 * @return  Returns number of bytes read on success else -1 is returned.
 */
ssize_t Utils_Read(INT32 iFd, void *pBuffer, size_t iBytes, UINT32PTR pErrorCode)
{

    ssize_t tStatus = 0;

    if((INVALID_FILE_FD == iFd) || (NULL == pBuffer))
    {
        EPRINT(UTILS, "invld input param");
        return tStatus;
    }

    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE)

    tStatus = read(iFd, pBuffer, iBytes);

    if(-1 == tStatus)
    {
        SET_ERROR_NUMBER(pErrorCode, errno)
        EPRINT(UTILS, "fail to read: [err=%s]", strerror(errno));
    }
    else if(tStatus < (ssize_t)iBytes)
    {
        size_t tRemainingBytes = iBytes - tStatus;
        tStatus = read(iFd, (const CHARPTR)pBuffer + tStatus,  tRemainingBytes);

        if(-1 == tStatus)
        {
            SET_ERROR_NUMBER(pErrorCode, errno)

            EPRINT(UTILS, "fail to read: [err=%s]", strerror(errno));
        }
        else
        {
            /* Total read bytes */
            tStatus = iBytes - tRemainingBytes + tStatus;
        }
    }

    return tStatus;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
