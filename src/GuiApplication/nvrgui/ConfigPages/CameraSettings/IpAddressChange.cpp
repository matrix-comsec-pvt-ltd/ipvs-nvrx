#include "IpAddressChange.h"
#include <QPainter>
#include <QKeyEvent>
#include "ValidationMessage.h"

#define IP_ADDRESS_CHANGE_WIDTH       (BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(40))
#define IP_ADDRESS_CHANGE_HEIGHT      SCALE_HEIGHT(250) + BGTILE_HEIGHT
#define IP_ADDRESS_CHANGE_BG_WIDTH    BGTILE_MEDIUM_SIZE_WIDTH
#define LEFT_MARGIN_FROM_CENTER       SCALE_WIDTH(50)

static const QString ipAddrChangeStr[] =
{
    "Change IP Address",
    "IP Address Type",
    "IPv4 Address",
    "Subnet Mask",
    "Default Gateway",
    "IPv6 Address",
    "Prefix Length",
    "Default Gateway",
    "OK"
};

static QMap<quint8, QString> ipAddrTypeMapList =
{
    {IP_ADDR_FAMILY_IPV4, "IPv4"},
    {IP_ADDR_FAMILY_IPV6, "IPv6"}
};

IpAddressChange::IpAddressChange(QString ipAddr, QString, QString, QWidget *parent): KeyBoard(parent)
{
    IP_ADDR_FAMILY_e    ipAddrType;
    QHostAddress        cameraAddress(ipAddr);

    this->setGeometry(0, 0, parent->width(), parent->height());
    isCommandSend = false;
    createDefaultElements();

    if(cameraAddress.protocol() == QAbstractSocket::IPv4Protocol)
    {
        ipv4AddrTextBox->setIpaddress(ipAddr);
        ipv4SubnetTextbox->setIpaddress("255.255.255.0");
        ipv4GatewayTextbox->setIpaddress("");
        ipAddrType = IP_ADDR_FAMILY_IPV4;
    }
    else if(cameraAddress.protocol() == QAbstractSocket::IPv6Protocol)
    {
        ipv6AddrTextBox->setIpaddress(ipAddr);
        ipv6PrefixLenTextbox->setInputText("64");
        ipv6GatewayTextbox->setIpaddress("");
        ipAddrType = IP_ADDR_FAMILY_IPV6;
    }
    else
    {
        ipv4AddrTextBox->setIpaddress("");
        ipv4SubnetTextbox->setIpaddress("");
        ipv4GatewayTextbox->setIpaddress("");
        ipAddrType = IP_ADDR_FAMILY_IPV4;
    }

    ipAddrTypeDropdown->setCurrValue(ipAddrTypeMapList[ipAddrType]);
    slotSpinboxValueChanged(ipAddrTypeMapList[ipAddrType], IP_ADDR_CHNG_IP_ADDR_DROPDOWN);
    this->show();
}

