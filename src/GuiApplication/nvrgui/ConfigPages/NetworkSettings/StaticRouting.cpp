#include "StaticRouting.h"
#include "ValidationMessage.h"

#define FIRST_ELE_TOP_MARGIN            SCALE_HEIGHT(15)
#define DEFAULT_ROUTE_TABLE_INDEX       1
#define DEFAULT_ROUTE_TABLE_FIELD       1
#define STATIC_ROUTE_TABLE_MAX_INDEX    5
#define LEFT_MARGIN_FROM_CENTER         SCALE_WIDTH(50)

#define HEADING_STATIC_ROUTE_V4         "Static Route (IPv4)"
#define HEADING_STATIC_ROUTE_V6         "Static Route (IPv6)"
#define HEADING_STATIC_ROUTE_LIST       "Static Route List"
// List of control
typedef enum
{
    STATIC_ROUTE_DFLT_EXIT_INTERFACE,
    STATIC_ROUTE_IPV4_NETWORK_ADDR,
    STATIC_ROUTE_IPV4_SUBNET_MASK,
    STATIC_ROUTE_IPV4_EXIT_INTERFACE,
    STATIC_ROUTE_IPV4_ADD_BTN,
    STATIC_ROUTE_IPV6_NETWORK_ADDR,
    STATIC_ROUTE_IPV6_PREFIX_LEN,
    STATIC_ROUTE_IPV6_EXIT_INTERFACE,
    STATIC_ROUTE_IPV6_ADD_BTN,
    STATIC_ROUTE_LIST_1STCHECKBOX,
    STATIC_ROUTE_DELETE_BTN = STATIC_ROUTE_LIST_1STCHECKBOX + ROUTE_LIST_MAX_ROW,
    MAX_STATIC_ROUTE_ELEMETS
}STATIC_ROUTE_ELELIST_e;

typedef enum
{
    FIELD_DFLT_ROUTE =0,
    FIELD_NETWORK_ADDRESS,
    FIELD_SUBNET_MASK,
    FIELD_ROUTE_PORT,
    MAX_STATIC_ROUTE_FIELD = 3
}STATIC_ROUTE_FIELD_e;

const static QString eleLabels[MAX_STATIC_ROUTE_ELEMETS] =
{
    "Default Exit Interface",
    "Network Address",
    "Subnet Mask",
    "Exit Interface",
    "Add",
    "Network Address",
    "Prefix Length",
    "Exit Interface",
    "Add",
    "",
    "",
    "",
    "",
    "",
    "Delete"
};

const static QString tableHeading[ROUTE_LIST_MAX_COL] =
{
    "Network Address",
    "Subnet Mask/Prefix Length",
    "Exit Interface",
    "Select"
};

const static QStringList subNetMaskList = QStringList()
<<"None"                <<"255.255.255.255"
<<"255.255.255.254"     <<"255.255.255.252"
<<"255.255.255.248"     <<"255.255.255.240"
<<"255.255.255.224"     <<"255.255.255.192"
<<"255.255.255.128"     <<"255.255.255.0"
<<"255.255.254.0"       <<"255.255.252.0"
<<"255.255.248.0"       <<"255.255.240.0"
<<"255.255.224.0"       <<"255.255.192.0"
<<"255.255.128.0"       <<"255.255.0.0"
<<"255.254.0.0"         <<"255.252.0.0"
<<"255.248.0.0"         <<"255.240.0.0"
<<"255.224.0.0"         <<"255.192.0.0"
<<"255.128.0.0"         <<"255.0.0.0"
<<"254.0.0.0"           <<"252.0.0.0"
<<"248.0.0.0"           <<"240.0.0.0"
<<"224.0.0.0"           <<"192.0.0.0"
<<"128.0.0.0";

const static QStringList referenceList2 = QStringList()
        << "None" << "LAN 1" << "LAN 2" << "Broadband";

const static QStringList referenceList1 = QStringList()
        << "None" << "LAN 1" << "Broadband" ;

const static QStringList actualList2 = QStringList()
        <<"LAN 1" << "LAN 2" << "Broadband";

const static QStringList actualList1 = QStringList()
        <<"LAN 1" << "Broadband";

StaticRouting::StaticRouting(QString devName, QWidget *parent, DEV_TABLE_INFO_t* devTabInfo )
    :ConfigPageControl(devName, parent, MAX_STATIC_ROUTE_ELEMETS,devTabInfo ),
     currDfltPort(MAX_PORT), currListCount(0)
{
    netAddrStrList.reserve (ROUTE_LIST_MAX_ROW);
    subnetMaskStrList.reserve (ROUTE_LIST_MAX_ROW);
    exitInterfaceStrList.reserve (ROUTE_LIST_MAX_ROW);

    createDefaultComponent();
    StaticRouting::getConfig();
}

