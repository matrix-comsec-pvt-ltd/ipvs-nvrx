#include "WizardStatus.h"
#include "ValidationMessage.h"

#define WIZARD_BG_TILE_HEIGHT               SCALE_HEIGHT(50)
#define WIZARD_BG_TILE_WIDTH                SCALE_WIDTH(945)
#define WIZARD_BG_TILE_HEIGHT_NW_PARAM      WIZARD_BG_TILE_HEIGHT + SCALE_HEIGHT(20)
#define DATE_TIME_STRING                    "Date Time"
#define CONNECTED_CAMERA                    "Connected Cameras"
#define INTERNET_CONNECTIVITY               "Internet"
#define ADVANCE_DETAIL_WIDTH                SCALE_WIDTH(1126)
#define ADVANCE_DETAIL_HEIGHT               SCALE_HEIGHT(790)

#define INTERNET_CONNECTED                  ":/Images_Nvrx/StatusIcon/Connected.png"
#define INTERNET_DISCONNECTED               ":/Images_Nvrx/StatusIcon/Disconnected.png"
#define INTERFACE_TABLECELL_WIDTH           SCALE_WIDTH(424)
#define INTERFACE_TABLECELL_WIDTH_NW_PARAM  SCALE_WIDTH(250)
const QString lanstateStr[2] =
{
    "Up",
    "Down"
};

const QString interfaceStr[5] =
{
    "Interface",
    "Status",
    "IP Address",
    "LAN 1",
    "LAN 2",
};

const QString monthsName[] = {"Jan", "Feb",
                              "Mar", "Apr",
                              "May", "Jun",
                              "Jul", "Aug",
                              "Sep", "Oct",
                              "Nov", "Dec"};

WizardStatus::WizardStatus(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId)
    : WizardCommon(parent, pageId)
{
    for(quint8 index = 0; index < MAX_PARAM_STS; index++)
    {
        for(quint8 camIndex = 0; camIndex < MAX_CAMERAS; camIndex++)
        {
            hlthRsltParam[index][camIndex] = 0;
        }
    }
    noOfConnectedCameras = 0;
    currentDevName = devName;
    m_internetConn = 0;

    createDefaultElements(subHeadStr);
    infoPage = new InfoPage(0,
                            0,
                            ADVANCE_DETAIL_WIDTH,
                            ADVANCE_DETAIL_HEIGHT,
                            INFO_ADVANCE_DETAILS,
                            this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTable);
    m_updateTimer = new QTimer();
    connect(m_updateTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotUpdateDateTime()));

    m_updateTimer->setInterval(999);
    QDateTime localDateTime = QDateTime::currentDateTime();
    updateDateTime(localDateTime);
    getDateAndTime(devName);
}

bool WizardStatus::getAdvanceDetailFrmDev(QString devName, SET_COMMAND_e command)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = command;
    if(applController->processActivity(devName, DEVICE_COMM, param) == false)
    {
        WizardCommon:: InfoPageImage();
        infoPage->raise();
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
    }
    return true;
}

void WizardStatus::slotUpdateDateTime()
{
    QDateTime newDateTime = QDateTime(QDate(m_currentYear, m_currentMonth, m_currentDate),
                                      QTime(m_currentHour, m_currentMinute, m_currentSecond),
                                      Qt::UTC);
    updateDateTime(newDateTime.addSecs(1));
    m_dateTime->changeText(changeToStandardDateTime());
    m_dateTime->update();
}

void WizardStatus::slotInfoPageCnfgBtnClick(int)
{
    WizardCommon::UnloadInfoPageImage();
}

void WizardStatus::updateDateTime(QDateTime dateTime)
{
    m_currentDate = dateTime.date().day();
    m_currentMonth = dateTime.date().month();
    m_currentYear = dateTime.date().year();
    m_currentHour = dateTime.time().hour();
    m_currentMinute = dateTime.time().minute();
    m_currentSecond = dateTime.time().second();
}

