//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "DhcpServer.h"
#include "ValidationMessage.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define FIRST_ELE_XOFFSET               SCALE_WIDTH(70)
#define FIRST_ELE_YOFFSET               SCALE_HEIGHT(50)
#define CNFG_TO_INDEX                   1

#define CLIENT_LIST_FIELD_CELL_HEIGHT   SCALE_HEIGHT(60)
#define CLIENT_LIST_BGTILE_WIDTH        SCALE_WIDTH(876)

#define WIZARD_BG_TILE_HEIGHT           SCALE_HEIGHT(45)
#define WIZARD_BG_TILE_WIDTH            SCALE_WIDTH(945)
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
typedef enum {
    CLIENT_LIST_ELEMENT_CLOSE_BUTTON,
    CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON,
    CLIENT_LIST_ELEMENT_NEXT_BUTTON,

    CLIENT_LIST_ELEMENT_MAX
}CLIENT_LIST_ELEMENT_e;

/* client status page strings */
typedef enum {
    CLIENT_LIST_STR_IP_ADDR,
    CLIENT_LIST_STR_MAC_ADDR,
    CLIENT_LIST_STR_HOSTNAME,
    CLIENT_LIST_STR_LEASE_TIME,
    CLIENT_LIST_STR_NEXT_BUTTON,
    CLIENT_LIST_STR_PREV_BUTTON,

    CLIENT_LIST_STR_MAX
}CLIENT_LIST_STR_e;


/* client status page reply command response */
typedef enum {
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

const static QStringList interfaceLanList = QStringList() << "LAN 1" << "LAN 2";

//#################################################################################################
// @FUNCTIONS
//#################################################################################################

/**
 * @brief DhcpServer::DhcpServer
 * @param devName
 * @param parent
 * @param devTabInfo
 */
DhcpServer::DhcpServer(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId)
    : WizardCommon(parent, pageId)
{
    m_clientList = NULL;
    applController = ApplController::getInstance ();
    currDevName = devName;
    m_cnfgInterface = DHCP_SERVER_INTERFACE_LAN1;
    m_enableState = OFF_STATE;
    m_lan1IpMode = WIZ_IPV4_ASSIGN_MODE_STATIC;

    applController->GetDeviceInfo(currDevName, devTableInfo);
    createDefaultComponent(subHeadStr);
    WizardCommon::LoadProcessBar();
    DhcpServer::getConfig();
    this->show();
}

/**
 * @brief DhcpServer::~DhcpServer
 */
DhcpServer::~DhcpServer()
{
    WizardCommon::UnloadProcessBar();
    DELETE_OBJ(m_dhcpServerHeading);

    if(IS_VALID_OBJ(m_leaseHours))
    {
        disconnect(m_leaseHours,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(m_leaseHours);
    }

    DELETE_OBJ(m_leaseHoursParam);
    DELETE_OBJ(m_dnsIpAddr);
    DELETE_OBJ(m_gatewayAddr);
    DELETE_OBJ(m_subnetMask);

    if(IS_VALID_OBJ(m_numberOfHosts))
    {
        disconnect(m_numberOfHosts,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(m_numberOfHosts);
    }
    DELETE_OBJ(m_numberOfHostsParam);
    DELETE_OBJ(m_startIpAddress);

    if(IS_VALID_OBJ(m_intrfaceDropDownBox))
    {
        disconnect(m_intrfaceDropDownBox,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotInterfaceDropdownChanged(QString,quint32)));

        DELETE_OBJ(m_intrfaceDropDownBox);
    }

    if(IS_VALID_OBJ(m_enableDhcpServer))
    {
        disconnect(m_enableDhcpServer,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e, int)),
                   this,
                   SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e, int)));
        DELETE_OBJ(m_enableDhcpServer);
    }

    if(IS_VALID_OBJ(m_dhcpClientListButton))
    {
        disconnect(m_dhcpClientListButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_dhcpClientListButton);
    }

    DELETE_OBJ(m_footerNoteLable);
    DELETE_OBJ(m_footerNote);
    DELETE_OBJ(payloadLib);

    if (IS_VALID_OBJ(m_infoPage))
    {
        disconnect(m_infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(m_infoPage);
    }

    DELETE_OBJ(m_clientList);
}

/**
 * @brief DhcpServer::createDefaultComponent
 */
