///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : NVR [Network Video Recorder]
//   Owner        : Aekam Parmar
//   File         : MediaRequest.cpp
//   Description  : This module provides common APIs to receive media for live
//                  and playback.
//
/////////////////////////////////////////////////////////////////////////////

#include "MediaRequest.h"
#include <QTime>

//******** Defines and Data Types ****
#define SYNC_RECV_HEADER_TIMEOUT        500

//******** Global Variables **********
const quint16 frameResolution[MAX_RESOLUTION][MediaRequest::MAX_FRAME_DIMENSIONS] =
{
		{0,     0},     // resolution 00
		{160,   90},    // resolution 01
		{160,   100},   // resolution 02
		{160,   112},   // resolution 03
		{160,   120},   // resolution 04
		{160,   128},   // resolution 05
		{176,   112},   // resolution 06
		{176,   120},   // resolution 07
		{176,   144},   // resolution 08
		{192,   144},   // resolution 09
		{192,   192},   // resolution 10
		{240,   135},   // resolution 11
		{240,   180},   // resolution 12
		{256,   192},   // resolution 13
		{256,   256},   // resolution 14
		{320,   180},   // resolution 15
		{320,   192},   // resolution 16
		{320,   200},   // resolution 17
		{320,   240},   // resolution 18
		{320,   256},   // resolution 19
		{320,   320},   // resolution 20
		{352,   240},   // resolution 21
		{352,   244},   // resolution 22
		{352,   288},   // resolution 23
		{384,   216},   // resolution 24
		{384,   288},   // resolution 25
		{384,   384},   // resolution 26
		{480,   270},   // resolution 27
		{480,   300},   // resolution 28
		{480,   360},   // resolution 29
		{512,   384},   // resolution 30
		{512,   512},   // resolution 31
		{528,   320},   // resolution 32
		{528,   328},   // resolution 33
		{640,   360},   // resolution 34
		{640,   368},   // resolution 35
		{640,   400},   // resolution 36
		{640,   480},   // resolution 37
		{640,   512},   // resolution 38
		{704,   240},	// resolution 39
		{704,   288},	// resolution 40
		{704,   480},	// resolution 41
		{704,   570},	// resolution 42
		{704,   576},	// resolution 43
		{720,   480},	// resolution 44
		{720,   576},	// resolution 45
		{768,   576},	// resolution 46
		{768,   768},	// resolution 47
		{800,   450},	// resolution 48
		{800,   480},	// resolution 49
		{800,   500},	// resolution 50
		{800,   600},	// resolution 51
		{860,   540},	// resolution 52
		{960,   480},	// resolution 53
		{960,   540},	// resolution 54
		{960,   576},	// resolution 55
		{960,   720},	// resolution 56
		{960,   768},	// resolution 57
		{1024,  576},	// resolution 58
		{1024,  640},	// resolution 59
		{1024,  768},	// resolution 60
		{1056,  1056},	// resolution 61
		{1140,  1080},	// resolution 62
		{1280,  720},	// resolution 63
		{1280,  800},	// resolution 64
		{1280,  960},	// resolution 65
		{1280,  1024},	// resolution 66
		{1280,  1280},	// resolution 67
		{1286,  972},	// resolution 68
		{1296,  968},	// resolution 69
		{1296,  972},	// resolution 70
		{1360,  768},	// resolution 71
		{1376,  768},	// resolution 72
		{1440,	912},	// resolution 73
		{1472,	960},	// resolution 74
		{1536,	1536},	// resolution 75
		{1600,	904},	// resolution 76
		{1600,	912},	// resolution 77
		{1600,	1200},	// resolution 78
		{1680,	1056},	// resolution 79
		{1824,	1376},	// resolution 80
		{1920,	1080},	// resolution 81
		{1920,	1200},	// resolution 82
		{1920,	1440},	// resolution 83
		{2032,	1920},	// resolution 84
		{2048,	1536},	// resolution 85
		{2560,	1600},	// resolution 86
		{2560,	1920},	// resolution 87
		{2592,	1944},	// resolution 88
		{2944,	1920},	// resolution 89
		{3648,	2752},	// resolution 90
		{1408,	920},	// resolution 91
		{1120,	630},	// resolution 92
		{528,	384},	// resolution 93
		{1600,	720},	// resolution 94
		{2592,	1520},	// resolution 95
		{2688,	1520},	// resolution 96
		{2560,	1440},	// resolution 97
		{4096,	2160},	// resolution 98
		{3840,	2160},	// resolution 99
		{2560,	2048},	// resolution 100
		{1400,	1050},	// resolution 101
		{2304,	1296},	// resolution 102
        {3072,	2048},	// resolution 103
		{3072,	1728},	// resolution 104
        {2944,	1656},	// resolution 105
        {3200,	1800}	// resolution 106
};

