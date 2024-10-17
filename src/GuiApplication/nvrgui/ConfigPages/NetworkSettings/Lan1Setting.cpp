#include "Lan1Setting.h"
#include "ValidationMessage.h"

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(13)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(12)
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(50)
#define LAN1_INDEX                  1

#define CNFG_TO_INDEX               1

// List of control
typedef enum
{
    LAN1_STG_IP_ADDR_MODE_DRP_DWN,
    LAN1_STG_IPV4_ASSIGN_STATIC,
    LAN1_STG_IPV4_ASSIGN_DHCP,
    LAN1_STG_IPV4_ASSIGN_PPPOE,
    LAN1_STG_IPV4_STATIC_IP_ADDR,
    LAN1_STG_IPV4_STATIC_SUBNETMASK,
    LAN1_STG_IPV4_STATIC_DFLT_GATEWAY,
    LAN1_STG_PPPOE_USRNAME,
    LAN1_STG_PPPOE_PASSWORD,
    LAN1_STG_DNSV4_MODE_STATIC,
    LAN1_STG_DNSV4_MODE_AUTO,
    LAN1_STG_DNSV4_IP_ADDR,
    LAN1_STG_DNSV4_ALTR_IP_ADDR,
    LAN1_STG_IPV6_ASSIGN_STATIC,
    LAN1_STG_IPV6_ASSIGN_DHCP,
    LAN1_STG_IPV6_ASSIGN_SLAAC,
    LAN1_STG_IPV6_STATIC_IP_ADDR,
    LAN1_STG_IPV6_STATIC_PREFIX_LEN,
    LAN1_STG_IPV6_STATIC_DFLT_GATEWAY,
    LAN1_STG_DNSV6_MODE_STATIC,
    LAN1_STG_DNSV6_MODE_AUTO,
    LAN1_STG_DNSV6_IP_ADDR,
    LAN1_STG_DNSV6_ALTR_IP_ADDR,
    MAX_LAN1_STG_ELEMETS
}LAN1_STG_ELELIST_e;

static const QMap<quint8, QString> ipAddrModeMapList =
{
    {IP_ADDR_MODE_IPV4,             "IPv4"},
    {IP_ADDR_MODE_IPV4_AND_IPV6,    "IPv4 & IPv6"},
};

static const QString ipv4AssignModeStr[MAX_IPV4_ASSIGN_MODE] = {"Static", "DHCP", "PPPoE"};

static const QString ipv6AssignModeStr[MAX_IPV6_ASSIGN_MODE] = {"Static", "DHCP", "SLAAC"};

static const QString staticIpv4Str[MAX_STATIC_IPV4] = {"IPv4 Address", "Subnet Mask", "Default Gateway"};

static const QString staticIpv6Str[MAX_STATIC_IPV6] = {"IPv6 Address", "Default Gateway"};

static const QString ipv4ElementHeadingStr[MAX_IPV4_SETTINGS] = {"IPv4 Assignment Mode", "PPPoE Mode", "DNSv4 Settings"};

static const QString ipv6ElementHeadingStr[MAX_IPV6_SETTINGS] = {"IPv6 Assignment Mode", "DNSv6 Settings"};

static const QString dnsIpStr[MAX_DNS_IP] = {"Preferred DNS", "Alternate DNS"};

Lan1Setting::Lan1Setting(QString devName, QWidget *parent)
    : ConfigPageControl(devName, parent, MAX_LAN1_STG_ELEMETS),
      ipv4AssignMode(MAX_IPV4_ASSIGN_MODE), ipv6AssignMode(MAX_IPV6_ASSIGN_MODE),
      dnsv4Mode(MAX_DNS_MODE), dnsv6Mode(MAX_DNS_MODE)
{
    currIpAddrMode = MAX_IP_ADDR_MODE;
    isIpAddrDropDwnChng = false;

    createDefaultComponent();
    resetGeometryOfCnfgbuttonRow(SCALE_HEIGHT(20));
    Lan1Setting::getConfig();
}

