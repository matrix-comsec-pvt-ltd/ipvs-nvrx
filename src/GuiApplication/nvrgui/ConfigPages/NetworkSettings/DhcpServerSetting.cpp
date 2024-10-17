//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "DhcpServerSetting.h"
#include "ValidationMessage.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define FIRST_ELE_XOFFSET               SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET               SCALE_HEIGHT(119)
#define CNFG_TO_INDEX                   1

#define CLIENT_LIST_FIELD_CELL_HEIGHT   SCALE_HEIGHT(60)
#define CLIENT_LIST_BGTILE_WIDTH        SCALE_WIDTH(876)

//#################################################################################################
// @TYPEDEFS
//#################################################################################################
/* List of control */
typedef enum
{
    DHCP_SERVER_ENABLE = 0,
    DHCP_SERVER_INTERFACE,
    DHCP_SERVER_START_IP_ADDR,
    DHCP_SERVER_NUMBER_OF_HOSTS,
    DHCP_SERVER_SUBNET_MASK,
    DHCP_SERVER_GATEWAY_ADDR,
    DHCP_SERVER_DNS_ADDRESS,
    DHCP_SERVER_LEASE_HOURS,
    DHCP_SERVER_CLIENT_LIST,
    MAX_DHCP_SERVER_ELEMETS
}DHCP_SERVER_ELELIST_e;

/* Server configuation param */
typedef enum
{
    DHCP_SERVER_CFG_ENABLE = 0,
    DHCP_SERVER_CFG_ETHERNET_PORT,
    DHCP_SERVER_CFG_START_IP_ADDR,
    DHCP_SERVER_CFG_NUMBER_OF_HOSTS,
    DHCP_SERVER_CFG_GATEWAY,
    DHCP_SERVER_CFG_DNS_SERVER,
    DHCP_SERVER_CFG_LEASE_HOURS,
    MAX_DHCP_SERVER_CFG_FIELD
}DHCP_SERVER_CFG_FIELD_e;

/* client status page elements */
typedef enum
{
    CLIENT_LIST_ELEMENT_CLOSE_BUTTON,
    CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON,
    CLIENT_LIST_ELEMENT_NEXT_BUTTON,
    CLIENT_LIST_ELEMENT_MAX
}CLIENT_LIST_ELEMENT_e;

/* client status page strings */
typedef enum
{
    CLIENT_LIST_STR_IP_ADDR,
    CLIENT_LIST_STR_MAC_ADDR,
    CLIENT_LIST_STR_HOSTNAME,
    CLIENT_LIST_STR_LEASE_TIME,
    CLIENT_LIST_STR_NEXT_BUTTON,
    CLIENT_LIST_STR_PREV_BUTTON,
    CLIENT_LIST_STR_MAX
}CLIENT_LIST_STR_e;

/* client status page reply command response */
typedef enum
{
    CLIENT_LIST_CMD_REPLY_IP_ADDR,
    CLIENT_LIST_CMD_REPLY_MAC_ADDR,
    CLIENT_LIST_CMD_REPLY_HOSTNAME,
    CLIENT_LIST_CMD_REPLY_LEASE_TIME,
    CLIENT_LIST_CMD_REPLY_MAX
}CLIENT_LIST_RESP_e;

//#################################################################################################
// @VARIABLES
//#################################################################################################
const static QString labelStr[MAX_DHCP_SERVER_ELEMETS] =
{
    "Enable DHCP Server",
    "Ethernet Port",
    "Start IP Address",
    "Number of Hosts",
    "Subnet Mask",
    "Default Gateway",
    "DNS Address",
    "Lease Hours",
    "IP List and Status"
};

const static QString fieldHeadingStr[CLIENT_LIST_STR_MAX] =
{
    "IP Address",
    "MAC Address",
    "Host Name",
    "Lease Time (HHH:MM)",
    "Next",
    "Previous"
};

const static QStringList interfaceList = QStringList() << "LAN 1" << "LAN 2";

//#################################################################################################
// @FUNCTIONS
//#################################################################################################

/**
 * @brief DhcpServerSetting::DhcpServerSetting
 * @param devName
 * @param parent
 * @param devTabInfo
 */
DhcpServerSetting::DhcpServerSetting(QString devName, QWidget *parent, DEV_TABLE_INFO_t* devTabInfo)
    : ConfigPageControl(devName, parent, MAX_DHCP_SERVER_ELEMETS,devTabInfo), m_enableState(MAX_STATE),
      m_cnfgInterface(DHCP_SERVER_INTERFACE_MAX), m_lan1IpMode(MAX_IPV4_ASSIGN_MODE)
{
    INIT_OBJ(m_clientList);
    createDefaultComponent();
    DhcpServerSetting::getConfig();
}

/**
 * @brief DhcpServerSetting::~DhcpServerSetting
 */
DhcpServerSetting::~DhcpServerSetting()
{
    if(IS_VALID_OBJ(m_leaseHours))
    {
        disconnect(m_leaseHours,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_leaseHours,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(m_leaseHours);
    }
    DELETE_OBJ(m_leaseHoursParam);

    if(IS_VALID_OBJ(m_dnsIpAddr))
    {
        disconnect(m_dnsIpAddr,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_dnsIpAddr);
    }

    if(IS_VALID_OBJ(m_gatewayAddr))
    {
        disconnect(m_gatewayAddr,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_gatewayAddr);
    }

    if(IS_VALID_OBJ(m_subnetMask))
    {
        disconnect(m_subnetMask,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_subnetMask);
    }

    if(IS_VALID_OBJ(m_numberOfHosts))
    {
        disconnect(m_numberOfHosts,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_numberOfHosts,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(m_numberOfHosts);
    }
    DELETE_OBJ(m_numberOfHostsParam);

    if(IS_VALID_OBJ(m_startIpAddress))
    {
        disconnect(m_startIpAddress,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_startIpAddress);
    }

    if(IS_VALID_OBJ(m_intrfaceDropDownBox))
    {
        disconnect(m_intrfaceDropDownBox,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotInterfaceDropdownChanged(QString,quint32)));
        disconnect(m_intrfaceDropDownBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_intrfaceDropDownBox);
    }

    if(IS_VALID_OBJ(m_enableDhcpServer))
    {
        disconnect(m_enableDhcpServer,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        disconnect(m_enableDhcpServer,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e, int)),
                   this,
                   SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e, int)));
        DELETE_OBJ(m_enableDhcpServer);
    }

    if(IS_VALID_OBJ(m_dhcpClientListButton))
    {
        disconnect(m_dhcpClientListButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_dhcpClientListButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_dhcpClientListButton);
    }

    DELETE_OBJ(m_footerNoteLable);
    DELETE_OBJ(m_footerNote);
    DELETE_OBJ(m_clientList);
}

