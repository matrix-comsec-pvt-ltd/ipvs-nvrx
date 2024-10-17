#ifndef IPADDRESSCHANGE_H
#define IPADDRESSCHANGE_H

#include <QWidget>
#include <QHostAddress>

#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/InfoPage.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/IpTextBox.h"
#include "Controls/TextBox.h"
#include "Controls/CnfgButton.h"
#include "Controls/DropDown.h"
#include "KeyBoard.h"

typedef enum
{
    IP_ADDR_CHNG_CLOSE_BUTTON,
    IP_ADDR_CHNG_IP_ADDR_DROPDOWN,
    IP_ADDR_CHNG_IPV4_ADDR_TEXTBOX,
    IP_ADDR_CHNG_IPV4_SUBNET_TEXTBOX,
    IP_ADDR_CHNG_IPV4_GATEWAY_TEXTBOX,
    IP_ADDR_CHNG_IPV6_ADDR_TEXTBOX,
    IP_ADDR_CHNG_IPV6_PREFIX_LEN_TEXTBOX,
    IP_ADDR_CHNG_IPV6_GATEWAY_TEXTBOX,
    IP_ADDR_CHNG_OK_BUTTON,
    MAX_IP_ADDRESS_CHANGE_ELEMENTS
}IP_ADDRESS_CHANGE_ELEMENTS_e;

class IpAddressChange: public KeyBoard
{
    Q_OBJECT

public:
    IpAddressChange(QString ipAddr, QString subnet, QString defaultGateWay, QWidget *parent);
    ~IpAddressChange();

    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *event);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

signals:
    void sigObjectDelete();
    void sigDataSelectedForIpChange(QString,QString,QString);

public slots:
    void slotUpdateCurrentElement(int);
    void slotButtonClicked(int index);
    void slotIpTextBoxLoadInfopage(quint32);
    void slotInfoPageBtnclick(int);
    void slotIpTextBoxEntryDone(quint32);
    void slotSpinboxValueChanged(QString, quint32);

private:

    Rectangle*          backGround;
    CloseButtton*       closeButton;
    Heading*            heading;
    DropDown*           ipAddrTypeDropdown;

    Ipv4TextBox*        ipv4AddrTextBox;
    Ipv4TextBox*        ipv4SubnetTextbox;
    Ipv4TextBox*        ipv4GatewayTextbox;

    IpTextBox*          ipv6AddrTextBox;
    TextBox*            ipv6PrefixLenTextbox;
    TextboxParam*       ipv6PrefixLenParam;
    IpTextBox*          ipv6GatewayTextbox;

    CnfgButton*         okButton;
    InfoPage*           infoPage;
    NavigationControl*  m_elementlist[MAX_IP_ADDRESS_CHANGE_ELEMENTS];
    quint8              currElement;
    bool                isCommandSend;

    void createDefaultElements();
    void takeLeftKeyAction();
    void takeRightKeyAction();
};

#endif // IPADDRESSCHANGE_H
