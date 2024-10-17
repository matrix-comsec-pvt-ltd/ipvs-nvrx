#include "NetworkSetting.h"
#include "ValidationMessage.h"
#include <QPainter>

#define WIZARD_BG_TILE_HEIGHT       SCALE_HEIGHT(50)
#define WIZARD_BG_TILE_WIDTH        SCALE_WIDTH(945)
#define IPV4_MODE_ASSIGN_STRING     "IPv4 Assignment Mode"
#define IPV6_MODE_ASSIGN_STRING     "IPv6 Assignment Mode"
#define DROP_DOWN_BOX_LABEL         "LAN"
#define DNSV4_MODE_STRING           "DNSv4 Mode"
#define DNSV6_MODE_STRING           "DNSv6 Mode"

#define CNFG_TO_INDEX               1
#define CNFG_FRM_INDEX              1
#define LAN1_INDEX                  1
#define CNFG_FRM_FIELD              1

static const QStringList ipAddrModeList= QStringList() << "IPv4" << "IPv4 & IPv6";

static const QString ipv4AssignModeStr[MAX_LAN]                     = { "Static", "DHCP" };
static const QString staticIpv4Str[WIZ_MAX_STATIC_IPV4]             = { "IPv4 Address", "Subnet Mask", "Default Gateway" };
static const QString dnsIpStr[WIZ_MAX_DNS_IP]                       = { "Preferred DNS", "Alternate DNS" };
static const QString lanSettingString[MAX_LAN]                      = { "LAN 1", "LAN 2"};
static const QString ipv6AssignModeStr[WIZ_MAX_IPV6_ASSIGN_MODE]    = { "Static", "DHCP", "SLAAC" };
static const QString staticIpv6Str[WIZ_MAX_STATIC_IPV6]             = { "IPv6 Address", "Default Gateway" };

typedef enum
{
    WIZ_LAN_STG_LAN_DRP_DWN,
    WIZ_LAN_STG_IP_ADDR_MODE_DRP_DWN,
    WIZ_LAN_STG_IPV4_ASSIGN_STATIC,
    WIZ_LAN_STG_IPV4_ASSIGN_DHCP,
    WIZ_LAN_STG_IPV4_STATIC_IP_ADDR,
    WIZ_LAN_STG_IPV4_STATIC_SUBNETMASK,
    WIZ_LAN_STG_IPV4_STATIC_DFLT_GATEWAY,
    WIZ_LAN_STG_DNSV4_MODE_STATIC,
    WIZ_LAN_STG_DNSV4_MODE_AUTO,
    WIZ_LAN_STG_DNSV4_IP,
    WIZ_LAN_STG_DNSV4_ALTR_IP,
    WIZ_LAN_STG_IPV6_ASSIGN_STATIC,
    WIZ_LAN_STG_IPV6_ASSIGN_DHCP,
    WIZ_LAN_STG_IPV6_ASSIGN_SLAAC,
    WIZ_LAN_STG_IPV6_STATIC_IP_ADDR,
    WIZ_LAN_STG_IPV6_STATIC_PREFIX_LEN,
    WIZ_LAN_STG_IPV6_STATIC_DFLT_GATEWAY,
    WIZ_LAN_STG_DNSV6_MODE_STATIC,
    WIZ_LAN_STG_DNSV6_MODE_AUTO,
    WIZ_LAN_STG_DNSV6_IP_ADDR,
    WIZ_LAN_STG_DNSV6_ALTR_IP_ADDR,

    MAX_WIZ_LAN_STG_ELEMETS
}WIZ_LAN_STG_ELELIST_e;


NetworkSetting::NetworkSetting(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId)
    : WizardCommon(parent, pageId), ipv4AssignMode(WIZ_MAX_IPV4_ASSIGN_MODE), ipv6AssignMode(WIZ_MAX_IPV6_ASSIGN_MODE),
      dnsv4Mode(WIZ_MAX_DNS_MODE), dnsv6Mode(WIZ_MAX_DNS_MODE)
{
    lanSelected = SELECT_LAN1;
    m_nextBtnClick = false;
    currIpAddrMode = WIZ_MAX_IP_ADDR_MODE;
    isIpAddrDropDwnChng = false;
    currDevName = devName;
    applController = ApplController::getInstance ();
    applController->GetDeviceInfo(currDevName, devTableInfo);

    WizardCommon :: LoadProcessBar();
    createDefaultElements(subHeadStr);
    displayLan1Lan2();
    this->show();
}