void DhcpServer::createDefaultComponent(QString subHeadStr)
{
    payloadLib = new PayloadLib();

    m_dhcpServerHeading = new TextLabel((SCALE_WIDTH(500)),
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

    QMap<quint8, QString>  referenceMapList;

    for(quint8 index = 0; index < devTableInfo.numOfLan; index++)
    {
        referenceMapList.insert(index,interfaceLanList.at(index));
    }

    m_enableDhcpServer = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                                FIRST_ELE_YOFFSET,
                                                WIZARD_BG_TILE_WIDTH,
                                                WIZARD_BG_TILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                COMMON_LAYER,
                                                labelStr[DHCP_SERVER_ENABLE],
                                                "", -1,
                                                DHCP_SERVER_ENABLE);
    connect (m_enableDhcpServer,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_intrfaceDropDownBox = new DropDown(m_enableDhcpServer->x(),
                                         m_enableDhcpServer->y() + m_enableDhcpServer->height(),
                                         WIZARD_BG_TILE_WIDTH,
                                         WIZARD_BG_TILE_HEIGHT,
                                         DHCP_SERVER_INTERFACE,
                                         DROPDOWNBOX_SIZE_200,
                                         labelStr[DHCP_SERVER_INTERFACE],
                                         referenceMapList,
                                         this, "", true,
                                         0, COMMON_LAYER);
    connect (m_intrfaceDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotInterfaceDropdownChanged(QString,quint32)));

    m_startIpAddress = new Ipv4TextBox(m_intrfaceDropDownBox->x(),
                                     m_intrfaceDropDownBox->y() + m_intrfaceDropDownBox->height(),
                                     WIZARD_BG_TILE_WIDTH,
                                     WIZARD_BG_TILE_HEIGHT,
                                     DHCP_SERVER_START_IP_ADDR,
                                     labelStr[DHCP_SERVER_START_IP_ADDR],
                                     this, COMMON_LAYER);

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
                                  WIZARD_BG_TILE_WIDTH,
                                  WIZARD_BG_TILE_HEIGHT,
                                  DHCP_SERVER_NUMBER_OF_HOSTS,
                                  TEXTBOX_SMALL, this, m_numberOfHostsParam, COMMON_LAYER);
    connect (m_numberOfHosts,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_subnetMask = new Ipv4TextBox(m_numberOfHosts->x(),
                                 m_numberOfHosts->y() + m_numberOfHosts->height(),
                                 WIZARD_BG_TILE_WIDTH,
                                 WIZARD_BG_TILE_HEIGHT,
                                 DHCP_SERVER_SUBNET_MASK,
                                 labelStr[DHCP_SERVER_SUBNET_MASK],
                                 this, COMMON_LAYER);

    m_gatewayAddr = new Ipv4TextBox(m_subnetMask->x(),
                                 m_subnetMask->y() + m_subnetMask->height(),
                                 WIZARD_BG_TILE_WIDTH,
                                 WIZARD_BG_TILE_HEIGHT,
                                 DHCP_SERVER_GATEWAY_ADDR,
                                 labelStr[DHCP_SERVER_GATEWAY_ADDR],
                                 this, COMMON_LAYER);

    m_dnsIpAddr = new Ipv4TextBox(m_gatewayAddr->x(),
                                m_gatewayAddr->y() + m_gatewayAddr->height(),
                                WIZARD_BG_TILE_WIDTH,
                                WIZARD_BG_TILE_HEIGHT,
                                DHCP_SERVER_DNS_ADDRESS,
                                labelStr[DHCP_SERVER_DNS_ADDRESS],
                                this, COMMON_LAYER);

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
                               WIZARD_BG_TILE_WIDTH,
                               WIZARD_BG_TILE_HEIGHT,
                               DHCP_SERVER_LEASE_HOURS,
                               TEXTBOX_SMALL, this, m_leaseHoursParam, COMMON_LAYER);
    connect (m_leaseHours,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_dhcpClientListButton = new PageOpenButton(m_leaseHours->x(),
                                                m_leaseHours->y() + m_leaseHours->height() + SCALE_HEIGHT(20),
                                                WIZARD_BG_TILE_WIDTH,
                                                WIZARD_BG_TILE_HEIGHT,
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
    connect(m_dhcpClientListButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

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

    m_footerNote = new ElementHeading(FIRST_ELE_XOFFSET,
                                      m_dhcpClientListButton->y() + m_dhcpClientListButton->height() + SCALE_HEIGHT(20),
                                      WIZARD_BG_TILE_WIDTH,
                                      WIZARD_BG_TILE_HEIGHT,
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
    m_footerNoteLable->resize(WIZARD_BG_TILE_WIDTH, WIZARD_BG_TILE_HEIGHT);
    m_footerNoteLable->raise();
    m_footerNoteLable->show();
}

/**
 * @brief DhcpServer::getConfig
 */
void DhcpServer::getConfig()
{
    createPayload(MSG_GET_CFG);
}

/**
 * @brief DhcpServer::saveConfig
 */
void DhcpServer::saveConfig()
{
    /* We will not store config if mode is not static for single ethernet models and we will also not validate any params */
    if ((devTableInfo.numOfLan == 1) && (m_lan1IpMode != WIZ_IPV4_ASSIGN_MODE_STATIC))
    {
        m_deletePage = true;
        getConfig();
        return;
    }

    if ((m_startIpAddress->getIpaddress () == "0.0.0.0") || (m_startIpAddress->getIpaddress () == ""))
    {
        WizardCommon:: InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_ENABLE, m_enableDhcpServer->getCurrentState());
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_ETHERNET_PORT, m_intrfaceDropDownBox->getIndexofCurrElement());
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_START_IP_ADDR, m_startIpAddress->getIpaddress());
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_NUMBER_OF_HOSTS, m_numberOfHosts->getInputText());
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_GATEWAY, m_gatewayAddr->getIpaddress());
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_DNS_SERVER, m_dnsIpAddr->getIpaddress());
        payloadLib->setCnfgArrayAtIndex (DHCP_SERVER_CFG_LEASE_HOURS, m_leaseHours->getInputText());
        createPayload(MSG_SET_CFG);
    }
}

