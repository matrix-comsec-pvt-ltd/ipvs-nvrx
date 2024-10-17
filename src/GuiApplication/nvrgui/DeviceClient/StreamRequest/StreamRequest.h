#ifndef STREAMREQUEST_H
#define STREAMREQUEST_H

#include <QObject>
#include <QThread>
#include <QList>
#include <QMutex>
#include "AnalogMedia/AnalogMedia.h"
#include "LiveMedia/LiveMedia.h"
#include "PlaybackMedia/PlaybackMedia.h"
#include "SyncPbMedia/SyncPbMedia.h"
#include "InstantPlaybackMedia/InstantPlaybackMedia.h"
#include "ClientMedia/ClientMedia.h"
#include "../CommandRequest/CommandRequest.h"
#include "AudioIn.h"

#define MAX_STREAM_COMMAND_REQUEST      5

/* set stack size of thread */
#define STREAM_REQUEST_THREAD_STACK_SIZE (1 * MEGA_BYTE)

typedef enum
{
    START_STREAM_COMMAND,
    STOP_STREAM_COMMAND,
    REPLACE_STREAM_COMMAND,
    CHANGE_STREAM_TYPE_COMMAND,
    INCLUDE_AUDIO_COMMAND,
    EXCLUDE_AUDIO_COMMAND,
	INCL_EXCL_AUDIO_IN_COMMAND,

    GET_PLAYBACK_ID_COMMAND,
    PLAY_PLABACK_STREAM_COMMAND,
    STEP_PLABACK_STREAM_COMMAND,
    STOP_PLABACK_STREAM_COMMAND,
    AUDIO_PLABACK_STREAM_COMMAND,
    CLEAR_PLAYBACK_ID_COMMAND,

    PLAY_SYNCPLABACK_STREAM_COMMAND,
    SYNCPLAY_SYNCPLABACK_STREAM_COMMAND,
	SET_SPEED_SYNCPLABACK_STREAM_COMMAND,
    STEPFORWARD_SYNCPLABACK_STREAM_COMMAND,
    STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND,
    PAUSE_SYNCPLABACK_STREAM_COMMAND,
    STOP_SYNCPLABACK_STREAM_COMMAND,
    AUDIO_SYNCPLABACK_STREAM_COMMAND,
    CLEAR_SYNCPLABACK_STREAM_COMMAND,

    START_INSTANTPLAYBACK_COMMAND,
    SEEK_INSTANTPLAYBACK_COMMAND,
    STOP_INSTANTPLAYBACK_COMMAND,
    PAUSE_INSTANTPLAYBACK_COMMAND,
    AUDIO_INSTANTPLAYBACK_COMMAND,

    STRT_CLNT_AUDIO_COMMAND,
    STOP_CLNT_AUDIO_COMMAND,

    MAX_STREAM_COMMAND

}STREAM_COMMAND_TYPE_e;

const QString streamCmdString[MAX_STREAM_COMMAND] = {"START_STREAM",
                                                     "STOP_STREAM",
                                                     "REPLACE_STREAM",
                                                     "CHANGE_STREAM_TYPE",
                                                     "INCLUDE_AUDIO",
                                                     "EXCLUDE_AUDIO",
                                                     "INCL_EXCL_AUDIO_IN",

                                                     "GET_PLAYBACK_ID",
                                                     "PLAY_PLABACK_STREAM",
                                                     "STEP_PLABACK_STREAM",
                                                     "STOP_PLABACK_STREAM",
                                                     "AUDIO_PLABACK_STREAM",
                                                     "CLEAR_PLAYBACK_ID",

                                                     "PLAY_SYNCPLABACK_STREAM",
                                                     "SYNCPLAY_SYNCPLABACK_STREAM",
                                                     "STEPFORWARD_SYNCPLABACK_STREAM",
                                                     "STEPBACKWARD_SYNCPLABACK_STREAM",
                                                     "PAUSE_SYNCPLABACK_STREAM",
                                                     "STOP_SYNCPLABACK_STREAM",
                                                     "AUDIO_SYNCPLABACK_STREAM",
                                                     "CLEAR_SYNCPLABACK_STREAM",

                                                     "START_INSTANTPLAYBACK",
                                                     "SEEK_INSTANTPLAYBACK",
                                                     "STOP_INSTANTPLAYBACK",
                                                     "PAUSE_INSTANTPLAYBACK",
                                                     "AUDIO_INSTANTPLAYBACK"};