void NetworkSetting::createDefaultElements(QString subHeadStr)
{
    INIT_OBJ(m_networkSettingHeading);
    INIT_OBJ(m_lanDropDownBox);
    INIT_OBJ(ipv4ModeOptionSelButton[WIZ_IPV4_ASSIGN_MODE_STATIC]);
    INIT_OBJ(ipv4ModeOptionSelButton[WIZ_IPV4_ASSIGN_MODE_DHCP]);
    INIT_OBJ(staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]);
    INIT_OBJ(staticIpv4AddrSet[WIZ_STATIC_IPV4_SUBNETMASK]);
    INIT_OBJ(staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY]);
    INIT_OBJ(dnsv4ModeOptionSelButton[WIZ_DNS_MODE_STATIC]);
    INIT_OBJ(dnsv4ModeOptionSelButton[WIZ_DNS_MODE_AUTO]);
    INIT_OBJ(dnsv4IpAddrSet[WIZ_DNS_IP]);
    INIT_OBJ(dnsv4IpAddrSet[WIZ_DNS_ALTR_IP]);
    INIT_OBJ(m_infoPage);
    INIT_OBJ(payloadLib);

    for(quint8 index = WIZ_IPV6_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV6_ASSIGN_MODE; index++)
    {
        INIT_OBJ(ipv6ModeOptionSelButton[index]);
    }

    for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
    {
        INIT_OBJ(staticIpv6AddrSet[index]);
    }

    INIT_OBJ(ipv6PrefixLen);
    INIT_OBJ(prefixLenParam);

    for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
    {
        INIT_OBJ(dnsv6ModeOptionSelButton[index]);
    }

    for(quint8 index = WIZ_DNS_IP; index < WIZ_MAX_DNS_IP; index++)
    {
        INIT_OBJ(dnsv6IpAddrSet[index]);
    }

    applController = ApplController::getInstance();
    payloadLib = new PayloadLib();

    m_networkSettingHeading = new TextLabel((SCALE_WIDTH(500)),
                                      (SCALE_HEIGHT(23)),
                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                      subHeadStr,
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y,
                                      0,
                                      false,
                                      WIZARD_BG_TILE_WIDTH,
                                      0);
    QMap<quint8, QString>  lanList;
    lanList.clear ();
    lanList.insert (SELECT_LAN1,lanSettingString[SELECT_LAN1]);
    lanList.insert (SELECT_LAN2,lanSettingString[SELECT_LAN2]);

    m_lanDropDownBox = new DropDown(SCALE_WIDTH(50),
                                    SCALE_HEIGHT(50),
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    WIZ_LAN_STG_LAN_DRP_DWN,
                                    DROPDOWNBOX_SIZE_225,
                                    DROP_DOWN_BOX_LABEL,
                                    lanList,
                                    this,
                                    "",
                                    true);

    if(devTableInfo.numOfLan == MAX_LAN)
    {
        connect(m_lanDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownBoxValueChanged(QString,quint32)));
    }
    else
    {
        m_lanDropDownBox->setEnabled(false);
    }

    QMap<quint8, QString> ipAddrModeListMap;
    for(quint8 index = 0; index < ipAddrModeList.length(); index++)
    {
        ipAddrModeListMap.insert(index,ipAddrModeList.at(index));
    }

    ipAddrModeDrpDwn = new DropDown(m_lanDropDownBox->x(),
                                    m_lanDropDownBox->y() + BGTILE_HEIGHT,
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    WIZ_LAN_STG_IP_ADDR_MODE_DRP_DWN,
                                    DROPDOWNBOX_SIZE_225,
                                    "IP Addressing Mode",
                                    ipAddrModeListMap,
                                    this,
                                    "",
                                    true);
    connect(ipAddrModeDrpDwn,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotDropDownBoxValueChanged(QString,quint32)));

    m_infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(1145),
                             SCALE_HEIGHT(750),
                             MAX_INFO_PAGE_TYPE,
                             parentWidget());
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    this->show();
}

void NetworkSetting :: deleteDynamicElement()
{
    for(quint8 index = WIZ_IPV4_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV4_ASSIGN_MODE; index++)
    {
        if(IS_VALID_OBJ(ipv4ModeOptionSelButton[index]))
        {
            disconnect (ipv4ModeOptionSelButton[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
            DELETE_OBJ(ipv4ModeOptionSelButton[index]);
        }
    }

    if(IS_VALID_OBJ(staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]))
    {
        disconnect (staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR],
                    SIGNAL(sigEntryDone(quint32)),
                    this,
                    SLOT(slotIpTextBoxEntryDone(quint32)));
    }

    for(quint8 index = WIZ_STATIC_IPV4_ADDR; index < WIZ_MAX_STATIC_IPV4; index++)
    {
        if(IS_VALID_OBJ(staticIpv4AddrSet[index]))
        {
            disconnect (staticIpv4AddrSet[index],
                        SIGNAL(sigLoadInfopage(quint32)),
                        this,
                        SLOT(slotIpTextBoxLoadInfopage(quint32)));
            DELETE_OBJ(staticIpv4AddrSet[index]);
        }
    }

    for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
    {
        if(IS_VALID_OBJ(dnsv4ModeOptionSelButton[index]))
        {
            disconnect (dnsv4ModeOptionSelButton[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
            DELETE_OBJ(dnsv4ModeOptionSelButton[index]);
        }
    }

    for(quint8 index = WIZ_DNS_IP; index < WIZ_MAX_DNS_IP; index++)
    {
        if(IS_VALID_OBJ(dnsv4IpAddrSet[index]))
        {
            disconnect (dnsv4IpAddrSet[index],
                        SIGNAL(sigLoadInfopage(quint32)),
                        this,
                        SLOT(slotIpTextBoxLoadInfopage(quint32)));
            DELETE_OBJ(dnsv4IpAddrSet[index]);
        }
    }

    for(quint8 index = WIZ_IPV6_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV6_ASSIGN_MODE; index++)
    {
        if (IS_VALID_OBJ(ipv6ModeOptionSelButton[index]))
        {
            disconnect (ipv6ModeOptionSelButton[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));

            DELETE_OBJ(ipv6ModeOptionSelButton[index]);
        }
    }

    for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
    {
        if (IS_VALID_OBJ(staticIpv6AddrSet[index]))
        {
            disconnect (staticIpv6AddrSet[index],
                        SIGNAL(sigLoadInfopage(quint32)),
                        this,
                        SLOT(slotIpTextBoxLoadInfopage(quint32)));

            DELETE_OBJ(staticIpv6AddrSet[index]);
        }
    }

    DELETE_OBJ(ipv6PrefixLen);
    DELETE_OBJ(prefixLenParam);

    for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
    {
        if (IS_VALID_OBJ(dnsv6ModeOptionSelButton[index]))
        {
            disconnect (dnsv6ModeOptionSelButton[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));

            DELETE_OBJ(dnsv6ModeOptionSelButton[index]);
        }
    }

    for(quint8 index = WIZ_DNS_IP; index < WIZ_MAX_DNS_IP; index++)
    {
        if (IS_VALID_OBJ(dnsv6IpAddrSet[index]))
        {
            disconnect (dnsv6IpAddrSet[index],
                        SIGNAL(sigLoadInfopage(quint32)),
                        this,
                        SLOT(slotIpTextBoxLoadInfopage(quint32)));

            DELETE_OBJ(dnsv6IpAddrSet[index]);
        }
    }
}

NetworkSetting::~NetworkSetting()
{
    WizardCommon::UnloadProcessBar();
    deleteDynamicElement();
    DELETE_OBJ(m_infoPage);
    DELETE_OBJ(m_networkSettingHeading);

    if (IS_VALID_OBJ(ipAddrModeDrpDwn))
    {
        disconnect(ipAddrModeDrpDwn,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownBoxValueChanged(QString,quint32)));

        DELETE_OBJ(ipAddrModeDrpDwn);
    }

    if(IS_VALID_OBJ(m_lanDropDownBox))
    {
        disconnect(m_lanDropDownBox,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotDropDownBoxValueChanged(QString,quint32)));
        DELETE_OBJ(m_lanDropDownBox);
    }

    DELETE_OBJ(payloadLib);
}