StaticRouting::~StaticRouting()
{
    disconnect (dfltExitIntrfaceDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDfltExitSpinboxValueChanged(QString,quint32)));
    disconnect(dfltExitIntrfaceDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete dfltExitIntrfaceDropDownBox;

    delete ipv4StaticRouteHeading;

    disconnect(ipv4NetAddress,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete ipv4NetAddress;

    disconnect(ipv4SubnetMaskDrpDwn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete ipv4SubnetMaskDrpDwn;

    disconnect(ipv4ExitIntrfaceDrpDwn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete ipv4ExitIntrfaceDrpDwn;

    disconnect (ipv4AddBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotAddDeleteButtonClick(int)));
    disconnect(ipv4AddBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete ipv4AddBtn;

    delete routeListHeading;
    delete tableBg;

    disconnect (deleteBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotAddDeleteButtonClick(int)));
    disconnect(deleteBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete deleteBtn;

    for(quint8 col = 0; col < ROUTE_LIST_MAX_COL; col++)
    {
        delete tableCellHeadings[col];
        delete tableHeadingTextlabel[col];
    }

    for(quint8 row = 0; row < ROUTE_LIST_MAX_ROW; row++)
    {
        for(quint8 col = 0; col < ROUTE_LIST_MAX_COL; col++)
        {
            delete feildTableCell[col][row];

            if(col < (ROUTE_LIST_MAX_COL-1))
            {
                delete feildTableCellTextlabel[col][row];
            }
        }

        disconnect(listOptionSelBtn[row],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete listOptionSelBtn[row];
    }

    DELETE_OBJ(ipv6StaticRouteHeading);

    if (IS_VALID_OBJ(ipv6NetAddress))
    {
        disconnect (ipv6NetAddress,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(ipv6NetAddress);
    }


    if (IS_VALID_OBJ(ipv6PrefixLen))
    {
        disconnect (ipv6PrefixLen,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(ipv6PrefixLen);
    }

    DELETE_OBJ(prefixLenParam);

    if (IS_VALID_OBJ(ipv6ExitIntrfaceDrpDwn))
    {
        disconnect(ipv6ExitIntrfaceDrpDwn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(ipv6ExitIntrfaceDrpDwn);
    }

    if (IS_VALID_OBJ(ipv6AddBtn))
    {
        disconnect(ipv6AddBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect (ipv6AddBtn,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotAddDeleteButtonClick(int)));

        DELETE_OBJ(ipv6AddBtn);
    }
}

void StaticRouting::createDefaultComponent ()
{
    if (devTableInfo->numOfLan == 1)
    {
        referenceList = referenceList1;
        actualList = actualList1;
    }
    else
    {
        referenceList = referenceList2;
        actualList = actualList2;
    }

    QMap<quint8, QString>  referenceMapList;

    for(quint8 index = 0; index <  referenceList.length (); index++)
    {
        referenceMapList.insert (index,referenceList.at (index));
    }

    dfltExitIntrfaceDropDownBox = new DropDown((this->width () - BGTILE_LARGE_SIZE_WIDTH)/2,
                                               FIRST_ELE_TOP_MARGIN,
                                               BGTILE_LARGE_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               STATIC_ROUTE_DFLT_EXIT_INTERFACE,
                                               DROPDOWNBOX_SIZE_200,
                                               eleLabels[STATIC_ROUTE_DFLT_EXIT_INTERFACE],
                                               referenceMapList,
                                               this, "", false,
                                               SCALE_WIDTH(10));
    m_elementList[STATIC_ROUTE_DFLT_EXIT_INTERFACE] = dfltExitIntrfaceDropDownBox;
    connect(dfltExitIntrfaceDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (dfltExitIntrfaceDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotDfltExitSpinboxValueChanged(QString,quint32)));

    ipv4StaticRouteHeading = new ElementHeading(dfltExitIntrfaceDropDownBox->x (),
                                                dfltExitIntrfaceDropDownBox->y () + dfltExitIntrfaceDropDownBox->height () + SCALE_HEIGHT(6),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                HEADING_STATIC_ROUTE_V4,
                                                TOP_LAYER, this, false,SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    ipv4NetAddress = new Ipv4TextBox(ipv4StaticRouteHeading->x (),
                                     ipv4StaticRouteHeading->y () + ipv4StaticRouteHeading->height (),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     STATIC_ROUTE_IPV4_NETWORK_ADDR,
                                     eleLabels[STATIC_ROUTE_IPV4_NETWORK_ADDR],
                                     this, MIDDLE_TABLE_LAYER);
    m_elementList[STATIC_ROUTE_IPV4_NETWORK_ADDR] = ipv4NetAddress;
    connect(ipv4NetAddress,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString>  subNetMaskMapList;

    for(quint8 index = 0; index <  subNetMaskList.length (); index++)
    {
        subNetMaskMapList.insert (index,subNetMaskList.at (index));
    }

    ipv4SubnetMaskDrpDwn = new DropDown(ipv4NetAddress->x (),
                                        ipv4NetAddress->y () + ipv4NetAddress->height (),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        STATIC_ROUTE_IPV4_SUBNET_MASK,
                                        DROPDOWNBOX_SIZE_200,
                                        eleLabels[STATIC_ROUTE_IPV4_SUBNET_MASK],
                                        subNetMaskMapList,
                                        this, "", true, 0,
                                        MIDDLE_TABLE_LAYER);
    m_elementList[STATIC_ROUTE_IPV4_SUBNET_MASK] = ipv4SubnetMaskDrpDwn;
    connect(ipv4SubnetMaskDrpDwn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv4ExitIntrfaceDrpDwn  = new DropDown(ipv4SubnetMaskDrpDwn->x (),
                                           ipv4SubnetMaskDrpDwn->y () + ipv4SubnetMaskDrpDwn->height (),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           STATIC_ROUTE_IPV4_EXIT_INTERFACE,
                                           DROPDOWNBOX_SIZE_200,
                                           eleLabels[STATIC_ROUTE_IPV4_EXIT_INTERFACE],
                                           referenceMapList,
                                           this, "", true, 0,
                                           MIDDLE_TABLE_LAYER);
    m_elementList[STATIC_ROUTE_IPV4_EXIT_INTERFACE] = ipv4ExitIntrfaceDrpDwn;
    connect(ipv4ExitIntrfaceDrpDwn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv4AddBtn = new ControlButton(ADD_BUTTON_INDEX,
                                   ipv4ExitIntrfaceDrpDwn->x (),
                                   ipv4ExitIntrfaceDrpDwn->y () + ipv4ExitIntrfaceDrpDwn->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this, BOTTOM_TABLE_LAYER,
                                   SCALE_WIDTH(380), eleLabels[STATIC_ROUTE_IPV4_ADD_BTN],
                                   true, STATIC_ROUTE_IPV4_ADD_BTN);
    m_elementList[STATIC_ROUTE_IPV4_ADD_BTN] = ipv4AddBtn;
    connect(ipv4AddBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (ipv4AddBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotAddDeleteButtonClick(int)));

    ipv6StaticRouteHeading = new ElementHeading(ipv4StaticRouteHeading->x () + ipv4StaticRouteHeading->width() + SCALE_WIDTH(6),
                                                ipv4StaticRouteHeading->y (),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                HEADING_STATIC_ROUTE_V6,
                                                TOP_LAYER, this, false,SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    ipv6NetAddress = new IpTextBox(ipv6StaticRouteHeading->x(),
                                   ipv6StaticRouteHeading->y() + ipv6StaticRouteHeading->height(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   STATIC_ROUTE_IPV6_NETWORK_ADDR,
                                   eleLabels[STATIC_ROUTE_IPV6_NETWORK_ADDR],
                                   IP_ADDR_TYPE_IPV6_ONLY,
                                   this, MIDDLE_TABLE_LAYER,
                                   true, 0, true, IP_FIELD_TYPE_STATIC_ROUTE_NW_ADDR,
                                   IP_TEXTBOX_ULTRALARGE, LEFT_MARGIN_FROM_CENTER);

    m_elementList[STATIC_ROUTE_IPV6_NETWORK_ADDR] = ipv6NetAddress;

    connect (ipv6NetAddress,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    prefixLenParam = new TextboxParam();
    prefixLenParam->labelStr = eleLabels[STATIC_ROUTE_IPV6_PREFIX_LEN];
    prefixLenParam->maxChar = 3;
    prefixLenParam->validation = QRegExp(QString("[0-9]"));
    prefixLenParam->isNumEntry = true;
    prefixLenParam->minNumValue = 1;
    prefixLenParam->maxNumValue = 128;

    ipv6PrefixLen = new TextBox(ipv6NetAddress->x(),
                                ipv6NetAddress->y() + ipv6NetAddress->height(),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                STATIC_ROUTE_IPV6_PREFIX_LEN,
                                TEXTBOX_ULTRALARGE,
                                this, prefixLenParam,
                                MIDDLE_TABLE_LAYER, true, false, false,
                                LEFT_MARGIN_FROM_CENTER);

    m_elementList[STATIC_ROUTE_IPV6_PREFIX_LEN] = ipv6PrefixLen;

    connect (ipv6PrefixLen,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    ipv6ExitIntrfaceDrpDwn  = new DropDown(ipv6PrefixLen->x (),
                                           ipv6PrefixLen->y () + ipv6PrefixLen->height (),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           STATIC_ROUTE_IPV6_EXIT_INTERFACE,
                                           DROPDOWNBOX_SIZE_200,
                                           eleLabels[STATIC_ROUTE_IPV4_EXIT_INTERFACE],
                                           referenceMapList,
                                           this, "", true, 0,
                                           MIDDLE_TABLE_LAYER, true, 8, false, false,
                                           5, LEFT_MARGIN_FROM_CENTER);

    m_elementList[STATIC_ROUTE_IPV6_EXIT_INTERFACE] = ipv6ExitIntrfaceDrpDwn;

    connect(ipv6ExitIntrfaceDrpDwn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ipv6AddBtn = new ControlButton(ADD_BUTTON_INDEX,
                                   ipv6ExitIntrfaceDrpDwn->x (),
                                   ipv6ExitIntrfaceDrpDwn->y () + ipv6ExitIntrfaceDrpDwn->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this, BOTTOM_TABLE_LAYER,
                                   SCALE_WIDTH(380), eleLabels[STATIC_ROUTE_IPV4_ADD_BTN],
                                   true, STATIC_ROUTE_IPV6_ADD_BTN);

    m_elementList[STATIC_ROUTE_IPV6_ADD_BTN] = ipv6AddBtn;

    connect(ipv6AddBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (ipv6AddBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotAddDeleteButtonClick(int)));

    routeListHeading = new ElementHeading(ipv4AddBtn->x (),
                                          ipv4AddBtn->y () + ipv4AddBtn->height () + SCALE_HEIGHT(6),
                                          BGTILE_LARGE_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          HEADING_STATIC_ROUTE_LIST,
                                          TOP_LAYER, this, false, SCALE_HEIGHT(25), NORMAL_FONT_SIZE, true);

    tableBg = new BgTile(routeListHeading->x (),
                         routeListHeading->y () + routeListHeading->height (),
                         BGTILE_LARGE_SIZE_WIDTH,
                         SCALE_HEIGHT(180), MIDDLE_LAYER, this);

    deleteBtn = new ControlButton(DELETE_BUTTON_INDEX,
                                  tableBg->x (),
                                  tableBg->y () + tableBg->height (),
                                  BGTILE_LARGE_SIZE_WIDTH,
                                  SCALE_HEIGHT(30),
                                  this, BOTTOM_LAYER,
                                  SCALE_WIDTH(880), eleLabels[STATIC_ROUTE_DELETE_BTN],
                                  true, STATIC_ROUTE_DELETE_BTN);
    m_elementList[STATIC_ROUTE_DELETE_BTN] = deleteBtn;
    connect(deleteBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (deleteBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotAddDeleteButtonClick(int)));

    for(quint8 col = 0; col < ROUTE_LIST_MAX_COL; col++)
    {
        tableCellHeadings[col] = new TableCell((col == 0) ? tableBg->x () + SCALE_WIDTH(34) : tableCellHeadings[col-1]->x() + tableCellHeadings[col-1]->width(),
                                               tableBg->y (),
                                               ((col < 2) ? TABELCELL_EXTRAMEDIUM_SIZE_WIDTH : ((col == 2) ? TABELCELL_MEDIUM_SIZE_WIDTH : TABELCELL_EXTRASMALL_SIZE_WIDTH)),
                                               TABELCELL_HEIGHT,this, true);

        tableHeadingTextlabel[col] = new TextLabel(tableCellHeadings[col]->x () + (tableCellHeadings[col]->width() / 2),
                                                   tableCellHeadings[col]->y () + (tableCellHeadings[col]->height() / 2),
                                                   NORMAL_FONT_SIZE, tableHeading[col],
                                                   this, HIGHLITED_FONT_COLOR,
                                                   NORMAL_FONT_FAMILY, ALIGN_CENTRE_X_CENTER_Y);
    }

    for(quint8 row = 0; row < ROUTE_LIST_MAX_ROW; row++)
    {
        for(quint8 col = 0; col < ROUTE_LIST_MAX_COL; col++)
        {
            feildTableCell[col][row] = new TableCell(tableCellHeadings[col]->x (),
                                                     (tableCellHeadings[col]->y () + tableCellHeadings[col]->height () +
                                                     (row * TABELCELL_HEIGHT)),
                                                     tableCellHeadings[col]->width() - 1,
                                                     TABELCELL_HEIGHT,this);

            if(col < (ROUTE_LIST_MAX_COL-1))
            {
                feildTableCellTextlabel[col][row] = new TextLabel(feildTableCell[col][row]->x () + SCALE_WIDTH(10),
                                                                  feildTableCell[col][row]->y () + feildTableCell[col][row]->height ()/2,
                                                                  NORMAL_FONT_SIZE,
                                                                  "", this, NORMAL_FONT_COLOR,
                                                                  NORMAL_FONT_FAMILY,
                                                                  ALIGN_START_X_CENTRE_Y, 0, false, 280);
            }
        }

        listOptionSelBtn[row] = new OptionSelectButton(feildTableCell[3][row]->x () + SCALE_WIDTH(38),
                                                       feildTableCell[3][row]->y (),
                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                       TABELCELL_HEIGHT, CHECK_BUTTON_INDEX,
                                                       this, NO_LAYER, "",
                                                       "", -1,
                                                       (STATIC_ROUTE_LIST_1STCHECKBOX + row), true);
        m_elementList[STATIC_ROUTE_LIST_1STCHECKBOX + row] = listOptionSelBtn[row];
        connect(listOptionSelBtn[row],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        listOptionSelBtn[row]->setIsEnabled (false);
        listOptionSelBtn[row]->setVisible (false);
    }
}

void StaticRouting::getConfig ()
{
    ipv4NetAddress->setIpaddress("");
    ipv4SubnetMaskDrpDwn->setIndexofCurrElement(0);
    ipv4ExitIntrfaceDrpDwn->setIndexofCurrElement(0);
    ipv6NetAddress->setIpaddress ("");
    ipv6PrefixLen->setInputText(0);
    ipv6ExitIntrfaceDrpDwn->setIndexofCurrElement(0);
    createPayload (MSG_GET_CFG);
}

void StaticRouting::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void StaticRouting::saveConfig ()
{
    currDfltPort = (PORT_TYPE_e)referenceList2.indexOf(dfltExitIntrfaceDropDownBox->getCurrValue());

    payloadLib->setCnfgArrayAtIndex(FIELD_DFLT_ROUTE, currDfltPort);

    for(quint8 row = 0; row < ROUTE_LIST_MAX_ROW; row++)
    {
        if(row < netAddrStrList.length ())
        {
            payloadLib->setCnfgArrayAtIndex(FIELD_NETWORK_ADDRESS + (MAX_LIST_STRING_INDEX * row), netAddrStrList.at (row));

            if(QHostAddress(netAddrStrList.at(row)).protocol() == QAbstractSocket::IPv4Protocol)
            {
                payloadLib->setCnfgArrayAtIndex(FIELD_SUBNET_MASK + (MAX_LIST_STRING_INDEX * row), subNetMaskList.indexOf(subnetMaskStrList.at (row)));
            }
            else
            {
                payloadLib->setCnfgArrayAtIndex(FIELD_SUBNET_MASK + (MAX_LIST_STRING_INDEX * row), subnetMaskStrList.at(row));
            }
            payloadLib->setCnfgArrayAtIndex(FIELD_ROUTE_PORT + (MAX_LIST_STRING_INDEX * row), referenceList2.indexOf(exitInterfaceStrList.at (row)));
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex(FIELD_NETWORK_ADDRESS + (MAX_LIST_STRING_INDEX * row), "");
            payloadLib->setCnfgArrayAtIndex(FIELD_SUBNET_MASK + (MAX_LIST_STRING_INDEX * row), 0);
            payloadLib->setCnfgArrayAtIndex(FIELD_ROUTE_PORT + (MAX_LIST_STRING_INDEX * row), 0);
        }
    }
    createPayload(MSG_SET_CFG);
}

void StaticRouting::createPayload (REQ_MSG_ID_e requestType)
{
    QString payloadString1 = payloadLib->createDevCnfgPayload(requestType,
                                                              DFT_ROUTING_TABLE_INDEX,
                                                              DEFAULT_ROUTE_TABLE_INDEX,
                                                              DEFAULT_ROUTE_TABLE_INDEX,
                                                              DEFAULT_ROUTE_TABLE_FIELD,
                                                              DEFAULT_ROUTE_TABLE_FIELD,
                                                              DEFAULT_ROUTE_TABLE_FIELD);

    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             STATIC_ROUTING_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             STATIC_ROUTE_TABLE_MAX_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_STATIC_ROUTE_FIELD,
                                                             (MAX_STATIC_ROUTE_FIELD * STATIC_ROUTE_TABLE_MAX_INDEX),
                                                             payloadString1,
                                                             DEFAULT_ROUTE_TABLE_FIELD);
    DevCommParam* param = new DevCommParam();

    param->msgType = requestType;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void StaticRouting::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    processBar->unloadProcessBar();
    if(deviceName != currDevName)
    {
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
                    if ((payloadLib->getcnfgTableIndex(0) != DFT_ROUTING_TABLE_INDEX) || (payloadLib->getcnfgTableIndex(1) != STATIC_ROUTING_TABLE_INDEX))
                    {
                        break;
                    }

                    QString ipAddrStr;
                    QMap<quint8, QString> defaultRoutePort;

                    currDfltPort = (PORT_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_DFLT_ROUTE).toUInt());

                    defaultRoutePort.clear();
                    defaultRoutePort.insert(0, referenceList2.at(currDfltPort));

                    dfltExitIntrfaceDropDownBox->setNewList(defaultRoutePort);

                    netAddrStrList.clear();
                    subnetMaskStrList.clear();
                    exitInterfaceStrList.clear();
                    for(quint8 row = 0 ; row < ROUTE_LIST_MAX_ROW; row++)
                    {
                        ipAddrStr = payloadLib->getCnfgArrayAtIndex(FIELD_NETWORK_ADDRESS + (MAX_LIST_STRING_INDEX * row)).toString();
                        if (ipAddrStr == "")
                        {
                            continue;
                        }

                        netAddrStrList.append(ipAddrStr);
                        QHostAddress ipAddress(ipAddrStr);
                        if(ipAddress.protocol() == QAbstractSocket::IPv4Protocol)
                        {
                            subnetMaskStrList.append(subNetMaskList.at(payloadLib->getCnfgArrayAtIndex(FIELD_SUBNET_MASK + (MAX_LIST_STRING_INDEX * row)).toUInt()));
                        }
                        else
                        {
                            subnetMaskStrList.append(payloadLib->getCnfgArrayAtIndex(FIELD_SUBNET_MASK + (MAX_LIST_STRING_INDEX * row)).toString());
                        }

                        exitInterfaceStrList.append(referenceList2.at(payloadLib->getCnfgArrayAtIndex(FIELD_ROUTE_PORT + (MAX_LIST_STRING_INDEX * row)).toUInt()));
                        currListCount++;
                    }

                    showTableList();
                    setDfltExitSpinboxList();
                    slotDfltExitSpinboxValueChanged(dfltExitIntrfaceDropDownBox->getCurrValue(), 0);
                }
                break;

                case MSG_SET_CFG:
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                    getConfig ();
                }
                break;

                case MSG_DEF_CFG:
                {
                    getConfig();
                }
                break;

                default:
                {
                }
                break;
            }
        }
        break;

        default:
        {
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            if(param->deviceStatus != CMD_SERVER_NOT_RESPONDING)
            {
                getConfig();
            }
        }
        break;
    }
}

void StaticRouting::showTableList(void)
{
    if(netAddrStrList.length () > 0)
    {
        for(quint8 index = 0; index < netAddrStrList.length (); index++)
        {
            feildTableCellTextlabel[LIST_NETWORK_ADDR][index]->changeText (netAddrStrList.at (index));
            feildTableCellTextlabel[LIST_NETWORK_ADDR][index]->update ();
            feildTableCellTextlabel[LIST_SUBNET_MASK][index]->changeText (subnetMaskStrList.at (index));
            feildTableCellTextlabel[LIST_SUBNET_MASK][index]->update ();
            feildTableCellTextlabel[LIST_ROUTE_PORT][index]->changeText (exitInterfaceStrList.at (index));
            feildTableCellTextlabel[LIST_ROUTE_PORT][index]->update ();
            listOptionSelBtn[index]->setVisible (true);
            listOptionSelBtn[index]->setIsEnabled (true);
            listOptionSelBtn[index]->changeState (OFF_STATE);
        }

        deleteBtn->setIsEnabled (true);

        if(netAddrStrList.length () <= (ROUTE_LIST_MAX_ROW - 1))
        {
            ipv4AddBtn->setIsEnabled (true);
            ipv6AddBtn->setIsEnabled (true);
        }
        else
        {
            if(ipv4AddBtn->hasFocus ())
            {
                m_currentElement = STATIC_ROUTE_IPV4_NETWORK_ADDR;
                m_elementList[m_currentElement]->forceActiveFocus ();
            }
            else if (ipv6AddBtn->hasFocus ())
            {
                m_currentElement = STATIC_ROUTE_IPV6_NETWORK_ADDR;
                m_elementList[m_currentElement]->forceActiveFocus ();
            }

            ipv4AddBtn->setIsEnabled (false);
            ipv6AddBtn->setIsEnabled (false);
        }
    }
    else
    {
        ipv4AddBtn->setIsEnabled (true);
        ipv6AddBtn->setIsEnabled (true);
        if(deleteBtn->hasFocus ())
        {
            m_currentElement = STATIC_ROUTE_IPV4_NETWORK_ADDR;
            m_elementList[m_currentElement]->forceActiveFocus ();
        }
        deleteBtn->setIsEnabled (false);
    }

    for(quint8 row = netAddrStrList.length () ; row < ROUTE_LIST_MAX_ROW; row++)
    {
        feildTableCellTextlabel[LIST_NETWORK_ADDR][row]->changeText ("");
        feildTableCellTextlabel[LIST_SUBNET_MASK][row]->changeText ("");
        feildTableCellTextlabel[LIST_ROUTE_PORT][row]->changeText ("");
        listOptionSelBtn[row]->setIsEnabled (false);
        listOptionSelBtn[row]->setVisible (false);
        listOptionSelBtn[row]->changeState (OFF_STATE);
    }
}

void StaticRouting::setDfltExitSpinboxList(void)
{
    quint8 maxPort = MAX_PORT;
    QMap<quint8, QString> newList;
    quint8 insertIndex = 0;
    newList.clear ();

    if(netAddrStrList.length () > 0)
    {
        if(devTableInfo->numOfLan == 1)
        {
            maxPort = PORT_BROADBAND;
        }
        else if (devTableInfo->numOfLan == 2)
        {
            maxPort = MAX_PORT;
        }

        for(quint8 ethCnt = 1; ethCnt < maxPort; ethCnt++)
        {
            bool flagSet = false;
            for(quint8 cnt = 0; cnt < exitInterfaceStrList.length(); cnt++)
            {
                if((referenceList.at(ethCnt) == exitInterfaceStrList.at(cnt)))
                {
                    flagSet = true;
                    break;
                }
            }

            if (flagSet == false)
            {
                newList.insert (insertIndex++, referenceList.at(ethCnt));
            }
        }
    }
    else
    {
        newList.clear ();
        for(quint8 index = 0; index < actualList.length (); index++)
        {
            newList.insert (index,actualList.at(index));
        }
    }

    currentDfltExitSpinBoxValue = dfltExitIntrfaceDropDownBox->getCurrValue();

    quint8 tempIndex = newList.key(currentDfltExitSpinBoxValue);

    dfltExitIntrfaceDropDownBox->setNewList (newList,(tempIndex > 0) ? tempIndex : 0);
}

void StaticRouting::slotDfltExitSpinboxValueChanged (QString str, quint32)
{
    ipv4NetAddress->setIpaddress ("");
    ipv4SubnetMaskDrpDwn->setIndexofCurrElement (0);
    ipv4ExitIntrfaceDrpDwn->setIndexofCurrElement (0);
    ipv6NetAddress->setIpaddress ("");
    ipv6PrefixLen->setInputText(0);
    ipv6ExitIntrfaceDrpDwn->setIndexofCurrElement(0);

    QMap<quint8, QString>  newList;

    if (devTableInfo->numOfLan == 2)
    {
        if(str == referenceList.at (PORT_LAN1))
        {
            currDfltPort = PORT_LAN1;
            newList.insert (0,referenceList.at (PORT_NONE));
            newList.insert (1,referenceList.at (PORT_LAN2));
            newList.insert (2,referenceList.at (PORT_BROADBAND));
            ipv4ExitIntrfaceDrpDwn->setNewList (newList);
            ipv6ExitIntrfaceDrpDwn->setNewList (newList);
        }
        else if (str == referenceList.at (PORT_BROADBAND))
        {
            currDfltPort = PORT_BROADBAND;
            newList.insert (0,referenceList.at (PORT_NONE));
            newList.insert (1,referenceList.at (PORT_LAN1));
            newList.insert (2,referenceList.at (PORT_LAN2));
            ipv4ExitIntrfaceDrpDwn->setNewList (newList);
            ipv6ExitIntrfaceDrpDwn->setNewList (newList);
        }
        else if ( str == referenceList.at (PORT_LAN2))
        {
            currDfltPort = PORT_LAN2;
            newList.insert (0,referenceList.at (PORT_NONE));
            newList.insert (1,referenceList.at (PORT_LAN1));
            newList.insert (2,referenceList.at (PORT_BROADBAND));
            ipv4ExitIntrfaceDrpDwn->setNewList (newList);
            ipv6ExitIntrfaceDrpDwn->setNewList (newList);
        }
    }
    else
    {
        //This else part is for the devices in which only one LAN available
        //Here enum PORT_TYPE_e is taken as a reference but consider only index of the enum
        //like when LAN 2 index is 2 when LAN 1 and LAN 2 available but
        //is for broadband when only one LAN is available
        if(str == referenceList.at (PORT_LAN1))
        {
            currDfltPort = PORT_LAN1;
            newList.insert (0,referenceList.at (PORT_NONE));
            newList.insert (1,referenceList.at (PORT_LAN2));
            ipv4ExitIntrfaceDrpDwn->setNewList (newList);
            ipv6ExitIntrfaceDrpDwn->setNewList (newList);
        }
        else if (str == referenceList.at (PORT_LAN2))
        {
            currDfltPort = PORT_LAN2;
            newList.insert (0,referenceList.at (PORT_NONE));
            newList.insert (1,referenceList.at (PORT_LAN1));
            ipv4ExitIntrfaceDrpDwn->setNewList (newList);
            ipv6ExitIntrfaceDrpDwn->setNewList (newList);
        }
    }
}

void StaticRouting::slotAddDeleteButtonClick(int index)
{
    QString tempStr;
    QMap<quint8, QString> newList;
    quint8 insertIndex = 0;
    QStringList tempList[MAX_LIST_STRING_INDEX];
    qint8 tempListIndex;
    quint8 maxPort = MAX_PORT;

    currentDfltExitSpinBoxValue = dfltExitIntrfaceDropDownBox->getCurrValue();

    if (index == STATIC_ROUTE_DELETE_BTN)
    {
        for(tempListIndex = 0; tempListIndex < netAddrStrList.length (); tempListIndex++)
        {
            if(listOptionSelBtn[tempListIndex]->getCurrentState () == OFF_STATE)
            {
                tempList[LIST_NETWORK_ADDR].append (netAddrStrList.at (tempListIndex));
                tempList[LIST_SUBNET_MASK].append (subnetMaskStrList.at (tempListIndex));
                tempList[LIST_ROUTE_PORT].append (exitInterfaceStrList.at (tempListIndex));
            }
            else
            {
                currListCount--;
            }
        }

        netAddrStrList = tempList[LIST_NETWORK_ADDR];
        subnetMaskStrList = tempList[LIST_SUBNET_MASK];
        exitInterfaceStrList = tempList[LIST_ROUTE_PORT];
        if(devTableInfo->numOfLan == 1)
        {
            maxPort = PORT_BROADBAND;
        }
        else if (devTableInfo->numOfLan == 2)
        {
            maxPort = MAX_PORT;
        }

        newList.clear();
        insertIndex = 0;

        for(quint8 ethCnt = 1; ethCnt < maxPort ; ethCnt++)
        {
            bool flagSet= false;
            for(quint8 cnt = 0; cnt < exitInterfaceStrList.length(); cnt ++)
            {
                if((referenceList.at (ethCnt) == exitInterfaceStrList.at(cnt)))
                {
                    flagSet = true;
                    break;
                }
            }

            if (flagSet == false)
            {
                newList.insert (insertIndex++,referenceList.at (ethCnt));
            }

        }

        currentDfltExitSpinBoxValue = dfltExitIntrfaceDropDownBox->getCurrValue();

        quint8 tempIndex = newList.key(currentDfltExitSpinBoxValue);

        dfltExitIntrfaceDropDownBox->setNewList (newList,(tempIndex > 0) ? tempIndex : 0);
    }
    else
    {
        if (index == STATIC_ROUTE_IPV4_ADD_BTN)
        {
            ipv4NetAddress->getIpaddress (tempStr);
            if (tempStr == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STATIC_ROUT_ENT_NW_ADDR));
                return;
            }
            else if ( ipv4SubnetMaskDrpDwn->getIndexofCurrElement () == 0)
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STATIC_ROUT_SELECT_VALID_SUBNET));
                return;
            }
            else if ( ipv4ExitIntrfaceDrpDwn->getIndexofCurrElement () == 0 )
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STATIC_ROUT_SELECT_VALID_INTERFACE));
                return;
            }

            subnetMaskStrList.append (ipv4SubnetMaskDrpDwn->getCurrValue());
            exitInterfaceStrList.append (ipv4ExitIntrfaceDrpDwn->getCurrValue());
        }
        else if (index == STATIC_ROUTE_IPV6_ADD_BTN)
        {
            ipv6NetAddress->getIpaddress (tempStr);
            if (tempStr == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STATIC_ROUT_ENT_NW_ADDR));
                return;
            }
            else if (ipv6PrefixLen->getInputText() == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PREFIX_LEN));
                return;
            }
            else if (ipv6ExitIntrfaceDrpDwn->getIndexofCurrElement () == 0)
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STATIC_ROUT_SELECT_VALID_INTERFACE));
                return;
            }

            subnetMaskStrList.append (ipv6PrefixLen->getInputText());
            exitInterfaceStrList.append (ipv6ExitIntrfaceDrpDwn->getCurrValue());
        }

        netAddrStrList.append (tempStr);

        if (devTableInfo->numOfLan == 1)
        {
            maxPort = PORT_BROADBAND;
        }
        else if (devTableInfo->numOfLan == 2)
        {
            maxPort = MAX_PORT;
        }

        newList.clear ();
        insertIndex = 0;
        for(quint8 ethCnt = 1; ethCnt < maxPort; ethCnt++)
        {
            bool flagSet = false;
            for(quint8 cnt = 0; cnt < exitInterfaceStrList.length(); cnt++)
            {
                if((referenceList.at(ethCnt) == exitInterfaceStrList.at(cnt)))
                {
                    flagSet = true;
                    break;
                }
            }

            if (flagSet == false)
            {
                newList.insert (insertIndex++, referenceList.at(ethCnt));
            }
        }

        currentDfltExitSpinBoxValue = dfltExitIntrfaceDropDownBox->getCurrValue();
        quint8 tempIndex = newList.key(currentDfltExitSpinBoxValue);
        dfltExitIntrfaceDropDownBox->setNewList (newList,(tempIndex > 0) ? tempIndex : 0);
        currListCount++;

        if (currListCount >= ROUTE_LIST_MAX_ROW)
        {
            ipv4AddBtn->setIsEnabled (false);
            ipv6AddBtn->setIsEnabled (false);
        }

        if (index == STATIC_ROUTE_IPV4_ADD_BTN)
        {
            ipv4NetAddress->setIpaddress ("");
            ipv4SubnetMaskDrpDwn->setIndexofCurrElement (0);
            ipv4ExitIntrfaceDrpDwn->setIndexofCurrElement (0);
        }
        else
        {
            ipv6NetAddress->setIpaddress ("");
            ipv6PrefixLen->setInputText("");
            ipv6ExitIntrfaceDrpDwn->setIndexofCurrElement (0);
        }
    }

    showTableList();
}
