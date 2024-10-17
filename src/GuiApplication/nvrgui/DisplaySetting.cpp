#include <signal.h>
#include <unistd.h>

#include <QApplication>
#include <QKeyEvent>

#include "DisplaySetting.h"
#include "Layout/Layout.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define DISP_SETTING_LAYOUT_CREATOR_WIDTH   SCALE_WIDTH(780)
#define DISP_SETTING_LAYOUT_CREATOR_HEIGHT  SCALE_HEIGHT(720)

#define DISP_SETTING_LAYOUT_LIST_BG_WIDTH   SCALE_WIDTH(160)
#define DISP_SETTING_CAM_LIST_BG_WIDTH      SCALE_WIDTH(298)

#define DISP_STG_MOST_OUTER_OFFSET          20
#define DISP_STG_INTER_ELE_OFFSET           10

#define DISP_SETTING_HEADING_STR            "Live View"

static const QString labelStr[MAX_DISPLAY_STG_ELEMENTS] =
{
    "",                         // DISPLAY_STG_CLOSE_BTN
    "Live View",                // DISPLAY_STG_LIVE_VIEW_DROPDOWN
    "Resolution",               // DISPLAY_STG_RESOLUTION_PICKLIST
    "TV Adjust",                // DISPLAY_STG_TV_ADJUST
    "Optimize BW",              // DISPLAY_STG_BANDWITH_OPT
    "Appearance Setting",       // DISPLAY_STG_APPEARANCE_BTN
    "", "", "", "", "",
    "", "", "", "", "",
    "",
    "Layout Style",             // DISPLAY_STG_LAYOUT_STYLE_PICKLIST
    "Default",                  // DISPLAY_STG_DEFAUILT_SEL
    "Auto Page Navigation",     // DISPLAY_STG_SEQ_ENABLE
    "(10-255)sec",              // DISPLAY_STG_SEQ_INTERVAL_SPINBOX
    "Apply",                    // DISPLAY_STG_APPLY_BTN
    "Save",                     // DISPLAY_STG_SAVE_BTN
    "Cancel"                    // DISPLAY_STG_CANCEL_BTN
};

static QMap<quint8, QString> liveViewTypeMapList =
{
    {LIVE_VIEW_TYPE_SMOOTH,     "Smooth"},
    {LIVE_VIEW_TYPE_REAL_TIME,  "Real-Time"}
};

static QMap<quint8, QString> displayResolutionMapList =
{
    {DISPLAY_RESOLUTION_720P,   "1280x720"},
    {DISPLAY_RESOLUTION_1080P,  "1920x1080"},
    {DISPLAY_RESOLUTION_2160P,  "3840x2160"}
};

static const QStringList styleNameList = QStringList() << "Current Style" << "Style 1" << "Style 2" << "Style 3" << "Style 4" << "Style 5";

bool DisplaySetting::pageSequenceStatus = false;

DisplaySetting::DisplaySetting(QWidget *parent)
    : BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH)) / 2)),
                 (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT)) / 2)),
                 SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH),
                 SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT) - SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                 BACKGROUND_TYPE_1,
                 LIVE_VIEW_BUTTON,
                 parent,
                 SCALE_WIDTH(DISP_SETTING_PAGE_HEADING_WIDTH),
                 SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                 DISP_SETTING_HEADING_STR),
      m_currLayout(MAX_LAYOUT), m_currentDisplayId(MAIN_DISPLAY), m_currentStyle(MAX_STYLE_TYPE),
      m_currentWindow(MAX_CHANNEL_FOR_SEQ), m_username(""), m_password("")
{
    INIT_OBJ(m_styleSelectionOpt);
    INIT_OBJ(m_appearanceSetting);
    INIT_OBJ(m_windowSequenceSetting);
    INIT_OBJ(m_backGroundImage);
    INIT_OBJ(m_userValidation);
    m_currDfltStyle = MAX_STYLE_TYPE;
    maxWindows = 0;
    m_payloadLib = new PayloadLib();

    m_currentdisplayIndex = MAIN_DISPLAY_HDMI;
    nextPageSelected = false;
    m_tempStartAudioInWindow = MAX_CHANNEL_FOR_SEQ;
    m_applController = ApplController::getInstance();
    m_imageSource = QString(IMAGE_PATH) + QString("InfoPageImages/LiveView.png");

    createDefaultComponent();

    m_processBar = new ProcessBar(0, SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                                  SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH),
                                  SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT) - SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                                  SCALE_WIDTH(15), this);

    m_infoPage = new InfoPage(0, 0,
                              SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH),
                              SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT),
                              INFO_LIVEVIEW,
                              this);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageCnfgBtnClick(int)));

    m_isChangeDone = false;
    m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, m_devTable);
    currResolution = prevResolution = m_applController->readResolution((DISPLAY_TYPE_e)m_currentdisplayIndex);
    doChangeDisplayTypeProcess();
    m_tvAdjustSpinBox->setIndexofCurrElement(Layout::tvAdjustParam);
    m_LiveViewDropdown->setIndexofCurrElement(m_applController->GetLiveViewType());
    m_currElement = DISPLAY_STG_LIVE_VIEW_DROPDOWN;
    m_elementList[m_currElement]->forceActiveFocus();

    this->show();
}

