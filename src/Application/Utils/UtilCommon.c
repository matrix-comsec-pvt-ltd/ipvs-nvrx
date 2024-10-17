//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		UtilCommon.c
@brief      File containing the defination of different common functions for all applications
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>

/* Application Includes */
#include "ConfigApi.h"
#include "DebugLog.h"
#include "UtilCommon.h"
#include "SysTimer.h"

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
//NOTE: To add new name in codec please add it at end with '\n' termination
static const CHARPTR audioCodec[MAX_AUDIO_CODEC] =
{
	"NONE\n",
	"PCMU\nG711U\nbasic\ng.711-64k\n",
	"G726-8\ng.726-8k\n8kadpcm\n",
	"G726-16\ng.726-16k\n16kadpcm\n",
	"G726-24\ng.726-24k\n24kadpcm\n",
	"G726-32\ng.726-32k\n32kadpcm\nG726-Mixed\n",
	"G726-40\ng.726-40k\n40kadpcm\n",
	"MPEG4-GENERIC\n",
	"L16\n",
    "PCM\n",
    "PCMA\nG711A\n",
};

//NOTE: To add new name in codec please add it at end with '\n' termination
static const CHARPTR videoCodec[MAX_VIDEO_CODEC] =
{
	"NONE\n",
	"JPEG\nMJPEG\nMotion JPEG\n",
	"H264\nH.264\n",
	"MPEG4\nMP4V-ES\nmpeg\n",
    "",
	"H265\nH.265\n"
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetAudioCodec
 * @param   codecString
 * @return  Returns audio codec type
 */
STREAM_CODEC_TYPE_e GetAudioCodec(CHARPTR codecString)
{
    STREAM_CODEC_TYPE_e codecType;
    UINT8               length;
    UINT8               codecLength = strlen(codecString);
    CHARPTR             codecStr, tempStr;

	for(codecType = AUDIO_CODEC_NONE; codecType < MAX_AUDIO_CODEC; codecType++)
	{
        /* Get the all possible codec name strings */
		tempStr = audioCodec[codecType];

        /* Get next codec name string */
        while ((codecStr = strchr(tempStr, '\n')) != NULL)
		{
            /* Get the length of the codec string */
            length = (codecStr - tempStr);

            /* We have to take larger length to avoid matching of PCM with PCMA/U and PCMA/U with PCM */
            if (strncasecmp(codecString, tempStr, codecLength > length ? codecLength : length) == 0)
			{
                /* Codec string matched */
                return codecType;
			}

            /* Check next string of same codec if available */
			tempStr += (length + 1);
        }
	}

    /* Codec string not matched */
    return MAX_AUDIO_CODEC;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetVideoCodec
 * @param   codecString
 * @return  Returns video codec number
 */
STREAM_CODEC_TYPE_e GetVideoCodec(CHARPTR codecString)
{
    STREAM_CODEC_TYPE_e codecType;
    UINT8               length;
    CHARPTR             codecStr, tempStr;

    for(codecType = VIDEO_CODEC_NONE; codecType < MAX_VIDEO_CODEC; codecType++)
	{
        /* Get the all possible codec name strings */
		tempStr = videoCodec[codecType];

        /* Get next codec name string */
        while ((codecStr = strchr(tempStr, '\n')) != NULL)
		{
            /* Get the length of the codec string */
			length = (codecStr - tempStr);

            /* Compare codec string */
			if (strncasecmp(codecString, tempStr, length) == 0)
			{
                /* Codec string matched */
                return codecType;
			}

            /* Check next string of same codec if available */
			tempStr += (length + 1);
		}
	}

    /* Codec string not matched */
    return MAX_VIDEO_CODEC;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function send frame to client
 * @param   connFd - Fd of client
 * @param   pSendBuff - Buffer which contains data
 * @param   buffLen - Length of data inside buffer
 * @param   timeoutSec - Timeout for select in us
 * @return  Returns -1 on failure else number data bytes sent on socket
 */
INT32 SendToClient(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeoutSec)
{
    INT32           pollSts;
    INT16           pollRevent;

    /* Validate socket fd */
    if (connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(UTILS, "Invld fd found");
        return -1;
    }

    /* Poll for write event on socket */
    pollSts = GetSocketPollEvent(connFd, (POLLWRNORM | POLLRDHUP), timeoutSec/1000, &pollRevent);

    /* Is error found on socket? */
    if (FAIL == pollSts)
    {
        /* Error found on socket */
        EPRINT(UTILS, "poll fail: [fd=%d], [err=%s]", connFd, STR_ERR);
        return -1;
    }

    /* Is timeout happens? */
    if (TIMEOUT == pollSts)
    {
        /* Timeout occurs */
        return 0;
    }

    /* Is remote connection closed event? */
    if ((pollRevent & POLLRDHUP) == POLLRDHUP)
    {
        EPRINT(UTILS, "remote connection closed: [fd=%d]", connFd);
        return -1;
    }

    /* Check if other than write event has occured */
    if ((pollRevent & POLLWRNORM) != POLLWRNORM)
    {
        EPRINT(UTILS, "invalid poll event: [0x%x], [fd=%d]", pollRevent, connFd);
        return -1;
    }

    /* Send data on socket and returns number data bytes sent on socket */
    return send(connFd, pSendBuff, buffLen, MSG_NOSIGNAL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function send message to client
 * @param   connFd - Fd of client
 * @param   pSendBuff - Buffer which contains data
 * @param   buffLen - Length of data inside buffer
 * @param   timeoutSec - Timeout for select
 * @return  Returns SUCCESS/FAIL
 * @note    Select API doesn't interrupt on remote connection closed. In this case, It interrupts
 *          only on timeout. Till that time, Select blocks application or theard. To overcome this,
 *          we have replaced select with poll.
 */
BOOL SendToSocket(INT32 connFd, UINT8 *pSendBuff, UINT32 buffLen, UINT32 timeoutSec)
{
    INT32 			sendCnt = 0;
    INT16           pollEvent = (POLLWRNORM | POLLRDHUP);
    INT16           recvEvent;
    UINT32 			totalSend = 0;
    UINT64          prevTimeMs = 0;
    UINT8           pollSts;

    /* Validate socket fd */
    if (connFd == INVALID_CONNECTION)
	{
        /* Invalid FD found */
        EPRINT(UTILS, "Invld fd found");
        return FAIL;
    }

    while(TRUE)
    {
        /* Poll for fd till event or timeout */
        pollSts = GetSocketPollEvent(connFd, pollEvent, GetRemainingPollTime(&prevTimeMs, timeoutSec*1000), &recvEvent);

        if (FAIL == pollSts)
        {
            EPRINT(UTILS, "socket event poll fail: [fd=%d]", connFd);
            break;
        }

        if (TIMEOUT == pollSts)
        {
            EPRINT(UTILS, "send timeout: [fd=%d], [dataLen=%d], [sendCnt=%d]", connFd, buffLen, sendCnt);
            break;
        }

        /* Is remote connection closed event? */
        if ((recvEvent & POLLRDHUP) == POLLRDHUP)
        {
            EPRINT(UTILS, "remote connection closed: [fd=%d]", connFd);
            break;
        }

        /* Is other than write event? */
        if ((recvEvent & POLLWRNORM) != POLLWRNORM)
        {
            EPRINT(UTILS, "invld event: [fd=%d], [revents=0x%x]", connFd, recvEvent);
            break;
        }

        /* Send data on socket */
        sendCnt = send(connFd, pSendBuff + totalSend, (buffLen - totalSend), MSG_NOSIGNAL);
        if (sendCnt > 0)
        {
            /* Calculate totol data sent */
            totalSend = totalSend + sendCnt;
            if (totalSend >= buffLen)
            {
                /* Required data sent */
                return SUCCESS;
            }
        }
        else
        {
            /* Is other than resource unavailable error? */
            if (!(errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS))
            {
                INT32 errcode = errno;
                struct sockaddr_in addr = {0};
                socklen_t addr_size = sizeof(addr);
                getpeername(connFd, (struct sockaddr_in*)&addr, &addr_size);
                EPRINT(UTILS, "send failed: [fd=%d], [err=%s], [addr=%s]", connFd, strerror(errcode), inet_ntoa(addr.sin_addr));
                return FAIL;
            }
        }
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function received data from TCP socket and gives into output parameter
 * @param   connFd
 * @param   rcvMsg
 * @param   rcvLen
 * @param   startOfData
 * @param   endOfData
 * @param   maxData
 * @param   timeoutSec
 * @return  Returns SUCCESS/FAIL/REFUSE
 */
UINT8 RecvMessage(INT32 connFd, CHARPTR rcvMsg, UINT32PTR rcvLen, UINT8 startOfData, UINT8 endOfData, UINT32 maxData, UINT32 timeoutSec)
{
    INT32			cnt = 0;
    INT16           pollEvent = (POLLRDNORM | POLLRDHUP);
    INT16           recvEvent;
	UINT32 			recvCntSeg = 0;
    UINT64          prevTimeMs = 0;
    UINT8           pollSts;
    UINT32          timeoutMs;

    if (connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(UTILS, "invld fd found");
        return REFUSE;
    }

    if (timeoutSec == 0)
    {
        /* if supplied timeout is 0 sec, then by default wait for 15 msec */
        timeoutMs = 15;
    }
    else
    {
        /* Convert seconds to milli seconds to maintain accuracy of timeout */
        timeoutMs = timeoutSec*1000;
    }

    /* Set receive length to zero */
    *rcvLen = 0;

    /* Receive entire message upto timeout or max retry */
    while(TRUE)
    {
        /* Poll for fd till event or timeout */
        pollSts = GetSocketPollEvent(connFd, pollEvent, GetRemainingPollTime(&prevTimeMs, timeoutMs), &recvEvent);

        if (FAIL == pollSts)
        {
            EPRINT(UTILS, "socket event poll fail: [fd=%d]", connFd);
            break;
        }

        if (TIMEOUT == pollSts)
        {
            EPRINT(UTILS, "recv timeout: [fd=%d], [cnt=%d]", connFd, cnt);
            break;
        }

        /* Is other than read normal event? */
        if ((recvEvent & POLLRDNORM) != POLLRDNORM)
        {
            /* Is remote connection closed event? */
            if ((recvEvent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(UTILS, "remote connection closed: [fd=%d]", connFd);
                return REFUSE;
            }
            else
            {
                EPRINT(UTILS, "invld event: [fd=%d], [revents=0x%x]", connFd, recvEvent);
                return FAIL;
            }
        }

        /* Receive data from socket */
        cnt = recv(connFd, (rcvMsg + recvCntSeg), (maxData - recvCntSeg), MSG_DONTWAIT);
        if(cnt > 0)
        {
            recvCntSeg += cnt;
            if(rcvMsg[0] == startOfData)
            {
                if(rcvMsg[recvCntSeg - 1] == endOfData)
                {
                    *rcvLen = recvCntSeg;
                    return SUCCESS;
                }
                else if (recvCntSeg >= maxData)
                {
                    EPRINT(UTILS, "EOM not found: [fd=%d], [recv_len=%d]", connFd, recvCntSeg);
                    return FAIL;
                }
            }
            else
            {
                EPRINT(UTILS, "invld SOM found: [SOM=%x] [fd=%d]", rcvMsg[0], connFd);
                return FAIL;
            }
        }
        else if (cnt == 0)
        {
            /* Error occurred on socket */
            EPRINT(UTILS, "no data recv due to connection close: [fd=%d]", connFd);
            return REFUSE;
        }
        else
        {
            /* Recv API interrupted by signal */
            if (errno == EINTR)
            {
                EPRINT(UTILS, "recv interrupted by siganl: [fd=%d]", connFd);
            }
            else if (errno == EBADF)
            {
                EPRINT(UTILS, "bad fd found: [fd=%d]", connFd);
                return REFUSE;
            }
            else if (errno != EWOULDBLOCK)
            {
                return FAIL;
            }
        }
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   RecvFrame
 * @param   connFd
 * @param   rcvMsg
 * @param   rcvLen
 * @param   maxData
 * @param   timeoutSec
 * @param   timeoutUs
 * @return  Returns SUCCESS/FAIL/REFUSE
 */
BOOL RecvFrame(INT32 connFd, CHARPTR rcvMsg, UINT32PTR rcvLen,UINT32 maxData, UINT32 timeoutSec,UINT32 timeoutUs)
{
	INT32			cnt;
    UINT8			pollSts;
    UINT64          prevTimeMs = 0;
    UINT64          timeoutMs;
    INT16           pollRevent;

    if(connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(UTILS, "invld fd found");
        return FAIL;
    }

    /* Calculate timeout in milli seconds */
    timeoutMs = ((UINT64)timeoutSec*1000) + ((UINT64)timeoutUs/1000);

    /* Set receive length to zero */
    *rcvLen = 0;

    /* Receive entire message upto timeout and max retry */
    while(TRUE)
    {
        /* Wait for data or timeout occur */
        pollSts = GetSocketPollEvent(connFd, (POLLRDNORM | POLLRDHUP), GetRemainingPollTime(&prevTimeMs, timeoutMs), &pollRevent);

        if (FAIL == pollSts)
        {
            /* poll failed */
            EPRINT(UTILS, "poll fail: [fd=%d], [err=%s]", connFd, strerror(errno));
            return FAIL;
        }

        /* No data received till timeout */
        if(TIMEOUT == pollSts)
        {
            /* Timeout occurred */
            return FAIL;
        }

        /* if other than read event */
        if ((pollRevent & POLLRDNORM) != POLLRDNORM)
        {
            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(UTILS, "remote connection closed: [fd=%d]", connFd);
                return REFUSE;
            }
            else
            {
                EPRINT(UTILS, "invalid poll event:[0x%x]", pollRevent);
                return FAIL;
            }
        }

        /* Receive the data from socket */
        cnt = recv(connFd, (rcvMsg + *rcvLen), (maxData - *rcvLen), MSG_DONTWAIT);
        if (cnt < 0)
        {
            /* Recv API interrupted by signal */
            if (errno == EINTR)
            {
                EPRINT(UTILS, "recv interrupted by siganl");
                continue;
            }

            /* Is no more data available on socket? */
            if (errno == EWOULDBLOCK)
            {
                continue;
            }

            /* fail to receive data due to error on socket */
            EPRINT(UTILS, "recv error: [fd=%d], [err=%s]", connFd, strerror(errno));
            return ((errno == EBADF) ? REFUSE : FAIL);
        }

        /* Is connection closed? */
        if (cnt == 0)
        {
            /* Error occurred on socket */
            EPRINT(UTILS, "remote connection closed: [fd=%d]", connFd);
            return REFUSE;
        }

        /* Data recieved */
        *rcvLen += cnt;
        if (maxData == *rcvLen)
        {
            /* Required data received */
            return SUCCESS;
        }
    }

    /* Fail to receive required data */
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Get socket polling events
 * @param connFd - Socket FD
 * @param pollEvent - Events to be monitor
 * @param timeoutMs - Timeout in milliseconds
 * @param recvEvent - Event received on socket
 * @return Returns status of poll - Success/Timeout/Failure
 */
UINT8 GetSocketPollEvent(INT32 connFd, INT16 pollEvent, INT32 timeoutMs, INT16 *recvEvent)
{
    INT32 			pollSts;
    struct pollfd   pollFd;

    /* Validate socket fd */
    if (connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(UTILS, "Invld fd found");
        return FAIL;
    }

    /* Add fd for polling with events */
    pollFd.fd = connFd;
    pollFd.events = pollEvent;
    pollFd.revents = 0;

    /* Poll for fd till event or timeout */
    pollSts = poll(&pollFd, 1, timeoutMs);
    if (pollSts == -1)
    {
        /* Poll failed */
        EPRINT(UTILS, "poll fail: [fd=%d], [err=%s]", connFd, strerror(errno));
        return FAIL;
    }
    else if (pollSts == 0)
    {
        /* Poll timeout */
        return TIMEOUT;
    }

    // Store receive events
    if (recvEvent != NULL)
    {
        *recvEvent = pollFd.revents;
    }

    // Event received on socket
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns remaining time for polling
 * @param   prevTimeMs - Last time stamp in milliseconds
 * @param   timeoutMs - Time out in milliseconds
 * @return  Returns remaining time for polling
 */
UINT64 GetRemainingPollTime(UINT64 *prevTimeMs, UINT64 timeoutMs)
{
    struct timespec     ts = {0, 0};
    UINT64              currentTimeMs;
    UINT64              elapsedTimeMs;

    if (timeoutMs == 0)
    {
        return 0;
    }

    /* Get current timestamp in milli sec */
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    currentTimeMs = (((UINT64)ts.tv_sec * 1000) + ts.tv_nsec / NANO_SEC_PER_MILLI_SEC);

    /* Set init time if remaining */
    if (*prevTimeMs == 0)
    {
        *prevTimeMs = currentTimeMs;
        return timeoutMs;
    }

    /* Calculate elapsed time */
    elapsedTimeMs = currentTimeMs - (*prevTimeMs);

    return (elapsedTimeMs >= timeoutMs) ? 0 : (timeoutMs - elapsedTimeMs);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function closing tcp socket connection
 * @param   connFd
 */
void CloseSocket(INT32PTR connFd)
{
    /* Nothing to do if socket is invalid */
    if (*connFd == INVALID_CONNECTION)
	{
        return;
    }

    /* Close the socket connection */
    shutdown(*connFd, SHUT_RDWR);
    close(*connFd);
    *connFd = INVALID_CONNECTION;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function closing file fd
 * @param   fileFd
 */
void CloseFileFd(INT32PTR fileFd)
{
    /* Nothing to do if fd is invalid */
    if (*fileFd == INVALID_FILE_FD)
    {
        return;
    }

    /* Close the file fd */
    close(*fileFd);
    *fileFd = INVALID_FILE_FD;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used check whether socket is closed from client side or not.
 * @param   sockFd
 * @return  Returns TRUE if socket alive else returns FALSE
 */
BOOL CheckSocketFdState(INT32 sockFd)
{
    struct pollfd   pollFd;
    struct timespec timeout;

    /* Set socket information to check connection status */
    pollFd.fd = sockFd;
    pollFd.events = POLLRDHUP | POLLERR;
    pollFd.revents = 0;

    /* Set timeout of poll */
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10;

    /* Provide fd info for polling */
    if (ppoll(&pollFd, 1, &timeout, NULL) > 0)
    {
        /* Check connection status */
        if (pollFd.revents & POLLRDHUP)
        {
            /* Socket connection closed */
            return FALSE;
        }
    }

    /* Socket is alive */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Utils_OpenSharedMemory
 * @param   fileName
 * @param   sizeInBytes
 * @param   shmFd
 * @param   shmBaseAddr
 * @param   readOnly
 * @return
 */
BOOL Utils_OpenSharedMemory(CHARPTR fileName, size_t sizeInBytes, INT32PTR shmFd, UINT8PTR *shmBaseAddr, BOOL readOnly)
{
    INT32 shmFlags;
    INT32 mmapProtection;

    if ((NULL == fileName) || (0 == sizeInBytes) || (NULL == shmFd) || (NULL == shmBaseAddr))
    {
        return FALSE;
    }

    *shmFd = INVALID_FILE_FD;
    *shmBaseAddr = NULL;

    /* modify flags based on type */
    if (TRUE == readOnly)
    {
        shmFlags = READ_ONLY_MODE;
        mmapProtection = PROT_READ;
    }
    else
    {
        shmFlags = CREATE_RDWR_MODE;
        mmapProtection = PROT_READ | PROT_WRITE;
    }

    /* open the shared memory segment */
    *shmFd = shm_open(fileName, shmFlags, (S_IRUSR | S_IWUSR) | (S_IRGRP) | (S_IROTH));
    if (INVALID_FILE_FD == *shmFd)
    {
        EPRINT(UTILS, "fail to open shared memory: [file=%s], [err=%s]", fileName, strerror(errno));
        return FALSE;
    }

    /* resize shared memory file */
    if (FALSE == readOnly)
    {
        if (-1 == ftruncate(*shmFd, sizeInBytes))
        {
            EPRINT(UTILS, "fail to truncate shared memory: [file=%s], [err=%s]", fileName, strerror(errno));

            /* close the shared memory segment file */
            if (close(*shmFd) == -1)
            {
                EPRINT(UTILS, "fail to close shared memory: [file=%s], [err=%s]", fileName, strerror(errno));
            }

            *shmFd = INVALID_FILE_FD;

            return FALSE;
        }
    }

    /* map the shared memory segment to the address space of the process */
    *shmBaseAddr = (UINT8PTR)mmap(0, sizeInBytes, mmapProtection, MAP_SHARED, *shmFd, 0);

    if (MAP_FAILED == *shmBaseAddr)
    {
        EPRINT(UTILS, "fail to map shared memory: [file=%s], [err=%s]", fileName, strerror(errno));

        *shmBaseAddr = NULL;

        /* close the shared memory segment file */
        if (close(*shmFd) == -1)
        {
            EPRINT(UTILS, "fail to close shared memory: [file=%s], [err=%s]", fileName, strerror(errno));
        }

        *shmFd = INVALID_FILE_FD;

        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Utils_DestroySharedMemory
 * @param   fileName
 * @param   sizeInBytes
 * @param   shmFd
 * @param   shmBaseAddr
 */
void Utils_DestroySharedMemory(CHARPTR fileName, size_t sizeInBytes, INT32PTR shmFd, UINT8PTR *shmBaseAddr)
{
    if ((NULL == fileName) || (0 == sizeInBytes) || (NULL == shmFd) || (NULL == shmBaseAddr))
    {
        return;
    }

    /* unmap memory first */
    if (-1 == munmap(*shmBaseAddr, sizeInBytes))
    {
        EPRINT(UTILS, "fail to unmap shared memory: [file=%s], [err=%s]", fileName, strerror(errno));
    }

    /* close shared memory file */
    if (-1 == close(*shmFd))
    {
        EPRINT(UTILS, "fail to close shared memory: [file=%s], [err=%s]", fileName, strerror(errno));
    }

    /* remove shared memory file */
    shm_unlink(fileName);

    /* reset variables */
    *shmFd = -1;
    *shmBaseAddr = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   RecvUnixSockSeqPkt
 * @param   connFd
 * @param   pData
 * @param   dataLenMax
 * @param   timeoutSec
 * @param   pReadBytes
 * @return
 */
UINT8 RecvUnixSockSeqPkt(INT32 connFd, void *pData, UINT32 dataLenMax, UINT16 timeoutSec, UINT32 *pReadBytes)
{
    INT32   retVal;
    UINT8   pollSts;
    INT16   pollRevent;

    if (connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(UTILS, "invld fd found");
        return FAIL;
    }

    /* Wait for data or timeout occur */
    pollSts = GetSocketPollEvent(connFd, (POLLRDNORM | POLLRDHUP), timeoutSec*1000, &pollRevent);

    /* Poll failed */
    if (FAIL == pollSts)
    {
        EPRINT(UTILS, "poll failed: [fd=%d], [err=%s]", connFd, strerror(errno));
        return FAIL;
    }

    /* Timeout occurred */
    if (TIMEOUT == pollSts)
    {
        return FAIL;
    }

    /* if other than read event */
    if ((pollRevent & POLLRDNORM) != POLLRDNORM)
    {
        if ((pollRevent & POLLRDHUP) == POLLRDHUP)
        {
            EPRINT(UTILS, "remote connection closed: [fd=%d]", connFd);
            return REFUSE;
        }
        else
        {
            EPRINT(UTILS, "invalid poll event: [0x%x]", pollRevent);
            return FAIL;
        }
    }

    retVal = recv(connFd, pData, dataLenMax, MSG_DONTWAIT);
    if (retVal > 0)
    {
        *pReadBytes = retVal;
        return SUCCESS;
    }

    if (0 == retVal)
    {
        /* connection is closed */
        return REFUSE;
    }

    switch(errno)
    {
        case EINTR:
        {
            /* The recv() function was interrupted by a signal that was caught, before any data was available.*/
            retVal = recv(connFd, pData, dataLenMax, MSG_DONTWAIT);
            if (retVal > 0)
            {
                *pReadBytes = retVal;
                return SUCCESS;
            }

            if (0 == retVal)
            {
                /* connection is closed */
                return REFUSE;
            }
        }
        break;

        case ECONNRESET:
        case ENOTCONN:
        case EBADF:
        {
            /* connection is closed */
            EPRINT(UTILS, "connection is closed with server: [fd=%d], [err=%s]", connFd, strerror(errno));
            return REFUSE;
        }
        break;

        case EAGAIN:
        #if (EWOULDBLOCK != EAGAIN)	/* If we don't keep this an error will be raised at compile time. */
        case EWOULDBLOCK:
        #endif
        {
           EPRINT(UTILS, "no data available on socket: [fd=%d], [err=%s]", connFd, strerror(errno));
        }
        break;

        default:
        {
            EPRINT(UTILS, "data recv failed with unknown error: [fd=%d], [err=%s]", connFd, strerror(errno));
        }
        break;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set common socket options on fd
 * @param   sockFd
 * @return  Returns TRUE on success else returns FALSE
 */
BOOL SetSockFdOption(INT32 sockFd)
{
    INT32 sockOpts;

    /* Get Socket Flag Options */
    sockOpts = fcntl(sockFd, F_GETFL, 0);
    if (sockOpts < 0)
    {
        EPRINT(UTILS, "fail to get socket flag options: [fd=%d], [err=%s]\n", sockFd, strerror(errno));
        return FALSE;
    }

    /* Set socket as non blocking if not */
    if ((sockOpts & O_NONBLOCK) == 0)
    {
        sockOpts = (sockOpts | O_NONBLOCK);
        if (fcntl(sockFd, F_SETFL, sockOpts) < 0)
        {
            EPRINT(UTILS, "fail to set socket flag options: [fd=%d], [err=%s]\n", sockFd, strerror(errno));
            return FALSE;
        }
    }

    /* Get Socket FD Options */
    sockOpts = fcntl(sockFd, F_GETFD, 0);
    if (sockOpts < 0)
    {
        EPRINT(UTILS, "fail to get socket fd options: [fd=%d], [err=%s]\n", sockFd, strerror(errno));
        return FALSE;
    }

    /* Set Socket FD Options */
    if ((sockOpts & FD_CLOEXEC) == 0)
    {
        sockOpts |= FD_CLOEXEC;
        if(fcntl(sockFd, F_SETFD, sockOpts) <  0)
        {
            EPRINT(UTILS, "fail to set socket fd options: [fd=%d], [err=%s]\n", sockFd, strerror(errno));
            return FALSE;
        }
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Format Ip Address string based on family for URLs
 * @param   ipAddr
 * @param   ipAddressForUrl
 */
void PrepareIpAddressForUrl(const CHAR *ipAddr, CHAR *ipAddrForUrl)
{
    struct in6_addr ipv6Addr;

    if (1 == inet_pton(AF_INET6, ipAddr, &ipv6Addr))
    {
        snprintf(ipAddrForUrl, DOMAIN_NAME_SIZE_MAX, "[%s]", ipAddr);
    }
    else
    {
        snprintf(ipAddrForUrl, DOMAIN_NAME_SIZE_MAX, "%s", ipAddr);
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
