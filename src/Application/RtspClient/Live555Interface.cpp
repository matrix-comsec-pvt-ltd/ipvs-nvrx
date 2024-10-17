//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Live555Interface.cpp
@brief      This module is wrapper of live555 opensource RTSP streaming library. Other modules can
            request media stream from IP camera using start stream API. This module supports multiple
            concurrent streams. Media frames when arrived from the camera are passed back to invoking
            module through callback function along with media information.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/resource.h>

/* Application Includes */
#include "RtspClient.h"
#include "Live555Interface.h"
#include "AudioParser.h"

/* Library Includes */
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

//#################################################################################################
// @DEFINES
//#################################################################################################
// by default, print verbose output from each "RTSPClient"
#define VERBOSITY_LEVEL 					(0)
#define MEDIA_COMMAND_TIMEOUT 				(5)
#define MEDIA_PACKET_TIMEOUT				(3)
#define	MEDIA_TIMEOUT_SAMPLE				(1)
#define	OPTION_CMD_SAMPLE_CNT				(10)
#define DUMMY_BUF_SIZE 						(100 * KILO_BYTE)
#define RTP_PORT_PER_CAMERA					(4)
#define RUN_TIME_CONFIG_H264				(1024)

#define LIBNAME								"libRTSP"
#define RTSP_URL							"rtsp://%s:%d%s"
#define RTSP_DEBUG_URL						"[URL:\"%s\"]:"

#define WATCHDOG_THRD_STACK_SIZE            (1 * MEGA_BYTE)
#define MEDIA_THRD_STACK_SIZE               (0 * MEGA_BYTE)

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
    H265_VPS_NALU,
    H265_SPS_NALU,
    H265_PPS_NALU,
    H265_NALU_MAX
}H265_NALU_e;

class StreamClientState
{
    public:
        StreamClientState();
        virtual ~StreamClientState();

    public:
        MediaSubsessionIterator		*iter;
        MediaSession 				*session;
        MediaSubsession 			*subsession;
        UINT32						recvPacketCnt;
        UINT16						packGapCount : 16;
        UINT16						timerCount : 16;
        TaskToken 					checkPacGapTask;
        BOOL						videoSetupFlg;
        BOOL						audioSetupFlg;
        UINT16						rtpPortRange[RTP_PORT_PER_CAMERA];
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:
class ourRTSPClient: public RTSPClient
{
    public:
        static ourRTSPClient *createNew(UsageEnvironment &env, CHAR const *rtspURL, INT32 verbosityLevel = 0,
                                        CHAR const *applicationName = NULL, portNumBits tunnelOverHTTPPortNum = 0,
                                        Boolean streamUsingTCP = False, Authenticator *authMethod = NULL);

    protected:
        ourRTSPClient(UsageEnvironment &env, CHAR const *rtspURL, INT32 verbosityLevel, CHAR const *applicationName,
                      portNumBits tunnelOverHTTPPortNum, Boolean streamUsingTCP, Authenticator *authMethod);

        // called only by createNew();
        virtual ~ourRTSPClient();

    public:
        StreamClientState		scs;
        Boolean 				isInterleaved;
        Authenticator			*ourAuthenticator;
        RTSP_HANDLE             mediaHandle;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.
class AudioStreamSink: public MediaSink
{
    public:
        static AudioStreamSink *createNew(UsageEnvironment &env, MediaSubsession &subsession,
                                          RTSP_HANDLE handle, CHAR const *streamId = NULL);

    private:
        AudioStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~AudioStreamSink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);

    private:
        // redefined virtual functions:
        virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;
        UINT8PTR			fReceiveBuffer;
        UINT8PTR			shmBase;
        INT32				shmFd;
        CHARPTR				fStreamId;
        MediaSubsession 	&fSubsession;
        Boolean 			fHaveWrittenFirstFrame;
};

class DummySink: public MediaSink
{
    public:
        static DummySink *createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId = NULL);

    private:
        DummySink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~DummySink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);

    private:
        // redefined virtual functions:
        virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;
        UINT8PTR			fReceiveBuffer;
        CHARPTR				fStreamId;
        MediaSubsession 	&fSubsession;
};

class H264VideoStreamSink: public MediaSink
{
    public:
        static H264VideoStreamSink *createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId = NULL);

    private:
        H264VideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~H264VideoStreamSink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);

    private:
      // redefined virtual functions:
      virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;
        CHAR const			*fSPropParameterSetsStr;
        Boolean 			fHaveWrittenFirstFrame;
        UINT8				newConfigRecv;
        UINT32				newHeadSize;
        UINT8				newConfig[RUN_TIME_CONFIG_H264];

        UINT8PTR 			fReceiveBuffer;
        INT32				shmFd;
        UINT8PTR			shmBase;

        UINT8PTR 			fReceiveBufferMark;
        UINT32 				fheadSize;
        MediaSubsession 	&fSubsession;
        CHARPTR 			fStreamId;
        UINT16				width;
        UINT16				height;
        UINT8				noOfRefFrame;
};

class H265VideoStreamSink: public MediaSink
{
    public:
        static H265VideoStreamSink *createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,  CHAR const *streamId = NULL);

    private:
        H265VideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~H265VideoStreamSink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);
        BOOL handleH265VCLFrame(UINT32 frameSize, FRAME_TYPE_e frameType, BOOL firstSlice, timeval presentationTime);
        BOOL handleH265NonVCLFrame(UINT32 frameSize, FRAME_TYPE_e frameType);
        BOOL handleCallback(FRAME_TYPE_e frameType, UINT32 frameSize);
        BOOL appendMetaData(UINT32 frameSize);

    private:
      // redefined virtual functions:
      virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;                    // Camera Number
        Boolean 			fHaveWrittenFirstFrame;
        UINT8PTR 			fReceiveBuffer;                 //for storing New NAL units
        UINT8PTR 			fReceiveBufferMark;             //Points to start of new NAL unit
        UINT8PTR 			fIframeMark;                    //Points to pending I frame location
        UINT8PTR            fPframeMark;                    //Points to pending P frame location
        MediaSubsession 	&fSubsession;
        CHARPTR 			fStreamId;
        UINT16				width;
        UINT16				height;
        UINT8				noOfRefFrame;
        UINT8PTR            fMetaData[H265_NALU_MAX];       // stores VPS ,SPS & PPS
        UINT32              fMetaDataSize[H265_NALU_MAX];   // size of VPS SPS PPS
        UINT32              fIsize;                         // pending I frame size (Meta Data+ I frame)
        UINT32              fPsize;                         // pending P frame size
        MEDIA_FRAME_INFO_t 	PmediaInfo;
        MEDIA_FRAME_INFO_t 	ImediaInfo;
        INT32				shmFd;
        UINT8PTR			shmBase;
};

class Mpeg4VideoStreamSink: public MediaSink
{
    public:
        static Mpeg4VideoStreamSink *createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId = NULL);

    private:
        Mpeg4VideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~Mpeg4VideoStreamSink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);

    private:
        // redefined virtual functions:
        virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;
        Boolean 			fHaveWrittenFirstFrame;

        UINT8PTR 			fReceiveBuffer;
        INT32				shmFd;
        UINT8PTR			shmBase;

        UINT8PTR 			fReceiveBufferMark;
        MediaSubsession 	&fSubsession;
        CHARPTR 			fStreamId;
        UINT8PTR			configStr;
        UINT32				configSize;
        UINT8				noOfRefFrame;
        UINT16				width;
        UINT16				height;
};

class AACAudioStreamSink: public MediaSink
{
    public:
        static AACAudioStreamSink *createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId = NULL);

    private:
        AACAudioStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~AACAudioStreamSink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);

    private:
        // redefined virtual functions:
        virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;
        CHAR const			*config;
        UINT8 				configData[20];
        UINT32 				configLen;
        UINT8 				audioObjectType;
        UINT32 				sampleFreq;
        UINT8 				channelConfig;
        Boolean 			fHaveWrittenFirstFrame;
        UINT8PTR 			fReceiveBuffer;
        INT32				shmFd;
        UINT8PTR			shmBase;
        MediaSubsession 	&fSubsession;
        CHARPTR 			fStreamId;
};

class MJPEGVideoStreamSink: public MediaSink
{
    public:
        static MJPEGVideoStreamSink *createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId = NULL);

    private:
        MJPEGVideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId);

        // called only by "createNew()"
        virtual ~MJPEGVideoStreamSink();

        static void afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds);

        void processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds);

    private:
        // redefined virtual functions:
        virtual Boolean continuePlaying();

    private:
        RTSP_HANDLE         mediaHandle;
        UINT8				noOfRefFrame;
        UINT16				width;
        UINT16				height;
        Boolean 			fHaveWrittenFirstFrame;
        UINT8PTR			fReceiveBuffer;
        INT32				shmFd;
        UINT8PTR			shmBase;
        MediaSubsession&	fSubsession;
        CHARPTR				fStreamId;
};