/**
 * @brief DhcpServerSetting::createDefaultComponent
 */
void DhcpServerSetting::createDefaultComponent()
{
    QMap<quint8, QString>  referenceMapList;

    for(quint8 index = 0; index < devTableInfo->numOfLan; index++)
    {
        referenceMapList.insert(index,interfaceList.at(index));
    }

    m_enableDhcpServer = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                                FIRST_ELE_YOFFSET,
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                UP_LAYER,
                                                labelStr[DHCP_SERVER_ENABLE],
                                                "", -1,
                                                DHCP_SERVER_ENABLE);
    m_elementList[DHCP_SERVER_ENABLE] = m_enableDhcpServer;
    connect(m_enableDhcpServer,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_enableDhcpServer,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_intrfaceDropDownBox = new DropDown(m_enableDhcpServer->x(),
                                         m_enableDhcpServer->y() + m_enableDhcpServer->height(),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         DHCP_SERVER_INTERFACE,
                                         DROPDOWNBOX_SIZE_200,
                                         labelStr[DHCP_SERVER_INTERFACE],
                                         referenceMapList,
                                         this, "", true,
                                         0, MIDDLE_TABLE_LAYER);
    m_elementList[DHCP_SERVER_INTERFACE] = m_intrfaceDropDownBox;
    connect(m_intrfaceDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_intrfaceDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotInterfaceDropdownChanged(QString,quint32)));

    m_startIpAddress = new Ipv4TextBox(m_intrfaceDropDownBox->x(),
                                       m_intrfaceDropDownBox->y() + m_intrfaceDropDownBox->height(),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       DHCP_SERVER_START_IP_ADDR,
                                       labelStr[DHCP_SERVER_START_IP_ADDR],
                                       this, MIDDLE_TABLE_LAYER);
    m_elementList[DHCP_SERVER_START_IP_ADDR] = m_startIpAddress;
    connect(m_startIpAddress,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_numberOfHostsParam = new TextboxParam();
    m_numberOfHostsParam->labelStr = labelStr[DHCP_SERVER_NUMBER_OF_HOSTS];
    m_numberOfHostsParam->isNumEntry = true;
    m_numberOfHostsParam->minNumValue = 1;
    m_numberOfHostsParam->maxNumValue = 100;
    m_numberOfHostsParam->suffixStr = QString("(%1-%2)").arg(m_numberOfHostsParam->minNumValue).arg(m_numberOfHostsParam->maxNumValue);
    m_numberOfHostsParam->maxChar = 3;
    m_numberOfHostsParam->validation = QRegExp(QString("[0-9]"));

    m_numberOfHosts = new TextBox(m_startIpAddress->x(),
                                  m_startIpAddress->y()+ m_startIpAddress->height(),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  DHCP_SERVER_NUMBER_OF_HOSTS,
                                  TEXTBOX_SMALL, this, m_numberOfHostsParam, MIDDLE_TABLE_LAYER);
    m_elementList[DHCP_SERVER_NUMBER_OF_HOSTS] = m_numberOfHosts;
    connect(m_numberOfHosts,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_numberOfHosts,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_subnetMask = new Ipv4TextBox(m_numberOfHosts->x(),
                                   m_numberOfHosts->y() + m_numberOfHosts->height(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   DHCP_SERVER_SUBNET_MASK,
                                   labelStr[DHCP_SERVER_SUBNET_MASK],
                                   this, MIDDLE_TABLE_LAYER);
    m_elementList[DHCP_SERVER_SUBNET_MASK] = m_subnetMask;
    connect(m_subnetMask,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_gatewayAddr = new Ipv4TextBox(m_subnetMask->x(),
                                    m_subnetMask->y() + m_subnetMask->height(),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    DHCP_SERVER_GATEWAY_ADDR,
                                    labelStr[DHCP_SERVER_GATEWAY_ADDR],
                                    this, MIDDLE_TABLE_LAYER);
    m_elementList[DHCP_SERVER_GATEWAY_ADDR] = m_gatewayAddr;
    connect(m_gatewayAddr,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_dnsIpAddr = new Ipv4TextBox(m_gatewayAddr->x(),
                                  m_gatewayAddr->y() + m_gatewayAddr->height(),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  DHCP_SERVER_DNS_ADDRESS,
                                  labelStr[DHCP_SERVER_DNS_ADDRESS],
                                  this, MIDDLE_TABLE_LAYER);
    m_elementList[DHCP_SERVER_DNS_ADDRESS] = m_dnsIpAddr;
    connect(m_dnsIpAddr,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_leaseHoursParam = new TextboxParam();
    m_leaseHoursParam->labelStr = labelStr[DHCP_SERVER_LEASE_HOURS];
    m_leaseHoursParam->isNumEntry = true;
    m_leaseHoursParam->minNumValue = 1;
    m_leaseHoursParam->maxNumValue = 192;
    m_leaseHoursParam->suffixStr = QString("(%1-%2)").arg(m_leaseHoursParam->minNumValue).arg(m_leaseHoursParam->maxNumValue);
    m_leaseHoursParam->maxChar = 3;
    m_leaseHoursParam->validation = QRegExp(QString("[0-9]"));

    m_leaseHours = new TextBox(m_dnsIpAddr->x(),
                               m_dnsIpAddr->y()+ m_dnsIpAddr->height(),
                               BGTILE_MEDIUM_SIZE_WIDTH,
                               BGTILE_HEIGHT,
                               DHCP_SERVER_LEASE_HOURS,
                               TEXTBOX_SMALL, this, m_leaseHoursParam, BOTTOM_TABLE_LAYER);
    m_elementList[DHCP_SERVER_LEASE_HOURS] = m_leaseHours;
    connect(m_leaseHours,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_leaseHours,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_dhcpClientListButton = new PageOpenButton(m_leaseHours->x(),
                                                m_leaseHours->y() + m_leaseHours->height() + SCALE_HEIGHT(20),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                DHCP_SERVER_CLIENT_LIST,
                                                PAGEOPENBUTTON_EXTRALARGE,
                                                labelStr[DHCP_SERVER_CLIENT_LIST],
                                                this,
                                                "","",
                                                true,
                                                0,
                                                COMMON_LAYER,
                                                true,
                                                ALIGN_START_X_CENTRE_Y);
    m_elementList[DHCP_SERVER_CLIENT_LIST] = m_dhcpClientListButton;
    connect(m_dhcpClientListButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_dhcpClientListButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    m_footerNote = new ElementHeading(FIRST_ELE_XOFFSET,
                                      m_dhcpClientListButton->y() + m_dhcpClientListButton->height() + SCALE_HEIGHT(20),
                                      SCALE_WIDTH(500),
                                      SCALE_HEIGHT(50),
                                      "",
                                      NO_LAYER,
                                      this);

    QString fontColor = "#c8c8c8";
    QString fontWidth = "" + QString::number(SCALE_WIDTH(15)) +"px";
    QString styl = "ElidedLabel \
    { \
        color: %1; \
        font-size: %2; \
        font-family: %3; \
    }";

    m_footerNote->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
    m_footerNoteLable = new ElidedLabel(DHCP_SERVER_IP_LIST_CLEAR_NOTE, m_footerNote);
    m_footerNoteLable->resize(SCALE_WIDTH(500), SCALE_HEIGHT(50));
    m_footerNoteLable->raise();
    m_footerNoteLable->show();
}

/**
 * @brief DhcpServerSetting::getConfig
 */
void DhcpServerSetting::getConfig()
{
    createPayload(MSG_GET_CFG);
}

/**
 * @brief DhcpServerSetting::defaultConfig
 */
void DhcpServerSetting::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

/**
 * @brief DhcpServerSetting::saveConfig
 */
void DhcpServerSetting::saveConfig()
{
    /* We will not store config if mode is not static for single ethernet models and we will also not validate any params */
    if ((devTableInfo->numOfLan == 1) && (m_lan1IpMode != IPV4_ASSIGN_MODE_STATIC))
    {
        return;
    }

    if((m_startIpAddress->getIpaddress() == "0.0.0.0") || (m_startIpAddress->getIpaddress() == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
        return;
    }

    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_ENABLE, m_enableDhcpServer->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_ETHERNET_PORT, m_intrfaceDropDownBox->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_START_IP_ADDR, m_startIpAddress->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_NUMBER_OF_HOSTS, m_numberOfHosts->getInputText());
    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_GATEWAY, m_gatewayAddr->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_DNS_SERVER, m_dnsIpAddr->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(DHCP_SERVER_CFG_LEASE_HOURS, m_leaseHours->getInputText());
    createPayload(MSG_SET_CFG);
}

/**
 * @brief DhcpServerSetting::createPayload
 * @param requestType
 */
void DhcpServerSetting::createPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             DHCP_SERVER_SETTINGS_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_DHCP_SERVER_CFG_FIELD,
                                                             (MSG_SET_CFG == requestType) ? MAX_DHCP_SERVER_CFG_FIELD : 0);
    if(requestType == MSG_GET_CFG)
    {
        payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                         LAN1_TABLE_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         CNFG_TO_INDEX,
                                                         CNFG_FRM_FIELD,
                                                         MAX_LAN1_FIELDS,
                                                         0,
                                                         payloadString,
                                                         MAX_LAN1_FIELDS);

        payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                         LAN2_TABLE_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         CNFG_TO_INDEX,
                                                         CNFG_FRM_FIELD,
                                                         MAX_LAN2_FIELDS,
                                                         0,
                                                         payloadString,
                                                         MAX_LAN2_FIELDS);
    }

    DevCommParam* param = new DevCommParam();
    param->msgType = requestType;
    param->payload = payloadString;
    if(applController->processActivity(currDevName, DEVICE_COMM, param))
    {
        processBar->loadProcessBar();
    }
}

/**
 * @brief DhcpServerSetting::processDeviceResponse
 * @param param
 * @param deviceName
 */
void DhcpServerSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    processBar->unloadProcessBar();

    if (deviceName != currDevName)
    {
        return;
    }

    /* process SET_CMD separately */
    if (MSG_SET_CMD == param->msgType)
    {
         m_clientList->processDeviceResponse(param, deviceName);
         return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex(0) != DHCP_SERVER_SETTINGS_TABLE_INDEX)
            {
                break;
            }

            m_enableState = (OPTION_STATE_TYPE_e) payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_ENABLE).toUInt();
            m_enableDhcpServer->changeState(m_enableState);
            m_cnfgInterface = (DHCP_SERVER_INTERFACE_e)payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_ETHERNET_PORT).toUInt();
            m_intrfaceDropDownBox->setIndexofCurrElement(m_cnfgInterface);
            m_cnfgStartIpAddr = payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_START_IP_ADDR).toString();
            m_startIpAddress->setIpaddress(m_cnfgStartIpAddr);
            m_numberOfHosts->setInputText(payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_NUMBER_OF_HOSTS).toString());
            m_cnfgGatewayAddr = payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_GATEWAY).toString();
            m_gatewayAddr->setIpaddress(m_cnfgGatewayAddr);
            m_cnfgDnsAddr = payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_DNS_SERVER).toString();
            m_dnsIpAddr->setIpaddress(m_cnfgDnsAddr);
            m_leaseHours->setInputText(payloadLib->getCnfgArrayAtIndex(DHCP_SERVER_CFG_LEASE_HOURS).toString());

            if(payloadLib->getcnfgTableIndex(1) != LAN1_TABLE_INDEX)
            {
                break;
            }

            quint8 fieldIdx = MAX_DHCP_SERVER_CFG_FIELD;
            m_lan1IpMode = (IPV4_ASSIGN_MODE_e)payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN1_FIELD_IPV4_ASSIGN_MODE).toUInt();
            m_lan1IpAddr = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN1_FIELD_STATIC_IPV4_ADDR).toString();
            m_lan1Subnet = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN1_FIELD_STATIC_IPV4_SUBNET).toString();
            m_lan1Gateway = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN1_FIELD_STATIC_IPV4_DFLT_GATEWAY).toString();
            m_startIpAddress->setIpaddress(m_lan1IpAddr);
            m_lan1IpAddr = m_startIpAddress->getSubnetOfIpaddress();
            m_lan1IpAddr.append(".").append("100");
            if ((devTableInfo->numOfLan == 2) && (m_lan1IpMode != IPV4_ASSIGN_MODE_STATIC))
            {
                m_cnfgInterface = DHCP_SERVER_INTERFACE_LAN2;
                m_intrfaceDropDownBox->setIndexofCurrElement(m_cnfgInterface);
            }

            m_lan1Dns = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN1_FIELD_DNSV4_IP).toString();
            if (m_lan1Dns == "")
            {
                m_lan1Dns = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN1_FIELD_DNSV4_ALTR_IP).toString();
            }

            if(payloadLib->getcnfgTableIndex(2) != LAN2_TABLE_INDEX)
            {
                break;
            }

            fieldIdx = MAX_DHCP_SERVER_CFG_FIELD+MAX_LAN1_FIELDS;
            m_lan2IpAddr = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN2_FIELD_IPV4_ADDR).toString();
            m_lan2Subnet = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN2_FIELD_IPV4_SUBNET).toString();
            m_lan2Gateway = payloadLib->getCnfgArrayAtIndex(fieldIdx+LAN2_FIELD_IPV4_GATEWAY).toString();
            m_startIpAddress->setIpaddress(m_lan2IpAddr);
            m_lan2IpAddr = m_startIpAddress->getSubnetOfIpaddress();
            m_lan2IpAddr.append(".").append("100");

            slotEnableButtonClicked(m_enableState, 0);
            m_subnetMask->setIsEnabled(false);
            slotInterfaceDropdownChanged(interfaceList.at(m_cnfgInterface), 0);
        }
        break;

        case MSG_SET_CFG:
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            getConfig();
        }
        break;

        case MSG_DEF_CFG:
        {
            getConfig();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

/**
 * @brief DhcpServerSetting::slotEnableButtonClicked
 * @param state
 */
void DhcpServerSetting::slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int)
{
    m_enableState = state;
    if ((devTableInfo->numOfLan == 1) && (m_lan1IpMode != IPV4_ASSIGN_MODE_STATIC))
    {
        m_enableDhcpServer->setIsEnabled(OFF_STATE);
    }

    if ((devTableInfo->numOfLan == 1) || ((devTableInfo->numOfLan == 2) && (m_lan1IpMode != IPV4_ASSIGN_MODE_STATIC)))
    {
        m_intrfaceDropDownBox->setIsEnabled(OFF_STATE);
    }
    else
    {
        m_intrfaceDropDownBox->setIsEnabled(m_enableState);
    }

    if ((DHCP_SERVER_INTERFACE_LAN1 == m_intrfaceDropDownBox->getIndexofCurrElement()) && (m_lan1IpMode != IPV4_ASSIGN_MODE_STATIC))
    {
        updateFieldStatus(OFF_STATE);
    }
    else
    {
        updateFieldStatus(m_enableState);
    }
}

