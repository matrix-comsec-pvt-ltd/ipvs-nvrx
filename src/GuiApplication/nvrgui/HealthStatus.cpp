#include "HealthStatus.h"
#include "ValidationMessage.h"

#include <QKeyEvent>

#define HLTH_MAIN_RECT_WIDTH        SCALE_WIDTH(1126)
#define HLTH_MAIN_RECT_HEIGHT       SCALE_HEIGHT(790)
#define HLTH_HEADER_RECT_WIDTH      SCALE_WIDTH(305)
#define HLTH_HEADER_RECT_HEIGHT     SCALE_HEIGHT(45)
#define HLTH_INNER_RECT_WIDTH       SCALE_WIDTH(1096)
#define HLTH_INNER_RECT_HEIGHT      SCALE_HEIGHT(565)
#define HLTH_RECT_RADIUS            SCALE_WIDTH(15)
#define HLTH_INNER_RECT_RADIUS      SCALE_WIDTH(5)
#define HLTH_PREV_NEXT_BTN_WIDTH    SCALE_WIDTH(30)
#define NO_OF_HDD                   6

static const QString HlthStsStr[MAX_DEVICE_HLT_STATUS] =
{
    // Camera status fields
    "Camera Status",
    "Manual Recording",
    "Scheduled Recording",
    "Alarm Recording",
    "Adaptive Recording",
    #if !defined(OEM_JCI)
    "COSEC Recording",
    #endif
    "Camera Alarm 1",
    "Camera Alarm 2",
    "Camera Alarm 3",
    "PTZ Tour",

    // Camera event status fields
    "Motion Detection",
    "View Tampering",
    "Trip Wire",
    "Object Intrusion",
    "Missing Object",
    "Suspicious Object",
    "Loitering Detection",
    "Audio Exception",
    "Camera Sensor 1",
    "Camera Sensor 2",
    "Camera Sensor 3",
    "Object Counting",

    // System status fields
    "Sensor Input",
    "Alarm Output",
    "Recording Drive Status",
    "Scheduled Backup",
    "Manual Trigger"
};

static const QString tooltipStateStr[MAX_HLT_TOOLTIP_PARAM] =
{
    "",
    "Active",
    "Camera Offline",
    "Camera Online",
    "Video Loss",
    "ON",
    "Auto Tour",
    "Manual Tour",
    "Pause",
    "Normal",
    "Full",
    "Low Memory",
    "Fault",
    "Busy",
    "OFF",
    "ON",
    "Complete",
    "Incomplete",
    "Fail"
};

static const QString statusColor[MAX_HLT_COLOR] =
{
    NORMAL_BKG_COLOR,
    GREEN_COLOR,
    RED_COLOR,
    YELOW_COLOR,
    BLUE_COLOR,
    WHITE_COLOR
};

static quint8 bgStatusTileIndex[MAX_DEVICE_HLT_STATUS - MAX_HLT_TOTAL_CAM_STS] =
{
    MAX_DEV_SENSOR, MAX_DEV_ALARM, 1, 1, 1
};

typedef enum
{
    INACTIVE_HLTH_STATUS = 0,
    ACTIVE_HLTH_STATUS
}ACTIVE_HLTH_STATE_e;

typedef enum
{
    REC_STOP = 0,
    REC_START,
    REC_FAIL
}REC_STATE_e;

typedef enum
{
    NORAL_DISK_STS = 0,
    NO_DISK_STS,
    FULL_DISK_STS,
    LOWMEM_DISK_STS,
    FAULT_DISK_STS,
    BUSY_DISK_STS
}DISK_HLTH_STATUS_e;

typedef enum
{
    NO_TOUR = 0,
    AUTO_TOUR,
    MANUAL_TOUR,
    PAUSE_TOUR
}TOUR_HLTH_STATE_e;

typedef enum
{
    STOP_SCHBACKUP = 0,
    START_SCHBACKUP,
    COMPLETE_SCHBACKUP,
    INCOMPLETE_SCHBACKUP,
    DISABLE_SCHBACKUP
}SCHBACKUP_STATE_e;