Lan1Setting::~Lan1Setting()
{
    DELETE_OBJ(macaddresReadOnly);

    if (IS_VALID_OBJ(ipAddrModeDrpDwn))
    {
        disconnect(ipAddrModeDrpDwn,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotSpinBoxValueChanged(QString,quint32)));
        disconnect(ipAddrModeDrpDwn,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(ipAddrModeDrpDwn);
    }

    DELETE_OBJ(ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]);

    for(quint8 index = IPV4_ASSIGN_MODE_STATIC; index < MAX_IPV4_ASSIGN_MODE; index++)
    {
        if(IS_VALID_OBJ(ipv4ModeOptionSelButton[index]))
        {
            disconnect(ipv4ModeOptionSelButton[index],
                       SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                       this,
                       SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
            disconnect(ipv4ModeOptionSelButton[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(ipv4ModeOptionSelButton[index]);
        }
    }

     if(IS_VALID_OBJ(staticIpv4AddrSet[STATIC_IPV4_ADDR]))
     {
         disconnect(staticIpv4AddrSet[STATIC_IPV4_ADDR],
                    SIGNAL(sigEntryDone(quint32)),
                    this,
                    SLOT(slotIpTextBoxEntryDone(quint32)));
     }

    for(quint8 index = STATIC_IPV4_ADDR; index < MAX_STATIC_IPV4; index++)
    {
        if(IS_VALID_OBJ(staticIpv4AddrSet[index]))
        {
            disconnect(staticIpv4AddrSet[index],
                       SIGNAL(sigLoadInfopage(quint32)),
                       this,
                       SLOT(slotIpTextBoxLoadInfopage(quint32)));
            disconnect(staticIpv4AddrSet[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(staticIpv4AddrSet[index]);
        }
    }

    DELETE_OBJ(ipv4ElementHeadings[IPV4_SETTINGS_DNSV4]);

    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        if(IS_VALID_OBJ(dnsv4ModeOptionSelButton[index]))
        {
            disconnect(dnsv4ModeOptionSelButton[index],
                       SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                       this,
                       SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
            disconnect(dnsv4ModeOptionSelButton[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(dnsv4ModeOptionSelButton[index]);
        }
    }

    for(quint8 index = DNS_IP; index < MAX_DNS_IP; index++)
    {
        if(IS_VALID_OBJ(dnsv4IpAddrSet[index]))
        {
            disconnect(dnsv4IpAddrSet[index],
                       SIGNAL(sigLoadInfopage(quint32)),
                       this,
                       SLOT(slotIpTextBoxLoadInfopage(quint32)));
            disconnect(dnsv4IpAddrSet[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(dnsv4IpAddrSet[index]);
        }
    }

    DELETE_OBJ(ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC]);

    for(quint8 index = IPV6_ASSIGN_MODE_STATIC; index < MAX_IPV6_ASSIGN_MODE; index++)
    {
        if (IS_VALID_OBJ(ipv6ModeOptionSelButton[index]))
        {
            disconnect(ipv6ModeOptionSelButton[index],
                       SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                       this,
                       SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
            disconnect(ipv6ModeOptionSelButton[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(ipv6ModeOptionSelButton[index]);
        }
    }

    for(quint8 index = STATIC_IPV6_ADDR; index < MAX_STATIC_IPV6; index++)
    {
        if (IS_VALID_OBJ(staticIpv6AddrSet[index]))
        {
            disconnect(staticIpv6AddrSet[index],
                       SIGNAL(sigLoadInfopage(quint32)),
                       this,
                       SLOT(slotIpTextBoxLoadInfopage(quint32)));
            disconnect(staticIpv6AddrSet[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(staticIpv6AddrSet[index]);
        }
    }

    if (IS_VALID_OBJ(ipv6PrefixLen))
    {
        disconnect(ipv6PrefixLen,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(ipv6PrefixLen);
    }

    DELETE_OBJ(prefixLenParam);
    DELETE_OBJ(ipv6ElementHeadings[IPV6_SETTINGS_DNSV6]);

    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        if (IS_VALID_OBJ(dnsv6ModeOptionSelButton[index]))
        {
            disconnect(dnsv6ModeOptionSelButton[index],
                       SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                       this,
                       SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
            disconnect(dnsv6ModeOptionSelButton[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(dnsv6ModeOptionSelButton[index]);
        }
    }

    for(quint8 index = DNS_IP; index < MAX_DNS_IP; index++)
    {
        if (IS_VALID_OBJ(dnsv6IpAddrSet[index]))
        {
            disconnect(dnsv6IpAddrSet[index],
                       SIGNAL(sigLoadInfopage(quint32)),
                       this,
                       SLOT(slotIpTextBoxLoadInfopage(quint32)));
            disconnect(dnsv6IpAddrSet[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(dnsv6IpAddrSet[index]);
        }
    }

    if (IS_VALID_OBJ(pppoeUsername))
    {
        disconnect(pppoeUsername,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(pppoeUsername);
    }
    DELETE_OBJ(pppoeUsernameParam);

    if (IS_VALID_OBJ(pppoePassword))
    {
        disconnect(pppoePassword,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(pppoePassword);
    }
    DELETE_OBJ(pppoePasswordParam);

    DELETE_OBJ(ipv4ElementHeadings[IPV4_SETTINGS_PPPOE_MODE]);
}

void Lan1Setting::createDefaultComponent()
{
    macaddresReadOnly = new ReadOnlyElement(FIRST_ELE_XOFFSET, FIRST_ELE_YOFFSET,
                                            BGTILE_LARGE_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            SCALE_WIDTH(READONLY_LARGE_WIDTH),
                                            READONLY_HEIGHT , "", this,
                                            COMMON_LAYER, -1,
                                            SCALE_WIDTH(10), "MAC Address");

    ipAddrModeDrpDwn = new DropDown(macaddresReadOnly->x(),
                                    (macaddresReadOnly->y() + macaddresReadOnly->height() + SCALE_HEIGHT(6)),
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    LAN1_STG_IP_ADDR_MODE_DRP_DWN,
                                    DROPDOWNBOX_SIZE_200,
                                    "IP Addressing Mode",
                                    ipAddrModeMapList,
                                    this,
                                    "", true, 10,
                                    COMMON_LAYER);
    m_elementList[LAN1_STG_IP_ADDR_MODE_DRP_DWN] = ipAddrModeDrpDwn;
    connect(ipAddrModeDrpDwn,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChanged(QString,quint32)));
    connect(ipAddrModeDrpDwn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE] = new ElementHeading(ipAddrModeDrpDwn->x(),
                                                                        ipAddrModeDrpDwn->y() + BGTILE_HEIGHT + SCALE_HEIGHT(6),
                                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                                        BGTILE_HEIGHT,
                                                                        ipv4ElementHeadingStr[IPV4_SETTINGS_ASSIGN_MODE],
                                                                        TOP_LAYER, this,
                                                                        false, SCALE_WIDTH(10));

    ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_STATIC] = new OptionSelectButton(ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->x() + SCALE_WIDTH(205),
                                                                              ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->y(),
                                                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                                                              BGTILE_HEIGHT,
                                                                              RADIO_BUTTON_INDEX,
                                                                              ipv4AssignModeStr[IPV4_ASSIGN_MODE_STATIC],
                                                                              this,
                                                                              NO_LAYER,
                                                                              0,
                                                                              MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                              NORMAL_FONT_SIZE,
                                                                              LAN1_STG_IPV4_ASSIGN_STATIC);

    ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_DHCP] = new OptionSelectButton(ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_STATIC]->x() +
                                                                            ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_STATIC]->width() + SCALE_WIDTH(10),
                                                                            ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->y(),
                                                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                                                            BGTILE_HEIGHT,
                                                                            RADIO_BUTTON_INDEX,
                                                                            ipv4AssignModeStr[IPV4_ASSIGN_MODE_DHCP],
                                                                            this,
                                                                            NO_LAYER,
                                                                            0,
                                                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                            NORMAL_FONT_SIZE,
                                                                            LAN1_STG_IPV4_ASSIGN_DHCP);

    ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_PPPOE] = new OptionSelectButton(ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_DHCP]->x() +
                                                                             ipv4ModeOptionSelButton[IPV4_ASSIGN_MODE_DHCP]->width() + SCALE_WIDTH(10),
                                                                             ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->y(),
                                                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                                                             BGTILE_HEIGHT,
                                                                             RADIO_BUTTON_INDEX,
                                                                             ipv4AssignModeStr[IPV4_ASSIGN_MODE_PPPOE],
                                                                             this,
                                                                             NO_LAYER,
                                                                             0,
                                                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                             NORMAL_FONT_SIZE,
                                                                             LAN1_STG_IPV4_ASSIGN_PPPOE);
    for(quint8 index = IPV4_ASSIGN_MODE_STATIC; index < MAX_IPV4_ASSIGN_MODE; index++)
    {
        m_elementList[index + LAN1_STG_IPV4_ASSIGN_STATIC] = ipv4ModeOptionSelButton[index];
        connect(ipv4ModeOptionSelButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(ipv4ModeOptionSelButton[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
    }

    staticIpv4AddrSet[STATIC_IPV4_ADDR] = new Ipv4TextBox(ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->x(),
                                                          ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->y() + ipv4ElementHeadings[IPV4_SETTINGS_ASSIGN_MODE]->height(),
                                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                                          BGTILE_HEIGHT,
                                                          LAN1_STG_IPV4_STATIC_IP_ADDR,
                                                          staticIpv4Str[STATIC_IPV4_ADDR],
                                                          this, MIDDLE_TABLE_LAYER,
                                                          true, 0, true, false,
                                                          LEFT_MARGIN_FROM_CENTER);
    connect(staticIpv4AddrSet[STATIC_IPV4_ADDR],
            SIGNAL(sigEntryDone(quint32)),
            this,
            SLOT(slotIpTextBoxEntryDone(quint32)));

    staticIpv4AddrSet[STATIC_IPV4_SUBNETMASK] = new Ipv4TextBox(staticIpv4AddrSet[STATIC_IPV4_ADDR]->x(),
                                                                staticIpv4AddrSet[STATIC_IPV4_ADDR]->y() + staticIpv4AddrSet[STATIC_IPV4_ADDR]->height(),
                                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                                BGTILE_HEIGHT,
                                                                LAN1_STG_IPV4_STATIC_SUBNETMASK,
                                                                staticIpv4Str[STATIC_IPV4_SUBNETMASK],
                                                                this, MIDDLE_TABLE_LAYER,
                                                                true, 0, true,true,
                                                                LEFT_MARGIN_FROM_CENTER);

    staticIpv4AddrSet[STATIC_IPV4_DFLT_GATEWAY] = new Ipv4TextBox(staticIpv4AddrSet[STATIC_IPV4_SUBNETMASK]->x(),
                                                                  staticIpv4AddrSet[STATIC_IPV4_SUBNETMASK]->y() + staticIpv4AddrSet[STATIC_IPV4_SUBNETMASK]->height(),
                                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  LAN1_STG_IPV4_STATIC_DFLT_GATEWAY,
                                                                  staticIpv4Str[STATIC_IPV4_DFLT_GATEWAY],
                                                                  this, BOTTOM_TABLE_LAYER,
                                                                  true, 0, true, false,
                                                                  LEFT_MARGIN_FROM_CENTER);
    for(quint8 index = STATIC_IPV4_ADDR; index < MAX_STATIC_IPV4; index++)
    {
        m_elementList[index + LAN1_STG_IPV4_STATIC_IP_ADDR] = staticIpv4AddrSet[index];
        connect(staticIpv4AddrSet[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(staticIpv4AddrSet[index],
                SIGNAL(sigLoadInfopage(quint32)),
                this,
                SLOT(slotIpTextBoxLoadInfopage(quint32)));
    }

    ipv4ElementHeadings[IPV4_SETTINGS_DNSV4] = new ElementHeading(staticIpv4AddrSet[STATIC_IPV4_DFLT_GATEWAY]->x(),
                                                                  staticIpv4AddrSet[STATIC_IPV4_DFLT_GATEWAY]->y() + staticIpv4AddrSet[STATIC_IPV4_DFLT_GATEWAY]->height()+ SCALE_HEIGHT(6),
                                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  ipv4ElementHeadingStr[IPV4_SETTINGS_DNSV4],
                                                                  TOP_LAYER, this,
                                                                  false, SCALE_WIDTH(10), NORMAL_FONT_SIZE, true);

    dnsv4ModeOptionSelButton[DNS_MODE_STATIC] = new OptionSelectButton(ipv4ElementHeadings[IPV4_SETTINGS_DNSV4]->x(),
                                                                       ipv4ElementHeadings[IPV4_SETTINGS_DNSV4]->y() + ipv4ElementHeadings[IPV4_SETTINGS_DNSV4]->height(),
                                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                                       BGTILE_HEIGHT,
                                                                       RADIO_BUTTON_INDEX,
                                                                       this, MIDDLE_TABLE_LAYER,
                                                                       "DNSv4 Mode",
                                                                       "Static",
                                                                       -1,
                                                                       LAN1_STG_DNSV4_MODE_STATIC,
                                                                       true,
                                                                       NORMAL_FONT_SIZE,
                                                                       NORMAL_FONT_COLOR, false,
                                                                       LEFT_MARGIN_FROM_CENTER);

    dnsv4ModeOptionSelButton[DNS_MODE_AUTO] = new OptionSelectButton((dnsv4ModeOptionSelButton[DNS_MODE_STATIC]->x() + SCALE_WIDTH(350) - LEFT_MARGIN_FROM_CENTER),
                                                                     dnsv4ModeOptionSelButton[DNS_MODE_STATIC]->y(),
                                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                                     BGTILE_HEIGHT,
                                                                     RADIO_BUTTON_INDEX,
                                                                     "Automatic",
                                                                     this,
                                                                     NO_LAYER,
                                                                     0,
                                                                     MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                     NORMAL_FONT_SIZE,
                                                                     LAN1_STG_DNSV4_MODE_AUTO);
    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        m_elementList[index + LAN1_STG_DNSV4_MODE_STATIC] = dnsv4ModeOptionSelButton[index];
        connect(dnsv4ModeOptionSelButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(dnsv4ModeOptionSelButton[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
    }

    dnsv4IpAddrSet[DNS_IP] = new Ipv4TextBox(dnsv4ModeOptionSelButton[DNS_MODE_STATIC]->x(),
                                             dnsv4ModeOptionSelButton[DNS_MODE_STATIC]->y() + dnsv4ModeOptionSelButton[DNS_MODE_STATIC]->height(),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             LAN1_STG_DNSV4_IP_ADDR,
                                             dnsIpStr[DNS_IP],
                                             this, MIDDLE_TABLE_LAYER,
                                             true, 0, true, false,
                                             LEFT_MARGIN_FROM_CENTER);

    dnsv4IpAddrSet[DNS_ALTR_IP] = new Ipv4TextBox(dnsv4IpAddrSet[DNS_IP]->x(),
                                                  dnsv4IpAddrSet[DNS_IP]->y() + dnsv4IpAddrSet[DNS_IP]->height(),
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  LAN1_STG_DNSV4_ALTR_IP_ADDR,
                                                  dnsIpStr[DNS_ALTR_IP],
                                                  this, BOTTOM_TABLE_LAYER,
                                                  true, 0, true, false,
                                                  LEFT_MARGIN_FROM_CENTER);
    for(quint8 index = DNS_IP; index < MAX_DNS_IP; index++)
    {
        m_elementList[index + LAN1_STG_DNSV4_IP_ADDR] = dnsv4IpAddrSet[index];
        connect(dnsv4IpAddrSet[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(dnsv4IpAddrSet[index],
                SIGNAL(sigLoadInfopage(quint32)),
                this,
                SLOT(slotIpTextBoxLoadInfopage(quint32)));
    }

    ipv4ElementHeadings[IPV4_SETTINGS_PPPOE_MODE] = new ElementHeading(dnsv4IpAddrSet[DNS_ALTR_IP]->x(),
                                                                       dnsv4IpAddrSet[DNS_ALTR_IP]->y() + dnsv4IpAddrSet[DNS_ALTR_IP]->height()+ SCALE_HEIGHT(6),
                                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                                       BGTILE_HEIGHT,
                                                                       ipv4ElementHeadingStr[IPV4_SETTINGS_PPPOE_MODE],
                                                                       TOP_LAYER, this,
                                                                       false, SCALE_WIDTH(10), NORMAL_FONT_SIZE, true);

    pppoeUsernameParam = new TextboxParam();
    pppoeUsernameParam->labelStr = "Username";
    pppoeUsernameParam->maxChar = 60;
    pppoeUsernameParam->isTotalBlankStrAllow = true;
    pppoeUsernameParam->validation = QRegExp(QString("[") + QString("a-zA-Z0-9\\-_.,():@!#$*+\\[\\]/\\\\") + QString("]"));

    pppoeUsername = new TextBox(ipv4ElementHeadings[IPV4_SETTINGS_PPPOE_MODE]->x(),
                                ipv4ElementHeadings[IPV4_SETTINGS_PPPOE_MODE]->y() + ipv4ElementHeadings[IPV4_SETTINGS_PPPOE_MODE]->height(),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                LAN1_STG_PPPOE_USRNAME,
                                TEXTBOX_LARGE,
                                this, pppoeUsernameParam,
                                MIDDLE_TABLE_LAYER, true, false,
								false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN1_STG_PPPOE_USRNAME] = pppoeUsername;
    connect(pppoeUsername,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    pppoePasswordParam = new TextboxParam();
    pppoePasswordParam->labelStr = "Password";
    pppoePasswordParam->maxChar = 40;
    pppoePasswordParam->isTotalBlankStrAllow = true;
    pppoePasswordParam->validation = QRegExp(QString("[") + QString("a-zA-Z0-9\\-_.,():@!#$*+\\[\\]/\\\\") + QString("]"));

    pppoePassword = new PasswordTextbox(pppoeUsername->x(),
                                        pppoeUsername->y() + pppoeUsername->height(),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        LAN1_STG_PPPOE_PASSWORD,
                                        TEXTBOX_LARGE,
                                        this, pppoePasswordParam,
                                        BOTTOM_TABLE_LAYER, true,
										LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN1_STG_PPPOE_PASSWORD] = pppoePassword;
    connect(pppoePassword,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC] = new ElementHeading((ipAddrModeDrpDwn->x() + BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(6)),
                                                                      (ipAddrModeDrpDwn->y() + BGTILE_HEIGHT + SCALE_HEIGHT(6)),
                                                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                                                      BGTILE_HEIGHT,
                                                                      ipv6ElementHeadingStr[IPV6_SETTINGS_ASSIGN_MODE],
                                                                      TOP_LAYER, this,
                                                                      false, SCALE_WIDTH(10));

    ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_STATIC] = new OptionSelectButton((ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC]->x() + SCALE_WIDTH(205)),
                                                                              ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC]->y(),
                                                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                                                              BGTILE_HEIGHT,
                                                                              RADIO_BUTTON_INDEX,
                                                                              ipv6AssignModeStr[IPV6_ASSIGN_MODE_STATIC],
                                                                              this,
                                                                              NO_LAYER,
                                                                              0,
                                                                              MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                              NORMAL_FONT_SIZE,
                                                                              LAN1_STG_IPV6_ASSIGN_STATIC);

    ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_DHCP] = new OptionSelectButton(ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_STATIC]->x() +
                                                                            ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_STATIC]->width() + SCALE_WIDTH(10),
                                                                            ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_STATIC]->y(),
                                                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                                                            BGTILE_HEIGHT,
                                                                            RADIO_BUTTON_INDEX,
                                                                            ipv6AssignModeStr[IPV6_ASSIGN_MODE_DHCP],
                                                                            this,
                                                                            NO_LAYER,
                                                                            0,
                                                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                            NORMAL_FONT_SIZE,
                                                                            LAN1_STG_IPV6_ASSIGN_DHCP);

    ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_SLAAC] = new OptionSelectButton(ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_DHCP]->x() +
                                                                             ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_DHCP]->width() + SCALE_WIDTH(10),
                                                                             ipv6ModeOptionSelButton[IPV6_ASSIGN_MODE_DHCP]->y(),
                                                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                                                             BGTILE_HEIGHT,
                                                                             RADIO_BUTTON_INDEX,
                                                                             ipv6AssignModeStr[IPV6_ASSIGN_MODE_SLAAC],
                                                                             this,
                                                                             NO_LAYER,
                                                                             0,
                                                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                             NORMAL_FONT_SIZE,
                                                                             LAN1_STG_IPV6_ASSIGN_SLAAC);
    for(quint8 index = IPV6_ASSIGN_MODE_STATIC; index < MAX_IPV6_ASSIGN_MODE; index++)
    {
        m_elementList[index + LAN1_STG_IPV6_ASSIGN_STATIC] = ipv6ModeOptionSelButton[index];
        connect(ipv6ModeOptionSelButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(ipv6ModeOptionSelButton[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
    }

    staticIpv6AddrSet[STATIC_IPV6_ADDR] = new IpTextBox(ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC]->x(),
                                                        ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC]->y() + ipv6ElementHeadings[IPV6_ASSIGN_MODE_STATIC]->height(),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        LAN1_STG_IPV6_STATIC_IP_ADDR,
                                                        staticIpv6Str[STATIC_IPV6_ADDR],
                                                        IP_ADDR_TYPE_IPV6_ONLY,
                                                        this, MIDDLE_TABLE_LAYER,
                                                        true, 0, true,
														IP_FIELD_TYPE_IPV6_ADDR,
                                                        IP_TEXTBOX_ULTRALARGE,
														LEFT_MARGIN_FROM_CENTER);

    m_elementList[LAN1_STG_IPV6_STATIC_IP_ADDR] = staticIpv6AddrSet[STATIC_IPV6_ADDR];
    connect(staticIpv6AddrSet[STATIC_IPV6_ADDR],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(staticIpv6AddrSet[STATIC_IPV6_ADDR],
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    prefixLenParam = new TextboxParam();
    prefixLenParam->labelStr = "Prefix Length";
    prefixLenParam->maxChar = 3;
    prefixLenParam->validation = QRegExp(QString("[0-9]"));
    prefixLenParam->isNumEntry = true;
    prefixLenParam->minNumValue = 1;
    prefixLenParam->maxNumValue = 128;

    ipv6PrefixLen = new TextBox(staticIpv6AddrSet[STATIC_IPV6_ADDR]->x(),
                                staticIpv6AddrSet[STATIC_IPV6_ADDR]->y() + staticIpv6AddrSet[STATIC_IPV6_ADDR]->height(),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                LAN1_STG_IPV6_STATIC_PREFIX_LEN,
                                TEXTBOX_MEDIAM,
                                this, prefixLenParam,
                                MIDDLE_TABLE_LAYER,
								true, false, false,
								LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN1_STG_IPV6_STATIC_PREFIX_LEN] = ipv6PrefixLen;
    connect(ipv6PrefixLen,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY] = new IpTextBox(ipv6PrefixLen->x(),
                                                                ipv6PrefixLen->y() + ipv6PrefixLen->height(),
                                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                                BGTILE_HEIGHT,
                                                                LAN1_STG_IPV6_STATIC_DFLT_GATEWAY,
                                                                staticIpv6Str[STATIC_IPV6_DFLT_GATEWAY],
                                                                IP_ADDR_TYPE_IPV6_ONLY,
                                                                this, BOTTOM_TABLE_LAYER,
                                                                true, 0, true,
																IP_FIELD_TYPE_IPV6_ADDR,
																IP_TEXTBOX_ULTRALARGE,
																LEFT_MARGIN_FROM_CENTER);

    m_elementList[LAN1_STG_IPV6_STATIC_DFLT_GATEWAY] = staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY];
    connect(staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY],
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));


    ipv6ElementHeadings[IPV6_SETTINGS_DNSV6] = new ElementHeading(staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY]->x(),
                                                                  staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY]->y() + staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY]->height() + SCALE_HEIGHT(6),
                                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  ipv6ElementHeadingStr[IPV6_SETTINGS_DNSV6],
                                                                  TOP_LAYER, this,
                                                                  false, SCALE_WIDTH(10), NORMAL_FONT_SIZE, true);

    dnsv6ModeOptionSelButton[DNS_MODE_STATIC] = new OptionSelectButton((ipv6ElementHeadings[IPV6_SETTINGS_DNSV6]->x()),
                                                                       ipv6ElementHeadings[IPV6_SETTINGS_DNSV6]->y()+ ipv6ElementHeadings[IPV6_SETTINGS_DNSV6]->height(),
                                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                                       BGTILE_HEIGHT,
                                                                       RADIO_BUTTON_INDEX,
                                                                       this, MIDDLE_TABLE_LAYER,
                                                                       "DNSv6 Mode",
                                                                       "Static",
                                                                       -1,
                                                                       LAN1_STG_DNSV6_MODE_STATIC,
                                                                       true,
                                                                       NORMAL_FONT_SIZE,
                                                                       NORMAL_FONT_COLOR, false,
                                                                       LEFT_MARGIN_FROM_CENTER);

    dnsv6ModeOptionSelButton[DNS_MODE_AUTO] = new OptionSelectButton((dnsv6ModeOptionSelButton[DNS_MODE_STATIC]->x() + SCALE_WIDTH(350) - LEFT_MARGIN_FROM_CENTER),
                                                                     dnsv6ModeOptionSelButton[DNS_MODE_STATIC]->y(),
                                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                                     BGTILE_HEIGHT,
                                                                     RADIO_BUTTON_INDEX,
                                                                     "Automatic",
                                                                     this,
                                                                     NO_LAYER,
                                                                     0,
                                                                     MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                     NORMAL_FONT_SIZE,
                                                                     LAN1_STG_DNSV6_MODE_AUTO);
    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        m_elementList[index + LAN1_STG_DNSV6_MODE_STATIC] = dnsv6ModeOptionSelButton[index];
        connect(dnsv6ModeOptionSelButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(dnsv6ModeOptionSelButton[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
    }

    dnsv6IpAddrSet[DNS_IP] = new IpTextBox(dnsv6ModeOptionSelButton[DNS_MODE_STATIC]->x(),
                                           dnsv6ModeOptionSelButton[DNS_MODE_STATIC]->y() + dnsv6ModeOptionSelButton[DNS_MODE_STATIC]->height(),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           LAN1_STG_DNSV6_IP_ADDR,
                                           dnsIpStr[DNS_IP],
                                           IP_ADDR_TYPE_IPV6_ONLY,
                                           this, MIDDLE_TABLE_LAYER,
                                           true, 0, true, IP_FIELD_TYPE_DNSV6,
                                           IP_TEXTBOX_ULTRALARGE, LEFT_MARGIN_FROM_CENTER);

    dnsv6IpAddrSet[DNS_ALTR_IP] = new IpTextBox(dnsv6IpAddrSet[DNS_IP]->x(),
                                               dnsv6IpAddrSet[DNS_IP]->y() + dnsv6IpAddrSet[DNS_IP]->height(),
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               LAN1_STG_DNSV6_ALTR_IP_ADDR,
                                               dnsIpStr[DNS_ALTR_IP],
                                               IP_ADDR_TYPE_IPV6_ONLY,
                                               this, BOTTOM_TABLE_LAYER,
                                               true, 0, true, IP_FIELD_TYPE_DNSV6,
                                               IP_TEXTBOX_ULTRALARGE, LEFT_MARGIN_FROM_CENTER);
    for(quint8 index = DNS_IP; index < MAX_DNS_IP; index++)
    {
        m_elementList[index + LAN1_STG_DNSV6_IP_ADDR] = dnsv6IpAddrSet[index];
        connect(dnsv6IpAddrSet[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(dnsv6IpAddrSet[index],
                SIGNAL(sigLoadInfopage(quint32)),
                this,
                SLOT(slotIpTextBoxLoadInfopage(quint32)));
    }
}

void Lan1Setting::showHideElements()
{
    bool isVisible = (ipv4AssignMode == IPV4_ASSIGN_MODE_PPPOE) ? true : false;
    ipv4ElementHeadings[IPV4_SETTINGS_PPPOE_MODE]->setVisible(isVisible);
    pppoeUsername->setVisible(isVisible);
    pppoePassword->setVisible(isVisible);
}

void Lan1Setting::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             LAN1_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_LAN1_FIELDS,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Lan1Setting::getMacAddress()
{
    payloadLib->setCnfgArrayAtIndex(0, LAN1_INDEX);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_MAC;
    param->payload = payloadLib->createDevCmdPayload(1);
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Lan1Setting::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             LAN1_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_LAN1_FIELDS,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Lan1Setting::saveConfig()
{
    QString tIpv4InfoStr[MAX_STATIC_IPV4];
    QString tIpv6InfoStr[MAX_STATIC_IPV6];
    QString tempDnsAddr, tempDnsAltrIp;

    for(quint8 index = STATIC_IPV4_ADDR; index < MAX_STATIC_IPV4; index++)
    {
        staticIpv4AddrSet[index]->getIpaddress(tIpv4InfoStr[index]);
    }
    dnsv4IpAddrSet[DNS_IP]->getIpaddress(tempDnsAddr);

    for(quint8 index = STATIC_IPV6_ADDR; index < MAX_STATIC_IPV6; index++)
    {
        staticIpv6AddrSet[index]->getIpaddress(tIpv6InfoStr[index]);
    }

    if((ipv4AssignMode == IPV4_ASSIGN_MODE_STATIC) && (tIpv4InfoStr[STATIC_IPV4_ADDR] == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
    }
    else if((ipv4AssignMode == IPV4_ASSIGN_MODE_STATIC) && (tIpv4InfoStr[STATIC_IPV4_SUBNETMASK] == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_SUBNET_MASK));
    }
    else if((ipv4AssignMode == IPV4_ASSIGN_MODE_STATIC) && (tIpv4InfoStr[STATIC_IPV4_DFLT_GATEWAY] == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
    }
    else if((ipv4AssignMode == IPV4_ASSIGN_MODE_PPPOE) && (pppoeUsername->getInputText () == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_PPPoE_USERNAME));
    }
    else if((ipv4AssignMode == IPV4_ASSIGN_MODE_PPPOE) && ((pppoePassword->getInputText () == "")))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_PPPoE_PASSWORD));
    }
    else if((ipv4AssignMode == IPV4_ASSIGN_MODE_DHCP) && (dnsv4Mode == DNS_MODE_STATIC) &&(tempDnsAddr == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_PREF_DNS_VALUE));
    }
    else if(((IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement() == IP_ADDR_MODE_IPV4_AND_IPV6)
            && (ipv6AssignMode == IPV6_ASSIGN_MODE_STATIC) && (tIpv6InfoStr[STATIC_IPV6_ADDR] == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
    }
    else if(((IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement() == IP_ADDR_MODE_IPV4_AND_IPV6)
            && (ipv6AssignMode == IPV6_ASSIGN_MODE_STATIC) && (tIpv6InfoStr[STATIC_IPV6_DFLT_GATEWAY] == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
    }
    else
    {
        currIpAddrMode = (IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement();
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_ADDR_MODE, currIpAddrMode);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_IPV4_ASSIGN_MODE, ipv4AssignMode);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_STATIC_IPV4_ADDR, tIpv4InfoStr[STATIC_IPV4_ADDR]);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_STATIC_IPV4_SUBNET, tIpv4InfoStr[STATIC_IPV4_SUBNETMASK]);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_STATIC_IPV4_DFLT_GATEWAY, tIpv4InfoStr[STATIC_IPV4_DFLT_GATEWAY]);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_PPPOE_USRNAME, pppoeUsername->getInputText());
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_PPPOE_PASSWORD, pppoePassword->getInputText());

        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_DNSV4_SER_MODE, dnsv4Mode);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_DNSV4_IP, tempDnsAddr);
        dnsv4IpAddrSet[DNS_ALTR_IP]->getIpaddress(tempDnsAltrIp);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_DNSV4_ALTR_IP, tempDnsAltrIp);

        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_IPV6_ASSIGN_MODE, ipv6AssignMode);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_IPV6_ADDRESS, tIpv6InfoStr[STATIC_IPV6_ADDR]);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_IPV6_PREFIX_LEN, ipv6PrefixLen->getInputText());
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_IPV6_GATEWAY, tIpv6InfoStr[STATIC_IPV6_DFLT_GATEWAY]);

        dnsv6IpAddrSet[DNS_IP]->getIpaddress(tempDnsAddr);
        dnsv6IpAddrSet[DNS_ALTR_IP]->getIpaddress(tempDnsAltrIp);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_DNSV6_MODE, dnsv6Mode);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_DNSV6_PRIMARY_ADDRESS, tempDnsAddr);
        payloadLib->setCnfgArrayAtIndex(LAN1_FIELD_DNSV6_SECONDARY_ADDRESS, tempDnsAltrIp);

        //create the payload for Get Cnfg
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 LAN1_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_TO_INDEX,
                                                                 CNFG_FRM_FIELD,
                                                                 MAX_LAN1_FIELDS,
                                                                 MAX_LAN1_FIELDS);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;
        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void Lan1Setting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        update();
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex() != LAN1_TABLE_INDEX)
            {
                break;
            }

            // fill all field value
            if (isIpAddrDropDwnChng == false)
            {
                currIpAddrMode = (IP_ADDR_MODE_e)payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_ADDR_MODE).toUInt();
            }

            isIpAddrDropDwnChng = false;
            ipAddrModeDrpDwn->setIndexofCurrElement(currIpAddrMode);

            ipv4AssignMode = (IPV4_ASSIGN_MODE_e)payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_IPV4_ASSIGN_MODE).toUInt();
            staticIpv4AddrSet[STATIC_IPV4_ADDR]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_STATIC_IPV4_ADDR).toString());
            staticIpv4AddrSet[STATIC_IPV4_SUBNETMASK]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_STATIC_IPV4_SUBNET).toString());
            staticIpv4AddrSet[STATIC_IPV4_DFLT_GATEWAY]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_STATIC_IPV4_DFLT_GATEWAY).toString());

            dnsv4Mode = (DNS_MODE_e)payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_DNSV4_SER_MODE).toUInt();
            dnsv4IpAddrSet[DNS_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_DNSV4_IP).toString());
            dnsv4IpAddrSet[DNS_ALTR_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_DNSV4_ALTR_IP).toString());

            ipv6AssignMode = (IPV6_ASSIGN_MODE_e)payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_IPV6_ASSIGN_MODE).toUInt();
            staticIpv6AddrSet[STATIC_IPV6_ADDR]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_IPV6_ADDRESS).toString());
            ipv6PrefixLen->setInputText(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_IPV6_PREFIX_LEN).toString());
            staticIpv6AddrSet[STATIC_IPV6_DFLT_GATEWAY]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_IPV6_GATEWAY).toString());

            dnsv6Mode = (DNS_MODE_e)payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_DNSV6_MODE).toUInt();
            dnsv6IpAddrSet[DNS_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_DNSV6_PRIMARY_ADDRESS).toString());
            dnsv6IpAddrSet[DNS_ALTR_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_DNSV6_SECONDARY_ADDRESS).toString());

            selectIpAssignMode();
            getMacAddress();

            pppoeUsername->setInputText(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_PPPOE_USRNAME).toString());
            pppoePassword->setInputText(payloadLib->getCnfgArrayAtIndex(LAN1_FIELD_PPPOE_PASSWORD).toString());
        }
        break;

        case MSG_SET_CFG:
        {
            // unload processing icon
            processBar->unloadProcessBar();

            //load info page with msg
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
        }
        break;

        case MSG_DEF_CFG:
        {
            processBar->unloadProcessBar();
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            processBar->unloadProcessBar();
            if (param->cmdType != GET_MAC)
            {
                break;
            }

            payloadLib->parseDevCmdReply(true, param->payload);
            macaddresReadOnly->changeValue(payloadLib->getCnfgArrayAtIndex(0).toString());
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    update();
}