void NetworkSetting::getLan2Config()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             LAN2_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             WIZ_MAX_LAN2_FIELDS,
                                                             WIZ_MAX_LAN2_FIELDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void NetworkSetting::getLan1Config()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             LAN1_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             WIZ_MAX_LAN1_FIELDS,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void NetworkSetting::defaultLan2Config ()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             LAN2_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             WIZ_MAX_LAN2_FIELDS,
                                                             WIZ_MAX_LAN2_FIELDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void NetworkSetting::defaultLan1Config()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             LAN1_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             WIZ_MAX_LAN1_FIELDS,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void NetworkSetting::saveConfig()
{
    if(m_getConfig == false)
    {
        return;
    }

    if(lanSelected == SELECT_LAN1)
    {
        QString tIpv4InfoStr[WIZ_MAX_STATIC_IPV4];
        QString tIpv6InfoStr[WIZ_MAX_STATIC_IPV6];
        QString tempDnsAddr, tempDnsAltrIp;
        QString userName, passWord;

        for(quint8 index = WIZ_STATIC_IPV4_ADDR; index < WIZ_MAX_STATIC_IPV4; index++)
        {
            staticIpv4AddrSet[index]->getIpaddress(tIpv4InfoStr[index]);
        }

        dnsv4IpAddrSet[WIZ_DNS_IP]->getIpaddress(tempDnsAddr);
        for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
        {
            staticIpv6AddrSet[index]->getIpaddress(tIpv6InfoStr[index]);
        }

        if (ipv4AssignMode == WIZ_IPV4_ASSIGN_MODE_STATIC)
        {
            if (tIpv4InfoStr[WIZ_STATIC_IPV4_ADDR] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan1Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR));
                return;
            }

            if (tIpv4InfoStr[WIZ_STATIC_IPV4_SUBNETMASK] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan1Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_SUBNET_MASK));
                return;
            }

            if (tIpv4InfoStr[WIZ_STATIC_IPV4_DFLT_GATEWAY] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan1Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
                return;
            }
        }

        if ((ipv4AssignMode == WIZ_IPV4_ASSIGN_MODE_DHCP) && (dnsv4Mode == WIZ_DNS_MODE_STATIC) && (tempDnsAddr == ""))
        {
            m_lanDropDownBox->setIndexofCurrElement(lanSelected);
            getLan1Config();
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LAN_SETT_ENT_PREF_DNS_VALUE));
            return;
        }

        if (((WIZ_IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement() == WIZ_IP_ADDR_MODE_IPV4_AND_IPV6) && (ipv6AssignMode == WIZ_IPV6_ASSIGN_MODE_STATIC))
        {
            if (tIpv6InfoStr[WIZ_STATIC_IPV6_ADDR] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan1Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR));
                return;
            }

            if (tIpv6InfoStr[WIZ_STATIC_IPV6_DFLT_GATEWAY] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan1Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
                return;
            }
        }

        dnsv4IpAddrSet[WIZ_DNS_ALTR_IP]->getIpaddress (tempDnsAltrIp);
        currIpAddrMode = (WIZ_IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement();
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_ADDR_MODE, currIpAddrMode);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_ASSIGN_MODE, ipv4AssignMode);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_ADDRESS, tIpv4InfoStr[WIZ_STATIC_IPV4_ADDR]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_SUBNETMASK, tIpv4InfoStr[WIZ_STATIC_IPV4_SUBNETMASK]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_DFLT_GATEWAY, tIpv4InfoStr[WIZ_STATIC_IPV4_DFLT_GATEWAY]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_PPPOE_USRNAME, userName);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_PPPOE_PASSWORD, passWord);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV4_SER_MODE, dnsv4Mode);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV4_IP, tempDnsAddr);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV4_ALTR_IP, tempDnsAltrIp);

        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_ASSIGN_MODE, ipv6AssignMode);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_ADDRESS, tIpv6InfoStr[WIZ_STATIC_IPV6_ADDR]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_PREFIX_LEN, ipv6PrefixLen->getInputText());
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_GATEWAY, tIpv6InfoStr[WIZ_STATIC_IPV6_DFLT_GATEWAY]);

        dnsv6IpAddrSet[WIZ_DNS_IP]->getIpaddress (tempDnsAddr);
        dnsv6IpAddrSet[WIZ_DNS_ALTR_IP]->getIpaddress (tempDnsAltrIp);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV6_MODE, dnsv6Mode);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV6_IP, tempDnsAddr);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV6_ALTR_IP, tempDnsAltrIp);

        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 LAN1_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_TO_INDEX,
                                                                 CNFG_FRM_FIELD,
                                                                 WIZ_MAX_LAN1_FIELDS,
                                                                 WIZ_MAX_LAN1_FIELDS);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;
        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
    else if(lanSelected  == SELECT_LAN2)
    {
        QString tIpv4InfoStr[WIZ_MAX_STATIC_IPV4];
        QString tIpv6InfoStr[WIZ_MAX_STATIC_IPV6];

        for(quint8 index = WIZ_STATIC_IPV4_ADDR; index < WIZ_MAX_STATIC_IPV4; index++)
        {
            staticIpv4AddrSet[index]->getIpaddress (tIpv4InfoStr[index]);
        }

        for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
        {
            staticIpv6AddrSet[index]->getIpaddress(tIpv6InfoStr[index]);
        }

        if(tIpv4InfoStr[WIZ_STATIC_IPV4_ADDR] == "")
        {
            m_lanDropDownBox->setIndexofCurrElement(lanSelected);
            getLan2Config();
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR));
            return;
        }
        else if(tIpv4InfoStr[WIZ_STATIC_IPV4_SUBNETMASK] == "")
        {
            m_lanDropDownBox->setIndexofCurrElement(lanSelected);
            getLan2Config();
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_SUBNET_MASK));
            return;
        }
        else if(tIpv4InfoStr[WIZ_STATIC_IPV4_DFLT_GATEWAY] == "")
        {
            m_lanDropDownBox->setIndexofCurrElement(lanSelected);
            getLan2Config();
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
            return;
        }
        else if(((WIZ_IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement() == WIZ_IP_ADDR_MODE_IPV4_AND_IPV6))
        {
            if (tIpv6InfoStr[WIZ_STATIC_IPV6_ADDR] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan2Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR));
                return;
            }

            if (tIpv6InfoStr[WIZ_STATIC_IPV6_DFLT_GATEWAY] == "")
            {
                m_lanDropDownBox->setIndexofCurrElement(lanSelected);
                getLan2Config();
                WizardCommon::InfoPageImage();
                m_infoPage->raise();
                m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
                return;
            }
        }

        currIpAddrMode = (WIZ_IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement();
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_ADDR_MODE, currIpAddrMode);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_IP_ADDRESS, tIpv4InfoStr[WIZ_STATIC_IPV4_ADDR]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_SUBNET, tIpv4InfoStr[WIZ_STATIC_IPV4_SUBNETMASK]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_GATEWAY, tIpv4InfoStr[WIZ_STATIC_IPV4_DFLT_GATEWAY]);

        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_IPV6_ADDRESS, tIpv6InfoStr[WIZ_STATIC_IPV6_ADDR]);
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_IPV6_PREFIX_LEN, ipv6PrefixLen->getInputText());
        payloadLib->setCnfgArrayAtIndex (WIZ_LAN2_FIELD_IPV6_GATEWAY, tIpv6InfoStr[WIZ_STATIC_IPV6_DFLT_GATEWAY]);

        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 LAN2_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 WIZ_MAX_LAN2_FIELDS,
                                                                 WIZ_MAX_LAN2_FIELDS);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;
        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}
void NetworkSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    QString userName, passWord;
    if(deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
                case MSG_GET_CFG:
                {
                    payloadLib->parsePayload(param->msgType, param->payload);
                    if(payloadLib->getcnfgTableIndex () == LAN1_TABLE_INDEX)
                    {
                        if (isIpAddrDropDwnChng == false)
                        {
                            currIpAddrMode = (WIZ_IP_ADDR_MODE_e)payloadLib->getCnfgArrayAtIndex(WIZ_LAN1_FIELD_ADDR_MODE).toUInt();
                        }

                        ipAddrModeDrpDwn->setIndexofCurrElement(currIpAddrMode);
                        ipv4AssignMode = (WIZ_IPV4_ASSIGN_MODE_e)payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_ASSIGN_MODE).toUInt ();
                        staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_ADDRESS).toString ());
                        staticIpv4AddrSet[WIZ_STATIC_IPV4_SUBNETMASK]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_SUBNETMASK).toString ());
                        staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV4_DFLT_GATEWAY).toString ());
                        dnsv4Mode = (WIZ_DNS_MODE_e)payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV4_SER_MODE).toUInt ();
                        dnsv4IpAddrSet[WIZ_DNS_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV4_IP).toString ());
                        dnsv4IpAddrSet[WIZ_DNS_ALTR_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV4_ALTR_IP).toString ());
                        userName = payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_PPPOE_USRNAME).toString ();
                        passWord = payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_PPPOE_PASSWORD).toString ();

                        ipv6AssignMode = (WIZ_IPV6_ASSIGN_MODE_e)payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_ASSIGN_MODE).toUInt ();
                        staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_ADDRESS).toString ());
                        ipv6PrefixLen->setInputText (QString::number(payloadLib->getCnfgArrayAtIndex(WIZ_LAN1_FIELD_IPV6_PREFIX_LEN).toUInt()));
                        staticIpv6AddrSet[WIZ_STATIC_IPV6_DFLT_GATEWAY]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_IPV6_GATEWAY).toString ());
                        dnsv6Mode = (WIZ_DNS_MODE_e)payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV6_MODE).toUInt ();
                        dnsv6IpAddrSet[WIZ_DNS_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV6_IP).toString ());
                        dnsv6IpAddrSet[WIZ_DNS_ALTR_IP]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN1_FIELD_DNSV6_ALTR_IP).toString ());
                    }

                    if(payloadLib->getcnfgTableIndex () == LAN2_TABLE_INDEX)
                    {
                        if (isIpAddrDropDwnChng == false)
                        {
                            currIpAddrMode = (WIZ_IP_ADDR_MODE_e)payloadLib->getCnfgArrayAtIndex(WIZ_LAN2_FIELD_ADDR_MODE).toUInt();
                        }

                        ipAddrModeDrpDwn->setIndexofCurrElement(currIpAddrMode);
                        staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->setIpaddress (payloadLib->getCnfgArrayAtIndex (WIZ_LAN2_FIELD_IP_ADDRESS).toString ());
                        staticIpv4AddrSet[WIZ_STATIC_IPV4_SUBNETMASK]->setIpaddress (payloadLib->getCnfgArrayAtIndex (WIZ_LAN2_FIELD_SUBNET).toString ());
                        staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY]->setIpaddress (payloadLib->getCnfgArrayAtIndex (WIZ_LAN2_FIELD_GATEWAY).toString ());

                        staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN2_FIELD_IPV6_ADDRESS).toString ());
                        ipv6PrefixLen->setInputText (QString::number(payloadLib->getCnfgArrayAtIndex(WIZ_LAN2_FIELD_IPV6_PREFIX_LEN).toUInt()));
                        staticIpv6AddrSet[WIZ_STATIC_IPV6_DFLT_GATEWAY]->setIpaddress(payloadLib->getCnfgArrayAtIndex (WIZ_LAN2_FIELD_IPV6_GATEWAY).toString ());
                    }

                    selectIpAssignMode();
                    m_getConfig = true;
                    isIpAddrDropDwnChng = false;
                }
                break;

                case MSG_SET_CFG:
                {
                    displayLan1Lan2();
                    if(m_nextBtnClick == true)
                    {
                        m_deletePage = true;
                        m_nextBtnClick = false;
                    }
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                }
                break;

                case MSG_DEF_CFG:
                {
                    if(lanSelected == SELECT_LAN1)
                    {
                        getLan1Config();
                    }
                    else if(lanSelected == SELECT_LAN2)
                    {
                        getLan2Config();
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        default:
        {
            m_lanDropDownBox->setIndexofCurrElement(lanSelected);
            if(lanSelected == SELECT_LAN1)
            {
                getLan1Config();
            }
            else
            {
                getLan2Config();
            }

            WizardCommon:: InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }
        break;
    }

    update();
    processBar->unloadProcessBar();
}

