#include "MxQuickBackup.h"
#include <QKeyEvent>
#include "ValidationMessage.h"
#include "Controls/MessageBanner.h"


#define QUICK_BACKUP_HEADING                   "Quick Backup"
#define QUICK_BACKUP_MAIN_RECT_WIDTH           SCALE_WIDTH(1020)
#define QUICK_BACKUP_MAIN_RECT_HEIGHT          SCALE_HEIGHT(720)
#define QUICK_BACKUP_HEADER_RECT_WIDTH         SCALE_WIDTH(310)
#define QUICK_BACKUP_HEADER_RECT_HEIGHT        SCALE_HEIGHT(44)

#define MAX_SEEN_CAMERAS                6
#define SCROLLBAR_WIDTH                 SCALE_WIDTH(13)
#define OTHER_ELEMENT_HEIGHT            SCALE_HEIGHT(25)
#define CONTROL_WIDTH                   SCALE_WIDTH(295)
#define INVALID_FEILD                   255
#define LEFT_MARGIN_FROM_CENTER         SCALE_WIDTH(80)

#define QUICK_BACKUP_DISABLE_IMG_PATH ":/Images_Nvrx/InfoPageImages/QuickBkp.png"

static QString clipSettingsImagePath[] = {":/Images_Nvrx/UpArrow/",
                                   ":/Images_Nvrx/DownArrow/"};

static QString minimizeButtonPath[] = {":/Images_Nvrx/MinimizeButton/"};

static QString statusStr[MAX_STATUS+1] ={"In Progress",
                                  "Success",
                                  "Failed"
                                  ""            };



typedef enum
{
    UP_ARROW,
    DOWN_ARROW,

    MAX_SOURCE_TYPE
}SOURCE_TYPE_e;

const static QString labelStr[MAX_QUICK_BACKUP_ELEMENTS] =
{
    "Start",
    "View Status",
    "Clip Settings",
    "Hide Clip Settings",
    "Device",
    "Camera",
    "Backup Location",
    "File Format",
};

static const QStringList quickBackupLocations = QStringList ()
        << "USB Device"
        << "Network Drive 1"
        << "Network Drive 2";

static QStringList hourTypeString;

MxQuickBackup::MxQuickBackup(QWidget * parent)
    :BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - QUICK_BACKUP_MAIN_RECT_WIDTH) / 2)),
                (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - QUICK_BACKUP_MAIN_RECT_HEIGHT) / 2)),
                QUICK_BACKUP_MAIN_RECT_WIDTH,
                (QUICK_BACKUP_MAIN_RECT_HEIGHT/2) + SCALE_HEIGHT(10),
                BACKGROUND_TYPE_1,
                QUICK_BACKUP,
                parent,
                QUICK_BACKUP_HEADER_RECT_WIDTH,
                QUICK_BACKUP_HEADER_RECT_HEIGHT,
                QUICK_BACKUP_HEADING)
{

        hourTypeString = QStringList()
                << QString("1 ") + Multilang("Hour")
                << QString("6 ") + Multilang("Hour")
                << QString("12 ")+ Multilang("Hour")
                << QString("24 ")+ Multilang("Hour")
                << QString("48 ")+ Multilang("Hour")
                << QString("72 ")+ Multilang("Hour");
    applcontroller = ApplController::getInstance();
    INIT_OBJ(payloadLib);
    payloadLib = new PayloadLib();
    for(quint8 index = 0; index < MAXIMUM_REC_ALLOWED; index++)
    {
        arrIndexofrecData[index] = 0;
    }
    for(quint8 index = 0; index < (MAX_CAMERAS+1); index++)
    {
        recSelect[index] = 0;
    }
    tempCamNumber = 0;
    totalCamCount = 0;
    backupLocationLabel = NULL;
    cameraListLabel = NULL;
    clipSettingsBgTileLabel = NULL;
    clipSettingsImage = NULL;
    currBckpLocationIndex = 0;
    deviceDropDownBoxLabel = NULL;
    disabledImage = NULL;
    fileFormatLabel = NULL;
    m_advanceOptionLable = NULL;
    m_inVisibleWidget = NULL;
    isBackupReqSent = false;
    forQuickbackup = false;
    isShowClipSettings = false;
    m_firstCameraIndex = 0;
    m_lastCameraIndex = 0;
    m_Scrollbar = NULL;
    clipSettingsTile = NULL;
    complRect = NULL;
    startButton = NULL;
    backupStatusLabel =NULL;
    m_timeLineSlider = NULL;
    m_sliderTileToolTip = NULL;
    m_sliderToolTip = NULL;
    m_isminimize = false;
    m_isBackupOn = false;
    totalRec = 0;
    tempRecFound = 0;
    searchRecIndex = 0;
    isbackupcompleted = false;
    success = 0;
    fail = 0;
    currCamNo = 0;
    m_isDeviceConnected = true;
    fileFormatTextboxParam = NULL;
    fileFormatTextBox = NULL;

    for(quint8 index = 0; index < MAX_QUICK_BACKUP_ELEMENTS; index++)
    {
        m_elementList[index] = NULL;
    }

    createDefaultComponent ();
    slotSpinboxValueChanged (devNameDropDownBox->getCurrValue(), QUICK_BACKUP_DEVICE_SPINBOX);

    m_currElement = QUICK_BACKUP_START_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
}

MxQuickBackup::~MxQuickBackup()
{
    if(IS_VALID_OBJ(m_sliderToolTip))
    {
        disconnect(m_timeLineSlider,
                SIGNAL(sigSliderToolTipUpdate(QString,int,int)),
                this,
                SLOT(slotSliderToolTipUpdate(QString,int,int)));
        disconnect(m_timeLineSlider,
                SIGNAL(sigSliderToolTipHide(bool)),
                this,
                SLOT(slotSliderToolTipHide(bool)));
        DELETE_OBJ(m_sliderToolTip)
    }

    if(IS_VALID_OBJ(m_sliderTileToolTip))
    {
        disconnect(m_timeLineSlider,
                SIGNAL(sigTileToolTipUpdate(QString,int,int)),
                this,
                SLOT(slotTileToolTipUpdate(QString,int,int)));
        disconnect(m_timeLineSlider,
                SIGNAL(sigTileToolTipHide(bool)),
                this,
                SLOT(slotTileToolTipHide(bool)));
        DELETE_OBJ(m_sliderTileToolTip)
    }

    if(IS_VALID_OBJ(m_timeLineSlider))
    {
        DELETE_OBJ(m_timeLineSlider);
    }

    DELETE_OBJ(clipSettingsTile);
    DELETE_OBJ(cameraSelBgTile);
    DELETE_OBJ(cameraSelBgTitle);
    DELETE_OBJ(backupStatusLabel);
    cameraList.clear();

    if(IS_VALID_OBJ(m_mainCloseButton))
    {
        disconnect(m_mainCloseButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
     }

    if(IS_VALID_OBJ(devNameDropDownBox))
    {
        disconnect(devNameDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect (devNameDropDownBox,
                 SIGNAL(sigValueChanged(QString,quint32)),
                 this,
                 SLOT(slotSpinboxValueChanged(QString,quint32)));
        DELETE_OBJ(devNameDropDownBox);
    }

    if(IS_VALID_OBJ(backupLocationDropDownBox))
    {
        disconnect(backupLocationDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect (backupLocationDropDownBox,
                 SIGNAL(sigValueChanged(QString,quint32)),
                 this,
                 SLOT(slotSpinboxValueChanged(QString,quint32)));
        DELETE_OBJ(backupLocationDropDownBox);
    }

    DELETE_OBJ(fileFormatTextBox);
    DELETE_OBJ(fileFormatTextboxParam);

    for(quint8 index = 0; index < (MAX_CAMERAS + 1); index++)
    {
        if(IS_VALID_OBJ(m_cameraCheckbox[index]))
        {
            disconnect(m_cameraCheckbox[index],
                      SIGNAL(sigUpdateCurrentElement(int)),
                      this,
                      SLOT(slotUpdateCurrentElement(int)));
            disconnect(m_cameraCheckbox[index],
                      SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                      this,
                      SLOT(slotCameraCheckboxClicked(OPTION_STATE_TYPE_e,int)));

            DELETE_OBJ(m_cameraCheckbox[index]);
        }

    }

    if(IS_VALID_OBJ(m_advanceOptionButton))
    {
        disconnect(m_advanceOptionButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_advanceOptionButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotAdvanceButtonClicked(int)));

        m_advanceOptionButton->deleteLater();
    }

    if(IS_VALID_OBJ(startButton))
    {
        disconnect(startButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect (startButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));
        DELETE_OBJ(startButton);
    }

    if(IS_VALID_OBJ(viewStatusButton))
    {
        disconnect(viewStatusButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect (viewStatusButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));
        DELETE_OBJ(viewStatusButton);
    }

    if (m_Scrollbar != NULL)
    {
        disconnect(m_Scrollbar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScrollbarClick(int)));
        DELETE_OBJ(m_Scrollbar);
    }

    if(IS_VALID_OBJ(m_quickBcpTable))
    {
        disconnect (m_quickBcpTable,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    this,
                    SLOT(slotQuickBkupRptClose(TOOLBAR_BUTTON_TYPE_e )));

        DELETE_OBJ (m_quickBcpTable);
    }

    if(IS_VALID_OBJ(m_HourDropDownBox))
    {
        disconnect(m_HourDropDownBox,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotSpinboxValueChanged(QString,quint32)));

        disconnect(m_HourDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ (m_HourDropDownBox);
    }
     DELETE_OBJ(complRect);
     DELETE_OBJ(payloadLib);

     if(IS_VALID_OBJ(minimizeButtonImage))
     {
        disconnect(minimizeButtonImage,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotImageClicked(int)));
        DELETE_OBJ (minimizeButtonImage);
     }
}

