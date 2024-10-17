#include "PTZControl.h"
#include "ApplicationMode.h"
#include "ValidationMessage.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

#define PTZ_CTRL_WIDTH                          SCALE_WIDTH(435)
#define PTZ_CTRL_HEIGHT                         SCALE_HEIGHT(540)
#define PTZ_CTRL_HEADING                        "PTZ Control"

#define PTZ_CTRL_FOCUS_IMAGE_PATH               IMAGE_PATH "PTZControl/Focus/"
#define PTZ_CTRL_IRIS_IMAGE_PATH                IMAGE_PATH "PTZControl/Iris/"
#define PTZ_CTRL_ZOOM_IMAGE_PATH                IMAGE_PATH "PTZControl/Zoom/Button_1.png"
#define PTZ_CTRL_IN_IMAGE_PATH                  IMAGE_PATH "PTZControl/In/"
#define PTZ_CTRL_OUT_IMAGE_PATH                 IMAGE_PATH "PTZControl/Out/"

#define PTZ_CTRL_TOP_KEY_IMAGE_PATH             IMAGE_PATH "PTZControl/PtzKeyControl/Top/"
#define PTZ_CTRL_BOTTOM_KEY_IMAGE_PATH          IMAGE_PATH "PTZControl/PtzKeyControl/Bottom/"
#define PTZ_CTRL_LEFT_KEY_IMAGE_PATH            IMAGE_PATH "PTZControl/PtzKeyControl/Left/"
#define PTZ_CTRL_RIGHT_KEY_IMAGE_PATH           IMAGE_PATH "PTZControl/PtzKeyControl/Right/"

#define PTZ_CTRL_TOP_LEFT_KEY_IMAGE_PATH        IMAGE_PATH "PTZControl/PtzKeyControl/TopLeft/"
#define PTZ_CTRL_TOP_RIGHT_KEY_IMAGE_PATH       IMAGE_PATH "PTZControl/PtzKeyControl/TopRight/"
#define PTZ_CTRL_BOTTOM_LEFT_KEY_IMAGE_PATH     IMAGE_PATH "PTZControl/PtzKeyControl/BottomLeft/"
#define PTZ_CTRL_BOTTOM_RIGHT_KEY_IMAGE_PATH    IMAGE_PATH "PTZControl/PtzKeyControl/BottomRight/"

#define DEFAULT_PTZ_SPEED   5
#define MAX_PTZ_SPEED       10

typedef enum
{
    PTZ_PAN_LEFT = 0,
    PTZ_PAN_RIGHT,
    MAX_PTZ_PAN_OPTION,

    PTZ_TILT_UP = 0,
    PTZ_TILT_DOWN,
    MAX_PTZ_TILT_OPTION,

    PTZ_ZOOM_OUT = 0,
    PTZ_ZOOM_IN,
    MAX_PTZ_ZOOM_OPTION

}PTZ_ACTION_e;

typedef enum
{
    IRIS_CLOSE = 0,	// dark, -, close
    IRIS_OPEN,		// bright, +, open
    IRIS_AUTO,		// auto
    MAX_IRIS_OPTION

}IRIS_ACTION_e;

typedef enum
{
    FOCUS_FAR = 0,
    FOCUS_NEAR,
    FOCUS_AUTO,
    MAX_FOCUS_OPTION

}FOCUS_ACTION_e;

static const QString ptzControlStrings[PTZ_CTRL_LABLE_MAX] =
{
    "Resume Paused Tour",
    "Resume",
    "Manual Tour",
    "Start",
    "Stop",
    "Speed",
    "Preset Position Settings",
    "Position Number",
    "Go",
    "Position Name",
    "Save",
    "Delete"
};

static const QString ptzCtrlToolTipStrings[] =
{
    "Zoom Out", "Zoom In",
    "Focus",    "Focus Far",    "Focus Near",
    "Iris",     "Iris Close",   "Iris Open",
};

PTZControl::PTZControl(qint32 startx, qint32 starty, QString deviceName, quint8 cameraNum, QWidget *parent)
    : Rectangle(0, 0, PTZ_CTRL_WIDTH, PTZ_CTRL_HEIGHT, SCALE_WIDTH(RECT_RADIUS),
                BORDER_2_COLOR, NORMAL_BKG_COLOR, parent, 1),
      currentDeviceName(deviceName), startX(startx), startY(starty), m_cameraIndex(cameraNum), m_currentPositionIndex(0)
{
    this->setGeometry(QRect(startX, startY, PTZ_CTRL_WIDTH, PTZ_CTRL_HEIGHT));

    currentButtonClick = 0;
    m_positionNameList.clear();
    createDefaultComponent();
    getManualTourStatus();
    getPositionConfig();

    m_currentElement = PTZ_CTRL_SET_POS_DROPDOWNBOX;
    m_elementList[m_currentElement]->forceActiveFocus();

    this->show();
}