/**
 * @brief DhcpServer::createPayload
 * @param requestType
 */
void DhcpServer::createPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString =  "";
    payloadString = payloadLib->createDevCnfgPayload(requestType,
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
                                                         WIZ_MAX_LAN1_FIELDS,
                                                         0,
                                                         payloadString,
                                                         WIZ_MAX_LAN1_FIELDS);
        payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                         LAN2_TABLE_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         CNFG_TO_INDEX,
                                                         CNFG_FRM_FIELD,
                                                         WIZ_MAX_LAN2_FIELDS,
                                                         0,
                                                         payloadString,
                                                         WIZ_MAX_LAN2_FIELDS);
    }

    DevCommParam* param = new DevCommParam();
    param->msgType = requestType;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

/**
 * @brief DhcpServer::processDeviceResponse
 * @param param
 * @param deviceName
 */
void DhcpServer::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    quint8                  tFieldIdx = 0;
    QMap<quint8, QString>   templist;

    templist.clear();

    if (deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    /* process SET_CMD separately */
    if (MSG_SET_CMD == param->msgType)
    {
         m_clientList->processDeviceResponse(param, deviceName);
         return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
                case MSG_GET_CFG:
                    payloadLib->parsePayload(param->msgType, param->payload);
                    if ((payloadLib->getcnfgTableIndex(0) == DHCP_SERVER_SETTINGS_TABLE_INDEX)
                            && (payloadLib->getcnfgTableIndex(1) == LAN1_TABLE_INDEX)
                            && (payloadLib->getcnfgTableIndex(2) == LAN2_TABLE_INDEX))
                    {
                        /* DHCP Server Configuration */
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

                        /* LAN1 Configuration */
                        tFieldIdx = MAX_DHCP_SERVER_CFG_FIELD;
                        m_lan1IpMode = (WIZ_IPV4_ASSIGN_MODE_e)payloadLib->getCnfgArrayAtIndex(tFieldIdx+WIZ_LAN1_FIELD_IPV4_ASSIGN_MODE).toUInt();
                        m_lan1IpAddr = payloadLib->getCnfgArrayAtIndex(tFieldIdx+WIZ_LAN1_FIELD_IPV4_ADDRESS).toString();
                        m_lan1Subnet = payloadLib->getCnfgArrayAtIndex(tFieldIdx+WIZ_LAN1_FIELD_IPV4_SUBNETMASK).toString();
                        m_lan1Gateway = payloadLib->getCnfgArrayAtIndex(tFieldIdx+WIZ_LAN1_FIELD_IPV4_DFLT_GATEWAY).toString();
                        m_startIpAddress->setIpaddress(m_lan1IpAddr);
                        m_lan1IpAddr = m_startIpAddress->getSubnetOfIpaddress();
                        m_lan1IpAddr.append(".").append("100");
                        if ((devTableInfo.numOfLan == 2) && (m_lan1IpMode != WIZ_IPV4_ASSIGN_MODE_STATIC))
                        {
                            m_cnfgInterface = DHCP_SERVER_INTERFACE_LAN2;
                            m_intrfaceDropDownBox->setIndexofCurrElement(m_cnfgInterface);
                        }

                        m_lan1Dns = payloadLib->getCnfgArrayAtIndex(tFieldIdx+WIZ_LAN1_FIELD_DNSV4_IP).toString();
                        if (m_lan1Dns == "")
                        {
                            m_lan1Dns = payloadLib->getCnfgArrayAtIndex(tFieldIdx+WIZ_LAN1_FIELD_DNSV4_ALTR_IP).toString();
                        }

                        /* LAN2 Configuration */
                        tFieldIdx = MAX_DHCP_SERVER_CFG_FIELD+WIZ_MAX_LAN1_FIELDS;
                        m_lan2IpAddr = payloadLib->getCnfgArrayAtIndex(tFieldIdx + WIZ_LAN2_FIELD_IP_ADDRESS).toString();
                        m_lan2Subnet = payloadLib->getCnfgArrayAtIndex(tFieldIdx + WIZ_LAN2_FIELD_SUBNET).toString();
                        m_lan2Gateway = payloadLib->getCnfgArrayAtIndex(tFieldIdx + WIZ_LAN2_FIELD_GATEWAY).toString();
                        m_startIpAddress->setIpaddress(m_lan2IpAddr);
                        m_lan2IpAddr = m_startIpAddress->getSubnetOfIpaddress();
                        m_lan2IpAddr.append(".").append("100");

                        slotEnableButtonClicked(m_enableState, 0);
                        m_subnetMask->setIsEnabled(false);
                        slotInterfaceDropdownChanged(QString("LAN ") + INT_TO_QSTRING(m_cnfgInterface+1), 0);
                    }
                    break;

                case MSG_SET_CFG:
                    m_deletePage = true;
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                    getConfig();
                    break;

                default:
                    break;
            }
            break;

        default:
            WizardCommon:: InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
    }
    update();
    processBar->unloadProcessBar();
}

