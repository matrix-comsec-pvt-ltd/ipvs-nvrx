//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxPcap.c
@brief      This file facilitates capturing Network Packets.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxPcap.h"
#include "DebugLog.h"
#include "CommonApi.h"
#include "NetworkInterface.h"
#include "MobileBroadBand.h"
#include "Utils.h"

/* Library Includes */
#include <pcap.h>

//#################################################################################################
// @DEFINES
//#################################################################################################
#define SNAPLEN                     BUFSIZ
#define	NOT_IN_PROMISCUOUS_MODE     0		// This is to tell libpcap not to change promiscuous mode
#define	PCAP_READ_TIME_OUT          1000	// time in mili-seconds
#define MAX_DUMP_FILE_SIZE          (20 * MEGA_BYTE)
#define CAPTURE_FILE_NAME           "/tmp/PcapTrace.pcap"
#define PCAP_LINK                   HTML_PAGES_DIR "/PcapTrace.pcap"
#define PCAP_THREAD_STACK_SZ        (4*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	pthread_t		threadId;
	pthread_mutex_t	pcapMutex;
	BOOL			terminateFlag;
	PCAP_STATUS_e	pcapStatus;
	pcap_t			*handle;
	pcap_dumper_t	*dump;
	UINT16			numOfPktCaptured;
	UINT32			currFileSize;

    // This two variables are declared just to avoid mutex lock/unlock in both callback & thread
	UINT16			tmpNumOfPktCaptured;
	UINT32			tmpCurrFileSize;

}PCAP_PARAM_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static PCAP_PARAM_t	*pcapParam = NULL;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void pcapCallback(UINT8 *user, const struct pcap_pkthdr *hdr, const UINT8 *sp);
//-------------------------------------------------------------------------------------------------
static VOIDPTR	pcapThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   These functions starts packet capture on requested interface and with given filter.
 * @param   startParam
 * @return  Status
 * @note    This API is supporting a single capture at a time. this API should not be called if
 *          any Capture is already running.
 */