IpAddressChange::~IpAddressChange()
{
    delete backGround;
    disconnect(closeButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(closeButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;
    delete heading;

    disconnect(ipAddrTypeDropdown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(ipAddrTypeDropdown,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinboxValueChanged(QString,quint32)));
    delete ipAddrTypeDropdown;

    disconnect(ipv4AddrTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(ipv4AddrTextBox,
               SIGNAL(sigLoadInfopage(quint32)),
               this,
               SLOT(slotIpTextBoxLoadInfopage(quint32)));
    delete ipv4AddrTextBox;

    disconnect(ipv4SubnetTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(ipv4SubnetTextbox,
               SIGNAL(sigLoadInfopage(quint32)),
               this,
               SLOT(slotIpTextBoxLoadInfopage(quint32)));
    delete ipv4SubnetTextbox;

    disconnect(ipv4GatewayTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(ipv4GatewayTextbox,
               SIGNAL(sigLoadInfopage(quint32)),
               this,
               SLOT(slotIpTextBoxLoadInfopage(quint32)));
    delete ipv4GatewayTextbox;

    disconnect(ipv6AddrTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(ipv6AddrTextBox,
               SIGNAL(sigLoadInfopage(quint32)),
               this,
               SLOT(slotIpTextBoxLoadInfopage(quint32)));
    delete ipv6AddrTextBox;

    disconnect(ipv6PrefixLenTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete ipv6PrefixLenTextbox;
    delete ipv6PrefixLenParam;

    disconnect(ipv6GatewayTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(ipv6GatewayTextbox,
               SIGNAL(sigLoadInfopage(quint32)),
               this,
               SLOT(slotIpTextBoxLoadInfopage(quint32)));
    delete ipv6GatewayTextbox;

    disconnect(okButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(okButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    disconnect(infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
}

void IpAddressChange::createDefaultElements()
{
    backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - IP_ADDRESS_CHANGE_WIDTH) / 2)),
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- IP_ADDRESS_CHANGE_HEIGHT) / 2)),
                               IP_ADDRESS_CHANGE_WIDTH,
                               IP_ADDRESS_CHANGE_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton((backGround->x()+ backGround->width() - SCALE_WIDTH(20)),
                                   (backGround->y() + SCALE_HEIGHT(20)),
                                   this,
                                   CLOSE_BTN_TYPE_1,
                                   IP_ADDR_CHNG_CLOSE_BUTTON);
    m_elementlist[IP_ADDR_CHNG_CLOSE_BUTTON] = closeButton;
    connect(closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading((backGround->x() + (backGround->width() / 2)),
                          (backGround->y() + SCALE_HEIGHT(20)),
                          ipAddrChangeStr[0],
                          this,
                          HEADING_TYPE_2);

    ipAddrTypeDropdown = new DropDown(backGround->x() + SCALE_WIDTH(20),
                                      backGround->y() + SCALE_HEIGHT(50),
                                      IP_ADDRESS_CHANGE_BG_WIDTH,
                                      BGTILE_HEIGHT,
                                      IP_ADDR_CHNG_IP_ADDR_DROPDOWN,
                                      DROPDOWNBOX_SIZE_200,
                                      ipAddrChangeStr[IP_ADDR_CHNG_IP_ADDR_DROPDOWN],
                                      ipAddrTypeMapList,
                                      this, "", true, 0,
                                      COMMON_LAYER, true , 8 , false, false,
                                      5, LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IP_ADDR_DROPDOWN] = ipAddrTypeDropdown;
    connect(ipAddrTypeDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipAddrTypeDropdown,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinboxValueChanged(QString,quint32)));

    ipv4AddrTextBox = new Ipv4TextBox(ipAddrTypeDropdown->x(),
                                      ipAddrTypeDropdown->y() + BGTILE_HEIGHT,
                                      IP_ADDRESS_CHANGE_BG_WIDTH,
                                      BGTILE_HEIGHT,
                                      IP_ADDR_CHNG_IPV4_ADDR_TEXTBOX,
                                      ipAddrChangeStr[IP_ADDR_CHNG_IPV4_ADDR_TEXTBOX],
                                      this, COMMON_LAYER, true, 0, true, false,
                                      LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IPV4_ADDR_TEXTBOX] = ipv4AddrTextBox;
    connect(ipv4AddrTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv4AddrTextBox,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));
    connect(ipv4AddrTextBox,
            SIGNAL(sigEntryDone(quint32)),
            this,
            SLOT(slotIpTextBoxEntryDone(quint32)));

    ipv4SubnetTextbox = new Ipv4TextBox(ipv4AddrTextBox->x(),
                                        (ipv4AddrTextBox->y() + ipv4AddrTextBox->height()),
                                        IP_ADDRESS_CHANGE_BG_WIDTH,
                                        BGTILE_HEIGHT,
                                        IP_ADDR_CHNG_IPV4_SUBNET_TEXTBOX,
                                        ipAddrChangeStr[IP_ADDR_CHNG_IPV4_SUBNET_TEXTBOX],
                                        this, COMMON_LAYER, true, 0, true, true,
                                        LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IPV4_SUBNET_TEXTBOX] = ipv4SubnetTextbox;
    connect(ipv4SubnetTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect(ipv4SubnetTextbox,
             SIGNAL(sigLoadInfopage(quint32)),
             this,
             SLOT(slotIpTextBoxLoadInfopage(quint32)));

    ipv4GatewayTextbox = new Ipv4TextBox(ipv4SubnetTextbox->x(),
                                         (ipv4SubnetTextbox->y() + ipv4SubnetTextbox->height()),
                                         IP_ADDRESS_CHANGE_BG_WIDTH,
                                         BGTILE_HEIGHT,
                                         IP_ADDR_CHNG_IPV4_GATEWAY_TEXTBOX,
                                         ipAddrChangeStr[IP_ADDR_CHNG_IPV4_GATEWAY_TEXTBOX],
                                         this, COMMON_LAYER, true, 0, true, false,
                                         LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IPV4_GATEWAY_TEXTBOX] = ipv4GatewayTextbox;
    connect(ipv4GatewayTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv4GatewayTextbox,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    ipv6AddrTextBox = new IpTextBox(ipAddrTypeDropdown->x(),
                                    ipAddrTypeDropdown->y() + BGTILE_HEIGHT,
                                    IP_ADDRESS_CHANGE_BG_WIDTH,
                                    BGTILE_HEIGHT,
                                    IP_ADDR_CHNG_IPV4_ADDR_TEXTBOX,
                                    ipAddrChangeStr[IP_ADDR_CHNG_IPV6_ADDR_TEXTBOX],
                                    IP_ADDR_TYPE_IPV6_ONLY,
                                    this, COMMON_LAYER,
                                    true, 0, true,
                                    IP_FIELD_TYPE_IPV6_ADDR,
                                    IP_TEXTBOX_ULTRALARGE,
                                    LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IPV6_ADDR_TEXTBOX] = ipv6AddrTextBox;
    connect(ipv6AddrTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv6AddrTextBox,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    ipv6PrefixLenParam = new TextboxParam();
    ipv6PrefixLenParam->labelStr = ipAddrChangeStr[IP_ADDR_CHNG_IPV6_PREFIX_LEN_TEXTBOX];
    ipv6PrefixLenParam->maxChar = 3;
    ipv6PrefixLenParam->validation = QRegExp(QString("[0-9]"));
    ipv6PrefixLenParam->isNumEntry = true;
    ipv6PrefixLenParam->minNumValue = 1;
    ipv6PrefixLenParam->maxNumValue = 128;

    ipv6PrefixLenTextbox = new TextBox(ipv6AddrTextBox->x(),
                                       ipv6AddrTextBox->y() + BGTILE_HEIGHT,
                                       IP_ADDRESS_CHANGE_BG_WIDTH,
                                       BGTILE_HEIGHT,
                                       IP_ADDR_CHNG_IPV6_PREFIX_LEN_TEXTBOX,
                                       TEXTBOX_MEDIAM,
                                       this, ipv6PrefixLenParam,
                                       COMMON_LAYER,
                                       true, false, false,
                                       LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IPV6_PREFIX_LEN_TEXTBOX] = ipv6PrefixLenTextbox;
    connect(ipv6PrefixLenTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv6GatewayTextbox = new IpTextBox(ipv6PrefixLenTextbox->x(),
                                       ipv6PrefixLenTextbox->y() + BGTILE_HEIGHT,
                                       IP_ADDRESS_CHANGE_BG_WIDTH,
                                       BGTILE_HEIGHT,
                                       IP_ADDR_CHNG_IPV6_GATEWAY_TEXTBOX,
                                       ipAddrChangeStr[IP_ADDR_CHNG_IPV6_GATEWAY_TEXTBOX],
                                       IP_ADDR_TYPE_IPV6_ONLY,
                                       this, COMMON_LAYER,
                                       true, 0, true,
                                       IP_FIELD_TYPE_IPV6_ADDR,
                                       IP_TEXTBOX_ULTRALARGE,
                                       LEFT_MARGIN_FROM_CENTER);
    m_elementlist[IP_ADDR_CHNG_IPV6_GATEWAY_TEXTBOX] = ipv6GatewayTextbox;
    connect(ipv6GatewayTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipv6GatewayTextbox,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpTextBoxLoadInfopage(quint32)));

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              ipv4GatewayTextbox->x() + (IP_ADDRESS_CHANGE_BG_WIDTH/2),
                              (ipv4GatewayTextbox->y() + ipv4GatewayTextbox->height() + SCALE_HEIGHT(40)),
                              ipAddrChangeStr[IP_ADDR_CHNG_OK_BUTTON],
                              this,
                              IP_ADDR_CHNG_OK_BUTTON);
    m_elementlist[IP_ADDR_CHNG_OK_BUTTON] = okButton;
    connect(okButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(okButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    infoPage = new InfoPage(0, 0,
                            SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                            SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                            INFO_CONFIG_PAGE,
                            this);
    connect(infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageBtnclick(int)));

    currElement = IP_ADDR_CHNG_CLOSE_BUTTON;
    m_elementlist[currElement]->forceActiveFocus();
}

void IpAddressChange::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor color;
    QRect mainRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT));
    color.setAlpha(150);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(mainRect, SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void IpAddressChange::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void IpAddressChange::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_IP_ADDRESS_CHANGE_ELEMENTS) % MAX_IP_ADDRESS_CHANGE_ELEMENTS;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void IpAddressChange::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_IP_ADDRESS_CHANGE_ELEMENTS;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void IpAddressChange::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void IpAddressChange::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = IP_ADDR_CHNG_CLOSE_BUTTON;
    m_elementlist[currElement]->forceActiveFocus();
}

void IpAddressChange::slotButtonClicked(int indexInPage)
{
    switch(indexInPage)
    {
        case IP_ADDR_CHNG_OK_BUTTON:
        {
            if(ipAddrTypeDropdown->getCurrValue() == ipAddrTypeMapList[IP_ADDR_FAMILY_IPV4])
            {
                if((ipv4AddrTextBox->getIpaddress() == "0.0.0.0") || (ipv4AddrTextBox->getIpaddress() == ""))
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
                }
                else if((ipv4SubnetTextbox->getIpaddress() == "0.0.0.0") || (ipv4SubnetTextbox->getIpaddress() == ""))
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_SUBNET_MASK));
                }
                else if((ipv4GatewayTextbox->getIpaddress() == "0.0.0.0") || (ipv4GatewayTextbox->getIpaddress() == ""))
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
                }
                else if(ipv4AddrTextBox->getSubnetOfIpaddress() != ipv4GatewayTextbox->getSubnetOfIpaddress())
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_ADD_CHANGE_IP_ADDR_AND_DEF_GATEWAY_SAME_SUBNET));
                }
                else
                {
                    isCommandSend = true;
                    emit sigDataSelectedForIpChange(ipv4AddrTextBox->getIpaddress(),
                                                    ipv4SubnetTextbox->getSubnetToPrefixLength(),
                                                    ipv4GatewayTextbox->getIpaddress());
                }
            }
            else
            {
                if(ipv6AddrTextBox->getIpaddress() == "")
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
                }
                else if(ipv6PrefixLenTextbox->getInputText() == "")
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_PREFIX_LEN));
                }
                else if(ipv6GatewayTextbox->getIpaddress() == "")
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_DEFAULT_GATEWAY));
                }
                else
                {
                    isCommandSend = true;
                    emit sigDataSelectedForIpChange(ipv6AddrTextBox->getIpaddress(),
                                                    ipv6PrefixLenTextbox->getInputText(),
                                                    ipv6GatewayTextbox->getIpaddress());
                }
            }
        }
        break;

        case IP_ADDR_CHNG_CLOSE_BUTTON:
        default:
        {
            emit sigObjectDelete();
        }
        break;
    }
}