void WizardStatus::createDefaultElements(QString subHeadStr)
{
    applController = ApplController::getInstance ();
    INIT_OBJ(payloadLib);
    payloadLib = new PayloadLib();

    INIT_OBJ(m_statusPageHeading);
    for(quint8 index = WIZ_DATE_TIME; index < WIZ_MAX_STATUS_PAGE_FIELD; index++)
    {
        INIT_OBJ(statusPage[index]);
        INIT_OBJ(statusPageLabel[index]);
    }
    INIT_OBJ(m_dateTime);
    INIT_OBJ(m_connectedCam);
    INIT_OBJ(m_internetImage);
    INIT_OBJ(bgTile);

    for(quint8 index = WIZ_INTERFACE; index < WIZ_MAX_INTERFACE_HEADER_INDEX; index++)
    {
        INIT_OBJ(interfaceHeader[index]);
        INIT_OBJ(interfaceHeaderStr[index]);
    }

    for(quint8 index = WIZ_LAN1_STATUS; index < WIZ_MAX_LAN_STATUS_INDEX; index++)
    {
        INIT_OBJ(interfaceFieldStatus[index]);
        INIT_OBJ(interfaceFieldIpAddress[index]);
    }

    m_statusPageHeading = new TextLabel(SCALE_WIDTH(510),
                                        SCALE_HEIGHT(23),
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

    statusPage[WIZ_DATE_TIME] = new BgTile(SCALE_WIDTH(70),
                                           SCALE_HEIGHT(50),
                                           WIZARD_BG_TILE_WIDTH,
                                           WIZARD_BG_TILE_HEIGHT,
                                           COMMON_LAYER,this);

    statusPageLabel[WIZ_DATE_TIME] = new TextLabel(SCALE_WIDTH(450),
                                                   SCALE_HEIGHT(65),
                                                   NORMAL_FONT_SIZE,
                                                   DATE_TIME_STRING,
                                                   this);

    m_dateTime = new TextLabel(SCALE_WIDTH(718),
                               SCALE_HEIGHT(75),
                               NORMAL_FONT_SIZE,
                               "",
                               this,
                               NORMAL_FONT_COLOR,
                               NORMAL_FONT_FAMILY,
                               ALIGN_END_X_CENTRE_Y);

    statusPage[WIZ_CONNECTED_CAMERA] = new BgTile(statusPage[WIZ_DATE_TIME]->x(),
                                                  statusPage[WIZ_DATE_TIME]->y() + WIZARD_BG_TILE_HEIGHT,
                                                  WIZARD_BG_TILE_WIDTH,
                                                  WIZARD_BG_TILE_HEIGHT,
                                                  COMMON_LAYER,this);

    statusPageLabel[WIZ_CONNECTED_CAMERA] = new TextLabel(SCALE_WIDTH(360),
                                                          SCALE_HEIGHT(115) ,
                                                          NORMAL_FONT_SIZE,
                                                          CONNECTED_CAMERA,
                                                          this);

    m_connectedCam = new TextLabel(SCALE_WIDTH(550),
                                   SCALE_HEIGHT(115),
                                   NORMAL_FONT_SIZE,
                                   "",
                                   this);

    statusPage[WIZ_INTERNET] = new BgTile(SCALE_WIDTH(70),
                                          SCALE_HEIGHT(50) + (WIZARD_BG_TILE_HEIGHT * 2),
                                          WIZARD_BG_TILE_WIDTH,
                                          WIZARD_BG_TILE_HEIGHT,
                                          COMMON_LAYER,this);

    statusPageLabel[WIZ_INTERNET] = new TextLabel(SCALE_WIDTH(470),
                                                  SCALE_HEIGHT(165),
                                                  NORMAL_FONT_SIZE,
                                                  INTERNET_CONNECTIVITY,
                                                  this);

    m_internetImage = new Image(SCALE_WIDTH(560),
                                SCALE_HEIGHT(175),
                                "",
                                this, CENTER_X_CENTER_Y,
                                0, false, true);

    bgTile = new BgTile(statusPage[WIZ_INTERNET]->x(),
                        statusPage[WIZ_INTERNET]->y () + SCALE_HEIGHT(10) + WIZARD_BG_TILE_HEIGHT,
                        WIZARD_BG_TILE_WIDTH,
                        SCALE_HEIGHT(206),
                        COMMON_LAYER,
                        this);

    /* Creating table cell for titles : Interface, Status, IP Address */
    for(quint8 index = WIZ_INTERFACE; index < WIZ_LAN1; index++)
    {
        if (index < WIZ_IP_ADDRESS)
        {
            interfaceHeader[index] = new TableCell((bgTile->x () + SCALE_WIDTH(10) + (INTERFACE_TABLECELL_WIDTH_NW_PARAM * index)),
                                                 (bgTile->y () + SCALE_HEIGHT(10)),
                                                 INTERFACE_TABLECELL_WIDTH_NW_PARAM, WIZARD_BG_TILE_HEIGHT, this, true);
        }
        else
        {
            interfaceHeader[index] = new TableCell((bgTile->x () + SCALE_WIDTH(10) + (INTERFACE_TABLECELL_WIDTH_NW_PARAM * index)),
                                                 (bgTile->y () + SCALE_HEIGHT(10)),
                                                 INTERFACE_TABLECELL_WIDTH, WIZARD_BG_TILE_HEIGHT, this, true);
        }

    }

    /* Creating table cell for titles : LAN 1, LAN 2 */
    interfaceHeader[WIZ_LAN1] = new TableCell((bgTile->x () + SCALE_WIDTH(10)),
                                       interfaceHeader[WIZ_INTERFACE]->y() + interfaceHeader[WIZ_INTERFACE]->height(),
                                       INTERFACE_TABLECELL_WIDTH_NW_PARAM, WIZARD_BG_TILE_HEIGHT_NW_PARAM, this, true);

    interfaceHeader[WIZ_LAN2] = new TableCell((bgTile->x () + SCALE_WIDTH(10)),
                                       interfaceHeader[WIZ_LAN1]->y() + interfaceHeader[WIZ_LAN1]->height(),
                                       INTERFACE_TABLECELL_WIDTH_NW_PARAM, WIZARD_BG_TILE_HEIGHT_NW_PARAM, this, true);

    /* Creating text labels : Interface, Status, IP Address, LAN 1, LAN 2 */
    for(quint8 index = WIZ_INTERFACE; index < WIZ_MAX_INTERFACE_HEADER_INDEX; index++)
    {
        interfaceHeaderStr[index] = new TextLabel((interfaceHeader[index]->x() + SCALE_WIDTH(10)),
                                                (interfaceHeader[index]->y() + (interfaceHeader[index]->height()/2)),
                                                NORMAL_FONT_SIZE, interfaceStr[index],
                                                this, HIGHLITED_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, false,
                                                (INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(8)));
    }

    /* Creating table cell for Status & IP Address for LAN 1 & LAN 2 */
    for(quint8 row = WIZ_LAN1_STATUS; row < WIZ_MAX_LAN_STATUS_INDEX; row++)
    {
        interfaceFieldStatus[row] = new TableCell((bgTile->x () + SCALE_WIDTH(10) + INTERFACE_TABLECELL_WIDTH_NW_PARAM),
                                               (interfaceHeader[WIZ_INTERFACE]->y() + interfaceHeader[WIZ_INTERFACE]->height()  + ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) * row)),
                                                INTERFACE_TABLECELL_WIDTH_NW_PARAM,
                                                WIZARD_BG_TILE_HEIGHT_NW_PARAM,
                                                this);
    }

    for(quint8 row = WIZ_LAN1_IPV4_ADDR; row < WIZ_MAX_INTERFACE_TABLE_ROW; row++)
    {
        interfaceFieldIpAddress[row] = new TableCell((bgTile->x () + SCALE_WIDTH(10) + INTERFACE_TABLECELL_WIDTH_NW_PARAM * 2),
                                                 (interfaceHeader[WIZ_INTERFACE]->y() + interfaceHeader[WIZ_INTERFACE]->height()  + ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) * row)),
                                                 INTERFACE_TABLECELL_WIDTH,
                                                 WIZARD_BG_TILE_HEIGHT_NW_PARAM,
                                                 this);
    }


    /* Creating text labels for Status & IP Address for LAN 1 & LAN 2 */
    for(quint8 row = WIZ_LAN1_STATUS; row < WIZ_MAX_LAN_STATUS_INDEX; row++)
    {
        lanStatusLabel[row] = new TextLabel(bgTile->x () + SCALE_WIDTH(20) + INTERFACE_TABLECELL_WIDTH_NW_PARAM,
                                           (interfaceHeader[WIZ_INTERFACE]->y() + interfaceHeader[WIZ_INTERFACE]->height() + ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) * row) +
                                           ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) / 2)),
                                           NORMAL_FONT_SIZE, "", this,
                                           NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y, 0, false,
                                           (INTERFACE_TABLECELL_WIDTH_NW_PARAM - SCALE_WIDTH(8)));

        ipv4AddressLabel[row] = new TextLabel(bgTile->x () + SCALE_WIDTH(20) + (INTERFACE_TABLECELL_WIDTH_NW_PARAM * 2),
                                             (interfaceHeader[WIZ_INTERFACE]->y() + interfaceHeader[WIZ_INTERFACE]->height() + ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) * row) +
                                             ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) / 4)),
                                             NORMAL_FONT_SIZE, "", this,
                                             NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y, 0, false,
                                             (INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(8)));

        ipv6AddressLabel[row] = new TextLabel(bgTile->x () + SCALE_WIDTH(20) + (INTERFACE_TABLECELL_WIDTH_NW_PARAM * 2),
                                             (interfaceHeader[WIZ_INTERFACE]->y() + interfaceHeader[WIZ_INTERFACE]->height() + ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) * row) +
                                             ((WIZARD_BG_TILE_HEIGHT_NW_PARAM) / 4) * 3),
                                             NORMAL_FONT_SIZE, "", this,
                                             NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y, 0, false,
                                             (INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(8)));
    }

    this->show();
}