HealthStatus::HealthStatus(QWidget * parent)
    :BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - HLTH_MAIN_RECT_WIDTH) / 2)),
                (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - HLTH_MAIN_RECT_HEIGHT) / 2)),
                HLTH_MAIN_RECT_WIDTH,
                HLTH_MAIN_RECT_HEIGHT - HLTH_HEADER_RECT_HEIGHT,
                BACKGROUND_TYPE_1,
                SYSTEM_STATUS_BUTTON,
                parent,
                HLTH_HEADER_RECT_WIDTH,
                HLTH_HEADER_RECT_HEIGHT,
                "Device Status")
{
    toolTip = NULL;
    payloadLib = NULL;
    for(quint8 index = 0; index < MAX_DEVICE_HLT_STATUS; index++)
    {
        for(quint8 camIndex = 0; camIndex < MAX_CAMERAS; camIndex++)
        {
            colorState[index][camIndex] = 0;
            tooltipState[index][camIndex] = 0;
        }
    }

    for(quint8 index = 0; index < MAX_PARAM_STS; index++)
    {
        for(quint8 camIndex = 0; camIndex < MAX_CAMERAS; camIndex++)
        {
            hlthRsltParam[index][camIndex] = 0;
        }
    }
    //    this->setAttribute ( Qt::WA_PaintOutsidePaintEvent);
    this->setMouseTracking (true);

    applControl = ApplController::getInstance ();
    payloadLib = new PayloadLib();

    currDevName = "";
    currCamPage = 0;
    totalPages = 0;
    selectedTabIndex = CAMERA_TAB;
    responseStatus = false;
    nextPageSelected = false;

    createDefaultComponent ();

    applControl->GetDeviceInfo(LOCAL_DEVICE_NAME, devTable);

    processBar = new ProcessBar(0, HLTH_HEADER_RECT_HEIGHT,
                                HLTH_MAIN_RECT_WIDTH,
                                (HLTH_MAIN_RECT_HEIGHT - HLTH_HEADER_RECT_HEIGHT),
                                SCALE_WIDTH(15), this);

    infoPage = new InfoPage(0, 0,
                            HLTH_MAIN_RECT_WIDTH,
                            HLTH_MAIN_RECT_HEIGHT,
                            INFO_ADVANCE_DETAILS,
                            this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    cameraTab->setShowClickedImage (true);
    devNameChanged(applControl->GetRealDeviceName(deviceDropDown->getCurrValue()));
    setDefaultFocus();
    this->show ();
}

HealthStatus :: ~HealthStatus()
{
    delete processBar;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;

    disconnect (m_mainCloseButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));

    disconnect (deviceDropDown,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect (deviceDropDown,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    disconnect(deviceDropDown,
               SIGNAL(sigDropDownListDestroyed()),
               this,
               SLOT(slotDropDownListDestroyed()));
    delete deviceDropDown;

    disconnect (cameraTab,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect (cameraTab,
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotTabSelected(int)));
    delete cameraTab;

    disconnect (cameraEventsTab,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect (cameraEventsTab,
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotTabSelected(int)));
    delete cameraEventsTab;

    disconnect (systemTab,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect (systemTab,
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotTabSelected(int)));
    delete systemTab;

    delete innerRect;

    for(quint8 index = 0; index < HLT_MAX_FIELD_ROW; index++)
    {
        delete camFieldArray[index];
    }

    for(quint8 row = 0; row < HLT_MAX_FIELD_ROW; row++)
    {
        for(quint8 col = 0; col < HLT_MAX_CAM_COL; col++)
        {
            disconnect (statusBgRect1[row][col],
                        SIGNAL(sigMouseHover(int,bool)),
                        this,
                        SLOT(slotStatusTileMouseHover(int,bool)));
            delete statusBgRect1[row][col];
        }
    }

    for(quint8 index = 0; index < HLT_MAX_CAM_COL; index++)
    {
        delete camNoText[index];
    }
    delete toolTip;

    disconnect (cnfgButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect (cnfgButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotRefreshButtonClick(int)));
    delete cnfgButton;

    disconnect(pageOpenButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect (pageOpenButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotNextPageClick(int)));
    delete pageOpenButton;


    disconnect (prevButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPrevNextCameraClicked(int)));
    disconnect (prevButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete prevButton;

    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPrevNextCameraClicked(int)));
    disconnect (nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete nextButton;

    for(quint8 index = 0; index < MAX_CAM_STS_PAGE; index++)
    {
        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigMousePressClick(QString)),
                   this,
                   SLOT(slotPageNumberButtonClick(QString)));

        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpadateCurrentElement(int)));
        delete m_PageNumberLabel[index];
    }

    DELETE_OBJ(payloadLib);
}