DisplaySetting::~DisplaySetting()
{
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));

    delete m_processBar;

    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageCnfgBtnClick(int)));
    delete m_infoPage;

    disconnect(m_LiveViewDropdown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_LiveViewDropdown;

    disconnect(m_resolutionPicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPicklistValueChanged(quint8,QString,int)));
    disconnect(m_resolutionPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_resolutionPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotPickListButtonClick(int)));
    delete m_resolutionPicklist;

    if(IS_VALID_OBJ(m_tvAdjustSpinBox))
    {
        disconnect(m_tvAdjustSpinBox,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotSpinBoxValueChanged(QString,quint32)));
        DELETE_OBJ(m_tvAdjustSpinBox);
    }

    if(IS_VALID_OBJ(m_appearancePageopenBtn))
    {
        disconnect(m_appearancePageopenBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotAppearanceButtonClick(int)));
        disconnect(m_appearancePageopenBtn,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpadateCurrentElement(int)));
        DELETE_OBJ(m_appearancePageopenBtn);
    }

    delete m_layoutListBackground;
    disconnect(m_layoutList,
               SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
               this,
               SLOT(slotChangeLayout(LAYOUT_TYPE_e)));
    disconnect(m_layoutList,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_layoutList;

    disconnect(m_layoutCreator,
               SIGNAL(sigSwapWindows(quint16,quint16)),
               this,
               SLOT(slotSwapWindows(quint16,quint16)));
    disconnect(m_layoutCreator,
               SIGNAL(sigWindowSelected(quint16)),
               this,
               SLOT(slotWindowSelected(quint16)));
    disconnect(m_layoutCreator,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_layoutCreator,
               SIGNAL(sigDragStartStopEvent(bool)),
               this,
               SLOT(slotDragStartStopEvent(bool)));
    disconnect(m_layoutCreator,
               SIGNAL(sigWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint16)),
               this,
               SLOT(slotWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint16)));
    disconnect(m_layoutCreator,
               SIGNAL(sigWindowImageHover(WINDOW_IMAGE_TYPE_e,quint16,bool)),
               this,
               SLOT(slotWindowImageHover(WINDOW_IMAGE_TYPE_e,quint16,bool)));
    delete m_layoutCreator;

    disconnect(m_stylePicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPicklistValueChanged(quint8,QString,int)));
    disconnect(m_stylePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_stylePicklist;

    disconnect(m_dfltOptSelBtn,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotSettingOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(m_dfltOptSelBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_dfltOptSelBtn;

    disconnect(m_seqOptSelBtn,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotSettingOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(m_seqOptSelBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_seqOptSelBtn;

    disconnect(m_seqIntervalSpinbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_seqIntervalSpinbox;

    disconnect(m_bandwidthOpt,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_bandwidthOpt;

    disconnect(m_applyBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCnfgButtonClick(int)));
    disconnect(m_applyBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_applyBtn;

    disconnect(m_saveBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCnfgButtonClick(int)));
    disconnect(m_saveBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_saveBtn;

    disconnect(m_cancelBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCnfgButtonClick(int)));
    disconnect(m_cancelBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_cancelBtn;
    delete m_camListBackground;

    disconnect(m_cameraList,
               SIGNAL(sigCameraConfigListUpdate()),
               this,
               SLOT(slotCameraConfigListUpdate()));
    disconnect(m_cameraList,
               SIGNAL(sigCameraButtonClicked(quint8,QString,CAMERA_STATE_TYPE_e,bool,bool)),
               this,
               SLOT(slotCameraButtonClicked(quint8,QString,CAMERA_STATE_TYPE_e,bool,bool)));
    disconnect(m_cameraList,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_cameraList;

    delete m_pageNoReadOnly;

    disconnect(m_firstPageCntrlBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_firstPageCntrlBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNextPrevPageClick(int)));
    delete m_firstPageCntrlBtn;

    disconnect(m_prevPageCntrlBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNextPrevPageClick(int)));
    disconnect(m_prevPageCntrlBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_prevPageCntrlBtn;

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
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

    disconnect(m_nextPageCntrlBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNextPrevPageClick(int)));
    disconnect(m_nextPageCntrlBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_nextPageCntrlBtn;

    disconnect(m_lastPageCntrlBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_lastPageCntrlBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNextPrevPageClick(int)));
    delete m_lastPageCntrlBtn;
    delete m_payloadLib;

    DELETE_OBJ(m_windowSequenceSetting);
    DELETE_OBJ(m_appearanceSetting);
    DELETE_OBJ(m_userValidation);
    DELETE_OBJ(m_backGroundImage);
    DELETE_OBJ(m_styleSelectionOpt);
}

void DisplaySetting::createDefaultComponent()
{
    QStringList seqIntervalList;

    seqIntervalList.reserve(246);
    for(quint16 index = 10; index <= 255; index++)
    {
        seqIntervalList.append(QString("%1").arg(index));
    }

    m_elementList[DISPLAY_STG_CLOSE_BTN] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(DISPLAY_STG_CLOSE_BTN);
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    m_LiveViewDropdown = new DropDown(SCALE_WIDTH(DISP_STG_MOST_OUTER_OFFSET),
                                      (SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT) + SCALE_HEIGHT(DISP_STG_MOST_OUTER_OFFSET)),
                                      (SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH) - (2*(SCALE_WIDTH(DISP_STG_MOST_OUTER_OFFSET)))),
                                      BGTILE_HEIGHT,
                                      DISPLAY_STG_LIVE_VIEW_DROPDOWN,
                                      DROPDOWNBOX_SIZE_200,
                                      labelStr[DISPLAY_STG_LIVE_VIEW_DROPDOWN],
                                      liveViewTypeMapList,
                                      this,
                                      "",
                                      false,
                                      SCALE_WIDTH(10));
    m_elementList[DISPLAY_STG_LIVE_VIEW_DROPDOWN] = m_LiveViewDropdown;
    connect(m_LiveViewDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    m_resolutionPicklist = new PickList((m_LiveViewDropdown->x() + SCALE_WIDTH(320)),
                                        m_LiveViewDropdown->y(),
                                        BGTILE_EXTRALARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(120),
                                        READONLY_HEIGHT,
                                        labelStr[DISPLAY_STG_RESOLUTION_PICKLIST],
                                        displayResolutionMapList,
                                        DISPLAY_RESOLUTION_2160P,
                                        "Select Resolution",
                                        this,
                                        NO_LAYER,
                                        0,
                                        DISPLAY_STG_RESOLUTION_PICKLIST,
                                        true, true, true);
    m_elementList[DISPLAY_STG_RESOLUTION_PICKLIST] = m_resolutionPicklist;
    connect(m_resolutionPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_resolutionPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPicklistValueChanged(quint8,QString,int)));
    connect(m_resolutionPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotPickListButtonClick(int)));

    QStringList tvAdjustValueList = QStringList() << "0" << "1" << "2" << "3";
    m_tvAdjustSpinBox = new SpinBox(m_resolutionPicklist->x() + m_resolutionPicklist->width() + SCALE_WIDTH(20),
                                    m_resolutionPicklist->y(),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    DISPLAY_STG_TV_ADJUST,
                                    SPINBOX_SIZE_90,
                                    "TV Adjust", tvAdjustValueList, this,
                                    "",false, 0, NO_LAYER);
    m_elementList[DISPLAY_STG_TV_ADJUST] = m_tvAdjustSpinBox;
    connect(m_tvAdjustSpinBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChanged(QString,quint32)));

    m_bandwidthOpt = new OptionSelectButton((m_resolutionPicklist->x() + m_resolutionPicklist->width() + SCALE_WIDTH(310)),
                                            m_resolutionPicklist->y(),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            labelStr[DISPLAY_STG_BANDWITH_OPT],
                                            this,
                                            NO_LAYER,
                                            -1,
                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                            NORMAL_FONT_SIZE,
                                            DISPLAY_STG_BANDWITH_OPT);
    m_elementList[DISPLAY_STG_BANDWITH_OPT] = m_bandwidthOpt;
    connect(m_bandwidthOpt,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    m_appearancePageopenBtn = new PageOpenButton(m_bandwidthOpt->x() + m_bandwidthOpt->width() + SCALE_WIDTH(10) ,
                                                 m_bandwidthOpt->y(),
                                                 BGTILE_LARGE_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 DISPLAY_STG_APPEARANCE_BTN,
                                                 PAGEOPENBUTTON_EXTRALARGE,
                                                 labelStr[DISPLAY_STG_APPEARANCE_BTN],
                                                 this, "", "",
                                                 false, 0, NO_LAYER);
    m_elementList[DISPLAY_STG_APPEARANCE_BTN] = m_appearancePageopenBtn;
    connect(m_appearancePageopenBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_appearancePageopenBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotAppearanceButtonClick(int)));

    m_layoutListBackground = new BgTile(m_LiveViewDropdown->x(),
                                        (m_LiveViewDropdown->y() +
                                        m_LiveViewDropdown->height() +
                                        SCALE_HEIGHT(DISP_STG_INTER_ELE_OFFSET)),
                                        DISP_SETTING_LAYOUT_LIST_BG_WIDTH,
                                        DISP_SETTING_LAYOUT_CREATOR_HEIGHT,
                                        COMMON_LAYER, this);

    m_layoutList = new LayoutList(m_layoutListBackground->x() + SCALE_WIDTH(DISP_STG_INTER_ELE_OFFSET) - SCALE_WIDTH(2),
                                  m_layoutListBackground->y() + SCALE_HEIGHT(DISP_STG_INTER_ELE_OFFSET) + SCALE_HEIGHT(16),
                                  LAYOUT_LIST_WITH_SCROLL,
                                  this,
                                  16,
                                  DISPLAY_STG_LAYOUT_LIST);
    m_elementList[DISPLAY_STG_LAYOUT_LIST] = m_layoutList;
    connect(m_layoutList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_layoutList,
            SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
            this,
            SLOT(slotChangeLayout(LAYOUT_TYPE_e)));

    m_applController->readMaxWindowsForDisplay(maxWindows);

    m_layoutCreator = new LayoutCreator((m_layoutListBackground->x() + m_layoutListBackground->width() + SCALE_WIDTH(DISP_STG_INTER_ELE_OFFSET) - SCALE_WIDTH(4)),
                                        m_layoutListBackground->y(),
                                        DISP_SETTING_LAYOUT_CREATOR_WIDTH,
                                        DISP_SETTING_LAYOUT_CREATOR_HEIGHT,
                                        this,
                                        DISPLAY_STG_LAYOUT_CREATOR,
                                        WINDOW_TYPE_DISPLAYSETTINGS,
                                        true,
                                        maxWindows);
    m_elementList[DISPLAY_STG_LAYOUT_CREATOR] = m_layoutCreator;
    connect(m_layoutCreator,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_layoutCreator,
            SIGNAL(sigWindowSelected(quint16)),
            this,
            SLOT(slotWindowSelected(quint16)));
    connect(m_layoutCreator,
            SIGNAL(sigSwapWindows(quint16,quint16)),
            this,
            SLOT(slotSwapWindows(quint16,quint16)));
    connect(m_layoutCreator,
            SIGNAL(sigDragStartStopEvent(bool)),
            this,
            SLOT(slotDragStartStopEvent(bool)));
    connect(m_layoutCreator,
            SIGNAL(sigWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint16)),
            this,
            SLOT(slotWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint16)));
    connect(m_layoutCreator,
            SIGNAL(sigWindowImageHover(WINDOW_IMAGE_TYPE_e,quint16,bool)),
            this,
            SLOT(slotWindowImageHover(WINDOW_IMAGE_TYPE_e,quint16,bool)));

    QMap<quint8, QString> styleMap;
    for(quint8 index = 0; index < styleNameList.length(); index++)
    {
        styleMap.insert(index, styleNameList.at(index));
    }
    m_stylePicklist = new PickList(m_layoutListBackground->x(),
                                   (m_layoutListBackground->y() + m_layoutListBackground->height() + SCALE_HEIGHT(DISP_STG_INTER_ELE_OFFSET)),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   SCALE_WIDTH(READONLY_MEDIAM_WIDTH),
                                   READONLY_HEIGHT,
                                   labelStr[DISPLAY_STG_LAYOUT_STYLE_PICKLIST],
                                   styleMap,
                                   0,
                                   "Select Style",
                                   this,
                                   NO_LAYER, -1,
                                   DISPLAY_STG_LAYOUT_STYLE_PICKLIST);
    m_elementList[DISPLAY_STG_LAYOUT_STYLE_PICKLIST] = m_stylePicklist;
    connect(m_stylePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_stylePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPicklistValueChanged(quint8,QString,int)));

    m_dfltOptSelBtn = new OptionSelectButton(m_stylePicklist->x() + m_stylePicklist->width() + SCALE_WIDTH(25),
                                             m_stylePicklist->y(),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CHECK_BUTTON_INDEX,
                                             labelStr[DISPLAY_STG_DEFAUILT_SEL],
                                             this,
                                             NO_LAYER,
                                             -1,
                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                             NORMAL_FONT_SIZE,
                                             DISPLAY_STG_DEFAUILT_SEL);
    m_elementList[DISPLAY_STG_DEFAUILT_SEL] = m_dfltOptSelBtn;
    connect(m_dfltOptSelBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_dfltOptSelBtn,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotSettingOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_seqOptSelBtn = new OptionSelectButton(m_dfltOptSelBtn->x() + m_dfltOptSelBtn->width() + SCALE_WIDTH(30),
                                            m_dfltOptSelBtn->y(),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            this, NO_LAYER,
                                            "",
                                            "",
                                            -1, DISPLAY_STG_SEQ_ENABLE, true,
                                            NORMAL_FONT_SIZE);
    m_elementList[DISPLAY_STG_SEQ_ENABLE] = m_seqOptSelBtn;
    connect(m_seqOptSelBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_seqOptSelBtn,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotSettingOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_seqIntervalSpinbox = new SpinBox(m_seqOptSelBtn->x() + m_seqOptSelBtn->width() + SCALE_WIDTH(10),
                                       m_seqOptSelBtn->y(),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       DISPLAY_STG_SEQ_INTERVAL_SPINBOX,
                                       SPINBOX_SIZE_90,
                                       "Auto Page Navigation", seqIntervalList, this,
                                       labelStr[DISPLAY_STG_SEQ_INTERVAL_SPINBOX],
                                       false, 0, NO_LAYER);
    m_elementList[DISPLAY_STG_SEQ_INTERVAL_SPINBOX] = m_seqIntervalSpinbox;
    connect(m_seqIntervalSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    m_applyBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                m_seqIntervalSpinbox->x() + m_seqIntervalSpinbox->width() + SCALE_WIDTH(100),
                                m_seqIntervalSpinbox->y() + m_seqIntervalSpinbox->height()/2,
                                labelStr[DISPLAY_STG_APPLY_BTN],
                                this, DISPLAY_STG_APPLY_BTN);
    m_elementList[DISPLAY_STG_APPLY_BTN] = m_applyBtn;
    connect(m_applyBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_applyBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClick(int)));

    m_saveBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               m_applyBtn->x() + m_applyBtn->width() + SCALE_WIDTH(60),
                               m_applyBtn->y() + m_applyBtn->height()/2,
                               labelStr[DISPLAY_STG_SAVE_BTN],
                               this, DISPLAY_STG_SAVE_BTN);
    m_elementList[DISPLAY_STG_SAVE_BTN] = m_saveBtn;
    connect(m_saveBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_saveBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClick(int)));

    m_cancelBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 m_saveBtn->x() + m_saveBtn->width() + SCALE_WIDTH(60),
                                 m_saveBtn->y() + m_saveBtn->height()/2,
                                 labelStr[DISPLAY_STG_CANCEL_BTN],
                                 this, DISPLAY_STG_CANCEL_BTN);
    m_elementList[DISPLAY_STG_CANCEL_BTN] = m_cancelBtn;
    connect(m_cancelBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_cancelBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClick(int)));

    m_camListBackground = new BgTile(m_layoutCreator->x() + m_layoutCreator->width() + SCALE_WIDTH(DISP_STG_INTER_ELE_OFFSET) - SCALE_WIDTH(4),
                                     m_layoutCreator->y(),
                                     DISP_SETTING_CAM_LIST_BG_WIDTH,
                                     DISP_SETTING_LAYOUT_CREATOR_HEIGHT,
                                     COMMON_LAYER, this);

    m_cameraList = new CameraList(m_camListBackground->x() + SCALE_WIDTH(DISP_STG_INTER_ELE_OFFSET),
                                  m_camListBackground->y() + SCALE_HEIGHT(DISP_STG_INTER_ELE_OFFSET),
                                  this,
                                  DISPLAY_STG_CAMERA_LIST,
                                  CALLED_BY_DISPLAY_SETTING);
    m_elementList[DISPLAY_STG_CAMERA_LIST] = m_cameraList;
    connect(m_cameraList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_cameraList,
            SIGNAL(sigCameraButtonClicked(quint8,QString,CAMERA_STATE_TYPE_e,bool,bool)),
            this,
            SLOT(slotCameraButtonClicked(quint8,QString,CAMERA_STATE_TYPE_e,bool,bool)));
    connect(m_cameraList,
            SIGNAL(sigCameraConfigListUpdate()),
            this,
            SLOT(slotCameraConfigListUpdate()));

    m_pageNoReadOnly = new ReadOnlyElement(m_layoutCreator->x() + (m_layoutCreator->width()/2) - (SCALE_WIDTH(READONLY_LARGE_WIDTH/2)),
                                           (m_layoutCreator->y() + m_layoutCreator->height() - READONLY_HEIGHT - SCALE_HEIGHT(12)),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           (SCALE_WIDTH(READONLY_LARGE_WIDTH) + SCALE_WIDTH(30)),
                                           READONLY_HEIGHT,
                                           "", this, NO_LAYER);

    m_prevPageCntrlBtn = new ControlButton(PREVIOUS_BUTTON_1_INDEX,
                                           m_pageNoReadOnly->x() - SCALE_WIDTH(30),
                                           m_pageNoReadOnly->y(),
                                           BGTILE_SMALL_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           this, NO_LAYER, -1,
                                           "", true, DISPLAY_STG_PREV_PAGE);
    m_elementList[DISPLAY_STG_PREV_PAGE] = m_prevPageCntrlBtn;
    connect(m_prevPageCntrlBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_prevPageCntrlBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNextPrevPageClick(int)));

    m_firstPageCntrlBtn = new ControlButton(FIRSTPAGE_BUTTON_1_INDEX,
                                            m_pageNoReadOnly->x() - SCALE_WIDTH(60),
                                            m_pageNoReadOnly->y(),
                                            BGTILE_SMALL_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            this, NO_LAYER, -1,
                                            "", true, DISPLAY_STG_FIRST_PAGE);
    m_elementList[DISPLAY_STG_FIRST_PAGE] = m_firstPageCntrlBtn;
    connect(m_firstPageCntrlBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_firstPageCntrlBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNextPrevPageClick(int)));

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_PageNumberLabel[index] = new TextWithBackground((m_pageNoReadOnly->x() + (index*SCALE_WIDTH(50)) + SCALE_WIDTH(20)),
                                                          m_pageNoReadOnly->y() + SCALE_HEIGHT(7),
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
                                                          (DISPLAY_STG_PAGE_NUM_BTN + index));
        m_elementList[(DISPLAY_STG_PAGE_NUM_BTN + index)] = m_PageNumberLabel[index];
        connect(m_PageNumberLabel[index],
                SIGNAL(sigMousePressClick(QString)),
                this,
                SLOT(slotPageNumberButtonClick(QString)));
        connect(m_PageNumberLabel[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    }

    m_nextPageCntrlBtn = new ControlButton(NEXT_BUTTON_1_INDEX,
                                           m_pageNoReadOnly->x() + m_pageNoReadOnly->width(),
                                           m_pageNoReadOnly->y(),
                                           BGTILE_SMALL_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           this, NO_LAYER, -1,
                                           "", true, DISPLAY_STG_NEXT_PAGE);
    m_elementList[DISPLAY_STG_NEXT_PAGE] = m_nextPageCntrlBtn;
    connect(m_nextPageCntrlBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_nextPageCntrlBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNextPrevPageClick(int)));

    m_lastPageCntrlBtn = new ControlButton(LAST_BUTTON_1_INDEX,
                                           m_nextPageCntrlBtn->x() + m_nextPageCntrlBtn->width(),
                                           m_nextPageCntrlBtn->y(),
                                           BGTILE_SMALL_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           this, NO_LAYER, -1,
                                           "", true, DISPLAY_STG_LAST_PAGE);
    m_elementList[DISPLAY_STG_LAST_PAGE] = m_lastPageCntrlBtn;
    connect(m_lastPageCntrlBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_lastPageCntrlBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNextPrevPageClick(int)));
}

