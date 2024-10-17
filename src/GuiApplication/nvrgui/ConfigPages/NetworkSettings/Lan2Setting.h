#ifndef LAN2SETTING_H
#define LAN2SETTING_H

#include <QWidget>
#include "Controls/Ipv4TextBox.h"
#include "Controls/IpTextBox.h"
#include "Controls/TextBox.h"
#include "Controls/DropDown.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/ConfigPageControl.h"

typedef enum
{
    LAN2_FIELD_IPV4_ADDR,
    LAN2_FIELD_IPV4_SUBNET,
    LAN2_FIELD_IPV4_GATEWAY,
    LAN2_FIELD_IP_ADDR_MODE,
    LAN2_FIELD_IPV6_ADDR,
    LAN2_FIELD_IPV6_PREFIX_LEN,
    LAN2_FIELD_IPV6_GATEWAY,
    MAX_LAN2_FIELDS
}LAN2_CNFG_FIELD_e;

class Lan2Setting : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit Lan2Setting(QString devName, QWidget *parent = 0);
    ~Lan2Setting();

    void getConfig ();
    void defaultConfig ();
    void saveConfig ();
    void processDeviceResponse (DevCommParam *param, QString deviceName);

signals:
    
public slots:
    void slotIpTextBoxLoadInfopage(quint32);
    void slotIpTextBoxEntryDone (quint32);
    void slotSpinBoxValueChanged(QString, quint32 index);

private:
    // private Variable
    Ipv4TextBox*        ipv4Address;
    Ipv4TextBox*        ipv4Subnet;
    Ipv4TextBox*        ipv4GateWay;
    IpTextBox*          ipv6Address;
    TextBox*            ipv6PrefixLen;
    IpTextBox*          ipv6GateWay;
    TextboxParam*       prefixLenParam;
    ReadOnlyElement*    macAddress;
    DropDown*           ipAddrModeDrpDwn;
    bool                isIpAddrDropDwnChng;

    //private Function
    void getMac2Address();
    void createPayload(REQ_MSG_ID_e msgType);
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds = 0);
    void enableDisableIpv6Ctrls();
};

#endif // LAN2SETTING_H
