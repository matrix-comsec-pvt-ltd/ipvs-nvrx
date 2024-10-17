#include "Lan2Setting.h"
#include "ValidationMessage.h"

#define LAN2                    	2
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(50)

typedef enum
{
    LAN2_IP_ADDR_MODE_DRP_DWN,
    LAN2_IPV4_ADDRESS,
    LAN2_IPV4_SUBNET_MASK,
    LAN2_IPV4_GATEWAY,
    LAN2_IPV6_ADDRESS,
    LAN2_IPV6_PREFIX_LEN,
    LAN2_IPV6_GATEWAY,
    MAX_LAN2_SETTING_ELEMENTS
}LAN2_SETTING_ELEMENTS_e;

typedef enum
{
    IP_ADDR_MODE_IPV4,
    IP_ADDR_MODE_IPV4_AND_IPV6,
    MAX_IP_ADDR_MODE
}IP_ADDR_MODE_e;

static const QString lan2Str[] =
{
    "IP Addressing Mode",
    "IPv4 Address",
    "Subnet Mask",
    "Default Gateway",
    "IPv6 Address",
    "Prefix Length",
    "Default Gateway",
    "MAC Address"
};

static const QMap<quint8, QString> ipAddrModeMapList =
{
    {IP_ADDR_MODE_IPV4,             "IPv4"},
    {IP_ADDR_MODE_IPV4_AND_IPV6,    "IPv4 & IPv6"},
};

