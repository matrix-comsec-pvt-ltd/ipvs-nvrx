#ifndef PLAYBACKRECORDDATA_H
#define PLAYBACKRECORDDATA_H

#include <QObject>

#define MAX_ASYNC_PB_RECORD    100

class PlaybackRecordData
{
public:
    QString deviceName;
    QString startTime;
    QString endTime;
    quint8 camNo;
    quint8 evtType;
    quint8 overlap;
    quint8 hddIndicator;
    quint8 partionIndicator;
    quint8 recDriveIndex;
    quint16 asyncPbIndex;

    PlaybackRecordData();
    void operator =(const PlaybackRecordData &);

    void clearPlaybackInfo();


};

#endif // PLAYBACKRECORDDATA_H
