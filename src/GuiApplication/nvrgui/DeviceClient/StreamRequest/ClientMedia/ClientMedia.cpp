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
//   Project      : NVR ( Network Video Recorder)
//   Owner        : Kaushal Patel
//   File         : ClientMedia.cpp
//   Description  : This module provides APIs to make client audio request.
//                  It receives frame and frame header and feeds it to decoder
//                  module.
//
/////////////////////////////////////////////////////////////////////////////

#include "ClientMedia.h"

//******** Extern Variables **********


//******** Defines and Data Types ****


//******** Function Prototypes *******


//******** Global Variables **********


//******** Static Variables **********


//******** Function Definitions ******

//*****************************************************************************
//  ClientMedia ()
//      Param:
//          IN : SERVER_INFO_t serverInfo   // server informations
//               REQ_INFO_t requestInfo     // request information
//               SET_COMMAND_e commandId    // command index
//          OUT: NONE
//	Returns:
//		Not Applicable
//      Description:
//          This API is constructor of a class LiveMedia.
//          It initializes the newly created object with server, request and
//          windowId information.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
ClientMedia::ClientMedia(SERVER_INFO_t serverInfo,
                      REQ_INFO_t &requestInfo,
                      SET_COMMAND_e commandId,
                      quint8 decoderId,
                      quint8 strmId)
    : MediaRequest (serverInfo,
                    requestInfo,
                    commandId,
                    decoderId), m_streamId(strmId)
{
    setObjectName("CLIENT_MED_"+ QString("%1").arg(m_streamId));
}

//*****************************************************************************
//  ~ClientMedia
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//	Returns:
//		Not Applicable
//      Description:
//          This API is destructor of class LiveMedia.
//          As of now it does no functionality, kept for future use, if needed.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
ClientMedia::~ClientMedia ()
{

}

//*****************************************************************************
//  run ()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          The starting point for the thread. After calling start(), the newly
//          created thread calls this function.
//          This API sends request for live media, receives frames and feed it
//          to the decoder module.
//          It also notifies the live media status like, success, failure and
//          video loss.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ClientMedia::run ()
{
    bool    status = false;
    bool    feedFrame = true;

    QTcpSocket      tcpSocket;
    FRAME_HEADER_t  header;
    QByteArray      frame;
    DECODER_ERROR_e decError = MAX_DEC_ERROR;
    FRAME_INFO_t    frameInfo;

    // connect to server
    status = connectToServer (tcpSocket);

    // IF failed to connect to server
    if (status == true)
    {
        // send request
        status = sendRequest (tcpSocket);

        // IF failed to send request
        if (status == true)
        {
            // receive response
            status = receiveResponse (tcpSocket, STREAM_RESPONSE_SIZE);

            // IF status id is failure
            if ( (status == true) && (statusId == CMD_SUCCESS) )
            {
                // emit signal
                emit sigMediaResponse (request.requestId,
                                       cmdId,
                                       statusId,
                                       request.payload);

                // LOOP till run flag is true

                while (getRunFlag () == true)
                {
                    // IF header not received OR frame not received
                    if (receiveHeader (header, tcpSocket) == false)
                    {
                        // set run flag to false
                        setRunFlag (false);
                    }
                    // ELSE
                    else
                    {
                        if (receiveFrame (frame,
                                          (header.frameSize - sizeof(header)),
                                          tcpSocket) == true)
                        {
                            if (header.streamStatus == STREAM_NORMAL)
                            {
                                switch(header.streamType)
                                {
                                case STREAM_TYPE_AUDIO:
                                    feedFrame = true;
                                    break;

                                default:
                                    feedFrame = false;
                                    break;
                                }
                                // IF feed frame flag is true
                                if ((feedFrame == true) && (getStopFlag() == false))
                                {
                                    // feed frame to decoder
                                    if(header.resolution < MAX_RESOLUTION)
                                    {
                                        frameInfo.framePayload = frame.data ();
                                        frameInfo.frameSize = header.frameSize - sizeof (header);
                                        frameInfo.mediaType = (STREAM_TYPE_e)header.streamType;
                                        frameInfo.codecType = (STREAM_CODEC_TYPE_e)header.codecType;
                                        frameInfo.frameType = (FRAME_TYPE_e)header.frameType;

                                        if((header.version == 2)  && (header.frameRate != 0))
                                            frameInfo.frameRate = header.frameRate;
                                        else
                                            frameInfo.frameRate = DEFAULT_FRAME_RATE;

                                        status = DecodeDispFrame(decId, &frameInfo, &decError);
                                        if (status == false)
                                        {
											if(DEC_ERROR_NO_CAPACITY == decError)
											{
												statusId = CMD_DECODER_CAPACITY_ERROR;
											}
											else
											{
												statusId = CMD_DECODER_ERROR;
											}
                                            setRunFlag (false);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                statusId = CMD_PROCESS_ERROR;
                                setRunFlag (false);
                            }
                        }
                        else
                        {
                            setRunFlag (false);
                        }
                    }
                    // ENDIF
                }
                // ENDLOOP
            }
            // ENDIF
        }
        // ENDIF

        // disconnect from host
        tcpSocket.disconnectFromHost ();

        // close tcp socket
        tcpSocket.close ();
    }
    // ENDIF

    // IF stop stream flag is true
    if (getStopFlag () == true)
    {
        // set status id to stream stopped
        statusId = CMD_STREAM_STOPPED;
    }
    // ENDIF

    // emit signal
    emit sigMediaResponse (request.requestId,
                           cmdId,
                           statusId,
                           request.payload);
}