bool DisplaySetting::setTVApperanceParameter()
{
    if(m_currentDisplayId == MAIN_DISPLAY)
    {
        Layout::tvAdjustParam = m_tvAdjustSpinBox->getIndexofCurrElement();
        return m_applController->writeTVApperanceParameters(Layout::tvAdjustParam);
    }
    return false;
}

bool DisplaySetting::saveBandwidthOptimizeFlag()
{
    m_applController->writeBandwidthOptFlag(((m_bandwidthOpt->getCurrentState() == ON_STATE) ? true : false));
    return true;
}

void DisplaySetting::saveLiveViewType(void)
{
    m_applController->SetLiveViewType((LIVE_VIEW_TYPE_e)m_LiveViewDropdown->getIndexofCurrElement());
}

void DisplaySetting::displayLayoutcreatorForCurrentPage()
{
    QString tempCamName = "";
    QString stringToDisplay;
    quint8 tempChannelId = 0;
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;
    Layout::getFirstAndLastWindow(m_currPage, m_currLayout, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        stringToDisplay = "";
        for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if(m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
            {
                stringToDisplay = m_applController->GetDispDeviceName(QString(m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName));
                break;
            }
        }

        if((stringToDisplay != "") && (Layout::streamInfoArray[m_currentDisplayId][windowIndex].m_videoType != VIDEO_TYPE_INSTANTPLAYBACKSTREAM))
        {
            if(windowIndex < MAX_CHANNEL_FOR_SEQ && channelIndex < MAX_WIN_SEQ_CAM)
            {
                tempChannelId = m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel;
                tempCamName = Layout::getCameraNameOfDevice(stringToDisplay, (tempChannelId - 1));
                stringToDisplay.append("\n");
                stringToDisplay.append(INT_TO_QSTRING(tempChannelId) + ":");
                stringToDisplay.append(tempCamName);
            }
        }

        m_layoutCreator->setWinHeaderForDispSetting(windowIndex, stringToDisplay);

        if(isMultipleChannelAssigned(windowIndex))
        {
            m_layoutCreator->updateSequenceImageType(windowIndex, MULTIPLE_CHANNEL_ASSIGN);
        }
        else
        {
            m_layoutCreator->updateSequenceImageType(windowIndex, NONE_CHANNEL_ASSIGN);
        }
    }
}