void MxQuickBackup::createDefaultComponent ()
{
    m_elementList[QUICK_BACKUP_CLOSE_BTN] = m_mainCloseButton;
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    connect(m_mainCloseButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseBtnClick(int)));


    complRect = new BgTile(SCALE_WIDTH(20),
                           QUICK_BACKUP_HEADER_RECT_HEIGHT + SCALE_HEIGHT(25),
                           this->width()-SCALE_WIDTH(50),
                           this->height()-QUICK_BACKUP_HEADER_RECT_HEIGHT-SCALE_HEIGHT(45),
                           COMMON_LAYER,
                           this);


    for(quint8 index = 0; index < MAX_HOUR_FORMAT; index++)
    {
        modeList.insert ((index),hourTypeString[index]);
    }

    INIT_OBJ(m_HourDropDownBox);
    m_HourDropDownBox = new DropDown(complRect->x () + SCALE_WIDTH(10),
                                     complRect->y () + SCALE_HEIGHT(10),
                                     BGTILE_LARGE_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     QUICK_BACKUP_HOURS_DROPDOWN,
                                     DROPDOWNBOX_SIZE_200,
                                     "Hours",
                                     modeList,
                                     this,
                                     "",
                                     false,
                                     SCALE_WIDTH(10),
                                     NO_LAYER);
    m_elementList[QUICK_BACKUP_HOURS_DROPDOWN] = m_HourDropDownBox;

    if(IS_VALID_OBJ(m_HourDropDownBox))
    {
        connect(m_HourDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChanged(QString,quint32)));

        connect(m_HourDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    clipSettingsTile = new TileWithText(SCALE_WIDTH(30),
                                        (QUICK_BACKUP_MAIN_RECT_HEIGHT/2)-SCALE_HEIGHT(20),
                                        complRect->width()-SCALE_WIDTH(20),
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(5),
                                        labelStr[QUICK_BACKUP_SHOW_CLIP_SETTNGS_TILE],
                                        COMMON_LAYER,
                                        this,
                                        true,
                                        0,
                                        "#c8c8c8",
                                        QUICK_BACKUP_SHOW_CLIP_SETTNGS_TILE);

     m_elementList[QUICK_BACKUP_SHOW_CLIP_SETTNGS_TILE] = NULL;

     m_elementList[QUICK_BACKUP_HIDE_CLIP_SETTNGS_TILE] = NULL;

    startButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 this->width()/2 - SCALE_WIDTH(10),
                                 complRect->height() - SCALE_HEIGHT(10),
                                 labelStr[QUICK_BACKUP_START_BTN],
                                 this,
                                 QUICK_BACKUP_START_BTN,
                                 true);

    m_elementList[QUICK_BACKUP_START_BTN] = startButton;

    connect(startButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(startButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    viewStatusButton = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                      startButton->x()+startButton->width()+(startButton->width()/2)+ SCALE_WIDTH(35),
                                      complRect->height() - SCALE_HEIGHT(10),
                                      labelStr[QUICK_BACKUP_VIEW_STATUS_BTN],
                                      this,
                                      QUICK_BACKUP_VIEW_STATUS_BTN,
                                      true);

    m_elementList[QUICK_BACKUP_VIEW_STATUS_BTN] = viewStatusButton;

    viewStatusButton->hide();
    viewStatusButton->setEnabled(false);
    viewStatusButton->setIsEnabled(false);
    connect(viewStatusButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(viewStatusButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    INIT_OBJ(m_advanceOptionButton);
    /* PARASOFT: Memory Deallocated in destructor */
    m_advanceOptionButton = new PageOpenButton(complRect->width() - SCALE_HEIGHT(240),
                                               complRect->y () + SCALE_HEIGHT(10),
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               QUICK_BACKUP_ADVANCE_BUTTON,
                                               PAGEOPENBUTTON_ULTRALARGE,
                                               "Advanced Options",
                                               this,
                                               "", "",
                                               false,
                                               0,
                                               NO_LAYER);


    if(IS_VALID_OBJ(m_advanceOptionButton))
    {
        m_elementList[QUICK_BACKUP_ADVANCE_BUTTON] = m_advanceOptionButton;

        connect(m_advanceOptionButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_advanceOptionButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotAdvanceButtonClicked(int)));
    }


    backupStatusLabel = new TextLabel((complRect->width() / 2) - SCALE_WIDTH(45),
                                      complRect->y() + SCALE_HEIGHT(7),
                                      NORMAL_FONT_SIZE,
                                      "",
                                      this,
                                      QUICK_BACKUP_STATUS_COLOR);
     INIT_OBJ(minimizeButtonImage);
     minimizeButtonImage = new Image(m_mainCloseButton->x(),
                                     (QUICK_BACKUP_HEADER_RECT_HEIGHT / 2),
                                     minimizeButtonPath[0],
                                     this,
                                     END_X_CENTER_Y,
                                     QUICK_BACKUP_MINIMIZE_BTN,
                                     true,
                                     false,
                                     true,
                                     false);

     m_elementList[QUICK_BACKUP_MINIMIZE_BTN] = minimizeButtonImage;

     connect(minimizeButtonImage,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotImageClicked(int)));

    QMap<quint8, QString> deviceMapList;
    applcontroller->GetDevNameDropdownMapList(deviceMapList);
    m_currDevName = applcontroller->GetRealDeviceName(deviceMapList.value(0));

    devNameDropDownBox = new DropDown((QUICK_BACKUP_MAIN_RECT_WIDTH/4 +SCALE_WIDTH(40)),
                                      clipSettingsTile->y()+clipSettingsTile->height()+SCALE_HEIGHT(15),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      QUICK_BACKUP_DEVICE_SPINBOX,
                                      DROPDOWNBOX_SIZE_200,
                                      labelStr[QUICK_BACKUP_DEVICE_SPINBOX],
                                      deviceMapList, this, "", true,
                                      0, COMMON_LAYER, true, 8, false,
                                      false, 5, LEFT_MARGIN_FROM_CENTER);
    m_elementList[QUICK_BACKUP_DEVICE_SPINBOX] = devNameDropDownBox;

    connect(devNameDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (devNameDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));
    devNameDropDownBox->setVisible(false);
    devNameDropDownBox->setIsEnabled(false);

   m_isDeviceConnected = applcontroller->GetAllCameraNamesOfDevice(m_currDevName,cameraList,PLAYBACK_CAM_LIST_TYPE,&m_cameraIndex[0]);
    //Memory Deallocate in destructor
    cameraSelBgTile = new TileWithText(devNameDropDownBox->x(),
                                        devNameDropDownBox->y() + devNameDropDownBox->height()+SCALE_HEIGHT(10),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT * 5,
                                        SCALE_WIDTH(90),
                                        "",
                                        COMMON_LAYER,
                                        this,
                                        true
                                        );
    int textWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY,
                                                    NORMAL_FONT_SIZE)).width(labelStr[QUICK_BACKUP_CAM_LIST]);
    cameraSelBgTitle = new TextLabel(cameraSelBgTile->x()+SCALE_WIDTH(87),
                                     cameraSelBgTile->y()+SCALE_HEIGHT(17),
                                     NORMAL_FONT_SIZE,
                                     labelStr[QUICK_BACKUP_CAM_LIST],
                                     this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                     0, 0, textWidth);

    m_elementList[QUICK_BACKUP_CAM_LIST] = NULL;

    for(quint8 index = 0; index < (MAX_CAMERAS + 1); index++)
    {
        QString labelString;
        quint16 tileWidth = (index == 0 ? CONTROL_WIDTH : (CONTROL_WIDTH - SCROLLBAR_WIDTH));
        if(index-1 < cameraList.length())
        {
             labelString = (index == 0 ? "Select All" :cameraList.at(index-1));
        }
        else
        {
             labelString = (index == 0 ? "Select All" :" ");
        }
        m_cameraCheckbox[index] = new OptionSelectButton(cameraSelBgTitle->x()+cameraSelBgTitle->width()+SCALE_WIDTH(12),
                                                         (cameraSelBgTitle->y() + (index * OTHER_ELEMENT_HEIGHT)-SCALE_HEIGHT(5)),
                                                         tileWidth,
                                                         OTHER_ELEMENT_HEIGHT,
                                                         CHECK_BUTTON_INDEX,
                                                         labelString,
                                                         this,
                                                         COMMON_LAYER,
                                                         10,
                                                         MX_OPTION_TEXT_TYPE_SUFFIX,
                                                         NORMAL_FONT_SIZE,
                                                         (QUICK_BACKUP_CAMERASELECT_CHECKBOX + index),
                                                         true,
                                                         NORMAL_FONT_COLOR);

        m_elementList[QUICK_BACKUP_CAMERASELECT_CHECKBOX + index] = m_cameraCheckbox[index];

        if(index != 0)
        {
            m_cameraCheckbox[index]->setVisible(false);
        }
        connect(m_cameraCheckbox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_cameraCheckbox[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCameraCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    }

    m_cameraCheckbox[0]->changeState(ON_STATE);
    changeCameraElement();

    QMap<quint8, QString>  backupLocationList;

    for(quint8 index = 0; index < quickBackupLocations.length (); index++)
    {
        backupLocationList.insert (index,quickBackupLocations.at (index));
    }

     backupLocationDropDownBox = new DropDown(cameraSelBgTile->x(),
                                              cameraSelBgTile->y () + cameraSelBgTile->height()+SCALE_HEIGHT(10),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              QUICK_BACKUP_BCKP_LOCATION,
                                              DROPDOWNBOX_SIZE_200,
                                              labelStr[QUICK_BACKUP_BCKP_LOCATION - MAX_CAMERAS -2],
                                              backupLocationList,this,"",true, 0,
                                              COMMON_LAYER, true, 8, false, false,
                                              5, LEFT_MARGIN_FROM_CENTER);

    m_elementList[QUICK_BACKUP_BCKP_LOCATION] = backupLocationDropDownBox;
    backupLocationDropDownBox->setIsEnabled(false);

    connect(backupLocationDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (backupLocationDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    /* file format texbox parameters */
    fileFormatTextboxParam = new TextboxParam();
    fileFormatTextboxParam->textStr = "Avi";
    fileFormatTextboxParam->labelStr = labelStr[QUICK_BACKUP_FILE_FORMAT - MAX_CAMERAS - 2];
    fileFormatTextboxParam->maxChar = 3;

    /* file format text box with text Avi */
    fileFormatTextBox = new TextBox(backupLocationDropDownBox->x(),
                                    (backupLocationDropDownBox->y() + backupLocationDropDownBox->height() + SCALE_HEIGHT(10)),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    QUICK_BACKUP_FILE_FORMAT,
                                    TEXTBOX_EXTRASMALL,
                                    this,
                                    fileFormatTextboxParam,
                                    COMMON_LAYER,
                                    false, false, false,
                                    LEFT_MARGIN_FROM_CENTER);

    m_elementList[QUICK_BACKUP_FILE_FORMAT] = fileFormatTextBox;

    m_timeLineSlider = new MxTimelineSlider(m_HourDropDownBox->x() ,
                                            m_HourDropDownBox->y() + SCALE_HEIGHT(25),
                                            QUICK_BACKUP_TIMELINE_SLIDER,
                                            this);
    if(IS_VALID_OBJ(m_timeLineSlider))
    {
        m_timeLineSlider->PrivateVarAccess(&m_elementList[QUICK_BACKUP_TIMELINE_PREV],QUICK_BACKUP_PREV_BUTTON);
        m_timeLineSlider->PrivateVarAccess(&m_elementList[QUICK_BACKUP_TIMELINE_NEXT],QUICK_BACKUP_NEXT_BUTTON);
    }

    m_sliderTileToolTip = new ToolTip(complRect->x() + SCALE_WIDTH(50),
                                      complRect->y() + SCALE_HEIGHT(40),
                                      "",
                                      this,
                                      CENTER_X_START_Y);
    m_sliderTileToolTip->setVisible(false);

    connect(m_timeLineSlider,
            SIGNAL(sigTileToolTipUpdate(QString,int,int)),
            this,
            SLOT(slotTileToolTipUpdate(QString,int,int)));
    connect(m_timeLineSlider,
            SIGNAL(sigTileToolTipHide(bool)),
            this,
            SLOT(slotTileToolTipHide(bool)));

    m_sliderToolTip = new ToolTip(m_timeLineSlider->x() + SCALE_WIDTH(5),
                                  m_timeLineSlider->y() + SCALE_HEIGHT(44),
                                  "",
                                  this,
                                  CENTER_X_START_Y);
    m_sliderToolTip->setVisible(false);

    connect(m_timeLineSlider,
            SIGNAL(sigSliderToolTipUpdate(QString,int,int)),
            this,
            SLOT(slotSliderToolTipUpdate(QString,int,int)));
    connect(m_timeLineSlider,
            SIGNAL(sigSliderToolTipHide(bool)),
            this,
            SLOT(slotSliderToolTipHide(bool)));

    m_quickBcpTable = new MxQuickBckupReportTable(this);
    m_quickBcpTable->hide();
    connect(m_quickBcpTable ,
             SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
             this,
             SLOT(slotQuickBkupRptClose(TOOLBAR_BUTTON_TYPE_e)));

     m_elementList[QUICK_BACKUP_SCROLLBAR] = NULL;

     showHideElements(true);
}


void MxQuickBackup::changeCameraElement()
{
    if(m_Scrollbar != NULL)
    {
        disconnect(m_Scrollbar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScrollbarClick(int)));
        delete m_Scrollbar;
        m_Scrollbar = NULL;

    }

    m_firstCameraIndex = 0;
    m_lastCameraIndex = ((cameraList.length() > MAX_SEEN_CAMERAS) ? (MAX_SEEN_CAMERAS - 1) : (cameraList.length() - 1));

    // for network device which disconnected
    bool isControlActive = (!cameraList.empty ());

    m_cameraCheckbox[0]->setVisible(true);
    m_cameraCheckbox[0]->setIsEnabled(isControlActive);


    if(m_isDeviceConnected == false)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
    }
    else if(cameraList.isEmpty() && ((applcontroller->getUserRightsResponseNotify ())))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
    }
    resetGeometryForCameraOption();

    for(quint8 index = 0; index < (MAX_CAMERAS + 1); index++)
    {
        if(index > cameraList.length())
        {
            m_cameraCheckbox[index]->changeState(OFF_STATE);
        }
        else
        {
            m_cameraCheckbox[index]->changeState(ON_STATE);
        }
    }

    if(cameraList.length() == 0)
    {
        m_cameraCheckbox[0]->changeState(OFF_STATE);
    }
    else
    {
        m_Scrollbar = new ScrollBar((m_cameraCheckbox[1]->x() + m_cameraCheckbox[1]->width()),
                                    m_cameraCheckbox[1]->y(),
                                    SCROLLBAR_WIDTH,
                                    MAX_SEEN_CAMERAS,
                                    OTHER_ELEMENT_HEIGHT,
                                    cameraList.length(),
                                    m_firstCameraIndex,
                                    this,
                                    VERTICAL_SCROLLBAR,
                                    QUICK_BACKUP_SCROLLBAR,
                                    (cameraList.length() > MAX_SEEN_CAMERAS),
                                    false);
        connect(m_Scrollbar,
                SIGNAL(sigScroll(int)),
                this,
                SLOT(slotScrollbarClick(int)));
    }
}