void HealthStatus::createDefaultComponent ()
{
    m_elementList[HLT_CLOSE_BUTTON] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(HLT_CLOSE_BUTTON);
    connect (m_mainCloseButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    QMap<quint8, QString> deviceMapList;
    applControl->GetDevNameDropdownMapList(deviceMapList);

    deviceDropDown = new DropDown(((HLTH_MAIN_RECT_WIDTH - HLTH_INNER_RECT_WIDTH) /2),
                                  SCALE_HEIGHT(60),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  HLT_SPIN_BOX,
                                  DROPDOWNBOX_SIZE_200,
                                  "Devices",
                                  deviceMapList,
                                  this,
                                  "",
                                  false,
                                  0,
                                  NO_LAYER);

    m_elementList[HLT_SPIN_BOX] = deviceDropDown;
    connect (deviceDropDown,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString,quint32)));

    connect (deviceDropDown,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    connect(deviceDropDown,
            SIGNAL(sigDropDownListDestroyed()),
            this,
            SLOT(slotDropDownListDestroyed()));

    cameraTab = new MenuButton(0,
                               SCALE_WIDTH(150),
                               SCALE_HEIGHT(30),
                               "Camera",
                               this,
                               0,
                               ((HLTH_MAIN_RECT_WIDTH - HLTH_INNER_RECT_WIDTH) /2),
                               (deviceDropDown->y () + deviceDropDown->height () + SCALE_HEIGHT(20)),
                               HLT_CAMERA_TAB);
    m_elementList[HLT_CAMERA_TAB] = cameraTab;
    connect (cameraTab,
             SIGNAL(sigButtonClicked(int)),
             this,
             SLOT(slotTabSelected(int)));
    connect (cameraTab,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    cameraEventsTab = new MenuButton(1,
                                     SCALE_WIDTH(150),
                                     SCALE_HEIGHT(30),
                                     "Camera Events",
                                     this,
                                     0,
                                     (cameraTab->x() + cameraTab->width()),
                                     (cameraTab->y() - SCALE_HEIGHT(30)),
                                     HLT_CAMERA_EVENTS_TAB);
    m_elementList[HLT_CAMERA_EVENTS_TAB] = cameraEventsTab;
    connect (cameraEventsTab,
             SIGNAL(sigButtonClicked(int)),
             this,
             SLOT(slotTabSelected(int)));
    connect (cameraEventsTab,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    systemTab = new MenuButton(2,
                               SCALE_WIDTH(150),
                               SCALE_HEIGHT(30),
                               "System",
                               this,
                               0,
                               (cameraEventsTab->x() + cameraEventsTab->width()),
                               (cameraEventsTab->y() - SCALE_HEIGHT(60)),
                               HLT_SYSTEM_TAB);
    m_elementList[HLT_SYSTEM_TAB] = systemTab;
    connect (systemTab,
             SIGNAL(sigButtonClicked(int)),
             this,
             SLOT(slotTabSelected(int)));
    connect (systemTab,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    innerRect = new Rectangle(((HLTH_MAIN_RECT_WIDTH - HLTH_INNER_RECT_WIDTH) /2),
                               cameraTab->y () + cameraTab->height (),
                               HLTH_INNER_RECT_WIDTH,
                               HLTH_INNER_RECT_HEIGHT,
                               HLTH_INNER_RECT_RADIUS,
                               BORDER_2_COLOR,
                               CLICKED_BKG_COLOR,
                               this);

    for(quint8 index = 0; index < HLT_MAX_FIELD_ROW; index++)
    {
        camFieldArray[index] = new HlthStsBgTile((innerRect->x () + SCALE_WIDTH(20)),
                                                  ((innerRect->y () + SCALE_HEIGHT(23)) + (index * SCALE_HEIGHT(30))),
                                                  HLT_BGTILE_LARGE_WIDTH,
                                                  HLT_BGTILE_HEIGHT,
                                                  17,
                                                  this,
                                                  (index < MAX_HLT_CAM_STS) ? (HlthStsStr[index]) : "");
    }

    for(quint8 row = 0; row < HLT_MAX_FIELD_ROW; row++)
    {
        for(quint8 col = 0; col < HLT_MAX_CAM_COL; col++)
        {
            statusBgRect1[row][col] = new HlthStsBgTile((camFieldArray[row]->x () +
                                                           camFieldArray[row]->width () +
                                                           SCALE_WIDTH(10) + (col * HLT_BGTILE_SMALL_WIDTH)),
                                                          camFieldArray[row]->y (),
                                                          HLT_BGTILE_SMALL_WIDTH,
                                                          HLT_BGTILE_HEIGHT,
                                                          ((row *HLT_MAX_CAM_COL) + col),
                                                          this,
                                                          "",
                                                          true);

            connect (statusBgRect1[row][col],
                     SIGNAL(sigMouseHover(int,bool)),
                     this,
                     SLOT(slotStatusTileMouseHover(int,bool)), Qt::QueuedConnection);
        }
    }

    for(quint8 index = 0; index < MAX_CAM_STS_PAGE; index++)
    {
        m_PageNumberLabel[index] = new TextWithBackground((statusBgRect1[HLT_MAX_FIELD_ROW - 1][5]->x() -
                                                          SCALE_WIDTH(100) + (index * SCALE_WIDTH(40))),
                                                          (statusBgRect1[HLT_MAX_FIELD_ROW - 1][0]->y () +
                                                          statusBgRect1[HLT_MAX_FIELD_ROW - 1][0]->height () + SCALE_HEIGHT(3)),
                                                          NORMAL_FONT_SIZE,
                                                          "",
                                                          this,
                                                          NORMAL_FONT_COLOR,
                                                          NORMAL_FONT_FAMILY,
                                                          ALIGN_START_X_START_Y,
                                                          0,
                                                          false,
                                                          TRANSPARENTKEY_COLOR,
                                                          true,
                                                          (HLT_PAGE_NUM_BTN + index));

        m_elementList[(HLT_PAGE_NUM_BTN + index)] = m_PageNumberLabel[index];

        connect(m_PageNumberLabel[index],
                SIGNAL(sigMousePressClick(QString)),
                this,
                SLOT(slotPageNumberButtonClick(QString)));

        connect(m_PageNumberLabel[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));

    }

    currCamPage = 0;
    for(quint8 index = 0; index < MAX_CAM_STS_PAGE; index++)
    {
        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(currCamPage + 1 + index) + QString(" "));
        if (index == 0)
        {
            m_PageNumberLabel[index]->setBackGroundColor(CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor(HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(true);
            m_PageNumberLabel[index]->forceActiveFocus();
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor(TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor(NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(false);
            m_PageNumberLabel[index]->deSelectControl();
        }

        m_PageNumberLabel[index]->update();
    }

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                   m_PageNumberLabel[0]->x() - SCALE_WIDTH(60),
                                   m_PageNumberLabel[0]->y() - SCALE_HEIGHT(7),
                                   HLTH_PREV_NEXT_BTN_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER, -1, "", true,
                                   HLT_PREV_CAM_BTN, false);
    m_elementList[HLT_PREV_CAM_BTN] = prevButton;
    connect (prevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPrevNextCameraClicked(int)));
    connect (prevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   m_PageNumberLabel[MAX_CAM_STS_PAGE-1]->x() + SCALE_WIDTH(60),
                                   m_PageNumberLabel[MAX_CAM_STS_PAGE-1]->y() - SCALE_HEIGHT(7),
                                   HLTH_PREV_NEXT_BTN_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER, -1, "", true,
                                   HLT_NEXT_CAM_BTN);
    m_elementList[HLT_NEXT_CAM_BTN] = nextButton;
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPrevNextCameraClicked(int)));
    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    for(quint8 index = 0; index < HLT_MAX_CAM_COL; index++)
    {
        camNoText[index] = new TextLabel((statusBgRect1[0][index]->x () + SCALE_WIDTH(20)),
                                         innerRect->y () + SCALE_HEIGHT(5),
                                         SCALE_FONT(10),
                                         QString("%1").arg (index +1),
                                         this,
                                         SUFFIX_FONT_COLOR);
    }

    toolTip = new ToolTip(0, 0, "", this);
    toolTip->setVisible(false);

    cnfgButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                HLTH_MAIN_RECT_WIDTH/2,
                                HLTH_MAIN_RECT_HEIGHT - SCALE_HEIGHT(35),
                                "Refresh",
                                this,
                                HLT_REFRESH_BUTTON);
    m_elementList[HLT_REFRESH_BUTTON] = cnfgButton;
    connect (cnfgButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotRefreshButtonClick(int)));

    connect (cnfgButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    pageOpenButton = new PageOpenButton(HLTH_MAIN_RECT_WIDTH - SCALE_WIDTH(100),
                                        HLTH_MAIN_RECT_HEIGHT - SCALE_HEIGHT(55),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        SCALE_HEIGHT(40),
                                        HLT_NEXT_PAGE_BUTTON,
                                        PAGEOPENBUTTON_MEDIAM_NEXT,
                                        "More",
                                        this,"", "",false,
                                        0,
                                        NO_LAYER);

    m_elementList[HLT_NEXT_PAGE_BUTTON] = pageOpenButton;

    connect (pageOpenButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotNextPageClick(int)));
    connect(pageOpenButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
}

bool HealthStatus::getHlthStatusFrmDev(QString devName, SET_COMMAND_e command)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = command;
    param->payload = "";

    if(applControl->processActivity(devName, DEVICE_COMM, param) == false)
    {
        processBar->unloadProcessBar ();
        //device not connected handle properly
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
    }
    return true;
}