void Lan1Setting::selectIpAssignMode()
{
    for(quint8 index = IPV4_ASSIGN_MODE_STATIC; index < MAX_IPV4_ASSIGN_MODE; index++)
    {
        ipv4ModeOptionSelButton[index]->changeState((ipv4AssignMode == index) ? ON_STATE : OFF_STATE);
    }

    for(quint8 index = IPV6_ASSIGN_MODE_STATIC; index < MAX_IPV6_ASSIGN_MODE; index++)
    {
        ipv6ModeOptionSelButton[index]->changeState((ipv6AssignMode == index) ? ON_STATE : OFF_STATE);
    }

    switch(ipv4AssignMode)
    {
        case IPV4_ASSIGN_MODE_STATIC:
            for(quint8 index = STATIC_IPV4_ADDR; index < MAX_STATIC_IPV4; index++)
            {
                staticIpv4AddrSet[index]->setIsEnabled(true);
            }

            dnsv4Mode = DNS_MODE_STATIC;
            setDnsSerMode();

            for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
            {
                dnsv4ModeOptionSelButton[index]->setIsEnabled(false);
            }
            break;

        case IPV4_ASSIGN_MODE_DHCP:
            for(quint8 index = STATIC_IPV4_ADDR; index < MAX_STATIC_IPV4; index++)
            {
                staticIpv4AddrSet[index]->setIsEnabled(false);
            }
            for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
            {
                dnsv4ModeOptionSelButton[index]->setIsEnabled(true);
            }
            setDnsSerMode();
            break;

        case IPV4_ASSIGN_MODE_PPPOE:
            for(quint8 index = STATIC_IPV4_ADDR; index < MAX_STATIC_IPV4; index++)
            {
                staticIpv4AddrSet[index]->setIsEnabled(false);
            }
            for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
            {
                dnsv4ModeOptionSelButton[index]->setIsEnabled(true);
            }
            setDnsSerMode();
            break;

        default:
            break;
    }

    showHideElements();
    enableDisableIpv6Ctrls();

    if(currIpAddrMode != IP_ADDR_MODE_IPV4_AND_IPV6)
    {
        return;
    }

    switch(ipv6AssignMode)
    {
        case IPV6_ASSIGN_MODE_STATIC:
            for(quint8 index = STATIC_IPV6_ADDR; index < MAX_STATIC_IPV6; index++)
            {
                staticIpv6AddrSet[index]->setIsEnabled(true);
            }
            ipv6PrefixLen->setIsEnabled(true);

            dnsv6Mode = DNS_MODE_STATIC;
            setDnsSerMode();

            for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
            {
                dnsv6ModeOptionSelButton[index]->setIsEnabled(false);
            }
            break;

        case IPV6_ASSIGN_MODE_DHCP:
            for(quint8 index = STATIC_IPV6_ADDR; index < MAX_STATIC_IPV6; index++)
            {
                staticIpv6AddrSet[index]->setIsEnabled(false);
            }
            ipv6PrefixLen->setIsEnabled(false);

            for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
            {
                dnsv6ModeOptionSelButton[index]->setIsEnabled(true);
            }
            setDnsSerMode();
            break;

        case IPV6_ASSIGN_MODE_SLAAC:
            for(quint8 index = STATIC_IPV6_ADDR; index < MAX_STATIC_IPV6; index++)
            {
                staticIpv6AddrSet[index]->setIsEnabled(false);
            }
            ipv6PrefixLen->setIsEnabled(false);

            for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
            {
                dnsv6ModeOptionSelButton[index]->setIsEnabled(true);
            }
            setDnsSerMode();
            break;

        default:
            break;
    }
}

