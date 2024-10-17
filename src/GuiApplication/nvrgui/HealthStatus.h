#ifndef HEALTHSTATUS_H
#define HEALTHSTATUS_H

#include "EnumFile.h"
#include "DataStructure.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "Controls/Rectangle.h"
#include "Controls/TextLabel.h"
#include "Controls/Closebuttton.h"
#include "Controls/HlthStsBgTile.h"
#include "Controls/DropDown.h"
#include "Controls/ToolTip.h"
#include "Controls/CnfgButton.h"
#include "Controls/PageOpenButton.h"
#include "Controls/BackGround.h"
#include "Controls/TextWithBackground.h"

#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "Controls/MenuButton.h"
#include "Controls/ControlButton.h"

#define MAX_CAM_STS_PAGE                6
#define MAX_COL_DEVICEFIELD_ARRY        16
#define HLT_MAX_CAM_COL                 16
#define HLT_MAX_FIELD_ROW               17

typedef enum
{
    // Camera status fields
    HLT_CAMERA_CONNECTIVITY = 0,                // 0
    HLT_MANUAL_RECORDING,
    HLT_SCHEDULE_RECORDING,
    HLT_ALARM_RECORDING,
    HLT_ADAPTIVE_RECORDING,
    #if !defined(OEM_JCI)
    HLT_COSEC_RECORDING,
    #endif
    HLT_CAM_ALARM_1,
    HLT_CAM_ALARM_2,
    HLT_CAM_ALARM_3,
    HLT_PTZ_TOUR,
    MAX_HLT_CAM_STS,

    // Camera event status fields
    HLT_MOTION_DETECTION = MAX_HLT_CAM_STS,     // 10
    HLT_VIEW_TEMPER,
    HLT_TRIP_WIRE,
    HLT_OBJECT_ITRUSION,
    HLT_MISSING_OBJECT,
    HLT_SUSPICIOUS_OBJECT,
    HLT_LOITERING_DETECTION,
    HLT_AUDIO_EXCEPTION,
    HLT_CAM_SENSOR_1,
    HLT_CAM_SENSOR_2,
    HLT_CAM_SENSOR_3,
    HLT_CAM_OBJECT_COUNTING,
    MAX_HLT_TOTAL_CAM_STS,

    // System status fields
    HLT_SENSOR_INPUT = MAX_HLT_TOTAL_CAM_STS,   // 22
    HLT_ALARM_OUTPUT,
    HLT_HDD_STATUS,
    HLT_SCHEDULE_BACKUP,
    HLT_MANUAL_TRIGGER,

    MAX_DEVICE_HLT_STATUS,

}HLT_LIST_e;

typedef enum
{
    HLT_TRANSPARENT_COLOR = 0,
    HLT_GREEN_COLOR,
    HLT_RED_COLOR,
    HLT_YELLOW_COLOR,
    HLT_BLUE_COLOR,
    HLT_WHITE_COLOR,

    MAX_HLT_COLOR
}HLT_COLOR_e;

typedef enum
{
    HLT_NO_TOOLTIP,
    HLT_TOOLTIP_ACTIVE,
    HLT_TOOLTIP_DISCONN,
    HLT_TOOLTIP_CONN,
    HLT_TOOLTIP_VIDEO_LOSS,
    HLT_TOOLTIP_ON,
    HLT_TOOLTIP_AUTO_TOUR,
    HLT_TOOLTIP_MAN_TOUR,
    HLT_TOOLTIP_PAUSE_TOUR,
    HLT_TOOLTIP_HDD_NOR,
    HLT_TOOLTIP_HDD_FULL,
    HLT_TOOLTIP_HDD_LOWMEM,
    HLT_TOOLTIP_HDD_FAULT,
    HLT_TOOLTIP_HDD_BUSY,
    HLT_TOOLTIP_SHEBKP_OFF,
    HLT_TOOLTIP_SHEBKP_ON,
    HLT_TOOLTIP_SHEBKP_COMPLETE,
    HLT_TOOLTIP_SHEBKP_INCOMP,
    HLT_TOOLTIP_FAIL,

    MAX_HLT_TOOLTIP_PARAM
}HLT_TOOLTIP_PARAM_e;

