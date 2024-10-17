#ifndef REMOTEDEVICECONFIG_H
#define REMOTEDEVICECONFIG_H
#include <QObject>
#include <QMutex>
#include <QStringList>
#include "EnumFile.h"
#include "DataStructure.h"

//******** Defines and Data Types ****
#define MAX_IP_ADDRESS_SIZE     41
#define MAX_USERNAME_SIZE       25
#define MAX_PASSWORD_SIZE       17

// default remote device parameters
#define REMOTE_DEVICE_NAME          ""
#define REMOTE_CONNECTION_TYPE      BY_IP_ADDRESS
#define REMOTE_IP_ADDRESS           ""
#define REMOTE_TCP_PORT             8000
#define REMOTE_FORWARDED_TCP_PORT   8001
#define REMOTE_USERNAME             "admin"
#define REMOTE_PASSWORD             ""
#define REMOTE_ENABLE_STATE         false
#define REMOTE_AUTO_LOGIN_STATE     true

// structure which contains device information
typedef struct
{
    char                deviceName[MAX_DEVICE_NAME_SIZE];
    CONNECTION_TYPE_e   connType;
    char                ipAddress[MAX_IP_ADDRESS_SIZE];
    quint16             port;
    char                username[MAX_USERNAME_SIZE];
    char                password[MAX_PASSWORD_SIZE];
    bool                enable;
    bool                autoLogin;
    quint8              liveStreamType;
    bool                nativeDeviceCredential;
    quint16             forwardedTcpPort;

}DEVICE_CONFIG_t;

extern DEVICE_CONFIG_t rdevConfig[MAX_REMOTE_DEVICES];

typedef struct
{
    bool rememberMe;
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];

}OTHER_LOGIN_CONFIG_t;

typedef struct
{
    bool wizardOpen;

}WIZ_OPEN_CONFIG_t;

//******** Function Prototypes *******
class DeviceConfig : public QObject
{
    Q_OBJECT

public:
    // constructor of class
    DeviceConfig ();

    // destructor of class
    ~DeviceConfig ();

    // initializes configuration
    bool InitConfig ();

    // get configured device list
    void GetDeviceList (quint8 &deviceCount, QStringList &deviceList);
    void GetEnabledDevices (quint8 &deviceCount, QStringList &deviceList);

    // get device index
    bool GetDeviceIndex (QString name, quint8 &indexndex);

    bool InitOtherLoginConfig (void);
    bool InitWizardSettingConfig(void);
    void initLocalDevice (void);
    bool WriteOtherLoginConfig (OTHER_LOGIN_CONFIG_t* config);
    bool ReadOtherLoginConfig (OTHER_LOGIN_CONFIG_t* config);
    bool WriteWizOtherLoginConfig (WIZ_OPEN_CONFIG_t* config);
    bool ReadWizOtherLoginConfig (WIZ_OPEN_CONFIG_t* config);
    bool ReadLocalLoginConfig (QString &username, QString &password);

protected:

private:
    // list of remote device configuration
    DEVICE_CONFIG_t cfg[MAX_DEVICES];

    // read write access lock for local config copy
    QMutex cfgAccess;

    // other login parameter config
    OTHER_LOGIN_CONFIG_t otherLoginCfg;
    WIZ_OPEN_CONFIG_t  wizardCfg;

    // read write access lock for other user login config
    QMutex otherLoginCfgAccess;

signals:
    // signal to notify change in device configuration
    void sigDeviceCfgChanged (quint8 index, const DEVICE_CONFIG_t *currentConfig, const DEVICE_CONFIG_t *newConfig);
};

#endif // REMOTEDEVICECONFIG_H
