//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DhcpServerNotify.c
@brief      This file provides APIs to communicate between DHCP server and application
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/un.h>

/* Application Includes */
#include "NetworkManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_BUFFER_SIZE 1024

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static CHAR *getEnvValue(const CHAR *envVar, CHAR *dataBuf, UINT16 dataLen);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Handles udev device connect and disconnect event
 * @param   argc
 * @param   argv
 * @return
 */
INT32 main(INT32 argc, CHAR **argv)
{
    INT32 				clientFd;
    INT32 				dataLen;
    CHAR 				argData[256];
    CHAR 				txRxBuffer[MAX_BUFFER_SIZE];
    struct sockaddr_un 	clientParam;

    // Check app notify type
    if (strcmp(argv[1], "dhcpserver") == 0)
    {
        /* Construct message to send application */
        dataLen = snprintf(txRxBuffer, MAX_BUFFER_SIZE, "%c%s%c", SOM, "DHCP_SERVER_NOTIFY", FSP);

        /* Add action parameter: add, del or old */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", argv[2], FSP);

        /* Add client's MAC address */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", argv[3], FSP);

        /* Add assigned IP address */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", argv[4], FSP);

        /* Get lease expire time (since epoch) from environment variable */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", getEnvValue("DNSMASQ_LEASE_EXPIRES", argData, sizeof(argData)), FSP);

        /* Get lease remaining time from environment variable */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", getEnvValue("DNSMASQ_TIME_REMAINING", argData, sizeof(argData)), FSP);

        /* Is hostname available in passing arg? */
        if ((argc >= 6) && (argv[5][0] != '\0'))
        {
            /* Get hostname data */
            dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", argv[5], FSP);
        }
        else
        {
            /* Add null hostname */
            dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%c", FSP);
        }

        /* Add end of message */
        dataLen += snprintf(&txRxBuffer[dataLen],MAX_BUFFER_SIZE - dataLen, "%c", EOM);
    }
    else if (strcmp(argv[1], "ddnsclient") == 0)
    {
        /* Construct message to send application */
        dataLen = snprintf(txRxBuffer, MAX_BUFFER_SIZE, "%c%s%c", SOM, "DDNS_CLIENT_NOTIFY", FSP);

        /* Get event type */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", getEnvValue("INADYN_EVENT", argData, sizeof(argData)), FSP);

        /* Get registered ip address */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", getEnvValue("INADYN_IP", argData, sizeof(argData)), FSP);

        /* Get error code */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", getEnvValue("INADYN_ERROR", argData, sizeof(argData)), FSP);

        /* Get error message */
        dataLen += snprintf(&txRxBuffer[dataLen], MAX_BUFFER_SIZE - dataLen, "%s%c", getEnvValue("INADYN_ERROR_MESSAGE", argData, sizeof(argData)), FSP);

        /* Add end of message */
        dataLen += snprintf(&txRxBuffer[dataLen],MAX_BUFFER_SIZE - dataLen, "%c", EOM);
    }
    else
    {
        return 0;
    }

    /* Create client unix socket */
    clientFd = socket(AF_UNIX, TCP_SOCK_OPTIONS, 0);
    if (clientFd == INVALID_CONNECTION)
    {
        /* Failed to create socket */
        return 0;
    }

    /* Add socket parameters */
    memset(&clientParam, 0, sizeof(clientParam));
    clientParam.sun_family = AF_UNIX;
    snprintf(clientParam.sun_path, sizeof(clientParam.sun_path), INTERNAL_SOCKET_FILE);

    /* Connecting to server */
    if (connect(clientFd, (struct sockaddr*)&clientParam, sizeof(clientParam)) < STATUS_OK)
    {
        /* Failed to create connection */
        close(clientFd);
        return 0;
    }

    /* Send request to network manager */
    send(clientFd, txRxBuffer, dataLen, 0);

    /* closing connection */
    close(clientFd);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get environment variable value
 * @param   envVar
 * @param   dataBuf
 * @param   dataLen
 * @return  Pointer to env string
 */
static CHAR *getEnvValue(const CHAR *envVar, CHAR *dataBuf, UINT16 dataLen)
{
    const CHAR *envVal;

    /* Init with null before use it */
    dataBuf[0] = '\0';

    /* Get environment variable value */
    envVal = getenv(envVar);
    if ((envVal == NULL) || (envVal[0] == '\0'))
    {
        /* No data present */
        return dataBuf;
    }

    /* Copy env data */
    snprintf(dataBuf, dataLen, "%s", envVal);
    return dataBuf;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
