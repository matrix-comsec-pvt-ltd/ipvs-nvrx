#ifndef NETWORKSETTING_H
#define NETWORKSETTING_H

#include "Controls/ElementHeading.h"
#include "Controls/Bgtile.h"
#include "NavigationControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/Heading.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/IpTextBox.h"
#include "Controls/MessageBanner.h"
#include "Controls/DropDown.h"
#include "Controls/TextBox.h"
#include "Controls/ProcessBar.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "Controls/InfoPage.h"
#include "WizardCommon.h"

typedef enum
{
    SELECT_LAN1,
    SELECT_LAN2,
    MAX_LAN
}LAN_SELECT_TYPE_e;

typedef enum
{
    WIZ_IP_ADDR_MODE_IPV4,
    WIZ_IP_ADDR_MODE_IPV4_AND_IPV6,

    WIZ_MAX_IP_ADDR_MODE
}WIZ_IP_ADDR_MODE_e;

typedef enum
{
    WIZ_IPV4_ASSIGN_MODE_STATIC,
    WIZ_IPV4_ASSIGN_MODE_DHCP,

    WIZ_MAX_IPV4_ASSIGN_MODE
}WIZ_IPV4_ASSIGN_MODE_e;

typedef enum
{
    WIZ_IPV6_ASSIGN_MODE_STATIC,
    WIZ_IPV6_ASSIGN_MODE_DHCP,
    WIZ_IPV6_ASSIGN_MODE_SLAAC,

    WIZ_MAX_IPV6_ASSIGN_MODE
}WIZ_IPV6_ASSIGN_MODE_e;

typedef enum
{
    WIZ_DNS_MODE_STATIC,
    WIZ_DNS_MODE_AUTO,

    WIZ_MAX_DNS_MODE
}WIZ_DNS_MODE_e;

typedef enum
{
    WIZ_DNS_IP,
    WIZ_DNS_ALTR_IP,

    WIZ_MAX_DNS_IP
}WIZ_DNS_IP_e;

typedef enum
{
    WIZ_STATIC_IPV4_ADDR,
    WIZ_STATIC_IPV4_SUBNETMASK,
    WIZ_STATIC_IPV4_DFLT_GATEWAY,

    WIZ_MAX_STATIC_IPV4
}WIZ_STATIC_IPV4_e;

typedef enum
{
    WIZ_STATIC_IPV6_ADDR,
    WIZ_STATIC_IPV6_DFLT_GATEWAY,

    WIZ_MAX_STATIC_IPV6
}WIZ_STATIC_IPV6_e;

typedef enum
{
    WIZ_LAN1_FIELD_IPV4_ASSIGN_MODE,
    WIZ_LAN1_FIELD_IPV4_ADDRESS,
    WIZ_LAN1_FIELD_IPV4_SUBNETMASK,
    WIZ_LAN1_FIELD_IPV4_DFLT_GATEWAY,
    WIZ_LAN1_FIELD_PPPOE_USRNAME,
    WIZ_LAN1_FIELD_PPPOE_PASSWORD,
    WIZ_LAN1_FIELD_DNSV4_SER_MODE,
    WIZ_LAN1_FIELD_DNSV4_IP,
    WIZ_LAN1_FIELD_DNSV4_ALTR_IP,
    WIZ_LAN1_FIELD_ADDR_MODE,
    WIZ_LAN1_FIELD_IPV6_ASSIGN_MODE,
    WIZ_LAN1_FIELD_IPV6_ADDRESS,
    WIZ_LAN1_FIELD_IPV6_PREFIX_LEN,
    WIZ_LAN1_FIELD_IPV6_GATEWAY,
    WIZ_LAN1_FIELD_DNSV6_MODE,
    WIZ_LAN1_FIELD_DNSV6_IP,
    WIZ_LAN1_FIELD_DNSV6_ALTR_IP,

    WIZ_MAX_LAN1_FIELDS
}WIZ_LAN1_FIELD_e;

typedef enum
{
    WIZ_LAN2_FIELD_IP_ADDRESS,
    WIZ_LAN2_FIELD_SUBNET,
    WIZ_LAN2_FIELD_GATEWAY,
    WIZ_LAN2_FIELD_ADDR_MODE,
    WIZ_LAN2_FIELD_IPV6_ADDRESS,
    WIZ_LAN2_FIELD_IPV6_PREFIX_LEN,
    WIZ_LAN2_FIELD_IPV6_GATEWAY,

    WIZ_MAX_LAN2_FIELDS
}WIZ_LAN2_FIELDS_e;


class NetworkSetting : public WizardCommon
{
    Q_OBJECT
public:
    explicit NetworkSetting(QString devName, QString subHeadStr, QWidget *parent = 0,WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);
    virtual ~NetworkSetting();
    void createDefaultElements(QString subHeadStr);

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getLan1Config();
    void getLan2Config();
    void defaultLan1Config();
    void defaultLan2Config();
    void saveConfig();
    void selectIpAssignMode();
    void setPayLoad();
    void setDnsSerMode();
    void displayLan1Lan2();
    void setFlag(bool);
    void deleteDynamicElement();
    void enableDisableIpv6Ctrls();

private:
    WIZ_IP_ADDR_MODE_e      currIpAddrMode;
    DropDown                *ipAddrModeDrpDwn;
    TextLabel               *m_networkSettingHeading;
    Ipv4TextBox             *staticIpv4AddrSet[WIZ_MAX_STATIC_IPV4];
    IpTextBox               *staticIpv6AddrSet[WIZ_MAX_STATIC_IPV6];
    TextboxParam            *prefixLenParam;
    TextBox                 *ipv6PrefixLen;
    OptionSelectButton      *dnsv4ModeOptionSelButton[WIZ_MAX_DNS_MODE];
    OptionSelectButton      *dnsv6ModeOptionSelButton[WIZ_MAX_DNS_MODE];
    Ipv4TextBox             *dnsv4IpAddrSet[WIZ_MAX_DNS_IP];
    IpTextBox               *dnsv6IpAddrSet[WIZ_MAX_DNS_IP];
    OptionSelectButton      *ipv4ModeOptionSelButton[WIZ_MAX_IPV4_ASSIGN_MODE];
    OptionSelectButton      *ipv6ModeOptionSelButton[WIZ_MAX_IPV6_ASSIGN_MODE];
    DropDown                *m_lanDropDownBox;
    WIZ_IPV4_ASSIGN_MODE_e  ipv4AssignMode;
    WIZ_IPV6_ASSIGN_MODE_e  ipv6AssignMode;
    WIZ_DNS_MODE_e          dnsv4Mode;
    WIZ_DNS_MODE_e          dnsv6Mode;
    InfoPage                *m_infoPage;
    PayloadLib              *payloadLib;
    ApplController          *applController;
    QString                 currDevName;
    quint8                  lanSelected;
    bool                    isIpAddrDropDwnChng;
    bool                    m_nextBtnClick;
    DEV_TABLE_INFO_t        devTableInfo;

public slots:
    void slotIpTextBoxEntryDone(quint32);
    void slotIpTextBoxLoadInfopage(quint32 index);
    void slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e state,int index);
    void slotDropDownBoxValueChanged(QString, quint32 indexInPage);
    void slotInfoPageBtnclick(int);
};

#endif // NETWORKSETTING_H