void WizardStatus::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName != currentDevName) || (param->msgType != MSG_SET_CMD))
    {
        return;
    }

    payloadLib->parseDevCmdReply(true, param->payload);
    if(param->cmdType == ADVANCE_STS)
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            lanStatusStr[WIZ_LAN1_STATUS] = lanstateStr[payloadLib->getCnfgArrayAtIndex(0).toInt ()];
            ipv4AddrStr[WIZ_LAN1_IPV4_ADDR] = payloadLib->getCnfgArrayAtIndex(1).toString ();
            ipv6AddrStr[WIZ_LAN1_IPV6_ADDR] = payloadLib->getCnfgArrayAtIndex(22).toString ();

            if(devTable.numOfLan == 2)
            {
                lanStatusStr[WIZ_LAN2_STATUS] = lanstateStr[payloadLib->getCnfgArrayAtIndex(4).toInt ()];
                ipv4AddrStr[WIZ_LAN2_IPV4_ADDR] = payloadLib->getCnfgArrayAtIndex(5).toString ();
                ipv6AddrStr[WIZ_LAN2_IPV6_ADDR] = payloadLib->getCnfgArrayAtIndex(23).toString ();
            }
            else
            {
                lanStatusStr[WIZ_LAN2_STATUS]       = "-- ";
                ipv4AddrStr[WIZ_LAN2_IPV4_ADDR] = "-- ";
                ipv6AddrStr[WIZ_LAN2_IPV6_ADDR] = "-- ";
            }

            m_internetConn = payloadLib->getCnfgArrayAtIndex(21).toBool();
            getHlthStatusFrmDev(currentDevName, HEALTH_STS);
            displayStatus();
        }
        else
        {
            WizardCommon:: InfoPageImage();
            infoPage->raise();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }
    }
    else if(param->cmdType == GET_DATE_TIME)
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            payloadLib->parseDevCmdReply(true, param->payload);
            updateDateTime(payloadLib->getCnfgArrayAtIndex(0).toString());
        }

        m_dateTime->changeText(changeToStandardDateTime());
        if(!m_updateTimer->isActive())
        {
            m_updateTimer->start();
        }
        getAdvanceDetailFrmDev(currentDevName, ADVANCE_STS);
    }
    else if(param->cmdType == HEALTH_STS)
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            applController->UpdateHlthStatusAll (currentDevName,param->payload);
            getHlthStatusFrmDev(currentDevName);
        }
    }
}