void DisplaySetting::getDefaultStyle()
{
    QList<QVariant> paramList;

    paramList.append (READ_DFLTSTYLE_ACTIVITY);
    paramList.append(m_currentDisplayId);
    m_applController->processActivity(DISPLAY_SETTING, paramList, &m_currDfltStyle);
}

void DisplaySetting::getConfig()
{
    quint8 channelIndex;
    m_isChangeDone = false;

    if(m_currentStyle == MAX_STYLE_TYPE)
    {
        m_displayConfig[m_currentDisplayId] = Layout::currentDisplayConfig[m_currentDisplayId];
        m_dfltOptSelBtn->changeState(OFF_STATE);
    }
    else
    {
        QList<QVariant> paramList;

        paramList.append(READ_DISP_ACTIVITY);
        paramList.append(m_currentDisplayId);
        paramList.append(m_currentStyle);

        if(m_applController->processActivity(DISPLAY_SETTING, paramList, &m_displayConfig[m_currentDisplayId]) == true)
        {
            for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
            {
                channelIndex = m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].currentChannel;
                if((strcmp(m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName, LOCAL_DEVICE_NAME) == 0)
                        && (m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel > m_devTable.totalCams))
                {
                    m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                    m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                }

                if(isMultipleChannelAssigned(windowIndex) == false)
                {
                    m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].sequenceStatus = false;
                }
            }

            m_dfltOptSelBtn->changeState((m_currentStyle == m_currDfltStyle) ? ON_STATE : OFF_STATE);
        }
    }

    m_currLayout = m_displayConfig[m_currentDisplayId].layoutId;
    m_currPage = m_displayConfig[m_currentDisplayId].currPage;
    m_currentWindow = m_displayConfig[m_currentDisplayId].selectedWindow;

    if(m_displayConfig[m_currentDisplayId].seqInterval < 10)
    {
         m_displayConfig[m_currentDisplayId].seqInterval = 10;
    }

    m_seqIntervalSpinbox->setIndexofCurrElement(m_displayConfig[m_currentDisplayId].seqInterval - 10);
    m_seqOptSelBtn->changeState((m_displayConfig[m_currentDisplayId].seqStatus == true) ? ON_STATE : OFF_STATE);

    quint16 totalPages = ((maxWindows % windowPerPage[m_currLayout] == 0)
                          ? maxWindows / windowPerPage[m_currLayout] : ((maxWindows / windowPerPage[m_currLayout]) + 1));
    quint16 tempPageNum = 0;

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        quint8 tempIndex = MAX_PAGE_NUMBER;
        tempPageNum = m_currPage - (m_currPage % MAX_PAGE_NUMBER);

        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));

        tempIndex = (m_currPage % MAX_PAGE_NUMBER);
        if((index == tempIndex) && ((tempPageNum + index) < totalPages))
        {
            m_PageNumberLabel[index]->setBackGroundColor(CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor(HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(true);
            m_PageNumberLabel[index]->forceActiveFocus();
            m_currElement = (DISPLAY_STG_PAGE_NUM_BTN + index);
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

    if(totalPages < MAX_PAGE_NUMBER)
    {
        for(quint8 index = totalPages; index < MAX_PAGE_NUMBER; index++)
        {
            m_PageNumberLabel[index]->setVisible(false);
            m_PageNumberLabel[index]->setIsEnabled(false);
        }
    }
    else
    {
        for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
        {
            m_PageNumberLabel[index]->setVisible(true);
            m_PageNumberLabel[index]->setIsEnabled(true);
        }
    }

    m_layoutCreator->setLayoutStyle(m_currLayout, m_currentWindow);
    displayLayoutcreatorForCurrentPage();

    m_cameraList->setCurrentDisplayConfig(m_displayConfig[m_currentDisplayId]);
    m_bandwidthOpt->changeState((m_applController->readBandwidthOptFlag() == true) ? ON_STATE:OFF_STATE);
}

void DisplaySetting::updatePageNumbers()
{
    quint8 tempIndex = 0;
    quint16 tempPageNum = 0;
    quint16 totalPages = 0;

    if((m_currLayout >= 0) && (m_currLayout < MAX_LAYOUT))
    {
        totalPages = ((maxWindows % windowPerPage[m_currLayout] == 0)
                      ? maxWindows / windowPerPage[m_currLayout] : ((maxWindows / windowPerPage[m_currLayout]) + 1));
    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        tempIndex = MAX_PAGE_NUMBER;
        tempPageNum = m_currPage - (m_currPage % MAX_PAGE_NUMBER);

        if(nextPageSelected == true)
        {
            if(((totalPages - m_currPage) % MAX_PAGE_NUMBER) != 0)
            {
                if(totalPages < MAX_PAGE_NUMBER)
                {
                    tempIndex = (totalPages - ((totalPages - m_currPage) % MAX_PAGE_NUMBER));
                    if((tempPageNum + index) < totalPages)
                    {
                        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                    }
                    else
                    {
                        m_PageNumberLabel[index]->changeText("");
                    }
                }
                else
                {
                    tempIndex = (m_currPage % MAX_PAGE_NUMBER);
                    if((m_currPage % MAX_PAGE_NUMBER) == 0)
                    {
                        tempIndex = 0;
                    }

                    if((tempPageNum + index) < totalPages)
                    {
                        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                    }
                    else
                    {
                        m_PageNumberLabel[index]->changeText("");
                    }
                }
            }
            else
            {
                tempIndex = (m_currPage % MAX_PAGE_NUMBER);
                if((tempPageNum + index) < totalPages)
                {
                    m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                }
                else
                {
                    m_PageNumberLabel[index]->changeText("");
                }
            }
        }
        else
        {
            if((totalPages % MAX_PAGE_NUMBER) == 0)
            {
                if(((totalPages - m_currPage - 1) % MAX_PAGE_NUMBER) != 0)
                {
                    if(totalPages < MAX_PAGE_NUMBER)
                    {
                        tempIndex = (totalPages - ((totalPages - m_currPage) % MAX_PAGE_NUMBER));
                        if((tempPageNum + index) < totalPages)
                        {
                            m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                        }
                        else
                        {
                            m_PageNumberLabel[index]->changeText("");
                        }
                    }
                    else
                    {
                        tempIndex = (MAX_PAGE_NUMBER - ((totalPages - m_currPage) % MAX_PAGE_NUMBER));
                        if((tempPageNum + index) < totalPages)
                        {
                            m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg((tempPageNum + 1 + index)) + QString(" "));
                        }
                        else
                        {
                            m_PageNumberLabel[index]->changeText("");
                        }
                    }

                    if(((m_currPage % MAX_PAGE_NUMBER) == 0) && (tempIndex == MAX_PAGE_NUMBER))
                    {
                        tempIndex = 0;
                    }
                }
                else
                {
                    tempIndex = (m_currPage % MAX_PAGE_NUMBER);
                    m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg((m_currPage + 1) - (MAX_PAGE_NUMBER - index) + 1) + QString(" "));
                }
            }
            else
            {
                if(((totalPages - m_currPage) % MAX_PAGE_NUMBER) != 0)
                {
                    if(totalPages < MAX_PAGE_NUMBER)
                    {
                        tempIndex = (totalPages - ((totalPages - m_currPage) % MAX_PAGE_NUMBER));
                        if((tempPageNum + index) < totalPages)
                        {
                            m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                        }
                        else
                        {
                            m_PageNumberLabel[index]->changeText("");
                        }
                    }
                    else
                    {
                        tempIndex = (m_currPage % MAX_PAGE_NUMBER);
                        if((tempPageNum + index) < totalPages)
                        {
                            m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg((tempPageNum + 1 + index)) + QString(" "));
                        }
                        else
                        {
                            m_PageNumberLabel[index]->changeText("");
                        }
                    }

                    if(((m_currPage % MAX_PAGE_NUMBER) == 0) && (tempIndex == MAX_PAGE_NUMBER))
                    {
                        tempIndex = 0;
                    }
                }
                else
                {
                    tempIndex = (m_currPage % MAX_PAGE_NUMBER);
                    if((tempPageNum + index) < totalPages)
                    {
                        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg((tempPageNum + 1 + index)) + QString(" "));
                    }
                    else
                    {
                        m_PageNumberLabel[index]->changeText("");
                    }
                }
            }
        }

        if((index == tempIndex) && ((tempPageNum + index) < totalPages))
        {
            m_PageNumberLabel[index]->setBackGroundColor(CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor(HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(true);
            m_PageNumberLabel[index]->forceActiveFocus();
            m_currElement = (DISPLAY_STG_PAGE_NUM_BTN + index);
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
}

void DisplaySetting::saveConfig(quint8 styleToSave)
{
    QList<QVariant> paramList;

    m_processBar->loadProcessBar();
    m_isChangeDone = false;

    setTVApperanceParameter();
    saveLiveViewType();
    saveBandwidthOptimizeFlag();

    paramList.append(WRITE_DISP_ACTIVITY);
    paramList.append( m_currentDisplayId);
    paramList.append(styleToSave);
    paramList.append(m_dfltOptSelBtn->getCurrentState());

    m_displayConfig[m_currentDisplayId].seqInterval = (m_seqIntervalSpinbox->getIndexofCurrElement() + 10);
    m_displayConfig[m_currentDisplayId].seqStatus = ((m_seqOptSelBtn->getCurrentState() == OFF_STATE) ? false : true);
    m_displayConfig[m_currentDisplayId].selectedWindow = m_currentWindow;
    m_displayConfig[m_currentDisplayId].currPage = m_currPage;
    m_displayConfig[m_currentDisplayId].layoutId = m_currLayout;

    if(m_applController->processActivity(DISPLAY_SETTING, paramList, &m_displayConfig[m_currentDisplayId]) == true)
    {
        m_processBar->unloadProcessBar();
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SETTING_SAVE_SUCCESS));
    }
    else
    {
        m_processBar->unloadProcessBar();
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ERROR_SAVE_MSG));
    }
    getDefaultStyle();
}

void DisplaySetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != LOCAL_DEVICE_NAME)
    {
        return;
    }

    if (param->msgType != MSG_SET_CMD)
    {
        return;
    }

    if (param->cmdType != VALIDATE_USER_CRED)
    {
        return;
    }

    if (param->deviceStatus == CMD_SUCCESS)
    {
        EPRINT(LAYOUT, "display resolution config changed, restart UI application: [resolution=%d --> %d]", prevResolution, currResolution);
        m_applController->writeResolution((DISPLAY_TYPE_e)m_currentdisplayIndex, currResolution);
        kill(getpid(), SIGTERM);
    }
    else
    {
        m_resolutionPicklist->changeValue(m_applController->getCurrentDisplayResolution());
        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
    }
}

void DisplaySetting::applyChangesInStyle()
{
    m_processBar->loadProcessBar();
    m_displayConfig[m_currentDisplayId].layoutId = m_currLayout;
    m_displayConfig[m_currentDisplayId].seqInterval = (m_seqIntervalSpinbox->getIndexofCurrElement() + 10);
    m_displayConfig[m_currentDisplayId].seqStatus = ((m_seqOptSelBtn->getCurrentState() == OFF_STATE) ? false : true);
    m_displayConfig[m_currentDisplayId].selectedWindow = m_currentWindow;
    m_displayConfig[m_currentDisplayId].currPage = m_currPage;

    if((m_currentDisplayId == MAIN_DISPLAY) && (m_currentStyle == MAX_STYLE_TYPE)
            && (m_displayConfig[m_currentDisplayId].seqStatus == true) && (Layout::isPlaybackRunning()))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAYOUT_PB_ERROR_MESSAGE));
        m_processBar->unloadProcessBar();
        return;
    }

    saveLiveViewType();
    saveBandwidthOptimizeFlag();
    if((m_displayConfig[m_currentDisplayId].seqStatus == true) && (isWindowWiseSequeningConfigure()))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAYOUT_WINDOW_SEQENC_ERROR_MESSAGE));
        m_processBar->unloadProcessBar();
        return;
    }

    if(m_currentDisplayId == MAIN_DISPLAY)
    {
        emit sigToolbarStyleChnageNotify(m_currentStyle);
    }

    emit sigProcessApplyStyleToLayout(m_currentDisplayId, m_displayConfig[m_currentDisplayId], m_currentStyle);
}


void DisplaySetting::audioStopOnApplyCondition()
{
    if(m_tempStartAudioInWindow != MAX_CHANNEL_FOR_SEQ)
    {
        emit sigliveViewAudiostop();
        m_tempStartAudioInWindow = MAX_CHANNEL_FOR_SEQ;
    }
    else if(pageSequenceStatus == true)
    {
        pageSequenceStatus = false;
        emit sigliveViewAudiostop();
    }
}

void DisplaySetting::updateDeviceState(QString devName, DEVICE_STATE_TYPE_e devState)
{
    quint16 tempWindowCount = m_applController->GetTotalCameraOfEnableDevices();
    
    m_cameraList->updateDeviceCurrentState(devName, devState);
    if(m_windowSequenceSetting != NULL)
    {
        m_windowSequenceSetting->updateDeviceState(devName, devState);
    }

    if(maxWindows < tempWindowCount)
    {             
        maxWindows = tempWindowCount;
        m_layoutCreator->setMaxWinodws(maxWindows);
        m_layoutCreator->setLayoutStyle(m_currLayout, m_currentWindow);
        updatePageNumbers();
    }

    if((devState == CONFLICT) || (devState == DELETED))
    {
        for(quint8 displayIdx = 0; displayIdx < MAX_DISPLAY_TYPE; displayIdx++)
        {
            for(quint16 windowIndex = 0; windowIndex < maxWindows; windowIndex++)
            {
                for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
                {
                    if(strcmp(m_displayConfig[displayIdx].windowInfo[windowIndex].camInfo[channelIndex].deviceName, devName.toLatin1().constData()) == 0)
                    {                            
                        m_displayConfig[displayIdx].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                        m_displayConfig[displayIdx].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                    }
                }

                if(isMultipleChannelAssigned(windowIndex) == false)
                {
                    m_displayConfig[displayIdx].windowInfo[windowIndex].sequenceStatus = false;
                }
            }
        }
    }

    displayLayoutcreatorForCurrentPage();
}