void HealthStatus ::getHlthStatusFrmDev (QString devName)
{
    quint8 indx;

    applControl->GetHlthStatusAll (devName, hlthRsltParam[0]);

    for(quint8 row = 0; row < MAX_DEVICE_HLT_STATUS; row++)
    {
        for(quint8 col = 0; col < MAX_CAMERAS; col++)
        {
            colorState[row][col] = HLT_TRANSPARENT_COLOR;
            tooltipState[row][col] = HLT_NO_TOOLTIP;
        }
    }

    for(indx = 0; indx < MAX_DEVICE_HLT_STATUS; indx++)
    {
        switch(indx)
        {
        case HLT_MOTION_DETECTION:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[MOTION_DETECTION_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_VIEW_TEMPER:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[VIEW_TAMPERED_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAMERA_CONNECTIVITY:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_CONN_STS][camIndex])
                {
                    // analog cameras
                    if(camIndex < devTable.analogCams)
                    {
                        colorState[indx][camIndex] = HLT_GREEN_COLOR;
                        tooltipState[indx][camIndex] = HLT_TOOLTIP_CONN;
                    }
                    else // ip cameras
                    {
                        // connected and Stream availble
                        if(hlthRsltParam[CAM_STREAM_STS][camIndex])
                        {
                            colorState[indx][camIndex] = HLT_GREEN_COLOR;
                            tooltipState[indx][camIndex] = HLT_TOOLTIP_CONN;
                        }
                        else   // connected and video loss
                        {
                            colorState[indx][camIndex] = HLT_BLUE_COLOR;
                            tooltipState[indx][camIndex] = HLT_TOOLTIP_VIDEO_LOSS;
                        }
                    }
                }
                else    // disconnected device
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_DISCONN;
                }
            }
            break;

        case HLT_CAM_SENSOR_1:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_SENSOR1_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAM_SENSOR_2:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_SENSOR2_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAM_SENSOR_3:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_SENSOR3_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAM_ALARM_1:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_ALARM1_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAM_ALARM_2:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_ALARM2_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAM_ALARM_3:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[CAM_ALARM3_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_MANUAL_RECORDING:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[MANUAL_RECORDING_STS][camIndex] == REC_START)
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ON;
                }
                else if(hlthRsltParam[MANUAL_RECORDING_STS][camIndex] == REC_FAIL)
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_FAIL;
                }
            }
            break;

        case HLT_ALARM_RECORDING:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[ALARM_RECORDING_STS][camIndex] == REC_START)
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ON;
                }
                else if(hlthRsltParam[ALARM_RECORDING_STS][camIndex] == REC_FAIL)
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_FAIL;
                }
            }
            break;

        case HLT_SCHEDULE_RECORDING:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[SCHEDULE_RECORDING_STS][camIndex] == REC_START)
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ON;
                }
                else if(hlthRsltParam[SCHEDULE_RECORDING_STS][camIndex] == REC_FAIL)
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_FAIL;
                }
            }
            break;

        case HLT_ADAPTIVE_RECORDING:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[ADAPTIVE_RECORDING_STS][camIndex] == REC_START)
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ON;
                }
                else if(hlthRsltParam[ADAPTIVE_RECORDING_STS][camIndex] == REC_FAIL)
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_FAIL;
                }
            }
            break;

        #if !defined(OEM_JCI)
        case HLT_COSEC_RECORDING:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[COSEC_RECORDING_STS][camIndex] == REC_START)
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ON;
                }
                else if(hlthRsltParam[COSEC_RECORDING_STS][camIndex] == REC_FAIL)
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_FAIL;
                }
            }
            break;
        #endif

        case HLT_PTZ_TOUR:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                switch(hlthRsltParam[PTZ_TOUR_TYPE][camIndex])
                {
                case AUTO_TOUR:
                    colorState[indx][camIndex] = HLT_BLUE_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_AUTO_TOUR;
                    break;

                case MANUAL_TOUR:
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_MAN_TOUR;
                    break;

                case PAUSE_TOUR:
                    colorState[indx][camIndex] = HLT_WHITE_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_PAUSE_TOUR;
                    break;

                default:
                    break;
                }
            }
            break;

        case HLT_SENSOR_INPUT:
            for(quint8 camIndex = 0; camIndex < MAX_DEV_SENSOR; camIndex++)
            {
                if(hlthRsltParam[SENSOR_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_ALARM_OUTPUT:
            for(quint8 camIndex = 0; camIndex < MAX_DEV_ALARM; camIndex++)
            {
                if(hlthRsltParam[ALARM_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_HDD_STATUS:
            for(quint8 camIndex = 0; camIndex < NO_OF_HDD; camIndex++)
            {
                switch(hlthRsltParam[DISK_STS][camIndex])
                {
                case NORAL_DISK_STS:
                    colorState[indx][camIndex] = HLT_GREEN_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_HDD_NOR;
                    break;

                case FULL_DISK_STS:
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_HDD_FULL;
                    break;


                case LOWMEM_DISK_STS:
                    colorState[indx][camIndex] = HLT_WHITE_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_HDD_LOWMEM;
                    break;

                case FAULT_DISK_STS:
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_HDD_FAULT;
                    break;

                case BUSY_DISK_STS:
                    colorState[indx][camIndex] = HLT_YELLOW_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_HDD_BUSY;
                    break;

                default:
                    break;
                }
            }
            break;

        case HLT_SCHEDULE_BACKUP:
            switch(hlthRsltParam[SCHEDULE_BACKUP_STS][0])
            {
            case START_SCHBACKUP:
                colorState[indx][0] = HLT_GREEN_COLOR;
                tooltipState[indx][0] = HLT_TOOLTIP_SHEBKP_ON;
                break;

            case STOP_SCHBACKUP:
                colorState[indx][0] = HLT_WHITE_COLOR;
                tooltipState[indx][0] = HLT_TOOLTIP_SHEBKP_OFF;
                break;

            case COMPLETE_SCHBACKUP:
                colorState[indx][0] = HLT_BLUE_COLOR;
                tooltipState[indx][0] = HLT_TOOLTIP_SHEBKP_COMPLETE;
                break;

            case INCOMPLETE_SCHBACKUP:
                colorState[indx][0] = HLT_RED_COLOR;
                tooltipState[indx][0] = HLT_TOOLTIP_SHEBKP_INCOMP;
                break;
            }
            break;

        case HLT_MANUAL_TRIGGER:
            if(hlthRsltParam[MANUAL_TRIGGER_STS][0])
            {
                colorState[indx][0] = HLT_GREEN_COLOR;
                tooltipState[indx][0] = HLT_TOOLTIP_ACTIVE;
            }
            break;

        case HLT_TRIP_WIRE:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[TRIP_WIRE_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_OBJECT_ITRUSION:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[OBJECT_INTRUSION_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_AUDIO_EXCEPTION:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[AUDIO_EXCEPTION_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_MISSING_OBJECT:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[MISSING_OBJJECT_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_SUSPICIOUS_OBJECT:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[SUSPIOUS_OBJECT_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_LOITERING_DETECTION:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[LOITERING_OBJECT_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        case HLT_CAM_OBJECT_COUNTING:
            for(quint8 camIndex = 0; camIndex < devTable.totalCams; camIndex++)
            {
                if(hlthRsltParam[OBJECT_COUNTING_STS][camIndex])
                {
                    colorState[indx][camIndex] = HLT_RED_COLOR;
                    tooltipState[indx][camIndex] = HLT_TOOLTIP_ACTIVE;
                }
            }
            break;

        default:
            break;
        }
    }
}

void HealthStatus::devNameChanged(QString devName)
{
    responseStatus = false;

    if(false == applControl->GetDeviceInfo(devName, devTable))
    {
        switch (selectedTabIndex)
        {
            case CAMERA_TAB:
            case CAMERA_EVENTS_TAB:
                showCamStatus(selectedTabIndex);
                break;

            case SYSTEM_TAB:
                showSystemStausTab();
                break;

            default:
                break;
        }

        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        infoPage->show();
    }
    else
    {
        processBar->loadProcessBar();
        responseStatus = true;
        applControl->GetMaxSensorInputSupport(devName, bgStatusTileIndex[0]);
        applControl->GetMaxAlarmOutputSupport(devName, bgStatusTileIndex[1]);
        getHlthStatusFrmDev(devName,HEALTH_STS);
    }

    currDevName = devName;
}

void HealthStatus::showCamStatus (MENU_TAB_e tabIndex)
{
    quint8 startRowIndex, lastFieldIndex;
    quint8 camCount = 0;
    totalPages = ((devTable.totalCams % HLT_MAX_CAM_COL) == 0) ? (devTable.totalCams / HLT_MAX_CAM_COL) : (devTable.totalCams / HLT_MAX_CAM_COL) + 1;

    if(tabIndex == 0)
    {
        startRowIndex = 0;
        lastFieldIndex = MAX_HLT_CAM_STS;
    }
    else
    {
        startRowIndex = MAX_HLT_CAM_STS;
        lastFieldIndex = MAX_HLT_TOTAL_CAM_STS - MAX_HLT_CAM_STS;
    }

    if(devTable.totalCams < (HLT_MAX_CAM_COL*(currCamPage + 1)))
    {
        camCount = devTable.totalCams - ((HLT_MAX_CAM_COL*(currCamPage)));
    }
    else
    {
        camCount = HLT_MAX_CAM_COL;
    }

    for(quint8 index = 0; index < lastFieldIndex; index++)
    {
        camFieldArray[index]->changeLabelText(HlthStsStr[index + startRowIndex]);
        camFieldArray[index]->setVisible(true);
    }

    for(quint8 index = lastFieldIndex; index < HLT_MAX_FIELD_ROW; index++)
    {
        camFieldArray[index]->changeLabelText("");
        camFieldArray[index]->setVisible(false);
    }

    for(quint8 index = 0; index < camCount; index++)
    {
        if(responseStatus)
        {
            camNoText[index]->changeText(QString("%1").arg((HLT_MAX_CAM_COL*currCamPage) + 1 + index));
        }
        camNoText[index]->setVisible(responseStatus);
    }

    for(quint8 col = camCount; col < HLT_MAX_CAM_COL; col++)
    {
        camNoText[col]->setVisible(false);
    }

    for(quint8 row = 0; row < lastFieldIndex; row++)
    {
        for(quint8 col = 0; col < camCount; col++)
        {
            if(responseStatus)
            {
                statusBgRect1[row][col]->changeInnerRectColor(statusColor[colorState[row + startRowIndex][(HLT_MAX_CAM_COL*currCamPage) + col]]);
            }
            statusBgRect1[row][col]->setVisible(responseStatus);
        }

        for(quint8 col = camCount; col < HLT_MAX_CAM_COL; col++)
        {
            statusBgRect1[row][col]->setVisible (false);
        }
    }

    for(quint8 row = lastFieldIndex; row < HLT_MAX_FIELD_ROW; row++)
    {
        for(quint8 col = 0; col < HLT_MAX_CAM_COL; col++)
        {
            statusBgRect1[row][col]->changeInnerRectColor (NORMAL_BKG_COLOR);
            statusBgRect1[row][col]->setVisible (false);
        }
    }

    if((devTable.totalCams <= HLT_MAX_CAM_COL) || (responseStatus == false))
    {
        prevButton->setIsEnabled (false);
        nextButton->setIsEnabled (false);
        prevButton->setVisible (false);
        nextButton->setVisible (false);

        for(quint8 index = 0; index < MAX_CAM_STS_PAGE; index++)
        {
            m_PageNumberLabel[index]->setVisible (false);
            m_PageNumberLabel[index]->setIsEnabled (false);
        }
    }
    else
    {
        prevButton->setVisible (true);
        nextButton->setVisible (true);
        prevButton->setIsEnabled (true);
        nextButton->setIsEnabled (true);

        for(quint8 index = 0; index < MAX_CAM_STS_PAGE; index++)
        {
            m_PageNumberLabel[index]->setOffset(statusBgRect1[HLT_MAX_FIELD_ROW-1][5]->x() - SCALE_WIDTH(20 + (totalPages-2)*20) +
                                                (index*SCALE_WIDTH(40)), m_PageNumberLabel[index]->y());

            if(index == (currCamPage % MAX_CAM_STS_PAGE))
            {
                m_PageNumberLabel[index]->setBackGroundColor(CLICKED_BKG_COLOR);
                m_PageNumberLabel[index]->changeTextColor(HIGHLITED_FONT_COLOR);
                m_PageNumberLabel[index]->setBold(true);
                m_PageNumberLabel[index]->forceActiveFocus();
                m_currElement = (HLT_PAGE_NUM_BTN + index);
            }
            else
            {
                m_PageNumberLabel[index]->setBackGroundColor(TRANSPARENTKEY_COLOR);
                m_PageNumberLabel[index]->changeTextColor(NORMAL_FONT_COLOR);
                m_PageNumberLabel[index]->setBold(false);
                m_PageNumberLabel[index]->deSelectControl();
            }

            m_PageNumberLabel[index]->update();
        }

        prevButton->resetGeometry((m_PageNumberLabel[0]->x() - SCALE_WIDTH(60)),
                                  (m_PageNumberLabel[0]->y() - SCALE_HEIGHT(7)),
                                  HLTH_PREV_NEXT_BTN_WIDTH,
                                  BGTILE_HEIGHT);

        quint8 maxVisblePageNumVisible = (totalPages < MAX_CAM_STS_PAGE) ? totalPages : MAX_CAM_STS_PAGE;
        nextButton->resetGeometry((m_PageNumberLabel[maxVisblePageNumVisible-1]->x() + SCALE_WIDTH(60)),
                                  (m_PageNumberLabel[maxVisblePageNumVisible-1]->y() - SCALE_HEIGHT(7)),
                                  HLTH_PREV_NEXT_BTN_WIDTH,
                                  BGTILE_HEIGHT);

        for(quint8 index = 0; index < maxVisblePageNumVisible; index++)
        {
            m_PageNumberLabel[index]->setVisible (true);
            m_PageNumberLabel[index]->setIsEnabled (true);
        }

        for(quint8 index = maxVisblePageNumVisible; index < MAX_CAM_STS_PAGE; index++)
        {
            m_PageNumberLabel[index]->setVisible (false);
            m_PageNumberLabel[index]->setIsEnabled (false);
        }

        if(currCamPage != 0)
        {
            prevButton->setIsEnabled (true);
            m_elementList[m_currElement]->forceActiveFocus();
        }
        else
        {
            if((m_currElement == HLT_NEXT_CAM_BTN) || (m_currElement == HLT_PREV_CAM_BTN))
            {
                m_currElement = HLT_NEXT_CAM_BTN;
                m_elementList[m_currElement]->forceActiveFocus();
            }
            prevButton->setIsEnabled(false);
        }

        if(currCamPage == (totalPages - 1))
        {
            if((m_currElement == HLT_NEXT_CAM_BTN) || (m_currElement == HLT_PREV_CAM_BTN))
            {
                m_currElement = HLT_PREV_CAM_BTN;
                m_elementList[m_currElement]->forceActiveFocus();
            }
            nextButton->setIsEnabled(false);
        }
        else
        {
            nextButton->setIsEnabled(true);
            m_elementList[m_currElement]->forceActiveFocus();
        }
    }

    update();
}

void HealthStatus::showSystemStausTab()
{
    quint8 offset = 0 ,maxrow = 5;
    nextButton->setIsEnabled (false);
    prevButton->setIsEnabled (false);
    nextButton->setVisible (false);
    prevButton->setVisible (false);

    for(quint8 index = 0; index < MAX_CAM_STS_PAGE; index++)
    {
        m_PageNumberLabel[index]->setVisible (false);
        m_PageNumberLabel[index]->setIsEnabled (false);
    }

    for(quint8 row = 0; row < 5; row++)
    {
        if((devTable.alarms == 0) && (devTable.sensors == 0))
        {
            if(row == 0 || row == 1)
            {
                offset = 2;
                continue;
            }
        }
        camFieldArray[row - offset]->setVisible (true);
        camFieldArray[row -offset]->changeLabelText (HlthStsStr[MAX_HLT_TOTAL_CAM_STS + row]);

    }

    if((devTable.alarms == 0) && (devTable.sensors == 0))
    {
        camFieldArray[3]->setVisible (false);
        camFieldArray[3]->changeLabelText ("");
        camFieldArray[4]->setVisible (false);
        camFieldArray[4]->changeLabelText ("");
    }

    if(responseStatus)
    {
        if((devTable.alarms == 0) && (devTable.sensors == 0))
        {
             offset = 2;
             maxrow = 3;
        }

        for(quint8 row = 0; row < maxrow; row++)
        {
            for(quint8 col = 0; col < bgStatusTileIndex[row + offset]; col++)
            {
                statusBgRect1[row][col]->changeInnerRectColor (statusColor[colorState[row + MAX_HLT_TOTAL_CAM_STS + offset] [col]]);
                statusBgRect1[row][col]->setVisible (true);
            }

            for(quint8 col = bgStatusTileIndex[row+offset]; col < HLT_MAX_CAM_COL; col++)
            {
                statusBgRect1[row][col]->setVisible (false);
            }

            if(row == 0)
            {
                for(quint8 index = 0; index < bgStatusTileIndex[row]; index++)
                {
                    camNoText[index]->changeText (QString("%1").arg (1 + index));
                    camNoText[index]->setVisible (true);
                }

                for(quint8 col = bgStatusTileIndex[row]; col < HLT_MAX_CAM_COL; col++)
                {
                    camNoText[col]->setVisible (false);
                }
            }
        }

        if((devTable.alarms == 0) && (devTable.sensors == 0))
        {
             for(quint8 row = maxrow; row < 5; row++)
             {
                 for(quint8 col = 0; col < HLT_MAX_CAM_COL; col++)
                 {
                     statusBgRect1[row][col]->setVisible (false);
                 }
             }
        }
    }
    else
    {
        for(quint8 row = 0; row < 5; row++)
        {
            for(quint8 col = 0; col < bgStatusTileIndex[row]; col++)
            {
                statusBgRect1[row][col]->setVisible (false);
            }

            for(quint8 col = bgStatusTileIndex[row]; col < HLT_MAX_CAM_COL; col++)
            {
                statusBgRect1[row][col]->setVisible (false);
            }

            if(row == 0)
            {
                for(quint8 index = 0; index < bgStatusTileIndex[row]; index++)
                {
                    camNoText[index]->setVisible (false);
                }

                for(quint8 col = bgStatusTileIndex[row]; col < HLT_MAX_CAM_COL; col++)
                {
                    camNoText[col]->setVisible (false);
                }
            }
        }
    }

    for(quint8 row = 5; row < HLT_MAX_FIELD_ROW; row++)
    {
        camFieldArray[row]->setVisible (false);

        for(quint8 col = 0; col < HLT_MAX_CAM_COL; col++)
        {
            statusBgRect1[row][col]->setVisible (false);
        }
    }
    update ();
}

void HealthStatus::showHlthStatus(QString devName, MENU_TAB_e tabToShow)
{
    currCamPage = totalPages = 0;
    selectedTabIndex = tabToShow;

    switch (tabToShow)
    {
        case CAMERA_TAB:
            cameraTab->setShowClickedImage (true);
            cameraEventsTab->setShowClickedImage (false);
            systemTab->setShowClickedImage (false);
            break;

        case CAMERA_EVENTS_TAB:
            cameraTab->setShowClickedImage (false);
            cameraEventsTab->setShowClickedImage (true);
            systemTab->setShowClickedImage (false);
            break;

        case SYSTEM_TAB:
            cameraTab->setShowClickedImage (false);
            cameraEventsTab->setShowClickedImage (false);
            systemTab->setShowClickedImage (true);
            break;

        default:
            break;
    }

    deviceDropDown->setCurrValue(applControl->GetDispDeviceName(devName));
    devNameChanged(devName);
}

void HealthStatus::setDefaultFocus()
{
    m_currElement = HLT_SPIN_BOX;

    if(!(infoPage->isVisible ()))
    {
        m_elementList[m_currElement]->forceActiveFocus ();
    }
}

void HealthStatus::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName == currDevName) && (param->msgType == MSG_SET_CMD))
    {
        if(param->cmdType == HEALTH_STS)
        {
            switch(param->deviceStatus)
            {
                case CMD_SUCCESS:
                    applControl->UpdateHlthStatusAll (currDevName,param->payload);
                    getHlthStatusFrmDev(currDevName);
                    break;

                default:
                    responseStatus = false;
                    break;
            }

            switch (selectedTabIndex)
            {
                case CAMERA_TAB:
                case CAMERA_EVENTS_TAB:
                {
                    quint8 temp = m_currElement;
                    showCamStatus (selectedTabIndex);
                    m_currElement = temp;
                    m_elementList[m_currElement]->forceActiveFocus();
                }
                break;

                case SYSTEM_TAB:
                    showSystemStausTab ();
                    break;

                default:
                    break;
            }

            processBar->unloadProcessBar ();
        }
    }
}

void HealthStatus::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);   
    if(!(infoPage->isVisible ()))
    {
        m_elementList[m_currElement]->forceActiveFocus ();
    }
}

void HealthStatus::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void HealthStatus::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void HealthStatus::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void HealthStatus::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_F5:
            devNameChanged(currDevName);
            break;

        default:
            break;
    }
}

void HealthStatus::escKeyPressed (QKeyEvent *event)
{
    event->accept();
    m_currElement = HLT_CLOSE_BUTTON;

    if((m_currElement < MAX_HLT_CONTROL) && (IS_VALID_OBJ(m_elementList[m_currElement])))
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void HealthStatus::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_HLT_CONTROL) % MAX_HLT_CONTROL;

    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void HealthStatus::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_HLT_CONTROL;

    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void HealthStatus::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void HealthStatus::slotStatusTileMouseHover (int tileIndex, bool hoverState)
{
    quint8 col, row, actualRow = 0;
    if(tileIndex == -1)
    {
        return;
    }

    if(hoverState == false)
    {
        toolTip->setVisible (false);
        return;
    }

    row = (tileIndex / HLT_MAX_CAM_COL);
    col = (tileIndex % HLT_MAX_CAM_COL);

    switch (selectedTabIndex)
    {
        case CAMERA_TAB:
            actualRow = row;
            break;

        case CAMERA_EVENTS_TAB:
            actualRow = row + MAX_HLT_CAM_STS;
            break;

        case SYSTEM_TAB:
            actualRow = row + MAX_HLT_TOTAL_CAM_STS;
            break;

        default:
            break;
    }

    if((selectedTabIndex == SYSTEM_TAB) && (devTable.alarms == 0) && (devTable.sensors == 0))
    {
        actualRow = actualRow +2;
    }

    if(tooltipStateStr[tooltipState[actualRow][(currCamPage * HLT_MAX_CAM_COL) + col]] != "")
    {
        toolTip->textChange (tooltipStateStr[tooltipState[actualRow][(currCamPage * HLT_MAX_CAM_COL) + col]]);
        toolTip->setVisible (true);
        toolTip->resetGeometry ((statusBgRect1[row][col]->x () + (statusBgRect1[row][col]->width () / 2)),
                                (statusBgRect1[row][col]->y () - (statusBgRect1[row][col]->height () / 2)));
    }
}

void HealthStatus::slotSpinBoxValueChange(QString newName, quint32)
{
    newName = applControl->GetRealDeviceName(newName);
    if(newName != currDevName)
    {
        currCamPage = 0;
        nextPageSelected = false;
        // update all component
        devNameChanged (newName);
    }
}

void HealthStatus::slotRefreshButtonClick (int)
{
    // update only status param
    devNameChanged (currDevName);
}

void HealthStatus::slotNextPageClick (int)
{
    // update all component
    emit sigNextPage(currDevName, selectedTabIndex);
}

void HealthStatus::slotInfoPageCnfgBtnClick(int)
{
    m_currElement = HLT_SPIN_BOX;
    m_elementList[m_currElement]->forceActiveFocus ();
}

void HealthStatus::slotPrevNextCameraClicked (int index)
{
    if(index == HLT_NEXT_CAM_BTN)
    {
        totalPages = ((devTable.totalCams % HLT_MAX_CAM_COL) == 0) ? (devTable.totalCams / HLT_MAX_CAM_COL) : (devTable.totalCams / HLT_MAX_CAM_COL) + 1;

        if (currCamPage != (totalPages))
        {
            currCamPage ++;
        }
        nextPageSelected = true;
    }
    else
    {
        if(currCamPage > 0)
        {
            currCamPage --;
        }
        nextPageSelected = false;
    }

    showCamStatus (selectedTabIndex);
}

void HealthStatus::slotTabSelected (int index)
{
    selectedTabIndex = (MENU_TAB_e)index;

    switch (index)
    {
        case 0:
            currCamPage = 0;
            cameraEventsTab->setShowClickedImage (false);
            systemTab->setShowClickedImage (false);
            showCamStatus (CAMERA_TAB);
            break;

        case 1:
            currCamPage = 0;
            cameraTab->setShowClickedImage (false);
            systemTab->setShowClickedImage (false);
            showCamStatus (CAMERA_EVENTS_TAB);
            break;

        case 2:
            currCamPage = 0;
            cameraTab->setShowClickedImage (false);
            cameraEventsTab->setShowClickedImage (false);
            showSystemStausTab();
            break;

        default:
            break;
    }
}

void HealthStatus::slotPageNumberButtonClick (QString str)
{
    quint8 tempPageNum = ((quint8)str.toUInt() - 1);

    nextPageSelected = (tempPageNum < currCamPage) ? false : true;
    currCamPage = tempPageNum;
    showCamStatus (selectedTabIndex);
}

void HealthStatus::slotDropDownListDestroyed()
{
    if((infoPage->isVisible ()))
    {
        infoPage->forceActiveFocus();
    }
}

void HealthStatus::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    applControl->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (deviceDropDown->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            deviceDropDown->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* If selected device is local device then it will update the device name only otherwise it will clear the data and will select the local device */
    deviceDropDown->setNewList(deviceMapList, 0);
    slotSpinBoxValueChange(deviceMapList.value(0), HLT_SPIN_BOX);
}