PCAP_STATUS_e PcapStart(PCAP_START_t *startParam)
{
	PCAP_STATUS_e		retStatus;
	CHAR 				errbuf[PCAP_ERRBUF_SIZE];
    UINT32 				localnet = 0;
    UINT32 				netmask = 0;
	struct bpf_program	filterPrgm;
    CHAR 				ifaceName[INTERFACE_NAME_LEN_MAX];

    /* Is pcap capturing already goingon? */
	if (pcapParam != NULL)
	{
        /* Pcap capturing is already running */
        WPRINT(SYS_LOG, "pcap capturing is already running");
        return PCAP_STATUS_CAPTURING_PACKET;
	}

    if (startParam->interface >= NETWORK_PORT_MAX)
    {
        /* Invalid interface */
        EPRINT(SYS_LOG, "invld interface found: [interface=%d]", startParam->interface);
        return PCAP_STATUS_PROCESS_ERROR;
    }

    /* Is network port link up? */
    if (FALSE == IsNetworkPortLinkUp(startParam->interface))
    {
        /* LAN1 is not up */
        WPRINT(SYS_LOG, "interface port is not up: [interface=%d]", startParam->interface);
        return PCAP_STATUS_INTERFACE_DOWN;
    }

    /* Allocate Memory to Store Data related to Capture */
    pcapParam = (PCAP_PARAM_t *)malloc(sizeof(PCAP_PARAM_t));
    if (pcapParam == NULL)
	{
        /* Fail to allocate memory */
        EPRINT(SYS_LOG, "fail to alloc memory");
        return PCAP_STATUS_PROCESS_ERROR;
	}

    MUTEX_INIT(pcapParam->pcapMutex, NULL);
    pcapParam->handle = NULL;
    pcapParam->dump = NULL;
    pcapParam->terminateFlag = FALSE;
    pcapParam->currFileSize = 0;
    pcapParam->numOfPktCaptured = 0;
    pcapParam->tmpCurrFileSize = 0;
    pcapParam->tmpNumOfPktCaptured = 0;

    do
    {
        /* Get interface name string */
        GetNetworkPortName(startParam->interface, ifaceName);
        if (ifaceName[0] == '\0')
        {
            /* Fail to get interface name */
            retStatus = PCAP_STATUS_INTERFACE_DOWN;
            EPRINT(SYS_LOG, "fail to get interface name: [interface=%d]", startParam->interface);
            break;
        }

        /* Open PCAP Session */
        pcapParam->handle = pcap_open_live(ifaceName, SNAPLEN, NOT_IN_PROMISCUOUS_MODE, PCAP_READ_TIME_OUT, errbuf);
        if (pcapParam->handle == NULL)
        {
            /* Fail to open pcap session */
            retStatus = PCAP_STATUS_PROCESS_ERROR;
            EPRINT(SYS_LOG, "fail to open pcap session: [interface=%s], [err=%s]", ifaceName, errbuf);
            break;
        }

        /* Open PCAP dump file */
        pcapParam->dump = pcap_dump_open(pcapParam->handle, CAPTURE_FILE_NAME);
        if (pcapParam->dump == NULL)
        {
            /* Fail to open pcap dump file */
            retStatus = PCAP_STATUS_PROCESS_ERROR;
            EPRINT(SYS_LOG, "fail to open pcap dump: [interface=%s]", ifaceName);
            break;
        }

        /* Get the local network and netmask */
        if (pcap_lookupnet(ifaceName, &localnet, &netmask, errbuf) < 0)
        {
            /* Fail to open pcap session */
            retStatus = PCAP_STATUS_PROCESS_ERROR;
            EPRINT(SYS_LOG, "fail to get network: [interface=%s], [err=%s]", ifaceName, errbuf);
            break;
        }

        /* Make pcap capturing non blocking. Without non-blocking, It will block if no packet receives */
        if (pcap_setnonblock(pcapParam->handle, TRUE, errbuf) == -1)
        {
            /* Fail to set pcap session non-blocking */
            retStatus = PCAP_STATUS_PROCESS_ERROR;
            EPRINT(SYS_LOG, "fail to set capturing non-block: [interface=%s], [err=%s]", ifaceName, errbuf);
            break;
        }

        /* When we make pcap capturing non blocking, we have to poll for packets */
        if (pcap_get_selectable_fd(pcapParam->handle) == -1)
        {
            /* Fail to get pcap session fd */
            retStatus = PCAP_STATUS_PROCESS_ERROR;
            EPRINT(SYS_LOG, "fail to get fd: [interface=%s]", ifaceName);
            break;
        }

        /* Is pcap filter applied? */
        if (startParam->filterStr[0] != '\0')
        {
            /* Compile Filter String to convert it in BPF program */
            if (pcap_compile(pcapParam->handle, &filterPrgm, startParam->filterStr, 1, netmask) < 0)
            {
                retStatus = PCAP_STATUS_INVALID_FILTER;
                break;
            }

            /* Set this filter to PCAP handle */
            pcap_setfilter(pcapParam->handle, &filterPrgm);

            /* Free up memory allocated for BPF program */
            pcap_freecode(&filterPrgm);
        }

        /* Create thread to capture on interface */
        if (FAIL == Utils_CreateThread(&pcapParam->threadId, pcapThread, NULL, JOINABLE_THREAD, PCAP_THREAD_STACK_SZ))
        {
            retStatus = PCAP_STATUS_PROCESS_ERROR;
            break;
        }

        /* Create soft link of PcapTrace.pcap to html root directory if not present */
        if(access(PCAP_LINK,F_OK) != STATUS_OK)
        {
            symlink(CAPTURE_FILE_NAME,PCAP_LINK);
        }

        /* Set Status to PCAP_CAPTURING_PACKET */
        pcapParam->pcapStatus = PCAP_STATUS_CAPTURING_PACKET;
        return PCAP_STATUS_CAPTURING_PACKET;

    }while(0);

    /* Release resource on fail to start capture */
    if (pcapParam->dump != NULL)
    {
        pcap_dump_close(pcapParam->dump);
    }

    if (pcapParam->handle != NULL)
    {
        pcap_close(pcapParam->handle);
    }

    pthread_mutex_destroy(&pcapParam->pcapMutex);
    FREE_MEMORY(pcapParam);
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function gives current status of packet capture.
 * @param   capturedPacket
 * @param   bytesCaptured
 * @return  pcapStatus
 */
PCAP_STATUS_e PcapGetStatus(UINT16PTR capturedPacket, UINT32PTR bytesCaptured)
{
	if (pcapParam == NULL)
	{
		*capturedPacket = 0;
		*bytesCaptured = 0;
        return PCAP_STATUS_SAFELY_CLOSED;
	}

    MUTEX_LOCK(pcapParam->pcapMutex);
    *capturedPacket = pcapParam->numOfPktCaptured;
    *bytesCaptured = pcapParam->currFileSize;
    PCAP_STATUS_e pcapStatus = pcapParam->pcapStatus;
    MUTEX_UNLOCK(pcapParam->pcapMutex);
	return pcapStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function stops packet capture and release all resources related to capture.
 */
void PcapStop(void)
{
    if (pcapParam == NULL)
	{
        return;
    }

    MUTEX_LOCK(pcapParam->pcapMutex);
    pcapParam->terminateFlag = TRUE;
    MUTEX_UNLOCK(pcapParam->pcapMutex);
    pthread_join(pcapParam->threadId, NULL);
    pthread_mutex_destroy(&pcapParam->pcapMutex);
    FREE_MEMORY(pcapParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback Function to PCAP library. It dumps given pcaket to file and updates
 *          respective local variables.
 * @param   user
 * @param   h
 * @param   sp
 */
static void pcapCallback(UINT8 *user, const struct pcap_pkthdr *hdr, const UINT8 *sp)
{
	// dump packet to file
    pcap_dump(user, hdr, sp);

	// Increment number of Packet captured
	pcapParam->tmpNumOfPktCaptured++;
    pcapParam->tmpCurrFileSize += (hdr->caplen + sizeof(struct pcap_pkthdr));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread continuously request library to capture packet and Until an error condition occurs.
 * @param   arg
 * @return
 */
static VOIDPTR pcapThread(VOIDPTR arg)
{
    INT32           pcapPollFd = pcap_get_selectable_fd(pcapParam->handle);
    INT16           pollRevent;
    UINT8           pollSts;

    THREAD_START("PCAP_CAPTURE");

    while(TRUE)
    {
        pollSts = GetSocketPollEvent(pcapPollFd, (POLLRDNORM | POLLRDHUP), 1000, &pollRevent);

        /* If poll failure occurs */
        if (FAIL == pollSts)
        {
            EPRINT(SYS_LOG, "poll failed: [err=%s]", STR_ERR);
            break;
        }

        /* If poll timeout occurs */
        if (TIMEOUT == pollSts)
        {
            MUTEX_LOCK(pcapParam->pcapMutex);
            if (pcapParam->terminateFlag == TRUE)
            {
                MUTEX_UNLOCK(pcapParam->pcapMutex);
                break;
            }
            MUTEX_UNLOCK(pcapParam->pcapMutex);
            continue;
        }

        /* if other than read event */
        if ((pollRevent & POLLRDNORM) != POLLRDNORM)
        {
            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(SYS_LOG, "remote connection closed");
            }
            else
            {
                EPRINT(SYS_LOG, "invalid poll event:[0x%x]", pollRevent);
            }
            break;
        }

        /* Capture single packet */
        if (pcap_dispatch(pcapParam->handle, 1, pcapCallback, (UINT8PTR)pcapParam->dump) < 0)
		{
            MUTEX_LOCK(pcapParam->pcapMutex);
			pcapParam->pcapStatus = PCAP_STATUS_PROCESS_ERROR;
            MUTEX_UNLOCK(pcapParam->pcapMutex);
            EPRINT(SYS_LOG, "fail to get packet, exiting from thread");
            break;
		}

        MUTEX_LOCK(pcapParam->pcapMutex);
        pcapParam->numOfPktCaptured = pcapParam->tmpNumOfPktCaptured;
        pcapParam->currFileSize = pcapParam->tmpCurrFileSize;

        /* Check if the File size is exceeding the MAX_DUMP_FILE_SIZE */
        if (pcapParam->currFileSize > MAX_DUMP_FILE_SIZE)
        {
            pcapParam->pcapStatus = PCAP_STATUS_MAX_SIZE_REACHED;
            MUTEX_UNLOCK(pcapParam->pcapMutex);
            break;
        }

        /* EXIT if terminateFlag is SET */
        if (pcapParam->terminateFlag == TRUE)
        {
            pcapParam->pcapStatus = PCAP_STATUS_SAFELY_CLOSED;
            MUTEX_UNLOCK(pcapParam->pcapMutex);
            break;
        }

        /* Pcap capturing is going on */
        pcapParam->pcapStatus = PCAP_STATUS_CAPTURING_PACKET;
        MUTEX_UNLOCK(pcapParam->pcapMutex);
    }

    /* Close pcap dump and handle, exit from thread */
	pcap_dump_close(pcapParam->dump);
	pcap_close(pcapParam->handle);
	pthread_exit(NULL);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