/**
 * @brief DhcpServerSetting::updateFieldStatus
 * @param state
 */
void DhcpServerSetting::updateFieldStatus(OPTION_STATE_TYPE_e state)
{
    m_startIpAddress->setIsEnabled(state);
    m_numberOfHosts->setIsEnabled(state);
    m_gatewayAddr->setIsEnabled(state);
    m_dnsIpAddr->setIsEnabled(state);
    m_leaseHours->setIsEnabled(state);
    m_dhcpClientListButton->setIsEnabled(state);
}

/**
 * @brief DhcpServerSetting::slotInterfaceDropdownChanged
 * @param str
 */
void DhcpServerSetting::slotInterfaceDropdownChanged(QString str, quint32)
{
    if (DHCP_SERVER_INTERFACE_LAN1 == interfaceList.indexOf(str))
    {
        if (m_lan1IpMode != IPV4_ASSIGN_MODE_STATIC)
        {
            updateFieldStatus(OFF_STATE);
            m_startIpAddress->setIpaddress("");
            m_subnetMask->setIpaddress("");
            m_gatewayAddr->setIpaddress("");
            m_dnsIpAddr->setIpaddress("");
        }
        else
        {
            updateFieldStatus(m_enableState);
            m_subnetMask->setIpaddress(m_lan1Subnet);
            if ((m_cnfgInterface == DHCP_SERVER_INTERFACE_LAN1) && (m_cnfgStartIpAddr != ""))
            {
                m_startIpAddress->setIpaddress(m_cnfgStartIpAddr);
                m_gatewayAddr->setIpaddress(m_cnfgGatewayAddr);
                m_dnsIpAddr->setIpaddress(m_cnfgDnsAddr);
            }
            else
            {
                m_startIpAddress->setIpaddress(m_lan1IpAddr);
                m_gatewayAddr->setIpaddress(m_lan1Gateway);
                m_dnsIpAddr->setIpaddress(m_lan1Dns);
            }
        }
    }
    else
    {
        updateFieldStatus(m_enableState);
        m_subnetMask->setIpaddress(m_lan2Subnet);
        if ((m_cnfgInterface == DHCP_SERVER_INTERFACE_LAN2) && (m_cnfgStartIpAddr != ""))
        {
            m_startIpAddress->setIpaddress(m_cnfgStartIpAddr);
            m_gatewayAddr->setIpaddress(m_cnfgGatewayAddr);
            m_dnsIpAddr->setIpaddress(m_cnfgDnsAddr);
        }
        else
        {
            m_startIpAddress->setIpaddress(m_lan2IpAddr);
            m_gatewayAddr->setIpaddress(m_lan2Gateway);
            m_dnsIpAddr->setIpaddress("");
        }
    }
}