typedef struct _STREAM_REQUEST_QUEUE_t
{
    STREAM_COMMAND_TYPE_e commandType;
    SERVER_SESSION_INFO_t serverInfo;
    StreamRequestParam *requestParam;
    SERVER_SESSION_INFO_t nextServerInfo;
    StreamRequestParam *nextRequestParam;
    _STREAM_REQUEST_QUEUE_t() : commandType(MAX_STREAM_COMMAND), requestParam(NULL), nextRequestParam(NULL)
    {

    }
}STREAM_REQUEST_QUEUE_t;

class StreamRequest : public QObject
{
    Q_OBJECT

public:
    explicit StreamRequest(quint8 streamIndex);
    ~StreamRequest();

    void processRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                        SERVER_SESSION_INFO_t serverInfo,
                        StreamRequestParam *streamRequestParam);
    void processRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                        SERVER_SESSION_INFO_t serverInfo,
                        StreamRequestParam *streamRequestParam,
                        SERVER_SESSION_INFO_t m_nextServerInfo,
                        StreamRequestParam *nextStreamRequestParam);
    bool getClearFlag();
    void setClearFlag(bool flag);
    bool getFreeFlag();
    void setFreeFlag(bool flag);
    bool getSetFreeFlag();
    bool getReplaceFlag();
    void setReplaceFlag(bool flag);
    void getStreamWindowInfo(DISPLAY_TYPE_e *displayType, quint16 *windowIndex);
    QString getStreamDeviceName();
    bool getDeletionProcessFlag();
    void setDeletionProcessFlag(bool flag);
    void setDecoderIntanceCreated(bool flag);
    bool getDecoderIntanceCreated();
    void deleteStreamRequest ();
    void delMedControl();

    void getWindowInfo(DISPLAY_TYPE_e displayType,
                       quint8 &decoderWindow,
                       quint16 &actualWindow,
                       qint64 &timeStamp);
    void setWindowInfo(DISPLAY_TYPE_e displayType,
                       quint8 decoderWindow,
                       quint16 actualWindow = MAX_CHANNEL_FOR_SEQ,
                       qint64 timeStamp = 0);

    static void swapWindowInfo(DISPLAY_TYPE_e displayType,
                               StreamRequest *firstRequest,
                               StreamRequest *secondRequest);

    static QMutex audioDecoderLock;
    static quint8 audioStreamIndex;
    static quint8 audioIndex;

    static quint8 getAudioStreamIndex();
    static void setAudioStreamIndex(const quint8 &value);

    static quint8 getAudioIndex();
    static void setAudioIndex(const quint8 &value);

   static STREAM_REQUEST_QUEUE_t m_requestQueue[MAX_STREAM_SESSION][MAX_STREAM_COMMAND_REQUEST];
   static quint8 m_queueReadIndex[MAX_STREAM_SESSION], m_queueWriteIndex[MAX_STREAM_SESSION];
   static QMutex m_queueLock[MAX_STREAM_SESSION];