void MxQuickBackup::resetGeometryForCameraOption()
{
    for(quint8 index = 0; index < MAX_CAMERAS; index++)
    {
        m_cameraCheckbox[index + 1]->resetGeometry(cameraSelBgTitle->x()+cameraSelBgTitle->width()+SCALE_WIDTH(12),
                                                   (cameraSelBgTitle->y() + ((index+1) * OTHER_ELEMENT_HEIGHT)-SCALE_HEIGHT(5)),
                                                   (CONTROL_WIDTH - SCROLLBAR_WIDTH),
                                                   OTHER_ELEMENT_HEIGHT);


        if((index >= m_firstCameraIndex)
                && (index <= m_lastCameraIndex)
                && (!cameraList.empty ()))
        {
            m_cameraCheckbox[index + 1]->resetGeometry(cameraSelBgTitle->x()+cameraSelBgTitle->width()+SCALE_WIDTH(12),
                                                       (cameraSelBgTitle->y() + ((index + 1 - m_firstCameraIndex) * OTHER_ELEMENT_HEIGHT)-SCALE_HEIGHT(5)),
                                                       (CONTROL_WIDTH - SCROLLBAR_WIDTH),
                                                       OTHER_ELEMENT_HEIGHT);

            m_cameraCheckbox[(index + 1)]->setVisible(true);
            m_cameraCheckbox[(index + 1)]->setIsEnabled(true);
            m_cameraCheckbox[index + 1]->changeLabel (MX_OPTION_TEXT_TYPE_SUFFIX,QString("%1").arg(m_cameraIndex[index]) + ": " + cameraList.at(index));

        }
        else
        {
            m_cameraCheckbox[(index + 1)]->setVisible(false);
            m_cameraCheckbox[(index + 1)]->setIsEnabled(false);
            m_cameraCheckbox[index + 1]->changeLabel (MX_OPTION_TEXT_TYPE_SUFFIX,"");
        }
    }
}