/**
 * @brief DhcpServerSetting::slotTextboxLoadInfopage
 * @param index
 * @param msgType
 */
void DhcpServerSetting::slotTextboxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    if(msgType != INFO_MSG_ERROR)
    {
        return;
    }

    switch(index)
    {
        case DHCP_SERVER_START_IP_ADDR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_IP_ADDR));
            break;

        case DHCP_SERVER_NUMBER_OF_HOSTS:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT));
            break;

        case DHCP_SERVER_GATEWAY_ADDR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_GATEWAY_MASK));
            break;

        case DHCP_SERVER_DNS_ADDRESS:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_PREF_DNS_ADDR));
            break;

        case DHCP_SERVER_LEASE_HOURS:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT));
            break;

        default:
            break;
    }
}

/**
 * @brief DhcpServerSetting::slotButtonClick
 * @param index
 */
void DhcpServerSetting::slotButtonClick(int index)
{
    if (DHCP_SERVER_CLIENT_LIST != index)
    {
        return;
    }

    if (m_clientList == NULL)
    {
        m_clientList = new DhcpClientList(currDevName,
                                          parentWidget(),
                                          payloadLib);
        connect(m_clientList,
                SIGNAL(sigObjectDelete()),
                this,
                SLOT(slotClientListDelete()));
    }
}

