#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <dirent.h>
#include <sys/mount.h>
#include <mntent.h>
#include <poll.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>

#include <libudev.h>

#include "StorageManagment.h"
#include "CommonApi.h"

#define MAX_DEVICE_NAME_WIDTH   25
#define MAX_CONCURRANT_REQ      20
#define SYSTEM_COMMAND_SIZE     500
#define SOM                     0x01
#define EOM                     0x04
#define INTERNAL_SOCKET_FILE    "/tmp/IntSocket"
#define UDEV_START_STOP_SCRIPT  "/etc/init.d/S10udev %s"
#define MTAB_FILE_PATH          "/etc/mtab"
#define	UDEV_ENTRY_SIZE         1024
#define MAX_RECV_TIMEOUT        1		// In second
#define GET_DEVICE_PATH			"/dev/%s1"

// folder access permission in harddisk
#define	FOLDER_PERMISSION             (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define PROC_PATH					"/proc/"

// Maximum file name size specially for stream file
#define MAX_FILE_NAME_SIZE			(150)
#define	MAX_LINE			        (512)
#define MOUNTFSSIZE					500

/* Nano seconds in 1 millisec */
#define NANO_SEC_PER_MILLI_SEC      1000000LL

static const char fsTypeName[FORMAT_MAX][10] =
{
    "ext4",
    "vfat",
    "ntfs"
};

static const char fsOptionsStr[FORMAT_MAX][50] =
{
    "data=writeback",
    "",
    ""
};

static const char udevStartStop[2][10] =
{
    "stop",
    "start",
};