/**
 * @brief DhcpServer::slotEnableButtonClicked
 * @param state
 */
void DhcpServer::slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int)
{
    m_enableState = state;
    if ((devTableInfo.numOfLan == 1) && (m_lan1IpMode != WIZ_IPV4_ASSIGN_MODE_STATIC))
    {
        m_enableDhcpServer->setIsEnabled(OFF_STATE);
    }

    if ((devTableInfo.numOfLan == 1) || ((devTableInfo.numOfLan == 2) && (m_lan1IpMode != WIZ_IPV4_ASSIGN_MODE_STATIC)))
    {
        m_intrfaceDropDownBox->setIsEnabled(OFF_STATE);
    }
    else
    {
        m_intrfaceDropDownBox->setIsEnabled(m_enableState);
    }

    if ((DHCP_SERVER_INTERFACE_LAN1 == m_intrfaceDropDownBox->getIndexofCurrElement()) && (m_lan1IpMode != WIZ_IPV4_ASSIGN_MODE_STATIC))
    {
        updateFieldStatus(OFF_STATE);
    }
    else
    {
        updateFieldStatus(m_enableState);
    }
}

/**
 * @brief DhcpServer::updateFieldStatus
 * @param state
 */
void DhcpServer::updateFieldStatus(OPTION_STATE_TYPE_e state)
{
    m_startIpAddress->setIsEnabled(state);
    m_numberOfHosts->setIsEnabled(state);
    m_gatewayAddr->setIsEnabled(state);
    m_dnsIpAddr->setIsEnabled(state);
    m_leaseHours->setIsEnabled(state);
    m_dhcpClientListButton->setIsEnabled(state);
}

/**
 * @brief DhcpServer::slotInterfaceDropdownChanged
 * @param str
 * @param index
 */
void DhcpServer::slotInterfaceDropdownChanged(QString str, quint32 index)
{
    if (DHCP_SERVER_INTERFACE_LAN1 == interfaceLanList.indexOf(str))
    {
        if (m_lan1IpMode != WIZ_IPV4_ASSIGN_MODE_STATIC)
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
    Q_UNUSED(index)
}

/**
 * @brief DhcpServer::slotTextboxLoadInfopage
 * @param index
 * @param msgType
 */
void DhcpServer::slotTextboxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    QString tempMsg;

    if(msgType != INFO_MSG_ERROR)
    {
        return;
    }

    switch(index)
    {
        case DHCP_SERVER_START_IP_ADDR:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_IP_ADDR);
            break;

        case DHCP_SERVER_NUMBER_OF_HOSTS:
            tempMsg = ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT);
            break;

        case DHCP_SERVER_GATEWAY_ADDR:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_GATEWAY_MASK);
            break;

        case DHCP_SERVER_DNS_ADDRESS:
            tempMsg = ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_PREF_DNS_ADDR);
            break;

        case DHCP_SERVER_LEASE_HOURS:
            tempMsg = ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT);
            break;

        default:
            break;
    }
}