/**
 * @brief DhcpServerSetting::slotClientListDelete
 */
void DhcpServerSetting::slotClientListDelete()
{
    if (IS_VALID_OBJ(m_clientList))
    {
        disconnect(m_clientList,
                   SIGNAL(sigObjectDelete()),
                   this,
                   SLOT(slotClientListDelete()));
        DELETE_OBJ(m_clientList);
    }
}

//------------------------------------------------------------------------
// D H C P    C L I E N T     I P     L I S T
//------------------------------------------------------------------------
/**
 * @brief DhcpClientList::DhcpClientList
 * @param devName
 * @param parent
 * @param devTabInfo
 */
DhcpClientList::DhcpClientList(QString devName, QWidget *parent, PayloadLib *payloadLib)
    : KeyBoard(parent), NavigationControl(0, true)
{
    INIT_OBJ(m_topBgTile);
    INIT_OBJ(m_bottomBgTile);
    INIT_OBJ(m_backGround);
    INIT_OBJ(m_heading);
    INIT_OBJ(m_previousButton);
    INIT_OBJ(m_nextButton);
    INIT_OBJ(m_closeButton);
    INIT_OBJ(m_processBar);
    INIT_OBJ(m_infoPage);
    m_currentElement = 0;

    for (quint8 index = 0; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        INIT_OBJ(m_ipAddrCell[index]);
        INIT_OBJ(m_macAddrCell[index]);
        INIT_OBJ(m_hostNameCell[index]);
        INIT_OBJ(m_leaseTimeStrCell[index]);

        INIT_OBJ(m_ipAddrCellText[index]);
        INIT_OBJ(m_macAddrCellText[index]);
        INIT_OBJ(m_hostNameCellText[index]);
        INIT_OBJ(m_leaseTimeStrCellText[index]);
    }

    currentPageNo = 0;
    maximumPages = 0;
    maxSearchListCount = 0;

    this->setGeometry(0, 0, parent->width(), parent->height());
    applController = ApplController::getInstance();
    m_currDevName = devName;
    m_payloadLib = payloadLib;

    m_ipAddrList.reserve(DHCP_SERVER_IP_LIST_MAX_ROW);
    m_macAddrList.reserve(DHCP_SERVER_IP_LIST_MAX_ROW);
    m_hostNameList.reserve(DHCP_SERVER_IP_LIST_MAX_ROW);
    m_leaseTimeStrList.reserve(DHCP_SERVER_IP_LIST_MAX_ROW);

    /* draw status page UI */
    createDefaultComponent();

    /* show UI */
    this->show();

    /* send command to get dhcp client list */
    sendCommand(GET_DHCP_LEASE);
}

/**
 * @brief DhcpClientList::~DhcpClientList
 */