//******** Static Variables **********
// 4 byte magic code for varification of frame header
quint32 MediaRequest::magicCode = 0x000001FF;

//******** Function Definitions ******

//*****************************************************************************
//  MediaRequest ()
//      Param:
//          IN : SERVER_INFO_t serverInfo   // server informations, like
//                                          // IP address and TCP port
//               REQ_INFO_t requestInfo     // request information like,
//                                          // sessionId, requestId, payload,
//                                          // timeout
//               SET_COMMAND_e commandId    // command type
//               quint8 decoderId           // decoder session id
//          OUT: NONE
//	Returns:
//		 Not Applicable
//      Description:
//          This API is constructor of class MediaRequest. It initializes the
//          newly created object with server information, request information,
//          command type and decoder id.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
MediaRequest::MediaRequest (SERVER_INFO_t serverInfo,
                            REQ_INFO_t &requestInfo,
                            SET_COMMAND_e commandId,
                            quint8 decoderId,
                            const quint8 *pDecIdSet)
    : CommandRequest(serverInfo, requestInfo, commandId, STRM_RELAT_CMD_SES_ID)
{
    setObjectName ("MediaRequest[" + QString("%1").arg (decoderId) + "]");

    // set run flag to true
    runFlag = true;
    decId = decoderId;

    if(pDecIdSet != NULL)
    {
        for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
        {
            SyncDecIdSet[index] = pDecIdSet[index];
        }
    }
}

//*****************************************************************************
//  ~MediaRequest ()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//	Returns:
//		 Not Applicable
//      Description:
//          This API is destructor of class MediaRequest.
//          As of now it does no functionality, kept for future use.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
MediaRequest::~MediaRequest ()
{
}

//*****************************************************************************
//  setRunFlag ()
//      Param:
//          IN : bool flag
//          OUT: NONE
//	Returns:
//		 NONE
//      Description:
//          This API sets the status of run flag to true or false.
//          Which in turn breaks the media loop, if set to false.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void MediaRequest::setRunFlag (bool flag)
{
    // lock write access for run flag
    runFlagLock.lockForWrite ();
    // set run flag to the value of flag
    runFlag = flag;
    // unlock access to run flag
    runFlagLock.unlock ();
}


bool MediaRequest::getRunFlag()
{
    bool flag = false;
    runFlagLock.lockForRead();
    flag = runFlag;
    runFlagLock.unlock();
    return flag;
}


