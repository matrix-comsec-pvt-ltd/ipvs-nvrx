#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include "UiLogServer.h"
#include "DebugLog.h"
#include "CommonApi.h"

#define INTERNAL_SOCKET_FILE 		"/tmp/IntUiSocket"
#define RECV_TIMEOUT_MAX_IN_SEC     1

/* Nano seconds in 1 millisec */
#define NANO_SEC_PER_MILLI_SEC      1000000LL

static pthread_t    setLogUiThreadId;
static BOOL         uiLogServerRunF = TRUE;
static INT32        serverSockFd = INVALID_CONNECTION;

static void *setLogUiThread(void *threadArg);

void InitUiLogServer(void)
{
    INT32               yes = SUCCESS;
    struct sockaddr_un  sockAddr;

    serverSockFd = socket(AF_UNIX, (SOCK_STREAM | SOCK_CLOEXEC), 0);
    if(serverSockFd == INVALID_CONNECTION)
    {
        EPRINT(GUI_SYS, "fail to create log server socket: [err=%s]", strerror(errno));
        return;
    }

    unlink(INTERNAL_SOCKET_FILE);
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sun_family = AF_UNIX;
    snprintf(sockAddr.sun_path, sizeof(sockAddr.sun_path), INTERNAL_SOCKET_FILE);

    setsockopt(serverSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(bind(serverSockFd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < STATUS_OK)
    {
        EPRINT(GUI_SYS, "fail to bind log server socket: [err=%s]", strerror(errno));
        CloseSocket(&serverSockFd);
        return;
    }

    if(listen(serverSockFd, 5) < STATUS_OK)
    {
        EPRINT(GUI_SYS, "fail to listen log server socket: [err=%s]", strerror(errno));
        CloseSocket(&serverSockFd);
        return;
    }

    /* Changing permision of socket file */
    chmod(INTERNAL_SOCKET_FILE, 0777);

    /* Create thread receive message from internal AF_UNIX socket */
    if (FALSE == Utils_CreateThread(&setLogUiThreadId, setLogUiThread, NULL, JOINABLE_THREAD, (1 * MEGA_BYTE)))
    {
        EPRINT(GUI_SYS, "fail to create log server thread");
        CloseSocket(&serverSockFd);
        return;
    }
}

void DeInitUiLogServer(void)
{
    /* Deinit UI log server */
    uiLogServerRunF = FALSE;
    pthread_join(setLogUiThreadId, NULL);
}

static void *setLogUiThread(void *threadArg)
{
    INT32 				clientSockFd = INVALID_CONNECTION;
    INT32               sockOpts;
    struct pollfd       pollFd;
    DBG_CONFIG_PARAM_t  debugConfig;
    struct sockaddr_in 	clientAddr;
    socklen_t           clientAddrSize = sizeof(clientAddr);

    prctl(PR_SET_NAME, "UI_LOG_SERVER", 0, 0, 0);

    while(uiLogServerRunF)
    {
        /* Add fd for polling with write and */
        pollFd.fd = serverSockFd;
        pollFd.events = POLLRDNORM;
        pollFd.revents = 0;

        /* Poll for fd till event or timeout */
        if (poll(&pollFd, 1, 1000) <= 0)
        {
            /* Poll failed or timeout */
            continue;
        }

        clientSockFd = accept(serverSockFd, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if(clientSockFd < STATUS_OK)
        {
            EPRINT(GUI_SYS, "fail to accept log server connection: [err=%s]", strerror(errno));
            continue;
        }

        /* Make this connection as nonblocking */
        sockOpts = fcntl(clientSockFd, F_GETFL, 0);
        if (sockOpts != -1)
        {
            if ((sockOpts & O_NONBLOCK) == 0)
            {
                fcntl(clientSockFd, F_SETFL, sockOpts | O_NONBLOCK);
            }
        }

        /* Read data from TCP socket */
        if (RecvMessage(clientSockFd, (CHAR *)&debugConfig, sizeof(debugConfig), RECV_TIMEOUT_MAX_IN_SEC) == FAIL)
        {
            EPRINT(GUI_SYS, "fail to receive log server message");
        }
        else
        {
            /* Set and extract UI debug levels */
            SetUiDebugFlag(&debugConfig);
            DPRINT(GUI_SYS, "debug config updated successfully");
        }

        /* Close the client socket */
        CloseSocket(&clientSockFd);
    }

    /* Close the server socket */
    CloseSocket(&serverSockFd);
    (void)threadArg;
    pthread_exit(NULL);
}

BOOL RecvMessage(INT32 connFd, CHARPTR rcvMsg, UINT32 rcvLen, UINT32 timeout)
{
    INT32			cnt = 0;
    INT32 			pollSts;
    UINT32 			recvCntSeg = 0;
    UINT64          timestamp;
    UINT32          elapsedTime = 0;
    struct timespec ts;
    struct pollfd   pollFd;

    if (connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(GUI_SYS, "invld fd found");
        return FAIL;
    }

    /* Add fd for polling with write and */
    pollFd.fd = connFd;
    pollFd.events = POLLRDNORM | POLLRDHUP;

    /* Convert seconds to milli seconds to maintain accuracy of timeout */
    timeout *= 1000;

    /* Get current timestamp in milli sec to manage timeout */
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    timestamp = (((UINT64)ts.tv_sec * 1000) + ts.tv_nsec / NANO_SEC_PER_MILLI_SEC);

    /* Receive entire message upto timeout or max retry */
    while(TRUE)
    {
        /* Reset read events before polling */
        pollFd.revents = 0;

        /* Poll for fd till event or timeout */
        pollSts = poll(&pollFd, 1, timeout - elapsedTime);
        if (pollSts == -1)
        {
            /* Poll failed */
            EPRINT(GUI_SYS, "poll fail: [fd=%d], [err=%s]", connFd, strerror(errno));
            break;
        }
        else if (pollSts == 0)
        {
            /* Poll timeout */
            EPRINT(GUI_SYS, "poll timeout: [fd=%d], [recv_len=%d]", connFd, recvCntSeg);
            break;
        }

        /* Is other than read normal event? */
        if ((pollFd.revents & POLLRDNORM) != POLLRDNORM)
        {
            /* Is remote connection closed event? */
            if ((pollFd.revents & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(GUI_SYS, "remote connection closed: [fd=%d]", connFd);
            }
            else
            {
                EPRINT(GUI_SYS, "invld event: [fd=%d], [revents=0x%x]", connFd, pollFd.revents);
            }
            return FAIL;
        }

        /* Receive data from socket */
        cnt = recv(connFd, (rcvMsg + recvCntSeg), (rcvLen - recvCntSeg), MSG_DONTWAIT);
        if(cnt > 0)
        {
            recvCntSeg += cnt;
            if (recvCntSeg >= rcvLen)
            {
                return SUCCESS;
            }
        }
        else if(errno != EWOULDBLOCK)
        {
            EPRINT(GUI_SYS, "recv failed: [fd=%d], [err=%s]", connFd, strerror(errno));
            return FAIL;
        }

        /* Calculate elapsed time */
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        elapsedTime = (((UINT64)ts.tv_sec * 1000) + ts.tv_nsec / NANO_SEC_PER_MILLI_SEC) - timestamp;

        /* Validate elapsed time with timeout */
        if (elapsedTime >= timeout)
        {
            EPRINT(GUI_SYS, "recv timeout: [fd=%d], [cnt=%d]", connFd, cnt);
            break;
        }
    }

    return FAIL;
}

void CloseSocket(INT32PTR connFd)
{
    if (*connFd != INVALID_FILE_FD)
    {
        shutdown(*connFd, SHUT_RDWR);
        close(*connFd);
        *connFd = INVALID_FILE_FD;
    }
}