DhcpClientList::~DhcpClientList()
{
    m_ipAddrList.clear();
    m_macAddrList.clear();
    m_hostNameList.clear();
    m_leaseTimeStrList.clear();

    DELETE_OBJ(m_topBgTile);
    DELETE_OBJ(m_bottomBgTile);
    DELETE_OBJ(m_backGround);
    DELETE_OBJ(m_heading);
    DELETE_OBJ(m_processBar);

    for (quint8 index = 0; index < DHCP_SERVER_IP_LIST_MAX_COL; index++)
    {
        DELETE_OBJ(m_tableCellHeadings[index]);
        DELETE_OBJ(m_tableHeadingTextlabel[index]);
    }

    for (quint8 index = 0; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        DELETE_OBJ(m_ipAddrCell[index]);
        DELETE_OBJ(m_macAddrCell[index]);
        DELETE_OBJ(m_hostNameCell[index]);
        DELETE_OBJ(m_leaseTimeStrCell[index]);

        DELETE_OBJ(m_ipAddrCellText[index]);
        DELETE_OBJ(m_macAddrCellText[index]);
        DELETE_OBJ(m_hostNameCellText[index]);
        DELETE_OBJ(m_leaseTimeStrCellText[index]);
    }

    if (IS_VALID_OBJ(m_infoPage))
    {
        disconnect(m_infoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(m_infoPage);
    }

    if (IS_VALID_OBJ(m_previousButton))
    {
        disconnect(m_previousButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(m_previousButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_previousButton);
    }

    if (IS_VALID_OBJ(m_nextButton))
    {
        disconnect(m_nextButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(m_nextButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_nextButton);
    }

    if (IS_VALID_OBJ(m_closeButton))
    {
        disconnect(m_closeButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(m_closeButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_closeButton);
    }
}

/**
 * @brief DhcpClientList::createDefaultComponent
 */
void DhcpClientList::createDefaultComponent()
{
    quint8 headerWidthArray[] = {200, 200, 255, 200};

    /* backgound of the page */
    m_backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CLIENT_LIST_BGTILE_WIDTH) / 2))-SCALE_WIDTH(15),
                                 (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - (6*CLIENT_LIST_FIELD_CELL_HEIGHT)) / 2)) - SCALE_HEIGHT(60),
                                 CLIENT_LIST_BGTILE_WIDTH + SCALE_WIDTH(30),
                                 (6*CLIENT_LIST_FIELD_CELL_HEIGHT) + SCALE_HEIGHT(160),
                                 0,
                                 NORMAL_BKG_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this);

    /* top background tile */
    m_topBgTile = new BgTile((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CLIENT_LIST_BGTILE_WIDTH) / 2)),
                             (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- (6*CLIENT_LIST_FIELD_CELL_HEIGHT)) / 2)),
                             CLIENT_LIST_BGTILE_WIDTH,
                             (6*CLIENT_LIST_FIELD_CELL_HEIGHT),
                             TOP_LAYER,
                             this);

    /* bottom background tile */
    m_bottomBgTile = new BgTile(m_topBgTile->x(),
                                m_topBgTile->y() + m_topBgTile->height(),
                                m_topBgTile->width(),
                                CLIENT_LIST_FIELD_CELL_HEIGHT,
                                BOTTOM_LAYER,
                                this);

    /* heading of the page */
    m_heading = new Heading((m_backGround->x() + (m_backGround->width() / 2)),
                            (m_backGround->y() + SCALE_HEIGHT(30)),
                            "IP List and Status",
                            this,
                            HEADING_TYPE_2);

    /* close button */
    m_closeButton = new CloseButtton((m_backGround->x() + m_backGround->width() - SCALE_WIDTH(20)),
                                     (m_backGround->y() + SCALE_HEIGHT(30)),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     CLIENT_LIST_ELEMENT_CLOSE_BUTTON);
    m_elementList[CLIENT_LIST_ELEMENT_CLOSE_BUTTON] = m_closeButton;
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* field heading */
    m_tableCellHeadings[0] = new TableCell(m_topBgTile->x() + SCALE_WIDTH(10),
                                           m_topBgTile->y() + SCALE_HEIGHT(10),
                                           (SCALE_WIDTH(headerWidthArray[0]) - 1),
                                           SCALE_HEIGHT(50),
                                           this,
                                           true);

    /* field heading labels */
    m_tableHeadingTextlabel[0] = new TextLabel(m_tableCellHeadings[0]->x() + SCALE_WIDTH(10),
                                               m_tableCellHeadings[0]->y() + (m_tableCellHeadings[0]->height())/2,
                                               NORMAL_FONT_SIZE,
                                               fieldHeadingStr[0],
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, 0, (SCALE_WIDTH(headerWidthArray[0])));
    /* draw columns */
    for (quint8 index = 1; index < DHCP_SERVER_IP_LIST_MAX_COL; index++)
    {
        m_tableCellHeadings[index] = new TableCell(m_tableCellHeadings[index-1]->x() + m_tableCellHeadings[index-1]->width(),
                                                   m_tableCellHeadings[index-1]->y(),
                                                   (SCALE_WIDTH(headerWidthArray[index]) - 1),
                                                   SCALE_HEIGHT(50),
                                                   this,
                                                   true);

        m_tableHeadingTextlabel[index] = new TextLabel(m_tableCellHeadings[index]->x() + SCALE_WIDTH(10),
                                                       m_tableCellHeadings[index]->y() + (m_tableCellHeadings[index]->height())/2,
                                                       NORMAL_FONT_SIZE,
                                                       fieldHeadingStr[index],
                                                       this,
                                                       NORMAL_FONT_COLOR,
                                                       NORMAL_FONT_FAMILY,
                                                       ALIGN_START_X_CENTRE_Y, 0, 0, (SCALE_WIDTH(headerWidthArray[index])));
    }

    /* draw raws */
    m_ipAddrCell[0] = new TableCell(m_tableCellHeadings[0]->x(),
                                    m_tableCellHeadings[0]->y() + m_tableCellHeadings[0]->height(),
                                    m_tableCellHeadings[0]->width(),
                                    BGTILE_HEIGHT,
                                    this);

    m_macAddrCell[0] = new TableCell(m_tableCellHeadings[1]->x(),
                                     m_tableCellHeadings[1]->y() + m_tableCellHeadings[1]->height(),
                                     m_tableCellHeadings[1]->width(),
                                     BGTILE_HEIGHT,
                                     this);

    m_hostNameCell[0] = new TableCell(m_tableCellHeadings[2]->x(),
                                      m_tableCellHeadings[2]->y() + m_tableCellHeadings[2]->height(),
                                      m_tableCellHeadings[2]->width(),
                                      BGTILE_HEIGHT,
                                      this);

    m_leaseTimeStrCell[0] = new TableCell(m_tableCellHeadings[3]->x(),
                                          m_tableCellHeadings[3]->y() + m_tableCellHeadings[3]->height(),
                                          m_tableCellHeadings[3]->width()-1,
                                          BGTILE_HEIGHT,
                                          this);

    for (quint8 index = 1; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        m_ipAddrCell[index] = new TableCell(m_ipAddrCell[(index-1)]->x(),
                                            m_ipAddrCell[(index-1)]->y() + m_ipAddrCell[0]->height(),
                                            m_ipAddrCell[0]->width(),
                                            BGTILE_HEIGHT,
                                            this);

        m_macAddrCell[index] = new TableCell(m_macAddrCell[(index-1)]->x(),
                                             m_macAddrCell[(index-1)]->y() + m_macAddrCell[0]->height(),
                                             m_macAddrCell[0]->width(),
                                             BGTILE_HEIGHT,
                                             this);

        m_hostNameCell[index] = new TableCell(m_hostNameCell[(index-1)]->x(),
                                              m_hostNameCell[(index-1)]->y() + m_hostNameCell[0]->height(),
                                              m_hostNameCell[0]->width(),
                                              BGTILE_HEIGHT,
                                              this);

        m_leaseTimeStrCell[index] = new TableCell(m_leaseTimeStrCell[(index-1)]->x(),
                                                  m_leaseTimeStrCell[(index-1)]->y() + m_leaseTimeStrCell[0]->height(),
                                                  m_leaseTimeStrCell[0]->width()-1,
                                                  BGTILE_HEIGHT,
                                                  this);
    }

    /* cell text */
    m_ipAddrCellText[0] = new TextLabel(m_ipAddrCell[0]->x() + SCALE_WIDTH(10),
                                        m_ipAddrCell[0]->y() + (m_ipAddrCell[0]->height())/2,
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[0]->width());


    m_macAddrCellText[0] = new TextLabel(m_macAddrCell[0]->x() + SCALE_WIDTH(10),
                                         m_macAddrCell[0]->y() + (m_macAddrCell[0]->height())/2,
                                         NORMAL_FONT_SIZE,
                                         "",
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[1]->width());

    m_hostNameCellText[0] = new TextLabel(m_hostNameCell[0]->x() + SCALE_WIDTH(10),
                                          m_hostNameCell[0]->y() + (m_hostNameCell[0]->height())/2,
                                          NORMAL_FONT_SIZE,
                                          "",
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[2]->width());

    m_leaseTimeStrCellText[0] = new TextLabel(m_leaseTimeStrCell[0]->x() + SCALE_WIDTH(10),
                                              m_leaseTimeStrCell[0]->y() + (m_leaseTimeStrCell[0]->height())/2,
                                              NORMAL_FONT_SIZE,
                                              "",
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[3]->width());

    for (quint8 index = 1; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        m_ipAddrCellText[index] = new TextLabel(m_ipAddrCell[index]->x() + SCALE_WIDTH(10),
                                                m_ipAddrCell[index]->y() + (m_ipAddrCell[index]->height())/2,
                                                NORMAL_FONT_SIZE,
                                                "",
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, m_ipAddrCell[(index-1)]->width() - 1);

        m_macAddrCellText[index] = new TextLabel(m_macAddrCell[index]->x() + SCALE_WIDTH(10),
                                                 m_macAddrCell[index]->y() + (m_macAddrCell[index]->height())/2,
                                                 NORMAL_FONT_SIZE,
                                                 "",
                                                 this,
                                                 NORMAL_FONT_COLOR,
                                                 NORMAL_FONT_FAMILY,
                                                 ALIGN_START_X_CENTRE_Y, 0, 0, m_macAddrCell[(index-1)]->width() - 1);

        m_hostNameCellText[index] = new TextLabel(m_hostNameCell[index]->x() + SCALE_WIDTH(10),
                                                  m_hostNameCell[index]->y() + (m_hostNameCell[index]->height())/2,
                                                  NORMAL_FONT_SIZE,
                                                  "",
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, 0, m_hostNameCell[(index-1)]->width() - 1);

        m_leaseTimeStrCellText[index] = new TextLabel(m_leaseTimeStrCell[index]->x() + SCALE_WIDTH(10),
                                                      m_leaseTimeStrCell[index]->y() + (m_leaseTimeStrCell[index]->height())/2,
                                                      NORMAL_FONT_SIZE,
                                                      "",
                                                      this,
                                                      NORMAL_FONT_COLOR,
                                                      NORMAL_FONT_FAMILY,
                                                      ALIGN_START_X_CENTRE_Y, 0, 0, m_leaseTimeStrCell[(index-1)]->width() - 1);
    }

    /* previous button */
    m_previousButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                         m_topBgTile->x() + SCALE_WIDTH(15),
                                         m_bottomBgTile->y() + SCALE_HEIGHT(20),
                                         m_topBgTile->width(),
                                         BGTILE_HEIGHT,
                                         this,
                                         NO_LAYER,
                                         -1,
                                         fieldHeadingStr[CLIENT_LIST_STR_PREV_BUTTON],
                                         false,
                                         CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON,
                                         false);
    m_elementList[CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON] = m_previousButton;
    connect(m_previousButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_previousButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                     m_topBgTile->x() + m_topBgTile->width() - SCALE_WIDTH(90),
                                     m_bottomBgTile->y() + SCALE_HEIGHT(20),
                                     m_topBgTile->width(),
                                     BGTILE_HEIGHT,
                                     this,
                                     NO_LAYER,
                                     -1,
                                     fieldHeadingStr[CLIENT_LIST_STR_NEXT_BUTTON],
                                     false,
                                     CLIENT_LIST_ELEMENT_NEXT_BUTTON);
    m_elementList[CLIENT_LIST_ELEMENT_NEXT_BUTTON] = m_nextButton;
    connect(m_nextButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_nextButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* process wait bar */
    m_processBar = new ProcessBar(m_backGround->x(), m_backGround->y(),
                                  m_backGround->width(),
                                  m_backGround->height(),
                                  SCALE_WIDTH(0), this);

    /* info pop-up for error */
    m_infoPage = new InfoPage(0, 0,
                              (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH)),
                              SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                              INFO_CONFIG_PAGE,
                              parentWidget());
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageBtnclick(int)));
}

