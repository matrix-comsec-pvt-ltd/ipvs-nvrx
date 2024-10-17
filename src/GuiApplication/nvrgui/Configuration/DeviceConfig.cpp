#include <QFile>
#include "fileWrite.h"
#include "DeviceConfig.h"

//******** Extern Variables **********
DEVICE_CONFIG_t rdevConfig[MAX_REMOTE_DEVICES];

//******** Defines and Data Types ****
// seek file pointer at index
#define SEEK_AT(INDEX, TYPE)        (sizeof(VERSION_e) + (INDEX * sizeof(TYPE)))

// default local device parameters
#define LOCAL_CONNECTION_TYPE       BY_IP_ADDRESS
#define LOCAL_IP_ADDRESS            "127.0.0.1"
#define LOCAL_TCP_PORT              0
#define LOCAL_FORWARDED_TCP_PORT    0
#define LOCAL_USERNAME              "local"
#define LOCAL_PASSWORD              "local"
#define LOCAL_ENABLE_STATE          true
#define LOCAL_AUTO_LOGIN_STATE      true
#define LOCAL_PREFER_NATIVE_DEVICE  false

// information file which contains remote device parameters
#define CONFIG_FILE                 CONFIG_FILE_PATH"NetworkDevice.cfg"

// device configuration file version
#define CONFIG_VERSION              5

// remote device pointer
#define REMOTE_DEVICE(ID)           (MAX_LOCAL_DEVICE + ID)

// file index to write
#define FILE_INDEX(ID)              (ID - MAX_LOCAL_DEVICE)

// information file which contains other user login parameters and 'remember me' flag
#define OTHER_LOGIN_PARAM_FILE      CONFIG_FILE_PATH"OtherLoginParam.cfg"
#define WIZARD_SETTING_FILE         CONFIG_FILE_PATH"WizOtherLoginParam.cfg"

// login parameter file version
#define OTHER_LOGIN_CONFIG_VERSION  1    // Only device name length changed (to 17)in version 2 //

// default remember me flag status
#define DFLT_REMEMBER_ME_STATUS     false

// default username for login
#define OTHER_LOGIN_USERNAME        ""

// default password for login
#define OTHER_LOGIN_PASSWORD        ""

#define WIZARD_SETTING_ENABLE_FLAG  true

//******** Function Definations ******
//*****************************************************************************
//  DeviceConfig ()
//  Param:
//      IN :
//      OUT:
//  Returns:
//
//  Description:
//
//*****************************************************************************
DeviceConfig::DeviceConfig ()
{
    memset(cfg, 0, sizeof(cfg));
    memset(&otherLoginCfg, 0, sizeof(otherLoginCfg));
}

