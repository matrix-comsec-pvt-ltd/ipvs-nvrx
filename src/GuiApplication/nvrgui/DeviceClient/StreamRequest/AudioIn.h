#ifndef AUDIOIN_H
#define AUDIOIN_H
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
#include <sys/prctl.h>
#include "MediaRequest.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define AUDIO_FRAME_SIZE    (1024)
#define MEDIA_HEADER_SIZE   sizeof(FRAME_HEADER_t)
//#################################################################################################
// @CLASS
//#################################################################################################
class AudioIn : public MediaRequest
{    
private:
    /* media header for audio frame */
    FRAME_HEADER_t mFrameHeader;

    /* audio frame buffer */
    char mFrameBuf[AUDIO_FRAME_SIZE + MEDIA_HEADER_SIZE];

    /* current frame length */
    INT32 mFrameLen;

public:
    /* constructor */
    AudioIn(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId);

    /* distructor */
    ~AudioIn();

    /* audio in thread which send command to server for two-way audio, send audio frames to server */
    void run(void);

private:
    /* init command request to server */
    bool DoServerHandshake(QTcpSocket &tcpSocket);

    /* send stream data to server */
    qint64 SendStreamData(QTcpSocket &tcpSocket);

};

//#################################################################################################
// @END OF FILE
//#################################################################################################

#endif // AUDIOIN_H
