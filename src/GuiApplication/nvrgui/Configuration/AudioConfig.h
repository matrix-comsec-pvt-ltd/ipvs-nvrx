#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H
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
//   Project      : NVR (Network Video Recorder)
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMutex>
#include "EnumFile.h"
#include "DataStructure.h"


//******** Defines and Data Types ****

//******** Function Prototypes *******
class AudioConfig : public QObject
{
    Q_OBJECT

public:
    // constructor of class Audio config
    AudioConfig ();
    // destructor of class audio config
    ~AudioConfig ();

    // initializes all audio parameters
    bool InitConfig (void);
    // writes new configuration
    bool WriteConfig (AUDIO_CONFIG_t* config);
    // reads audio configuration
    bool ReadConfig (AUDIO_CONFIG_t* config);

protected:

private:
    AUDIO_CONFIG_t cfg;
    QMutex cfgAccess;

signals:
    void sigAudioCfgChanged (const AUDIO_CONFIG_t *oldConfig,
                             const AUDIO_CONFIG_t *newConfig);
};

#endif // AUDIOCONFIG_H