void NetworkSetting::slotDropDownBoxValueChanged(QString, quint32 indexInPage)
{
    if (indexInPage == WIZ_LAN_STG_LAN_DRP_DWN)
    {
        setFlag(false);
        saveConfig();
    }
    else if (indexInPage == WIZ_LAN_STG_IP_ADDR_MODE_DRP_DWN)
    {
        currIpAddrMode = (WIZ_IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement();
        selectIpAssignMode();
        isIpAddrDropDwnChng = true;
    }
}

void NetworkSetting::enableDisableIpv6Ctrls()
{
    bool isEnable = (currIpAddrMode == WIZ_IP_ADDR_MODE_IPV4_AND_IPV6) ? true : false;

    ipv6PrefixLen->setIsEnabled(isEnable);
    for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
    {
        staticIpv6AddrSet[index]->setIsEnabled(isEnable);
    }

    if (lanSelected == SELECT_LAN2)
    {
        return;
    }

    for(quint8 index = WIZ_IPV6_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV6_ASSIGN_MODE; index++)
    {
        ipv6ModeOptionSelButton[index]->setIsEnabled(isEnable);
    }

    for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
    {
        dnsv6ModeOptionSelButton[index]->setIsEnabled(isEnable);
    }

    for(quint8 index = WIZ_DNS_IP; index < WIZ_MAX_DNS_IP; index++)
    {
        dnsv6IpAddrSet[index]->setIsEnabled(isEnable);
    }
}

void NetworkSetting::displayLan1Lan2()
{
    quint8 yOffset = 0;
    lanSelected = m_lanDropDownBox->getIndexofCurrElement();
    deleteDynamicElement();

    if (lanSelected == SELECT_LAN2)
    {
        yOffset = 40;
    }

    staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR] = new Ipv4TextBox(ipAddrModeDrpDwn->x(),
                                                            ipAddrModeDrpDwn->y() + BGTILE_HEIGHT*2 - SCALE_HEIGHT(yOffset) + SCALE_HEIGHT(5),
                                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                                            BGTILE_HEIGHT,
                                                            WIZ_LAN_STG_IPV4_STATIC_IP_ADDR,
                                                            staticIpv4Str[WIZ_STATIC_IPV4_ADDR],
                                                            this, COMMON_LAYER,
                                                            true, 0, true);
    connect (staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR],
             SIGNAL(sigEntryDone(quint32)),
             this,
             SLOT(slotIpTextBoxEntryDone(quint32)));

    staticIpv4AddrSet[WIZ_STATIC_IPV4_SUBNETMASK] = new Ipv4TextBox(staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->x (),
                                                                  staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->y () + BGTILE_HEIGHT,
                                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  WIZ_LAN_STG_IPV4_STATIC_SUBNETMASK,
                                                                  staticIpv4Str[WIZ_STATIC_IPV4_SUBNETMASK],
                                                                  this, COMMON_LAYER,
                                                                  true, 0, true,true);

    staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY] = new Ipv4TextBox(staticIpv4AddrSet[WIZ_STATIC_IPV4_SUBNETMASK]->x (),
                                                                    staticIpv4AddrSet[WIZ_STATIC_IPV4_SUBNETMASK]->y () + BGTILE_HEIGHT,
                                                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                                                    BGTILE_HEIGHT,
                                                                    WIZ_LAN_STG_IPV4_STATIC_DFLT_GATEWAY,
                                                                    staticIpv4Str[WIZ_STATIC_IPV4_DFLT_GATEWAY],
                                                                    this, COMMON_LAYER,
                                                                    true, 0, true);

    for(quint8 index = WIZ_STATIC_IPV4_ADDR; index < WIZ_MAX_STATIC_IPV4; index++)
    {
        connect (staticIpv4AddrSet[index],
                 SIGNAL(sigLoadInfopage(quint32)),
                 this,
                 SLOT(slotIpTextBoxLoadInfopage(quint32)));
    }

    staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR] = new IpTextBox(staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->x() +
                                                            staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->width() + SCALE_WIDTH(6),
                                                            staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->y (),
                                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                                            BGTILE_HEIGHT,
                                                            WIZ_LAN_STG_IPV6_STATIC_IP_ADDR,
                                                            staticIpv6Str[WIZ_STATIC_IPV6_ADDR],
                                                            IP_ADDR_TYPE_IPV6_ONLY,
                                                            this, COMMON_LAYER,
                                                            true, 0, true);

    connect (staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR],
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

    ipv6PrefixLen = new TextBox(staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->x (),
                                staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->y () + staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->height (),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                WIZ_LAN_STG_IPV6_STATIC_PREFIX_LEN,
                                TEXTBOX_LARGE,
                                this, prefixLenParam,
                                COMMON_LAYER);

    staticIpv6AddrSet[WIZ_STATIC_IPV6_DFLT_GATEWAY] = new IpTextBox(ipv6PrefixLen->x (),
                                                                    ipv6PrefixLen->y () + ipv6PrefixLen->height (),
                                                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                                                    BGTILE_HEIGHT,
                                                                    WIZ_LAN_STG_IPV6_STATIC_DFLT_GATEWAY,
                                                                    staticIpv6Str[WIZ_STATIC_IPV6_DFLT_GATEWAY],
                                                                    IP_ADDR_TYPE_IPV6_ONLY,
                                                                    this, COMMON_LAYER,
                                                                    true, 0, true);
    connect (staticIpv6AddrSet[WIZ_STATIC_IPV6_DFLT_GATEWAY],
             SIGNAL(sigLoadInfopage(quint32)),
             this,
             SLOT(slotIpTextBoxLoadInfopage(quint32)));

    if(lanSelected == SELECT_LAN1)
    {
        ipv4ModeOptionSelButton[WIZ_IPV4_ASSIGN_MODE_STATIC] = new OptionSelectButton(ipAddrModeDrpDwn->x(),
                                                                                      ipAddrModeDrpDwn->y() + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                                                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                                                                      BGTILE_HEIGHT,
                                                                                      RADIO_BUTTON_INDEX,
                                                                                      this,
                                                                                      COMMON_LAYER,
                                                                                      IPV4_MODE_ASSIGN_STRING,
                                                                                      ipv4AssignModeStr[WIZ_IPV4_ASSIGN_MODE_STATIC],
                                                                                      -1,
                                                                                      WIZ_LAN_STG_IPV4_ASSIGN_STATIC,
                                                                                      true,
                                                                                      NORMAL_FONT_SIZE,
                                                                                      NORMAL_FONT_COLOR);

        ipv4ModeOptionSelButton[WIZ_IPV4_ASSIGN_MODE_DHCP] = new OptionSelectButton(ipv4ModeOptionSelButton[WIZ_IPV4_ASSIGN_MODE_STATIC]->x() + SCALE_WIDTH(350),
                                                                                    ipv4ModeOptionSelButton[WIZ_IPV4_ASSIGN_MODE_STATIC]->y(),
                                                                                    SCALE_WIDTH(20),
                                                                                    BGTILE_HEIGHT,
                                                                                    RADIO_BUTTON_INDEX,
                                                                                    ipv4AssignModeStr[WIZ_IPV4_ASSIGN_MODE_DHCP],
                                                                                    this,
                                                                                    NO_LAYER,
                                                                                    0,
                                                                                    MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                                    NORMAL_FONT_SIZE,
                                                                                    WIZ_LAN_STG_IPV4_ASSIGN_DHCP);

        for(quint8 index = WIZ_IPV4_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV4_ASSIGN_MODE; index++)
        {
            connect (ipv4ModeOptionSelButton[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
        }

        dnsv4ModeOptionSelButton[WIZ_DNS_MODE_STATIC] = new OptionSelectButton(staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY]->x(),
                                                                               staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY]->y() + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                                                               BGTILE_HEIGHT,
                                                                               RADIO_BUTTON_INDEX,
                                                                               this, COMMON_LAYER,
                                                                               DNSV4_MODE_STRING,
                                                                               "Static",
                                                                               -1,
                                                                               WIZ_LAN_STG_DNSV4_MODE_STATIC,
                                                                               true,
                                                                               NORMAL_FONT_SIZE,
                                                                               NORMAL_FONT_COLOR);

        dnsv4ModeOptionSelButton[WIZ_DNS_MODE_AUTO] = new OptionSelectButton((dnsv4ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->x() + SCALE_WIDTH(350)),
                                                                             dnsv4ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->y(),
                                                                             SCALE_WIDTH(20),
                                                                             BGTILE_HEIGHT,
                                                                             RADIO_BUTTON_INDEX,
                                                                             "Automatic",
                                                                             this,
                                                                             NO_LAYER,
                                                                             -1,
                                                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                             NORMAL_FONT_SIZE,
                                                                             WIZ_LAN_STG_DNSV4_MODE_AUTO);

        for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
        {
            connect (dnsv4ModeOptionSelButton[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
        }

        dnsv4IpAddrSet[WIZ_DNS_IP] = new Ipv4TextBox(dnsv4ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->x(),
                                                   dnsv4ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->y() + BGTILE_HEIGHT,
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   WIZ_LAN_STG_DNSV4_IP,
                                                   dnsIpStr[WIZ_DNS_IP],
                                                   this, COMMON_LAYER,
                                                   true, 0, true);

        dnsv4IpAddrSet[WIZ_DNS_ALTR_IP] = new Ipv4TextBox(dnsv4IpAddrSet[WIZ_DNS_IP]->x (),
                                                        dnsv4IpAddrSet[WIZ_DNS_IP]->y () + BGTILE_HEIGHT,
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        WIZ_LAN_STG_DNSV4_ALTR_IP,
                                                        dnsIpStr[WIZ_DNS_ALTR_IP],
                                                        this, COMMON_LAYER,
                                                        true, 0, true);

        for(quint8 index = WIZ_DNS_IP; index < WIZ_MAX_DNS_IP; index++)
        {
            connect (dnsv4IpAddrSet[index],
                     SIGNAL(sigLoadInfopage(quint32)),
                     this,
                     SLOT(slotIpTextBoxLoadInfopage(quint32)));
        }

        ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_STATIC] = new OptionSelectButton((staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->x ()),
                                                                                       staticIpv6AddrSet[WIZ_STATIC_IPV6_ADDR]->y () - BGTILE_HEIGHT,
                                                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                                                       BGTILE_HEIGHT,
                                                                                       RADIO_BUTTON_INDEX,
                                                                                       this,
                                                                                       COMMON_LAYER,
                                                                                       IPV6_MODE_ASSIGN_STRING,
                                                                                       ipv6AssignModeStr[WIZ_IPV6_ASSIGN_MODE_STATIC], 30,
                                                                                       WIZ_LAN_STG_IPV6_ASSIGN_STATIC, true, NORMAL_FONT_SIZE, NORMAL_FONT_COLOR);

        ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_DHCP] = new OptionSelectButton(ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_STATIC]->x () + SCALE_WIDTH(305),
                                                                                    ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_STATIC]->y (),
                                                                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                                                                    BGTILE_HEIGHT,
                                                                                    RADIO_BUTTON_INDEX,
                                                                                    ipv6AssignModeStr[WIZ_IPV6_ASSIGN_MODE_DHCP],
                                                                                    this,
                                                                                    NO_LAYER,
                                                                                    0,
                                                                                    MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                                    NORMAL_FONT_SIZE,
                                                                                    WIZ_LAN_STG_IPV6_ASSIGN_DHCP);

        ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_SLAAC] = new OptionSelectButton(ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_DHCP]->x () +
                                                                                     ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_DHCP]->width () + SCALE_WIDTH(10),
                                                                                     ipv6ModeOptionSelButton[WIZ_IPV6_ASSIGN_MODE_DHCP]->y (),
                                                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                                                     BGTILE_HEIGHT,
                                                                                     RADIO_BUTTON_INDEX,
                                                                                     ipv6AssignModeStr[WIZ_IPV6_ASSIGN_MODE_SLAAC],
                                                                                     this,
                                                                                     NO_LAYER,
                                                                                     0,
                                                                                     MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                                     NORMAL_FONT_SIZE,
                                                                                     WIZ_LAN_STG_IPV6_ASSIGN_SLAAC);

        for(quint8 index = WIZ_IPV6_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV6_ASSIGN_MODE; index++)
        {
            connect (ipv6ModeOptionSelButton[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
        }

        dnsv6ModeOptionSelButton[WIZ_DNS_MODE_STATIC] = new OptionSelectButton(staticIpv6AddrSet[WIZ_STATIC_IPV6_DFLT_GATEWAY]->x(),
                                                                               staticIpv6AddrSet[WIZ_STATIC_IPV6_DFLT_GATEWAY]->y () + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                                                               BGTILE_HEIGHT,
                                                                               RADIO_BUTTON_INDEX,
                                                                               this, COMMON_LAYER,
                                                                               DNSV6_MODE_STRING,
                                                                               "Static",
                                                                               -1,
                                                                               WIZ_LAN_STG_DNSV6_MODE_STATIC,
                                                                               true,
                                                                               NORMAL_FONT_SIZE,
                                                                               NORMAL_FONT_COLOR);

        dnsv6ModeOptionSelButton[WIZ_DNS_MODE_AUTO] = new OptionSelectButton((dnsv6ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->x () + SCALE_WIDTH(350)),
                                                                             dnsv6ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->y (),
                                                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                                                             BGTILE_HEIGHT,
                                                                             RADIO_BUTTON_INDEX,
                                                                             "Automatic",
                                                                             this,
                                                                             NO_LAYER,
                                                                             0,
                                                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                             NORMAL_FONT_SIZE,
                                                                             WIZ_LAN_STG_DNSV6_MODE_AUTO);

        for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
        {
            connect (dnsv6ModeOptionSelButton[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e,int)));
        }

        dnsv6IpAddrSet[WIZ_DNS_IP] = new IpTextBox(dnsv6ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->x (),
                                                   dnsv6ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->y () + dnsv6ModeOptionSelButton[WIZ_DNS_MODE_STATIC]->height (),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   WIZ_LAN_STG_DNSV6_IP_ADDR,
                                                   dnsIpStr[WIZ_DNS_IP],
                                                   IP_ADDR_TYPE_IPV6_ONLY,
                                                   this, COMMON_LAYER,
                                                   true, 0, true, IP_FIELD_TYPE_DNSV6);

        dnsv6IpAddrSet[WIZ_DNS_ALTR_IP] = new IpTextBox(dnsv6IpAddrSet[WIZ_DNS_IP]->x (),
                                                        dnsv6IpAddrSet[WIZ_DNS_IP]->y () + dnsv6IpAddrSet[WIZ_DNS_IP]->height (),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        WIZ_LAN_STG_DNSV6_ALTR_IP_ADDR,
                                                        dnsIpStr[WIZ_DNS_ALTR_IP],
                                                        IP_ADDR_TYPE_IPV6_ONLY,
                                                        this, COMMON_LAYER,
                                                        true, 0, true, IP_FIELD_TYPE_DNSV6);

        for(quint8 index = WIZ_DNS_IP; index < WIZ_MAX_DNS_IP; index++)
        {
            connect (dnsv6IpAddrSet[index],
                     SIGNAL(sigLoadInfopage(quint32)),
                     this,
                     SLOT(slotIpTextBoxLoadInfopage(quint32)));
        }

        getLan1Config();
        this->update();
    }
    else
    {
        getLan2Config();
        this->update();
    }
}

