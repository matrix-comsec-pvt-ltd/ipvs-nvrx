#ifndef AUDIOOUT_H
#define AUDIOOUT_H

#include "HardwareTestControl.h"
#include "DecDispLib.h"

class AudioInOut : public HardwareTestControl
{
    Q_OBJECT
public:
     AudioInOut(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0);
    ~AudioInOut() { };

     void hardwareTestStart();
     void hardwareTestStop();
     void saveHwTestStatus(CONDUNCT_TEST_e index);

private:
     AUDIO_TEST_DEVICE_e    audioNowPlaying;
     bool                   isStopDone;
};

#endif // AUDIO_OUT_H
