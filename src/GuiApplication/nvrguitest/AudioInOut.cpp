#include <unistd.h>
#include "AudioInOut.h"

AudioInOut::AudioInOut(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent)
    : HardwareTestControl(hwIndex, parent)
{
    QStringList strlst;

    audioNowPlaying = MAX_AUDIO_TEST_DEVICE;
    isStopDone = false;

    strlst << "Audio IN";
    createDefaultComponent(xPos, yPos, totalRow, totalCol, true, false, TEST_HEADING_FONT_SIZE, strlst);
}

void AudioInOut::hardwareTestStart()
{
    while(1)
    {
        if(isStopDone)
        {
            break;
        }

        if(AUDIO_TEST_AO_DEV == audioNowPlaying)
        {
            audioNowPlaying = AUDIO_TEST_HDMI_AO_DEV;
        }
        else
        {
            audioNowPlaying = AUDIO_TEST_AO_DEV;
        }
        StartAudioOutTest(audioNowPlaying);
        sleep(5);
        if(false == isStopDone)
        {
            StopAudioOutTest(audioNowPlaying);
        }
    }    
    isStopDone = false;
}

void AudioInOut::hardwareTestStop()
{
    isStopDone = true;
    StopAudioOutTest(audioNowPlaying);
    audioNowPlaying = MAX_AUDIO_TEST_DEVICE;
}

void AudioInOut::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}