void NetworkSetting :: selectIpAssignMode()
{
    enableDisableIpv6Ctrls();
    if (lanSelected == SELECT_LAN2)
    {
        return;
    }

    for(quint8 index = WIZ_IPV4_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV4_ASSIGN_MODE; index++)
    {
        ipv4ModeOptionSelButton[index]->changeState ((ipv4AssignMode == index) ? ON_STATE : OFF_STATE);
    }

    for(quint8 index = WIZ_IPV6_ASSIGN_MODE_STATIC; index < WIZ_MAX_IPV6_ASSIGN_MODE; index++)
    {
        ipv6ModeOptionSelButton[index]->changeState ((ipv6AssignMode == index) ? ON_STATE : OFF_STATE);
    }

    switch(ipv4AssignMode)
    {
        case WIZ_IPV4_ASSIGN_MODE_STATIC:
        {
            for(quint8 index = WIZ_STATIC_IPV4_ADDR; index < WIZ_MAX_STATIC_IPV4; index++)
            {
                staticIpv4AddrSet[index]->setIsEnabled (true);
            }

            dnsv4Mode = WIZ_DNS_MODE_STATIC;
            setDnsSerMode();

            for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
            {
                dnsv4ModeOptionSelButton[index]->setIsEnabled (false);
            }
        }
        break;

        case WIZ_IPV4_ASSIGN_MODE_DHCP:
        {
            for(quint8 index = WIZ_STATIC_IPV4_ADDR; index < WIZ_MAX_STATIC_IPV4; index++)
            {
                staticIpv4AddrSet[index]->setIsEnabled (false);
            }

            for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
            {
                dnsv4ModeOptionSelButton[index]->setIsEnabled (true);
            }
            setDnsSerMode();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(currIpAddrMode != WIZ_IP_ADDR_MODE_IPV4_AND_IPV6)
    {
        return;
    }

    switch(ipv6AssignMode)
    {
        case WIZ_IPV6_ASSIGN_MODE_STATIC:
            for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
            {
                staticIpv6AddrSet[index]->setIsEnabled (true);
            }

            ipv6PrefixLen->setIsEnabled(true);
            dnsv6Mode = WIZ_DNS_MODE_STATIC;
            setDnsSerMode();

            for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
            {
                dnsv6ModeOptionSelButton[index]->setIsEnabled (false);
            }
            break;

        case WIZ_IPV6_ASSIGN_MODE_DHCP:
            for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
            {
                staticIpv6AddrSet[index]->setIsEnabled (false);
            }

            ipv6PrefixLen->setIsEnabled(false);
            for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
            {
                dnsv6ModeOptionSelButton[index]->setIsEnabled (true);
            }
            setDnsSerMode();
            break;

        case WIZ_IPV6_ASSIGN_MODE_SLAAC:
            for(quint8 index = WIZ_STATIC_IPV6_ADDR; index < WIZ_MAX_STATIC_IPV6; index++)
            {
                staticIpv6AddrSet[index]->setIsEnabled (false);
            }

            ipv6PrefixLen->setIsEnabled(false);
            for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
            {
                dnsv6ModeOptionSelButton[index]->setIsEnabled (true);
            }

            setDnsSerMode();
            break;

        default:
            break;
    }
}

void NetworkSetting::setDnsSerMode()
{
    for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
    {
        dnsv4ModeOptionSelButton[index]->changeState ((dnsv4Mode == index) ? ON_STATE : OFF_STATE);
    }

    for(quint8 index = WIZ_DNS_MODE_STATIC; index < WIZ_MAX_DNS_MODE; index++)
    {
        dnsv6ModeOptionSelButton[index]->changeState ((dnsv6Mode == index) ? ON_STATE : OFF_STATE);
    }

    if(dnsv4Mode == WIZ_DNS_MODE_AUTO)
    {  
        dnsv4IpAddrSet[WIZ_DNS_IP]->setIsEnabled (false);
        dnsv4IpAddrSet[WIZ_DNS_ALTR_IP]->setIsEnabled (false);
    }
    else
    {
        dnsv4IpAddrSet[WIZ_DNS_IP]->setIsEnabled (true);
        dnsv4IpAddrSet[WIZ_DNS_ALTR_IP]->setIsEnabled (true);
    }

    if(currIpAddrMode != WIZ_IP_ADDR_MODE_IPV4_AND_IPV6)
    {
        return;
    }

    if(dnsv6Mode == WIZ_DNS_MODE_AUTO)
    {
        dnsv6IpAddrSet[WIZ_DNS_IP]->setIsEnabled (false);
        dnsv6IpAddrSet[WIZ_DNS_ALTR_IP]->setIsEnabled (false);
    }
    else
    {
        dnsv6IpAddrSet[WIZ_DNS_IP]->setIsEnabled (true);
        dnsv6IpAddrSet[WIZ_DNS_ALTR_IP]->setIsEnabled (true);
    }
}

void NetworkSetting::slotIpAssignModeButtonClicked(OPTION_STATE_TYPE_e state,int index)
{
    if(state == ON_STATE)
    {
        switch(index)
        {
            case WIZ_LAN_STG_IPV4_ASSIGN_STATIC:
                ipv4AssignMode = WIZ_IPV4_ASSIGN_MODE_STATIC;
                selectIpAssignMode();
                break;

            case WIZ_LAN_STG_IPV4_ASSIGN_DHCP:
                ipv4AssignMode = WIZ_IPV4_ASSIGN_MODE_DHCP;
                selectIpAssignMode();
                break;

            case WIZ_LAN_STG_DNSV4_MODE_STATIC:
                dnsv4Mode = WIZ_DNS_MODE_STATIC;
                setDnsSerMode();
                break;

            case WIZ_LAN_STG_DNSV4_MODE_AUTO:
                dnsv4Mode = WIZ_DNS_MODE_AUTO;
                setDnsSerMode();
                break;

            case WIZ_LAN_STG_IPV6_ASSIGN_STATIC:
                ipv6AssignMode = WIZ_IPV6_ASSIGN_MODE_STATIC;
                selectIpAssignMode();
                break;

            case WIZ_LAN_STG_IPV6_ASSIGN_DHCP:
                ipv6AssignMode = WIZ_IPV6_ASSIGN_MODE_DHCP;
                selectIpAssignMode();
                break;

            case WIZ_LAN_STG_IPV6_ASSIGN_SLAAC:
                ipv6AssignMode = WIZ_IPV6_ASSIGN_MODE_SLAAC;
                selectIpAssignMode();
                break;

            case WIZ_LAN_STG_DNSV6_MODE_STATIC:
                dnsv6Mode = WIZ_DNS_MODE_STATIC;
                setDnsSerMode();
                break;

            case WIZ_LAN_STG_DNSV6_MODE_AUTO:
                dnsv6Mode = WIZ_DNS_MODE_AUTO;
                setDnsSerMode();
                break;

            default:
                break;
        }
    }
}

void NetworkSetting::slotIpTextBoxEntryDone (quint32)
{
    QString gateWay = staticIpv4AddrSet[WIZ_STATIC_IPV4_ADDR]->getSubnetOfIpaddress ();
    if(gateWay == "0.0.0")
    {
        if(lanSelected == SELECT_LAN1)
        {
            getLan1Config();
        }
        else
        {
            getLan2Config();
        }
		return;
    }

    gateWay.append (".").append ("1");
    staticIpv4AddrSet[WIZ_STATIC_IPV4_DFLT_GATEWAY]->setIpaddress (gateWay);
}

void NetworkSetting::slotIpTextBoxLoadInfopage(quint32 index)
{
    QString tempMsg = "";
    switch(index)
    {
        case WIZ_LAN_STG_IPV4_STATIC_IP_ADDR:
        case WIZ_LAN_STG_IPV6_STATIC_IP_ADDR:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_IP_ADDR);
            break;

        case WIZ_LAN_STG_IPV4_STATIC_SUBNETMASK:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_SUBNET_MASK);
            break;

        case WIZ_LAN_STG_IPV4_STATIC_DFLT_GATEWAY:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_GATEWAY_MASK);
            break;

        case WIZ_LAN_STG_DNSV4_IP:
        case WIZ_LAN_STG_DNSV6_IP_ADDR:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_PREF_DNS_ADDR);
            break;

        case WIZ_LAN_STG_DNSV4_ALTR_IP:
        case WIZ_LAN_STG_DNSV6_ALTR_IP_ADDR:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_ALTR_DNS_ADDR);
            break;

        case WIZ_LAN_STG_IPV6_STATIC_DFLT_GATEWAY:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_GATEWAY_MASK);
            break;

        default:
            break;
    }

    WizardCommon::InfoPageImage();
    m_infoPage->raise();
    m_infoPage->loadInfoPage (tempMsg);
}

void NetworkSetting:: setFlag(bool status)
{
    m_nextBtnClick = status;
}

void NetworkSetting::slotInfoPageBtnclick(int)
{
    WizardCommon::UnloadInfoPageImage();
}