void Lan1Setting::setDnsSerMode()
{
    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        dnsv4ModeOptionSelButton[index]->changeState((dnsv4Mode == index) ? ON_STATE : OFF_STATE);
    }

    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        dnsv6ModeOptionSelButton[index]->changeState((dnsv6Mode == index) ? ON_STATE : OFF_STATE);
    }

    dnsv4IpAddrSet[DNS_IP]->setIsEnabled((dnsv4Mode == DNS_MODE_AUTO) ? false : true);
    dnsv4IpAddrSet[DNS_ALTR_IP]->setIsEnabled((dnsv4Mode == DNS_MODE_AUTO) ? false : true);

    if(currIpAddrMode != IP_ADDR_MODE_IPV4_AND_IPV6)
    {
        return;
    }

    dnsv6IpAddrSet[DNS_IP]->setIsEnabled((dnsv6Mode == DNS_MODE_AUTO) ? false : true);
    dnsv6IpAddrSet[DNS_ALTR_IP]->setIsEnabled((dnsv6Mode == DNS_MODE_AUTO) ? false : true);
}

void Lan1Setting::enableDisableIpv6Ctrls()
{
    bool isEnable = (currIpAddrMode == IP_ADDR_MODE_IPV4_AND_IPV6) ? true : false;

    ipv6PrefixLen->setIsEnabled(isEnable);
    for(quint8 index = IPV6_ASSIGN_MODE_STATIC; index < MAX_IPV6_ASSIGN_MODE; index++)
    {
        ipv6ModeOptionSelButton[index]->setIsEnabled(isEnable);
    }

    for(quint8 index = STATIC_IPV6_ADDR; index < MAX_STATIC_IPV6; index++)
    {
        staticIpv6AddrSet[index]->setIsEnabled(isEnable);
    }

    for(quint8 index = DNS_MODE_STATIC; index < MAX_DNS_MODE; index++)
    {
        dnsv6ModeOptionSelButton[index]->setIsEnabled(isEnable);
    }

    for(quint8 index = DNS_IP; index < MAX_DNS_IP; index++)
    {
        dnsv6IpAddrSet[index]->setIsEnabled(isEnable);
    }
}

