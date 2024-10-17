#include "PlaybackRecordData.h"
#include "EnumFile.h"

PlaybackRecordData::PlaybackRecordData()
{
    asyncPbIndex = 0;
    clearPlaybackInfo();
}

void PlaybackRecordData::operator =(const PlaybackRecordData & newObject)
{
    deviceName = newObject.deviceName;
    startTime = newObject.startTime;
    endTime = newObject.endTime;
    camNo = newObject.camNo;
    evtType = newObject.evtType;
    overlap = newObject.overlap;
    hddIndicator = newObject.hddIndicator;
    partionIndicator = newObject.partionIndicator;
    recDriveIndex = newObject.recDriveIndex;
    asyncPbIndex = newObject.asyncPbIndex;
}

void PlaybackRecordData::clearPlaybackInfo ()
{
    deviceName = "";
    startTime = endTime = "";
    camNo = evtType = overlap = hddIndicator = partionIndicator = 0;
    //when we give MAX_RECORDING_STORAGE_DRIVE in search parameter,
    //server will search in current storage recording drive
    recDriveIndex = MAX_RECORDING_STORAGE_DRIVE;
}