bool WizardStatus::getHlthStatusFrmDev(QString devName, SET_COMMAND_e command)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = command;
    param->payload = "";
    if(applController->processActivity(devName, DEVICE_COMM, param) == false)
    {
        WizardCommon:: InfoPageImage();
        infoPage->raise();
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
    }
    return true;
}

void WizardStatus ::getHlthStatusFrmDev (QString devName)
{
    applController->GetHlthStatusAll (devName, hlthRsltParam[0]);
    for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
    {
        if(hlthRsltParam[CAM_CONN_STS][camIndex])
        {
            if(camIndex >= devTable.analogCams)
            {
                if(hlthRsltParam[CAM_STREAM_STS][camIndex])
                {
                    noOfConnectedCameras++;
                }
            }
        }
    }
    m_connectedCam->changeText(QString("%2").arg(noOfConnectedCameras));
}

void WizardStatus::getDateAndTime(QString deviceName)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;
    applController->processActivity(deviceName, DEVICE_COMM, param);
}

QString WizardStatus::changeToStandardDateTime()
{
    QString dateTime = (m_currentDate >= 10 ? QString("%1").arg(m_currentDate) : "0" + QString("%1").arg(m_currentDate))
            + "-"
            + monthsName[m_currentMonth - 1]
            + "-"
            + QString("%1").arg(m_currentYear)
            + " "
            + (m_currentHour >= 10 ? QString("%1").arg(m_currentHour) : "0" + QString("%1").arg(m_currentHour))
            + ":"
            + (m_currentMinute >= 10 ? QString("%1").arg(m_currentMinute) : "0" + QString("%1").arg(m_currentMinute))
            + ":"
            + (m_currentSecond >= 10 ? QString("%1").arg(m_currentSecond) : "0" + QString("%1").arg(m_currentSecond));

    return dateTime;
}