void Lan1Setting::handleInfoPageMessage(int index)
{
    if (index != INFO_OK_BTN)
    {
        return;
    }
}

void Lan1Setting::slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e state, int index)
{
    if(state != ON_STATE)
    {
        return;
    }

    switch(index)
    {
        case LAN1_STG_IPV4_ASSIGN_STATIC:
            ipv4AssignMode = IPV4_ASSIGN_MODE_STATIC;
            selectIpAssignMode();
            break;

        case LAN1_STG_IPV4_ASSIGN_DHCP:
            ipv4AssignMode = IPV4_ASSIGN_MODE_DHCP;
            selectIpAssignMode();
            break;

        case LAN1_STG_IPV4_ASSIGN_PPPOE:
            ipv4AssignMode = IPV4_ASSIGN_MODE_PPPOE;
            selectIpAssignMode();
            break;

        case LAN1_STG_DNSV4_MODE_STATIC:
            dnsv4Mode = DNS_MODE_STATIC;
            setDnsSerMode();
            break;

        case LAN1_STG_DNSV4_MODE_AUTO:
            dnsv4Mode = DNS_MODE_AUTO;
            setDnsSerMode();
            break;

        case LAN1_STG_IPV6_ASSIGN_STATIC:
            ipv6AssignMode = IPV6_ASSIGN_MODE_STATIC;
            selectIpAssignMode();
            break;

        case LAN1_STG_IPV6_ASSIGN_DHCP:
            ipv6AssignMode = IPV6_ASSIGN_MODE_DHCP;
            selectIpAssignMode();
            break;

        case LAN1_STG_IPV6_ASSIGN_SLAAC:
            ipv6AssignMode = IPV6_ASSIGN_MODE_SLAAC;
            selectIpAssignMode();
            break;

        case LAN1_STG_DNSV6_MODE_STATIC:
            dnsv6Mode = DNS_MODE_STATIC;
            setDnsSerMode();
            break;

        case LAN1_STG_DNSV6_MODE_AUTO:
            dnsv6Mode = DNS_MODE_AUTO;
            setDnsSerMode();
            break;

        default:
            break;
    }
}