StorageManagment::StorageManagment(QObject *parent): QObject(parent)
{
    int                     yes = true;
    struct                  sockaddr_un internalServerAddr;
    char                    sysCommand[SYSTEM_COMMAND_SIZE];
    UINT8                   cnt = 0;
    INT32                   udevPid;
    INT32                   sockOpts;
    bool                    restart_udev = false;
    struct udev             *udev = NULL;
    struct udev_enumerate	*udev_enumerate = NULL;

    do
    {
        //Create local server for handle requst.(e.g cgi, etc)
        if( (internalServerFd = socket(AF_UNIX, (SOCK_STREAM | SOCK_CLOEXEC), 0) ) < 0)
        {
            EPRINT(GUI_SYS, "Internal server socket: socket() failed [err=%s]", strerror(errno));
            break;
        }

        unlink(INTERNAL_SOCKET_FILE);
        bzero(&internalServerAddr, sizeof(internalServerAddr) );
        internalServerAddr.sun_family = AF_UNIX;
        strcpy(internalServerAddr.sun_path, INTERNAL_SOCKET_FILE);

        sockOpts = fcntl(internalServerFd, F_GETFL, 0);
        if (sockOpts != -1)
        {
            if ((sockOpts & O_NONBLOCK) == 0)
            {
                fcntl(internalServerFd, F_SETFL, sockOpts | O_NONBLOCK);
            }
        }

        setsockopt(internalServerFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if(bind(internalServerFd, (struct sockaddr*)&internalServerAddr, sizeof(internalServerAddr)) < STATUS_OK)
        {
            EPRINT(GUI_SYS, "Internal server socket: bind() failed [err=%s]", strerror(errno));
            CloseSocket(&internalServerFd);
            break;
        }

        if(listen(internalServerFd, MAX_CONCURRANT_REQ) < STATUS_OK)
        {
            EPRINT(GUI_SYS, "Internal server socket: listen() failed [err=%s]", strerror(errno));
            CloseSocket(&internalServerFd);
            break;
        }

        //Changing permision of socket file
        chmod(INTERNAL_SOCKET_FILE, 0777);
    }
    while(0);

    udev = udev_new();
    if(NULL != udev)
    {
        udev_enumerate = udev_enumerate_new(udev);
        if(NULL != udev_enumerate)
        {
            DPRINT(GUI_SYS, "udev_enumerate_scan_devices");
            udev_enumerate_scan_devices(udev_enumerate);
            udev_update_uevent(udev_enumerate, "add");
            udev_enumerate_unref(udev_enumerate);
        }
        else
        {
            restart_udev = true;
            DPRINT(GUI_SYS, "Failed to get udev_enumerate handle");
        }
        udev_unref(udev);
    }
    else
    {
        restart_udev = true;
        DPRINT(GUI_SYS, "Failed to get udev handle");
    }
    if (restart_udev == true)
    {
        do
        {
            //Stop Udev
            sprintf(sysCommand, UDEV_START_STOP_SCRIPT, udevStartStop[false]);
            Utils_ExeCmd(sysCommand);
            sleep(2);

            udevPid = GetPidOfProcess((CHARPTR)"udevd");
            if(udevPid == (INT32)NILL)
            {
                DPRINT(GUI_SYS, "Udev Script Stop");
                break;
            }
            else
            {
                cnt++;
            }
        }
        while(cnt < 10);

        if (cnt >= 10)
        {
            EPRINT(GUI_SYS, "Udev Script Not Stoped");
            return;
        }

        cnt = 0;
        do
        {
            //Start Udev
            sprintf(sysCommand, UDEV_START_STOP_SCRIPT, udevStartStop[true]);
            Utils_ExeCmd(sysCommand);
            sleep(2);

            udevPid = GetPidOfProcess((CHARPTR)"udevd");
            if (udevPid != (INT32)NILL)
            {
                DPRINT(GUI_SYS, "Udev Script Started ID [%d]", udevPid);
                break;
            }
            else
            {
                cnt++;
                DPRINT(GUI_SYS, "Udev Script Started ID [%d]", udevPid);
            }
        }
        while(cnt < 10);
        if (cnt >= 10)
        {
            EPRINT(GUI_SYS, "Udev Script Not Started");
            return;
        }
    }
}

INT32 StorageManagment::GetPidOfProcess(CHARPTR processName)
{
    INT32			ret, loop;
    INT32			pid = NILL;
    UINT64			tempVal;
    DIR				* dirPtr = NULL;
    FILE			* filePtr = NULL;
    CHAR			filePath[FILENAME_MAX];
    CHAR			cmdlineBuf[FILENAME_MAX];
    CHARPTR 		procStr = NULL;
    struct	dirent	* dirEntry = NULL;
    struct	stat	fileStat;

    dirPtr = opendir(PROC_PATH);
    if (dirPtr != NULL)
    {
        while(1)
        {
            dirEntry = readdir(dirPtr);

            if (dirEntry == NULL)
            {
                break;
            }

            sprintf(filePath, PROC_PATH);
            strcat(filePath, dirEntry->d_name);

            memset(&fileStat, 0, sizeof(struct stat));

            if (stat(filePath, &fileStat) < STATUS_OK)
                continue;

            if ((fileStat.st_mode & S_IFMT) && S_IFDIR)
            {
                strcat(filePath, "/cmdline");

                filePtr = fopen(filePath, "r");

                if (filePtr != NULL)
                {
                    memset(cmdlineBuf, 0, sizeof(cmdlineBuf));

                    ret = fread(cmdlineBuf, sizeof(CHAR), (FILENAME_MAX - 1), filePtr);
                    fclose(filePtr);

                    for (loop = 0; (ret > 0) && (loop < (ret - 1)); loop++)
                    {
                        if (cmdlineBuf[loop] == '\0')
                        {
                            cmdlineBuf[loop] = ' ';
                        }
                    }

                    procStr = strstr(cmdlineBuf, processName);

                    if ( (procStr != NULL) && (strncmp(procStr, processName, strlen(processName)) == STATUS_OK) )
                    {
                        if (AsciiToInt(dirEntry->d_name, &tempVal) == SUCCESS)
                        {
                            pid = (INT32)tempVal;
                            EPRINT(GUI_SYS, "[rocess=%s] [pid=%d]", processName, pid);
                        }
                        break;
                    }
                }
            }
        }
        closedir(dirPtr);
    }
    return pid;
}

BOOL StorageManagment::AsciiToInt(CHARPTR ascii, UINT64PTR integer)
{
    BOOL 		status = SUCCESS;
    UINT8 		asciiLen;
    UINT64 		multiplicant;

    // check for invalid input/output parameters
    if ( (ascii != NULL) && (integer != NULL) )
    {
        *integer = 0;
        multiplicant = 1;

        // run the loop, length times, and sum up the individual element in integer
        for (asciiLen = strlen(ascii); (asciiLen > 0); asciiLen--)
        {
            // check if the value falls between '0' to '9'
            if ( (ascii[asciiLen - 1] >= '0') && (ascii[asciiLen - 1] <= '9') )
            {
                *integer += ( (ascii[asciiLen - 1] - '0') * multiplicant);
                multiplicant *= 10;
            }
            else
            {
                *integer = 0;
                status = FAIL;
                break;
            }
        }
    }
    else
    {
        status = FAIL;
    }
    return status;
}

//*****************************************************************************
//	runNetworkManagerInt()
//	Param:
//		IN :	//Internal Server socket fd
//		OUT:	NONE
//	Returns:	SUCCESS/FAIL
//	Description:
//		This is thread function. This function server function which receive
//		message from internal loopback. Send response to internal loopback.
//	[Pre-condition:]
//
//	[Constraints:]
//		NONE
//
//*****************************************************************************
void StorageManagment::internalMsgServerLoop()
{
    INT32               sockOpts;
    CHAR                *msgPtr;
    INT32               connFd;
    struct sockaddr_in  clientAddr;
    socklen_t           clientAddrSize = sizeof(clientAddr);
    UDEV_DEVICE_INFO_t  udevInfo;
    char 				rcvMsg[MAX_RCV_SZ];
    unsigned int		rcvMsgLen;

    while(1)
    {
        //Wait for client to connected
        connFd = accept(internalServerFd, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (connFd < 0)
        {
            break;
        }

        //Make this connection as nonblocking
        sockOpts = fcntl(connFd, F_GETFL, 0);
        if (sockOpts != -1)
        {
            if ((sockOpts & O_NONBLOCK) == 0)
            {
                fcntl(connFd, F_SETFL, sockOpts | O_NONBLOCK);
            }
        }

        // read data from TCP socket
        if (RecvMessage(connFd, rcvMsg, &rcvMsgLen, SOM, EOM, MAX_RCV_SZ, MAX_RECV_TIMEOUT) == FAIL)
        {
            CloseSocket(&connFd);
            continue;
        }

        rcvMsg[rcvMsgLen] = '\0';
        msgPtr = (rcvMsg + 1); // for remove SOM from received data
        if(FindHeaderIndex(&msgPtr) != CMD_SUCCESS)
        {
            CloseSocket(&connFd);
            continue;
        }

        CloseSocket(&connFd);
        if(ParseStr(&msgPtr, FSP, udevInfo.subSys, sizeof(udevInfo.subSys)) == FAIL)
        {
            break;
        }

        if(ParseStr(&msgPtr, FSP, udevInfo.action, sizeof(udevInfo.action)) == FAIL)
        {
            break;
        }

        if(ParseStr(&msgPtr, FSP, udevInfo.path, sizeof(udevInfo.path)) == FAIL)
        {
            break;
        }

        if(ParseStr(&msgPtr, FSP, udevInfo.serial, sizeof(udevInfo.serial)) == FAIL)
        {
            break;
        }

        if(ParseStr(&msgPtr, FSP, udevInfo.baseNode, sizeof(udevInfo.baseNode)) == FAIL)
        {
            break;
        }

        emit act1(udevInfo);
    }
}

//*****************************************************************************
//	CloseSocket()
//	Param:
//		IN:	 ConnFd	- Connection which is closed	.
//
//	Returns:
//		None
//	Description:
//		This function closing connection(Socket)
//
//*****************************************************************************
void StorageManagment::CloseSocket(int* connFd)
{
    if(*connFd != INVALID_CONNECTION)
    {
        shutdown(*connFd, SHUT_RDWR);
        close(*connFd);
        *connFd = INVALID_CONNECTION;
    }
}

///////////////////////////////////////////////////////////////////////////////
//RecvMessage()
//Parameter:
//Input : 	int 		sockFd,
//			char 		startOfData,
//			char 		endOfData,
//			unsigned int 	timeOut
//Output:	char* 	rcvMsg,
//			unsigned int* 	rcvLen,
//Return :	SUCCESS/FAIL
//Description:
//			 This function received data from TCP socket and gives into out put
//			 parameter.
///////////////////////////////////////////////////////////////////////////////
bool StorageManagment::RecvMessage(int connFd, char* rcvMsg, unsigned int* rcvLen, char startOfData,
                                     char endOfData, unsigned int maxData, UINT32 timeout)
{
    struct timespec ts;
    struct pollfd   pollFd;
    INT32			cnt = 0;
    INT32 			pollSts;
    UINT32 			recvCntSeg = 0;
    UINT64          timestamp;
    UINT32          elapsedTime = 0;

    if (connFd == INVALID_CONNECTION)
    {
        /* Invalid FD found */
        EPRINT(GUI_SYS, "Invld fd found");
        return FAIL;
    }

    /* Add fd for polling with write and */
    pollFd.fd = connFd;
    pollFd.events = POLLRDNORM | POLLRDHUP;

    if (timeout == 0)
    {
        /* if supplied timeout is 0 sec, then by default wait for 15 msec */
        timeout = 15*1000;
    }
    else
    {
        /* Convert seconds to milli seconds to maintain accuracy of timeout */
        timeout *= 1000;
    }

    /* Get current timestamp in milli sec to manage timeout */
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    timestamp = (((UINT64)ts.tv_sec * 1000) + ts.tv_nsec / NANO_SEC_PER_MILLI_SEC);

    /* Set receive length to zero */
    *rcvLen = 0;

    /* Receive entire message upto timeout or max retry */
    while(1)
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
                return FAIL;
            }
            else
            {
                EPRINT(GUI_SYS, "invld event: [fd=%d], [revents=0x%x]", connFd, pollFd.revents);
                break;
            }
        }

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
                    EPRINT(GUI_SYS, "EOM not found: [fd=%d], [recv_len=%d]", connFd, recvCntSeg);
                    return FAIL;
                }
            }
            else
            {
                EPRINT(GUI_SYS, "invld SOM found: [SOM=%x] [fd=%d]", rcvMsg[0], connFd);
                return FAIL;
            }
        }
        else if (cnt == 0)
        {
            /* Error occurred on socket */
            EPRINT(GUI_SYS, "no data recv due to connection close: [fd=%d]", connFd);
            return FAIL;
        }
        else
        {
            /* Recv API interrupted by signal */
            if (errno == EINTR)
            {
                EPRINT(GUI_SYS, "recv interrupted by siganl: [fd=%d]", connFd);
            }
            else if (errno == EBADF)
            {
                EPRINT(GUI_SYS, "bad fd found: [fd=%d]", connFd);
                return FAIL;
            }
            else if (errno != EWOULDBLOCK)
            {
                return FAIL;
            }
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

//*****************************************************************************
//	ParseStr()
//	Param:
//		IN:	 src - source pointer from which to parse string.
//			 maxDestSize - Length of string buffer to be filled.
//		OUT: dest - Pointer to destination buffer where parse string is stored.
//	Returns:
//		None
//	Description:
//		This function will parse string from source buffer at current pointer till
//		FSP and copy to destination buffer
//
//*****************************************************************************
bool StorageManagment::ParseStr(char** src, char delim, char* dest, unsigned int maxDestSize)
{
    bool		retVal = FAIL;
    char*		sptr;
    char*       eptr;
    unsigned int 		length;

    sptr = *src;
    eptr = (char *)memchr(*src, delim, maxDestSize);

    if(eptr != NULL)
    {
        length = eptr - sptr;

        if(length < maxDestSize)
        {
            strncpy(dest, *src, length);
            dest[length] = '\0';
            *src += (length + 1);
            retVal = SUCCESS;
        }
    }
    return retVal;
}

//*****************************************************************************
//	ConvertStringToIndex()
//	Param:
//		IN:	strPtr- string ptr which is compare with all string
//			strBuffPtr- buffer pointer to all string
//			maxIndex - max number of string in given buffer pointer
//		OUT:NONE
//	Returns:
//		index-index of match string from string buffer
//	Description:
//			This function compare two string. if both match then return
//			index compare string.
//
//*****************************************************************************
unsigned char StorageManagment::ConvertStringToIndex(char* strPtr,const char** strBuffPtr, unsigned char maxIndex)
{
    unsigned char	index = 0;

    //check upto max index in string buffer
    while(index < maxIndex)
    {
        //if match outof loop
        if( (strcasecmp(strPtr, strBuffPtr[index]) ) == 0)
        {
            break;
        }
        index++;
    }
    //return maxindex or match string index
    return index;
}

//*****************************************************************************
//	FindHeaderIndex()
//	Param:
//		IN:	buf pointer
//		OUT: index of header
//	Returns:
//		Message parse success or error reason
//	Description:
//			This function out index of header if success otherwise error
//
//*****************************************************************************
NET_CMD_STATUS_e StorageManagment::FindHeaderIndex(char** tmpBufPtr)
{
    char buf[RECV_MSG_HEADER_LENGTH];

    if (ParseStr(tmpBufPtr, FSP, buf, RECV_MSG_HEADER_LENGTH) == SUCCESS)
    {
        return (strcasecmp(buf, "DEV_DETECT") == 0) ? CMD_SUCCESS : CMD_INVALID_MESSAGE;
    }

    return CMD_INVALID_SYNTAX;
}

//*****************************************************************************
//	mountDevice()
//	Param:
//		IN : char* dev		//device name which is to be mount
//			 char* path		// path where device should mount
//			 RAW_MEDIA_FORMAT_TYPE_e fsType	// format type to mount
//
//		OUT: None
//	Returns:
//		SUCCESS/FAIL
//	Description:
// 		This function mounts the give device at given node in inpur parameters.
//*****************************************************************************
bool StorageManagment::mountDevice(QString type, QString baseNode)
{
    char                    sysCmd[SYSTEM_COMMAND_SIZE];
    char                    path[MAX_FILE_NAME_SIZE];
    RAW_MEDIA_FORMAT_TYPE_e fsType;

    if(type.contains ("HDD"))
    {
        fsType = EXT_4;
    }
    else
    {
        fsType = FAT;
    }
    snprintf(path, sizeof(path), "%s%s/", MOUNT_PATH, type.toUtf8().constData());
    sprintf(sysCmd, GET_DEVICE_PATH, baseNode.toUtf8().constData());

    return (mountDisk(sysCmd,path,fsType));
}

bool StorageManagment::mountDisk(CHARPTR deviceNode, CHARPTR mountPath, RAW_MEDIA_FORMAT_TYPE_e fsType)
{
    do
    {
        if(access(mountPath, F_OK) != STATUS_OK)
        {
            if(mkdir(mountPath, FOLDER_PERMISSION) != STATUS_OK)
            {
                EPRINT(GUI_SYS, "fail to create dir: [path=%s], [err=%s]", mountPath, strerror(errno));
                break;
            }
        }

        if(mount(deviceNode, mountPath, fsTypeName[fsType], (MS_MGC_VAL | MS_NOATIME | MS_NODIRATIME), fsOptionsStr[fsType]) != STATUS_OK)
        {
            EPRINT(GUI_SYS, "fail to mount: [node=%s], [path=%s], [err=%s]", deviceNode, mountPath, strerror(errno));
            break;
        }

        /* Get information about mounted filesystem */
        struct statfs64 fs;
        if (statfs64(mountPath, &fs) != STATUS_OK)
        {
            EPRINT(GUI_SYS, "statfs64 fail: [node=%s], [path=%s], [err=%s]", deviceNode, mountPath, strerror(errno));
            unmountDevice(mountPath);
            break;
        }

        /* Is filesystem mounted with read-only? */
        if ((fs.f_flags & ST_RDONLY) != 0)
        {
            EPRINT(GUI_SYS, "file system mounted read-only: [node=%s], [path=%s]", deviceNode, mountPath);
            unmountDevice(mountPath);
            break;
        }

        /* File system mounted with read and write */
        return SUCCESS;

    }while(0);

    EPRINT(GUI_SYS, "fail to mount device: [node=%s]", deviceNode);
    if (rmdir(mountPath) != STATUS_OK)
    {
        EPRINT(GUI_SYS, "fail to remove mount point: [path=%s]", mountPath);
    }
    return FAIL;
}

//*****************************************************************************
//	unmountDevice()
//	Param:
//		IN : char* mountPath		// path where device should mount
//		OUT: None
//	Returns:
//		SUCCESS/FAIL
//	Description:
// 		This function unmount the device as from given mount point
//*****************************************************************************
bool StorageManagment::unmountDevice(QString type)
{
    char   mountPath[MAX_FILE_NAME_SIZE];

    snprintf(mountPath, sizeof(mountPath), "%s%s/", MOUNT_PATH, type.toUtf8().constData());

    do
    {
        // check mount point was present
        if (access(mountPath, F_OK) != STATUS_OK)
        {
            EPRINT(GUI_SYS, "Mount Point not present %s", mountPath);
            break;
        }
        // unmount device
        if (umount(mountPath) != STATUS_OK)
        {
            EPRINT(GUI_SYS, "Device not unmounted [%s], Err[%s]", mountPath, strerror(errno));
            if(umount2(mountPath,MNT_FORCE) != STATUS_OK)
            {
                EPRINT(GUI_SYS, "Device force unmount Failed [%s] errNo [%s]",mountPath, strerror(errno));
                if(umount2(mountPath,MNT_DETACH) != STATUS_OK)
                {
                    EPRINT(GUI_SYS, "Device not detached [%s] errNo [%s]",mountPath, strerror(errno));
                    break;
                }
            }
        }
        // remove mount point
        if (rmdir(mountPath) != STATUS_OK)
        {
            EPRINT(GUI_SYS, "Mount point not removed %s", mountPath);
        }
        DPRINT(GUI_SYS, "UnMount Device %s Success", mountPath);
        return SUCCESS;
    }while(0);

    return FAIL;
}

void StorageManagment::udev_update_uevent(struct udev_enumerate *udev_enumerate, const char *action)
{
    qint32	uevent_fd = -1;
    char	uevent_file[UDEV_ENTRY_SIZE];
    char*	udev_list_entry;

    struct udev_list_entry *entry = NULL;

    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(udev_enumerate))
    {
        udev_list_entry = (char *)udev_list_entry_get_name(entry);

        memset(uevent_file, 0, UDEV_ENTRY_SIZE);
        strncpy(uevent_file, udev_list_entry, (UDEV_ENTRY_SIZE - 1));
        strncat(uevent_file, "/uevent", (UDEV_ENTRY_SIZE - 1 - strlen(uevent_file)));

        uevent_fd = open(uevent_file, O_WRONLY | O_CLOEXEC);
        if (uevent_fd < 0)
            continue;

        if (write(uevent_fd, action, strlen(action)) < 0)
            EPRINT(GUI_SYS, "Failed to update %s", uevent_file);

        close(uevent_fd);
        uevent_fd = -1;
    }
}