//*****************************************************************************
//  receiveHeader ()
//      Param:
//          IN : QTcpSocket &tcpSocket      // tcp socket from which the header
//                                          // will be read
//          OUT: FRAME_HEADER_t &header     // output buffer for frame header
//	Returns:
//		 bool [true / false]
//      Description:
//          This API reads a header from the specified tcp socket.
//          If any error occurs, it set status id to appropriate code.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool MediaRequest::receiveHeader (FRAME_HEADER_t &header,
                                  QTcpSocket &tcpSocket)
{
    bool        status = true;
    bool        tempStopFlag = false;
    quint8      index = 0;

    qint64      chunkSize = 0;
	size_t      transferredBytes = 0;
    qint64      dataAvailable = 0;

    char *headerPtr = (char *)&header;

    // loop till whole header is not received
    while (transferredBytes < sizeof(FRAME_HEADER_t))
    {
        // check if the data is available to read
        dataAvailable = tcpSocket.bytesAvailable ();

        if (dataAvailable >= 0)
        {
            if(dataAvailable == 0)
            {
                do
                {
                    tempStopFlag = getStopFlag ();

                    if(tempStopFlag == true)
                    {
                        status = false;
                        break;
                    }
                    else
                    {
                        // wait for socket to become readable
                        status = tcpSocket.waitForReadyRead (1 * ONE_MILISEC);

                        if(status == false)
                        {
                            if(!((tcpSocket.error() == QAbstractSocket::ConnectionRefusedError)
                                 || (tcpSocket.error() == QAbstractSocket::SocketTimeoutError)))
                            {
                                break;
                            }
                            else
                            {
                                index++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                while(index < request.timeout);

                if(status == false)
                {
                    statusId = CMD_SERVER_NOT_RESPONDING;
                    break;
                }
            }
            else
            {
                // read data from socket
                chunkSize = tcpSocket.read (headerPtr + transferredBytes,
                                            (sizeof(FRAME_HEADER_t) -
                                             transferredBytes));
                if(chunkSize >= 0)
                {
                    transferredBytes += chunkSize;
                }
                else
                {
                    status = false;
                    statusId = CMD_PROCESS_ERROR;
                    break;
                }
            }
        }
        else
        {
            status = false;
            statusId = CMD_SERVER_NOT_RESPONDING;
            break;
        }
    }

    // if magic code in header does not match
    if( (status == true) && (header.magicCode != MediaRequest::magicCode) )
    {
        status = false;
        statusId = CMD_PROCESS_ERROR;
    }
    return status;
}

/**
 * @brief MediaRequest::receiveHeader_v2
 * @param writePtr
 * @param size
 * @param tcpSocket
 * @param timeoutMs
 * @param bytesRead
 * @return
 */
bool MediaRequest::receiveHeader_v2(char* writePtr, qint64 size, QTcpSocket &tcpSocket, quint16 timeoutMs, quint64 *bytesRead)
{

    bool status = true;
    QAbstractSocket::SocketError sockError;
    qint64 chunkSize = 0;
    qint64 dataAvailable = 0;

    /* set default status */
    statusId = CMD_PROCESS_ERROR;

    /* check if the enough data is available to read */
    dataAvailable = tcpSocket.bytesAvailable();

    if (dataAvailable <= 0)
    {
        /* wait for data on socket */
        status = tcpSocket.waitForReadyRead(timeoutMs);

        /* there may be timeout or other error */
        if (status == false)
        {
            /* check for socket errors */
            sockError = tcpSocket.error();

            if (sockError == QAbstractSocket::SocketTimeoutError)
            {
                statusId = CMD_REQUEST_IN_PROGRESS;
            }
            else
            {                
                statusId = CMD_SERVER_NOT_RESPONDING;
            }
            return false;
        }        
    }    

    /* read data from scoket */
    chunkSize = tcpSocket.read(writePtr, size);

    /* data should be read as per request */
    if (chunkSize == -1)
    {
        statusId = CMD_SERVER_NOT_RESPONDING;
        return false;
    }    

    *bytesRead = chunkSize;

    return true;
}

/**
 * @brief MediaRequest::receiveFrame_v2
 * @param writePtr
 * @param size
 * @param tcpSocket
 * @param timeoutMs
 * @param bytesRead
 * @return
 */
bool MediaRequest::receiveFrame_v2(char *writePtr, qint64 size, QTcpSocket &tcpSocket, quint16 timeoutMs, quint64 *bytesRead)
{

    bool status = true;
    QAbstractSocket::SocketError sockError;
    qint64 chunkSize = 0;
    qint64 dataAvailable = 0;

    /* set default status */
    statusId = CMD_PROCESS_ERROR;

    /* check if the enough data is available to read */
    dataAvailable = tcpSocket.bytesAvailable();

    if (dataAvailable <= 0)
    {
        /* wait for data on socket */
        status = tcpSocket.waitForReadyRead(timeoutMs);

        /* there may be timeout or other error */
        if (status == false)
        {
            /* check for socket errors */
            sockError = tcpSocket.error();

            if (sockError == QAbstractSocket::SocketTimeoutError)
            {
                statusId = CMD_REQUEST_IN_PROGRESS;
            }
            else
            {
                statusId = CMD_SERVER_NOT_RESPONDING;
            }

            return false;
        }
    }

    /* make read bytes as 0 */
    *bytesRead = 0;

    /* read data from scoket */
    chunkSize = tcpSocket.read(writePtr, size);

    if (chunkSize == -1)
    {
        statusId = CMD_SERVER_NOT_RESPONDING;
        return false;
    }

    /* fill read data */
    *bytesRead = chunkSize;

    return true;
}

//*****************************************************************************
//  receiveFrame ()
//      Param:
//          IN : QTcpSocket &tcpSocket      // tcp socket from which the header
//                                          // will be read
//               qint64 frameSize           // size of the frame to read
//          OUT: QByteArray &frame          // output buffer for frame
//	Returns:
//		 bool [true / false]
//      Description:
//          This API reads a frame from the specified tcp socket.
//          If any error occurs, it set status id to appropriate code.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool MediaRequest::receiveFrame (QByteArray &frame,
                                 qint64 size,
                                 QTcpSocket &tcpSocket)
{
    bool        status = true;
    bool        tempStopFlag = false;
    quint8      index = 0;

    qint64      prevChunk = 0;
    qint64      transferredBytes = 0;
    qint64      dataAvailable = 0;

    frame.clear ();

    // loop tll whole frame is not received
    while (transferredBytes < size)
    {
        // check if the data is available to read
        dataAvailable = tcpSocket.bytesAvailable ();

        if (dataAvailable >= 0)
        {
            if (dataAvailable == 0)
            {
                do
                {
                    tempStopFlag = getStopFlag ();

                    if(tempStopFlag == true)
                    {
                        status = false;
                        break;
                    }
                    else
                    {
                        // wait for socket to become readable
                        status = tcpSocket.waitForReadyRead (1 * ONE_MILISEC);

                        if(status == false)
                        {
                            if(!((tcpSocket.error() == QAbstractSocket::ConnectionRefusedError)
                                 || (tcpSocket.error() == QAbstractSocket::SocketTimeoutError)))
                            {
                                break;
                            }
                            else
                            {
                                index++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                while(index < request.timeout);

                if (status == false)
                {
					statusId = CMD_SERVER_NOT_RESPONDING;
                    break;
                }
            }
            else
            {
                prevChunk = transferredBytes;
                // read data from socket and store it to buffer
                frame.append (tcpSocket.read (size - transferredBytes));
                // calculate total number of received bytes
                transferredBytes = frame.size ();

                if(prevChunk == transferredBytes)
                {
                    status = false;
                    statusId = CMD_SERVER_NOT_RESPONDING;
                    break;
                }
            }
        }
        else
        {
            status = false;
            statusId = CMD_SERVER_NOT_RESPONDING;
        }
    }
    return status;
}
//*****************************************************************************
//  receiveHeaderToDirLoc ()
//      Param:
//          IN : QTcpSocket &tcpSocket      // tcp socket from which the header
//                                          // will be read
//          OUT: FRAME_HEADER_t &header     // output buffer for frame header
//	Returns:
//		 bool [true / false]
//      Description:
//          This API reads a header from the specified tcp socket.
//          If any error occurs, it set status id to appropriate code.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool MediaRequest::receiveHeaderToDirLoc (char * writePtr,
                                          QTcpSocket &tcpSocket)
{
    bool status = true;
    FRAME_HEADER_t *headerPtr = NULL;
    qint64 chunkSize = 0;
	size_t transferredBytes = 0;
    qint64 dataAvailable = 0;
    bool        tempStopFlag = false;
    quint8      index = 0;

    // loop till whole header is not received
    while (transferredBytes < sizeof(FRAME_HEADER_t))
    {
        // check if the data is available to read
        dataAvailable = tcpSocket.bytesAvailable ();

        if(dataAvailable >= 0)
        {
            if (dataAvailable == 0)
            {
                do
                {
                    tempStopFlag = getStopFlag ();
                    if(tempStopFlag == true)
                    {
                        status = false;
                        break;
                    }
                    else
                    {
                        // wait for socket to become readable
                        status = tcpSocket.waitForReadyRead (1 * ONE_MILISEC);

                        if(status == false)
                        {
                            if(!((tcpSocket.error() == QAbstractSocket::ConnectionRefusedError)
                                 || (tcpSocket.error() == QAbstractSocket::SocketTimeoutError)))
                            {
                                break;
                            }
                            else
                            {
                                index++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                while(index < request.timeout);

                if(status == false)
                {
                    statusId = CMD_SERVER_NOT_RESPONDING;
                    break;
                }
            }
            else
            {
                // read data from socket
                chunkSize = tcpSocket.read ((writePtr + transferredBytes),
                                            (sizeof(FRAME_HEADER_t) -
                                             transferredBytes));

                // calculate total number of received bytes
                if(chunkSize >= 0)
                {
                    transferredBytes += chunkSize;
                }
                else
                {
                    status = false;
                    statusId = CMD_PROCESS_ERROR;
                    break;
                }
            }
        }
        else
        {
            status = false;
            statusId = CMD_SERVER_NOT_RESPONDING;
            break;
        }
    }

    if(status == true)
    {
        headerPtr = (FRAME_HEADER_t *)writePtr;
        // if magic code in header does not match
        if (headerPtr->magicCode != MediaRequest::magicCode)
        {
            // set status to false
            statusId = CMD_PROCESS_ERROR;
            status = false;
        }
    }
    // return status
    return status;
}

//*****************************************************************************
//  receiveFrameToDirLoc ()
//      Param:
//          IN : QTcpSocket &tcpSocket      // tcp socket from which the header
//                                          // will be read
//               qint64 frameSize           // size of the frame to read
//          OUT: QByteArray &frame          // output buffer for frame
//	Returns:
//		 bool [true / false]
//      Description:
//          This API reads a frame from the specified tcp socket.
//          If any error occurs, it set status id to appropriate code.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool MediaRequest::receiveFrameToDirLoc (char * writePtr,
                                         qint64 size,
                                         QTcpSocket &tcpSocket)
{
    bool status = true;
    qint64 transferredBytes = 0, chunkSize = 0;
    qint64 dataAvailable = 0;
    bool tempStopFlag = false;
    quint8      index = 0;

    // loop tll whole frame is not received
    while (transferredBytes < size)
    {
        // check if the data is available to read
        dataAvailable = tcpSocket.bytesAvailable ();

        if(dataAvailable >= 0)
        {
            if (dataAvailable == 0)
            {
                do
                {
                    tempStopFlag = getStopFlag ();
                    if(tempStopFlag == true)
                    {
                        status = false;
                        break;
                    }
                    else
                    {
                        // wait for socket to become readable
                        status = tcpSocket.waitForReadyRead (1 * ONE_MILISEC);
                        if(status == false)
                        {
                            if(!((tcpSocket.error() == QAbstractSocket::ConnectionRefusedError)
                                 || (tcpSocket.error() == QAbstractSocket::SocketTimeoutError)))
                            {
                                break;
                            }
                            else
                            {
                                index++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                while(index < request.timeout);

                if (status == false)
                {
                    statusId = CMD_SERVER_NOT_RESPONDING;
                    break;
                }
            }
            else
            {
                // read data from socket and store it to buffer
                chunkSize =  tcpSocket.read (writePtr, (size - transferredBytes));
                writePtr += chunkSize;
                // calculate total number of received bytes
                transferredBytes += chunkSize;
            }
        }
        else
        {
            status = false;
            statusId = CMD_SERVER_NOT_RESPONDING;
            break;
        }
    }
    return status;
}


//*****************************************************************************
//  receiveHeaderToDirLoc ()
//      Param:
//          IN : QTcpSocket &tcpSocket      // tcp socket from which the header
//                                          // will be read
//          OUT: FRAME_HEADER_t &header     // output buffer for frame header
//	Returns:
//		 bool [true / false]
//      Description:
//          This API reads a header from the specified tcp socket.
//          If any error occurs, it set status id to appropriate code.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool MediaRequest::receiveHeaderToDirLocSyncPb (char * writePtr,
                                          QTcpSocket &tcpSocket)
{
    bool status = true;
    FRAME_HEADER_t *headerPtr = NULL;
    qint64 chunkSize = 0;
	size_t transferredBytes = 0;
    qint64 dataAvailable = 0;

    // loop till whole header is not received
    while (transferredBytes < sizeof(FRAME_HEADER_t))
    {
        // check if the data is available to read
        dataAvailable = tcpSocket.bytesAvailable ();

        if (dataAvailable == 0)
        {
            // wait for socket to become readable
            status = tcpSocket.waitForReadyRead (SYNC_RECV_HEADER_TIMEOUT);

            // if socket is not readable
            if (status == false)
            {
                break;
            }
        }
        else
        {
            // read data from socket
            chunkSize = tcpSocket.read ((writePtr + transferredBytes),
                                        (sizeof(FRAME_HEADER_t) -
                                         transferredBytes));
            // calculate total number of received bytes
            transferredBytes += chunkSize;
        }
    }

    if(status == true)
    {
        headerPtr = (FRAME_HEADER_t *)writePtr;
        // if magic code in header does not match
        if (headerPtr->magicCode != MediaRequest::magicCode)
        {
            // set status to false
            statusId = CMD_PROCESS_ERROR;
            status = false;
        }
    }
    // return status
    return status;
}

//*****************************************************************************
//  receiveFrameToDirLoc ()
//      Param:
//          IN : QTcpSocket &tcpSocket      // tcp socket from which the header
//                                          // will be read
//               qint64 frameSize           // size of the frame to read
//          OUT: QByteArray &frame          // output buffer for frame
//	Returns:
//		 bool [true / false]
//      Description:
//          This API reads a frame from the specified tcp socket.
//          If any error occurs, it set status id to appropriate code.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool MediaRequest::receiveFrameToDirLocSyncPb (char * writePtr,
                                         qint64 size,
                                         QTcpSocket &tcpSocket)
{
    bool status = true;
    qint64 transferredBytes = 0, chunkSize = 0;
    qint64 dataAvailable = 0;

    // loop tll whole frame is not received
    while (transferredBytes < size)
    {
        // check if the data is available to read
        dataAvailable = tcpSocket.bytesAvailable ();

        if (dataAvailable == 0)
        {
            // wait for socket to become readable
            status = tcpSocket.waitForReadyRead (request.timeout * ONE_MILISEC);

            // if socket is not readable
            if (status == false)
            {
                // set status id to timeout
                statusId = CMD_SERVER_NOT_RESPONDING;

                // break the loop
                break;
            }
        }
        else
        {
            // read data from socket and store it to buffer
            chunkSize =  tcpSocket.read (writePtr, (size - transferredBytes));
            writePtr += chunkSize;
            // calculate total number of received bytes
            transferredBytes += chunkSize;
        }
    }
    return status;
}