void Lan1Setting::slotIpTextBoxLoadInfopage(quint32 index)
{
    switch(index)
    {
        case LAN1_STG_IPV4_STATIC_IP_ADDR:
        case LAN1_STG_IPV6_STATIC_IP_ADDR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_IP_ADDR));
            break;

        case LAN1_STG_IPV4_STATIC_SUBNETMASK:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_SUBNET_MASK));
            break;

        case LAN1_STG_IPV4_STATIC_DFLT_GATEWAY:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_GATEWAY_MASK));
            break;

        case LAN1_STG_DNSV6_IP_ADDR:
        case LAN1_STG_DNSV4_IP_ADDR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_PREF_DNS_ADDR));
            break;

        case LAN1_STG_DNSV6_ALTR_IP_ADDR:
        case LAN1_STG_DNSV4_ALTR_IP_ADDR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_ALTR_DNS_ADDR));
            break;

        case LAN1_STG_IPV6_STATIC_DFLT_GATEWAY:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_GATEWAY_MASK));
            break;

        default:
            break;
    }
}

void Lan1Setting::slotIpTextBoxEntryDone(quint32)
{
    QString gateWay = staticIpv4AddrSet[STATIC_IPV4_ADDR]->getSubnetOfIpaddress();
    if(gateWay == "0.0.0")
    {
        getConfig();
		return;
    }

    gateWay.append(".").append("1");
    staticIpv4AddrSet[STATIC_IPV4_DFLT_GATEWAY]->setIpaddress(gateWay);
}

void Lan1Setting::slotSpinBoxValueChanged(QString, quint32 index)
{
    if (index != LAN1_STG_IP_ADDR_MODE_DRP_DWN)
    {
        return;
    }

    currIpAddrMode = (IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement();
    selectIpAssignMode();
    isIpAddrDropDwnChng = true;
    getConfig();
}
