//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		AudioIn.c
@brief      This moudle contains APIs for supporting Audio In feature.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "AudioIn.h"
#include "../DecoderLib/include/DecDispLib.h"

//#################################################################################################
// @DEFINES
//#################################################################################################

//#################################################################################################
// @DATA TYPES
//#################################################################################################

//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//------------------------------------------------------------------------------------------------
/**
 * @brief AudioIn::AudioIn
 * @param serverInfo
 * @param requestInfo
 * @param commandId
 */
AudioIn::AudioIn(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId)
		: MediaRequest(serverInfo, requestInfo, commandId, INVALID_DEC_DISP_PLAY_ID)
{    
    memset(mFrameBuf, 0x00, AUDIO_FRAME_SIZE);
    mFrameLen = 0;

    /* init media header */
    mFrameHeader.magicCode = MediaRequest::magicCode;
    mFrameHeader.version = 1; /* header version 1 */
    mFrameHeader.productType = 1; /* 1-NVR */
    mFrameHeader.frameSize = 0;
    mFrameHeader.seconds = 0;
    mFrameHeader.mSec = 0;
    mFrameHeader.channel = 0;
    mFrameHeader.streamType = STREAM_TYPE_AUDIO;
    mFrameHeader.codecType = AUDIO_G711_ULAW;
    mFrameHeader.frameRate = 0;    
    mFrameHeader.audioSampleFreq = 8000; /* audio sampling rate 8 KHz */
}

//------------------------------------------------------------------------------------------------
/**
 * @brief AudioIn::~AudioIn
 */
AudioIn::~AudioIn()
{
    /* nothing to do */
}

//------------------------------------------------------------------------------------------------
/**
 * @brief   Audio In thread which send SND_AUDIO command to server. On success response initialize
 *          Audio In channel and encoder. Get next frame from the audio channel and send it to server.
 */
void AudioIn::run(void)
{
    qint64      bytesWritten;
    QTcpSocket  tcpSocket;

    /* set thread name */
    prctl(PR_SET_NAME, "AUDIO_IN", 0, 0, 0);

    do
    {
        /* do initial server handshake */
        if (false == DoServerHandshake(tcpSocket))
        {
            break;
        }

        /* if server handshake is successful, start audio in process and create audio channel */
        if (FAIL == StartAudioIn())
        {
            /* emit signal for stream request response */
            emit sigMediaResponse(request.requestId, cmdId, CMD_AUD_SND_REQ_FAIL, request.payload);
            break;
        }

        while(getRunFlag() == true)
        {
            /* get next audio frame from audio channel */
            mFrameLen = AUDIO_FRAME_SIZE;
            if (FAIL == GetNextAudioInFrame(&mFrameBuf[MEDIA_HEADER_SIZE], &mFrameLen))
            {
                EPRINT(STREAM_REQ, "AudioIn: fail to get next audio frame: stop audio in");

                /* set run flag to false */
                setRunFlag(false);

                /* emit signal for stream request response */
                emit sigMediaResponse(request.requestId, cmdId, CMD_AUD_SND_REQ_FAIL, request.payload);
                break;
            }

            /* Update frame size in header and copy header in buffer */
            mFrameHeader.frameSize = (MEDIA_HEADER_SIZE + mFrameLen);
            memcpy(mFrameBuf, &mFrameHeader, MEDIA_HEADER_SIZE);

            /* send audio stream data to server */
            bytesWritten = SendStreamData(tcpSocket);
            if (bytesWritten >= mFrameHeader.frameSize)
            {
                continue;
            }

            EPRINT(STREAM_REQ, "AudioIn: fail to send audio frame: [bytesWritten= %lld]", bytesWritten);

            /* emit signal for stream request response */
            emit sigMediaResponse(request.requestId, cmdId, CMD_PROCESS_ERROR, request.payload);
            break;
        }

        /* stop audio in process and free audio channel */
        StopAudioIn();

    }while(0);

    /* close socket if open */
    if (tcpSocket.isOpen())
    {
        /* close socket */
        tcpSocket.close();
    }    
}

//------------------------------------------------------------------------------------------------
/**
 * @brief   send request to server for SND_AUDIO to start receiving client audio.
 * @param   tcpSocket
 * @return
 */
bool AudioIn::DoServerHandshake(QTcpSocket &tcpSocket)
{
    bool retVal = false;

    do
    {
        /* connect to server */
        if (connectToServer(tcpSocket) == false)
        {
            EPRINT(STREAM_REQ, "AudioIn: cannot connect to server");
            break;
        }

        /* send request to server */
        if (sendRequest(tcpSocket) == false)
        {
            EPRINT(STREAM_REQ, "AudioIn: fail to send request to server");
            break;
        }

        /* received response from server */
        retVal = receiveResponse(tcpSocket, STREAM_RESPONSE_SIZE);

        DPRINT(STREAM_REQ, "AudioIn: SND_AUDIO response: [retVal=%d], [statusId=%d]", retVal, statusId);

        if ((retVal != true) || (statusId != CMD_SUCCESS))
        {
            EPRINT(STREAM_REQ, "AudioIn: cannot start audio stream");
            retVal = false;
        }

    }while(0);

    /* emit signal for stream request response */
    emit sigMediaResponse(request.requestId, cmdId, statusId, request.payload);

    return retVal;
}

//------------------------------------------------------------------------------------------------
/**
 * @brief   AudioIn::SendStreamData
 * @param   tcpSocket
 * @return
 */
qint64 AudioIn::SendStreamData(QTcpSocket &tcpSocket)
{    
    qint64  transferredBytes = 0;
    qint64  chunkSize;

    do
    {
        if (false == tcpSocket.isOpen())
        {
            return transferredBytes;
        }

        chunkSize =  tcpSocket.write((mFrameBuf + transferredBytes), mFrameHeader.frameSize);
        tcpSocket.flush();
        if (chunkSize < 0)
        {            
            break;
        }

        transferredBytes += chunkSize;

    } while (transferredBytes < mFrameHeader.frameSize);

    return transferredBytes;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