PTZControl::~PTZControl()
{
    delete payloadLib;

    disconnect(closeButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(closeButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    delete ptzControlHeading;

    delete resumeButtonTile;
    delete resumeTourLabel;
    disconnect(resumeTourButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(resumeTourButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    delete resumeTourButton;

    delete manualButtonTile;
    delete manualButtonLabel;
    disconnect(manualButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(manualButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    delete manualButton;

    delete topBgTile;
    disconnect(topKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(topKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete topKeyImageControl;

    disconnect(topLeftKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(topLeftKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete topLeftKeyImageControl;

    disconnect(topRightKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(topRightKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete topRightKeyImageControl;

    disconnect(leftKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(leftKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete leftKeyImageControl;

    disconnect(rightKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(rightKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete rightKeyImageControl;

    disconnect(bottomKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(bottomKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete bottomKeyImageControl;

    disconnect(bottomLeftKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(bottomLeftKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete bottomLeftKeyImageControl;

    disconnect(bottomRightKeyImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(bottomRightKeyImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete bottomRightKeyImageControl;

    disconnect(speedDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete speedDropDownBox;

    delete imageTopBgTile;
    disconnect(zoomImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete zoomImageControl;

    disconnect(zoomOutImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(zoomOutImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(zoomOutImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete zoomOutImageControl;

    disconnect(zoomInImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(zoomInImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(zoomInImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete zoomInImageControl;
    delete zoomOutToolTip;
    delete zoomInToolTip;

    disconnect(focusImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(focusImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(focusImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete focusImageControl;

    disconnect(focusOutImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(focusOutImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(focusOutImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete focusOutImageControl;

    disconnect(focusInImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(focusInImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(focusInImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete focusInImageControl;
    delete focusToolTip;
    delete focusOutToolTip;
    delete focusInToolTip;

    disconnect(irisImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(irisImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(irisImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete irisImageControl;

    disconnect(irisOutImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(irisOutImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(irisOutImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete irisOutImageControl;

    disconnect(irisInImageControl,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotButtonClicked(int)));
    disconnect(irisInImageControl,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(irisInImageControl,
               SIGNAL(sigImageMouseHover(int,bool)),
               this,
               SLOT(slotImageMouseHover(int,bool)));
    delete irisInImageControl;
    delete irisToolTip;
    delete irisOutToolTip;
    delete irisInToolTip;

    delete presetSettingHeading;

    disconnect(setPositionDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(setPositionDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete setPositionDropDownBox;

    disconnect(goButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(goButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    delete goButton;

    disconnect(setPositionNameTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete setPositionNameTextBox;
    delete setPositionNameParam;

    delete cnfgButtonTile;
    disconnect(saveButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(saveButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    delete saveButton;

    disconnect(deleteButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(deleteButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClicked(int)));
    delete deleteButton;

    disconnect(infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
}

void PTZControl::createDefaultComponent()
{
    isMousePressed = false;
    isAdvanceMode = false;
    isDeleteCmd = false;
    applController = ApplController::getInstance();
    payloadLib = new PayloadLib();

    closeButton = new CloseButtton((PTZ_CTRL_WIDTH - SCALE_WIDTH(5)),
                                   SCALE_HEIGHT(5), 0, 0, this, CLOSE_BTN_TYPE_1,
                                   PTZ_CTRL_CLOSE_BUTTON, true, true, false);
    m_elementList[PTZ_CTRL_CLOSE_BUTTON] = closeButton;
    connect(closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    ptzControlHeading = new Heading(PTZ_CTRL_WIDTH/2,
                                    SCALE_HEIGHT(20),
                                    PTZ_CTRL_HEADING,
                                    this);
    createPTZControls();

    QMap<quint8, QString> presetPostionsList;
    for(quint8 index = 0; index < MAX_PRESET_POS; index++)
    {
        presetPostionsList.insert(index,QString("%1").arg(index + 1));
    }

    presetSettingHeading = new ElementHeading(topBgTile->x(),
                                              topBgTile->y() + topBgTile->height(),
                                              BGTILE_SMALL_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              ptzControlStrings[PTZ_CTRL_LABLE_PRESET_POSITION_SETTINGS],
                                              TOP_LAYER,
                                              this,
                                              false,
                                              SCALE_WIDTH(20));

    setPositionDropDownBox = new DropDown(topBgTile->x(),
                                          presetSettingHeading->y() + presetSettingHeading->height(),
                                          BGTILE_SMALL_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          PTZ_CTRL_SET_POS_DROPDOWNBOX,
                                          DROPDOWNBOX_SIZE_90,
                                          ptzControlStrings[PTZ_CTRL_LABLE_POSITION_NUMBR],
                                          presetPostionsList,
                                          this,
                                          "",
                                          true,
                                          SCALE_WIDTH(25),
                                          MIDDLE_TABLE_LAYER,
                                          true,
                                          10,
                                          false,
                                          true,
                                          5,
                                          SCALE_WIDTH(40));
    m_elementList[PTZ_CTRL_SET_POS_DROPDOWNBOX] = setPositionDropDownBox;
    connect(setPositionDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(setPositionDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));

    goButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              setPositionDropDownBox->x() + SCALE_WIDTH(340),
                              setPositionDropDownBox->y() + SCALE_HEIGHT(20),
                              ptzControlStrings[PTZ_CTRL_LABLE_GO],
                              this,
                              PTZ_CTRL_GO_BUTTON);
    m_elementList[PTZ_CTRL_GO_BUTTON] = goButton;
    connect(goButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(goButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));

    setPositionNameParam = new TextboxParam();
    setPositionNameParam->maxChar = 16;
    setPositionNameParam->labelStr = ptzControlStrings[PTZ_CTRL_LABLE_POSITION_NAME];

    setPositionNameTextBox = new TextBox(setPositionDropDownBox->x(),
                                         setPositionDropDownBox->y() + setPositionDropDownBox->height(),
                                         BGTILE_SMALL_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         PTZ_CTRL_SET_POSNAME_TEXTBOX,
                                         TEXTBOX_LARGE,
                                         this,
                                         setPositionNameParam,
                                         MIDDLE_TABLE_LAYER,
                                         true,
                                         false,
                                         false,
                                         SCALE_WIDTH(40));
    m_elementList[PTZ_CTRL_SET_POSNAME_TEXTBOX] = setPositionNameTextBox;
    connect(setPositionNameTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    cnfgButtonTile = new BgTile(setPositionNameTextBox->x(),
                                setPositionNameTextBox->y() + setPositionNameTextBox->height(),
                                BGTILE_SMALL_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                BOTTOM_LAYER,
                                this);

    saveButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                cnfgButtonTile->x() + (cnfgButtonTile->width()/2) - SCALE_WIDTH(70),
                                cnfgButtonTile->y() + SCALE_HEIGHT(25),
                                ptzControlStrings[PTZ_CTRL_LABLE_SAVE],
                                this,
                                PTZ_CTRL_SAVE_BUTTON);
    m_elementList[PTZ_CTRL_SAVE_BUTTON] = saveButton;
    connect(saveButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(saveButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));

    deleteButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  cnfgButtonTile->x() + (cnfgButtonTile->width()/2) + SCALE_WIDTH(70),
                                  cnfgButtonTile->y() + SCALE_HEIGHT(25),
                                  ptzControlStrings[PTZ_CTRL_LABLE_DELETE],
                                  this,
                                  PTZ_CTRL_DELETE_BUTTON);
    m_elementList[PTZ_CTRL_DELETE_BUTTON] = deleteButton;
    connect(deleteButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(deleteButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));

    infoPage = new InfoPage(0, 0,
                            this->window()->width(),
                            this->window()->height(),
                            MAX_INFO_PAGE_TYPE,
                            this->window(), false, false);
    connect(infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageBtnclick(int)));
}

void PTZControl::createPTZControls()
{
    resumeButtonTile = new BgTile((PTZ_CTRL_WIDTH - BGTILE_SMALL_SIZE_WIDTH)/2,
                                  SCALE_HEIGHT(40),
                                  BGTILE_SMALL_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  COMMON_LAYER,
                                  this);

    resumeTourLabel = new TextLabel(resumeButtonTile->x() + SCALE_WIDTH(25),
                                    resumeButtonTile->y() + SCALE_HEIGHT(10),
                                    NORMAL_FONT_SIZE,
                                    ptzControlStrings[PTZ_CTRL_LABLE_RESUME_PAUSED_TOUR],
                                    this);

    resumeTourButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                      resumeButtonTile->x() + SCALE_WIDTH(340),
                                      resumeButtonTile->y() + SCALE_HEIGHT(20) ,
                                      ptzControlStrings[PTZ_CTRL_LABLE_RESUME],
                                      this,
                                      PTZ_CTRL_RESUME_BUTTON);
    m_elementList[PTZ_CTRL_RESUME_BUTTON] = resumeTourButton;
    connect(resumeTourButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(resumeTourButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));

    manualButtonTile = new BgTile(resumeButtonTile->x(),
                                  resumeButtonTile->y() + resumeButtonTile->height(),
                                  BGTILE_SMALL_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  COMMON_LAYER,
                                  this);

    manualButtonLabel = new TextLabel(resumeButtonTile->x() + SCALE_WIDTH(25),
                                      manualButtonTile->y() + SCALE_HEIGHT(10),
                                      NORMAL_FONT_SIZE,
                                      ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR],
                                      this);

    manualButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  resumeButtonTile->x() + SCALE_WIDTH(340),
                                  manualButtonTile->y() + SCALE_HEIGHT(20),
                                  ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_START],
                                  this,
                                  PTZ_CTRL_MANUAL_START);
    m_elementList[PTZ_CTRL_MANUAL_START] = manualButton;
    connect(manualButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(manualButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));

    topBgTile = new BgTile((PTZ_CTRL_WIDTH - BGTILE_SMALL_SIZE_WIDTH)/2,
                           manualButtonTile->y() + manualButtonTile->height(),
                           BGTILE_SMALL_SIZE_WIDTH /2,
                           SCALE_HEIGHT(230),
                           COMMON_LAYER,
                           this);

    topKeyImageControl = new Image((topBgTile->x() + (topBgTile->width()/2) - SCALE_WIDTH(24)),
                                   topBgTile->y() + SCALE_HEIGHT(16),
                                   PTZ_CTRL_TOP_KEY_IMAGE_PATH,
                                   this,
                                   START_X_START_Y,
                                   PTZ_CTRL_TOP_KEY,
                                   true,
                                   false,
                                   true,
                                   true);
    m_elementList[PTZ_CTRL_TOP_KEY] = topKeyImageControl;
    connect(topKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(topKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    topLeftKeyImageControl = new Image(topKeyImageControl->x() - topKeyImageControl->width(),
                                       topKeyImageControl->y(),
                                       PTZ_CTRL_TOP_LEFT_KEY_IMAGE_PATH,
                                       this,
                                       START_X_START_Y,
                                       PTZ_CTRL_TOP_LEFT_KEY,
                                       true,
                                       false,
                                       true,
                                       true);
    m_elementList[PTZ_CTRL_TOP_LEFT_KEY] = topLeftKeyImageControl;
    connect(topLeftKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(topLeftKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    topRightKeyImageControl = new Image(topKeyImageControl->x() + topKeyImageControl->width(),
                                        topKeyImageControl->y(),
                                        PTZ_CTRL_TOP_RIGHT_KEY_IMAGE_PATH,
                                        this,
                                        START_X_START_Y,
                                        PTZ_CTRL_TOP_RIGHT_KEY,
                                        true,
                                        false,
                                        true,
                                        true);
    m_elementList[PTZ_CTRL_TOP_RIGHT_KEY] = topRightKeyImageControl;
    connect(topRightKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(topRightKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    leftKeyImageControl = new Image(topKeyImageControl->x() - topKeyImageControl->width(),
                                    topKeyImageControl->y() + topKeyImageControl->height(),
                                    PTZ_CTRL_LEFT_KEY_IMAGE_PATH,
                                    this,
                                    START_X_START_Y,
                                    PTZ_CTRL_LEFT_KEY,
                                    true,
                                    false,
                                    true,
                                    true);
    m_elementList[PTZ_CTRL_LEFT_KEY] = leftKeyImageControl;
    connect(leftKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(leftKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    rightKeyImageControl = new Image(topKeyImageControl->x() + topKeyImageControl->width(),
                                     topKeyImageControl->y() + topKeyImageControl->height(),
                                     PTZ_CTRL_RIGHT_KEY_IMAGE_PATH,
                                     this,
                                     START_X_START_Y,
                                     PTZ_CTRL_RIGHT_KEY,
                                     true,
                                     false,
                                     true,
                                     true);
    m_elementList[PTZ_CTRL_RIGHT_KEY] = rightKeyImageControl;
    connect(rightKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(rightKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    bottomKeyImageControl = new Image(topKeyImageControl->x() ,
                                      rightKeyImageControl->y() + rightKeyImageControl->height(),
                                      PTZ_CTRL_BOTTOM_KEY_IMAGE_PATH,
                                      this,
                                      START_X_START_Y,
                                      PTZ_CTRL_BOTTOM_KEY,
                                      true,
                                      false,
                                      true,
                                      true);
    m_elementList[PTZ_CTRL_BOTTOM_KEY] = bottomKeyImageControl;
    connect(bottomKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(bottomKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    bottomLeftKeyImageControl = new Image(bottomKeyImageControl->x() - bottomKeyImageControl->width(),
                                          bottomKeyImageControl->y(),
                                          PTZ_CTRL_BOTTOM_LEFT_KEY_IMAGE_PATH,
                                          this,
                                          START_X_START_Y,
                                          PTZ_CTRL_BOTTOM_LEFT_KEY,
                                          true,
                                          false,
                                          true,
                                          true);
    m_elementList[PTZ_CTRL_BOTTOM_LEFT_KEY] = bottomLeftKeyImageControl;
    connect(bottomLeftKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(bottomLeftKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    bottomRightKeyImageControl = new Image(bottomKeyImageControl->x() + bottomKeyImageControl->width(),
                                           bottomKeyImageControl->y(),
                                           PTZ_CTRL_BOTTOM_RIGHT_KEY_IMAGE_PATH,
                                           this,
                                           START_X_START_Y,
                                           PTZ_CTRL_BOTTOM_RIGHT_KEY,
                                           true,
                                           false,
                                           true,
                                           true);
    m_elementList[PTZ_CTRL_BOTTOM_RIGHT_KEY] = bottomRightKeyImageControl;
    connect(bottomRightKeyImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(bottomRightKeyImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString> speedList;
    for(quint8 index = 0; index < MAX_PTZ_SPEED; index++)
    {
        speedList.insert(index, QString("%1").arg(index + 1));
    }

    speedDropDownBox = new DropDown(topBgTile->x() + SCALE_WIDTH(10),
                                    bottomKeyImageControl->y() + bottomKeyImageControl->height() + SCALE_HEIGHT(15),
                                    BGTILE_SMALL_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    PTZ_CTRL_SPEED_DROPDOWNBOX,
                                    DROPDOWNBOX_SIZE_90,
                                    ptzControlStrings[PTZ_CTRL_LABLE_SPEED],
                                    speedList,
                                    this,
                                    "",
                                    false,
                                    0,
                                    NO_LAYER,
                                    true,
                                    5);
    m_elementList[PTZ_CTRL_SPEED_DROPDOWNBOX] = speedDropDownBox;
    connect(speedDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    speedDropDownBox->setCurrValue(QString("%1").arg(DEFAULT_PTZ_SPEED));

    imageTopBgTile = new BgTile(topBgTile->x() + topBgTile->width(),
                                topBgTile->y(),
                                BGTILE_SMALL_SIZE_WIDTH /2,
                                SCALE_HEIGHT(230),
                                COMMON_LAYER,
                                this);

    zoomImageControl = new Image((imageTopBgTile->x() + (imageTopBgTile->width()/2) - SCALE_WIDTH(38)),
                                 imageTopBgTile->y() + SCALE_HEIGHT(30),
                                 PTZ_CTRL_ZOOM_IMAGE_PATH,
                                 this,
                                 START_X_START_Y,
                                 PTZ_CTRL_ZOOM_KEY,
                                 true,
                                 true,
                                 true,
                                 false);
    m_elementList[PTZ_CTRL_ZOOM_KEY] = zoomImageControl;  
    connect(zoomImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    zoomOutImageControl = new Image(zoomImageControl->x() - SCALE_WIDTH(40),
                                    zoomImageControl->y() + SCALE_HEIGHT(5),
                                    PTZ_CTRL_OUT_IMAGE_PATH,
                                    this,
                                    START_X_START_Y,
                                    PTZ_CTRL_ZOOM_OUT_KEY,
                                    true,
                                    false,
                                    true,
                                    true);
    m_elementList[PTZ_CTRL_ZOOM_OUT_KEY] = zoomOutImageControl;
    connect(zoomOutImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(zoomOutImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(zoomOutImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    zoomInImageControl = new Image(zoomImageControl->x() + zoomImageControl->width(),
                                   zoomImageControl->y() + SCALE_HEIGHT(5),
                                   PTZ_CTRL_IN_IMAGE_PATH,
                                   this,
                                   START_X_START_Y,
                                   PTZ_CTRL_ZOOM_IN_KEY,
                                   true,
                                   false,
                                   true,
                                   true);
    m_elementList[PTZ_CTRL_ZOOM_IN_KEY] = zoomInImageControl;
    connect(zoomInImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(zoomInImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(zoomInImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    zoomOutToolTip = new ToolTip(zoomOutImageControl->x() + zoomOutImageControl->width()/2,
                                 zoomOutImageControl->y() - SCALE_HEIGHT(10),
                                 ptzCtrlToolTipStrings[0],
                                 this);

    zoomInToolTip = new ToolTip(zoomInImageControl->x() + zoomInImageControl->width()/2,
                                zoomInImageControl->y() - SCALE_HEIGHT(10),
                                ptzCtrlToolTipStrings[1],
                                this);

    focusImageControl = new Image(zoomImageControl->x(),
                                  zoomImageControl->y() + zoomImageControl->height(),
                                  PTZ_CTRL_FOCUS_IMAGE_PATH,
                                  this,
                                  START_X_START_Y,
                                  PTZ_CTRL_FOCUS_KEY,
                                  true,
                                  false,
                                  true,
                                  true);
    m_elementList[PTZ_CTRL_FOCUS_KEY] = focusImageControl;
    connect(focusImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(focusImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(focusImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    focusOutImageControl = new Image(focusImageControl->x() - SCALE_WIDTH(40),
                                     focusImageControl->y() + SCALE_HEIGHT(5),
                                     PTZ_CTRL_OUT_IMAGE_PATH,
                                     this,
                                     START_X_START_Y,
                                     PTZ_CTRL_FOCUS_OUT_KEY,
                                     true,
                                     false,
                                     true,
                                     true);
    m_elementList[PTZ_CTRL_FOCUS_OUT_KEY] = focusOutImageControl;
    connect(focusOutImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(focusOutImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(focusOutImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    focusInImageControl = new Image(focusImageControl->x() + focusImageControl->width(),
                                    focusImageControl->y() + SCALE_HEIGHT(5),
                                    PTZ_CTRL_IN_IMAGE_PATH,
                                    this,
                                    START_X_START_Y,
                                    PTZ_CTRL_FOCUS_IN_KEY,
                                    true,
                                    false,
                                    true,
                                    true);
    m_elementList[PTZ_CTRL_FOCUS_IN_KEY] = focusInImageControl;
    connect(focusInImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(focusInImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(focusInImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    focusToolTip = new ToolTip(focusImageControl->x() + focusImageControl->width()/2,
                               focusImageControl->y() - SCALE_HEIGHT(10),
                               ptzCtrlToolTipStrings[2],
                               this);

    focusOutToolTip = new ToolTip(focusOutImageControl->x() + focusOutImageControl->width()/2,
                                  focusOutImageControl->y() - SCALE_HEIGHT(10),
                                  ptzCtrlToolTipStrings[3],
                                  this);

    focusInToolTip = new ToolTip(focusInImageControl->x() + focusInImageControl->width()/2,
                                 focusImageControl->y() - SCALE_HEIGHT(10),
                                 ptzCtrlToolTipStrings[4],
                                 this);

    irisImageControl = new Image(focusImageControl->x(),
                                 focusImageControl->y() + focusImageControl->height(),
                                 PTZ_CTRL_IRIS_IMAGE_PATH,
                                 this,
                                 START_X_START_Y,
                                 PTZ_CTRL_IRIS_KEY,
                                 true,
                                 false,
                                 true,
                                 true);
    m_elementList[PTZ_CTRL_IRIS_KEY] = irisImageControl;
    connect(irisImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(irisImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(irisImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    irisOutImageControl = new Image(irisImageControl->x() - SCALE_WIDTH(40),
                                    irisImageControl->y() + SCALE_HEIGHT(5),
                                    PTZ_CTRL_OUT_IMAGE_PATH,
                                    this,
                                    START_X_START_Y,
                                    PTZ_CTRL_IRIS_OUT_KEY,
                                    true,
                                    false,
                                    true,
                                    true);
    m_elementList[PTZ_CTRL_IRIS_OUT_KEY] = irisOutImageControl;
    connect(irisOutImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(irisOutImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(irisOutImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    irisInImageControl = new Image(irisImageControl->x() + irisImageControl->width(),
                                   irisImageControl->y() + SCALE_HEIGHT(5),
                                   PTZ_CTRL_IN_IMAGE_PATH,
                                   this,
                                   START_X_START_Y,
                                   PTZ_CTRL_IRIS_IN_KEY,
                                   true,
                                   false,
                                   true,
                                   true);
    m_elementList[PTZ_CTRL_IRIS_IN_KEY] = irisInImageControl;
    connect(irisInImageControl,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotButtonClicked(int)));
    connect(irisInImageControl,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(irisInImageControl,
            SIGNAL(sigImageMouseHover(int,bool)),
            this,
            SLOT(slotImageMouseHover(int,bool)));

    irisToolTip = new ToolTip(irisImageControl->x() + irisImageControl->width()/2,
                              irisImageControl->y() - SCALE_HEIGHT(10),
                              ptzCtrlToolTipStrings[5],
                              this);

    irisOutToolTip = new ToolTip(irisOutImageControl->x() + irisOutImageControl->width()/2,
                                 irisOutImageControl->y() - SCALE_HEIGHT(10),
                                 ptzCtrlToolTipStrings[6],
                                 this);

    irisInToolTip = new ToolTip(irisInImageControl->x() + irisInImageControl->width()/2,
                                irisImageControl->y() - SCALE_HEIGHT(10),
                                ptzCtrlToolTipStrings[7],
                                this);
}

void PTZControl::getManualTourStatus()
{
    if (false == applController->GetHlthStatusSingleParam(currentDeviceName, manualTourHealthState, PTZ_TOUR_TYPE))
    {
        return;
    }

    if (manualTourHealthState[(m_cameraIndex - 1)] == EVENT_MANUAL)
    {
        manualButton->changeText(ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_STOP]);
    }
    else if ((manualTourHealthState[(m_cameraIndex - 1)] != EVENT_RESUME_TOUR) && ((manualTourHealthState[(m_cameraIndex - 1)] != EVENT_PAUSE_TOUR)))
    {
        manualButton->changeText(ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_START]);
    }

    resumeTourButton->setIsEnabled((manualTourHealthState[(m_cameraIndex - 1)] == EVENT_PAUSE_TOUR) ? true : false);
}

void PTZControl::getPositionConfig()
{
    createPayload(MSG_GET_CFG);
}

bool PTZControl::savePositiontoConfig()
{
    QString positionNameText = setPositionNameTextBox->getInputText();

    if (positionNameText == "")
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_CONTROL_ENT_POSITION_NAME));
        return FAIL;
    }

    if (m_positionNameList.at(m_currentPositionIndex).length() != 0)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_ALREADY_PRESENT_INDEX));
        return FAIL;
    }

    for(qint32 tIndex = 0; tIndex <= m_positionNameList.length(); tIndex++)
    {
        if ((tIndex != m_currentPositionIndex) && (m_positionNameList.value(tIndex) == positionNameText))
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_PRESET_NAME_ALREADY_PRESENT));
            return FAIL;
        }
    }

    payloadLib->setCnfgArrayAtIndex(0, positionNameText);
    isDeleteCmd = false;
    createPayload(MSG_SET_CFG);
    return SUCCESS;
}

void PTZControl::deletePositionFromConfig()
{
    isDeleteCmd = true;
    payloadLib->setCnfgArrayAtIndex(0, "");
    createPayload(MSG_SET_CFG);
}

void PTZControl::createPayload(REQ_MSG_ID_e msgType)
{
    quint16 cnfgFromIndx = (((m_cameraIndex - 1) * MAX_PRESET_POS + 1));
    quint16 cnfgToIndx = (cnfgFromIndx + MAX_PRESET_POS - 1);
    QString payloadString = "";

    if (msgType == MSG_SET_CFG)
    {
        cnfgFromIndx += m_currentPositionIndex;
        cnfgToIndx = cnfgFromIndx;
    }

    payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                     PRESET_POSITION_TABLE_INDEX,
                                                     cnfgFromIndx,
                                                     cnfgToIndx, 1, 1, 1);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void PTZControl::createCmdPayload(SET_COMMAND_e cmdType, quint8 totalFields)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalFields);
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void PTZControl::sendStartManualTourCmd()
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    createCmdPayload(SRT_MAN_PTZ_TOUR, 1);
}

void PTZControl::sendStopManualTourCmd()
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    createCmdPayload(STP_MAN_PTZ_TOUR, 1);
}

void PTZControl::sendZoomCmd(quint8 zoomVal)
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    payloadLib->setCnfgArrayAtIndex(1, zoomVal);
    payloadLib->setCnfgArrayAtIndex(2, (speedDropDownBox->getIndexofCurrElement() + 1));
    createCmdPayload(SETZOOM, 3);
}

void PTZControl::sendFocusCmd(quint8 focusVal)
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    payloadLib->setCnfgArrayAtIndex(1, focusVal);
    payloadLib->setCnfgArrayAtIndex(2, (speedDropDownBox->getIndexofCurrElement() + 1));
    createCmdPayload(SETFOCUS, 3);
}

void PTZControl::sendIrisCmd(quint8 irisVal)
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    payloadLib->setCnfgArrayAtIndex(1, irisVal);
    createCmdPayload(SETIRIS, 2);
}

void PTZControl::sendPresetPosGoCmd()
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    payloadLib->setCnfgArrayAtIndex(1, (setPositionDropDownBox->getIndexofCurrElement() + 1));
    createCmdPayload(CALLPRESET, 2);
}

void PTZControl::sendPanTiltCmd(quint8 panVal, quint8 tiltVal)
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    payloadLib->setCnfgArrayAtIndex(1, panVal);
    payloadLib->setCnfgArrayAtIndex(2, tiltVal);
    payloadLib->setCnfgArrayAtIndex(3, (speedDropDownBox->getIndexofCurrElement() + 1));
    createCmdPayload(SETPANTILT, 4);
}

void PTZControl::sendResumeTourCmd()
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    createCmdPayload(RESUME_PTZ_TOUR, 1);
}

void PTZControl::getTourStatus()
{
    payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
    createCmdPayload(PTZ_TOUR_STATUS, 1);
}

void PTZControl::hideAllTooltips()
{
    zoomOutToolTip->setVisible(false);
    zoomInToolTip->setVisible(false);

    focusToolTip->setVisible(false);
    focusOutToolTip->setVisible(false);
    focusInToolTip->setVisible(false);

    irisToolTip->setVisible(false);
    irisOutToolTip->setVisible(false);
    irisInToolTip->setVisible(false);
}

void PTZControl::giveFocusToControl(quint8 index)
{
    m_currentElement = currentButtonClick = index;
    showToolTipOfControl(m_currentElement);
    takeEnterKeyAction();
    m_elementList[m_currentElement]->forceActiveFocus();
}

void PTZControl:: showToolTipOfControl(quint8 index,bool isMouseHoverOnImage)
{
    switch(index)
    {
        case PTZ_CTRL_ZOOM_OUT_KEY:
            zoomOutToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_ZOOM_IN_KEY:
            zoomInToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_FOCUS_KEY:
            focusToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_FOCUS_OUT_KEY:
            focusOutToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_FOCUS_IN_KEY:
            focusInToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_IRIS_KEY:
            irisToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_IRIS_OUT_KEY:
            irisOutToolTip->setVisible(isMouseHoverOnImage);
            break;

        case PTZ_CTRL_IRIS_IN_KEY:
            irisInToolTip->setVisible(isMouseHoverOnImage);
            break;

        default:
            break;
    }
}

void PTZControl::mousePressEvent(QMouseEvent *event)
{
    isMousePressed = true;
    pressedPoint = event->pos();
}

void PTZControl::mouseMoveEvent(QMouseEvent *event)
{
    /* Don't call hideAllTooltips() because of On Image mouseMoveEvent also receive so try to show and hide tooltip at same time,
     * so device get crashed */
    if(isMousePressed)
    {
        if(event->pos().x() > pressedPoint.x())
        {
            startX += (event->pos().x() - pressedPoint.x());
        }
        else
        {
            startX -= pressedPoint.x() - event->pos().x();
        }

        if(event->pos().y() > pressedPoint.y())
        {
            startY += (event->pos().y() - pressedPoint.y());
        }
        else
        {
            startY -= pressedPoint.y() - event->pos().y();
        }

        if(startX < 0)
        {
            startX = 0;
        }

        if(startY < 0)
        {
            startY = 0;
        }

        if((startX + PTZ_CTRL_WIDTH) >= this->window()->width())
        {
            startX = (this->window()->width() - PTZ_CTRL_WIDTH - 1);
        }

        if((startY + PTZ_CTRL_HEIGHT) >= this->window()->height())
        {
            startY = (this->window()->height() - PTZ_CTRL_HEIGHT - 1);
        }

        resetGeometry(startX, startY, PTZ_CTRL_WIDTH, PTZ_CTRL_HEIGHT);
        update();
    }
}

void PTZControl::mouseReleaseEvent(QMouseEvent*)
{
    isMousePressed = false;
}

void PTZControl::takeLeftKeyAction()
{
    hideAllTooltips();
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_PTZ_CTRLS) % MAX_PTZ_CTRLS;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
    showToolTipOfControl(m_currentElement);
}

void PTZControl::takeRightKeyAction()
{
    hideAllTooltips();
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_PTZ_CTRLS;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
    showToolTipOfControl(m_currentElement);
}

void PTZControl::takeEnterKeyAction()
{
    switch(currentButtonClick)
    {
        case PTZ_CTRL_TOP_KEY:
            topKeyImageControl->takeClickAction();
            break;

        case PTZ_CTRL_TOP_LEFT_KEY:
            topLeftKeyImageControl->takeClickAction();
            break;

        case PTZ_CTRL_TOP_RIGHT_KEY:
            topRightKeyImageControl->takeClickAction();
            break;

        case PTZ_CTRL_BOTTOM_KEY:
            bottomKeyImageControl->takeClickAction();
            break;

        case PTZ_CTRL_BOTTOM_LEFT_KEY:
            bottomLeftKeyImageControl->takeClickAction();
            break;

        case PTZ_CTRL_BOTTOM_RIGHT_KEY:
            bottomRightKeyImageControl->takeClickAction();
            break;

        case PTZ_CTRL_LEFT_KEY:
            if(isAdvanceMode)
            {
                leftKeyImageControl->takeClickAction();
            }
            break;

        case PTZ_CTRL_RIGHT_KEY:
            if(isAdvanceMode)
            {
                rightKeyImageControl->takeClickAction();
            }
            break;

        case PTZ_CTRL_ZOOM_IN_KEY:
            zoomInImageControl->takeClickAction();
            break;

        case PTZ_CTRL_ZOOM_OUT_KEY:
            zoomOutImageControl->takeClickAction();
            break;

        case PTZ_CTRL_FOCUS_OUT_KEY:
            focusOutImageControl->takeClickAction();
            break;

        case PTZ_CTRL_FOCUS_KEY:
            focusImageControl->takeClickAction();
            break;

        case PTZ_CTRL_FOCUS_IN_KEY:
            focusInImageControl->takeClickAction();
            break;

        case PTZ_CTRL_IRIS_OUT_KEY:
            irisOutImageControl->takeClickAction();
            break;

        case PTZ_CTRL_IRIS_KEY:
            irisImageControl->takeClickAction();
            break;

        case PTZ_CTRL_IRIS_IN_KEY:
            irisInImageControl->takeClickAction();
            break;

        default:
            break;
    }
}

void PTZControl::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if(!infoPage->isVisible())
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void PTZControl::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if(infoPage->isVisible())
    {
        infoPage->unloadInfoPage();
    }
}

void PTZControl::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
            event->accept();
            if(isAdvanceMode)
            {
                m_currentElement = currentButtonClick = PTZ_CTRL_LEFT_KEY;
                takeEnterKeyAction();
            }
            else
            {
                takeLeftKeyAction();
            }
            break;

        case Qt::Key_Right:
            event->accept();
            if(isAdvanceMode)
            {
                m_currentElement = currentButtonClick =  PTZ_CTRL_RIGHT_KEY;
                takeEnterKeyAction();
            }
            else
            {
                takeRightKeyAction();
            }
            break;

        case Qt::Key_Up:
            event->accept();
            if(isAdvanceMode)
            {
                m_currentElement = currentButtonClick = PTZ_CTRL_TOP_KEY;
                takeEnterKeyAction();
            }
            break;

        case Qt::Key_Down:
            event->accept();
            if(isAdvanceMode)
            {
                m_currentElement = currentButtonClick = PTZ_CTRL_BOTTOM_KEY;
                takeEnterKeyAction();
            }
            break;

        default:
            event->accept();
            break;
    }

    m_elementList[m_currentElement]->forceActiveFocus();
}

void PTZControl::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = PTZ_CTRL_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void PTZControl::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_F5:
            m_currentElement = currentButtonClick = PTZ_CTRL_ZOOM_IN_KEY;
            takeEnterKeyAction();
            break;

        case Qt::Key_F6:
            m_currentElement = currentButtonClick = PTZ_CTRL_ZOOM_OUT_KEY;
            takeEnterKeyAction();
            break;

        case Qt::Key_F7:
            m_currentElement = currentButtonClick = PTZ_CTRL_IRIS_IN_KEY;
            takeEnterKeyAction();
            break;

        case Qt::Key_F8:
            m_currentElement = currentButtonClick = PTZ_CTRL_IRIS_OUT_KEY;
            takeEnterKeyAction();
            break;

        case Qt::Key_F9:
            m_currentElement = currentButtonClick = PTZ_CTRL_FOCUS_IN_KEY;
            takeEnterKeyAction();
            break;

        case Qt::Key_F10:
            m_currentElement = currentButtonClick = PTZ_CTRL_FOCUS_OUT_KEY;
            takeEnterKeyAction();
            break;

        case Qt::Key_F11:
            event->accept();
            if(!isAdvanceMode)
            {
                isAdvanceMode = true;
                m_currentElement = PTZ_CTRL_LEFT_KEY;
            }
            else
            {
                isAdvanceMode = false ;
                m_currentElement = PTZ_CTRL_SET_POS_DROPDOWNBOX;
            }
            break;

        default:
            event->accept();
            break;
    }
}

void PTZControl::updateManualTourStatus(QString deviceName, quint8 camNum, LOG_EVENT_STATE_e evtState)
{
    if((currentDeviceName == deviceName) && (m_cameraIndex == camNum))
    {
        if(evtState == EVENT_MANUAL)
        {
            manualButton->changeText(ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_STOP]);
        }
        else if ((evtState != EVENT_RESUME_TOUR) && ((evtState != EVENT_PAUSE_TOUR)))
        {
            manualButton->changeText(ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_START]);
        }

        resumeTourButton->setIsEnabled((evtState == EVENT_PAUSE_TOUR)? true : false);
    }
}

void PTZControl::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName != currentDeviceName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        if ((param->deviceStatus == CMD_INTERNAL_RESOURCE_LIMIT) || (param->deviceStatus == CMD_CAM_REQUEST_IN_PROCESS))
        {
            setPositionDropDownBox->setIndexofCurrElement(m_currentPositionIndex);
            return;
        }

        if (param->deviceStatus == CMD_ACTIVE_TOUR_PAUSE)
        {
            resumeTourButton->setIsEnabled(true);
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            setPositionDropDownBox->setIndexofCurrElement(m_currentPositionIndex);
            return;
        }

        notifyError();
        setPositionDropDownBox->setIndexofCurrElement(m_currentPositionIndex);
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            m_positionNameList.clear();
            payloadLib->parsePayload(param->msgType, param->payload);

            if(payloadLib->getcnfgTableIndex() == PRESET_POSITION_TABLE_INDEX)
            {
                setPositionNameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(m_currentPositionIndex).toString());
                for(qint32 tIndex = 0; tIndex < MAX_PRESET_POS; tIndex++)
                {
                    m_positionNameList.append(payloadLib->getCnfgArrayAtIndex(tIndex).toString());
                }
            }

            getTourStatus();
        }
        break;

        case MSG_SET_CFG:
        {
            if(isDeleteCmd == false)
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_CONTROL_POSITION_SAVE_SUCCESS));
                getPositionConfig();
                m_currentPositionIndex = setPositionDropDownBox->getIndexofCurrElement();
                setPositionNameTextBox->setInputText(m_positionNameList[m_currentPositionIndex]);
            }
            else
            {
                isDeleteCmd = false;
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_CONTROL_POSITION_DEL_SUCCESS));
                getPositionConfig();
            }
        }
        break;

        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
                case SRT_MAN_PTZ_TOUR:
                {
                    manualButton->changeText(ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_STOP]);
                }
                break;

                case STP_MAN_PTZ_TOUR:
                {
                    manualButton->changeText(ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_START]);
                }
                break;

                case RESUME_PTZ_TOUR:
                {
                    resumeTourButton->setIsEnabled(false);
                    m_currentElement = PTZ_CTRL_SET_POS_DROPDOWNBOX;
                    m_elementList[m_currentElement]->forceActiveFocus ();
                    resumeTourButton->setIsEnabled(false);
                }
                break;

                case PTZ_TOUR_STATUS:
                {
                    payloadLib->parseDevCmdReply(true,param->payload);
                    if((payloadLib->getCnfgArrayAtIndex(0).toUInt () == 0) )
                    {
                        resumeTourButton->setIsEnabled(false);
                    }
                    else
                    {
                        resumeTourButton->setIsEnabled(true);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void PTZControl::notifyError()
{
    switch(currentButtonClick)
    {
        case PTZ_CTRL_TOP_KEY:
            topKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_TOP_LEFT_KEY:
            topLeftKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_TOP_RIGHT_KEY:
            topRightKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_BOTTOM_KEY:
            bottomKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_BOTTOM_LEFT_KEY:
            bottomLeftKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_BOTTOM_RIGHT_KEY:
            bottomRightKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_LEFT_KEY:
            leftKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_RIGHT_KEY:
            rightKeyImageControl->notifyError();
            break;

        case PTZ_CTRL_ZOOM_IN_KEY:
            zoomInImageControl->notifyError();
            break;

        case PTZ_CTRL_ZOOM_OUT_KEY:
            zoomOutImageControl->notifyError();
            break;

        case PTZ_CTRL_FOCUS_OUT_KEY:
            focusOutImageControl->notifyError();
            break;

        case PTZ_CTRL_FOCUS_KEY:
            focusImageControl->notifyError();
            break;

        case PTZ_CTRL_FOCUS_IN_KEY:
            focusInImageControl->notifyError();
            break;

        case PTZ_CTRL_IRIS_OUT_KEY:
            irisOutImageControl->notifyError();
            break;

        case PTZ_CTRL_IRIS_KEY:
            irisImageControl->notifyError();
            break;

        case PTZ_CTRL_IRIS_IN_KEY:
            irisInImageControl->notifyError();
            break;

        default:
            break;
    }
}

void PTZControl::slotButtonClicked(int index)
{
    currentButtonClick = index;
    switch(index)
    {
        case PTZ_CTRL_CLOSE_BUTTON:
            emit sigObjectDelete();
            break;

        case PTZ_CTRL_SAVE_BUTTON:
            savePositiontoConfig();
            break;

        case PTZ_CTRL_DELETE_BUTTON:
        {
            if (m_positionNameList.at(setPositionDropDownBox->getIndexofCurrElement()) == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_CONTROL_POSITION_NUM_CONGI));
            }
            else
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PTZ_CONTROL_DEL_POSITION), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
            }
        }
        break;

        case PTZ_CTRL_MANUAL_START:
        {
            if(manualButton->getText() == ptzControlStrings[PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_START])
            {
                sendStartManualTourCmd();
            }
            else
            {
                sendStopManualTourCmd();
            }
        }
        break;

        case PTZ_CTRL_TOP_KEY:
            sendPanTiltCmd(MAX_PTZ_PAN_OPTION, PTZ_TILT_UP);
            break;

        case PTZ_CTRL_TOP_LEFT_KEY:
            sendPanTiltCmd(PTZ_PAN_LEFT, PTZ_TILT_UP);
            break;

        case PTZ_CTRL_TOP_RIGHT_KEY:
            sendPanTiltCmd(PTZ_PAN_RIGHT, PTZ_TILT_UP);
            break;

        case PTZ_CTRL_BOTTOM_KEY:
            sendPanTiltCmd(MAX_PTZ_PAN_OPTION, PTZ_TILT_DOWN);
            break;

        case PTZ_CTRL_BOTTOM_LEFT_KEY:
            sendPanTiltCmd(PTZ_PAN_LEFT, PTZ_TILT_DOWN);
            break;

        case PTZ_CTRL_BOTTOM_RIGHT_KEY:
            sendPanTiltCmd(PTZ_PAN_RIGHT, PTZ_TILT_DOWN);
            break;

        case PTZ_CTRL_LEFT_KEY:
            sendPanTiltCmd(PTZ_PAN_LEFT, MAX_PTZ_TILT_OPTION);
            break;

        case PTZ_CTRL_RIGHT_KEY:
            sendPanTiltCmd(PTZ_PAN_RIGHT, MAX_PTZ_TILT_OPTION);
            break;

        case PTZ_CTRL_ZOOM_IN_KEY:
            sendZoomCmd(PTZ_ZOOM_IN);
            break;

        case PTZ_CTRL_ZOOM_OUT_KEY:
            sendZoomCmd(PTZ_ZOOM_OUT);
            break;

        case PTZ_CTRL_FOCUS_OUT_KEY:
            sendFocusCmd(FOCUS_FAR);
            break;

        case PTZ_CTRL_FOCUS_KEY:
            sendFocusCmd(FOCUS_AUTO);
            break;

        case PTZ_CTRL_FOCUS_IN_KEY:
            sendFocusCmd(FOCUS_NEAR);
            break;

        case PTZ_CTRL_IRIS_OUT_KEY:
            sendIrisCmd(IRIS_CLOSE);
            break;

        case PTZ_CTRL_IRIS_KEY:
            sendIrisCmd(IRIS_AUTO);
            break;

        case PTZ_CTRL_IRIS_IN_KEY:
            sendIrisCmd(IRIS_OPEN);
            break;

        case PTZ_CTRL_GO_BUTTON:
            sendPresetPosGoCmd();
            break;

        case PTZ_CTRL_RESUME_BUTTON:
            sendResumeTourCmd();
            break;

        default:
            break;
    }
}

void PTZControl::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void PTZControl::slotImageMouseHover(int index, bool isMouseHoverOnImage)
{
    hideAllTooltips();
    showToolTipOfControl(index, isMouseHoverOnImage);
}

void PTZControl::slotSpinBoxValueChange(QString, quint32)
{
    if((m_positionNameList[m_currentPositionIndex].length() == 0) && (setPositionNameTextBox->getInputText() != 0))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
    }
    else
    {
        if(IS_VALID_OBJ(setPositionDropDownBox))
        {
            m_currentPositionIndex = setPositionDropDownBox->getIndexofCurrElement();
        }
        setPositionNameTextBox->setInputText(m_positionNameList[m_currentPositionIndex]);
    }
}

void PTZControl::slotInfoPageBtnclick(int index)
{
    if ((infoPage->getText() == ValidationMessage::getValidationMessage(PTZ_CONTROL_DEL_POSITION)) && (index == INFO_OK_BTN))
    {
        deletePositionFromConfig();
    }

    if (index == INFO_OK_BTN)
    {
        if (infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            if (FAIL == savePositiontoConfig())
            {
                m_currentPositionIndex = setPositionDropDownBox->getIndexofCurrElement();
                setPositionNameTextBox->setInputText(m_positionNameList[m_currentPositionIndex]);
            }
        }
    }
    else
    {
        if (infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            if (IS_VALID_OBJ(setPositionDropDownBox))
            {
                m_currentPositionIndex = setPositionDropDownBox->getIndexofCurrElement();
                setPositionNameTextBox->setInputText(m_positionNameList[m_currentPositionIndex]);
            }
            getPositionConfig();
        }
    }
    m_elementList[m_currentElement]->forceActiveFocus();
}

QString PTZControl::currentDevName()
{
    return currentDeviceName;
}