void DisplaySetting::doChangeDisplayTypeProcess()
{
    m_currentDisplayId = MAIN_DISPLAY;
    m_cameraList->setCurrentDisplayType(m_currentDisplayId);

    m_stylePicklist->changeValue(0);
    m_currentStyle = MAX_STYLE_TYPE;
    m_cameraList->setCurrentStyle(m_currentStyle);

    getDefaultStyle();
    getConfig();
    m_resolutionPicklist->changeValue(m_applController->getCurrentDisplayResolution());
}

bool DisplaySetting::findWindowIndexOfDisplayInfo(quint8 cameraIndex, QString deviceName, quint16 &windowIndex, quint8 &channelIndex)
{
    for(windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if((strcmp(deviceName.toUtf8().constData(), m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName) == 0)
                    && (cameraIndex == m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel))
            {
                return true;
            }
        }
    }

    return false;
}

bool DisplaySetting::findFreeWindowOfDisplayInfo(quint16 &windowIndex)
{
    quint8 channelIndex;

    for(windowIndex = m_currentWindow; ;)
    {
        channelIndex = m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].currentChannel;
        if(channelIndex < MAX_WIN_SEQ_CAM)
        {
            if((m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
                    && (m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel == INVALID_CAMERA_INDEX))
            {
                return true;
            }
        }

        windowIndex = ((windowIndex + 1) % maxWindows);
        if(windowIndex == m_currentWindow)
        {
            break;
        }
    }

    return false;
}

void DisplaySetting::windowCloseButtonClicked(quint16 windowIndex)
{
    m_currentWindow = windowIndex;
    m_layoutCreator->changeSelectedWindow(windowIndex, true);
    m_layoutCreator->setWinHeaderForDispSetting(windowIndex, "");

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
        m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
    }

    m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].currentChannel = 0;
    m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].sequenceStatus = false;
    m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;
    m_layoutCreator->updateSequenceImageType(windowIndex, NONE_CHANNEL_ASSIGN);
    m_cameraList->setCurrentDisplayConfig(m_displayConfig[m_currentDisplayId]);
}

void DisplaySetting::sequenceConfigButtonClicked(quint16 windowIndex)
{
    if(m_windowSequenceSetting == NULL)
    {
        m_currentWindow = windowIndex;
        m_layoutCreator->changeSelectedWindow(windowIndex, true);

        m_windowSequenceSetting = new WindowSequenceSettings(m_currentWindow,
                                                             &m_displayConfig[m_currentDisplayId],
                                                             m_isChangeDone,
                                                             this);
        connect(m_windowSequenceSetting,
                SIGNAL(sigObjectDelete()),
                this,
                SLOT(slotObjectDelete()));
    }
}

bool DisplaySetting::isMultipleChannelAssigned(quint16 windowIndex)
{
    quint8 assignCount = 0;

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        if((m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
                && (m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
        {
            assignCount++;
        }

        if(assignCount >= 2)
        {
            return true;
        }
    }

    return false;
}

bool DisplaySetting::isWindowWiseSequeningConfigure()
{
    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if((m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].sequenceStatus) && (isMultipleChannelAssigned(windowIndex)))
        {
            return true;
        }
    }

    return false;
}

void DisplaySetting::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void DisplaySetting::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = DISPLAY_STG_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus();
}