Lan2Setting::Lan2Setting(QString devName, QWidget *parent) : ConfigPageControl(devName, parent, MAX_LAN2_SETTING_ELEMENTS)
{
    macAddress = new ReadOnlyElement(SCALE_WIDTH(13),
                                     SCALE_HEIGHT(200),
                                     BGTILE_LARGE_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     SCALE_WIDTH(READONLY_LARGE_WIDTH),
                                     READONLY_HEIGHT , "", this,
                                     COMMON_LAYER, -1,
                                     SCALE_WIDTH(10),
                                     lan2Str[MAX_LAN2_SETTING_ELEMENTS]);

    ipAddrModeDrpDwn = new DropDown(macAddress->x(),
                                    (macAddress->y() + macAddress->height() + SCALE_HEIGHT(5)),
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    LAN2_IP_ADDR_MODE_DRP_DWN,
                                    DROPDOWNBOX_SIZE_200,
                                    lan2Str[LAN2_IP_ADDR_MODE_DRP_DWN],
                                    ipAddrModeMapList,
                                    this,
                                    "", true, 10,
                                    COMMON_LAYER);
    m_elementList[LAN2_IP_ADDR_MODE_DRP_DWN] = ipAddrModeDrpDwn;
    connect(ipAddrModeDrpDwn,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChanged(QString,quint32)));
    connect(ipAddrModeDrpDwn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv4Address = new Ipv4TextBox(ipAddrModeDrpDwn->x(),
                                  ipAddrModeDrpDwn->y() + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  LAN2_IPV4_ADDRESS,
                                  lan2Str[LAN2_IPV4_ADDRESS],
                                  this, TOP_TABLE_LAYER,
                                  true, 0, true, false,
                                  LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN2_IPV4_ADDRESS] = ipv4Address;
    connect(ipv4Address,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv4Address,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));
    connect(ipv4Address,
            SIGNAL(sigEntryDone(quint32)),
            this,
            SLOT(slotIpTextBoxEntryDone(quint32)));

    ipv4Subnet = new Ipv4TextBox(ipv4Address->x(),
                                 (ipv4Address->y() + ipv4Address->height()),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 LAN2_IPV4_SUBNET_MASK,
                                 lan2Str[LAN2_IPV4_SUBNET_MASK],
                                 this,
                                 MIDDLE_TABLE_LAYER,
                                 true, 0, true, true,
                                 LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN2_IPV4_SUBNET_MASK] = ipv4Subnet;
    connect(ipv4Subnet,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv4Subnet,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    ipv4GateWay = new Ipv4TextBox(ipv4Subnet->x(),
                                  (ipv4Subnet->y() + ipv4Subnet->height()),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  LAN2_IPV4_GATEWAY,
                                  lan2Str[LAN2_IPV4_GATEWAY],
                                  this, BOTTOM_TABLE_LAYER,
                                  true, 0, true, false,
                                  LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN2_IPV4_GATEWAY] = ipv4GateWay;
    connect(ipv4GateWay,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv4GateWay,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    ipv6Address = new IpTextBox(ipv4Address->x() + BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(5),
                                ipv4Address->y(),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                LAN2_IPV6_ADDRESS,
                                lan2Str[LAN2_IPV6_ADDRESS],
                                IP_ADDR_TYPE_IPV6_ONLY,
                                this, TOP_TABLE_LAYER,
                                true, 0, true,
                                IP_FIELD_TYPE_IPV6_ADDR,
                                IP_TEXTBOX_ULTRALARGE,
                                LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN2_IPV6_ADDRESS] = ipv6Address;
    connect(ipv6Address,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv6Address,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    prefixLenParam = new TextboxParam();
    prefixLenParam->labelStr = lan2Str[LAN2_IPV6_PREFIX_LEN];
    prefixLenParam->maxChar = 3;
    prefixLenParam->validation = QRegExp(QString("[0-9]"));
    prefixLenParam->isNumEntry = true;
    prefixLenParam->minNumValue = 1;
    prefixLenParam->maxNumValue = 128;

    ipv6PrefixLen = new TextBox(ipv6Address->x(),
                                ipv6Address->y() + ipv6Address->height(),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                LAN2_IPV6_PREFIX_LEN,
                                TEXTBOX_MEDIAM,
                                this, prefixLenParam,
                                MIDDLE_TABLE_LAYER,
                                true, false, false,
                                LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN2_IPV6_PREFIX_LEN] = ipv6PrefixLen;
    connect(ipv6PrefixLen,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv6GateWay = new IpTextBox(ipv6PrefixLen->x(),
                                ipv6PrefixLen->y() + ipv6PrefixLen->height(),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                LAN2_IPV6_GATEWAY,
                                lan2Str[LAN2_IPV6_GATEWAY],
                                IP_ADDR_TYPE_IPV6_ONLY,
                                this, BOTTOM_TABLE_LAYER,
                                true, 0, true,
                                IP_FIELD_TYPE_IPV6_ADDR,
                                IP_TEXTBOX_ULTRALARGE,
                                LEFT_MARGIN_FROM_CENTER);
    m_elementList[LAN2_IPV6_GATEWAY] = ipv6GateWay;
    connect(ipv6GateWay,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv6GateWay,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    isIpAddrDropDwnChng = false;
    Lan2Setting::getConfig();
    this->show();
}

Lan2Setting::~Lan2Setting()
{
    DELETE_OBJ(macAddress);

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

    if (IS_VALID_OBJ(ipv4Address))
    {
        disconnect(ipv4Address,
                   SIGNAL(sigEntryDone(quint32)),
                   this,
                   SLOT(slotIpTextBoxEntryDone(quint32)));
        disconnect(ipv4Address,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(ipv4Address,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpTextBoxLoadInfopage(quint32)));
        DELETE_OBJ(ipv4Address);
    }

    if (IS_VALID_OBJ(ipv4Subnet))
    {
        disconnect(ipv4Subnet,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(ipv4Subnet,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpTextBoxLoadInfopage(quint32)));
        DELETE_OBJ(ipv4Subnet);
    }

    if (IS_VALID_OBJ(ipv4GateWay))
    {
        disconnect(ipv4GateWay,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(ipv4GateWay,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpTextBoxLoadInfopage(quint32)));
        DELETE_OBJ(ipv4GateWay);
    }

    if (IS_VALID_OBJ(ipv6Address))
    {
        disconnect(ipv6Address,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(ipv6Address,
                  SIGNAL(sigLoadInfopage(quint32)),
                  this,
                  SLOT(slotIpTextBoxLoadInfopage(quint32)));
        DELETE_OBJ(ipv6Address);
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

    if (IS_VALID_OBJ(ipv6GateWay))
    {
        disconnect(ipv6GateWay,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(ipv6GateWay,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpTextBoxLoadInfopage(quint32)));
        DELETE_OBJ(ipv6GateWay);
    }
}

void Lan2Setting::getMac2Address()
{
    payloadLib->setCnfgArrayAtIndex(0, LAN2);
    sendCommand(GET_MAC, 1);
}

void Lan2Setting:: sendCommand(SET_COMMAND_e cmdType, int totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Lan2Setting::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             LAN2_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             MAX_LAN2_FIELDS,
                                                             MAX_LAN2_FIELDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Lan2Setting::getConfig()
{
    createPayload(MSG_GET_CFG);
}

void Lan2Setting::saveConfig()
{
    if((ipv4Address->getIpaddress() == "0.0.0.0") || (ipv4Address->getIpaddress() == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
        return;
    }

    if((ipv4Subnet->getIpaddress() == "0.0.0.0") || (ipv4Subnet->getIpaddress() == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_SUBNET_MASK));
        return;
    }

    if((ipv4GateWay->getIpaddress() == "0.0.0.0") || (ipv4GateWay->getIpaddress() == ""))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
        return;
    }

    if(((IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement() == IP_ADDR_MODE_IPV4_AND_IPV6))
    {
        if (ipv6Address->getIpaddress() == "")
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
            return;
        }

        if (ipv6GateWay->getIpaddress() == "")
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
            return;
        }
    }

    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IP_ADDR_MODE, (IP_ADDR_MODE_e)ipAddrModeDrpDwn->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IPV4_ADDR, ipv4Address->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IPV4_SUBNET, ipv4Subnet->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IPV4_GATEWAY, ipv4GateWay->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IPV6_ADDR, ipv6Address->getIpaddress());
    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IPV6_PREFIX_LEN, ipv6PrefixLen->getInputText());
    payloadLib->setCnfgArrayAtIndex(LAN2_FIELD_IPV6_GATEWAY, ipv6GateWay->getIpaddress());

    createPayload(MSG_SET_CFG);
}

void Lan2Setting::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void Lan2Setting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        update();
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex() != LAN2_TABLE_INDEX)
            {
                break;
            }

            if (isIpAddrDropDwnChng == false)
            {
                ipAddrModeDrpDwn->setIndexofCurrElement((IP_ADDR_MODE_e)payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IP_ADDR_MODE).toUInt());
            }

            ipv4Address->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IPV4_ADDR).toString());
            ipv4Subnet->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IPV4_SUBNET).toString());
            ipv4GateWay->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IPV4_GATEWAY).toString());

            ipv6Address->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IPV6_ADDR).toString());
            ipv6PrefixLen->setInputText(payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IPV6_PREFIX_LEN).toString());
            ipv6GateWay->setIpaddress(payloadLib->getCnfgArrayAtIndex(LAN2_FIELD_IPV6_GATEWAY).toString());

            processBar->unloadProcessBar();
            getMac2Address();
            enableDisableIpv6Ctrls();
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
            macAddress->changeValue(payloadLib->getCnfgArrayAtIndex(0).toString());
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