void MxQuickBackup::selectDeselectAllRecords(OPTION_STATE_TYPE_e state)
{
    for(quint8 index = 1; index < (cameraList.length() + 1); index++)
    {
        m_cameraCheckbox[index]->changeState(state);
    }
}

bool MxQuickBackup::isAllButtonChecked()
{
    bool status = true;
    for(quint8 index = 1; index < (cameraList.length() + 1); index++)
    {
        if(m_cameraCheckbox[index]->getCurrentState() == OFF_STATE)
        {
            status = false;
            break;
        }
    }
    return status;
}

bool MxQuickBackup::getMinimizeFlag()
{
    return m_isminimize;
}

void MxQuickBackup::setMinimizeFlag(bool state)
{
     m_isminimize = state;
}

bool MxQuickBackup::getBackupFlag()
{
    return m_isBackupOn;
}

void MxQuickBackup::setBackupFlag(bool state)
{
     m_isBackupOn = state;
}
bool MxQuickBackup::getQuickBackupFlag()
{
    return m_isBackupOn;
}


void MxQuickBackup::showHideElements(bool flag)
{
    devNameDropDownBox->setVisible(flag);

    devNameDropDownBox->setIsEnabled(flag);
    for(quint8 index = 0; index < (MAX_CAMERAS + 1); index++)
    {
        m_cameraCheckbox[index]->setIsEnabled(flag);
    }
    backupLocationDropDownBox->setIsEnabled(flag);

    if(flag == true)
    {
        isShowClipSettings = flag;
        this->resetGeometry(this->x(),
                            this->y(),
                            QUICK_BACKUP_MAIN_RECT_WIDTH,
                            (QUICK_BACKUP_MAIN_RECT_HEIGHT) + SCALE_HEIGHT(22));

        complRect->resetGeometry(complRect->width(),this->height()-QUICK_BACKUP_HEADER_RECT_HEIGHT-SCALE_HEIGHT(45));
        complRect->update();
		this->update();
    }
    else
    {
        isShowClipSettings = flag;
        this->resetGeometry(this->x(),
                            this->y(),
                            QUICK_BACKUP_MAIN_RECT_WIDTH,
                            this->height()/2-SCALE_HEIGHT(16));

        complRect->resetGeometry(complRect->width(),complRect->height()/2-SCALE_HEIGHT(20));
        complRect->update();
		 this->update();
    }
}

void MxQuickBackup::searchRecData()
{
     if(searchRecIndex <= cameraList.length())
     {
         if(recSelect[searchRecIndex] == true)
            {
                payloadLib->setCnfgArrayAtIndex (0, searchRecIndex);
                payloadLib->setCnfgArrayAtIndex (1, 15);
                payloadLib->setCnfgArrayAtIndex (2, m_startDtTmStr.mid(0,12));
                payloadLib->setCnfgArrayAtIndex (3, m_endDtTmStr.mid(0,12));
                payloadLib->setCnfgArrayAtIndex (4, MAXIMUM_REC_ALLOWED);
                QString payloadString = payloadLib->createDevCmdPayload(5);

                DevCommParam* param = new DevCommParam();
                param->msgType = MSG_SET_CMD;
                param->cmdType = SEARCH_RCD_ALL_DATA;
                param->payload = payloadString;

                applcontroller->processActivity(searchDevName, DEVICE_COMM, param);
           }
           else
           {
              searchRecIndex++;
              searchRecData();
           }
     }
     else
     {
        if(tempRecFound > 0)
        {
            for(quint8 index = 1; index <=  MAX_CAMERAS; index++)
            {
                if(recSelect[index] == true)
                {
                    if(statusReport[index].camNo == INVALID_FEILD)
                    {
                        statusReport[index].camNo = index;
                        statusReport[index].cameraName = cameraList.at(index - 1);
                        statusReport[index].status = statusStr[QB_FAILED];
                        statusReport[index].reason = "No records found.";
                    }
                    else
                    {
                        statusReport[index].camNo = index;
                        statusReport[index].cameraName = cameraList.at(index - 1);
                        statusReport[index].status = statusStr[QB_IN_PROGRESS];
                        statusReport[index].reason = "";
                    }
                }
            }
            fillReportTable();
            fillMultipleRecBackupCmd();
            sendMultipleRecBackupCmd();
        }
        else
        {
            backupStatusLabel->changeText("No records found.");
            backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(55));
            startButton->setEnabled(true);
            m_advanceOptionButton->setIsEnabled(true);
            startButton->changeText(labelStr[QUICK_BACKUP_START_BTN]);
            m_elementList[m_currElement]->forceActiveFocus();
        }
     }
}