//*****************************************************************************
//  ~DeviceConfig ()
//  Param:
//      IN :
//      OUT:
//  Returns:
//
//  Description:
//
//*****************************************************************************
DeviceConfig::~DeviceConfig ()
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
bool DeviceConfig::InitConfig ()
{
    bool        status = true;              // return status of operation
    QFile       configFile(CONFIG_FILE);    // config file handler
    qint64      transBytes;                 // transferred bytes during file operation
    VERSION_e   version = 0;                // file version

    // initialize local device
    initLocalDevice ();

    // open file in read only mode
    status = configFile.open (QIODevice::ReadWrite);
    if (status == false)
    {
        // failed to open file
        return status;
    }

    // read version from file
    transBytes = configFile.read ((char *)&version, sizeof(VERSION_e));

    // version doesn't match
    if (version != CONFIG_VERSION)
    {
        // set file permissions
        status = configFile.setPermissions (CONFIG_FILE_PERIMISSION);

        // succeeded to set file permission
        if (status == true)
        {
            // seek file pointer at base
            status = configFile.seek(0);

            // succeeded to set file pointer at base
            if (status == true)
            {
                // set version to current system configuration version
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
                    // default remote device configuration
                    for (quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
                    {
                        cfg[REMOTE_DEVICE(index)].deviceName[0] = '\0';
                        cfg[REMOTE_DEVICE(index)].connType = REMOTE_CONNECTION_TYPE;
                        cfg[REMOTE_DEVICE(index)].ipAddress[0] = '\0';
                        cfg[REMOTE_DEVICE(index)].port = REMOTE_TCP_PORT;
                        snprintf(cfg[REMOTE_DEVICE(index)].username, MAX_USERNAME_SIZE, REMOTE_USERNAME);
                        cfg[REMOTE_DEVICE(index)].password[0] = '\0';
                        cfg[REMOTE_DEVICE(index)].enable = REMOTE_ENABLE_STATE;
                        cfg[REMOTE_DEVICE(index)].autoLogin = REMOTE_AUTO_LOGIN_STATE;
                        cfg[REMOTE_DEVICE(index)].forwardedTcpPort = REMOTE_FORWARDED_TCP_PORT;
                    }

                    // write configuration to file
                    transBytes = configFile.write ((char*)cfg, sizeof(cfg));

                    // failed to write configuration file
                    if (transBytes < (qint64)sizeof(cfg))
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
        transBytes = configFile.read ((char*)cfg, sizeof(cfg));

        // failed to read data from file
        if (transBytes < (qint64)sizeof(cfg))
        {
            // set status to false
            status = false;
        }
    }

    // close file
    configFile.close ();

    // succeeded to initialize configuration?
    if (status == false)
    {
        return status;
    }

    // loop till maximum device
    for (quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        // device exists
        if(cfg[REMOTE_DEVICE(index)].deviceName[0] != '\0')
        {
            //emit signal to notify about existing device configurations
            emit sigDeviceCfgChanged (REMOTE_DEVICE(index), NULL, &cfg[REMOTE_DEVICE(index)]);
        }
    }

    // return status
    return status;
}

//*****************************************************************************
//  initLocalDevice ()
//  Param:
//      IN : None
//      OUT: None
//  Returns:
//      None
//  Description:
//
//*****************************************************************************
void DeviceConfig::initLocalDevice (void)
{
    quint8 localDevice = 0;

    // local device configuration
    snprintf(cfg[localDevice].deviceName,MAX_DEVICE_NAME_SIZE, LOCAL_DEVICE_NAME);
    cfg[localDevice].connType = LOCAL_CONNECTION_TYPE;
    snprintf(cfg[localDevice].ipAddress,MAX_IP_ADDRESS_SIZE, LOCAL_IP_ADDRESS);
    cfg[localDevice].port = LOCAL_TCP_PORT;
    snprintf(cfg[localDevice].username,MAX_USERNAME_SIZE, LOCAL_USERNAME);
    snprintf(cfg[localDevice].password,MAX_PASSWORD_SIZE, LOCAL_PASSWORD);
    cfg[localDevice].enable = LOCAL_ENABLE_STATE;
    cfg[localDevice].autoLogin = LOCAL_AUTO_LOGIN_STATE;
    cfg[localDevice].nativeDeviceCredential = LOCAL_PREFER_NATIVE_DEVICE;
    cfg[localDevice].liveStreamType = 2;
    cfg[localDevice].forwardedTcpPort = LOCAL_FORWARDED_TCP_PORT;

    //emit signal to notify about local device configurations
    emit sigDeviceCfgChanged (localDevice, NULL, &cfg[localDevice]);
}

//*****************************************************************************
//  GetDeviceList ()
//  Param:
//      IN : quint8 &deviceCount
//      OUT: QStringList &deviceList
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
void DeviceConfig::GetDeviceList (quint8 &deviceCount, QStringList &deviceList)
{
    deviceList.clear ();

    cfgAccess.lock ();
    for (quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        if (cfg[REMOTE_DEVICE(index)].deviceName[0] != '\0')
        {
            deviceList.append (cfg[REMOTE_DEVICE(index)].deviceName);
        }
    }
    cfgAccess.unlock ();

    deviceCount = deviceList.count ();
}

void DeviceConfig::GetEnabledDevices (quint8 &deviceCount, QStringList &deviceList)
{
    deviceList.clear ();

    cfgAccess.lock ();
    for (quint8 index = 0; index < MAX_DEVICES; index++)
    {
        if ((cfg[index].deviceName[0] != '\0') && (cfg[index].enable == true))
        {
            deviceList.append (cfg[index].deviceName);
        }
    }
    cfgAccess.unlock ();

   deviceCount = deviceList.count ();
}

//*****************************************************************************
//  GetDeviceIndex ()
//  Param:
//      IN : QString name,
//      OUT: quint8 &index
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool DeviceConfig::GetDeviceIndex(QString name, quint8 &index)
{
    bool status = false;

    if (name == "")
    {
        return false;
    }

    cfgAccess.lock ();
    for (quint8 devIndex = 0; devIndex < MAX_DEVICES; devIndex++)
    {
        if (cfg[devIndex].deviceName == name)
        {
            index = devIndex;
            status = true;
            break;
        }
    }
    cfgAccess.unlock ();

    return status;
}

//*****************************************************************************
//  InitOtherLoginConfig ()
//  Param:
//      IN : None
//      OUT: None
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool DeviceConfig::InitOtherLoginConfig (void)
{
    bool        status = true;                      // return status of operation
    QFile       configFile(OTHER_LOGIN_PARAM_FILE); // config file handler
    qint64      transBytes;                         // transferred bytes during file operation
    VERSION_e   version = 0;                        // file version

    // open file in read only mode
    status = configFile.open (QIODevice::ReadWrite);
    if (status == false)
    {
        // failed to open file
        return status;
    }

    // read version from file
    transBytes = configFile.read ((char *)&version, sizeof(VERSION_e));

    // version doesn't match
    if (version != OTHER_LOGIN_CONFIG_VERSION)
    {
        // set file permissions
        status = configFile.setPermissions (CONFIG_FILE_PERIMISSION);

        // succeeded to set file permission
        if (status == true)
        {
            // seek file pointer at base
            status = configFile.seek(0);

            // succeeded to set file pointer at base
            if (status == true)
            {
                // set version to current system configuration version
                version = OTHER_LOGIN_CONFIG_VERSION;

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
                    // default remote device configuration
                    otherLoginCfg.rememberMe = DFLT_REMEMBER_ME_STATUS;
                    snprintf(otherLoginCfg.username, MAX_USERNAME_SIZE, "%s", OTHER_LOGIN_USERNAME);
                    snprintf(otherLoginCfg.password, MAX_PASSWORD_SIZE, "%s", OTHER_LOGIN_PASSWORD);

                    // write configuration to file
                    transBytes = configFile.write ((char *)&otherLoginCfg, sizeof(OTHER_LOGIN_CONFIG_t));

                    // failed to write configuration file
                    if (transBytes < (qint64) sizeof(OTHER_LOGIN_CONFIG_t))
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
        transBytes = configFile.read ((char *)&otherLoginCfg, sizeof(OTHER_LOGIN_CONFIG_t));

        // failed to read data from file
        if (transBytes < (qint64) sizeof(OTHER_LOGIN_CONFIG_t))
        {
            // set status to false
            status = false;
        }
    }

    // close file
    configFile.close ();

    // return status
    return status;
}

bool DeviceConfig::InitWizardSettingConfig (void)
{
    bool    status = true;                          // return status of operation
    QFile   wizardconfigFile(WIZARD_SETTING_FILE);  // config file handler
    qint64  transBytes;                             // transferred bytes during file operation

    if(wizardconfigFile.exists() == false)
    {
        // open file in read only mode
        status = wizardconfigFile.open (QIODevice::ReadWrite);

        // succeeded to open file
        if (status == true)
        {
            // set file permissions
            status = wizardconfigFile.setPermissions (CONFIG_FILE_PERIMISSION);

            // succeeded to set file permission
            if (status == true)
            {
                // seek file pointer at base
                status = wizardconfigFile.seek (SEEK_AT(0, WIZ_OPEN_CONFIG_t));

                // succeeded to set file pointer at base
                if (status == true)
                {
                    // default remote device configuration
                    wizardCfg.wizardOpen = WIZARD_SETTING_ENABLE_FLAG;

                    // write configuration to file
                    transBytes = wizardconfigFile.write ((char *)&wizardCfg, sizeof(WIZ_OPEN_CONFIG_t));
                }
            }
        }
    }
    else
    {
        status = wizardconfigFile.open (QIODevice::ReadOnly);

        // succeeded to open file
        if (status == true)
        {
            // set file permissions
            status = wizardconfigFile.setPermissions (CONFIG_FILE_PERIMISSION);

            // succeeded to set file permission
            if (status == true)
            {
                // seek file pointer at base
                status = wizardconfigFile.seek (SEEK_AT(0, WIZ_OPEN_CONFIG_t));
                if (status == true)
                {
                    // read configuration from file
                    transBytes = wizardconfigFile.read ((char *)&wizardCfg, sizeof(WIZ_OPEN_CONFIG_t));

                    // failed to read data from file
					if (transBytes < (qint64) sizeof(WIZ_OPEN_CONFIG_t))
                    {
                        // set status to false
                        status = false;
                    }
                }
            }
        }
    }

    // close file
    wizardconfigFile.close ();

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
bool DeviceConfig::WriteOtherLoginConfig (OTHER_LOGIN_CONFIG_t* config)
{
    bool    status = true;                      // return status of operation
    QFile   configFile(OTHER_LOGIN_PARAM_FILE); // config file handler
    qint64  transBytes;                         // bytes transferred during file operation

    // lock access to configuration
    otherLoginCfgAccess.lock ();

    /* PARASOFT : Rule CERT_C-ARR38-a , MISRAC2012-RULE_21_18-a - Considering incorrect flow (sub activity type OTHER_LOGIN_CONFIG_t instead of WIZ_OPEN_CONFIG_t) */
    // compare current and new configurations
    if (memcmp(&otherLoginCfg, config, sizeof(OTHER_LOGIN_CONFIG_t)) != 0)
    {
        // open file in write mode
        status = configFile.open (QIODevice::ReadWrite);

        // succeeded to open file
        if (status == true)
        {
            // seek file pointer at index
            status = configFile.seek (SEEK_AT(0, OTHER_LOGIN_CONFIG_t));

            // succeeded to seek file pointer at index
            if (status == true)
            {
                if (config->rememberMe == false)
                {
                    snprintf(config->username, MAX_USERNAME_SIZE, "%s", OTHER_LOGIN_USERNAME);
                    snprintf(config->password, MAX_PASSWORD_SIZE, "%s", OTHER_LOGIN_PASSWORD);
                }

                // write new configuration to file
                transBytes = configFile.write ((char *)config, sizeof(OTHER_LOGIN_CONFIG_t));

                // failed to write configuration to file
				if (transBytes < (qint64) sizeof(OTHER_LOGIN_CONFIG_t))
                {
                    // set status to file
                    status = false;
                }
                else
                {
                    /* PARASOFT : Rule CERT_C-ARR38-a - Considering incorrect flow (sub activity type OTHER_LOGIN_CONFIG_t instead of WIZ_OPEN_CONFIG_t) */
                    memcpy (&otherLoginCfg, config, sizeof(OTHER_LOGIN_CONFIG_t));
                }
            }

            configFile.close ();
        }
    }

    // unlock access to configuration
    otherLoginCfgAccess.unlock ();

    // return status
    return status;
}

bool DeviceConfig::WriteWizOtherLoginConfig (WIZ_OPEN_CONFIG_t* config)
{
    bool    status = true;                          // return status of operation
    QFile   wizardconfigFile(WIZARD_SETTING_FILE);  // config file handler
    qint64  transBytes;                             // bytes transferred during file operation

    // lock access to configuration
    otherLoginCfgAccess.lock ();

    // compare current and new configurations
    if (memcmp(&wizardCfg, config, sizeof(WIZ_OPEN_CONFIG_t)) != 0)
    {
        // open file in write mode
        status = wizardconfigFile.open (QIODevice::ReadWrite);

        // succeeded to open file
        if (status == true)
        {
            // seek file pointer at index
            status = wizardconfigFile.seek (SEEK_AT(0, WIZ_OPEN_CONFIG_t));

            // succeeded to seek file pointer at index
            if (status == true)
            {
                // write new configuration to file
                transBytes = wizardconfigFile.write ((char *)config, sizeof(WIZ_OPEN_CONFIG_t));

                // failed to write configuration to file
				if (transBytes < (qint64) sizeof(WIZ_OPEN_CONFIG_t))
                {
                    // set status to file
                    status = false;
                }
                else
                {
                    memcpy (&wizardCfg, config, sizeof(WIZ_OPEN_CONFIG_t));
                }
            }

            wizardconfigFile.close ();
        }
    }

    // unlock access to configuration
    otherLoginCfgAccess.unlock ();

    // return status
    return status;
}

//*****************************************************************************
//  ReadOtherLoginConfig ()
//  Param:
//      IN : None
//      OUT: OTHER_LOGIN_CONFIG_t &config
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool DeviceConfig::ReadOtherLoginConfig (OTHER_LOGIN_CONFIG_t* config)
{
    // lock access to configuration
    otherLoginCfgAccess.lock ();

    // copy configuration to user buffer
    /* PARASOFT : Rule CERT_C-STR31-b - Considering incorrect flow (sub activity type READ_OTHER_LOGIN_PARAM instead of READ_WIZARD_PARAM) */
    memcpy (config, &otherLoginCfg, sizeof(OTHER_LOGIN_CONFIG_t));

    // unlock access to configuration
    otherLoginCfgAccess.unlock ();

    return true;
}

bool DeviceConfig::ReadWizOtherLoginConfig (WIZ_OPEN_CONFIG_t *config)
{
    // lock access to configuration
    otherLoginCfgAccess.lock ();

    // copy configuration to user buffer
    memcpy (config, &wizardCfg, sizeof(WIZ_OPEN_CONFIG_t));

    // unlock access to configuration
    otherLoginCfgAccess.unlock ();

    return true;
}

//*****************************************************************************
//  ReadLocalLoginConfig ()
//  Param:
//      IN : None
//      OUT: QString &username
//           QString &password
//  Returns:
//      bool [true/false]
//  Description:
//
//*****************************************************************************
bool DeviceConfig::ReadLocalLoginConfig (QString &username, QString &password)
{
    cfgAccess.lock ();
    username = cfg[0].username;
    password = cfg[0].password;
    cfgAccess.unlock ();
    return true;
}
