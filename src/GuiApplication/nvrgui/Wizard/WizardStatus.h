#ifndef WIZARDSTATUS_H
#define WIZARDSTATUS_H

#include <QWidget>
#include "Controls/TextLabel.h"
#include "Controls/Bgtile.h"
#include "Controls/TableCell.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "WizardCommon.h"
#include "EnumFile.h"
#include "Controls/InfoPage.h"

typedef enum
{
    WIZ_DATE_TIME,
    WIZ_CONNECTED_CAMERA,
    WIZ_INTERNET,
    WIZ_MAX_STATUS_PAGE_FIELD
}WIZ_STATUS_PAGE_FIELD_e;

typedef enum
{
    WIZ_INTERFACE,
    WIZ_STATUS,
    WIZ_IP_ADDRESS,
    WIZ_LAN1,
    WIZ_LAN2,
    WIZ_MAX_INTERFACE_HEADER_INDEX

}WIZ_INTERFACE_HEADER_INDEX_e;

typedef enum
{
    WIZ_LAN1_STATUS,
    WIZ_LAN2_STATUS,
    WIZ_MAX_LAN_STATUS_INDEX,

    WIZ_LAN1_IPV4_ADDR = 0,
    WIZ_LAN2_IPV4_ADDR,

    WIZ_LAN1_IPV6_ADDR = 0,
    WIZ_LAN2_IPV6_ADDR,

    WIZ_MAX_INTERFACE_TABLE_ROW = 2

}WIZ_INTERFACE_TABLE_FIELD_INDEX_e;

class WizardStatus : public WizardCommon
{
    Q_OBJECT
public:
    WizardStatus(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId);
    ~WizardStatus();
    void saveConfig();
    void createDefaultElements(QString subHeadStr);
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void displayStatus();
    void getDateAndTime(QString deviceName);
    void updateDateTime(QString dateTimeString);
    void updateDateTime(QDateTime dateTime);
    QString changeToStandardDateTime();
    bool getAdvanceDetailFrmDev(QString devName, SET_COMMAND_e command);
    bool getHlthStatusFrmDev(QString devName, SET_COMMAND_e command);
    void getHlthStatusFrmDev (QString devName);

private:
    TextLabel               *m_statusPageHeading;
    PayloadLib              *payloadLib;
    ApplController          *applController;
    BgTile                  *statusPage[WIZ_MAX_STATUS_PAGE_FIELD];
    TextLabel               *statusPageLabel[WIZ_MAX_STATUS_PAGE_FIELD];
    TextLabel               *m_dateTime;
    TextLabel               *m_connectedCam;
    BgTile                  *bgTile;
    TableCell               *interfaceHeader[WIZ_MAX_INTERFACE_HEADER_INDEX];
    TextLabel               *interfaceHeaderStr[WIZ_MAX_INTERFACE_HEADER_INDEX];
    InfoPage                *infoPage;
    QString                 currentDevName;
    QString                 lanStatusStr[WIZ_MAX_LAN_STATUS_INDEX];
    QString                 ipv4AddrStr[WIZ_MAX_INTERFACE_TABLE_ROW];
    QString                 ipv6AddrStr[WIZ_MAX_INTERFACE_TABLE_ROW];
    TableCell               *interfaceFieldStatus[WIZ_MAX_LAN_STATUS_INDEX];
    TableCell               *interfaceFieldIpAddress[WIZ_MAX_INTERFACE_TABLE_ROW];
    TextLabel               *lanStatusLabel[WIZ_MAX_LAN_STATUS_INDEX];
    TextLabel               *ipv4AddressLabel[WIZ_MAX_INTERFACE_TABLE_ROW];
    TextLabel               *ipv6AddressLabel[WIZ_MAX_INTERFACE_TABLE_ROW];
    int                     m_currentDate, m_currentMonth, m_currentYear,
                            m_currentHour, m_currentMinute, m_currentSecond;

    QTimer*                 m_updateTimer;
    bool                    m_internetConn;
    Image                   *m_internetImage;
    quint8                  hlthRsltParam[MAX_PARAM_STS][MAX_CAMERAS];
    quint8                  noOfConnectedCameras;
    DEV_TABLE_INFO_t        devTable;

public slots:
    void slotUpdateDateTime();
    void slotInfoPageCnfgBtnClick(int);
};

#endif // WIZARDSTATUS_H
