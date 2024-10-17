#ifndef DHCP_SERVER_SETTINGS_H
#define DHCP_SERVER_SETTINGS_H

//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "Elidedlabel.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/ControlButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/Bgtile.h"
#include "Controls/TableCell.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextBox.h"
#include "Controls/TextLabel.h"
#include "Controls/DropDown.h"
#include "Controls/PageOpenButton.h"
#include "WizardCommon.h"
#include "NetworkSetting.h"
#include "PayloadLib.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* DHCP maximum clients */
#define DHCP_SERVER_IP_LIST_MAX_ROW         100

/* DHCP client status columns */
#define DHCP_SERVER_IP_LIST_MAX_COL         4
#define CLIENT_LIST_SINGLE_PAGE_RECORDS     8

//#################################################################################################
// @TYPEDEFS
//#################################################################################################
/* Network interfaces */
typedef enum
{
    DHCP_SERVER_INTERFACE_LAN1 = 0,
    DHCP_SERVER_INTERFACE_LAN2,
    DHCP_SERVER_INTERFACE_MAX

}DHCP_SERVER_INTERFACE_e;

//#################################################################################################
// @CLASSES
//#################################################################################################
/**
 * @brief The DhcpClientStatus class
 */
class DhcpClientStatus : public KeyBoard, public NavigationControl
{
    Q_OBJECT

private:

    /* Protocol elements */
    QString                 m_currDevName;
    PayloadLib*             m_payloadLib;
    ApplController*         applController;
    ProcessBar*             m_processBar;
    InfoPage*               m_infoPage;

    /* UI elements */
    NavigationControl*      m_elementList[10];
    quint8                  m_currentElement;
    BgTile*                 m_topBgTile;
    BgTile*                 m_bottomBgTile;
    Rectangle*              m_backGround;
    Heading*                m_heading;
    ControlButton*          m_previousButton;
    ControlButton*          m_nextButton;
    CloseButtton*           m_closeButton;
    TableCell*              m_tableCellHeadings[DHCP_SERVER_IP_LIST_MAX_COL];
    TextLabel*              m_tableHeadingTextlabel[DHCP_SERVER_IP_LIST_MAX_COL];

    TableCell*              m_ipAddrCell[CLIENT_LIST_SINGLE_PAGE_RECORDS];
    TableCell*              m_macAddrCell[CLIENT_LIST_SINGLE_PAGE_RECORDS];
    TableCell*              m_leaseTimeStrCell[CLIENT_LIST_SINGLE_PAGE_RECORDS];
    TableCell*              m_hostNameCell[CLIENT_LIST_SINGLE_PAGE_RECORDS];

    TextLabel*              m_ipAddrCellText[CLIENT_LIST_SINGLE_PAGE_RECORDS];
    TextLabel*              m_macAddrCellText[CLIENT_LIST_SINGLE_PAGE_RECORDS];
    TextLabel*              m_leaseTimeStrCellText[CLIENT_LIST_SINGLE_PAGE_RECORDS];
    TextLabel*              m_hostNameCellText[CLIENT_LIST_SINGLE_PAGE_RECORDS];

    /* status variables */
    quint8                  currentPageNo;
    quint8                  maximumPages;
    quint8                  maxSearchListCount;

    /* client list database */
    QStringList             m_ipAddrList;
    QStringList             m_macAddrList;
    QStringList             m_leaseTimeStrList;
    QStringList             m_hostNameList;

    void parseAndUpdateClientList(DevCommParam *param);
    void createDefaultComponent();
    void sendCommand(SET_COMMAND_e cmdType);
    void clearClientList();
    void updateClientList();
    void updateNavigationControlStatus();

public:

    explicit DhcpClientStatus(QString devName, QWidget *parent = 0, PayloadLib *payloadLib = NULL);
    ~DhcpClientStatus();

    void processDeviceResponse(DevCommParam *param, QString deviceName);

signals:
    void sigObjectDelete();
    void sigInfoPageCnfgBtnClick();

public slots:

    void slotUpdateCurrentElement(int index);
    void slotButtonClick(int index);
    void slotInfoPageBtnclick(int index);
};

/**
 * @brief The DhcpServer class
 */
class DhcpServer : public WizardCommon
{
    Q_OBJECT

private:

    TextLabel               *m_dhcpServerHeading;
    OptionSelectButton      *m_enableDhcpServer;
    DropDown                *m_intrfaceDropDownBox;
    Ipv4TextBox               *m_startIpAddress;
    TextboxParam            *m_numberOfHostsParam;
    TextBox                 *m_numberOfHosts;
    Ipv4TextBox               *m_subnetMask;
    Ipv4TextBox               *m_gatewayAddr;
    Ipv4TextBox               *m_dnsIpAddr;
    TextboxParam            *m_leaseHoursParam;
    TextBox                 *m_leaseHours;
    PageOpenButton          *m_dhcpClientListButton;
    ElementHeading          *m_footerNote;
    ElidedLabel             *m_footerNoteLable;
    InfoPage                *m_infoPage;
    PayloadLib              *payloadLib;
    ApplController          *applController;
    DEV_TABLE_INFO_t        devTableInfo;

    OPTION_STATE_TYPE_e     m_enableState;
    DHCP_SERVER_INTERFACE_e m_cnfgInterface;
    QString                 m_cnfgStartIpAddr;
    QString                 m_cnfgGatewayAddr;
    QString                 m_cnfgDnsAddr;
    WIZ_IPV4_ASSIGN_MODE_e  m_lan1IpMode;
    QString                 m_lan1IpAddr;
    QString                 m_lan1Subnet;
    QString                 m_lan1Gateway;
    QString                 m_lan1Dns;
    QString                 m_lan2IpAddr;
    QString                 m_lan2Subnet;
    QString                 m_lan2Gateway;
    QString                 currDevName;

    /* DHCP client list */
    DhcpClientStatus        *m_clientList;

    void updateFieldStatus(OPTION_STATE_TYPE_e state);

public:

    explicit DhcpServer(QString devName, QString subHeadStr, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);
    ~DhcpServer();

    void createDefaultComponent(QString subHeadStr);
    void getConfig ();
    void saveConfig ();
    void createPayload(REQ_MSG_ID_e requestType);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

public slots:

    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int index);
    void slotInterfaceDropdownChanged(QString str, quint32 index);
    void slotTextboxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
    void slotButtonClick(int index);
    void slotClientListDelete(void);
    void slotInfoPageBtnclick(int);
};

#endif // DHCP_SERVER_SETTINGS_H