void MxQuickBackup::processDeviceResponseforSliderTime(DevCommParam *param, QString deviceName)
{
    if(m_timeLineSlider != NULL)
     {
        m_timeLineSlider->processDeviceResponseforSliderTime(param, deviceName);
     }
}
void MxQuickBackup::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName == searchDevName)
    {
        switch(param->msgType)
        {
        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
            case SEARCH_RCD_ALL_DATA:
                if((param->deviceStatus == CMD_SUCCESS) ||
                        (param->deviceStatus == CMD_MORE_DATA))
                {
                    payloadLib->parseDevCmdReply (false, param->payload);
                    quint32 totalReplyFields = 0;
                    totalReplyFields = payloadLib->getTotalReplyFields();
                    totalRec = (payloadLib->getTotalCmdFields () / totalReplyFields);

                    for(quint8 index = 0; index < totalRec; index++)
                    {
                        tempCamNumber = payloadLib->getCnfgArrayAtIndex (FIELD_CAM_NO + (index *totalReplyFields)).toUInt ();

                        if(tempRecFound < MAXIMUM_REC_ALLOWED)
                        {
                            playbackRecData[tempRecFound].startTime = payloadLib->getCnfgArrayAtIndex (FIELD_STRAT_TIME + (index *totalReplyFields)).toString ();
                            playbackRecData[tempRecFound].endTime = payloadLib->getCnfgArrayAtIndex (FIELD_END_TIME + (index *totalReplyFields)).toString ();
                            playbackRecData[tempRecFound].camNo = tempCamNumber;
                            playbackRecData[tempRecFound].evtType = payloadLib->getCnfgArrayAtIndex (FIELD_EVT_TYPE + (index *totalReplyFields)).toUInt ();
                            playbackRecData[tempRecFound].overlap = payloadLib->getCnfgArrayAtIndex (FIELD_OVERLAP + (index *totalReplyFields)).toUInt ();
                            playbackRecData[tempRecFound].hddIndicator = payloadLib->getCnfgArrayAtIndex (FIELD_HDD + (index *totalReplyFields)).toUInt ();
                            playbackRecData[tempRecFound].partionIndicator = payloadLib->getCnfgArrayAtIndex (FIELD_PARTION + (index *totalReplyFields)).toUInt ();
                            if( totalReplyFields > 8)
                            {
                                playbackRecData[tempRecFound].recDriveIndex = payloadLib->getCnfgArrayAtIndex (FIELD_STORAGE + (index *totalReplyFields)).toUInt ();
                            }
                            else
                            {
                                playbackRecData[tempRecFound].recDriveIndex = 3;
                            }
                            playbackRecData[tempRecFound].deviceName = searchDevName;
                            tempRecFound++;
                        }
                    }
                    statusReport[tempCamNumber].camNo = tempCamNumber;
                    statusReport[tempCamNumber].noofRecords = totalRec;
                    statusReport[tempCamNumber].cameraName = cameraList.at(tempCamNumber - 1);
                }
                if(param->deviceStatus == CMD_INVALID_MESSAGE)
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    backupStatusLabel->changeText("Error while connecting.");
                    backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(55));
                    startButton->setEnabled(true);
                    startButton->changeText(labelStr[QUICK_BACKUP_START_BTN]);
                    m_elementList[m_currElement]->forceActiveFocus();
                    m_advanceOptionButton->setIsEnabled(true);
                }
                else
                {
                    searchRecIndex++;
                    searchRecData();
                }
                break;

            case BKUP_RCD:
                if(param->deviceStatus != CMD_SUCCESS)
                {
                    if((param->deviceStatus == CMD_NO_DISK_FOUND) && (forQuickbackup == true))
                    {
                       startButton->setEnabled(true);
                       m_advanceOptionButton->setIsEnabled(true);
                       backupStatusLabel->changeText(ValidationMessage::getValidationMessage(QUICK_BKUP_DEV_ERR_MSG));
                       backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(95));
                       for(quint8 index = 1; index <=  MAX_CAMERAS; index++)
                       {
                           if(recSelect[index] == true)
                           {
                               statusReport[index].status = statusStr[QB_FAILED];
                               statusReport[index].reason = ValidationMessage::getValidationMessage(QUICK_BKUP_DEV_ERR_MSG);
                            }
                       }
                       forQuickbackup = false;
                       fillReportTable();
                       initRecData();
                       m_elementList[m_currElement]->forceActiveFocus();
                    }
                    else if(param->deviceStatus == CMD_INVALID_FILE_SIZE)
                    {
                     //   tempRecFound = 0;
                        startButton->setEnabled(true);
                        m_advanceOptionButton->setIsEnabled(true);
                        startButton->changeText(labelStr[QUICK_BACKUP_START_BTN]);
                        emit sigChangeToolbarBtnState(QUICK_BACKUP,false);
                        m_isBackupOn = false;
                        forQuickbackup = false;
                        emit sigUpdateQuickbackupFlag(m_isBackupOn);
                        backupStatusLabel->changeText((ValidationMessage::getValidationMessage(QUICK_BKUP_DISK_FULL_MSG)));
                        backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(120));
                        for(quint8 index = 1; index <= MAX_CAMERAS; index++)
                        {
                             if((recSelect[index] == true) &&(statusReport[index].noofRecords != 0))
                             {
                                 statusReport[index].status = statusStr[QB_FAILED];
                                 statusReport[index].reason = ValidationMessage::getValidationMessage(QUICK_BKUP_DISK_FULL_MSG);
                             }
                        }
                        fillReportTable();
                        initRecData();
                        m_elementList[m_currElement]->forceActiveFocus();
                    }
                    else
                    {
                       MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                       statusReport[currCamNo].status = statusStr[QB_FAILED];
                       statusReport[currCamNo].reason = ValidationMessage::getDeviceResponceMessage(param->deviceStatus);
                       sendMultipleRecBackupCmd ();
                    }
                }
                break;

            case STP_BCKUP:
                if(param->deviceStatus != CMD_SUCCESS)
                {
  //                  infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                else if(param->deviceStatus == CMD_SUCCESS)
                {
                    for(quint8 index = 1; index <= MAX_CAMERAS; index++)
                    {
                         if((recSelect[index] == true) &&(statusReport[index].noofRecords != 0))
                         {
                             statusReport[index].status = statusStr[QB_FAILED];
                         }
                    }
                    backupStatusLabel->changeText("Backup Stopped");
                    backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(45));
                    m_isBackupOn = false;
                    emit sigUpdateQuickbackupFlag(m_isBackupOn);
                    fillReportTable();
                    m_advanceOptionButton->setIsEnabled(true);
            //        initRecData();
                }
                break;

            default:
             break;
           }
         }
            break;

           default:
            break;
         }
    }
}