typedef struct
{
	BOOL				sessionState;
	BOOL				frameRec;
	BOOL				terminateFlg;
	BOOL				startCommF;
    ourRTSPClient		*rtspHandle;
    RTSP_CB         	rtspCb;
    RtspStreamInfo_t    rtspInfo;
	pthread_mutex_t		sessParaMutex;
}MEDIA_SESSION_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const UINT8          h264_h265_StartCode[4] = {0x00, 0x00, 0x00, 0x01};
static UsageEnvironment     *sysEnv;
static TaskToken            startCommTaskToken;
static CHAR                 eventLoopWatchVariable;
static pthread_t            rtspThreadId;
static MEDIA_SESSION_t      mediaSession[MEDIA_SESSION_PER_RTSP_APPL];
static BOOL                 rtspHang;
static pthread_mutex_t      rtspWatchDogMutex;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void continueAfterDESCRIBE(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString);
//-------------------------------------------------------------------------------------------------
static void continueAfterSETUP(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString);
//-------------------------------------------------------------------------------------------------
static void continueAfterPLAY(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString);
//-------------------------------------------------------------------------------------------------
static void continueAfterOption(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString);
//-------------------------------------------------------------------------------------------------
static void subsessionAfterPlaying(VOIDPTR clientData);
//-------------------------------------------------------------------------------------------------
static void subsessionByeHandler(VOIDPTR clientData);
//-------------------------------------------------------------------------------------------------
static BOOL openURL(RTSP_HANDLE mediaHandle);
//-------------------------------------------------------------------------------------------------
static void setupNextSubsession(RTSPClient *rtspClient);
//-------------------------------------------------------------------------------------------------
static void shutdownStream(RTSP_HANDLE mediaHandle, MediaFrameResp_e resp, BOOL exitCloseF);
//-------------------------------------------------------------------------------------------------
static VOIDPTR mediaThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static void checkInterPacketGaps(VOIDPTR clientData);
//-------------------------------------------------------------------------------------------------
static void startCommTask(VOIDPTR clientData);
//-------------------------------------------------------------------------------------------------
static void startPacketGapScheculeTask(TaskToken *tokanPtr, UINT32 timeout, VOIDPTR clientData);
//-------------------------------------------------------------------------------------------------
static void stopPacketGepScheculeTask(TaskToken *tokanPtr);
//-------------------------------------------------------------------------------------------------
static VOIDPTR rtspWatchDog(VOIDPTR clientData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes RTSP client. It initializes session table that control
 *          different media session. Should be called once before entering main superloop.
 * @return success or fail
 */
BOOL InitRtspClient(void)
{
    RTSP_HANDLE mediaHandle;

    for(mediaHandle = 0; mediaHandle < MEDIA_SESSION_PER_RTSP_APPL; mediaHandle++)
	{
        mediaSession[mediaHandle].sessionState = FREE;
        mediaSession[mediaHandle].terminateFlg = FALSE;
        mediaSession[mediaHandle].rtspHandle = NULL;
        mediaSession[mediaHandle].rtspCb = NULL;
        mediaSession[mediaHandle].frameRec = FALSE;
        mediaSession[mediaHandle].startCommF = FALSE;
        MUTEX_INIT(mediaSession[mediaHandle].sessParaMutex, NULL);
	}

    eventLoopWatchVariable = 0;
	startCommTaskToken = NULL;

    //create new media session thread which will send RTSP commands to IP camera to request media.
    if (FAIL == Utils_CreateThread(&rtspThreadId, &mediaThread, NULL, JOINABLE_THREAD, MEDIA_THRD_STACK_SIZE))
	{
        EPRINT(RTSP_CLIENT, "Failed to create RTSP media Thread");
        return FAIL;
	}

	rtspHang = TRUE;
    MUTEX_INIT(rtspWatchDogMutex, NULL);
    if (FAIL == Utils_CreateThread(NULL, rtspWatchDog, NULL, DETACHED_THREAD, WATCHDOG_THRD_STACK_SIZE))
    {
        EPRINT(RTSP_CLIENT, "Create rtspWatchDog Thread : [Fail], [Error: %s ]", strerror(errno));
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function De-initializes RTSP client.
 * @return
 */
void DeinitRtspClient(void)
{
    eventLoopWatchVariable = 1;
	pthread_join(rtspThreadId, NULL);
	DPRINT(SYS_LOG, "RTSP Client De-Initialize");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function processes request for starting new media stream. It checks for free session
 *          and then creates new media session using RTSP protocol.
 * @param   mediaHandle
 * @param   rtspInfoPtr
 * @param   callBack
 * @return  If resource is not available then returns failure else returns Success
 */
NET_CMD_STATUS_e StartRtspMedia(RTSP_HANDLE mediaHandle, RtspStreamInfo_t *rtspInfoPtr, RTSP_CB callBack)
{
    if (mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
    {
        EPRINT(RTSP_CLIENT, "invld media handle: [camera=%d], [handle=%d]", rtspInfoPtr->camIndex, mediaHandle);
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);

    if(mediaSession[mediaHandle].sessionState == BUSY)
    {
        MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);
        EPRINT(RTSP_CLIENT, "rtsp session already busy: [camera=%d], [handle=%d]", rtspInfoPtr->camIndex, mediaHandle);
        StopRtspMedia(mediaHandle);
        return CMD_RESOURCE_LIMIT;
    }

    mediaSession[mediaHandle].sessionState = BUSY;
    mediaSession[mediaHandle].terminateFlg = FALSE;
    mediaSession[mediaHandle].startCommF = TRUE;
    MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);

    mediaSession[mediaHandle].frameRec = FALSE;
    mediaSession[mediaHandle].rtspInfo = *rtspInfoPtr;
    mediaSession[mediaHandle].rtspCb = callBack;

    DPRINT(RTSP_CLIENT, "start rtsp media: [camera=%d], [handle=%d]", rtspInfoPtr->camIndex, mediaHandle);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops media stream. If session found than it stops media stream by setting
 *          terminate flag of the session. Media thread for the session periodically checks this
 *          flag and exits thread when terminate flag is set.
 * @param   mediaHandle
 * @return
 */
NET_CMD_STATUS_e StopRtspMedia(RTSP_HANDLE mediaHandle)
{
    if (mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        EPRINT(RTSP_CLIENT, "invld media handle: [handle=%d]", mediaHandle);
        return CMD_PROCESS_ERROR;
    }

    DPRINT(RTSP_CLIENT, "stop rtsp media: [camera=%d], [handle=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, mediaHandle);
    MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);
    mediaSession[mediaHandle].terminateFlg = TRUE;
    MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function which runs schedule for hadling incoming frame.
 * @param   arg
 * @return
 */
static VOIDPTR mediaThread(VOIDPTR arg)
{
    TaskScheduler *sysScheduler;

	THREAD_START("RTSP");

	setpriority(PRIO_PROCESS, PRIO_PROCESS, -1);

	// Begin by setting up our usage environment:
	sysScheduler = BasicTaskScheduler::createNew();
	sysEnv = BasicUsageEnvironment::createNew(*sysScheduler);

    startCommTaskToken = sysEnv->taskScheduler().scheduleDelayedTask (MICRO_SEC_PER_SEC, (TaskFunc*)startCommTask, NULL);

    /* This function call does not return, unless, at some point in time,
     * "eventLoopWatchVariable" gets set to something non-zero. */
	sysEnv->taskScheduler().doEventLoop(&eventLoopWatchVariable);

    /* If you choose to continue the application past this point (i.e., if you comment out the "return 0;"
     * statement above), and if you don't intend to do anything more with the "TaskScheduler" and
     * "UsageEnvironment" objects, then you can also reclaim the (small) memory used by these objects by
     * uncommenting the following code: */
	sysEnv->reclaim();
	sysEnv = NULL;

	delete sysScheduler;
	sysScheduler = NULL;
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief rtspWatchDog
 * @param arg
 * @return
 */
static VOIDPTR rtspWatchDog(VOIDPTR arg)
{
	THREAD_START("WatchDog");

	sleep(10);

    while(eventLoopWatchVariable != 1)
	{
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(rtspWatchDogMutex);
		if(rtspHang == TRUE)
		{
            MUTEX_UNLOCK(rtspWatchDogMutex);
            EPRINT(RTSP_CLIENT, "fail to recv keep alive for 5 second. restarting rtsp now...");
			exit(0);
		}
        else
        {
            rtspHang = TRUE;
            MUTEX_UNLOCK(rtspWatchDogMutex);
            sleep(5);
        }
	}

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is start point of live 555. It will create object of outRTSPClient classs which
 *          internally creates a connection with server. After that it will send DESCIBE command
 *          to get SDP parameters.
 * @param   mediaHandle
 * @return
 */
static BOOL openURL(RTSP_HANDLE mediaHandle)
{
    Authenticator	*ourAuthenticator;
	UINT8			count;
    CHAR 			urlAtCam[MAX_CAMERA_USERNAME_WIDTH + MAX_CAMERA_PASSWORD_WIDTH + IPV6_ADDR_LEN_MAX
                            + sizeof(UINT16) + MAX_CAMERA_URL_WIDTH + 10]; //10 - extra byte
	portNumBits 	tunnelOverHTTPPortNum = 0;
	Boolean 		streamUsingTCP = False;
    CHAR            ipAddressForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

    switch(mediaSession[mediaHandle].rtspInfo.transport)
	{
        default:
            break;

        case HTTP_TUNNELING:
            tunnelOverHTTPPortNum = mediaSession[mediaHandle].rtspInfo.port;
            break;

        case TCP_INTERLEAVED:
            streamUsingTCP = True;
            break;
	}

    PrepareIpAddressForUrl(mediaSession[mediaHandle].rtspInfo.ip, ipAddressForUrl);

	//Construct total url for IP cam
    snprintf(urlAtCam, sizeof(urlAtCam), RTSP_URL, ipAddressForUrl,
             mediaSession[mediaHandle].rtspInfo.port, mediaSession[mediaHandle].rtspInfo.url);

    ourAuthenticator = new Authenticator(mediaSession[mediaHandle].rtspInfo.usrname, mediaSession[mediaHandle].rtspInfo.pswd);

    /* Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object
     * for each stream that we wish to receive (even if more than stream uses the same "rtsp://" URL). */
    mediaSession[mediaHandle].rtspHandle = ourRTSPClient::createNew(*sysEnv, urlAtCam, VERBOSITY_LEVEL, (CHAR const *)LIBNAME,
                                                                    tunnelOverHTTPPortNum, streamUsingTCP, ourAuthenticator);
	if (mediaSession[mediaHandle].rtspHandle == NULL)
	{
		delete ourAuthenticator;
        EPRINT(RTSP_CLIENT, "Failed to create a RTSP client for URL %s:%s", urlAtCam, sysEnv->getResultMsg());
        return FAIL;
	}

    mediaSession[mediaHandle].rtspHandle->mediaHandle = mediaHandle;
    mediaSession[mediaHandle].rtspHandle->scs.checkPacGapTask = NULL;
    mediaSession[mediaHandle].rtspHandle->scs.recvPacketCnt = 0xFFFFFFFF;
    mediaSession[mediaHandle].rtspHandle->scs.packGapCount = 0;
    mediaSession[mediaHandle].rtspHandle->scs.timerCount = OPTION_CMD_SAMPLE_CNT;

    for(count = 0; count < RTP_PORT_PER_CAMERA; count++)
    {
        mediaSession[mediaHandle].rtspHandle->scs.rtpPortRange[count] = 0;
    }

    startPacketGapScheculeTask(&mediaSession[mediaHandle].rtspHandle->scs.checkPacGapTask,
                               MEDIA_COMMAND_TIMEOUT, (VOIDPTR)mediaSession[mediaHandle].rtspHandle);

    /* Next, send a RTSP "DESCRIBE" command, to get a SDP description. Note that this command:
     * like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
     * Instead, the following function call returns immediately, and we handle the RTSP response later,
     * from within the event loop: */
    mediaSession[mediaHandle].rtspHandle->sendDescribeCommand(continueAfterDESCRIBE,
                                                              mediaSession[mediaHandle].rtspHandle->ourAuthenticator);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets SDP parameters and handle the using creating MediaSession object.
 *          After that this function intiate SETUP command.
 * @param   rtspClient
 * @param   resultCode
 * @param   resultString
 * @return
 */
static void continueAfterDESCRIBE(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString)
{
    UsageEnvironment                &env = rtspClient->envir();
    StreamClientState               &scs = ((ourRTSPClient*)rtspClient)->scs;
	MediaSession* session 			= NULL;
    BOOL							status = FAIL;
    UINT16							desiredPortNum = 0;
    UINT8							sessionPortIdx = 0;
	UINT8							mediaHandle;

    mediaHandle = ((ourRTSPClient*)rtspClient)->mediaHandle;
    if (mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        /* free result string */
        if (NULL != resultString)
        {
            delete[] resultString;
        }

        return;
    }

    stopPacketGepScheculeTask(&scs.checkPacGapTask);

    if(resultCode != 0)
    {
        EPRINT(RTSP_CLIENT, RTSP_DEBUG_URL "Failed to get a SDP description %s", rtspClient->url(), resultString);
    }
    else
    {
        // Create a media session object from this SDP description:
        scs.session = MediaSession::createNew(env, resultString);

        if (scs.session == NULL)
        {
            EPRINT(RTSP_CLIENT, RTSP_DEBUG_URL "Failed to create a MediaSession object from the SDP description: %s",
                   rtspClient->url(), env.getResultMsg());
        }
        else if (!scs.session->hasSubsessions())
        {
            EPRINT(RTSP_CLIENT, RTSP_DEBUG_URL "This session has no media subsessions (i.e., no \"m=\" lines)", rtspClient->url());
        }
        else
        {
            if(mediaSession[mediaHandle].rtspInfo.transport == OVER_UDP)
            {
                session = scs.session;

                // Then, setup the "RTPSource"s for the session:
                MediaSubsessionIterator iter(*session);
                MediaSubsession *subsession;

                while ((subsession = iter.next()) != NULL)
                {
                    if(AllocateRtpPort(&desiredPortNum, mediaSession[mediaHandle].rtspInfo.camIndex) == SUCCESS)
                    {
                        subsession->setClientPortNum(desiredPortNum);
                        scs.rtpPortRange[sessionPortIdx++] = desiredPortNum;
                        scs.rtpPortRange[sessionPortIdx++] = (desiredPortNum + 1);
                    }
                }
            }

            /* Then, create and set up our data source objects for the session. We do this by iterating over
             * the session's 'subsessions', calling "MediaSubsession::initiate()", and then sending a RTSP
             * "SETUP" command, on each one. (Each 'subsession' will have its own data source.) */
            scs.iter = new MediaSubsessionIterator(*scs.session);

            // Check again, after the specified delay:
            startPacketGapScheculeTask(&scs.checkPacGapTask, MEDIA_COMMAND_TIMEOUT, rtspClient);
            setupNextSubsession(rtspClient);
            status = SUCCESS;
        }
    }

    /* free result string */
    if (NULL != resultString)
    {
        delete[] resultString;
    }

    if(status == FAIL)
    {
        EPRINT(RTSP_CLIENT, "describe command fail: [camera=%d]", mediaSession[mediaHandle].rtspInfo.camIndex);
        shutdownStream(mediaHandle, RTSP_RESP_CODE_CONNECT_FAIL, TRUE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function call from where we get SDP parameters.
 *          It will send SETUP command for audio and video.
 * @param   rtspClient
 * @return
 */
static void setupNextSubsession(RTSPClient *rtspClient)
{
    StreamClientState   &scs = ((ourRTSPClient*)rtspClient)->scs;
    scs.subsession      = scs.iter->next();

	if(scs.subsession != NULL)
	{
		if (!scs.subsession->initiate())
		{
			// give up on this subsession; go to the next one
			setupNextSubsession(rtspClient);
		}
		else
		{
			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, ((ourRTSPClient*)rtspClient)->isInterleaved, False);
		}
	}
	else
	{
        // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function handles setup response. It will also creates a class for codec type which
 *          gets from setup response for proper handling of frames. This function also send play
 *          request for that media type and also check is there any media type request still pending
 *          then sends its SETUP request.
 * @param   rtspClient
 * @param   resultCode
 * @param   resultString
 * @return
 */
static void continueAfterSETUP(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString)
{
    BOOL                    status = SUCCESS;
    UsageEnvironment        &env = rtspClient->envir();
    StreamClientState       &scs = ((ourRTSPClient*)rtspClient)->scs;
    STREAM_CODEC_TYPE_e		codecType;
    RTSP_HANDLE             mediaHandle;

    mediaHandle = ((ourRTSPClient*)rtspClient)->mediaHandle;
    if(mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        /* free result string */
        if (NULL != resultString)
        {
            delete[] resultString;
        }
        return;
    }

    stopPacketGepScheculeTask(&scs.checkPacGapTask);

    do
    {
        if (resultCode != STATUS_OK)
        {
            EPRINT(RTSP_CLIENT, "SETUP fail: [camera=%d], [url=%s], [media=%s/%s], [cause=%s]",
                   mediaSession[mediaHandle].rtspInfo.camIndex,
                   rtspClient->url(), scs.subsession->mediumName(), scs.subsession->codecName(), env.getResultMsg());
            status = FAIL;
            break;

        }

        DPRINT(RTSP_CLIENT, "SETUP success: [camera=%d], [url=%s], [media=%s/%s]",
               mediaSession[mediaHandle].rtspInfo.camIndex,
               rtspClient->url(), scs.subsession->mediumName(), scs.subsession->codecName());

        /* Having successfully setup the subsession, create a data sink for it, and call "startPlaying()"
         * on it. (This will prepare the data sink to receive data; the actual flow of data from the
         * client won't start happening until later, after we've sent a RTSP "PLAY" command.) */
        if ((strcmp(scs.subsession->mediumName(), "video") == STATUS_OK) && (scs.videoSetupFlg == FALSE))
        {
            scs.videoSetupFlg = TRUE;
            codecType = GetVideoCodec((CHARPTR )scs.subsession->codecName());

            switch(codecType)
            {
                case VIDEO_H264:
                {
                    scs.subsession->sink = H264VideoStreamSink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
                }
                break;

                case VIDEO_MPEG4:
                {
                    scs.subsession->sink = Mpeg4VideoStreamSink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
                }
                break;

                case VIDEO_MJPG:
                {
                    scs.subsession->sink = MJPEGVideoStreamSink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
                }
                break;

                case VIDEO_H265:
                {
                    scs.subsession->sink = H265VideoStreamSink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
                }
                break;

                default:
                {
                    DPRINT(RTSP_CLIENT, "dummy sink: [camera=%d], [url=%s], [media=%s/%s]",
                           mediaSession[mediaHandle].rtspInfo.camIndex,
                           rtspClient->url(), scs.subsession->mediumName(), scs.subsession->codecName());
                    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
                }
                break;
            }
        }
        else if ((strcmp(scs.subsession->mediumName(), "audio") == STATUS_OK) && (scs.audioSetupFlg == FALSE))
        {
            scs.audioSetupFlg = TRUE;

            codecType = GetAudioCodec((CHARPTR) scs.subsession->codecName());

            if (codecType == AUDIO_AAC)
            {
                scs.subsession->sink = AACAudioStreamSink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
            }
            else if ((codecType > AUDIO_CODEC_NONE) && (codecType < MAX_AUDIO_CODEC))
            {
                scs.subsession->sink = AudioStreamSink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
            }
            else
            {
                DPRINT(RTSP_CLIENT, "dummy sink: [camera=%d], [url=%s], [media=%s/%s]",
                       mediaSession[mediaHandle].rtspInfo.camIndex,
                       rtspClient->url(), scs.subsession->mediumName(), scs.subsession->codecName());
                scs.subsession->sink = DummySink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
            }
        }
        else
        {
            DPRINT(RTSP_CLIENT, "dummy sink: [camera=%d], [url=%s], [media=%s/%s]",
                   mediaSession[mediaHandle].rtspInfo.camIndex,
                   rtspClient->url(), scs.subsession->mediumName(), scs.subsession->codecName());
            scs.subsession->sink = DummySink::createNew(env, *scs.subsession, mediaHandle, rtspClient->url());
        }

        /* validate session */
        if (NULL == scs.subsession->sink)
        {
            status = FAIL;
            break;
        }

        DPRINT(RTSP_CLIENT, "created a data sink for the subsession: [url=%s], [mediumName=%s], [codec=%s]",
               rtspClient->url(), scs.subsession->mediumName(), scs.subsession->codecName());

        /* a hack to let subsession handle functions get the "RTSPClient" from the subsession */
        scs.subsession->miscPtr = rtspClient;
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()), subsessionAfterPlaying, scs.subsession);

        /* Also set a handler to be called if a RTCP "BYE" arrives for this subsession */
        if (scs.subsession->rtcpInstance() != NULL)
        {
            scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
        }

    }while(0);

    /* free result string */
    if (NULL != resultString)
    {
        delete[] resultString;
    }

    if (status == FAIL)
    {
        shutdownStream(mediaHandle, RTSP_RESP_CODE_CONNECT_FAIL, TRUE);
    }
    else
    {
        /* Check again, after the specified delay */
        startPacketGapScheculeTask(&scs.checkPacGapTask, MEDIA_COMMAND_TIMEOUT, rtspClient);

        /* Set up the next subsession, if any */
        setupNextSubsession(rtspClient);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief continueAfterPLAY
 * @param rtspClient
 * @param resultCode
 * @param resultString
 * @return
 */
static void continueAfterPLAY(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString)
{
    BOOL                status = FAIL;
    StreamClientState   &scs = ((ourRTSPClient*)rtspClient)->scs;
    RTSP_HANDLE         mediaHandle;
    MediaFrameResp_e    mediaResp;

    mediaHandle = ((ourRTSPClient*)rtspClient)->mediaHandle;

    if(mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        /* free result string */
        if (NULL != resultString)
        {
            delete[] resultString;
        }
        return;
    }

    stopPacketGepScheculeTask(&scs.checkPacGapTask);

    if (resultCode != 0)
    {
        EPRINT(RTSP_CLIENT, RTSP_DEBUG_URL "Failed to start playing session result [%s]", rtspClient->url(), resultString);

        MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);

        if(mediaSession[mediaHandle].terminateFlg == TRUE)
        {
            mediaResp = RTSP_RESP_CODE_CONN_CLOSE;
        }
        else
        {
            mediaResp = RTSP_RESP_CODE_CONNECT_FAIL;
        }

        MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);
    }
    else
    {
        MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);

        if(mediaSession[mediaHandle].terminateFlg == TRUE)
        {
            mediaResp = RTSP_RESP_CODE_CONN_CLOSE;
        }
        else
        {
            status = SUCCESS;
        }

        MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);
    }

    /* free result string */
    if (NULL != resultString)
    {
        delete[] resultString;
    }

    if (status == FAIL)
    {
        shutdownStream(mediaHandle, mediaResp, TRUE);
    }
    else
    {
        // Check again, after the specified delay:
        startPacketGapScheculeTask(&scs.checkPacGapTask, MEDIA_PACKET_TIMEOUT, rtspClient);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief continueAfterOption
 * @param rtspClient
 * @param resultCode
 * @param resultString
 * @return
 */
static void continueAfterOption(RTSPClient *rtspClient, INT32 resultCode, CHARPTR resultString)
{
    StreamClientState   &scs = ((ourRTSPClient*)rtspClient)->scs;
    RTSP_HANDLE			mediaHandle;

    mediaHandle = ((ourRTSPClient*)rtspClient)->mediaHandle;
    if (mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        /* free result string */
        if (NULL != resultString)
        {
            delete []resultString;
        }
        return;
    }

    if (resultCode != STATUS_OK)
    {
        EPRINT(RTSP_CLIENT, RTSP_DEBUG_URL "Failed response from option result [%s]", rtspClient->url(), resultString);
    }
    else
    {
        if(strstr(resultString, "GET_PARAMETER") != NULL)
        {
            mediaSession[mediaHandle].rtspHandle->sendGetParameterCommand(*scs.session, NULL, NULL);
        }
    }

    /* free result string */
    if (NULL != resultString)
    {
        delete[] resultString;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is event handler function. Any error occurs from server side this function executes
 *          and stops playing its session.(If server send BYE command on timeout then this function
 *          stops play RTSP session).
 * @param   clientData
 * @return
 */
static void subsessionAfterPlaying(VOIDPTR clientData)
{
    MediaSubsession     *subsession = (MediaSubsession*)clientData;
    RTSPClient			*rtspClient = (RTSPClient*)(subsession->miscPtr);
    RTSP_HANDLE			mediaHandle;
    MediaFrameResp_e    mediaResp;

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
    MediaSession 			&session = subsession->parentSession();
	MediaSubsessionIterator iter(session);

	while ((subsession = iter.next()) != NULL)
	{
		if (subsession->sink != NULL)
		{
			return; // this subsession is still active
		}
	}

    mediaHandle = ((ourRTSPClient*)rtspClient)->mediaHandle;

    if(mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        return;
    }

    EPRINT(RTSP_CLIENT, "after playing command fail: [camera=%d]", mediaSession[mediaHandle].rtspInfo.camIndex);
    if(mediaSession[mediaHandle].frameRec == TRUE)
    {
        mediaResp = RTSP_RESP_CODE_FRAME_TIMEOUT;
    }
    else
    {
        mediaResp = RTSP_RESP_CODE_CONNECT_FAIL;
    }

    shutdownStream(mediaHandle, mediaResp, TRUE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes when we received "BYE" response on RTCP side. It means server not
 *          happy with this connection and try to regret us. So, In this case we need to close the
 *          session by calling subsessionAfterPlaying.
 * @param   clientData
 * @return
 */
static void subsessionByeHandler(VOIDPTR clientData)
{
	MediaSubsession * subsession = (MediaSubsession*)clientData;
	RTSPClient 		* rtspClient = (RTSPClient*)subsession->miscPtr;

    EPRINT(RTSP_CLIENT, RTSP_DEBUG_URL "Received RTCP BYE on %s/%s subsession", rtspClient->url(),
           subsession->mediumName(), subsession->codecName());

	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This sends teardown command to server.
 * @param   mediaHandle
 * @param   resp
 * @param   exitCloseF
 * @return
 */
static void shutdownStream(RTSP_HANDLE mediaHandle, MediaFrameResp_e resp, BOOL exitCloseF)
{
    UINT8 cnt;

    DPRINT(RTSP_CLIENT, "shutdown rtsp media: [camera=%d]", mediaSession[mediaHandle].rtspInfo.camIndex);

    mediaSession[mediaHandle].rtspCb(resp, mediaHandle, NULL, mediaSession[mediaHandle].rtspInfo.camIndex, 0, 0);

	if(exitCloseF == TRUE)
	{
        RTSPClient 			*rtspClient = mediaSession[mediaHandle].rtspHandle;
		StreamClientState& 	scs = mediaSession[mediaHandle].rtspHandle->scs;

		do
		{
			stopPacketGepScheculeTask(&scs.checkPacGapTask);

			// First, check whether any subsessions have still to be closed:
			if (scs.session != NULL)
			{
				Boolean someSubsessionsWereActive = False;
				MediaSubsessionIterator iter(*scs.session);
				MediaSubsession * subsession;

				while ((subsession = iter.next()) != NULL)
				{
					if (subsession->sink != NULL)
					{
						Medium::close(subsession->sink);
						subsession->sink = NULL;

						if (subsession->rtcpInstance() != NULL)
						{
                            /* in case the server sends a RTCP "BYE" while handling "TEARDOWN" */
							subsession->rtcpInstance()->setByeHandler(NULL, NULL);
						}
						someSubsessionsWereActive = TRUE;
					}
				}

				if (someSubsessionsWereActive)
				{
                    /* Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
                     * Don't bother handling the response to the "TEARDOWN". */
                    rtspClient->sendTeardownCommand(*scs.session, NULL, ((ourRTSPClient*)rtspClient)->ourAuthenticator);
				}

                if(mediaSession[mediaHandle].rtspInfo.transport == OVER_UDP)
				{
					for(cnt = 0; cnt < RTP_PORT_PER_CAMERA; cnt += 2)
					{
                        DeallocRtpPort(&scs.rtpPortRange[cnt], mediaSession[mediaHandle].rtspInfo.camIndex);
					}
				}
			}
			Medium::close(rtspClient);
		}
		while(0);
	}

    MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);
	mediaSession[mediaHandle].sessionState = FREE;
    MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of ourRTSP client object.
 * @param env
 * @param rtspURL
 * @param verbosityLevel
 * @param applicationName
 * @param tunnelOverHTTPPortNum
 * @param streamUsingTCP
 * @param authMethod
 * @return
 */
ourRTSPClient *ourRTSPClient::createNew(UsageEnvironment &env, CHAR const *rtspURL, INT32 verbosityLevel,  CHAR const *applicationName,
                                        portNumBits tunnelOverHTTPPortNum, Boolean streamUsingTCP, Authenticator *authMethod)
{
    return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, streamUsingTCP, authMethod);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of ourRTSP client object.
 * @param env
 * @param rtspURL
 * @param verbosityLevel
 * @param applicationName
 * @param tunnelOverHTTPPortNum
 * @param streamUsingTCP
 * @param authMethod
 */
ourRTSPClient::ourRTSPClient(UsageEnvironment &env, CHAR const * rtspURL, INT32 verbosityLevel, CHAR const * applicationName,
                             portNumBits tunnelOverHTTPPortNum, Boolean streamUsingTCP, Authenticator * authMethod) :
                             RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
    mediaHandle = MEDIA_SESSION_PER_RTSP_APPL;
	isInterleaved = streamUsingTCP;
	ourAuthenticator = authMethod;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is de-constructor of ourRTSP client object.
 */
ourRTSPClient::~ourRTSPClient()
{
	// Finally, shut down our client:
	delete ourAuthenticator;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StreamClientState::StreamClientState
 */
StreamClientState::StreamClientState() : iter(NULL), session(NULL), subsession(NULL)
{
    for(UINT8 index = 0; index < RTP_PORT_PER_CAMERA; index++)
    {
        rtpPortRange[index] = 0;
    }

    packGapCount = 0;
    checkPacGapTask = NULL;
    timerCount = 0;
    recvPacketCnt = 0;
	videoSetupFlg = FALSE;
	audioSetupFlg = FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StreamClientState::~StreamClientState
 */
StreamClientState::~StreamClientState()
{
	videoSetupFlg = FALSE;
	audioSetupFlg = FALSE;

	delete iter;

	if (session != NULL)
	{
		Medium::close(session);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of StreamSink client object. This class used for other than AAC audio codec
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 * @return
 */
AudioStreamSink* AudioStreamSink::createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId)
{
    return new AudioStreamSink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of StreamSink client object.
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 */
AudioStreamSink::AudioStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,
                                 CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
    fStreamId               = strDup(streamId);
    mediaHandle             = handle;
    fHaveWrittenFirstFrame  = False;

	shmBase 				= NULL;
	fReceiveBuffer			= NULL;

    if (CreateSharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_AUDIO) == TRUE)
	{
        fReceiveBuffer 	= (shmBase + sizeof(MEDIA_FRAME_INFO_t));
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is de-constructor of StreamSink client object.
 */
AudioStreamSink::~AudioStreamSink()
{
	if (shmBase != NULL)
	{
        DestroySharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_AUDIO);
	}

	delete[] fStreamId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP.
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void AudioStreamSink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                        struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	AudioStreamSink * sink = (AudioStreamSink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP. And process the frame and pass it to camera interface by callback.
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void AudioStreamSink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes,  struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    MediaFrameResp_e    mediaResp;
    MEDIA_FRAME_INFO_t  mediaInfo;

	mediaInfo.isRTSP = TRUE;
	memcpy(&mediaInfo.avPresentationTime,&presentationTime,sizeof(timeval));

	if((((presentationTime.tv_sec)*1000) + ((presentationTime.tv_usec)/1000)) == 0)
	{
		mediaInfo.isRTSP = FALSE;
	}

	if(!fHaveWrittenFirstFrame)
	{
		fHaveWrittenFirstFrame = True;
		mediaResp = RTSP_RESP_CODE_CONFIG_AUDIO_DATA;
	}
	else
	{
		mediaResp = RTSP_RESP_CODE_AUDIO_DATA;
	}

	mediaInfo.codecType = GetAudioCodec((CHARPTR)fSubsession.codecName());
	mediaInfo.sampleRate = 8000;
	mediaInfo.len = frameSize;
	mediaInfo.videoInfo.frameType = MAX_FRAME_TYPE;

	memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

    mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, 0, 0);

	// Then continue, to request the next frame of data:
	continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function gets next frame from RTP layer.
 * @return
 */
Boolean AudioStreamSink::continuePlaying()
{
	if ((fSource == NULL) || (fReceiveBuffer == NULL))
	{
		return False; // sanity check (should not happen)
	}

    // Request the next frame of data from our input source. "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, AUDIO_BUF_SIZE_MEDIA, afterGettingFrame, this, onSourceClosure, this);
	return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DummySink::createNew
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 * @return
 */
DummySink* DummySink::createNew(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId)
{
    return new DummySink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DummySink::DummySink
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 */
DummySink::DummySink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,
                     CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
	fStreamId = strDup(streamId);
	fReceiveBuffer = new UINT8 [DUMMY_BUF_SIZE];
    mediaHandle = handle;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DummySink::~DummySink
 */
DummySink::~DummySink()
{
	delete[] fReceiveBuffer;
	delete[] fStreamId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP.
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void DummySink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                  struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    DummySink *sink = (DummySink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP. And process the frame and pass it to camera interface by callback.
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void DummySink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes, struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	// Then continue, to request the next frame of data:
	continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function gets next frame from RTP layer.
 * @return
 */
Boolean DummySink::continuePlaying()
{
	if ( fSource == NULL)
	{
		return False; // sanity check (should not happen)
	}

    // Request the next frame of data from our input source. "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, DUMMY_BUF_SIZE, afterGettingFrame, this, onSourceClosure, this);
	return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of AACAudioStreamSink client object. This class used for AAC audio codec
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 * @return
 */
AACAudioStreamSink* AACAudioStreamSink::createNew(UsageEnvironment &env, MediaSubsession &subsession,
                                                  RTSP_HANDLE handle, CHAR const *streamId)
{
    return new AACAudioStreamSink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of AACAudioStreamSink client object.
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 */
AACAudioStreamSink::AACAudioStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,
                                       CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
    UINT32              configSize;
    UINT8PTR            configStr = NULL;
    AAC_AUDIO_INFO_t	audioInfo;

    channelConfig           = 0;
	fStreamId 				= strDup(streamId);
    mediaHandle             = handle;
    fHaveWrittenFirstFrame  = False;
	config 					= subsession.fmtp_config();
	configLen 				= 0;
    audioObjectType         = 0;

	shmBase 				= NULL;
	shmFd 					= INVALID_FILE_FD;
	fReceiveBuffer			= NULL;

    memset(configData, 0, sizeof(configData));

    if (CreateSharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_AUDIO) == TRUE)
	{
        fReceiveBuffer 	= (shmBase + sizeof(MEDIA_FRAME_INFO_t));
	}

	configStr = parseGeneralConfigStr((char const *)config, configSize);
	if (configStr != NULL)
	{
        if (SUCCESS == GetAACAudioInfo(configStr, configSize, &audioInfo))
        {
            memcpy((VOIDPTR)configData, (const VOIDPTR)audioInfo.aacHeader, audioInfo.aacHeaderSize);
            configLen = audioInfo.aacHeaderSize;
            sampleFreq = audioInfo.samplingFreq;
        }

		delete[] configStr;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is de-constructor of AACAudioStreamSink client object.
 */
AACAudioStreamSink::~AACAudioStreamSink()
{
	if (shmBase != NULL)
	{
        DestroySharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_AUDIO);
	}

	delete[] fStreamId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP.
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void AACAudioStreamSink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                           struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	AACAudioStreamSink* sink = (AACAudioStreamSink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP. And process the frame and pass it to camera interface by callback.
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void AACAudioStreamSink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes,
                                      struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    MediaFrameResp_e 		mediaResp;
	MEDIA_FRAME_INFO_t 		mediaInfo;

	mediaInfo.isRTSP = TRUE;
	memcpy(&mediaInfo.avPresentationTime,&presentationTime,sizeof(timeval));

	if((((presentationTime.tv_sec)*1000) + ((presentationTime.tv_usec)/1000)) == 0)
	{
		mediaInfo.isRTSP = FALSE;
	}

	if(!fHaveWrittenFirstFrame)
	{
		fHaveWrittenFirstFrame = True;
		mediaResp = RTSP_RESP_CODE_CONFIG_AUDIO_DATA;
	}
	else
	{
		mediaResp = RTSP_RESP_CODE_AUDIO_DATA;
	}

	mediaInfo.codecType = AUDIO_AAC;
	mediaInfo.sampleRate = sampleFreq;
	mediaInfo.len = (frameSize + configLen);
	mediaInfo.videoInfo.frameType = MAX_FRAME_TYPE;

	if(ApendAACFrameLen(fReceiveBuffer, mediaInfo.len) == SUCCESS)
	{
		memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

        mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, 0, 0);
	}

	// Then continue, to request the next frame of data:
	continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function gets next frame from RTP layer.
 * @return
 */
Boolean AACAudioStreamSink::continuePlaying()
{
	if((fSource == NULL) || (fReceiveBuffer == NULL))
	{
		return False; // sanity check (should not happen)
	}

    // Request the next frame of data from our input source. "afterGettingFrame()" will get called later, when it arrives:
	memcpy((VOIDPTR)fReceiveBuffer, (const VOIDPTR)configData, configLen);
    fSource->getNextFrame((fReceiveBuffer + configLen), (AUDIO_BUF_SIZE_MEDIA - configLen),
                          afterGettingFrame, this, onSourceClosure, this);
	return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of Mpeg4VideoStreamSink client object. This class used for MPEG4 video codec
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 * @return
 */
Mpeg4VideoStreamSink* Mpeg4VideoStreamSink::createNew(UsageEnvironment &env, MediaSubsession &subsession,
                                                      RTSP_HANDLE handle, CHAR const *streamId)
{
    return new Mpeg4VideoStreamSink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of Mpeg4VideoStreamSink client object.
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 */
Mpeg4VideoStreamSink::Mpeg4VideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,
                                           CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
	fStreamId 				= strDup(streamId);
    mediaHandle             = handle;
	fHaveWrittenFirstFrame 	= False;

    configStr               = parseGeneralConfigStr(subsession.fmtp_config(), configSize);
	width 					= 0;
	height 					= 0;
	noOfRefFrame 			= 0;
    configSize              = 0;

	shmBase 				= NULL;
	fReceiveBuffer			= NULL;
	shmFd 					= INVALID_FILE_FD;
	fReceiveBufferMark		= NULL;

    if (CreateSharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO) == TRUE)
	{
        fReceiveBuffer 		= (shmBase + sizeof(MEDIA_FRAME_INFO_t));
		fReceiveBufferMark 	= fReceiveBuffer;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is de-constructor of Mpeg4VideoStreamSink client object.
 */
Mpeg4VideoStreamSink::~Mpeg4VideoStreamSink()
{
	delete[] fStreamId;
	delete[] configStr;

    if (shmBase != NULL)
	{
        DestroySharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP.
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void Mpeg4VideoStreamSink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                             struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	Mpeg4VideoStreamSink * sink = (Mpeg4VideoStreamSink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP. And process the frame and pass it to camera interface by callback.
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void Mpeg4VideoStreamSink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes,
                                        struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    UINT8               configPresent = FALSE;
    MediaFrameResp_e    mediaResp;
    MEDIA_FRAME_INFO_t  mediaInfo;

    mediaInfo.videoInfo.width = 0;
    mediaInfo.videoInfo.height = 0;
    mediaInfo.videoInfo.noOfRefFrame = 0;

    if ((numTruncatedBytes == 0) && (fReceiveBuffer != NULL))
	{
		mediaInfo.codecType = VIDEO_MPEG4;
		mediaInfo.sampleRate = 25;
		mediaInfo.isRTSP = TRUE;
		memcpy(&mediaInfo.avPresentationTime,&presentationTime,sizeof(timeval));

        if ((((presentationTime.tv_sec)*1000) + ((presentationTime.tv_usec)/1000)) == 0)
		{
			mediaInfo.isRTSP = FALSE;
		}

        if (!fHaveWrittenFirstFrame)
		{
			fHaveWrittenFirstFrame = True;
			mediaSession[mediaHandle].frameRec = TRUE;

			mediaResp = RTSP_RESP_CODE_CONFIG_VIDEO_DATA;
			mediaInfo.len = configSize;
            GetMpeg4Info(fReceiveBuffer, mediaInfo.len, &mediaInfo.videoInfo, &configPresent);
			width = mediaInfo.videoInfo.width;
			height = mediaInfo.videoInfo.height;

			memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

            mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, configSize, 0);
		}

		mediaResp = RTSP_RESP_CODE_VIDEO_DATA;
		mediaInfo.len = frameSize;

        if(GetMpeg4Info((fReceiveBuffer + configSize), frameSize, &mediaInfo.videoInfo, &configPresent) == SUCCESS)
		{
			if ( mediaInfo.videoInfo.frameType < MAX_FRAME_TYPE)
			{
                if ((mediaInfo.videoInfo.frameType == I_FRAME) && (configPresent == FALSE))
				{
					mediaInfo.len = (frameSize + configSize);
				}
				else
				{
					mediaInfo.len = frameSize;
				}

				mediaInfo.videoInfo.width = width;
				mediaInfo.videoInfo.height = height;
				mediaInfo.videoInfo.noOfRefFrame = noOfRefFrame;

				memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

                mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, configSize, 0);
			}
			else
			{
				width = mediaInfo.videoInfo.width;
				height = mediaInfo.videoInfo.height;
				noOfRefFrame = mediaInfo.videoInfo.noOfRefFrame;
			}
		}
	}
	else
	{
        EPRINT(RTSP_CLIENT, "frame data truncated: [camera=%d], [bytes=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, numTruncatedBytes);
		mediaInfo.isRTSP = FALSE;
	}

	// Then continue, to request the next frame of data:
	continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function gets next frame from RTP layer.
 * @return
 */
Boolean Mpeg4VideoStreamSink::continuePlaying()
{
	if ((fSource == NULL) || (fReceiveBuffer == NULL))
	{
		return False; // sanity check (should not happen)
	}

	if (!fHaveWrittenFirstFrame)
	{
		fReceiveBufferMark = fReceiveBuffer;

        // If we have PPS/SPS NAL units encoded in a "sprop parameter string", prepend these to the file:
		memcpy((VOIDPTR)fReceiveBufferMark, (const VOIDPTR)configStr, configSize);
	}

	fReceiveBufferMark = fReceiveBuffer + configSize;

    // Request the next frame of data from our input source. "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBufferMark, (VIDEO_BUF_SIZE_MEDIA - configSize), afterGettingFrame, this, onSourceClosure, this);
	return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of MJPEGVideoStreamSink client object. This class used for MJPEG video codec
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 * @return
 */
MJPEGVideoStreamSink* MJPEGVideoStreamSink::createNew(UsageEnvironment &env, MediaSubsession &subsession,
                                                      RTSP_HANDLE handle, char const *streamId)
{
    return new MJPEGVideoStreamSink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of MJPEGVideoStreamSink client object.
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 */
MJPEGVideoStreamSink::MJPEGVideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,
                                           CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
	fStreamId 				= strDup(streamId);
    mediaHandle             = handle;
	fHaveWrittenFirstFrame 	= False;
	width 					= subsession.videoWidth();
	height 					= subsession.videoHeight();
	noOfRefFrame 			= 0;

	shmBase 				= NULL;
	fReceiveBuffer			= NULL;
	shmFd 					= INVALID_FILE_FD;

    if (CreateSharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO) == TRUE)
	{
        fReceiveBuffer 		= (shmBase + sizeof(MEDIA_FRAME_INFO_t));
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is de-constructor of MJPEGVideoStreamSink client object.
 */
MJPEGVideoStreamSink::~MJPEGVideoStreamSink()
{
	delete[] fStreamId;

    if (shmBase != NULL)
	{
        DestroySharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP.
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void MJPEGVideoStreamSink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                             struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	MJPEGVideoStreamSink* sink = (MJPEGVideoStreamSink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP. And process the frame and pass it to camera interface by callback.
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void MJPEGVideoStreamSink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes,
                                        struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    MediaFrameResp_e    mediaResp;
    MEDIA_FRAME_INFO_t  mediaInfo;

    mediaInfo.videoInfo.noOfRefFrame = 0;

	if ((numTruncatedBytes == 0) && (fReceiveBuffer != NULL))
	{
		mediaInfo.codecType = VIDEO_MJPG;
		mediaInfo.len = frameSize;
		mediaInfo.sampleRate = 25;
		mediaInfo.isRTSP = TRUE;
		memcpy(&mediaInfo.avPresentationTime,&presentationTime,sizeof(timeval));

		if((((presentationTime.tv_sec)*1000) + ((presentationTime.tv_usec)/1000)) == 0)
		{
			mediaInfo.isRTSP = FALSE;
		}

		if(!fHaveWrittenFirstFrame)
		{
			fHaveWrittenFirstFrame = True;
			mediaSession[mediaHandle].frameRec = TRUE;

			mediaResp = RTSP_RESP_CODE_CONFIG_VIDEO_DATA;
			GetJpegSize(fReceiveBuffer, mediaInfo.len, &mediaInfo.videoInfo);

			memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

            mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, 0, 0);
		}

		mediaResp = RTSP_RESP_CODE_VIDEO_DATA;

        if(GetJpegSize(fReceiveBuffer, mediaInfo.len, &mediaInfo.videoInfo) == SUCCESS)
		{
			if(mediaInfo.videoInfo.frameType < MAX_FRAME_TYPE)
			{
				mediaInfo.videoInfo.noOfRefFrame = noOfRefFrame;

				memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

                mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, 0, 0);
			}
		}
	}
	else
	{
		mediaInfo.isRTSP = FALSE;
        EPRINT(RTSP_CLIENT, "frame data truncated: [camera=%d], [bytes=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, numTruncatedBytes);
	}

	// Then continue, to request the next frame of data:
	continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function gets next frame from RTP layer.
 * @return
 */
Boolean MJPEGVideoStreamSink::continuePlaying()
{
	if ((fSource == NULL) || (fReceiveBuffer == NULL))
	{
		return False; // sanity check (should not happen)
	}

    // Request the next frame of data from our input source. "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, VIDEO_BUF_SIZE_MEDIA, afterGettingFrame, this, onSourceClosure, this);
	return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of H264VideoStreamSink client object. This class used for H264 video codec
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 * @return
 */
H264VideoStreamSink* H264VideoStreamSink::createNew(UsageEnvironment &env, MediaSubsession &subsession,
                                                    RTSP_HANDLE handle, CHAR const *streamId)
{
    return new H264VideoStreamSink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is constructor of H264VideoStreamSink client object.
 * @param env
 * @param subsession
 * @param handle
 * @param streamId
 */
H264VideoStreamSink::H264VideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession,  RTSP_HANDLE handle,
                                         CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
    for(UINT32 index = 0; index < RUN_TIME_CONFIG_H264; index++)
    {
        newConfig[index]    = 0;
    }
	fStreamId 				= strDup(streamId);

    mediaHandle             = handle;
	fHaveWrittenFirstFrame 	= False;
	fSPropParameterSetsStr 	= subsession.fmtp_spropparametersets();
	fheadSize				= 0;
	width					= 0;
	height					= 0;
	noOfRefFrame			= 0;
	newConfigRecv			= FALSE;
	newHeadSize				= 0;
	shmBase 				= NULL;
	fReceiveBuffer			= NULL;
	shmFd 					= INVALID_FILE_FD;

    if (CreateSharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO) == TRUE)
	{
        fReceiveBuffer 		= (shmBase + sizeof(MEDIA_FRAME_INFO_t));
		fReceiveBufferMark 	= fReceiveBuffer;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief It is de-constructor of H264VideoStreamSink client object.
 */
H264VideoStreamSink::~H264VideoStreamSink()
{
	delete[] fStreamId;

	if(shmBase != NULL)
	{
        DestroySharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP.
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void H264VideoStreamSink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                            struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	H264VideoStreamSink* sink = (H264VideoStreamSink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function received RAW frames from RTP. And process the frame and pass it to camera interface by callback.
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void H264VideoStreamSink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes,
                                       struct timeval presentationTime, UINT32 durationInMicroseconds)
{
	UINT8					configPresent = FALSE;
	UINT32					configLen = 0;
    MediaFrameResp_e 		mediaResp;
	MEDIA_FRAME_INFO_t 		mediaInfo;

    mediaInfo.videoInfo.width = 0;
    mediaInfo.videoInfo.height = 0;
    mediaInfo.videoInfo.noOfRefFrame = 0;

	if ((numTruncatedBytes == 0) && (fReceiveBuffer != NULL))
	{
		mediaInfo.codecType = VIDEO_H264;
		mediaInfo.sampleRate = 25;
		mediaInfo.isRTSP = TRUE;
		memcpy(&mediaInfo.avPresentationTime,&presentationTime,sizeof(timeval));

		if((((presentationTime.tv_sec)*1000) + ((presentationTime.tv_usec)/1000)) == 0)
		{
			mediaInfo.isRTSP = FALSE;
		}

		if(!fHaveWrittenFirstFrame)
		{
			fHaveWrittenFirstFrame = True;
			mediaSession[mediaHandle].frameRec = TRUE;

			if(fheadSize > 0)
			{
				mediaInfo.len = fheadSize;

				mediaResp = RTSP_RESP_CODE_CONFIG_VIDEO_DATA;
                GetH264Info(fReceiveBuffer, mediaInfo.len, &mediaInfo.videoInfo, &configPresent);
				width = mediaInfo.videoInfo.width;
				height = mediaInfo.videoInfo.height;
				noOfRefFrame = mediaInfo.videoInfo.noOfRefFrame;

				memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

                mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, fheadSize, 0);
			}
		}

		mediaResp = RTSP_RESP_CODE_VIDEO_DATA;

        if (GetH264Info((fReceiveBuffer + fheadSize), (frameSize + sizeof(h264_h265_StartCode)), &mediaInfo.videoInfo, &configPresent) == SUCCESS)
		{
			if(mediaInfo.videoInfo.frameType < SPS_FRAME)
			{
                if( (mediaInfo.videoInfo.frameType == I_FRAME) && (configPresent == FALSE) )
				{
                    mediaInfo.len = (frameSize + fheadSize + sizeof(h264_h265_StartCode));
				}
				else
				{
                    mediaInfo.len = (frameSize + sizeof(h264_h265_StartCode));
				}

				mediaInfo.videoInfo.width = width;
				mediaInfo.videoInfo.height = height;
				mediaInfo.videoInfo.noOfRefFrame = noOfRefFrame;

				memcpy(shmBase,&mediaInfo,sizeof(MEDIA_FRAME_INFO_t));

                mediaSession[mediaHandle].rtspCb(mediaResp, mediaHandle, &mediaInfo, mediaSession[mediaHandle].rtspInfo.camIndex, fheadSize, 0);
			}
			else
			{
				if( mediaInfo.videoInfo.frameType == SPS_FRAME )
				{
                    configLen = (frameSize + sizeof(h264_h265_StartCode) );
					if(configLen > RUN_TIME_CONFIG_H264)
					{
						configLen = RUN_TIME_CONFIG_H264;
                        EPRINT(RTSP_CLIENT, "h264 frame config: [camera=%d], [frameSize=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, frameSize);
					}

					memcpy(newConfig, (fReceiveBuffer + fheadSize), configLen);

					newHeadSize = configLen;

					newConfigRecv = TRUE;

					width = mediaInfo.videoInfo.width;
					height = mediaInfo.videoInfo.height;
					noOfRefFrame = mediaInfo.videoInfo.noOfRefFrame;
				}
				else if(newConfigRecv == TRUE)
				{
					configLen = ( RUN_TIME_CONFIG_H264 - newHeadSize);

                    if(configLen < (frameSize + sizeof(h264_h265_StartCode) ) )
					{
                        EPRINT(RTSP_CLIENT, "rcvd pps size exceed: [camera=%d], [frameSize=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, frameSize);
					}
					else
					{
                        configLen = (frameSize + sizeof(h264_h265_StartCode) );
					}

					newConfigRecv = FALSE;
                    memcpy(newConfig + newHeadSize, (fReceiveBuffer + fheadSize), configLen);
					newHeadSize += configLen;

					if((newHeadSize != fheadSize) || (memcmp(fReceiveBuffer, newConfig, newHeadSize) != 0))
					{
                        WPRINT(RTSP_CLIENT, "sps and pps parameters changed: [camera=%d], [newHeadSize=%d]",
                               mediaSession[mediaHandle].rtspInfo.camIndex, newHeadSize);
						memcpy(fReceiveBuffer, newConfig, newHeadSize);
						fheadSize = newHeadSize;

						memset(newConfig, 0, RUN_TIME_CONFIG_H264);
						newHeadSize = 0;
					}
				}
			}
		}
	}
	else
	{
		mediaInfo.isRTSP = FALSE;
        EPRINT(RTSP_CLIENT, "frame data truncated: [camera=%d], [bytes=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, numTruncatedBytes);
	}
	// Then continue, to request the next frame of data:
	continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function gets next frame from RTP layer.
 * @return
 */
Boolean H264VideoStreamSink::continuePlaying()
{
     UINT32 cnt;

	 if ((fSource == NULL) || (fReceiveBuffer == NULL))
	 {
		 return False; // sanity check (should not happen)
	 }

	 if (!fHaveWrittenFirstFrame)
	 {
		 fReceiveBufferMark = fReceiveBuffer;

		 // If we have PPS/SPS NAL units encoded in a "sprop parameter string",
		 // prepend these to the file:
		 UINT32 		numSPropRecords;
         SPropRecord 	*sPropRecords;

         sPropRecords = parseSPropParameterSets(fSPropParameterSetsStr, numSPropRecords);
		 for (cnt = 0; cnt < numSPropRecords; cnt++)
		 {
             memcpy((VOIDPTR)fReceiveBufferMark, (const VOIDPTR)h264_h265_StartCode, sizeof(h264_h265_StartCode));

             fReceiveBufferMark += sizeof(h264_h265_StartCode);

             memcpy((VOIDPTR)fReceiveBufferMark, (const VOIDPTR)sPropRecords[cnt].sPropBytes, sPropRecords[cnt].sPropLength);
			 fReceiveBufferMark += sPropRecords[cnt].sPropLength;
		 }

		 delete[] sPropRecords;
		 fheadSize = (fReceiveBufferMark - fReceiveBuffer);
	 }

	 fReceiveBufferMark = fReceiveBuffer + fheadSize;

     memcpy((VOIDPTR)fReceiveBufferMark, (const VOIDPTR)h264_h265_StartCode, sizeof(h264_h265_StartCode));
     fReceiveBufferMark += sizeof(h264_h265_StartCode);

     // Request the next frame of data from our input source. "afterGettingFrame()" will get called later, when it arrives:
     fSource->getNextFrame(fReceiveBufferMark, ((VIDEO_BUF_SIZE_MEDIA - fheadSize) - sizeof(h264_h265_StartCode)),
                           afterGettingFrame, this, onSourceClosure, this);
	 return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::createNew
 * @param env
 * @param subsession
 * @param mediaHandle
 * @param streamId
 * @return
 */
H265VideoStreamSink* H265VideoStreamSink::createNew(UsageEnvironment &env,
    MediaSubsession &subsession, RTSP_HANDLE handle, CHAR const *streamId)
{
    return new H265VideoStreamSink(env, subsession, handle, streamId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::H265VideoStreamSink
 * @param env
 * @param subsession
 * @param mediaHandle
 * @param streamId
 */
H265VideoStreamSink::H265VideoStreamSink(UsageEnvironment &env, MediaSubsession &subsession, RTSP_HANDLE handle,
                                         CHAR const *streamId) : MediaSink(env), fSubsession(subsession)
{
    fStreamId 				= strDup(streamId);
    fReceiveBuffer			= NULL;
    fIframeMark             = NULL;
    fPframeMark             = NULL;
    fReceiveBufferMark      = NULL;
    mediaHandle             = handle;
    fHaveWrittenFirstFrame 	= 0;
    width					= 0;
    height					= 0;
    noOfRefFrame			= 0;
    shmBase 				= NULL;
    shmFd 					= INVALID_FILE_FD;
    fPsize                  = 0;
    fIsize                  = 0;

    for (INT32 nalu = H265_VPS_NALU; nalu < H265_NALU_MAX; nalu++)
    {
        fMetaData[nalu]     = NULL;
        fMetaDataSize[nalu] = 0;
    }

    PmediaInfo.videoInfo.frameType      = P_FRAME;
    PmediaInfo.videoInfo.noOfRefFrame   = 10;
    PmediaInfo.codecType                = VIDEO_H265;
    PmediaInfo.isRTSP                   = TRUE;
    PmediaInfo.sampleRate               = 25;

    ImediaInfo.videoInfo.frameType      = I_FRAME;
    ImediaInfo.videoInfo.noOfRefFrame   = 10;
    ImediaInfo.codecType                = VIDEO_H265;
    ImediaInfo.isRTSP                   = TRUE;
    ImediaInfo.sampleRate               = 25;


    if (CreateSharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO) == TRUE)
    {
        fReceiveBuffer 		= (shmBase + sizeof(MEDIA_FRAME_INFO_t));
        fReceiveBufferMark 	= fReceiveBuffer;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::~H265VideoStreamSink
 */
H265VideoStreamSink::~H265VideoStreamSink()
{
	delete[] fStreamId;

    if (shmBase != NULL)
	{
        DestroySharedMemory(mediaHandle, &shmFd, &shmBase, STREAM_TYPE_VIDEO);
    }

    for (INT32 nalu = H265_VPS_NALU; nalu < H265_NALU_MAX; nalu++)
    {
        if (fMetaData[nalu])
        {
            delete[] fMetaData[nalu];
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::afterGettingFrame
 * @param clientData
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void H265VideoStreamSink::afterGettingFrame(VOIDPTR clientData, UINT32 frameSize, UINT32 numTruncatedBytes,
                                            struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    H265VideoStreamSink *sink = (H265VideoStreamSink*)clientData;
    sink->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::processFrame
 * @param frameSize
 * @param numTruncatedBytes
 * @param presentationTime
 * @param durationInMicroseconds
 * @return
 */
void H265VideoStreamSink::processFrame(UINT32 frameSize, UINT32 numTruncatedBytes,
                                       struct timeval presentationTime, UINT32 durationInMicroseconds)
{
    BOOL                    firstSlice = FALSE;
    MEDIA_FRAME_INFO_t      mediaInfo;

    mediaInfo.videoInfo.height = 0;
    mediaInfo.videoInfo.width = 0;
    mediaInfo.videoInfo.noOfRefFrame = 0;
    mediaInfo.videoInfo.frameType = MAX_FRAME_TYPE;

    if (numTruncatedBytes > 0)
    {
        EPRINT(RTSP_CLIENT, "frame data truncated: [camera=%d], [bytes=%d]", mediaSession[mediaHandle].rtspInfo.camIndex, numTruncatedBytes);
    }

    mediaSession[mediaHandle].frameRec = TRUE;

    if(GetH265Info(fReceiveBufferMark, frameSize, &mediaInfo.videoInfo, &firstSlice) == FAIL)
    {
        EPRINT(RTSP_CLIENT, "failed to parse h265 info: [camera=%d]", mediaSession[mediaHandle].rtspInfo.camIndex);
    }

    switch (mediaInfo.videoInfo.frameType)
    {
        case I_FRAME:
        case P_FRAME:
        {
            if (SUCCESS != handleH265VCLFrame(frameSize,mediaInfo.videoInfo.frameType,firstSlice,presentationTime))
            {
                EPRINT(RTSP_CLIENT, "Failed to handle vcl frameType :%d", mediaInfo.videoInfo.frameType);
            }
        }
        break;

        case VPS_FRAME:
        case SPS_FRAME:
        case PPS_FRAME:
        {
            if (SUCCESS != handleH265NonVCLFrame(frameSize,mediaInfo.videoInfo.frameType))
            {
                EPRINT(RTSP_CLIENT, "Failed to handle non-vcl frameType :%d", mediaInfo.videoInfo.frameType);
            }

            //Update Resolution in case of SPS
            if(mediaInfo.videoInfo.frameType == SPS_FRAME)
            {
                width   = mediaInfo.videoInfo.width;
                height  = mediaInfo.videoInfo.height;
                PmediaInfo.videoInfo.width      = width;
                PmediaInfo.videoInfo.height     = height;
                ImediaInfo.videoInfo.width      = width;
                ImediaInfo.videoInfo.height     = height;
            }
        }
        break;

        case FRAME_TYPE_SEI:
        {
            //Do Nothing
        }
        break;

        default:
        {
            EPRINT(RTSP_CLIENT,"Invalid frameType :%d",mediaInfo.videoInfo.frameType);
        }
        break;
    }

    continuePlaying();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::handleH265NonVCLFrame
 * @param frameSize
 * @param frameType
 * @return
 */
BOOL H265VideoStreamSink::handleH265NonVCLFrame(UINT32 frameSize, FRAME_TYPE_e frameType)
{

    if((frameType != VPS_FRAME) && (frameType != SPS_FRAME) && frameType != PPS_FRAME)
    {
        EPRINT(RTSP_CLIENT,"Invalid frameType :%d",frameType);
        return FAIL;
    }

    INT32 index = frameType - VPS_FRAME;
    if (!fMetaData[index] || (frameSize != fMetaDataSize[index])
            || (memcmp(fMetaData[index], fReceiveBufferMark, fMetaDataSize[index]) != STATUS_OK))
    {
        if(fMetaData[index])
        {
            delete[] fMetaData[index];
        }

        fMetaData[index]= new UINT8[frameSize+sizeof(h264_h265_StartCode)];
        memcpy(fMetaData[index],fReceiveBufferMark,frameSize+sizeof(h264_h265_StartCode));
        fMetaDataSize[index] = frameSize+sizeof(h264_h265_StartCode);
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::handleH265VCLFrame : handle I and P frames and also manage callbacks
 * @param frameSize : current NAL unit size
 * @param frameType : NAL Unit type
 * @param firstSlice: if its first slice or not
 * @param presentationTime:
 * @return
 */
BOOL H265VideoStreamSink::handleH265VCLFrame(UINT32 frameSize, FRAME_TYPE_e frameType, BOOL firstSlice,struct timeval presentationTime)
{
    if((frameType != I_FRAME) && (frameType != P_FRAME))
    {
        EPRINT(RTSP_CLIENT,"Invalid frameType :%d",frameType);
        return FAIL;
    }

    if(frameType == P_FRAME)
    {
        //for first slice handle callback for last P & I frames
        if(firstSlice)
        {
            handleCallback(I_FRAME,frameSize);
            handleCallback(P_FRAME,frameSize);
            fPframeMark = fReceiveBufferMark;
        }
        fPsize += (frameSize+sizeof(h264_h265_StartCode));
        PmediaInfo.len                  = fPsize;
        PmediaInfo.avPresentationTime   = presentationTime;
    }
    else
    {
        //for first slice handle callback for last P frames
        if(firstSlice)
        {
            handleCallback(I_FRAME,frameSize);
            handleCallback(P_FRAME,frameSize);
            fIsize = 0;
            //append new meta data info to current first slice of I frame
            appendMetaData(frameSize);
            fIframeMark = fReceiveBuffer;
        }
        fIsize += frameSize+sizeof(h264_h265_StartCode);
        ImediaInfo.len                  = fIsize;
        ImediaInfo.avPresentationTime   = presentationTime;
    }
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::handleCallback : will deliver pending  complete I or P frames.
 * @param frameType : I & P only
 * @param frameSize : Require to manage buffer location
 * @return
 */
BOOL H265VideoStreamSink::handleCallback(FRAME_TYPE_e frameType,UINT32 frameSize)
{
    if((frameType == P_FRAME) && (fPsize !=0) && (fPframeMark!=NULL))
    {
        memcpy(shmBase,&PmediaInfo,sizeof(MEDIA_FRAME_INFO_t));
        mediaSession[mediaHandle].rtspCb(RTSP_RESP_CODE_VIDEO_DATA, mediaHandle, &PmediaInfo,
                                         mediaSession[mediaHandle].rtspInfo.camIndex, 0, (fReceiveBuffer-fPframeMark));

        //Update pending P frame stats
        fPsize      = 0;
        fPframeMark = NULL;

        //Handle free space created above and shift valid data to free space.
        memmove(fReceiveBuffer,fReceiveBufferMark,frameSize+sizeof(h264_h265_StartCode));
        fReceiveBufferMark = fReceiveBuffer;
    }
    else if(frameType == I_FRAME && fIsize !=0 && fIframeMark!=NULL)
    {
        memcpy(shmBase,&ImediaInfo,sizeof(MEDIA_FRAME_INFO_t));
        if(!fHaveWrittenFirstFrame)
        {
            mediaSession[mediaHandle].rtspCb(RTSP_RESP_CODE_CONFIG_VIDEO_DATA, mediaHandle, &ImediaInfo,
                                             mediaSession[mediaHandle].rtspInfo.camIndex, 0, (fReceiveBuffer-fIframeMark));
            fHaveWrittenFirstFrame = TRUE;
        }
        else
        {
            mediaSession[mediaHandle].rtspCb(RTSP_RESP_CODE_VIDEO_DATA, mediaHandle, &ImediaInfo,
                                             mediaSession[mediaHandle].rtspInfo.camIndex, 0, (fReceiveBuffer-fIframeMark));
        }

        //Update pending P frame stats
        fIsize      = 0;
        fIframeMark = NULL;

        //Handle free space created above and shift valid data to free space.
        memmove(fReceiveBuffer,fReceiveBufferMark,frameSize+sizeof(h264_h265_StartCode));
        fReceiveBufferMark = fReceiveBuffer;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::appendMetaData
 * @param frameSize
 * @return
 */
BOOL H265VideoStreamSink::appendMetaData(UINT32 frameSize)
{
    UINT8 size = 0 ;
    //There should be no pending P frame callback
    if(fPsize)
    {
        EPRINT(RTSP_CLIENT, "buffer should not contain pending p-frame: [camera=%d]", mediaSession[mediaHandle].rtspInfo.camIndex);
    }

    for(INT32 nalu= H265_VPS_NALU; nalu< H265_NALU_MAX; nalu++)
    {
        // Required to complete I frame.
        if((!fMetaData[nalu]) || (fMetaDataSize[nalu] == 0))
        {
            EPRINT(RTSP_CLIENT,"VPS SPS PPS missing");
            return FAIL;
        }
        size +=  fMetaDataSize[nalu];
    }

    memmove(fReceiveBuffer+size,fReceiveBufferMark,frameSize+sizeof(h264_h265_StartCode));
    size = 0;

    for(INT32 nalu= H265_VPS_NALU; nalu< H265_NALU_MAX; nalu++)
    {
        memcpy(fReceiveBuffer+size,fMetaData[nalu],fMetaDataSize[nalu]);
        size += fMetaDataSize[nalu];
    }

    fIsize += size;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief H265VideoStreamSink::continuePlaying
 * @return
 */
Boolean H265VideoStreamSink::continuePlaying()
{
    if (!fSource)
        return false;

    //set pointer location for new NAL unit from Network
    fReceiveBufferMark = fReceiveBuffer+fIsize+fPsize;
    memcpy((VOIDPTR)fReceiveBufferMark,(const VOIDPTR)h264_h265_StartCode,sizeof(h264_h265_StartCode));

    fSource->getNextFrame(fReceiveBufferMark+sizeof(h264_h265_StartCode), (VIDEO_BUF_SIZE - (fIsize+fPsize+sizeof(h264_h265_StartCode))),
                          afterGettingFrame, this, onSourceClosure, this);
    return True;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is schedule task which is called at every timeout and checks for packet
 *          updates on session. If no packetes are receive then shutdown rtsp session and give callback.
 * @param   clientData
 * @return
 */
static void checkInterPacketGaps(VOIDPTR clientData)
{
    RTSPClient              *rtspClient = (RTSPClient *)clientData;
    StreamClientState       &scs = ((ourRTSPClient*)rtspClient)->scs;
    RTSP_HANDLE 			mediaHandle;
    MediaFrameResp_e 		mediaResp = RTSP_RESP_CODE_CONN_CLOSE;
	UINT32 					newTotNumPacketsReceived = 0;
	BOOL					status = FAIL;

    mediaHandle = ((ourRTSPClient*)clientData)->mediaHandle;
    if (mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
	{
        return;
    }

    MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);

    if (mediaSession[mediaHandle].terminateFlg == TRUE)
    {
        MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);
    }
    else
    {
        MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);

        if (mediaSession[mediaHandle].frameRec == TRUE)
        {
            MediaSubsessionIterator 	iter(*scs.session);
            MediaSubsession 			*subsession;

            while ((subsession = iter.next()) != NULL)
            {
                RTPSource * src = subsession->rtpSource();

                if (src == NULL)
                {
                    continue;
                }
                newTotNumPacketsReceived += src->receptionStatsDB().totNumPacketsReceived();
            }

            if (scs.recvPacketCnt != newTotNumPacketsReceived)
            {
                scs.recvPacketCnt = newTotNumPacketsReceived;
                scs.packGapCount = 0;
                status = SUCCESS;
            }
            else
            {
                scs.packGapCount++;

                if (scs.packGapCount >= MEDIA_TIMEOUT_SAMPLE)
                {
                    mediaResp = RTSP_RESP_CODE_FRAME_TIMEOUT;
                }
                else
                {
                    status = SUCCESS;
                }
            }
        }
        else
        {
            mediaResp = RTSP_RESP_CODE_CONN_CLOSE;
        }
    }

    if (status == SUCCESS)
    {
        // Check again, after the specified delay:
        startPacketGapScheculeTask(&scs.checkPacGapTask, MEDIA_PACKET_TIMEOUT, clientData);

        if ((mediaSession[mediaHandle].rtspInfo.transport == OVER_UDP) || (mediaSession[mediaHandle].rtspInfo.transport == TCP_INTERLEAVED))
        {
            // Increament timer count and check for 30 Sec and send option
            scs.timerCount++;
            if(scs.timerCount >= OPTION_CMD_SAMPLE_CNT)
            {
                scs.timerCount = 0;
                mediaSession[mediaHandle].rtspHandle->sendOptionsCommand(continueAfterOption);
            }
        }
    }
    else
    {
        shutdownStream(mediaHandle, mediaResp, TRUE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startCommTask
 * @param clientData
 * @return
 */
static void startCommTask(VOIDPTR clientData)
{
    BOOL        localStartCommF;
    RTSP_HANDLE mediaHandle;

    MUTEX_LOCK(rtspWatchDogMutex);
    rtspHang  = FALSE;  // For RTSP Watchdog
    MUTEX_UNLOCK(rtspWatchDogMutex);

    for (mediaHandle = 0; mediaHandle < MEDIA_SESSION_PER_RTSP_APPL; mediaHandle++)
	{
        MUTEX_LOCK(mediaSession[mediaHandle].sessParaMutex);
        localStartCommF = mediaSession[mediaHandle].startCommF;
		mediaSession[mediaHandle].startCommF = FALSE;
        MUTEX_UNLOCK(mediaSession[mediaHandle].sessParaMutex);

        if ((localStartCommF == TRUE) && (openURL(mediaHandle) == FAIL))
        {
            shutdownStream(mediaHandle, RTSP_RESP_CODE_CONNECT_FAIL, FALSE);
        }
	}

    startCommTaskToken = sysEnv->taskScheduler().scheduleDelayedTask(MICRO_SEC_PER_SEC, (TaskFunc*)startCommTask, NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startPacketGapScheculeTask
 * @param tokanPtr
 * @param timeout
 * @param clientData
 * @return
 */
static void startPacketGapScheculeTask(TaskToken *tokanPtr, UINT32 timeout, VOIDPTR clientData)
{
    *tokanPtr = sysEnv->taskScheduler().scheduleDelayedTask((timeout * MICRO_SEC_PER_SEC),
                                                            (TaskFunc*)checkInterPacketGaps, (VOIDPTR)clientData);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief stopPacketGepScheculeTask
 * @param tokanPtr
 * @return
 */
static void stopPacketGepScheculeTask(TaskToken *tokanPtr)
{
	sysEnv->taskScheduler().unscheduleDelayedTask(*tokanPtr);
	*tokanPtr = NULL;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
