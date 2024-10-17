#ifndef LAN1SETTING_H
#define LAN1SETTING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/IpTextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/TextBox.h"
#include "Controls/DropDown.h"
#include "Controls/ElementHeading.h"
#include "DataStructure.h"

#define PREFIX_LEN_MAX 128

typedef enum
{
    IP_ADDR_MODE_IPV4,
    IP_ADDR_MODE_IPV4_AND_IPV6,
    MAX_IP_ADDR_MODE
}IP_ADDR_MODE_e;

typedef enum
{
    IPV4_ASSIGN_MODE_STATIC,
    IPV4_ASSIGN_MODE_DHCP,
    IPV4_ASSIGN_MODE_PPPOE,
    MAX_IPV4_ASSIGN_MODE
}IPV4_ASSIGN_MODE_e;

typedef enum
{
    IPV4_SETTINGS_ASSIGN_MODE,
    IPV4_SETTINGS_PPPOE_MODE,
    IPV4_SETTINGS_DNSV4,
    MAX_IPV4_SETTINGS
}IPV4_SETTINGS_e;

typedef enum
{
    IPV6_ASSIGN_MODE_STATIC,
    IPV6_ASSIGN_MODE_DHCP,
    IPV6_ASSIGN_MODE_SLAAC,
    MAX_IPV6_ASSIGN_MODE
}IPV6_ASSIGN_MODE_e;

typedef enum
{
    IPV6_SETTINGS_ASSIGN_MODE,
    IPV6_SETTINGS_DNSV6,
    MAX_IPV6_SETTINGS
}IPV6_SETTINGS_e;

typedef enum
{
    STATIC_IPV4_ADDR,
    STATIC_IPV4_SUBNETMASK,
    STATIC_IPV4_DFLT_GATEWAY,
    MAX_STATIC_IPV4
}STATIC_IPV4_e;

typedef enum
{
    STATIC_IPV6_ADDR,
    STATIC_IPV6_DFLT_GATEWAY,
    MAX_STATIC_IPV6
}STATIC_IPV6_e;

typedef enum
{
    DNS_MODE_STATIC,
    DNS_MODE_AUTO,
    MAX_DNS_MODE
}DNS_MODE_e;

typedef enum
{
    DNS_IP,
    DNS_ALTR_IP,
    MAX_DNS_IP
}DNS_IP_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    LAN1_FIELD_IPV4_ASSIGN_MODE,
    LAN1_FIELD_STATIC_IPV4_ADDR,
    LAN1_FIELD_STATIC_IPV4_SUBNET,
    LAN1_FIELD_STATIC_IPV4_DFLT_GATEWAY,
    LAN1_FIELD_PPPOE_USRNAME,
    LAN1_FIELD_PPPOE_PASSWORD,
    LAN1_FIELD_DNSV4_SER_MODE,
    LAN1_FIELD_DNSV4_IP,
    LAN1_FIELD_DNSV4_ALTR_IP,
    LAN1_FIELD_ADDR_MODE,
    LAN1_FIELD_IPV6_ASSIGN_MODE,
    LAN1_FIELD_IPV6_ADDRESS,
    LAN1_FIELD_IPV6_PREFIX_LEN,
    LAN1_FIELD_IPV6_GATEWAY,
    LAN1_FIELD_DNSV6_MODE,
    LAN1_FIELD_DNSV6_PRIMARY_ADDRESS,
    LAN1_FIELD_DNSV6_SECONDARY_ADDRESS,
    MAX_LAN1_FIELDS
}LAN1_CNFG_FIELD_e;

class Lan1Setting : public ConfigPageControl
{
    Q_OBJECT
private:

    IP_ADDR_MODE_e          currIpAddrMode;
    DropDown                *ipAddrModeDrpDwn;
    IPV4_ASSIGN_MODE_e      ipv4AssignMode;
    IPV6_ASSIGN_MODE_e      ipv6AssignMode;
    DNS_MODE_e              dnsv4Mode;
    DNS_MODE_e              dnsv6Mode;
    ElementHeading          *ipv4ElementHeadings[MAX_IPV4_SETTINGS];
    ElementHeading          *ipv6ElementHeadings[MAX_IPV6_SETTINGS];
    OptionSelectButton      *ipv4ModeOptionSelButton[MAX_IPV4_ASSIGN_MODE];
    OptionSelectButton      *ipv6ModeOptionSelButton[MAX_IPV6_ASSIGN_MODE];
    OptionSelectButton      *dnsv4ModeOptionSelButton[MAX_DNS_MODE];
    OptionSelectButton      *dnsv6ModeOptionSelButton[MAX_DNS_MODE];
    Ipv4TextBox             *staticIpv4AddrSet[MAX_STATIC_IPV4];
    IpTextBox               *staticIpv6AddrSet[MAX_STATIC_IPV6];
    Ipv4TextBox             *dnsv4IpAddrSet[MAX_DNS_IP];
    IpTextBox               *dnsv6IpAddrSet[MAX_DNS_IP];
    TextboxParam            *prefixLenParam;
    TextBox                 *ipv6PrefixLen;
    ReadOnlyElement         *macaddresReadOnly;
    TextboxParam            *pppoeUsernameParam;
    TextBox                 *pppoeUsername;
    TextboxParam            *pppoePasswordParam;
    PasswordTextbox         *pppoePassword;
    bool                    isIpAddrDropDwnChng;

public:
    explicit Lan1Setting(QString devName, QWidget *parent = 0);
    ~Lan1Setting();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    void getMacAddress();
    void selectIpAssignMode();
    void setDnsSerMode();
    void showHideElements();
    void enableDisableIpv6Ctrls();
    void handleInfoPageMessage(int);

public slots:
    void slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e state, int index);
    void slotIpTextBoxLoadInfopage(quint32 index);
    void slotIpTextBoxEntryDone(quint32);
    void slotSpinBoxValueChanged(QString, quint32 index);
};

#endif // LAN1SETTING_H