void MxQuickBackup::sendMultipleRecBackupCmd()
{
    quint8 index = 0;

    for(; index < tempRecFound; index++)
    {
        if(arrIndexofrecData[index] != INVALID_FEILD)
        {
            QString   tempStartDtTmStr,tempEndDtTmStr;
            QDateTime tempStartDtTm,tempEndDtTm;

            if((index < MAXIMUM_REC_ALLOWED) && (arrIndexofrecData[index] < MAXIMUM_REC_ALLOWED))
            {
                tempStartDtTmStr = playbackRecData[arrIndexofrecData[index]].startTime;
                tempEndDtTmStr = playbackRecData[arrIndexofrecData[index]].endTime;
            }

            tempStartDtTm = QDateTime(QDate(tempStartDtTmStr.mid(4,4).toInt(),
                                            tempStartDtTmStr.mid(2,2).toInt(),
                                            tempStartDtTmStr.mid(0,2).toInt()),
                                      QTime(tempStartDtTmStr.mid(8,2).toInt(),
                                            tempStartDtTmStr.mid(10,2).toInt(),
                                            tempStartDtTmStr.mid(12,2).toInt()));

            tempEndDtTm = QDateTime(QDate(tempEndDtTmStr.mid(4,4).toInt(),
                                            tempEndDtTmStr.mid(2,2).toInt(),
                                            tempEndDtTmStr.mid(0,2).toInt()),
                                      QTime(tempEndDtTmStr.mid(8,2).toInt(),
                                            tempEndDtTmStr.mid(10,2).toInt(),
                                            tempEndDtTmStr.mid(12,2).toInt()));

            if(tempStartDtTm < m_startDtTm)
            {
                tempStartDtTmStr = m_startDtTmStr;
            }

            if(tempEndDtTm > m_endDtTm)
            {
                tempEndDtTmStr = m_endDtTmStr;
            }
            if(m_startDtTm > tempEndDtTm )
            {
                statusReport[playbackRecData[arrIndexofrecData[index]].camNo].status = statusStr[QB_FAILED];
                statusReport[playbackRecData[arrIndexofrecData[index]].camNo].reason = "No records found.";

                 arrIndexofrecData[index] = INVALID_FEILD;
                continue;
            }

            payloadLib->setCnfgArrayAtIndex (0,tempStartDtTmStr);
            payloadLib->setCnfgArrayAtIndex (1,tempEndDtTmStr);
            payloadLib->setCnfgArrayAtIndex (2,playbackRecData[arrIndexofrecData[index]].camNo);
            payloadLib->setCnfgArrayAtIndex (3,playbackRecData[arrIndexofrecData[index]].overlap);
            payloadLib->setCnfgArrayAtIndex (4,playbackRecData[arrIndexofrecData[index]].hddIndicator);
            payloadLib->setCnfgArrayAtIndex (5,1);
            payloadLib->setCnfgArrayAtIndex (6,playbackRecData[arrIndexofrecData[index]].partionIndicator);
            payloadLib->setCnfgArrayAtIndex (7,playbackRecData[arrIndexofrecData[index]].recDriveIndex);
            payloadLib->setCnfgArrayAtIndex (8,currBckpLocationIndex);

			currCamNo = playbackRecData[arrIndexofrecData[index]].camNo;
            arrIndexofrecData[index] = INVALID_FEILD;
            break;
        }
    }
    if(index == tempRecFound)
    {
        isbackupcompleted = true;
        tempRecFound = 0;
        startButton->setEnabled(true);
        startButton->changeText(labelStr[QUICK_BACKUP_START_BTN]);
        backupStatusLabel->changeText("Backup Completed");
        backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(45));
        emit sigChangeToolbarBtnState(QUICK_BACKUP,false);
        m_isBackupOn = false;
        forQuickbackup = false;
        emit sigUpdateQuickbackupFlag(m_isBackupOn);
        fillReportTable();

        viewStatusButton->setEnabled(true);
        viewStatusButton->setIsEnabled(true);
        m_advanceOptionButton->setIsEnabled(true);
        viewStatusButton->show();
        initRecData();
        m_elementList[m_currElement]->forceActiveFocus();

     }
    else
    {
        QString payloadString = payloadLib->createDevCmdPayload(9);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = BKUP_RCD;
        param->payload = payloadString;

        if(applcontroller->processActivity(m_currDevName, DEVICE_COMM, param))
        {
            isBackupReqSent = true;
        }
    }
}

void MxQuickBackup::fillMultipleRecBackupCmd ()
{
    quint8 RecIndex = 0;

    for(RecIndex = 0; RecIndex < MAXIMUM_REC_ALLOWED; RecIndex++)
    {
        if(playbackRecData[RecIndex].camNo !=INVALID_FEILD)
        {
            arrIndexofrecData[RecIndex] = RecIndex;
        }
    }
}

void MxQuickBackup::BkpSysEvtAction(QString devName, LOG_EVENT_STATE_e evtState)
{
    if(devName == searchDevName)
    {
        if(evtState == EVENT_START)
        {
            if((m_isBackupOn == false) && (forQuickbackup == true))
            {
                backupStatusLabel->changeText("Backup Started");
                backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(45));
                startButton->setEnabled(true);
                startButton->changeText("Stop");
                viewStatusButton->setEnabled(true);
                viewStatusButton->setIsEnabled(true);
                viewStatusButton->show();
                emit sigChangeToolbarBtnState(QUICK_BACKUP,true);
                m_isBackupOn = true;
                m_elementList[m_currElement]->forceActiveFocus();
            }
        }
        else if(evtState == EVENT_COMPLETE)
        {
            if((isbackupcompleted == false)  && (forQuickbackup == true))
            {
                statusReport[currCamNo].noofRecords--;
                if((statusReport[currCamNo].noofRecords) == 0)
                {
                   statusReport[currCamNo].status = statusStr[QB_SUCCESS];
                }
                fillReportTable();
                sendMultipleRecBackupCmd ();
            }
            else
            {
                m_isBackupOn = false;
                fillReportTable();

            }
        }
        else
        {
            if(forQuickbackup == true)
            {
                startButton->setEnabled(true);
                m_advanceOptionButton->setIsEnabled(true);
                startButton->changeText(labelStr[QUICK_BACKUP_START_BTN]);
                emit sigChangeToolbarBtnState(QUICK_BACKUP,false);
                m_isBackupOn = false;
                forQuickbackup = false;
                emit sigUpdateQuickbackupFlag(m_isBackupOn);
                backupStatusLabel->changeText("");
                backupStatusLabel->changeText(ValidationMessage::getValidationMessage(QUICK_BKUP_FILE_COPY_FAIL));
                backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(45));
                for(quint8 index = 1; index <= MAX_CAMERAS; index++)
                {
                    if((recSelect[index] == true) &&(statusReport[index].noofRecords != 0))
                    {
                         statusReport[index].status = statusStr[QB_FAILED];
                         statusReport[index].reason = ValidationMessage::getValidationMessage(QUICK_BKUP_FILE_COPY_FAIL);
                    }
                }
                fillReportTable();
                initRecData();
                m_elementList[m_currElement]->forceActiveFocus();
            }
		 }
    }
}
void MxQuickBackup::initRecData()
{
    isbackupcompleted = false;
    isBackupReqSent = false;
    totalCamCount = 0;
    m_isBackupOn = false;
    for(quint8 index = 0; index < MAXIMUM_REC_ALLOWED; index++)
    {
        playbackRecData[index].camNo = INVALID_FEILD;
        arrIndexofrecData[index] = INVALID_FEILD;
    }
    for(quint8 index = 0; index <= MAX_CAMERAS; index++)
    {
        recSelect[index] = false;
    }
}

void MxQuickBackup::sendStopBackupCmd()
{
    payloadLib->setCnfgArrayAtIndex (0, 0);

    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = STP_BCKUP;
    param->payload = payloadString;
    m_advanceOptionButton->setIsEnabled(true);
    if(applcontroller->processActivity(searchDevName, DEVICE_COMM, param))
    {
        isBackupReqSent = false;
    }
}