void DisplaySetting::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if((m_appearanceSetting == NULL) && (m_windowSequenceSetting == NULL))
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void DisplaySetting::takeLeftKeyAction()
{
    do
    {
        if (m_currElement == 0)
        {
            m_currElement = MAX_DISPLAY_STG_ELEMENTS;
        }

        if (m_currElement)
        {
            m_currElement = (m_currElement - 1);
        }
        else
        {
              return;
        }

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    switch(m_currElement)
    {
        case DISPLAY_STG_LAYOUT_LIST:
        case DISPLAY_STG_LAYOUT_CREATOR:
        case DISPLAY_STG_CAMERA_LIST:
            m_elementList[m_currElement]->forceFocusToPage(true);
            break;

        default:
            m_elementList[m_currElement]->forceActiveFocus();
            break;
    }
}

void DisplaySetting::takeRightKeyAction()
{
    do
    {
        if(m_currElement == (MAX_DISPLAY_STG_ELEMENTS - 1))
        {
            m_currElement = -1;
        }

        if(m_currElement != (MAX_DISPLAY_STG_ELEMENTS - 1))
        {
            m_currElement = (m_currElement + 1);
        }
        else
        {
              return;
        }

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    switch(m_currElement)
    {
        case DISPLAY_STG_LAYOUT_LIST:
        case DISPLAY_STG_LAYOUT_CREATOR:
        case DISPLAY_STG_CAMERA_LIST:
            m_elementList[m_currElement]->forceFocusToPage(true);
            break;

        default:
            m_elementList[m_currElement]->forceActiveFocus();
            break;
    }
}

void DisplaySetting::slotUpadateCurrentElement(int index)
{
    m_currElement = index;
}

void DisplaySetting::slotPickListButtonClick(int index)
{
    switch(index)
    {
        case DISPLAY_STG_RESOLUTION_PICKLIST:
        {
            m_resolutionPicklist->loadPickListOnResponse(displayResolutionMapList, prevResolution);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void DisplaySetting::slotSettingOptSelButtonClicked(OPTION_STATE_TYPE_e, int index)
{
    switch(index)
    {
        case DISPLAY_STG_DEFAUILT_SEL:
        case DISPLAY_STG_SEQ_ENABLE:
            m_isChangeDone = true;
            break;

        default:
            break;
    }
}

void DisplaySetting::slotPicklistValueChanged(quint8 key, QString, int indexInPage)
{
    switch(indexInPage)
    {
        case DISPLAY_STG_RESOLUTION_PICKLIST:
            currResolution = (DISPLAY_RESOLUTION_e)key;
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(DISP_SETTING_RESTART_UI_APPL_NOTICE), true);
            break;

        case DISPLAY_STG_LAYOUT_STYLE_PICKLIST:
            if(key > 0)
            {
                m_currentStyle = (STYLE_TYPE_e)(key - 1);
            }
            else
            {
                m_currentStyle = MAX_STYLE_TYPE;
            }
            m_cameraList->setCurrentStyle(m_currentStyle);
            getConfig();
            break;

        default:
            break;
    }
}

void DisplaySetting::slotInfoPageCnfgBtnClick(int index)
{
    //Give focus before other task coz there are chances to again load info page
    // and in that case focus should be on info page not on current element
    m_elementList[m_currElement]->forceActiveFocus();

    if(m_infoPage->getText() == ValidationMessage::getValidationMessage(DISP_SETTING_SAVE_MSG))
    {
        if(index == INFO_OK_BTN)
        {
            slotCnfgButtonClick(DISPLAY_STG_SAVE_BTN);
        }
        else
        {
            doChangeDisplayTypeProcess();
        }
    }
    else if(m_infoPage->getText() == ValidationMessage::getValidationMessage(DISP_SETTING_RESTART_UI_APPL_NOTICE))
    {
        if(index == INFO_OK_BTN)
        {
            m_backGroundImage = new Image(0, 0,
                                          m_imageSource,
                                          this,
                                          START_X_START_Y,
                                          0,
                                          false,
                                          true);
            m_backGroundImage->setVisible(true);

            m_userValidation = new UsersValidation(this, PAGE_ID_LIVE_VIEW);
            connect(m_userValidation,
                    SIGNAL(sigOkButtonClicked(QString,QString)),
                    this,
                    SLOT(slotOkButtonClicked(QString,QString)),
                    Qt::QueuedConnection);
        }
        else
        {
            m_resolutionPicklist->changeValue(m_applController->getCurrentDisplayResolution());
        }
    }
}

void DisplaySetting::slotChangeLayout(LAYOUT_TYPE_e layoutId)
{
    m_isChangeDone = true;
    m_currLayout = layoutId;

    quint16 tempPageNum = (m_currentWindow / windowPerPage[m_currLayout]);

    nextPageSelected = (tempPageNum < m_currPage);
    m_currPage = tempPageNum;
    m_layoutCreator->setLayoutStyle(m_currLayout, m_currentWindow);
    updatePageNumbers();

    quint16 totalPages = ((maxWindows % windowPerPage[m_currLayout] == 0)
                          ? maxWindows / windowPerPage[m_currLayout] : ((maxWindows / windowPerPage[m_currLayout])+ 1));

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        if(index < totalPages)
        {
            m_PageNumberLabel[index]->setVisible(true);
            m_PageNumberLabel[index]->setIsEnabled(true);
            m_PageNumberLabel[index]->update();
        }
        else
        {
            m_PageNumberLabel[index]->setVisible(false);
            m_PageNumberLabel[index]->setIsEnabled(false);
        }
    }

    displayLayoutcreatorForCurrentPage();
}

void DisplaySetting::slotNextPrevPageClick(int index)
{
    m_isChangeDone = true;
    quint16 maxPages = ((maxWindows % windowPerPage[m_currLayout] == 0)
                        ? maxWindows / windowPerPage[m_currLayout] : ((maxWindows / windowPerPage[m_currLayout])+ 1));
    quint16 tempPageNum = 0;

    switch(index)
    {
        case DISPLAY_STG_FIRST_PAGE:
            m_currPage = 0;
            for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
            {
                tempPageNum = m_currPage - (m_currPage % MAX_PAGE_NUMBER);
                m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                m_PageNumberLabel[index]->update();
            }
            break;

        case DISPLAY_STG_PREV_PAGE:
            m_currPage = (m_currPage - 1 + maxPages) % maxPages;
            nextPageSelected = false;
            break;

        case DISPLAY_STG_NEXT_PAGE:
            nextPageSelected = true;
            m_currPage = (m_currPage + 1) % maxPages;
            break;

        case DISPLAY_STG_LAST_PAGE:
            m_currPage = (maxPages - 1);
            for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
            {
                tempPageNum = m_currPage - (m_currPage % MAX_PAGE_NUMBER);
                m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(tempPageNum  + 1 + index) + QString(" "));
                m_PageNumberLabel[index]->update();
            }
            break;
    }

    m_currentWindow = (m_currPage * windowPerPage[m_currLayout]);
    m_layoutCreator->setLayoutStyle(m_currLayout, m_currentWindow);
    updatePageNumbers();
    displayLayoutcreatorForCurrentPage();
}

void DisplaySetting::slotCnfgButtonClick(int index)
{
    switch(index)
    {
        case DISPLAY_STG_APPLY_BTN:
            audioStopOnApplyCondition();
            applyChangesInStyle();
            m_elementList[m_currElement]->forceActiveFocus();
            break;

        case DISPLAY_STG_SAVE_BTN:
            if((m_displayConfig[m_currentDisplayId].seqStatus == true) && (isWindowWiseSequeningConfigure()))
            {
                m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(DISP_SETTING_CONFI_AUTO_PAGE_WINDOW_SEQ));
                m_processBar->unloadProcessBar();
                return;
            }

            if(m_currentStyle == MAX_STYLE_TYPE)
            {
                if(m_styleSelectionOpt == NULL)
                {
                    m_styleSelectionOpt = new StyleSelectionOpt(this);
                    connect(m_styleSelectionOpt,
                            SIGNAL(sigStyleSelCnfgBtnClick(int,quint8)),
                            this,
                            SLOT(slotStyleSelCnfgBtnClick(int,quint8)));
                }
            }
            else
            {
                saveConfig(m_currentStyle);
                m_elementList[m_currElement]->forceActiveFocus();
            }
            break;

        case DISPLAY_STG_CANCEL_BTN:
            getConfig();
            break;

        default:
            break;
    }
}

void DisplaySetting::slotStyleSelCnfgBtnClick(int index, quint8 styleNo)
{
    if (IS_VALID_OBJ(m_styleSelectionOpt))
    {
        disconnect(m_styleSelectionOpt,
                   SIGNAL(sigStyleSelCnfgBtnClick(int,quint8)),
                   this,
                   SLOT(slotStyleSelCnfgBtnClick(int,quint8)));
        DELETE_OBJ(m_styleSelectionOpt);
    }

    if(index == STYLE_SEL_OK_BTN)
    {
        saveConfig(styleNo);
    }

    m_elementList[m_currElement]->forceActiveFocus();
}

void DisplaySetting::slotWindowSelected(quint16 windowIndex)
{
    m_currentWindow = windowIndex;
    m_layoutCreator->changeSelectedWindow(windowIndex, true);
}

void DisplaySetting::slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex)
{
    switch(imageType)
    {
        case WINDOW_CLOSE_BUTTON:
            windowCloseButtonClicked(windowIndex);
            break;

        case WINDOW_SEQUENCE_BUTTON:
            sequenceConfigButtonClicked(windowIndex);
            break;

        default:
            break;
    }
}

void DisplaySetting::slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover)
{
    switch(imageType)
    {
        case WINDOW_CLOSE_BUTTON:
            m_layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
            break;

        case WINDOW_SEQUENCE_BUTTON:
            m_layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
            break;

        default:
            break;
    }
}

void DisplaySetting::slotSwapWindows(quint16 firstWindow, quint16 secondWindow)
{
    if((Layout::startAudioInWindow == firstWindow) || (Layout::startAudioInWindow == secondWindow))
    {
        m_tempStartAudioInWindow = Layout::startAudioInWindow;
    }

    m_isChangeDone = true;
    m_currentWindow =  secondWindow;
    m_layoutCreator->changeSelectedWindow(secondWindow, true);

    QString tempDeviceName;
    quint8 tempChannel = 0;
    QString stringToDisplay;
    quint8 firstChannelIndex, secondChannelIndex;
    WINDOW_INFO_t tempConfig;

    firstChannelIndex = m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].currentChannel;
    secondChannelIndex = m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].currentChannel;

    tempConfig = m_displayConfig[m_currentDisplayId].windowInfo[firstWindow];
    m_displayConfig[m_currentDisplayId].windowInfo[firstWindow] = m_displayConfig[m_currentDisplayId].windowInfo[secondWindow];
    m_displayConfig[m_currentDisplayId].windowInfo[secondWindow] = tempConfig;

    if (m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[firstChannelIndex].deviceName[0] != '\0')
    {
        tempDeviceName = QString(m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[firstChannelIndex].deviceName);
        tempChannel = m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[firstChannelIndex].defChannel;
        stringToDisplay = m_applController->GetDispDeviceName(tempDeviceName);
        stringToDisplay.append("\n");
        stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
        stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
    }
    else
    {
        stringToDisplay = "";

        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if((m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[channelIndex].deviceName[0] != '\0')
                    && (m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
            {
                tempDeviceName = QString(m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[channelIndex].deviceName);
                tempChannel = m_displayConfig[m_currentDisplayId].windowInfo[firstWindow].camInfo[channelIndex].defChannel;
                stringToDisplay = m_applController->GetDispDeviceName(tempDeviceName);
                stringToDisplay.append("\n");
                stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
                stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
                break;
            }
        }
    }

    m_layoutCreator->setWinHeaderForDispSetting(firstWindow, stringToDisplay);

    if(isMultipleChannelAssigned(firstWindow))
    {
        m_layoutCreator->updateSequenceImageType(firstWindow, MULTIPLE_CHANNEL_ASSIGN);
    }
    else
    {
        m_layoutCreator->updateSequenceImageType(firstWindow, NONE_CHANNEL_ASSIGN);
    }

    if(m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[secondChannelIndex].deviceName[0] != '\0')
    {
        tempDeviceName = QString(m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[secondChannelIndex].deviceName);
        tempChannel = m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[secondChannelIndex].defChannel;
        stringToDisplay = m_applController->GetDispDeviceName(tempDeviceName);
        stringToDisplay.append("\n");
        stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
        stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
    }
    else
    {
        stringToDisplay = "";
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if((m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[channelIndex].deviceName[0] != '\0')
                    && (m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
            {
                tempDeviceName = QString(m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[channelIndex].deviceName);
                tempChannel = m_displayConfig[m_currentDisplayId].windowInfo[secondWindow].camInfo[channelIndex].defChannel;
                stringToDisplay = m_applController->GetDispDeviceName(tempDeviceName);
                stringToDisplay.append("\n");
                stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
                stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
                break;
            }
        }
    }

    m_layoutCreator->setWinHeaderForDispSetting(secondWindow, stringToDisplay);

    if(isMultipleChannelAssigned(secondWindow))
    {
        m_layoutCreator->updateSequenceImageType(secondWindow, MULTIPLE_CHANNEL_ASSIGN);
    }
    else
    {
        m_layoutCreator->updateSequenceImageType(secondWindow, NONE_CHANNEL_ASSIGN);
    }

    if((QApplication::overrideCursor() != NULL) && (QApplication::overrideCursor()->shape() == Qt::OpenHandCursor))
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }

    m_cameraList->setCurrentDisplayConfig(m_displayConfig[m_currentDisplayId]);
}