void WizardStatus::updateDateTime(QString dateTimeString)
{
    m_currentDate = dateTimeString.mid(0, 2).toInt();
    m_currentMonth = dateTimeString.mid(2, 2).toInt();
    m_currentYear = dateTimeString.mid(4, 4).toInt();

    m_currentHour = dateTimeString.mid(8, 2).toInt() ;
    m_currentMinute = dateTimeString.mid(10, 2).toInt();
    m_currentSecond = dateTimeString.mid(12, 2).toInt();
}

void WizardStatus:: displayStatus()
{
    for(quint8 row = 0; row < 2; row++)
    {
        lanStatusLabel[row]->changeText(lanStatusStr[row], (INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(8)));
        ipv4AddressLabel[row]->changeText(ipv4AddrStr[row], (INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(8)));
        ipv6AddressLabel[row]->changeText(ipv6AddrStr[row], (INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(8)));
    }

    if(m_internetConn == 1)
    {
        m_internetImage->updateImageSource(QString(INTERNET_CONNECTED), true);
    }
    else
    {
        m_internetImage->updateImageSource(QString(INTERNET_DISCONNECTED), true);
    }
}

WizardStatus::~WizardStatus()
{
    DELETE_OBJ(m_statusPageHeading);
    for(quint8 index = WIZ_DATE_TIME; index < WIZ_MAX_STATUS_PAGE_FIELD; index++)
    {
        DELETE_OBJ(statusPage[index]);
        DELETE_OBJ(statusPageLabel[index]);
    }

    DELETE_OBJ(m_dateTime);
    DELETE_OBJ(m_connectedCam);
    DELETE_OBJ(m_internetImage);
    DELETE_OBJ(bgTile);

    for(quint8 index = WIZ_INTERFACE; index < WIZ_MAX_INTERFACE_HEADER_INDEX; index++)
    {
        DELETE_OBJ(interfaceHeader[index]);
        DELETE_OBJ(interfaceHeaderStr[index]);
    }

    for(quint8 index = 0; index < 2; index++)
    {      
        DELETE_OBJ(interfaceFieldStatus[index]);
        DELETE_OBJ(interfaceFieldIpAddress[index]);
        DELETE_OBJ(lanStatusLabel[index]);
        DELETE_OBJ(ipv4AddressLabel[index]);
        DELETE_OBJ(ipv6AddressLabel[index]);
    }

    if(IS_VALID_OBJ(m_updateTimer))
    {
        if(m_updateTimer->isActive())
        {
            m_updateTimer->stop();
        }
        disconnect(m_updateTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotUpdateDateTime()));
        DELETE_OBJ(m_updateTimer);
    }

    if(IS_VALID_OBJ(infoPage))
    {
        disconnect(infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageCnfgBtnClick(int)));
        DELETE_OBJ(infoPage);
    }

    DELETE_OBJ(payloadLib);
}

void WizardStatus::saveConfig()
{

}