void MxQuickBackup::fillReportTable()
{
    success = 0;
    fail = 0;
    QStringList reportList;
    for(quint8 index =1;index <= MAX_CAMERAS;index++)
    {
        if(statusReport[index].camNo != INVALID_FEILD)
        {
            if(statusReport[index].status == statusStr[QB_SUCCESS])
            {
                success++;
            }
            else if(statusReport[index].status == statusStr[QB_FAILED])
            {
                fail++;
            }
            reportList.append(statusReport[index].cameraName);
            reportList.append(statusReport[index].status);
            reportList.append(statusReport[index].reason);
        }

    }

    if(IS_VALID_OBJ(m_quickBcpTable))
    {
        m_quickBcpTable->showReport(reportList, totalCamCount, success, fail);
    }
}


void MxQuickBackup::slotScrollbarClick(int numberOfSteps)
{
    quint8 availableCameras = cameraList.count();

    if ((m_lastCameraIndex + numberOfSteps) > (availableCameras - 1))
    {
        numberOfSteps = (availableCameras - 1) - m_lastCameraIndex;
    }

    if ((m_firstCameraIndex + numberOfSteps) < 0)
    {
        numberOfSteps = -m_firstCameraIndex;
    }

    m_firstCameraIndex += numberOfSteps;
    m_lastCameraIndex += numberOfSteps;
    resetGeometryForCameraOption();
}

void MxQuickBackup::slotTileToolTipUpdate(QString m_timeString, int curX, int curY)
{
    m_sliderTileToolTip->textChange(m_timeString);
    m_sliderTileToolTip->resetGeometry(complRect->x() + SCALE_WIDTH(12) + curX,complRect->y() + SCALE_HEIGHT(40) + curY);
    m_sliderTileToolTip->setVisible(true);
}

void MxQuickBackup::slotTileToolTipHide(bool toolTip)
{
    m_sliderTileToolTip->setVisible(toolTip);
}

void MxQuickBackup::slotSliderToolTipUpdate(QString m_SliderDateTime, int curX, int curY)
{
    m_sliderToolTip->textChange(m_SliderDateTime);

    if((curX < SCALE_WIDTH(110)) ||(curX > SCALE_WIDTH(850)))
    {
        (curX < SCALE_WIDTH(110)) ? (curX = SCALE_WIDTH(115)):(curX = SCALE_WIDTH(850));
        m_sliderToolTip->resetGeometry(complRect->x() + SCALE_WIDTH(10) + curX ,complRect->y() + SCALE_HEIGHT(40) + curY);
    }
    else
    {
        m_sliderToolTip->resetGeometry(complRect->x() + SCALE_WIDTH(10) + curX, complRect->y() + SCALE_HEIGHT(40) + curY);
    }
    m_sliderToolTip->setVisible(true);
}

void MxQuickBackup::slotSliderToolTipHide(bool sliderToolTip)
{
    m_sliderToolTip->setVisible(sliderToolTip);
}

void MxQuickBackup::slotButtonClick(int index)
{
    switch(index)
    {
     case QUICK_BACKUP_START_BTN:
        if(startButton->getText() == labelStr[QUICK_BACKUP_START_BTN])
        {
            if(m_isDeviceConnected == true)
            {
                initRecData();
                tempRecFound = 0;
                for(quint8 index = 1; index <= cameraList.length(); index++)
                {
                     if(m_cameraCheckbox[index]->getCurrentState() == ON_STATE)
                     {
                         recSelect[index] = true;
                         totalCamCount++;
                      }
                }
                if(totalCamCount > 0)
                {
                    for(quint8 index = 0; index <= MAX_CAMERAS; index++)
                    {
                        statusReport[index].camNo = INVALID_FEILD;
                        statusReport[index].noofRecords = 0;
                        statusReport[index].cameraName = "";
                        statusReport[index].status = "";
                        statusReport[index].reason = "";
                    }
                    forQuickbackup = true;
                    searchDevName = m_currDevName;
                    currBckpLocationIndex = backupLocationDropDownBox->getIndexofCurrElement();
                    searchRecIndex = 1;

                    m_timeLineSlider->getDateTime(&m_startDtTm,&m_endDtTm);

                    m_startDtTmStr = m_startDtTm.toString("ddMMyyyyHHmmss");
                    m_endDtTmStr = m_endDtTm.toString("ddMMyyyyHHmmss");

                    startButton->setEnabled(false);
                    m_advanceOptionButton->setIsEnabled(false);
                    searchRecData();
                }
                else
                {
                    backupStatusLabel->changeText("Please select atleast one camera");
                    backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(110));

                }
            }
        }
        else
        {
            if(isBackupReqSent)
            {
                startButton->setEnabled(false);
                sendStopBackupCmd();
//                startButton->changeText(labelStr[QUICK_BACKUP_START_BTN]);
//                backupStatusLabel->changeText("Backup Stopped");
//                backupStatusLabel->resetGeometry((complRect->width() / 2) - 45);
//                emit sigChangeToolbarBtnState(QUICK_BACKUP,false);
                m_isBackupOn = false;

            }
        }
        break;

      case QUICK_BACKUP_VIEW_STATUS_BTN:
        {
            /* PARASOFT: Memory Deallocated in slot QuickBkupRptClose */
            disabledImage = new Image (0,
                                       0,
                                       QUICK_BACKUP_DISABLE_IMG_PATH,
                                       this,
                                       START_X_START_Y,
                                       MAX_QUICK_BACKUP_ELEMENTS,
                                       false,
                                       true,
                                       false,
                                       false);
            m_quickBcpTable->raise();
            m_quickBcpTable->show();
            showHideElements(true);
        }
        break;
    }
}

void MxQuickBackup::slotQuickBkupRptClose(TOOLBAR_BUTTON_TYPE_e)
{
    if(IS_VALID_OBJ(m_quickBcpTable))
    {
        m_quickBcpTable->hide();
  //      showHideElements((false));
        DELETE_OBJ(disabledImage);
    }
    m_elementList[m_currElement]->forceActiveFocus();
}

void MxQuickBackup::slotCameraCheckboxClicked(OPTION_STATE_TYPE_e currentState,int indexInPage)
{
    if(indexInPage == QUICK_BACKUP_CAMERASELECT_CHECKBOX)
    {
        selectDeselectAllRecords(currentState);
    }
    else
    {
        m_cameraCheckbox[0]->changeState((OPTION_STATE_TYPE_e)isAllButtonChecked());
    }
}


void MxQuickBackup::slotUpdateCurrentElement (int index)
{
    m_currElement = index;
}