/**
 * @brief DhcpClientList::slotUpdateCurrentElement
 * @param index
 */
void DhcpClientList::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

/**
 * @brief DhcpClientList::slotButtonClick
 * @param index
 */
void DhcpClientList::slotButtonClick(int index)
{
    switch(index)
    {
        case CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON:
        {
            if (currentPageNo > 0)
            {
                currentPageNo--;
            }

            updateClientList();
        }
        break;

        case CLIENT_LIST_ELEMENT_NEXT_BUTTON:
        {
            if (currentPageNo < (maximumPages - 1))
            {
                currentPageNo ++;
            }

            updateClientList();
        }
        break;

        case CLIENT_LIST_ELEMENT_CLOSE_BUTTON:
        default:
        {
            emit sigObjectDelete();
        }
        break;
    }
}

/**
 * @brief DhcpClientList::slotInfoPageBtnclick
 */
void DhcpClientList::slotInfoPageBtnclick(int)
{
    m_elementList[m_currentElement]->forceActiveFocus();
}

/**
 * @brief DhcpClientList::sendCommand
 * @param cmdType
 */
void DhcpClientList::sendCommand(SET_COMMAND_e cmdType)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;

    if (applController->processActivity(m_currDevName, DEVICE_COMM, param))
    {
        m_processBar->loadProcessBar();
    }
}

