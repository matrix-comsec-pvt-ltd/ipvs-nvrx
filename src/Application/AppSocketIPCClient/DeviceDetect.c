//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DeviceDetect.c
@brief      This file provides APIs to communicate between udev event and application
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
#define REQUEST_NAME        "DEV_DETECT"
#define MAX_BUFFER_SIZE     1024

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
	UINT8 				cnt;
	INT32 				clientFd;
	CHAR 				txRxBuffer[MAX_BUFFER_SIZE];
	struct sockaddr_un 	clientParam;

    //socket created
    clientFd = socket(AF_UNIX, TCP_SOCK_OPTIONS, 0);
    if (clientFd == INVALID_CONNECTION)
    {
        return 0;
    }

    //socket parameters loaded
    memset(&clientParam, 0, sizeof(clientParam));
    clientParam.sun_family = AF_UNIX;
    snprintf(clientParam.sun_path,sizeof(clientParam.sun_path),INTERNAL_SOCKET_FILE);

    //connecting to server
    if(connect(clientFd, (struct sockaddr*)&clientParam, sizeof(clientParam)) < STATUS_OK)
    {
        close(clientFd);
        return 0;
    }

    //construct message
    snprintf(txRxBuffer, MAX_BUFFER_SIZE, "%c%s%c", SOM, REQUEST_NAME, FSP);
    for(cnt = 1; cnt < argc; cnt++)
    {
        if((strlen(argv[cnt]) + strlen(txRxBuffer)) >= (MAX_BUFFER_SIZE - 1))
        {
            close(clientFd);
            return 0;
        }

        snprintf(txRxBuffer + strlen(txRxBuffer),sizeof(txRxBuffer) - strlen(txRxBuffer),"%s%c", argv[cnt], FSP);
    }

    snprintf(txRxBuffer + strlen(txRxBuffer),sizeof(txRxBuffer) - strlen(txRxBuffer), "%c", EOM);

    //send request to network manager
    if(send(clientFd, txRxBuffer, strlen(txRxBuffer), 0) < (INT32)strlen(txRxBuffer))
    {
        /* Nothing to do */
    }

	//closing connection
	close(clientFd);
	return 0;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
