#ifndef ADVANCEDETAILS_H
#define ADVANCEDETAILS_H

#include "ApplController.h"
#include "DataStructure.h"
#include "PayloadLib.h"

#include "Controls/BackGround.h"
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/Bgtile.h"
#include "Controls/TableCell.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/PageOpenButton.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "Controls/MenuButton.h"
#include "Controls/ControlButton.h"
#include "Controls/TextWithBackground.h"

#define MAX_ADV_STS_PAGE            2
#define MAX_CAMERA_IN_ONE_BG        16
#define MAX_BGTILES                 4

typedef enum
{
    ADV_DETAIL_CLOSE_BUTTON,
    ADV_DETAIL_SPINBOX,
    ADV_DETAIL_CAMERA_TAB,
    ADV_DETAIL_SYSTEM_TAB,
    ADV_DETAIL_PREV_BTN,
    ADV_DETAIL_PAGE_NUM_BTN,
    ADV_DETAIL_NEXT_BTN = (ADV_DETAIL_PAGE_NUM_BTN + MAX_ADV_STS_PAGE),
    ADV_DETAIL_PREV_PAGE_BUTTON,
    MAX_ADV_DETAIL_CONTROL

}ADV_DETAIL_CONTROL_e;

typedef enum
{
    ADV_DETAIL_INTERFACE_LAN1,
    ADV_DETAIL_INTERFACE_LAN2,
    ADV_DETAIL_INTERFACE_BROADBAND,

    MAX_ADV_DETAIL_INTERFACE

}ADV_DETAIL_INTERFACE_e;

typedef enum
{
    ADV_DETAIL_STATUS_FIELD,
    ADV_DETAIL_IP_ADDR_FILED,
    ADV_DETAIL_UPLINK_FIELD,
    ADV_DETAIL_DOWNLINK_FIELD,

    MAX_ADV_DETAIL_INTERFACE_FIELD

}ADV_DETAIL_INTERFACE_FIELD_e;

typedef enum
{
    ADV_DETAIL_INTERFACE,
    ADV_DETAIL_STATUS,
    ADV_DETAIL_IP_ADDR,
    ADV_DETAIL_UPLINK,
    ADV_DETAIL_DOWNLINK,
    ADV_DETAIL_LAN1,
    ADV_DETAIL_LAN2,
    ADV_DETAIL_BROADBAND,

    MAX_ADV_DETAIL_INTERFACE_HEADER

}ADV_DETAIL_INTERFACE_HEADER_e;
class AdvanceDetails : public BackGround
{
    Q_OBJECT
public:
    explicit AdvanceDetails(QWidget *parent = 0);
    ~AdvanceDetails();

    void createDefaultComponent();
    bool getAdvanceDetailFrmDev(QString devName, SET_COMMAND_e command);
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void convertSecToHMS(int sec, quint8 index);
    void resetVisibleElements();
    void showEvent(QShowEvent *event);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void setDefaultFocus();
    void clearDetails();
    void showAdvDetail(QString devName, bool forceCamFieldToShow = true);
    void devNameChanged(QString devName);
    void showHideCamTile(quint8 tileIndex, bool showTile, quint8 totalCamInRow = MAX_CAMERA_IN_ONE_BG);
    void showHideSystemTiles(bool showTile);
    void updateDeviceList(void);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

private:
    bool                    isCamField;
    bool                    responseStatus;
    QString                 currentDevName;

    DEV_TABLE_INFO_t        devTable;

    quint32                 daysParse[2];
    quint32                 hourParse[2];
    quint32                 minuteParse[2];
    quint32                 secParse[2];

    quint8                  currCamPage;
    quint8                  totalPages;

    InfoPage*               infoPage;
    ProcessBar*             processBar;

    ApplController*         applController;
    PayloadLib*             payloadLib;

    Rectangle*              innerRect;
    DropDown*               deviceDropDown;
    MenuButton*             cameraTab;
    MenuButton*             systemTab;