void MxQuickBackup::takeLeftKeyAction()
{
    bool status = true;

    do
    {
        if(m_currElement == 0)
        {
            m_currElement = (MAX_QUICK_BACKUP_ELEMENTS);
        }

        if(m_currElement == QUICK_BACKUP_SCROLLBAR)
        {
            m_currElement = QUICK_BACKUP_CAMERASELECT_CHECKBOX;
        }
        else if((m_currElement >= QUICK_BACKUP_CAMERASELECT_CHECKBOX) &&
                (m_currElement < QUICK_BACKUP_SCROLLBAR))
        {
            m_currElement = QUICK_BACKUP_CAM_LIST;
        }
        else if(m_currElement)
        {
            m_currElement = (m_currElement - 1);
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }

    }while((m_elementList[m_currElement] == NULL)
           || (!m_elementList[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void MxQuickBackup::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(m_currElement == (MAX_QUICK_BACKUP_ELEMENTS - 1))
        {
            m_currElement = -1;
        }

        if((m_currElement >= QUICK_BACKUP_CAMERASELECT_CHECKBOX) &&
                (m_currElement < QUICK_BACKUP_SCROLLBAR))
        {
            m_currElement = QUICK_BACKUP_BCKP_LOCATION;
        }
        else if(m_currElement != (MAX_QUICK_BACKUP_ELEMENTS - 1))
        {
            m_currElement = (m_currElement + 1);
        }
        else
        {
            status = false;
            break;
        }

    }while((m_elementList[m_currElement] == NULL)
           || (!m_elementList[m_currElement]->getIsEnabled()));

    if( status == true)
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void MxQuickBackup::takeUpKeyAction()
{
    quint8 availableCameras = cameraList.count();

    if((m_currElement > QUICK_BACKUP_CAMERASELECT_CHECKBOX) &&
            (m_currElement <= (QUICK_BACKUP_CAMERASELECT_CHECKBOX + availableCameras)))
    {
        bool status = true;
        do
        {
            if(m_currElement > 0)
            {
                m_currElement = (m_currElement - 1);
            }
            else
            {
                status = false;
                break;
            }

            if((m_currElement > QUICK_BACKUP_CAMERASELECT_CHECKBOX) &&
                    (m_currElement < (QUICK_BACKUP_CAMERASELECT_CHECKBOX + m_firstCameraIndex + 1)))
            {
                status = true;
                break;
            }

        }while((m_elementList[m_currElement] == NULL)
               || (!m_elementList[m_currElement]->getIsEnabled()));

        if(status == true)
        {
            if(m_currElement == (QUICK_BACKUP_CAMERASELECT_CHECKBOX + m_firstCameraIndex))
            {
                if((IS_VALID_OBJ(m_Scrollbar)) &&
                        (m_Scrollbar->getIsEnabled()))
                {
                     m_Scrollbar->updateBarGeometry(-1);
                     if(IS_VALID_OBJ(m_elementList[m_currElement]))
                     {
                         m_elementList[m_currElement]->forceActiveFocus();
                     }
                }
            }
            else
            {
                m_elementList[m_currElement]->forceActiveFocus();
            }
        }
    }
}

void MxQuickBackup::takeDownKeyAction()
{
    quint8 availableCameras = cameraList.count();

    if((m_currElement >= QUICK_BACKUP_CAMERASELECT_CHECKBOX) &&
            (m_currElement < (QUICK_BACKUP_CAMERASELECT_CHECKBOX + availableCameras)))
    {
        bool status = true;
        do
        {
            if(m_currElement != (MAX_QUICK_BACKUP_ELEMENTS - 1))
            {
                m_currElement = (m_currElement + 1);
            }
            else
            {
                status = false;
                break;
            }

            if(m_currElement > (QUICK_BACKUP_CAMERASELECT_CHECKBOX + m_lastCameraIndex + 1))
            {
                status = true;
                break;
            }

        }while((m_elementList[m_currElement] == NULL)
               || (!m_elementList[m_currElement]->getIsEnabled()));

        if(status == true)
        {
            if(m_currElement == (QUICK_BACKUP_CAMERASELECT_CHECKBOX + m_lastCameraIndex + 2))
            {
                if((IS_VALID_OBJ(m_Scrollbar)) && (m_Scrollbar->getIsEnabled()))
                {
                     m_Scrollbar->updateBarGeometry(1);
                     if(IS_VALID_OBJ(m_elementList[m_currElement]))
                     {
                         m_elementList[m_currElement]->forceActiveFocus();
                     }
                }
            }
            else
            {
                m_elementList[m_currElement]->forceActiveFocus();
            }
        }
    }
}

void MxQuickBackup::wheelEvent(QWheelEvent *event)
{
    if ((IS_VALID_OBJ(m_Scrollbar))
            && (event->x() >= (m_Scrollbar->x() + m_Scrollbar->width() - CONTROL_WIDTH)) && (event->x() <= (m_Scrollbar->x() + m_Scrollbar->width()))
            && (event->y() >= m_Scrollbar->y()) && (event->y() <= (m_Scrollbar->y() + m_Scrollbar->height())))
    {
        m_Scrollbar->wheelEvent(event);
    }
    else
    {
        QWidget::wheelEvent(event);
    }
}

void MxQuickBackup::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void MxQuickBackup::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
    switch(event->key())
    {
    case Qt::Key_Up:
        takeUpKeyAction();
        break;

    case Qt::Key_Down:
        takeDownKeyAction();
        break;

    default:
        break;
    }
}

void MxQuickBackup::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void MxQuickBackup::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = QUICK_BACKUP_CLOSE_BTN;
    m_elementList[QUICK_BACKUP_CLOSE_BTN]->forceActiveFocus();
}

void MxQuickBackup::slotSpinboxValueChanged(QString str, quint32 index)
{
    switch(index)
    {
    case QUICK_BACKUP_DEVICE_SPINBOX:
        str = applcontroller->GetRealDeviceName(str);
        if (str == m_currDevName)
        {
            break;
        }

        m_currDevName = str;
        cameraList.clear ();
        m_isDeviceConnected = applcontroller->GetAllCameraNamesOfDevice(m_currDevName,cameraList,PLAYBACK_CAM_LIST_TYPE,&m_cameraIndex[0]);
        if(m_isDeviceConnected == false)
        {
            backupStatusLabel->changeText(ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
            backupStatusLabel->resetGeometry((complRect->width() / 2) - SCALE_WIDTH(40));
        }
        else
        {
            DEV_TABLE_INFO_t devTableInfo;
           if(applcontroller->GetDeviceConnectionState (m_currDevName) == CONFLICT)
           {
               QString msgStr = "Firmware mismatch detected. Please upgrade firmware";
               if(applcontroller->GetDeviceInfo (m_currDevName,devTableInfo))
               {
                   if(devTableInfo.deviceConflictType == MX_DEVICE_CONFLICT_TYPE_SERVER_NEW)
                   {
                       msgStr = "Firmware mismatch detected. Please upgrade network device firmware";
                   }
               }
               MessageBanner::addMessageInBanner(msgStr);
           }
        }
        changeCameraElement();
        resetGeometryForCameraOption();
        break;

     case QUICK_BACKUP_BCKP_LOCATION:
        break;

     case QUICK_BACKUP_HOURS_DROPDOWN:

        quint8 index = hourTypeString.indexOf(str);
        m_timeLineSlider->defaultSliderDraw((QUICK_BK_HOURS_FORMAT_e)index);
        break;
     }
}

void MxQuickBackup::updateChangeCameraElement()
{
    if(m_currDevName == LOCAL_DEVICE_NAME)
    {
        m_isDeviceConnected = applcontroller->GetAllCameraNamesOfDevice( LOCAL_DEVICE_NAME,
                                                                         cameraList,
                                                                         PLAYBACK_CAM_LIST_TYPE,
                                                                         &m_cameraIndex[0]);
        changeCameraElement();
    }
}

void MxQuickBackup::slotImageClicked(int indexInPage)
{
    switch(indexInPage)
    {
      case QUICK_BACKUP_MINIMIZE_BTN:
             this->hide();
               m_isminimize = true;
               emit sigClosePage(QUICK_BACKUP);
        break;
    }
}

void MxQuickBackup:: slotCloseBtnClick(int indexInpage)
{
    sendStopBackupCmd();
    Q_UNUSED(indexInpage);
}

void MxQuickBackup::slotTileWithTextClick(int indexInPage)
{
    switch (indexInPage)
    {
    case QUICK_BACKUP_SHOW_CLIP_SETTNGS_TILE:
        if(isShowClipSettings)
        {
            showHideElements(false);
        }
        else
        {
            showHideElements(true);
        }
        break;
    default:
        break;
    }
}

void MxQuickBackup::slotAdvanceButtonClicked(int index)
{
    if(index == QUICK_BACKUP_ADVANCE_BUTTON)
    {
        emit sigAdavnceOptions (QUICK_BACKUP_ADVANCE_BUTTON);
    }
}

void MxQuickBackup::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    applcontroller->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (devNameDropDownBox->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            devNameDropDownBox->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* If selected device is local device then it will update the device name only otherwise it will clear the data and will select the local device */
    devNameDropDownBox->setNewList(deviceMapList, 0);
    slotSpinboxValueChanged(deviceMapList.value(0), QUICK_BACKUP_DEVICE_SPINBOX);
}