/**
 * @brief DhcpServer::slotButtonClick
 * @param index
 */
void DhcpServer::slotButtonClick(int index)
{
    if (DHCP_SERVER_CLIENT_LIST != index)
    {
        return;
    }

    if (m_clientList == NULL)
    {
        m_clientList = new DhcpClientStatus(currDevName,
                                            parentWidget(),
                                            payloadLib);
        connect (m_clientList,
                 SIGNAL(sigObjectDelete()),
                 this,
                 SLOT(slotClientListDelete()));
    }
}

/**
 * @brief DhcpServer::slotClientListDelete
 */
void DhcpServer::slotClientListDelete()
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

void DhcpServer::slotInfoPageBtnclick(int)
{
    WizardCommon::UnloadInfoPageImage();
}

//------------------------------------------------------------------------
// D H C P    C L I E N T     I P     L I S T
//------------------------------------------------------------------------
/**
 * @brief DhcpClientStatus::DhcpClientStatus
 * @param devName
 * @param parent
 * @param devTabInfo
 */
DhcpClientStatus::DhcpClientStatus(QString devName, QWidget *parent, PayloadLib *payloadLib):
       KeyBoard(parent), NavigationControl(0, true)
{
    m_topBgTile = NULL;
    m_bottomBgTile = NULL;
    m_backGround = NULL;
    m_heading = NULL;
    m_previousButton = NULL;
    m_nextButton = NULL;
    m_closeButton = NULL;
    m_processBar = NULL;
    m_infoPage = NULL;
    m_currentElement = 0;

    for (quint8 index = 0; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        m_ipAddrCell[index] = NULL;
        m_macAddrCell[index] = NULL;
        m_hostNameCell[index] = NULL;
        m_leaseTimeStrCell[index] = NULL;

        m_ipAddrCellText[index] = NULL;
        m_macAddrCellText[index] = NULL;
        m_hostNameCellText[index] = NULL;
        m_leaseTimeStrCellText[index] = NULL;
    }

    currentPageNo = 0;
    maximumPages = 0;
    maxSearchListCount = 0;

    this->setGeometry (0, 0, parent->width(), parent->height());
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
 * @brief DhcpClientStatus::~DhcpClientStatus
 */
DhcpClientStatus::~DhcpClientStatus()
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
 * @brief DhcpClientStatus::createDefaultComponent
 */
void DhcpClientStatus::createDefaultComponent()
{
    quint8 headerWidthArray[] = {200, 200, 255, 200};

    /* backgound of the page */
    m_backGround = new Rectangle(((WIZARD_MAIN_RECT_WIDTH - (CLIENT_LIST_BGTILE_WIDTH + SCALE_WIDTH(30))) / 2), SCALE_HEIGHT(105),
                                 CLIENT_LIST_BGTILE_WIDTH + SCALE_WIDTH(30),
                                 (6*CLIENT_LIST_FIELD_CELL_HEIGHT) + SCALE_HEIGHT(200),
                                 0,
                                 BORDER_1_COLOR,
                                 CLICKED_BKG_COLOR,
                                 this, 1);

    /* top background tile */
    m_topBgTile = new BgTile(m_backGround->x() + SCALE_WIDTH(15), m_backGround->y() + SCALE_HEIGHT(60),
                             CLIENT_LIST_BGTILE_WIDTH,
                             (6*CLIENT_LIST_FIELD_CELL_HEIGHT) + SCALE_HEIGHT(40),
                             TOP_LAYER,
                             this);

    /* bottom background tile */
    m_bottomBgTile = new BgTile(m_topBgTile->x(),
                                m_topBgTile->y() + m_topBgTile->height(),
                                m_topBgTile->width (),
                                CLIENT_LIST_FIELD_CELL_HEIGHT,
                                BOTTOM_LAYER,
                                this);

    /* heading of the page */
    m_heading = new Heading((m_backGround->x () + (m_backGround->width () / 2)),
                            (m_backGround->y () + SCALE_HEIGHT(30)),
                            "IP List and Status",
                            this,
                            HEADING_TYPE_2);

    /* close button */
    m_closeButton = new CloseButtton ((m_backGround->x () + m_backGround->width () - SCALE_WIDTH(20)),
                                      (m_backGround->y () + SCALE_HEIGHT(30)),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      CLIENT_LIST_ELEMENT_CLOSE_BUTTON);
    m_elementList[CLIENT_LIST_ELEMENT_CLOSE_BUTTON] = m_closeButton;
    connect (m_closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_closeButton,
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
                                               m_tableCellHeadings[0]->y() +
                                               (m_tableCellHeadings[0]->height ())/2,
                                               NORMAL_FONT_SIZE,
                                               fieldHeadingStr[0],
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, 0, (SCALE_WIDTH(headerWidthArray[0])));
    /* draw columns */
    for (quint8 index = 1; index < DHCP_SERVER_IP_LIST_MAX_COL; index++)
    {
        m_tableCellHeadings[index] = new TableCell(m_tableCellHeadings[index -1]->x() +
                                                   m_tableCellHeadings[index -1]->width(),
                                                   m_tableCellHeadings[index -1]->y(),
                                                   (SCALE_WIDTH(headerWidthArray[index]) - 1),
                                                   SCALE_HEIGHT(50),
                                                   this,
                                                   true);

        m_tableHeadingTextlabel[index] = new TextLabel(m_tableCellHeadings[index]->x() + SCALE_WIDTH(10),
                                                       m_tableCellHeadings[index]->y() +
                                                       (m_tableCellHeadings[index]->height ())/2,
                                                       NORMAL_FONT_SIZE,
                                                       fieldHeadingStr[index],
                                                       this,
                                                       NORMAL_FONT_COLOR,
                                                       NORMAL_FONT_FAMILY,
                                                       ALIGN_START_X_CENTRE_Y, 0, 0, (SCALE_WIDTH(headerWidthArray[index])));
    }

    /* draw raws */
    m_ipAddrCell[0] = new TableCell(m_tableCellHeadings[0]->x(),
                                    m_tableCellHeadings[0]->y() +
                                    m_tableCellHeadings[0]->height(),
                                    m_tableCellHeadings[0]->width(),
                                    WIZARD_BG_TILE_HEIGHT,
                                    this);

    m_macAddrCell[0] = new TableCell(m_tableCellHeadings[1]->x(),
                                     m_tableCellHeadings[1]->y() +
                                     m_tableCellHeadings[1]->height(),
                                     m_tableCellHeadings[1]->width(),
                                     WIZARD_BG_TILE_HEIGHT,
                                     this);

    m_hostNameCell[0] = new TableCell(m_tableCellHeadings[2]->x(),
                                      m_tableCellHeadings[2]->y() +
                                      m_tableCellHeadings[2]->height(),
                                      m_tableCellHeadings[2]->width(),
                                      WIZARD_BG_TILE_HEIGHT,
                                      this);

    m_leaseTimeStrCell[0] = new TableCell(m_tableCellHeadings[3]->x(),
                                          m_tableCellHeadings[3]->y() +
                                          m_tableCellHeadings[3]->height(),
                                          m_tableCellHeadings[3]->width(),
                                          WIZARD_BG_TILE_HEIGHT,
                                          this);

    for (quint8 index = 1; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        m_ipAddrCell[index] = new TableCell(m_ipAddrCell[(index -1)]->x(),
                                            m_ipAddrCell[(index -1)]->y() +
                                            m_ipAddrCell[(index -1)]->height(),
                                            m_ipAddrCell[(index -1)]->width() - 1,
                                            WIZARD_BG_TILE_HEIGHT,
                                            this);

        m_macAddrCell[index] = new TableCell(m_macAddrCell[(index -1)]->x(),
                                             m_macAddrCell[(index -1)]->y() +
                                             m_macAddrCell[(index -1)]->height(),
                                             m_macAddrCell[(index -1)]->width() - 1,
                                             WIZARD_BG_TILE_HEIGHT,
                                             this);

        m_hostNameCell[index] = new TableCell(m_hostNameCell[(index -1)]->x(),
                                              m_hostNameCell[(index -1)]->y() +
                                              m_hostNameCell[(index -1)]->height(),
                                              m_hostNameCell[(index -1)]->width() - 1,
                                              WIZARD_BG_TILE_HEIGHT,
                                              this);

        m_leaseTimeStrCell[index] = new TableCell(m_leaseTimeStrCell[(index -1)]->x(),
                                                  m_leaseTimeStrCell[(index -1)]->y() +
                                                  m_leaseTimeStrCell[(index -1)]->height(),
                                                  m_leaseTimeStrCell[(index -1)]->width() - 1,
                                                  WIZARD_BG_TILE_HEIGHT,
                                                  this);
    }

    /* cell text */
    m_ipAddrCellText[0] = new TextLabel(m_ipAddrCell[0]->x () + SCALE_WIDTH(10),
                                        m_ipAddrCell[0]->y () +
                                        (m_ipAddrCell[0]->height ())/2,
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[0]->width ());


    m_macAddrCellText[0] = new TextLabel(m_macAddrCell[0]->x () + SCALE_WIDTH(10),
                                         m_macAddrCell[0]->y () +
                                         (m_macAddrCell[0]->height ())/2,
                                         NORMAL_FONT_SIZE,
                                         "",
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[1]->width ());

    m_hostNameCellText[0] = new TextLabel(m_hostNameCell[0]->x () + SCALE_WIDTH(10),
                                          m_hostNameCell[0]->y () +
                                          (m_hostNameCell[0]->height ())/2,
                                          NORMAL_FONT_SIZE,
                                          "",
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[2]->width ());

    m_leaseTimeStrCellText[0] = new TextLabel(m_leaseTimeStrCell[0]->x () + SCALE_WIDTH(10),
                                              m_leaseTimeStrCell[0]->y () +
                                              (m_leaseTimeStrCell[0]->height ())/2,
                                              NORMAL_FONT_SIZE,
                                              "",
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, 0, m_tableCellHeadings[3]->width ());

    for (quint8 index = 1; index < CLIENT_LIST_SINGLE_PAGE_RECORDS; index++)
    {
        m_ipAddrCellText[index] = new TextLabel(m_ipAddrCell[index]->x () + SCALE_WIDTH(10),
                                                m_ipAddrCell[index]->y () +
                                                (m_ipAddrCell[index]->height ())/2,
                                                NORMAL_FONT_SIZE,
                                                "",
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, m_ipAddrCell[(index-1)]->width () - 1);

        m_macAddrCellText[index] = new TextLabel(m_macAddrCell[index]->x () + SCALE_WIDTH(10),
                                                 m_macAddrCell[index]->y () +
                                                 (m_macAddrCell[index]->height ())/2,
                                                 NORMAL_FONT_SIZE,
                                                 "",
                                                 this,
                                                 NORMAL_FONT_COLOR,
                                                 NORMAL_FONT_FAMILY,
                                                 ALIGN_START_X_CENTRE_Y, 0, 0, m_macAddrCell[(index-1)]->width () - 1);

        m_hostNameCellText[index] = new TextLabel(m_hostNameCell[index]->x () + SCALE_WIDTH(10),
                                                  m_hostNameCell[index]->y () +
                                                  (m_hostNameCell[index]->height ())/2,
                                                  NORMAL_FONT_SIZE,
                                                  "",
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, 0, m_hostNameCell[(index-1)]->width () - 1);

        m_leaseTimeStrCellText[index] = new TextLabel(m_leaseTimeStrCell[index]->x () + SCALE_WIDTH(10),
                                                      m_leaseTimeStrCell[index]->y () +
                                                      (m_leaseTimeStrCell[index]->height ())/2,
                                                      NORMAL_FONT_SIZE,
                                                      "",
                                                      this,
                                                      NORMAL_FONT_COLOR,
                                                      NORMAL_FONT_FAMILY,
                                                      ALIGN_START_X_CENTRE_Y, 0, 0, m_leaseTimeStrCell[(index-1)]->width () - 1);
    }

    /* previous button */
    m_previousButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                         m_topBgTile->x() + SCALE_WIDTH(15),
                                         m_bottomBgTile->y() + SCALE_HEIGHT(20),
                                         m_topBgTile->width(),
                                         WIZARD_BG_TILE_HEIGHT,
                                         this,
                                         NO_LAYER,
                                         -1,
                                         fieldHeadingStr[CLIENT_LIST_STR_PREV_BUTTON],
                                         false,
                                         CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON,
                                         false);
    m_elementList[CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON] = m_previousButton;
    connect (m_previousButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_previousButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                     m_topBgTile->x() + m_topBgTile->width() - SCALE_WIDTH(90),
                                     m_bottomBgTile->y() + SCALE_HEIGHT(20),
                                     m_topBgTile->width(),
                                     WIZARD_BG_TILE_HEIGHT,
                                     this,
                                     NO_LAYER,
                                     -1,
                                     fieldHeadingStr[CLIENT_LIST_STR_NEXT_BUTTON],
                                     false,
                                     CLIENT_LIST_ELEMENT_NEXT_BUTTON);
    m_elementList[CLIENT_LIST_ELEMENT_NEXT_BUTTON] = m_nextButton;
    connect (m_nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_nextButton,
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
 * @brief DhcpClientStatus::slotUpdateCurrentElement
 * @param index
 */
void DhcpClientStatus::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

/**
 * @brief DhcpClientStatus::slotButtonClick
 * @param index
 */
void DhcpClientStatus::slotButtonClick(int index)
{
    switch(index)
    {
        case CLIENT_LIST_ELEMENT_PREVIOUS_BUTTON:
            if (currentPageNo > 0)
            {
                currentPageNo--;
            }

            updateClientList();
            break;

        case CLIENT_LIST_ELEMENT_NEXT_BUTTON:
            if (currentPageNo < (maximumPages - 1))
            {
                currentPageNo++;
            }

            updateClientList();
            break;

        case CLIENT_LIST_ELEMENT_CLOSE_BUTTON:
        default:
            emit sigObjectDelete();
            break;
    }
}

/**
 * @brief DhcpClientStatus::slotInfoPageBtnclick
 * @param index
 */
void DhcpClientStatus::slotInfoPageBtnclick(int index)
{
    m_elementList[m_currentElement]->forceActiveFocus();
    Q_UNUSED(index);
}

/**
 * @brief DhcpClientStatus::sendCommand
 * @param cmdType
 */
void DhcpClientStatus::sendCommand(SET_COMMAND_e cmdType)
{
    QString payloadString = "";

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;

    if (applController->processActivity(m_currDevName, DEVICE_COMM, param))
    {
        m_processBar->loadProcessBar();
    }
}

/**
 * @brief DhcpClientStatus::processDeviceResponse
 * @param param
 * @param deviceName
 */
void DhcpClientStatus::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (param->cmdType != GET_DHCP_LEASE)
    {
        return;
    }

    m_processBar->unloadProcessBar();

    /* fail response? */
    if (CMD_SUCCESS != param->deviceStatus)
    {
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    /* success response */
    parseAndUpdateClientList(param);
    Q_UNUSED(deviceName);
}

/**
 * @brief DhcpClientStatus::parseAndUpdateClientList
 * @param param
 */
void DhcpClientStatus::parseAndUpdateClientList(DevCommParam *param)
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
        m_ipAddrList.append(m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_IP_ADDR +
                                                              (index * CLIENT_LIST_CMD_REPLY_MAX)).toString ());
        m_macAddrList.append(m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_MAC_ADDR +
                                                           (index * CLIENT_LIST_CMD_REPLY_MAX)).toString ());
        m_hostNameList.append(m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_HOSTNAME +
                                                          (index * CLIENT_LIST_CMD_REPLY_MAX)).toString ());

        /* convert lease time seconds to HHH:MM format */
        leasetime = m_payloadLib->getCnfgArrayAtIndex(CLIENT_LIST_CMD_REPLY_LEASE_TIME +
                                                      (index * CLIENT_LIST_CMD_REPLY_MAX)).toUInt();

        m_leaseTimeStrList.append(QString::number(leasetime/3600).rightJustified(3, '0')
                                  + ":"
                                  + QString::number((leasetime%3600)/60).rightJustified(2, '0'));
    }

    /* derive maximum pages */
    maximumPages = (maxSearchListCount % CLIENT_LIST_SINGLE_PAGE_RECORDS == 0) ?
                (maxSearchListCount / CLIENT_LIST_SINGLE_PAGE_RECORDS) :
                ((maxSearchListCount / CLIENT_LIST_SINGLE_PAGE_RECORDS) + 1);


    /* show dhcp clients on UI */
    updateClientList();
}

/**
 * @brief DhcpClientStatus::clearClientList
 */
void DhcpClientStatus::clearClientList()
{
    m_ipAddrList.clear();
    m_macAddrList.clear();
    m_hostNameList.clear();
    m_leaseTimeStrList.clear();
    currentPageNo = 0;
}

/**
 * @brief DhcpClientStatus::updateClientList
 */
void DhcpClientStatus::updateClientList()
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
 * @brief DhcpClientStatus::updateNavigationControlStatus
 */
void DhcpClientStatus::updateNavigationControlStatus()
{
    m_previousButton->setIsEnabled((currentPageNo != 0 ? true : false));

    if (currentPageNo < (maximumPages - 1))
    {
        m_nextButton->setIsEnabled(true);
    }
    else if (currentPageNo == (maximumPages - 1))
    {
        m_nextButton->setIsEnabled(false);
    }
}