    BgTile*                 camStateTiles[MAX_BGTILES];
    TableCell*              channelHeaderCamNo[MAX_CAMERA_IN_ONE_BG*MAX_BGTILES];
    TextLabel*              channelHeaderCamNoStr[MAX_CAMERA_IN_ONE_BG*MAX_BGTILES];
    TableCell*              channelHeader[3*MAX_BGTILES];
    TextLabel*              channelHeaderStr[3*MAX_BGTILES];
    TableCell*              channelField[2*MAX_BGTILES][MAX_CAMERA_IN_ONE_BG];
    TextLabel*              channelFieldStr[2*MAX_BGTILES][MAX_CAMERA_IN_ONE_BG];

    quint8                  channelFpsValue[MAX_LIVE_STREAM_TYPE][MAX_CAMERAS];
    quint8                  channelGopValue[MAX_LIVE_STREAM_TYPE][MAX_CAMERAS];

    quint16                 liveStream;
    quint8                  pbStream;
    quint8                  cpuUseValue;
    quint8                  cpuTempValue;
    QString                 upTimeStr;

    QString                 recStatusPerHr;
    QString                 recStatusPerDay;
    QString                 activeRecStream;

    QTimer*                 refreshInterval;

    BgTile                  *bgTile4, *bgTile5, *bgTile6, *bgTile7, *bgTile8;
    TableCell*              interfaceHeader[MAX_ADV_DETAIL_INTERFACE_HEADER];
    TextLabel*              interfaceHeaderStr[MAX_ADV_DETAIL_INTERFACE_HEADER];

    TableCell*              interfaceField[MAX_ADV_DETAIL_INTERFACE][MAX_ADV_DETAIL_INTERFACE_FIELD];

    QString                 networkStatusStr[MAX_ADV_DETAIL_INTERFACE];
    QString                 ipv4AddrStr[MAX_ADV_DETAIL_INTERFACE];
    QString                 ipv6AddrStr[MAX_ADV_DETAIL_INTERFACE];
    QString                 upLinkStr[MAX_ADV_DETAIL_INTERFACE];
    QString                 downLinkStr[MAX_ADV_DETAIL_INTERFACE];

    TextLabel               *networkStatusLabel[MAX_ADV_DETAIL_INTERFACE];
    TextLabel               *ipv4AddressLabel[MAX_ADV_DETAIL_INTERFACE];
    TextLabel               *ipv6AddressLabel[MAX_ADV_DETAIL_INTERFACE];
    TextLabel               *upLinkLabel[MAX_ADV_DETAIL_INTERFACE];
    TextLabel               *downLinkLabel[MAX_ADV_DETAIL_INTERFACE];

    TextLabel*              networkStreamStr;
    ReadOnlyElement*        livePbElement[2];

    TextLabel*              systemStrHeader;
    ReadOnlyElement*        systemFieldReadOnly[3];

    TextLabel*              recStatusStr;
    ReadOnlyElement*        recStatus[3];

    TextLabel*              recRateStr;
    ReadOnlyElement*        recRate[2];

    PageOpenButton*         backButton;

    ControlButton*          prevButton;
    ControlButton*          nextButton;

    NavigationControl*      m_elementList[MAX_ADV_DETAIL_CONTROL];
    int                     m_currElement;

    TextWithBackground*     m_PageNumberLabel[MAX_ADV_STS_PAGE];
    bool                    nextPageSelected;
    bool                    m_internetConn;


signals:
    void sigPrevPage(QString devName, bool isCamFieldToShow);

public slots:
    void slotRefreshButtonClicked();
    void slotBackButtonClicked(int);
    void slotInfoPageCnfgBtnClick(int);
    void slotUpadateCurrentElement(int index);
    void slotSpinBoxValueChange(QString str,quint32);
    void slotTabSelected(int index);
    void slotPrevNextCameraClicked(int);
    void slotPageNumberButtonClick(QString);
    void slotDropDownListDestroyed();
};

#endif // ADVANCEDETAILS_H