void IpAddressChange::slotUpdateCurrentElement(int indexInPage)
{
    currElement = indexInPage;
}

void IpAddressChange::slotIpTextBoxLoadInfopage(quint32 indexInPage)
{
    switch(indexInPage)
    {
        case IP_ADDR_CHNG_IPV4_ADDR_TEXTBOX:
        case IP_ADDR_CHNG_IPV6_ADDR_TEXTBOX:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_IP_ADDR));
            break;

        case IP_ADDR_CHNG_IPV4_SUBNET_TEXTBOX:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_SUBNET_MASK));
            break;

        case IP_ADDR_CHNG_IPV4_GATEWAY_TEXTBOX:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_GATEWAY_MASK));
            break;

        case IP_ADDR_CHNG_IPV6_GATEWAY_TEXTBOX:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAN_SETT_ENT_CORRECT_GATEWAY_MASK));
            break;

        default:
            break;
    }
}

void IpAddressChange::slotInfoPageBtnclick(int)
{
    infoPage->unloadInfoPage();
    m_elementlist[currElement]->forceActiveFocus();
}

void IpAddressChange::slotIpTextBoxEntryDone(quint32)
{
    QString gateWay = ipv4AddrTextBox->getSubnetOfIpaddress();

    gateWay.append(".1");

    ipv4GatewayTextbox->setIpaddress(gateWay);
}

void IpAddressChange::slotSpinboxValueChanged(QString str, quint32)
{
    bool status = (str == ipAddrTypeMapList[IP_ADDR_FAMILY_IPV4]);

    ipv4AddrTextBox->setVisible(status);
    ipv4SubnetTextbox->setVisible(status);
    ipv4GatewayTextbox->setVisible(status);

    ipv6AddrTextBox->setVisible(!status);
    ipv6PrefixLenTextbox->setVisible(!status);
    ipv6GatewayTextbox->setVisible(!status);
}

void IpAddressChange::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void IpAddressChange::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void IpAddressChange::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}
