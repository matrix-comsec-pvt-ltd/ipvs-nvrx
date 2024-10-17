#ifndef STATICROUTING_H
#define STATICROUTING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/IpTextBox.h"
#include "Controls/TextBox.h"
#include "Controls/ControlButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/Bgtile.h"
#include "Controls/TableCell.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"
#include <QHostAddress>

#define ROUTE_LIST_MAX_ROW          5
#define ROUTE_LIST_MAX_COL          4

typedef enum
{
    PORT_NONE,
    PORT_LAN1,
    PORT_LAN2,
    PORT_BROADBAND,

    MAX_PORT
}PORT_TYPE_e;

typedef enum
{
    LIST_NETWORK_ADDR,
    LIST_SUBNET_MASK,
    LIST_ROUTE_PORT,

    MAX_LIST_STRING_INDEX
}LIST_STRING_INDEX_e;

class StaticRouting :public ConfigPageControl
{
    Q_OBJECT
private:
    PORT_TYPE_e currDfltPort;
    quint8 currListCount;

    DropDown        *dfltExitIntrfaceDropDownBox;
    Ipv4TextBox     *ipv4NetAddress;
    DropDown        *ipv4SubnetMaskDrpDwn;
    DropDown        *ipv4ExitIntrfaceDrpDwn;
    DropDown        *ipv6ExitIntrfaceDrpDwn;
    IpTextBox       *ipv6NetAddress;
    TextboxParam    *prefixLenParam;
    TextBox         *ipv6PrefixLen;

    ControlButton   *ipv4AddBtn, *ipv6AddBtn, *deleteBtn;
    ElementHeading  *ipv4StaticRouteHeading, *ipv6StaticRouteHeading, *routeListHeading;
    BgTile          *tableBg;
    TableCell       *tableCellHeadings[ROUTE_LIST_MAX_COL];
    TextLabel       *tableHeadingTextlabel[ROUTE_LIST_MAX_COL];

    OptionSelectButton *listOptionSelBtn[ROUTE_LIST_MAX_ROW];

    TableCell *feildTableCell[ROUTE_LIST_MAX_COL][ROUTE_LIST_MAX_ROW];
    TextLabel *feildTableCellTextlabel[MAX_LIST_STRING_INDEX][ROUTE_LIST_MAX_ROW];

    QStringList netAddrStrList;
    QStringList subnetMaskStrList;
    QStringList exitInterfaceStrList;

    QString currentDfltExitSpinBoxValue;
    QStringList referenceList;
    QStringList actualList;

public:
    explicit StaticRouting(QString devName,
                  QWidget *parent = 0,
                  DEV_TABLE_INFO_t* devTabInfo = NULL);
    ~StaticRouting();

    void createDefaultComponent();
    void getConfig ();
    void saveConfig ();
    void defaultConfig();
    void createPayload(REQ_MSG_ID_e requestType);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void setDfltExitSpinboxList(void);
    void showTableList(void);

public slots:
    void slotDfltExitSpinboxValueChanged(QString str,quint32 index);
    void slotAddDeleteButtonClick(int index);
};

#endif // STATICROUTING_H
