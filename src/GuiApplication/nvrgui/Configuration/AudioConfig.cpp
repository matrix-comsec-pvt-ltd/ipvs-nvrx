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
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////

#include "AudioConfig.h"
#include "fileWrite.h"
#include <QFile>

//******** Extern Variables **********


//******** Defines and Data Types ****
// seek file pointer at base
#define SEEK_AT_BASE                0
// seek file pointer at index
#define SEEK_AT(INDEX, TYPE)        (sizeof(VERSION_e) + (INDEX * sizeof(TYPE)))

#define DEFAULT_AUDIO_LEVEL         50
#define DEFAULT_MUTE_STATUS         AUDIO_UNMUTE

// information file which contains remote device parameters
#define CONFIG_FILE                 CONFIG_FILE_PATH"AudioConfig.cfg"
// device configuration file version
#define CONFIG_VERSION              1

//******** Function Prototypes *******



//******** Global Variables **********



//******** Static Variables **********
// backup of current config
static AUDIO_CONFIG_t oldConfig;


//******** Function Definations ******
//*****************************************************************************
//  AudioConfig ()
//  Param:
//      IN : None
//      OUT: None
//  Returns:
//      Not Applicable
//  Description:
//
//*****************************************************************************
AudioConfig::AudioConfig ()
{
    memset(&cfg, 0 ,sizeof(cfg));
    }

//*****************************************************************************
//  ~AudioConfig ()
//  Param:
//      IN : Not Applicable
//      OUT: Not Applicable
//  Returns:
//      Not Applicable
//  Description:
//
//*****************************************************************************
AudioConfig::~AudioConfig ()
{

}

//*****************************************************************************
//  InitConfig ()
//  Param:
//      IN : None
//      OUT: None
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool AudioConfig::InitConfig (void)
{
    // return status of operation
    bool status = true;
    // config file handle
    QFile configFile (CONFIG_FILE);
    // bytes transferred during file operation
    qint64 transBytes;
    // version of the configuration
    VERSION_e version = 0;

    // open file in read/write mode
    status = configFile.open (QIODevice::ReadWrite);
    // succeeded to open/create file
    if (status == true)
    {
        // read version from file
        transBytes = configFile.read ((char *)&version, sizeof(VERSION_e));
        // version doesnt match
        if (version != CONFIG_VERSION)
        {
            // set permission of file
            status = configFile.setPermissions (CONFIG_FILE_PERIMISSION);
            // succeeded to set file permission
            if (status == true)
            {
                // seek file pointer at base
                status = configFile.seek (SEEK_AT_BASE);
                // succeeded to set file pointer at base
                if (status == true)
                {
                    // set version to current version number
                    version = CONFIG_VERSION;
                    // write version to file
                    transBytes = configFile.write ((char *)&version, sizeof(VERSION_e));
                    // failed to write version to file
					if (transBytes < (qint64) sizeof(VERSION_e))
                    {
                        // set status to false
                        status = false;
                    }
                    // succeeded to write version to file
                    else
                    {
                        // update configuration with default values
                        cfg.level = DEFAULT_AUDIO_LEVEL;
                        cfg.muteStatus = DEFAULT_MUTE_STATUS;
                        // write configuration to file
                        transBytes = configFile.write ((char *)&cfg, sizeof(AUDIO_CONFIG_t));
                        // failed to write configuration
						if (transBytes < (qint64) sizeof(AUDIO_CONFIG_t))
                        {
                            // set status to false
                            status = false;
                        }
                    }
                }
            }
        }
        // version matches
        else
        {
            // read configuration from file
            transBytes = configFile.read ((char *)&cfg, sizeof(AUDIO_CONFIG_t));
            // failed to read configuration from file
			if (transBytes < (qint64) sizeof(AUDIO_CONFIG_t))
            {
                // set status to file
                status = false;
            }
        }

        // close file
        configFile.close ();

        // succeeded to update configuration
        if (status == true)
        {
            // emit signal for configuration change
            emit sigAudioCfgChanged (NULL, &cfg);
        }
    }

    // return status
    return status;
    }

//*****************************************************************************
//  WriteConfig ()
//  Param:
//      IN : AUDIO_CONFIG_t config
//      OUT: None
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool AudioConfig::WriteConfig(AUDIO_CONFIG_t* config)
{
    // return status of operation
    bool status = true;
    // config file handler
    QFile configFile (CONFIG_FILE);
    // bytes transferred during file operation
    qint64 transBytes;
    // memory compare result
    qint64 cmpReturn;

    // lock access to configuration
    cfgAccess.lock ();
    // compare current and new configurations
    cmpReturn = memcmp (&cfg, config, sizeof(AUDIO_CONFIG_t));
    // current and new config doesn't match
    if (cmpReturn != 0)
    {
        // open file in write mode
        status = configFile.open (QIODevice::ReadWrite);
        // succeeded to open file
        if (status == true)
        {
            // seek file pointer at index
            status = configFile.seek (SEEK_AT(0, AUDIO_CONFIG_t));
            // succeeded to seek file pointer at index
            if (status == true)
            {
                // write new configuration to file
                transBytes = configFile.write ((char *)config, sizeof(AUDIO_CONFIG_t));
                // failed to write configuration to file
				if (transBytes < (qint64) sizeof(AUDIO_CONFIG_t))
                {
                    // set status to file
                    status = false;
                }
                // succeeded to write configuration to file
                else
                {
                    // keep a copy of current configuration
                    memcpy (&oldConfig, &cfg, sizeof(AUDIO_CONFIG_t));
                    // update current configuration with new one
                    memcpy (&cfg, config, sizeof(AUDIO_CONFIG_t));
                    // emit a signal to notify configuration change
                    emit sigAudioCfgChanged (&oldConfig, &cfg);
                }
            }

            configFile.close ();
        }
    }
    // unlock access to configuration
    cfgAccess.unlock ();

    // return status
    return status;
    }

//*****************************************************************************
//  ReadConfig ()
//  Param:
//      IN : AUDIO_CONFIG_t &config
//      OUT: None
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool AudioConfig::ReadConfig(AUDIO_CONFIG_t* config)
{
    // return status of operation
    bool status = true;

    // lock access to configuration copy
    cfgAccess.lock ();
    // copy configuration to user buffer
    memcpy (config, &cfg, sizeof(AUDIO_CONFIG_t));
    // unlock access to configuration
    cfgAccess.unlock ();

    // return status
    return status;
    }