void Lan2Setting::enableDisableIpv6Ctrls()
{
    bool isEnable = (ipAddrModeDrpDwn->getIndexofCurrElement() == IP_ADDR_MODE_IPV4_AND_IPV6) ? true : false;
    ipv6Address->setIsEnabled(isEnable);
    ipv6PrefixLen->setIsEnabled(isEnable);
    ipv6GateWay->setIsEnabled(isEnable);
}

void Lan2Setting::slotIpTextBoxLoadInfopage(quint32 index)
{
    switch(index)
    {
        case LAN2_IPV4_ADDRESS:
        case LAN2_IPV6_ADDRESS:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_IP_ADDR));
            break;

        case LAN2_IPV4_SUBNET_MASK:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_SUBNET_MASK));
            break;

        case LAN2_IPV4_GATEWAY:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_GATEWAY_MASK));
            break;

        case LAN2_IPV6_GATEWAY:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_GATEWAY_MASK));
            break;

        default:
            break;
    }
}

void Lan2Setting::slotIpTextBoxEntryDone(quint32)
{
    QString ipAddr2 = ipv4Address->getSubnetOfIpaddress();
    if(ipAddr2 == "0.0.0")
    {
        getConfig();
    }

    ipAddr2.append(".").append("1");
    ipv4GateWay->setIpaddress(ipAddr2);
}

void Lan2Setting::slotSpinBoxValueChanged(QString, quint32 index)
{
    if (index != LAN2_IP_ADDR_MODE_DRP_DWN)
    {
        return;
    }

    isIpAddrDropDwnChng = true;
    getConfig();
}