private:
    SERVER_SESSION_INFO_t m_currentServerInfo;
    QMutex m_serverInfoAccess;
    SERVER_SESSION_INFO_t m_nextServerInfo;
    StreamRequestParam *m_nextRequestParam;

    quint8 m_streamId, m_decoderId;
    quint8 m_channelIndex;
    QString m_deviceName;
    STREAM_STATE_e m_streamStatus;
    STREAM_REQUEST_TYPE_e m_streamType;
    LIVE_STREAM_TYPE_e m_liveStreamType;
    qint64 m_timeStamp[MAX_DISPLAY_TYPE];
    bool m_isDecInstanceCreated;
    QMutex m_decoderInstanceLock;
    DISPLAY_TYPE_e m_displayType, m_duplicateDisplayType;
    quint8 m_windowId[MAX_DISPLAY_TYPE];
    quint16 m_actualWindowId[MAX_DISPLAY_TYPE];
    quint8 m_syncDecoderIdSet[MAX_SYNC_PB_SESSION];

    QString m_playbackId;

    bool m_clearFlag;
    QMutex m_clearFlagAccess;
    bool m_freeFlag;
    QMutex m_freeFlagAccess;
    bool m_deleteAfterMediaControlDeleteFlag;
    QMutex m_deleteAfterMediaControlDeleteFlagAccess;
    bool m_replaceFlag;
    QMutex m_replaceFlagAccess;
    bool m_mediaPauseForReplaceFlag;
    QMutex m_mediaPauseForReplaceFlagAccess;
    QMutex m_streamParamMutex;

    LiveMedia *m_liveMedia;
    AnalogMedia *m_analogMedia;
    PlaybackMedia *m_playbackMedia;
    SyncPbMedia *m_syncPlaybackMedia;
    InstantPlaybackMedia *m_instantPlaybackMedia;
    ClientMedia* m_clientMedia;
    AudioIn* m_audioIn;
    CommandRequest *m_mediaControl;

    bool m_deletionProcess;
    QMutex m_deletionProcessLock;
    bool m_reRequest;

    DISPLAY_TYPE_e m_displayTypeForDelete[MAX_DISPLAY_TYPE];
    quint16 m_actualWindowIdForDelete[MAX_DISPLAY_TYPE];

    bool createNewStream(SERVER_SESSION_INFO_t serverInfo,
                         StreamRequestParam *streamRequestParam,
                         bool &isDecoderError,
                         bool createMediaFlag = true);
    void startMedia(SET_COMMAND_e commandId = MAX_NET_COMMAND);
    void restartMedia();
    void stopMedia(bool deleteDecoder = true, bool isDecoderError = false);
    bool stopStream(StreamRequestParam *streamRequestParam,
                    bool deleteDecoder = true,
                    bool isDecoderError = false,
                    bool *sameReq=NULL);
    void deleteDecoderInstance();
    void pauseDecoderInstance(bool stopAnalogFlag = true);
    void resumeDecoderInstance();
    void freeStreamRequest();
    void replaceStreamRequest();
    void clearStreamInfo();
    bool createMediaRequest(SERVER_SESSION_INFO_t serverSessionInfo,
                            SET_COMMAND_e commandId,
                            StreamRequestParam *streamRequestParam);
    bool createMediaControl(SERVER_SESSION_INFO_t serverSessionInfo,
                            SET_COMMAND_e commandType,
                            QString payloadString);
    void deleteMediaRequest();
    void deleteMediaControl();
    void replaceMediaRequest();
    bool changeStreamType(StreamRequestParam *streamRequestParam);
    bool includeAudio(StreamRequestParam *streamRequestParam);
    bool excludeAudio(StreamRequestParam *streamRequestParam);
    void excludeAudio();
    bool includeExculdeAudioIn(StreamRequestParam *streamRequestParam);
    void forceStopAudioIn();
    void getPlaybackId(SERVER_SESSION_INFO_t serverInfo,
                       StreamRequestParam *streamRequestParam);
    void getPlaybackIdForInstatPlayback(SERVER_SESSION_INFO_t serverInfo,
                                        StreamRequestParam *streamRequestParam);
    void setAudioAction(STREAM_COMMAND_TYPE_e streamCommandType,
                        StreamRequestParam *streamRequestParam);
    void setSyncPlaybackAction(STREAM_COMMAND_TYPE_e streamCommandType,
                               StreamRequestParam *streamRequestParam);
    void sendInstatPlaybackAction(SERVER_SESSION_INFO_t serverInfo,
                                  STREAM_COMMAND_TYPE_e streamCommandType,
                                  StreamRequestParam *streamRequestParam);
    bool getDeleteAfterMediaControlDeleteFlag();
    void setDeleteAfterMediaControlDeleteFlag(bool flag);
    void getCurrentServerInfo(SERVER_SESSION_INFO_t &info);
    void setCurrentServerInfo(SERVER_SESSION_INFO_t info);

    bool getMediaPauseForReplaceFlag();
    void setMediaPauseForReplaceFlag(bool flag);
    bool getMediaRequestStopFlag();

    bool isMediaControlActive();
    bool isQueueEmpty();
    bool isQueueFull();
    bool putRequestInQueue(STREAM_REQUEST_QUEUE_t &request);
    bool getRequestFromQueue(STREAM_REQUEST_QUEUE_t *request);

signals:
    void sigProcessRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                           SERVER_SESSION_INFO_t serverInfo,
                           StreamRequestParam *streamRequestParam);

    void sigProcessRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                           SERVER_SESSION_INFO_t serverInfo,
                           StreamRequestParam *streamRequestParam,
                           SERVER_SESSION_INFO_t nextInfo,
                           StreamRequestParam *nextParam);

    void sigStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType,
                                  StreamRequestParam *streamRequestParam,
                                  DEVICE_REPLY_TYPE_e deviceReply);
    void sigDeleteStreamRequest(quint8 streamId);
    void sigDelMedControl(quint8 streamId);
    void sigChangeAudState();

public slots:
    void slotProcessRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                            SERVER_SESSION_INFO_t serverInfo,
                            StreamRequestParam *streamRequestParam);
    void slotProcessRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                            SERVER_SESSION_INFO_t serverInfo,
                            StreamRequestParam *streamRequestParam,
                            SERVER_SESSION_INFO_t nextInfo,
                            StreamRequestParam *nextParam);

    void slotStreamResponse(REQ_MSG_ID_e requestId,
                            SET_COMMAND_e commandId,
                            DEVICE_REPLY_TYPE_e statusId,
                            QString payload);
};

#endif // LIVESTREAMREQUEST_H