typedef enum
{
    HLT_CLOSE_BUTTON,
    HLT_SPIN_BOX,
    HLT_CAMERA_TAB,
    HLT_CAMERA_EVENTS_TAB,
    HLT_SYSTEM_TAB,
    HLT_PREV_CAM_BTN,
    HLT_PAGE_NUM_BTN,
    HLT_NEXT_CAM_BTN = (HLT_PAGE_NUM_BTN + MAX_CAM_STS_PAGE),
    HLT_REFRESH_BUTTON,
    HLT_NEXT_PAGE_BUTTON,

    MAX_HLT_CONTROL
}HLT_CONTROL_e;

typedef enum
{
    CAMERA_TAB,
    CAMERA_EVENTS_TAB,
    SYSTEM_TAB
}MENU_TAB_e;

class HealthStatus : public BackGround
{
    Q_OBJECT
public:
    explicit HealthStatus(QWidget *parent =0);
    ~HealthStatus();

    void showSystemStausTab();
    void showHlthStatus(QString devName, MENU_TAB_e tabToShow);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void setDefaultFocus();

    void showEvent (QShowEvent *event);
    void updateDeviceList(void);

    //keyboard support added
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void functionKeyPressed(QKeyEvent *event);

signals:
    void sigNextPage(QString devName, MENU_TAB_e tabToShow);

public slots:
    void slotStatusTileMouseHover(int tileIndex, bool hoverState);
    void slotSpinBoxValueChange(QString newName, quint32 indexInPage);
    void slotRefreshButtonClick(int index);
    void slotNextPageClick(int);
    void slotInfoPageCnfgBtnClick(int);
    void slotUpadateCurrentElement(int index);
    void slotPrevNextCameraClicked(int index);
    void slotTabSelected(int index);
    void slotPageNumberButtonClick(QString);
    void slotDropDownListDestroyed();

private:
    bool            responseStatus;
    bool            nextPageSelected;

    MENU_TAB_e      selectedTabIndex;
    quint8          currCamPage;
    quint8          totalPages;
    QString         currDevName;

    DEV_TABLE_INFO_t devTable;

    InfoPage*       infoPage;
    ProcessBar*     processBar;

    ApplController* applControl;
    PayloadLib*     payloadLib;

    Rectangle*      innerRect;
    DropDown*       deviceDropDown;
    ToolTip*        toolTip;
    TextLabel*      camNoText[HLT_MAX_CAM_COL];
    CnfgButton*     cnfgButton;
    PageOpenButton* pageOpenButton;
    MenuButton*     cameraTab;
    MenuButton*     cameraEventsTab;
    MenuButton*     systemTab;
    ControlButton*  prevButton;
    ControlButton*  nextButton;

    HlthStsBgTile*  camFieldArray[MAX_HLT_TOTAL_CAM_STS];
    HlthStsBgTile*  statusBgRect1[MAX_HLT_TOTAL_CAM_STS][HLT_MAX_CAM_COL];

    quint8 hlthRsltParam[MAX_PARAM_STS][MAX_CAMERAS];
    quint8 colorState[MAX_DEVICE_HLT_STATUS][MAX_CAMERAS];
    quint8 tooltipState[MAX_DEVICE_HLT_STATUS][MAX_CAMERAS];

    NavigationControl* m_elementList[MAX_HLT_CONTROL];
    int m_currElement;

    TextWithBackground*     m_PageNumberLabel[MAX_CAM_STS_PAGE];


    void createDefaultComponent();
    void devNameChanged(QString devName);

    void getHlthStatusFrmDev(QString devName);
    bool getHlthStatusFrmDev(QString devName, SET_COMMAND_e command);

    void showCamStatus(MENU_TAB_e tabIndex);

    void takeLeftKeyAction();
    void takeRightKeyAction();

};

#endif // HEALTHSTATUS_H