/**
 * @brief DhcpClientList::processDeviceResponse
 * @param param
 */
void DhcpClientList::processDeviceResponse(DevCommParam *param, QString)
{
    if (param->cmdType != GET_DHCP_LEASE)
    {
        return;
    }

    m_processBar->unloadProcessBar();

    /* fail response? */
    if (CMD_SUCCESS != param->deviceStatus)
    {
        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    /* success response */
    parseAndUpdateClientList(param);
}

/**
 * @brief DhcpClientList::parseAndUpdateClientList
 * @param param
 */
void DhcpClientList::parseAndUpdateClientList(DevCommParam *param)
{
    quint8 index = 0;
    quint32 leasetime = 0;

    /* clear qstring list of all data */
    clearClientList();

    m_payloadLib->parseDevCmdReply(true, param->payload);

    maxSearchListCount = (m_payloadLib->getTotalCmdFields() / CLIENT_LIST_CMD_REPLY_MAX);

    /* if no records found */
    if (0 == maxSearchListCount)
    {
        return;
    }

    /* parse reponse data */
    for (index = 0; index < maxSearchListCount; index++)
    {
        m_ipAddrList.append(m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_IP_ADDR +  (index * CLIENT_LIST_CMD_REPLY_MAX)).toString());
        m_macAddrList.append(m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_MAC_ADDR + (index * CLIENT_LIST_CMD_REPLY_MAX)).toString());
        m_hostNameList.append(m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_HOSTNAME + (index * CLIENT_LIST_CMD_REPLY_MAX)).toString());

        /* convert lease time seconds to HHH:MM format */
        leasetime = m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_LEASE_TIME + (index * CLIENT_LIST_CMD_REPLY_MAX)).toUInt();
        m_leaseTimeStrList.append(QString::number(leasetime/3600).rightJustified(3, '0') + ":" + QString::number((leasetime%3600)/60).rightJustified(2, '0'));
    }

    /* derive maximum pages */
    maximumPages = (maxSearchListCount % CLIENT_LIST_SINGLE_PAGE_RECORDS == 0) ?
                (maxSearchListCount / CLIENT_LIST_SINGLE_PAGE_RECORDS) : ((maxSearchListCount / CLIENT_LIST_SINGLE_PAGE_RECORDS) + 1);

    /* show dhcp clients on UI */
    updateClientList();
}

/**
 * @brief DhcpClientList::clearClientList
 */
void DhcpClientList::clearClientList()
{
    m_ipAddrList.clear();
    m_macAddrList.clear();
    m_hostNameList.clear();
    m_leaseTimeStrList.clear();
    currentPageNo = 0;
}

/**
 * @brief DhcpClientList::updateClientList
 */
void DhcpClientList::updateClientList()
{
    quint8 recordOnPage = 0, eleIndex = 0;

    /* derive number of records to be display on current page */
    if (maxSearchListCount < (CLIENT_LIST_SINGLE_PAGE_RECORDS * (currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((CLIENT_LIST_SINGLE_PAGE_RECORDS * (currentPageNo)));
    }
    else
    {
        recordOnPage = CLIENT_LIST_SINGLE_PAGE_RECORDS;
    }

    /* update next/prev button based on number of total records */
    updateNavigationControlStatus();

    /* display records as per current page numebr */
    for (quint8 index = 0; index < recordOnPage; index++)
    {
        eleIndex = ((index + (currentPageNo * CLIENT_LIST_SINGLE_PAGE_RECORDS)));

        m_ipAddrCellText[index]->changeText(m_ipAddrList.at(eleIndex));
        m_ipAddrCellText[index]->update();

        m_macAddrCellText[index]->changeText(m_macAddrList.at(eleIndex));
        m_macAddrCellText[index]->update();

        m_hostNameCellText[index]->changeText(m_hostNameList.at(eleIndex));
        m_hostNameCellText[index]->update();

        m_leaseTimeStrCellText[index]->changeText(m_leaseTimeStrList.at(eleIndex));
        m_leaseTimeStrCellText[index]->update();
    }

    /* clear remaining cell data */
    if (recordOnPage != CLIENT_LIST_SINGLE_PAGE_RECORDS)
    {
        for (quint8 index = recordOnPage; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
        {
            m_ipAddrCellText[index]->changeText("");
            m_ipAddrCellText[index]->update();

            m_macAddrCellText[index]->changeText("");
            m_macAddrCellText[index]->update();

            m_hostNameCellText[index]->changeText("");
            m_hostNameCellText[index]->update();

            m_leaseTimeStrCellText[index]->changeText("");
            m_leaseTimeStrCellText[index]->update();
        }
    }
}

/**
 * @brief DhcpClientList::updateNavigationControlStatus
 */
void DhcpClientList::updateNavigationControlStatus()
{
    m_previousButton->setIsEnabled((currentPageNo ? true : false));

    if (currentPageNo < (maximumPages - 1))
    {
        m_nextButton->setIsEnabled(true);
    }
    else if (currentPageNo == (maximumPages - 1))
    {
        m_nextButton->setIsEnabled(false);
    }
}