void DisplaySetting::slotDragStartStopEvent(bool isStart)
{
    QApplication::setOverrideCursor(isStart ? Qt::OpenHandCursor : Qt::ArrowCursor);
}

void DisplaySetting::slotWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e displayId, QString deviceName, quint8 cameraId, quint16 windowId)
{
    quint16 tempwinId;
    quint8  tempChanelId;

    if ((m_currentStyle != MAX_STYLE_TYPE) || (m_currentDisplayId != displayId))
    {
        return;
    }

    if (false == m_cameraList->getCurrSelectedDeviceName(deviceName))
    {
        return;
    }

    if (false == findWindowIndexOfDisplayInfo(cameraId, deviceName, tempwinId, tempChanelId))
    {
        return;
    }

    CAMERA_STATE_TYPE_e newConnectionState = Layout::changeVideoStatusToCameraStatus(Layout::streamInfoArray[displayId][windowId].m_videoStatus);
    quint8 deviceIndex = m_cameraList->getIndexofdeivce(deviceName);
    m_cameraList->updateCameraCurrentState(deviceIndex, cameraId, newConnectionState);
}

void DisplaySetting::slotAppearanceButtonClick(int)
{
    if(m_appearanceSetting == NULL)
    {
        m_appearanceSetting = new AppearanceSetting(this);
        connect(m_appearanceSetting,
                SIGNAL(sigObjectDelete()),
                this,
                SLOT(slotObjectDelete()));
    }
}

void DisplaySetting::slotObjectDelete()
{
    if (IS_VALID_OBJ(m_appearanceSetting))
    {
        disconnect(m_appearanceSetting,
                   SIGNAL(sigObjectDelete()),
                   this,
                   SLOT(slotObjectDelete()));
        DELETE_OBJ(m_appearanceSetting);
    }
    else if (IS_VALID_OBJ(m_windowSequenceSetting))
    {
        disconnect(m_windowSequenceSetting,
                   SIGNAL(sigObjectDelete()),
                   this,
                   SLOT(slotObjectDelete()));
        DELETE_OBJ(m_windowSequenceSetting);

        displayLayoutcreatorForCurrentPage();
        m_cameraList->setCurrentDisplayConfig(m_displayConfig[m_currentDisplayId]);
    }

    m_elementList[m_currElement]->forceActiveFocus();
}

void DisplaySetting::slotLayoutResponseOnApply(DISPLAY_TYPE_e displayType, bool isCurrentStyle)
{
	if(displayType == m_currentDisplayId)
    {
        if(isCurrentStyle == false)
        {
            m_currentStyle = MAX_STYLE_TYPE;
            m_cameraList->setCurrentStyle(m_currentStyle);
            m_stylePicklist->changeValue(0);
        }
        m_cameraList->updateCurrDeviceCamStatus();
        m_processBar->unloadProcessBar();
    }
}

void DisplaySetting::slotCameraButtonClicked(quint8 cameraIndex, QString deviceName, CAMERA_STATE_TYPE_e connectionState,
                                             bool pageSwitchFlag, bool isChangeSelection)
{
    bool status = false;
    quint16 windowIndex;
    quint8 channelIndex;

    Q_UNUSED(pageSwitchFlag);

    switch(connectionState)
    {
        case CAM_STATE_NONE:
            status = findFreeWindowOfDisplayInfo(windowIndex);
            if(status == false)
            {
                break;
            }

            channelIndex = m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].currentChannel;
            if(channelIndex < MAX_WIN_SEQ_CAM)
            {
                snprintf(m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName, MAX_DEVICE_NAME_SIZE, "%s", deviceName.toUtf8().constData());
                m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel = cameraIndex;
            }
            break;

        case CAM_STATE_CONNECTING:
        case CAM_STATE_LIVE_STREAM:
        case CAM_STATE_RETRY:
        case CAM_STATE_ASSIGNED:
            status = findWindowIndexOfDisplayInfo(cameraIndex, deviceName, windowIndex, channelIndex);
            if(status == false)
            {
                break;
            }

            if(channelIndex < MAX_WIN_SEQ_CAM)
            {
                m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
            }

            if(isMultipleChannelAssigned(windowIndex) == false)
            {
                m_displayConfig[m_currentDisplayId].windowInfo[windowIndex].sequenceStatus = false;
            }
            break;

        default:
            break;
    }

    if(status == true)
    {
        m_isChangeDone = true;
        m_currentWindow = windowIndex;

        if(isChangeSelection)
        {
            m_layoutCreator->changeSelectedWindow(windowIndex, true);
        }
    }
}

void DisplaySetting::slotCameraConfigListUpdate()
{
    quint16 newPageId = 0;

    m_cameraList->setCurrentDisplayConfig(m_displayConfig[m_currentDisplayId]);
    newPageId = (m_currentWindow / windowPerPage[m_currLayout]);
    if((newPageId != m_currPage))
    {
        nextPageSelected = (newPageId < m_currPage) ? false : true;
        m_currPage = newPageId;
        m_layoutCreator->setLayoutStyle(m_currLayout, m_currentWindow);
    }

    updatePageNumbers();
    displayLayoutcreatorForCurrentPage();
}

void DisplaySetting::slotPageNumberButtonClick(QString str)
{
    if (str == "")
    {
        return;
    }

    quint16 tempPageNum = (str.toUInt() - 1);

    nextPageSelected = (tempPageNum < m_currPage) ? false : true;
    m_currPage = tempPageNum;
    m_currentWindow = (m_currPage * windowPerPage[m_currLayout]);
    m_layoutCreator->setLayoutStyle(m_currLayout, m_currentWindow);

    updatePageNumbers ();
    displayLayoutcreatorForCurrentPage();
}

void DisplaySetting::slotOkButtonClicked(QString username, QString password)
{
    m_username = username;
    m_password = password;

    if(IS_VALID_OBJ(m_userValidation))
    {
        disconnect(m_userValidation,
                   SIGNAL(sigOkButtonClicked(QString,QString)),
                   this,
                   SLOT(slotOkButtonClicked(QString,QString)));
        DELETE_OBJ(m_userValidation);
    }

    m_backGroundImage->setVisible(false);
    DELETE_OBJ(m_backGroundImage);

    if((m_username != "") && (m_password != ""))
    {
        restartLocalClient();
    }
    else
    {
        m_resolutionPicklist->changeValue(m_applController->getCurrentDisplayResolution());
    }
}

void DisplaySetting::restartLocalClient()
{
    m_payloadLib->setCnfgArrayAtIndex(0, m_username);
    m_payloadLib->setCnfgArrayAtIndex(1, m_password);

    DevCommParam *param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = VALIDATE_USER_CRED;
    param->payload = m_payloadLib->createDevCmdPayload(2);
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void DisplaySetting::slotSpinBoxValueChanged(QString,quint32 index)
{
    m_isChangeDone = true;
    if(index != DISPLAY_STG_TV_ADJUST)
    {
        return;
    }

    if(Layout::tvAdjustParam == m_tvAdjustSpinBox->getIndexofCurrElement())
    {
        return;
    }

    Layout::tvAdjustParam = m_tvAdjustSpinBox->getIndexofCurrElement();
    Layout::isRedraw = true;
    m_applController->setTVApperanceParameters(Layout::tvAdjustParam);
    DISPLAY_CONFIG_t displayConfig = Layout::currentDisplayConfig[MAIN_DISPLAY];
    emit sigProcessApplyStyleToLayout(MAIN_DISPLAY, displayConfig, MAX_STYLE_TYPE);
}

void DisplaySetting::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void DisplaySetting::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void DisplaySetting::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
    slotCnfgButtonClick(DISPLAY_STG_SAVE_BTN);
}
