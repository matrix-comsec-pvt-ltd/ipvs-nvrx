#include "LayoutWindow.h"
#include "Layout/Layout.h"
#include "LiveEventParser.h"

#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>

#define BORDER_WIDTH                                    0.5
#define MARGIN                                          3
#define LAYOUT_WINDOW_LEFT_MARGIN(windowIconType)       (SCALE_WIDTH(BORDER_WIDTH) + ((windowIconType == ICON_TYPE_8X8) ? (SCALE_WIDTH(MARGIN) - SCALE_WIDTH(2)) : SCALE_WIDTH(MARGIN)))
#define LAYOUT_WINDOW_TOP_MARGIN(windowIconType)        (SCALE_HEIGHT(BORDER_WIDTH) + ((windowIconType == ICON_TYPE_8X8) ? (SCALE_HEIGHT(MARGIN) - SCALE_WIDTH(2)) : SCALE_HEIGHT(MARGIN)))
#define MAX_VIDEO_POPUP_LAYOUT                          3

#define AP_TOOLBAR_IMAGE_PATH           IMAGE_PATH "AutoPlayToolbar/"
#define MAX_AP_TOOLBAR_SIZE             5

#define CLOSE_ICON_WIDTH(windowIconType) ((windowIconType == ICON_TYPE_8X8) ? ((windowIconType * 5)+1) : ((windowIconType + SCALE_WIDTH(2)) * 12))

#define WINDOW_ICON_IMG_PATH            ":/Images_Nvrx/WindowIcon/"

#define CAMERA_ADD_BUTTON_IMG_SOURCE    "Add_Camera_Btn.png"
#define CENTER_DECODER_ERR_IMG_SOURCE	"StreamDecodingError.png"
#define CENTER_ICON_IMG_SOURCE          "Logo.png"
#define CLOSE_IMG_SOURCE                "Close.png"
#define PLAYBACK_IMG_PATH               "Playback.png"
#define INSTPLAYBACK_IMG_PATH           "InstantPlayback.png"
#define OPEN_TOOLBAR_IMG_PATH           "OpenToolbar/"

quint8 LayoutWindow :: m_firstClickedWindow = MAX_WINDOWS;

const QString windowIconTypeImgPath[4] = {"3X3/", "4X4/" , "5X5/", "8X8/"};

const QString centerImageSizePath[MAX_CENTER_ICON] = {"Small_Icon/", "Large_Icon/"};

const QString videoErrorSource[MAX_VIDEO_ERROR_TYPE] = {"",
                                                        "NoUserRights.png",
                                                        "CameraDisable.png",
                                                        "NoVideo.png",
                                                        "OtherErrors.png",
                                                        "OtherErrors.png",
														"VideoLoss.png",
														""};

const QString sequenceImageSource[] = {"WindowSequence/Collapse/",
                                       "WindowSequence/Expand/",
                                       "WindowSequence/Muliple_Config/",
                                       "WindowSequence/None_Config/",
                                       ""};

const quint8 maxValueForVideoPopup[MAX_VIDEO_POPUP_LAYOUT][3] = {{16, 16, 48}, //1X1
                                                                { 12, 12, 37},  //2X2
                                                                { 7, 7, 23}};   //3X3

const QString apButtonSizeFolder[MAX_AP_TOOLBAR_SIZE] =
{
    "1x1/",
    "2x2/",
    "3x3/",
    "4x4_5x5/",
    "6x6_8x8/"
};

const QString apButtonTypeFolder[MAX_AP_TOOLBAR_BTN] =
{
    "Reload/",
    "Previous/",
    "Next/",
};

const quint8 apFontSize[MAX_AP_TOOLBAR_SIZE] =
{
    LARGE_HEADING_FONT_SIZE,
    MEDIUM_HEADING_FONT_SIZE,
    HEADING_FONT_SIZE,
    SUB_HEADING_FONT_SIZE,
    SMALL_SUFFIX_FONT_SIZE

};

const quint8 apNextVideoTextVerOffset[MAX_AP_TOOLBAR_SIZE] =
{
    50,
    35,
    25,
    15,
    5
};

const quint8 apSizeButtonMultiplier[MAX_AP_TOOLBAR_SIZE] =
{
    1,
    1,
    1,
    2,
    5
};

const quint8 apSizeButtonDividend[MAX_AP_TOOLBAR_SIZE] =
{
    4,
    4,
    4,
    7,
    16
};

typedef enum
{
    RECORDING_STATUS,
    PLAYBACK_STATUS,
    MAX_HEALTH_STATUS
}HEALTH_STATUS_TYPE_e;

const QString videoStatusSource[MAX_WINDOW_ICON] = {"",
                                                    "Recording.png",
                                                    "MotionDetection.png",
                                                    "ViewTempering.png",
                                                    "ObjectIntrusion.png",
                                                    "TripWire.png",
                                                    "AudioException.png",
                                                    "AudioOn.png",
													"MicrophoneOn.png",
                                                    "missing_object.png",
                                                    "suspicious.png",
                                                    "loitering.png"
                                                   };

LayoutWindow::LayoutWindow(quint8 index,
                           QWidget *parent,
                           WINDOW_TYPE_e windowType,
                           bool isEnabled,
                           bool catchKey) :
    KeyBoard(parent), NavigationControl(index, isEnabled, catchKey, true),
     m_windowNumberText(NULL), m_audioOnIcon(NULL), m_sequenceIcon(NULL), m_openToolbarIcon(NULL),
     m_windowHeaderToolTip(NULL),
     m_windowType(windowType), m_sequenceImgType(NO_WINDOWSEQUENCE_IMAGE),
	 m_windowIconType(ICON_TYPE_5X5), m_apTimerCount(0), m_audioOn(false), m_microPhoneStatus(false), m_windowIndex(index),
     m_pbToolbarSize(0), m_apToolbarSize(3), m_windowColor(""),
     m_windowIconPath(QString(WINDOW_ICON_IMG_PATH) + QString(windowIconTypeImgPath[m_windowIconType])),
     m_arrayIndex(0), m_centerImageSizePath(centerImageSizePath[LARGE_CENTER_ICON])
{
    INIT_OBJ(m_apReloadIcon);
    INIT_OBJ(m_apPrevIcon);
    INIT_OBJ(m_apNextIcon);
    INIT_OBJ(m_apNextVideoText);
    INIT_OBJ(m_apNextTile);
    INIT_OBJ(m_windowToolTip);
    m_arrayOfIndex = 0;
    m_audioExceptionHealthStatus = 0;
    m_loiteringStatus = 0;
    m_missingObjectStatus = 0;
    m_motionDetectionHealtStatus = 0;
    m_objectIntrusionHealthStatus = 0;
    m_objectcountingStatus = 0;
    m_recordingHealthStatus = 0;
    m_suspiousObjectStatus = 0;
    m_tripWireHealthStatus = 0;
    m_viewTemperingHealthStatus = 0;
    applController = ApplController::getInstance();
    QString backColor = (((m_windowType == WINDOW_TYPE_DISPLAYSETTINGS)
                          || (m_windowType == WINDOW_TYPE_SEQUENCESETTINGS))
                         ? TRANSPARENTKEY_COLOR : CLICKED_BKG_COLOR);

    //border for window
	m_borderRect = new LayoutWindowRectangle(0,
                                0,
                                this->width(),
								this->height(),
                                WINDOW_GRID_COLOR,
                                this);

    //window close button
    m_closeButtonIcon = new Image((this->width() - LAYOUT_WINDOW_LEFT_MARGIN(m_windowIconType)),
                                  LAYOUT_WINDOW_TOP_MARGIN(m_windowIconType),
                                  (m_windowIconPath + QString(CLOSE_IMG_SOURCE)),
                                  this,
                                  END_X_START_Y,
                                  WINDOW_CLOSE_BUTTON,
                                  true,
                                  true);
    m_closeButtonIcon->setVisible(false);
    connect(m_closeButtonIcon,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotImageClicked(int)));

    //window headertext
    m_camInfo.camName = "";
    m_camInfo.nameOsdPosition = TOP_LEFT;
    m_camInfo.statusOsdPosition = OSD_NONE;
    if(m_windowType == WINDOW_TYPE_LAYOUT)
    {
        m_openToolbarIcon = new Image(SCALE_WIDTH(BORDER_WIDTH),
                                      SCALE_HEIGHT(BORDER_WIDTH),
                                      (m_windowIconPath + QString(OPEN_TOOLBAR_IMG_PATH)),
                                      this,
                                      START_X_START_Y,
                                      WINDOW_OPEN_TOOLBAR_BUTTON,
                                      false,
                                      false);
        m_windowHeaderText = new TextWithBackground((m_openToolbarIcon->x() + m_openToolbarIcon->width()),
                                                    SCALE_HEIGHT(BORDER_WIDTH),
                                                    ((m_windowIconType == ICON_TYPE_8X8) ? (SCALE_FONT(EXTRA_SMALL_SUFFIX_FONT_SIZE)) : (NORMAL_FONT_SIZE)),
                                                    m_camInfo.camName,
                                                    this,
                                                    NORMAL_FONT_COLOR,
                                                    WINDOW_FONT_FAMILY,
                                                    ALIGN_START_X_CENTRE_Y,
                                                    0,
                                                    false,
                                                    backColor,
                                                    false,
                                                    255,
                                                    (m_closeButtonIcon->x() - (m_openToolbarIcon->x() + m_openToolbarIcon->width()) - MARGIN),
                                                    true);
    }
    else
    {
        m_windowHeaderText = new TextWithBackground(SCALE_WIDTH(BORDER_WIDTH),
                                                    SCALE_HEIGHT(BORDER_WIDTH),
                                                    ((m_windowIconType == ICON_TYPE_8X8) ? (SCALE_FONT(EXTRA_SMALL_SUFFIX_FONT_SIZE)) : (NORMAL_FONT_SIZE)),
                                                    m_camInfo.camName,
                                                    this,
                                                    NORMAL_FONT_COLOR,
                                                    WINDOW_FONT_FAMILY,
                                                    ALIGN_START_X_CENTRE_Y,
                                                    0,
                                                    false,
                                                    backColor,
                                                    false,
                                                    255,
                                                    (this->width() - (this->width() - m_closeButtonIcon->x()) - MARGIN - (SCALE_HEIGHT(BORDER_WIDTH) * 2)),
                                                    true);
    }
    if(IS_VALID_OBJ(m_windowHeaderText))
    {
    connect(m_windowHeaderText,
               SIGNAL(sigMouseHover(int,bool)),
               this,
               SLOT(slotMouseHover(int,bool)));
    }

    m_windowToolTip = new ToolTip((m_windowHeaderText->x() + m_windowHeaderText->width()/2),
                                  (m_windowHeaderText->y() + m_windowHeaderText->height() + 5),
                                  "",
                                  (parentWidget()->parentWidget()),
                                  START_X_START_Y);
    if(IS_VALID_OBJ(m_windowToolTip))
    {
        m_windowToolTip->setVisible(false);
    }

    if((m_windowType == WINDOW_TYPE_DISPLAYSETTINGS)
            || (m_windowType == WINDOW_TYPE_SEQUENCESETTINGS))
    {
        m_windowNumberText = new TextLabel((this->width() / 2),
                                           (this->height() / 2),
                                           SCALE_FONT(28),
                                           QString("%1").arg(m_windowIndex + 1),
                                           this,
                                           "#303030",
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_CENTRE_X_CENTER_Y,
                                           SCALE_WIDTH(200));
    }

    if(m_windowType == WINDOW_TYPE_DISPLAYSETTINGS)
    {
        m_sequenceIcon = new Image((this->width() - LAYOUT_WINDOW_LEFT_MARGIN(m_windowIconType)),
                                   (m_closeButtonIcon->y() + m_closeButtonIcon->height() + 10),
                                   (m_windowIconPath + sequenceImageSource[m_sequenceImgType]),
                                   this,
                                   END_X_START_Y,
                                   WINDOW_SEQUENCE_BUTTON,
                                   false,
                                   false);

        m_sequenceIcon->changeImage (IMAGE_TYPE_NORMAL);

        m_toolTip = new ToolTip((this->width () - LAYOUT_WINDOW_LEFT_MARGIN(m_windowIconType)),
                                (m_sequenceIcon->y () + m_sequenceIcon->height ()),
                                "Configure",
                                this,
                                END_X_START_Y);

        m_toolTip->setVisible (false);

    }

    //playback icon
    m_pbIconPath = "";
    m_pbIcon = new Image(m_closeButtonIcon->x(),
                         (m_closeButtonIcon->y() + m_closeButtonIcon->height() + LAYOUT_WINDOW_TOP_MARGIN(m_windowIconType)),
                         "",
                         this,
                         START_X_START_Y,
                         0,
                         false,
                         true);

    m_audioOnIcon = new Image(m_pbIcon->x(),
                              (m_pbIcon->y()  + m_pbIcon->height() + SCALE_HEIGHT(10)),
                              "",
                              this,
                              START_X_START_Y,
                              1,
                              false,
                              true);

    if(m_audioOnIcon != NULL)
    {
        m_audioOnIcon->setVisible(false);
    }

    //window centerIcon image
    if(m_windowType == WINDOW_TYPE_LAYOUT)
    {
        m_centerWindowIcon = new Image((this->width() / 2),
                                       (this->height() / 2),
									   WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE,
                                       this,
                                       CENTER_X_CENTER_Y,
                                       WINDOW_ADD_CAMERA_BUTTON,
                                       false,
                                       true);
    }
    else
    {
        m_centerWindowIcon = new Image((this->width() / 2),
                                       (this->height() / 2),
                                       "",
                                       this,
                                       CENTER_X_CENTER_Y,
                                       WINDOW_ADD_CAMERA_BUTTON,
                                       false,
                                       true);
    }
	m_ActivateCenterWindowIconHoverF = true;
    
    /* To display the tool tip on decoder error on live view screen(Live view,async Pb, instant Pb) */
    /* On default the tooltip is not displayed as not sure on will decoder error present, neither will the mouse be always pointing on the the decoder image */
    /* Here the center window icon width and height may varry and it may change the x and y cordinates of the tooltip hence require a defined value */
    /* Here the parent widget is given as the tooltip is to be displayed on other window also if it overlaps */

    m_decoderTooltip = new ToolTip(this->x() + (this->width() / 2),
                                   this->y() + (this->height() / 2)  + m_centerWindowIcon->height()/2,
                                   "Maximum Decoding Capacity Reached.",
                                   parentWidget(),
                                   CENTER_X_CENTER_Y);

    m_decoderTooltip->setFontSize((SCALE_FONT(EXTRA_SMALL_SUFFIX_FONT_SIZE)));
    m_decoderTooltip->setVisible(false);

    for(qint8 index = 0; index < MAX_WINDOW_ICON; index++)
    {
        m_windowIconImageSource[index] = "";
        m_windowIcons[index] = new WindowIcon(BOTTOM_RIGHT,
                                              index,
                                              (m_windowIconPath + m_windowIconImageSource[index]),
                                              m_windowIconType,
                                              this);
    }

    if (!IS_VALID_OBJ(m_windowHeaderToolTip))
    {
        m_windowHeaderToolTip = new ToolTip((m_windowHeaderText->x() + SCALE_WIDTH(200) ),
                                            (m_windowHeaderText->y()+m_windowHeaderText->height() + SCALE_HEIGHT(20)),
                                            (""),
                                            this,
                                            START_X_START_Y);
    }
    m_windowHeaderToolTip->setVisible(false);

    if(m_windowType == WINDOW_TYPE_LAYOUT)
    {
        m_apNextTile = new Rectangle(0,
                                     0,
                                     0,
                                     0,
                                     TRANSPARENTKEY_COLOR,
									 m_borderRect,
                                     0,
                                     0,
                                     BORDER_1_COLOR,
                                     0);

        if(IS_VALID_OBJ(m_apNextTile))
        {     
            m_apReloadIcon = new Image((m_apNextTile->width() / 2),
                                       (m_apNextTile->height() / 2),
                                       (AP_TOOLBAR_IMAGE_PATH + apButtonSizeFolder[m_apToolbarSize] + apButtonTypeFolder[AP_TOOLBAR_RELOAD]) ,
                                       m_apNextTile,
                                       CENTER_X_CENTER_Y,
                                       AP_TOOLBAR_RELOAD,
                                       true,
                                       false);
            if(IS_VALID_OBJ(m_apReloadIcon))
            {
                connect(m_apReloadIcon,
                        SIGNAL(sigImageClicked(int)),
                        this,
                        SLOT(slotAPToolbarBtnClicked(int)));
            }

            m_apPrevIcon = new Image((m_apNextTile->width() / 2),
                                     (m_apNextTile->height() / 2),
                                     (AP_TOOLBAR_IMAGE_PATH + apButtonSizeFolder[m_apToolbarSize] + apButtonTypeFolder[AP_TOOLBAR_PREVIOUS]) ,
                                     m_apNextTile,
                                     CENTER_X_CENTER_Y,
                                     AP_TOOLBAR_PREVIOUS,
                                     true);
            if(IS_VALID_OBJ(m_apPrevIcon))
            {
                connect(m_apPrevIcon,
                        SIGNAL(sigImageClicked(int)),
                        this,
                        SLOT(slotAPToolbarBtnClicked(int)));
            }

            m_apNextIcon = new Image((m_apNextTile->width() / 2),
                                     (m_apNextTile->height() / 2),
                                     (AP_TOOLBAR_IMAGE_PATH + apButtonSizeFolder[m_apToolbarSize] + apButtonTypeFolder[AP_TOOLBAR_NEXT]) ,
                                     m_apNextTile,
                                     CENTER_X_CENTER_Y,
                                     AP_TOOLBAR_NEXT,
                                     true);
            if(IS_VALID_OBJ(m_apNextIcon))
            {
                connect(m_apNextIcon,
                        SIGNAL(sigImageClicked(int)),
                        this,
                        SLOT(slotAPToolbarBtnClicked(int)));
            }

            m_apNextVideoText = new TextLabel((m_apNextTile->width() / 2),
                                              (m_apNextTile->height() / 2),
                                              SCALE_FONT(28),
                                              ((QString("Next Video") + QString(" : %1s")).arg(MAX_AP_TIMER_COUNT)),
                                              m_apNextTile,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_CENTRE_X_CENTER_Y,
                                              SCALE_WIDTH(200));
            showAPToolbar(false, false, false);
        }
    }

    this->setWhatsThis("LayoutWindow");
    this->setEnabled(true);
    //If mouse tracking is switched off, mouse move events only occur if a mouse button is pressed while the mouse is being moved.
    //If mouse tracking is switched on, mouse move events occur even if no mouse button is pressed.
    this->setMouseTracking(true);
    //This is for "bool eventFilter(QObject *, QEvent *)"
    this->installEventFilter(this);

    this->setAcceptDrops(true);
    this->show();
}

LayoutWindow::~LayoutWindow()
{

    if(IS_VALID_OBJ(m_apReloadIcon))
    {
        disconnect(m_apReloadIcon,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotAPToolbarBtnClicked(int)));
        DELETE_OBJ(m_apReloadIcon);
    }

    if(IS_VALID_OBJ(m_apPrevIcon))
    {
        disconnect(m_apPrevIcon,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotAPToolbarBtnClicked(int)));
        DELETE_OBJ(m_apPrevIcon);
    }

    if(IS_VALID_OBJ(m_apNextIcon))
    {
        disconnect(m_apNextIcon,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotAPToolbarBtnClicked(int)));
        DELETE_OBJ(m_apNextIcon);
    }

    DELETE_OBJ(m_apNextVideoText);
    DELETE_OBJ(m_apNextTile);
	DELETE_OBJ(m_borderRect);

    disconnect(m_closeButtonIcon,
               SIGNAL(sigImageClicked(int)),
               this,
               SLOT(slotImageClicked(int)));
    delete m_closeButtonIcon;

    if(IS_VALID_OBJ(m_windowHeaderText))
    {
        disconnect(m_windowHeaderText,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotMouseHover(int,bool)));
        DELETE_OBJ(m_windowHeaderText);
    }
    delete m_pbIcon;

    if(m_audioOnIcon != NULL)
    {
        delete m_audioOnIcon;
    }
    
        DELETE_OBJ(m_decoderTooltip);

    delete m_centerWindowIcon;

    for(qint8 index = 0; index < MAX_WINDOW_ICON; index++)
    {
        delete m_windowIcons[index];
    }

    if(m_windowNumberText != NULL)
    {
        delete m_windowNumberText;
    }

    if(m_sequenceIcon != NULL)
    {
        delete m_sequenceIcon;
        delete m_toolTip;
    }

    if(m_openToolbarIcon != NULL)
    {
        delete m_openToolbarIcon;
    }

    DELETE_OBJ(m_windowHeaderToolTip);
    DELETE_OBJ(m_windowToolTip);
}

void LayoutWindow::setWindowIconType(WINDOW_ICON_TYPE_e iconType)
{
    m_windowIconType = iconType;
    m_windowIconPath = QString(WINDOW_ICON_IMG_PATH) + QString(windowIconTypeImgPath[m_windowIconType]);

    m_centerImageSizePath = (iconType == ICON_TYPE_8X8) ?
                (centerImageSizePath[SMALL_CENTER_ICON]) : (centerImageSizePath[LARGE_CENTER_ICON]);

    m_closeButtonIcon->updateImageSource((m_windowIconPath + QString(CLOSE_IMG_SOURCE)));

    m_pbIcon->updateImageSource((m_windowIconPath + m_pbIconPath));

    m_audioOnIcon->updateImageSource(m_windowIconPath + videoStatusSource[AUDIO_ON_ICON]);

    for(qint8 index = 0; index < MAX_WINDOW_ICON; index++)
    {
        m_windowIcons[index]->setWindowIconType(m_windowIconType);
        m_windowIcons[index]->updateImageSource((m_windowIconPath + m_windowIconImageSource[index]));
    }

    if(m_sequenceIcon != NULL)
    {
        m_sequenceIcon->updateImageSource((m_windowIconPath + sequenceImageSource[m_sequenceImgType]),
                                          true);
    }

    if(m_openToolbarIcon != NULL)
    {
        m_openToolbarIcon->updateImageSource((m_windowIconPath + QString(OPEN_TOOLBAR_IMG_PATH)),
                                             true);
    }

    if(IS_VALID_OBJ(m_apReloadIcon))
    {
        m_apReloadIcon->updateImageSource((AP_TOOLBAR_IMAGE_PATH + apButtonSizeFolder[m_apToolbarSize]
                                         + apButtonTypeFolder[AP_TOOLBAR_RELOAD]),true);
    }

    if(IS_VALID_OBJ(m_apPrevIcon))
    {
        m_apPrevIcon->updateImageSource((AP_TOOLBAR_IMAGE_PATH + apButtonSizeFolder[m_apToolbarSize]
                                         + apButtonTypeFolder[AP_TOOLBAR_PREVIOUS]),true);
    }

    if(IS_VALID_OBJ(m_apNextIcon))
    {
        m_apNextIcon->updateImageSource((AP_TOOLBAR_IMAGE_PATH + apButtonSizeFolder[m_apToolbarSize]
                                         + apButtonTypeFolder[AP_TOOLBAR_NEXT]),true);
    }

    m_windowHeaderText->setFontSize((m_windowIconType == ICON_TYPE_8X8) ? (SCALE_FONT(EXTRA_SMALL_SUFFIX_FONT_SIZE)) : (NORMAL_FONT_SIZE));

    if(IS_VALID_OBJ(m_apNextVideoText))
    {
        m_apNextVideoText->setFontSize(SCALE_FONT(apFontSize[m_apToolbarSize]));
    }
}

void LayoutWindow::setPbtoolbarSize(quint8 size)
{
    m_pbToolbarSize = size;
}

quint8 LayoutWindow::getPbToolbarSize()
{
    return m_pbToolbarSize;
}

WINDOW_ICON_TYPE_e LayoutWindow::getWindowIconType()
{
    return m_windowIconType;
}

void LayoutWindow::updateWindowHeaderText(QString headerText)
{
    if(m_windowHeaderText->getText() != headerText)
    {
        m_windowHeaderText->changeText(headerText);
        m_windowHeaderText->update();
    }
    if((Layout::currentModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING) && (headerText != ""))
    {
        m_windowHeaderText-> setVisible (true);
    }
//    else
//    {
//        m_windowHeaderText->setFontSize(NORMAL_FONT_SIZE);
//        m_windowHeaderText->setVisible (true);
//        m_windowHeaderText->changeText(headerText);
//        m_windowHeaderText->update();
//        m_windowHeaderText->raise();
//    }
}

void LayoutWindow::updateWindowHeaderForVideoPopup(quint16 windowIndex)
{
    if(IS_VALID_OBJ(m_openToolbarIcon))
    {
         m_openToolbarIcon->setVisible(false);
    }

    quint8 eventNo;
    QString camId, camName, eventName = "", deviceName, headerText, hoverTextForVideoPopup;
    deviceName = applController->GetDispDeviceName(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName);
    camId = INT_TO_QSTRING(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);
    camName = m_camInfo.camName;
    eventNo = (Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].eventType);
    if (eventNo < LOG_MAX_CAMERA_EVENT)
    {
        eventName = Multilang(cameraEventSubTypeString[eventNo].toUtf8().constData());
    }

    hoverTextForVideoPopup = deviceName + " \n" + camName + " \n" + eventName;

    m_windowHeaderToolTip->textChange(hoverTextForVideoPopup);
    m_windowHeaderToolTip->resetGeometry((m_windowHeaderText->x() + SCALE_WIDTH(200) ),
                                     (m_windowHeaderText->y()+m_windowHeaderText->height() +SCALE_HEIGHT(20)));
    m_windowHeaderToolTip->update();

    switch(Layout::currentDisplayConfig[MAIN_DISPLAY].layoutId)
    {
    case THREE_X_THREE:
    {
        deviceName = HeaderTextForVideoPopup(deviceName ,maxValueForVideoPopup[ONE_PLUS_FIVE][0] );
        camName = HeaderTextForVideoPopup(camName ,maxValueForVideoPopup[ONE_PLUS_FIVE][1] );
        eventName = HeaderTextForVideoPopup(eventName ,maxValueForVideoPopup[ONE_PLUS_FIVE][2] );

        headerText = deviceName + " : "+  camId + " : " + camName + " : "+ eventName ;
    }
    break;
    case TWO_X_TWO:
    {
        deviceName = HeaderTextForVideoPopup(deviceName ,maxValueForVideoPopup[TWO_X_TWO][0] );
        camName = HeaderTextForVideoPopup(camName ,maxValueForVideoPopup[TWO_X_TWO][1] );
        eventName = HeaderTextForVideoPopup(eventName ,maxValueForVideoPopup[TWO_X_TWO][2] );

        headerText = deviceName + " : "+  camId + " : " + camName + " : "+ eventName ;
    }
    break;
    case ONE_X_ONE:
    {
        deviceName = HeaderTextForVideoPopup(deviceName ,maxValueForVideoPopup[ONE_X_ONE][0] );
        camName = HeaderTextForVideoPopup(camName ,maxValueForVideoPopup[ONE_X_ONE][1] );
        eventName = HeaderTextForVideoPopup(eventName ,maxValueForVideoPopup[ONE_X_ONE][2] );

        headerText = deviceName + " : "+  camId + " : " + camName + " : "+ eventName ;
    }
        break;

    default:
        break;
    }

    if(m_windowHeaderText->getText() != headerText)
    {
       m_windowHeaderText->setBold(true);
        m_windowHeaderText->changeText(headerText);
    }

    m_closeButtonIcon->setVisible(false);
}

QString LayoutWindow::HeaderTextForVideoPopup(QString header, quint8 maxValue)
{
   return  (header.size() > maxValue) ? header.left(maxValue) + "... ": header;
}

void LayoutWindow::showAPToolbar(bool isApToolbarVisible, bool isPrevEnable, bool isNextEnable)
{
    if(IS_VALID_OBJ(m_apNextTile))
    {
        if(isApToolbarVisible)
        {
            m_apNextTile->setOpacity(1);
            m_apNextTile->changeColor(SHADOW_FONT_COLOR);
			m_apNextTile->resetGeometry(0,
										0,
										this->width(),
										this->height());

            if(IS_VALID_OBJ(m_apReloadIcon))
            {
                m_apReloadIcon->resetGeometry((m_apNextTile->width()/ 2),
                                              (m_apNextTile->height()/ 2));
            }
            if(IS_VALID_OBJ(m_apPrevIcon))
            {
                m_apPrevIcon->resetGeometry(((m_apNextTile->width()/ 2) - ((m_apNextTile->width() * apSizeButtonMultiplier[m_apToolbarSize])/
                                                                           apSizeButtonDividend[m_apToolbarSize])),
                                            (m_apNextTile->height()/ 2));
            }
            if(IS_VALID_OBJ(m_apNextIcon))
            {
                m_apNextIcon->resetGeometry(((m_apNextTile->width()/ 2) + ((m_apNextTile->width() * apSizeButtonMultiplier[m_apToolbarSize])/
                                                                           apSizeButtonDividend[m_apToolbarSize])),
                                            (m_apNextTile->height()/ 2));
            }
            if(IS_VALID_OBJ(m_apNextVideoText) && IS_VALID_OBJ(m_apReloadIcon))
            {
                m_apNextVideoText->setOffset((m_apNextTile->width()/2),
                                             (m_apReloadIcon->y() + m_apReloadIcon->height() + SCALE_HEIGHT(apNextVideoTextVerOffset[m_apToolbarSize])));
            }
        }
        else
        {
            m_apNextTile->setOpacity(0);
            m_apNextTile->changeColor(TRANSPARENTKEY_COLOR);
            m_apNextTile->resetGeometry(0, 0, 0, 0);
        }
    }
    if(IS_VALID_OBJ(m_apPrevIcon))
    {
        m_apPrevIcon->setIsEnabled(isPrevEnable);
    }
    if(IS_VALID_OBJ(m_apNextIcon))
    {
        m_apNextIcon->setIsEnabled(isNextEnable);
    }
    if(IS_VALID_OBJ(m_apNextVideoText))
    {
        m_apNextVideoText->setVisible(isApToolbarVisible && isNextEnable);
    }
}

void LayoutWindow::updateApToolbar(quint16 windowIndex)
{
    showAPToolbar(Layout::autoPlayData.autoPlayFeatureDataType[windowIndex].m_isApToolbarVisible,
                  Layout::autoPlayData.autoPlayFeatureDataType[windowIndex].m_isPrevEnable,
                  Layout::autoPlayData.autoPlayFeatureDataType[windowIndex].m_isNextEnable);
}

void LayoutWindow::setAPToolbarSize(quint8 size)
{
    m_apToolbarSize = size;
}

void LayoutWindow::updateWindowHeaderOSD()
{
    m_windowHeaderText->setFontSize((m_windowIconType == ICON_TYPE_8X8) ? (SCALE_FONT(EXTRA_SMALL_SUFFIX_FONT_SIZE)) : (NORMAL_FONT_SIZE));
    switch(m_camInfo.nameOsdPosition)
    {
    case OSD_NONE:
        if(Layout::currentModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING)
        {
            m_windowHeaderText->setVisible(true);
        }
        else
        {
            m_windowHeaderText->setVisible(false);
        }
        break;

    case TOP_RIGHT:
        m_windowHeaderText->setOffset((this->width() - SCALE_WIDTH(BORDER_WIDTH)),
                                      SCALE_HEIGHT(BORDER_WIDTH),
                                      ALIGN_END_X_START_Y);
        break;

    case TOP_LEFT:
		if(IS_VALID_OBJ(m_openToolbarIcon))
        {
            m_windowHeaderText->setOffset((m_openToolbarIcon->x() + m_openToolbarIcon->width()),
                                          SCALE_HEIGHT(BORDER_WIDTH),
                                          ALIGN_START_X_START_Y);
        }
        else
        {
            m_windowHeaderText->setOffset(SCALE_WIDTH(BORDER_WIDTH) + SCALE_WIDTH(5),
                                          SCALE_HEIGHT(BORDER_WIDTH),
                                          ALIGN_START_X_START_Y);
        }
        break;

    case BOTTOM_LEFT:
        m_windowHeaderText->setOffset(SCALE_WIDTH(BORDER_WIDTH),
                                      (this->height() - SCALE_HEIGHT(BORDER_WIDTH)),
                                      ALIGN_START_X_END_Y);
        break;

    case BOTTOM_RIGHT:
        m_windowHeaderText->setOffset((this->width() - SCALE_WIDTH(BORDER_WIDTH)),
                                      (this->height() - SCALE_HEIGHT(BORDER_WIDTH)),
                                      ALIGN_END_X_END_Y);
        break;

    default:
        break;
    }

    if((m_camInfo.nameOsdPosition != OSD_NONE)
            && (!m_windowHeaderText->isVisible()))
    {
        m_windowHeaderText->setVisible(true);
    }
}

void LayoutWindow::clearWindowIconImageSources(quint8 iconIndex)
{
    for(quint8 index = iconIndex; index < MAX_WINDOW_ICON; index++)
    {
        m_windowIconImageSource[index] = "";
        m_windowIcons[index]->updateImageSource("");
    }
}

void LayoutWindow::changeWindowColor(QString color)
{
    if(m_windowColor != color)
    {
        m_windowColor = color;
        update();
    }
}

void LayoutWindow::updateCamInfoOnWindow(quint16 arrayWindowIndex)
{
    if(Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_deviceName != "")
    {
        Layout::getOSDStatus(MAIN_DISPLAY, arrayWindowIndex, m_camInfo);
        if((Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
                || (Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
                || (Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoType == VIDEO_TYPE_SYNCPLAYBAKSTREAM))
        {
            m_camInfo.nameOsdPosition = TOP_LEFT;
            m_camInfo.statusOsdPosition = TOP_RIGHT;
			if(IS_VALID_OBJ(m_openToolbarIcon))
			{
				m_openToolbarIcon->setVisible (false);
			}
        }
		else if(IS_VALID_OBJ(m_openToolbarIcon))
        {
            if(m_openToolbarIcon->isVisible() == false)
            {
                m_openToolbarIcon->setVisible(true);
            }

            if(Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_errorType == VIDEO_ERROR_DISABLECAMERA)
            {
                m_openToolbarIcon->changeImage(IMAGE_TYPE_DISABLE, true);
            }
            else
            {
                m_openToolbarIcon->changeImage(IMAGE_TYPE_NORMAL, true);
            }
        }

        m_arrayIndex = arrayWindowIndex;

        if (m_windowType == WINDOW_TYPE_VIDEO_POP_UP)
        {
            updateWindowHeaderForVideoPopup(arrayWindowIndex);
        }
        else
        {
            updateWindowHeaderText(applController->GetDispDeviceName(Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_deviceName)
                                   + ":" + INT_TO_QSTRING(Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_cameraId)
                                   + ":" + m_camInfo.camName );
        }
        updateWindowHeaderOSD();
        updateCameraStatusOSD();

        if(Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
        {
            m_arrayIndex = arrayWindowIndex;
            m_pbIconPath = QString(PLAYBACK_IMG_PATH);
            m_pbIcon->updateImageSource(m_windowIconPath + m_pbIconPath);
            m_audioOnIcon->updateImageSource(m_windowIconPath + videoStatusSource[AUDIO_ON_ICON]);
        }
        else if(Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
        {
            m_arrayIndex = arrayWindowIndex;
            m_pbIconPath = QString(INSTPLAYBACK_IMG_PATH);
            m_pbIcon->updateImageSource(m_windowIconPath + m_pbIconPath);
            m_audioOnIcon->updateImageSource(m_windowIconPath + videoStatusSource[AUDIO_ON_ICON]);
        }
        if(IS_VALID_OBJ(m_windowHeaderToolTip))
        {
            m_windowHeaderToolTip->setVisible(false);
        }
    }
    else
    {
        EPRINT(LAYOUT, "device name not found");
    }
}

void LayoutWindow::updateCameraStatusOSD()
{
    for(qint8 index = 0; index < MAX_WINDOW_ICON; index++)
    {
        m_windowIcons[index]->changeAlignmentType(m_camInfo.statusOsdPosition);
    }
}

void LayoutWindow::updateWindowData(quint16 arrayWindowIndex)
{
    VIDEO_STREAM_TYPE_e videoType = VIDEO_TYPE_NONE;
    VIDEO_STATUS_TYPE_e videoStatus = VIDEO_STATUS_NONE;
    VIDEO_ERROR_TYPE_e videoError = VIDEO_ERROR_NONE;
	quint8 totalImages              = 0;

    if(arrayWindowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        videoType   = Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoType;

        videoStatus = Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_videoStatus;
        videoError   = Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_errorType;

        m_audioOn = Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_audioStatus;
		m_microPhoneStatus = Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_microPhoneStatus;
    }

	if(videoType != VIDEO_TYPE_NONE)
	{
		m_ActivateCenterWindowIconHoverF = false;
	}
    switch(videoType)
    {
    case VIDEO_TYPE_LIVESTREAM:
        clearPlaybackRelatedInfo(arrayWindowIndex);
        updateCamInfoOnWindow(arrayWindowIndex);
		if((VIDEO_STATUS_RUNNING == videoStatus) ||
		   (VIDEO_ERROR_NO_DECODING_CAP == videoError))
		{
			Layout::getRecordingStatus(MAIN_DISPLAY,
									   arrayWindowIndex,
									   m_recordingHealthStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   MOTION_DETECTION_STS,
											   m_motionDetectionHealtStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   VIEW_TAMPERED_STS,
											   m_viewTemperingHealthStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   OBJECT_INTRUSION_STS,
											   m_objectIntrusionHealthStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   TRIP_WIRE_STS,
											   m_tripWireHealthStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   AUDIO_EXCEPTION_STS,
											   m_audioExceptionHealthStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   MISSING_OBJJECT_STS,
											   m_missingObjectStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   SUSPIOUS_OBJECT_STS,
											   m_suspiousObjectStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   LOITERING_OBJECT_STS,
											   m_loiteringStatus);

			Layout::getSingleHealthParamStatus(MAIN_DISPLAY,
											   arrayWindowIndex,
											   OBJECT_COUNTING_STS,
											   m_objectcountingStatus);

			if(m_tripWireHealthStatus)
			{
				m_objectIntrusionHealthStatus = 0;
				m_motionDetectionHealtStatus = 0;
			}
			else if(m_objectIntrusionHealthStatus)
			{
				m_motionDetectionHealtStatus = 0;
			}
		}

        switch(videoStatus)
        {
        case VIDEO_STATUS_CONNECTING:
            if((m_centerWindowIcon->getImageSource() != WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_DECODER_ERR_IMG_SOURCE)
                    && (m_decoderTooltip->isVisible()))
            {
                m_decoderTooltip->setVisible(false);
            }
            break;

        case VIDEO_STATUS_RUNNING:
            m_centerWindowIcon->updateImageSource("");

            totalImages = 0;

            if(m_recordingHealthStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[RECORDING_STATUS_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_tripWireHealthStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[TRIP_WIRE_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_objectIntrusionHealthStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[OBJECT_INTRUSION_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_motionDetectionHealtStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[MOTION_DETECTION_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_viewTemperingHealthStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[VIEW_TEMPERING_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_audioExceptionHealthStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[AUDIO_EXCEPTION_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_audioOn)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[AUDIO_ON_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

			if(m_microPhoneStatus)
			{
				m_windowIconImageSource[totalImages] = videoStatusSource[MICROPHONE_STATUS_ICON];
				m_windowIcons[totalImages]->updateImageSource
						(m_windowIconPath + m_windowIconImageSource[totalImages]);
				totalImages++;
			}

            if(m_missingObjectStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[MISSING_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_suspiousObjectStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[SUSPICIOUS_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }

            if(m_loiteringStatus)
            {
                m_windowIconImageSource[totalImages] = videoStatusSource[LOITRING_ICON];
                m_windowIcons[totalImages]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[totalImages]);
                totalImages++;
            }
            if(m_objectcountingStatus)
            {
                //Add window ICON Image
            }
                /* Do not show the tooltip if the live view starts or when no camera found on window */
            if(m_decoderTooltip->isVisible())
            {
                m_decoderTooltip->setVisible(false);
            }

            clearWindowIconImageSources (totalImages);
            changeWindowColor(TRANSPARENTKEY_COLOR);

            break;

        case VIDEO_STATUS_VIDEOLOSS:
		case VIDEO_STATUS_RETRY:
		case VIDEO_STATUS_EVENTWAIT:
		case VIDEO_STATUS_ERROR:
			totalImages = 0;
			if(VIDEO_ERROR_NO_DECODING_CAP == videoError)
			{
				m_centerWindowIcon->updateImageSource(
							WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_DECODER_ERR_IMG_SOURCE);
				if(m_recordingHealthStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[RECORDING_STATUS_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_tripWireHealthStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[TRIP_WIRE_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_objectIntrusionHealthStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[OBJECT_INTRUSION_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_motionDetectionHealtStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[MOTION_DETECTION_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_viewTemperingHealthStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[VIEW_TEMPERING_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_audioExceptionHealthStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[AUDIO_EXCEPTION_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_audioOn)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[AUDIO_ON_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_microPhoneStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[MICROPHONE_STATUS_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_missingObjectStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[MISSING_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_suspiousObjectStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[SUSPICIOUS_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_loiteringStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[LOITRING_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}
				if(m_objectcountingStatus)
				{
					//Add window ICON Image
				}
			}
			else
			{
                /* Do not show the tooltip if error is other than decoder error */
                if(m_decoderTooltip->isVisible())
                {
                    m_decoderTooltip->setVisible(false);
                }

				m_centerWindowIcon->updateImageSource(
							WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
				m_windowIconImageSource[totalImages] = (videoStatusSource[CAMERA_STATUS_ICON] +
															   videoErrorSource[videoError]);
				m_windowIcons[totalImages]->updateImageSource(m_windowIconPath +
																 m_windowIconImageSource[totalImages]);
				totalImages++;
			}
			if(VIDEO_STATUS_VIDEOLOSS == videoStatus)
			{
				Layout::getRecordingStatus(MAIN_DISPLAY,
										   arrayWindowIndex,
										   m_recordingHealthStatus);
				if(m_recordingHealthStatus == EVENT_START)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[RECORDING_STATUS_ICON];
					m_windowIcons[totalImages]->updateImageSource(m_windowIconPath +
																  m_windowIconImageSource[totalImages]);
					totalImages++;
				}

				if(m_audioOn)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[AUDIO_ON_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}
				if(m_microPhoneStatus)
				{
					m_windowIconImageSource[totalImages] = videoStatusSource[MICROPHONE_STATUS_ICON];
					m_windowIcons[totalImages]->updateImageSource
							(m_windowIconPath + m_windowIconImageSource[totalImages]);
					totalImages++;
				}
			}

			clearWindowIconImageSources (totalImages);
			changeWindowColor("");
            break;

        default:
            m_centerWindowIcon->updateImageSource(
                        WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
            clearWindowIconImageSources ();
            changeWindowColor("");
            break;
        }
        break;

    case VIDEO_TYPE_PLAYBACKSTREAM:
    case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:

        clearWindowIconImageSources ();
        updateCamInfoOnWindow(arrayWindowIndex);

        switch(videoStatus)
        {
        case VIDEO_STATUS_CONNECTING:
            if(m_centerWindowIcon->getImageSource() != WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_DECODER_ERR_IMG_SOURCE)
            {
                if(m_decoderTooltip->isVisible())
                {
                    m_decoderTooltip->setVisible(false);
                }
            }
			break;

        case VIDEO_STATUS_RETRY:
		case VIDEO_STATUS_ERROR:
		case VIDEO_STATUS_VIDEOLOSS:
			if(VIDEO_ERROR_NO_DECODING_CAP == videoError)
			{
				m_centerWindowIcon->updateImageSource(
							WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_DECODER_ERR_IMG_SOURCE);
			}
			else
			{
                if(m_decoderTooltip->isVisible())
                {
                    m_decoderTooltip->setVisible(false);
                }

				m_centerWindowIcon->updateImageSource(
							WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
				m_windowIconImageSource[CAMERA_STATUS_ICON] = (videoStatusSource[CAMERA_STATUS_ICON] +
															   videoErrorSource[videoError]);
				m_windowIcons[CAMERA_STATUS_ICON]->updateImageSource(m_windowIconPath +
																	 m_windowIconImageSource[CAMERA_STATUS_ICON]);
			}
            changeWindowColor("");
            break;

        case VIDEO_STATUS_RUNNING:
            m_centerWindowIcon->updateImageSource("");

            if((m_audioOnIcon != NULL) && (m_pbIcon != NULL))
            {
                m_audioOnIcon->resetGeometry(m_pbIcon->x(),
                                             m_pbIcon->y() + m_pbIcon->height () + SCALE_HEIGHT(10));

                if(m_audioOn)
                {
                    m_audioOnIcon->setVisible(true);
                }
                else
                {
                    m_audioOnIcon->setVisible(false);
                }
            }

            m_apTimerCount = Layout::autoPlayData.autoPlayFeatureDataType[arrayWindowIndex].m_timerCount;
            if(IS_VALID_OBJ(m_apNextVideoText))
            {
                m_apNextVideoText->changeText((QString("Next Video") + QString(" : %1s")).arg(MAX_AP_TIMER_COUNT - m_apTimerCount));
            }
            updateApToolbar(arrayWindowIndex);
            
            if(m_decoderTooltip->isVisible())
            {
                m_decoderTooltip->setVisible(false);
            }
            
            // Raise due to show on auto play tile
            m_openToolbarIcon->raise();
            m_windowHeaderText->raise();
            m_closeButtonIcon->raise();
            changeWindowColor(TRANSPARENTKEY_COLOR);

            break;

        default:
            break;
        }
        break;

    case VIDEO_TYPE_SYNCPLAYBAKSTREAM:
        /* Do not show the decoder tooltip if sync playback is open */
        if(m_decoderTooltip->isVisible())
        {
            m_decoderTooltip->setVisible(false);
        }

        clearPlaybackRelatedInfo(arrayWindowIndex);
        clearWindowIconImageSources ();

        updateCamInfoOnWindow(arrayWindowIndex);

        switch(videoStatus)
        {
        case VIDEO_STATUS_RUNNING:
            m_centerWindowIcon->updateImageSource("");
            if(m_audioOn)
            {
                m_windowIcons[AUDIO_ON_ICON]->changeAlignmentType(BOTTOM_RIGHT);
                m_windowIcons[AUDIO_ON_ICON]->changeAlignmentOffset(0);
                m_windowIconImageSource[AUDIO_ON_ICON] = videoStatusSource[AUDIO_ON_ICON];
                m_windowIcons[AUDIO_ON_ICON]->updateImageSource
                        (m_windowIconPath + m_windowIconImageSource[AUDIO_ON_ICON]);
            }
            changeWindowColor(TRANSPARENTKEY_COLOR);
            break;

		case VIDEO_STATUS_VIDEOLOSS:
		case VIDEO_STATUS_ERROR:
			if(VIDEO_ERROR_NO_DECODING_CAP == videoError)
			{
				m_centerWindowIcon->updateImageSource(
							WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_DECODER_ERR_IMG_SOURCE);
			}
			else
			{
				m_centerWindowIcon->updateImageSource(
							WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
				m_windowIconImageSource[CAMERA_STATUS_ICON] = (videoStatusSource[CAMERA_STATUS_ICON] +
															   videoErrorSource[videoError]);
				m_windowIcons[CAMERA_STATUS_ICON]->updateImageSource(m_windowIconPath +
																	 m_windowIconImageSource[CAMERA_STATUS_ICON]);
			}
			changeWindowColor("");
			break;

        default:
            break;
        }
        break;

    case VIDEO_TYPE_NONE:
    {
        m_windowHeaderToolTip->setVisible(false);
        clearWindow(arrayWindowIndex);
        Layout::resetAutoPlayFeatureInWindow(arrayWindowIndex);
        break;
    }
    case VIDEO_TYPE_LIVESTREAM_AWAITING:
        /* Do not show the decoder tooltip if im waiting state */
        if(m_decoderTooltip->isVisible())
        {
            m_decoderTooltip->setVisible(false);
        }

        clearPlaybackRelatedInfo(arrayWindowIndex);
        clearWindowIconImageSources ();

        updateCamInfoOnWindow(arrayWindowIndex);
        m_centerWindowIcon->updateImageSource(
                    WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
        m_windowIconImageSource[CAMERA_STATUS_ICON] = (videoStatusSource[CAMERA_STATUS_ICON] +
                                                       videoErrorSource[VIDEO_ERROR_DEVICEDISCONNECTED]);
        m_windowIcons[CAMERA_STATUS_ICON]->updateImageSource(m_windowIconPath +
                                                             m_windowIconImageSource[CAMERA_STATUS_ICON]);
        changeWindowColor("");
        break;
    }
    raiseWindowIcons();
    update();
}

void LayoutWindow::setWinHeaderTextForDispSetting(QString camName)
{
    if(m_windowHeaderText->getText() != camName)
    {
        m_windowHeaderText->changeText(camName);

        if(m_windowHeaderText->getText() == "")
        {
            if(m_closeButtonIcon->isVisible())
            {
                m_closeButtonIcon->setVisible(false);
            }
        }
    }
}

void LayoutWindow::setWindowNumber(quint16 index)
{
    if(m_windowNumberText != NULL)
    {
        m_windowNumberText->changeText(QString("%1").arg(index + 1));
        m_windowNumberText->update();
    }
}

void LayoutWindow::selectWindow()
{
	m_borderRect->changeColor(HIGHLITED_FONT_COLOR);
    this->setFocus ();
    update();
}

void LayoutWindow::deselectWindow()
{
	m_borderRect->changeColor(WINDOW_GRID_COLOR);
    if(m_closeButtonIcon->isVisible())
    {
        m_closeButtonIcon->setVisible(false);
    }
    update();
}

void LayoutWindow::clearWindow(quint16 arrayWindowIndex)
{
    clearWindowIconImageSources ();
    clearPlaybackRelatedInfo(arrayWindowIndex);
    clearWindowData();

    if(m_windowType == WINDOW_TYPE_LAYOUT)
    {
        m_windowHeaderToolTip->setVisible(false);
		m_ActivateCenterWindowIconHoverF = true;
        /* Do not show the decoder tooltip when window gets cleared */
        if(m_decoderTooltip->isVisible())
        {
            m_decoderTooltip->setVisible(false);
        }
    }
    
	m_centerWindowIcon->updateImageSource(
				WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
	changeWindowColor("");

    if(m_closeButtonIcon->isVisible())
    {
        m_closeButtonIcon->setVisible(false);
    }
	if(IS_VALID_OBJ(m_openToolbarIcon))
    {
        m_openToolbarIcon->setVisible(false);
    } 
}

void LayoutWindow::clearWindowData()
{
    updateWindowHeaderText("");
    m_camInfo.camName = "";
    m_camInfo.nameOsdPosition = OSD_NONE;
    m_camInfo.statusOsdPosition = OSD_NONE;
}

void LayoutWindow::clearPlaybackRelatedInfo(quint16 arrayWindowIndex)
{
    m_pbIconPath = "";
    m_pbIcon->updateImageSource(m_windowIconPath + m_pbIconPath);
    m_audioOnIcon->updateImageSource(m_windowIconPath + "");
    updateApToolbar(arrayWindowIndex);
}

void LayoutWindow::changeWindowType(WINDOW_TYPE_e windowType)
{
    m_windowType = windowType;
}

WINDOW_TYPE_e LayoutWindow::getWindowType(void)
{
    return (m_windowType);
}

void LayoutWindow::raiseWindowIcons()
{
    //border for window
	m_borderRect->raise();

    //window headertext

    if(m_windowType == WINDOW_TYPE_LAYOUT)
    {
        m_openToolbarIcon->raise();
        m_windowHeaderText->raise();
    }
    else
    {
        m_windowHeaderText->raise();
    }

    if((m_windowType == WINDOW_TYPE_DISPLAYSETTINGS)
            || (m_windowType == WINDOW_TYPE_SEQUENCESETTINGS))
    {
        m_windowNumberText->raise();
    }

    //window close button
    m_closeButtonIcon->raise();

    if(m_windowType == WINDOW_TYPE_DISPLAYSETTINGS)
    {
        m_sequenceIcon->raise();
        m_toolTip->raise();
    }

    m_pbIcon->raise();

    for(qint8 index = 0; index < MAX_WINDOW_ICON; index++)
    {
        m_windowIcons[index]->raise();
    }
}

quint8 LayoutWindow::getWindowIndex()
{
    return m_windowIndex;
}

void LayoutWindow::forceActiveFocus()
{
    this->setFocus();
}

void LayoutWindow::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
    }
}

void LayoutWindow::takeMenuKeyAction()
{
    emit sigLoadMenuListOptions(m_windowIndex);
}

void LayoutWindow::takeEnterKeyAction()
{
    emit sigWindowSelected(m_windowIndex);
}

void LayoutWindow::takeCancelKeyAction()
{
    if(m_closeButtonIcon->isVisible())
    {
        m_closeButtonIcon->takeEnterKeyAction();
    }
    else
    {
        emit sigWindowImageHover(WINDOW_CLOSE_BUTTON, m_windowIndex, true);
    }
}

quint8 LayoutWindow::getFirstClickedWindow()
{
    return m_firstClickedWindow;
}

//QEvent will receieve event whenever that object
//Write different if condition as following
bool LayoutWindow::eventFilter(QObject * obj, QEvent * event)
{

    if(event->type() == QEvent::Leave)
    {
        if(m_closeButtonIcon->isVisible())
        {
            m_closeButtonIcon->setVisible(false);
        }

		if((m_centerWindowIcon->getImageSource() ==
			(WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CAMERA_ADD_BUTTON_IMG_SOURCE)))
		{
			m_centerWindowIcon->updateImageSource(
						WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE);
		}

        if(m_sequenceIcon != NULL)
        {
            m_sequenceIcon->changeImage(IMAGE_TYPE_NORMAL);
            m_toolTip->setVisible (false);
        }

        if(m_openToolbarIcon != NULL)
        {
            if(Layout::streamInfoArray[MAIN_DISPLAY][m_arrayIndex].m_errorType == VIDEO_ERROR_DISABLECAMERA)
            {
                m_openToolbarIcon->changeImage(IMAGE_TYPE_DISABLE, true);
            }
            else
            {
                m_openToolbarIcon->changeImage(IMAGE_TYPE_NORMAL);
            }
        }

		if(m_mouseClicked)
        {
			if(VIDEO_STATUS_CONNECTING != Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_videoStatus)
			{
				m_firstClickedWindow = m_windowIndex;
			}
            else
            {
                m_firstClickedWindow = MAX_WINDOWS;
            }
			m_mouseClicked = false;
        }
        else
        {
            m_firstClickedWindow = MAX_WINDOWS;
        }

        if(m_decoderTooltip->isVisible())
        {
            m_decoderTooltip->setVisible(false);
        }
    }
    else if(event->type() == QEvent::Enter)
    {
        if((m_firstClickedWindow != m_windowIndex) && 
            (m_firstClickedWindow != MAX_WINDOWS) && 
            (VIDEO_STATUS_CONNECTING != Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_videoStatus))
        {
            emit sigSwapWindow(m_firstClickedWindow,m_windowIndex);
        }
        m_firstClickedWindow = MAX_WINDOWS;
    }

    /*
     * Can add new event filter as
        if(event->type() == QEvent::______)
        {

        }
        //instead of Underscore add desired event ex. QEvent::Move
        //To know about more event press Ctrl + (Click on event name ex.Enter)
    */
    return QObject::eventFilter(obj, event);
}

void LayoutWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    QColor color;
    if(m_windowColor == "")
    {
        color = QColor(SHADOW_FONT_COLOR);
		painter.fillRect(0,
						 0,
						 this->width(),
						 this->height(),
                         color);
    }
    else
    {
        color = QColor(TRANSPARENTKEY_COLOR);
		painter.fillRect(0,
						 0,
						 this->width(),
						 this->height(),
                         Qt::transparent);
    }
//    painter.fillRect(BORDER_WIDTH,
//                     BORDER_WIDTH,
//                     (this->width() - (2 * BORDER_WIDTH)),
//                     (this->height() - (2 * BORDER_WIDTH)),
//                     color);
}

void LayoutWindow::mousePressEvent(QMouseEvent * event)
{
    /* Do not show the tooltip if mouse is pressed */
    if(m_decoderTooltip->isVisible())
    {
        m_decoderTooltip->setVisible(false);
    }

    if(event->button() == m_leftMouseButton)
    {
		if((m_firstClickedWindow == MAX_WINDOWS) &&
			(VIDEO_STATUS_CONNECTING != Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_videoStatus))
        {
            m_firstClickedWindow = m_windowIndex;
        }
        m_mouseClicked = true;
        if(m_centerWindowIcon->geometry().contains(event->pos()))
        {
			if((m_centerWindowIcon->getImageSource() ==
				(WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CAMERA_ADD_BUTTON_IMG_SOURCE)))
            {
                emit sigWindowImageClicked(WINDOW_ADD_CAMERA_BUTTON, m_windowIndex);
            }
            else
            {
                takeEnterKeyAction();
            }
        }
        else if((m_sequenceIcon != NULL)
                && (m_sequenceIcon->geometry().contains(event->pos())))
        {
            emit sigWindowImageClicked(WINDOW_SEQUENCE_BUTTON, m_windowIndex);
        }
        else if((m_openToolbarIcon != NULL)
                && (m_openToolbarIcon->geometry().contains(event->pos()))
                && (m_windowType != WINDOW_TYPE_VIDEO_POP_UP))
        {
            emit sigWindowImageClicked(WINDOW_OPEN_TOOLBAR_BUTTON, m_windowIndex);
        }
        else
        {
            takeEnterKeyAction();
        }
    }
    else if(event->button() == m_rightMouseButton)
    {
        m_mouseRightClicked = true;
    }

    QWidget::mousePressEvent(event);
}

void LayoutWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    /* Do not show the tooltip if mouse double clicked */
    if(m_decoderTooltip->isVisible())
    {
        m_decoderTooltip->setVisible(false);
    }
    
    if(event->button() == Qt::LeftButton)
    {
        emit sigWindowDoubleClicked(m_windowIndex);
    }
    m_firstClickedWindow = MAX_WINDOWS;
    m_mouseClicked = false;
    emit sigSwapWindow(MAX_WINDOWS,MAX_WINDOWS);
}

void LayoutWindow::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseRightClicked)
            && (event->button() == m_rightMouseButton))
    {
        takeMenuKeyAction();
    }
    m_mouseRightClicked = false;

    if(this->geometry().contains(mapToParent(event->pos())))
    {
        m_firstClickedWindow = MAX_WINDOWS;
        m_mouseClicked = false;
    }

    QWidget::mouseReleaseEvent(event);
}
//*****************************************************************************
//	LayoutWindow::mouseMoveEvent()
//	Description:
//  Qt return event on every mouse move
//  Mouse Move in Layout window:

//  Qt generates event on every mouse move

//  Buttons in LayoutWindow:
//  1)Close Button [X]
//  2)OpenToolBar Button
//  3)Sequence Button

//  When mouse positioned any of these button, respectively loading happnes
//  When mouse positioned on other than these buttons, only check for sequence button
//*****************************************************************************
void LayoutWindow::mouseMoveEvent(QMouseEvent * event)
{
	BOOL tCenterWindowIconHoverF = false;

    if(m_closeButtonIcon->geometry().contains(event->pos()))
    {   
        if(!m_closeButtonIcon->isVisible())
        {
            emit sigWindowImageHover(WINDOW_CLOSE_BUTTON, m_windowIndex, true);
        }

        if((m_sequenceIcon != NULL)
                && (m_sequenceIcon->getImageType() == IMAGE_TYPE_MOUSE_HOVER))
        {
            m_toolTip->setVisible (false);
            emit sigWindowImageHover(WINDOW_SEQUENCE_BUTTON, m_windowIndex, false);
        }
    }
    else if(m_centerWindowIcon->geometry().contains(event->pos()))
    {
         /* Checks whether the centreWindowimage is of decoding capacity or add camera and mouse drag is not happenning */
        if((m_centerWindowIcon->getImageSource() == WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_DECODER_ERR_IMG_SOURCE)
            && (m_mouseClicked == false))
        {
			if(Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_videoType == VIDEO_TYPE_SYNCPLAYBAKSTREAM)
            {
                m_decoderTooltip->setVisible(false);
            }
            else
            {
                /* Show the tooltip if mouse points at decoding image */
                /* The raise is used to show the tooltip on other windows */
                m_decoderTooltip->resetGeometry(this->x() + (this->width() / 2),
                                                this->y() + (this->height() / 2) + m_centerWindowIcon->height()/2);
                m_decoderTooltip->setVisible(true);
                m_decoderTooltip->raise();
            }
        }
        
		tCenterWindowIconHoverF = true;
        if(m_closeButtonIcon->isVisible())
        {
            emit sigWindowImageHover(WINDOW_CLOSE_BUTTON, m_windowIndex, false);
        }

        if((m_sequenceIcon != NULL)
                && (m_sequenceIcon->getImageType() == IMAGE_TYPE_MOUSE_HOVER))
        {
            m_toolTip->setVisible (false);
            emit sigWindowImageHover(WINDOW_SEQUENCE_BUTTON, m_windowIndex, false);
        }
    }
    else if((m_sequenceIcon != NULL)
            && (m_sequenceIcon->geometry().contains(event->pos())))
    {
        emit sigWindowImageHover(WINDOW_SEQUENCE_BUTTON, m_windowIndex, true);
        m_toolTip->setVisible (true);

        if(m_closeButtonIcon->isVisible())
        {
            emit sigWindowImageHover(WINDOW_CLOSE_BUTTON, m_windowIndex, false);
        }
    }
    else if((m_openToolbarIcon != NULL)
            && (m_openToolbarIcon->geometry().contains(event->pos())))
    {

        if((m_sequenceIcon != NULL)
                && (m_sequenceIcon->getImageType() == IMAGE_TYPE_MOUSE_HOVER))
        {
            m_toolTip->setVisible (false);
            emit sigWindowImageHover(WINDOW_SEQUENCE_BUTTON, m_windowIndex, false);
        }
    }
    else
    {
        //When mouse is positioned at not elements this else get called
        if(m_decoderTooltip->isVisible())
        {
            m_decoderTooltip->setVisible(false);
        }

        if(m_closeButtonIcon->isVisible())
        {
            emit sigWindowImageHover(WINDOW_CLOSE_BUTTON, m_windowIndex, false);
        }

        if((m_sequenceIcon != NULL)
                && (m_sequenceIcon->getImageType() == IMAGE_TYPE_MOUSE_HOVER))
        {
            m_toolTip->setVisible (false);
            emit sigWindowImageHover(WINDOW_SEQUENCE_BUTTON, m_windowIndex, false);
        }
    }

	if((tCenterWindowIconHoverF) && (m_windowHeaderText->getText() == "") && (m_ActivateCenterWindowIconHoverF == false))
	{
        EPRINT(LAYOUT, "[%d] Add Camera Button Hover not activated [m_cameraId=%d] [m_videoType=%d] [m_videoStatus=%d] [videoError=%d] [currentModeType=%d] [m_windowType=%d]",
                m_windowIndex,
                Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_cameraId,
				Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_videoType,
				Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_videoStatus,
				Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_errorType,
				Layout::currentModeType[MAIN_DISPLAY],
				m_windowType);
	}
	if(m_ActivateCenterWindowIconHoverF == true)
	{
		emit sigWindowImageHover(WINDOW_ADD_CAMERA_BUTTON, m_windowIndex, tCenterWindowIconHoverF);
	}
	QWidget::mouseMoveEvent(event);
}

void LayoutWindow::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(m_closeButtonIcon->isVisible())
    {
        takeCancelKeyAction();
    }
    else
    {
		if((m_centerWindowIcon->getImageSource() ==
			(WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CAMERA_ADD_BUTTON_IMG_SOURCE)))
        {
            emit sigWindowImageClicked(WINDOW_ADD_CAMERA_BUTTON, m_windowIndex);
        }
        else
        {
            emit sigWindowDoubleClicked(m_windowIndex);
        }
    }
}

void LayoutWindow::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeMenuKeyAction();
}

void LayoutWindow::deleteKeyPressed(QKeyEvent *event)
{
    event->accept();
    if((m_windowHeaderText->getText() != "")
            && (isMessageAlertLoaded == false))
    {
        takeCancelKeyAction();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void LayoutWindow::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(m_windowHeaderText->getText() != "")
    {
        takeCancelKeyAction();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

/* This function is called on reset geometry or text change or when the layout changes */
void LayoutWindow::resizeEvent(QResizeEvent * event)
{
	m_borderRect->resetGeometry(0,
                               0,
                               this->width(),
							   this->height());
    m_closeButtonIcon->resetGeometry((this->width() - LAYOUT_WINDOW_LEFT_MARGIN(m_windowIconType)),
                                     LAYOUT_WINDOW_TOP_MARGIN(m_windowIconType));

    m_pbIcon->resetGeometry(m_closeButtonIcon->x(),
                            (m_closeButtonIcon->y() + m_closeButtonIcon->height () + LAYOUT_WINDOW_TOP_MARGIN(m_windowIconType)));

    m_audioOnIcon->resetGeometry(m_pbIcon->x(),
                                 m_pbIcon->y() + m_pbIcon->height () + SCALE_HEIGHT(10));

    m_centerWindowIcon->resetGeometry((event->size().width() / 2),
                                      (event->size().height() / 2));

    m_decoderTooltip->resetGeometry(this->x() + (this->width() / 2),
                                    this->y() + (this->height() / 2) + m_centerWindowIcon->height()/2);

    if(m_windowNumberText != NULL)
    {
        m_windowNumberText->setOffset((this->width() / 2),
                                      (this->height() / 2));
    }

    if(m_sequenceIcon != NULL)
    {
        m_sequenceIcon->resetGeometry((this->width() - LAYOUT_WINDOW_LEFT_MARGIN(m_windowIconType)),
                                      (m_closeButtonIcon->y() + m_closeButtonIcon->height() + SCALE_HEIGHT(10)));

        m_toolTip->resetGeometry ((this->width () - LAYOUT_WINDOW_LEFT_MARGIN(m_windowIconType)),
                                  (m_sequenceIcon->y () + m_sequenceIcon->height ()));

    }

    for(qint8 index = 0; index < MAX_WINDOW_ICON; index++)
    {
        m_windowIcons[index]->resetGeometry(event->size().width(),
                                            event->size().height());
    }

    if(IS_VALID_OBJ(m_apNextTile))
    {
        if(Layout::autoPlayData.autoPlayFeatureDataType[m_arrayIndex].m_isApToolbarVisible == false)
        {
            m_apNextTile->changeColor(TRANSPARENTKEY_COLOR);
            m_apNextTile->resetGeometry(0, 0, 0, 0);
        }
        else
        {
			m_apNextTile->resetGeometry(0,
										0,
										event->size().width(),
										event->size().height());

            if(IS_VALID_OBJ(m_apReloadIcon))
            {
                m_apReloadIcon->resetGeometry((m_apNextTile->width()/ 2),
                                              (m_apNextTile->height()/ 2));
            }
            if(IS_VALID_OBJ(m_apPrevIcon))
            {
                m_apPrevIcon->resetGeometry(((m_apNextTile->width()/ 2) - ((m_apNextTile->width() * apSizeButtonMultiplier[m_apToolbarSize])/
                                                                           apSizeButtonDividend[m_apToolbarSize])),
                                            (m_apNextTile->height()/ 2));
            }
            if(IS_VALID_OBJ(m_apNextIcon))
            {
                m_apNextIcon->resetGeometry(((m_apNextTile->width()/ 2) + ((m_apNextTile->width() * apSizeButtonMultiplier[m_apToolbarSize])/
                                                                           apSizeButtonDividend[m_apToolbarSize])),
                                            (m_apNextTile->height()/ 2));
            }
            if(IS_VALID_OBJ(m_apNextVideoText) && IS_VALID_OBJ(m_apReloadIcon))
            {
                m_apNextVideoText->setOffset((m_apNextTile->width()/2),
                                             (m_apReloadIcon->y() + m_apReloadIcon->height() + SCALE_HEIGHT(apNextVideoTextVerOffset[m_apToolbarSize])));
            }
        }
    }  

    if(m_windowType == WINDOW_TYPE_LAYOUT)
    {
        m_windowHeaderText->setMaxWidth(m_closeButtonIcon->x() - (m_openToolbarIcon->x() + m_openToolbarIcon->width()) - MARGIN);
    }
    else
    {
        m_windowHeaderText->setMaxWidth(this->width() - (this->width() - m_closeButtonIcon->x()) - MARGIN - (SCALE_HEIGHT(BORDER_WIDTH) * 2));
    }

    if(IS_VALID_OBJ(m_windowToolTip) && (event->size().width() == 0) && (event->size().height() == 0))
    {
        m_windowToolTip->setVisible(false);
    }

    updateWindowHeaderOSD();
}

void LayoutWindow::updateImageMouseHover(WINDOW_IMAGE_TYPE_e imageType, bool isHover)
{
    switch(imageType)
    {
    case WINDOW_CLOSE_BUTTON:
        if(!isHover)
        {
            m_closeButtonIcon->setVisible(false);
        }
        else if(m_windowHeaderText->getText() != "")
        {
            m_closeButtonIcon->setVisible(true);
        }
        break;

    case WINDOW_ADD_CAMERA_BUTTON:
		if((m_centerWindowIcon->getImageSource() ==
			(WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CAMERA_ADD_BUTTON_IMG_SOURCE)) ||
			(m_centerWindowIcon->getImageSource() ==
			(WINDOW_ICON_IMG_PATH + m_centerImageSizePath + CENTER_ICON_IMG_SOURCE)))
		{
			QString imageSource = ((isHover == true)
								   ? CAMERA_ADD_BUTTON_IMG_SOURCE
								   : CENTER_ICON_IMG_SOURCE);
			m_centerWindowIcon->updateImageSource(WINDOW_ICON_IMG_PATH + m_centerImageSizePath + imageSource);
		}
		break;

    case WINDOW_SEQUENCE_BUTTON:
        if(m_sequenceIcon != NULL)
        {
            IMAGE_TYPE_e imgType = ((isHover == true)
                                    ? IMAGE_TYPE_MOUSE_HOVER
                                    : IMAGE_TYPE_NORMAL);
            m_sequenceIcon->changeImage(imgType);
        }
        break;

    case WINDOW_OPEN_TOOLBAR_BUTTON:
        if(m_openToolbarIcon != NULL)
        {
            IMAGE_TYPE_e imgType = (Layout::streamInfoArray[MAIN_DISPLAY][m_arrayIndex].m_errorType == VIDEO_ERROR_DISABLECAMERA)
                    ? IMAGE_TYPE_DISABLE : (((isHover == true)
                                             ? IMAGE_TYPE_MOUSE_HOVER
                                             : IMAGE_TYPE_NORMAL));
            m_openToolbarIcon->changeImage(imgType);
        }
        break;

    default:
        break;
    }
}

void LayoutWindow::updateSequenceImageType(WINDOWSEQUENCE_IMAGE_TYPE_e configImageType)
{
    if((m_sequenceIcon != NULL)
            && (m_sequenceImgType != configImageType))
    {
        m_sequenceImgType = configImageType;
        m_sequenceIcon->updateImageSource((m_windowIconPath + sequenceImageSource[m_sequenceImgType]),
                                          true);
    }
}

void LayoutWindow::slotImageClicked(int indexInPage)
{
    if(indexInPage == WINDOW_CLOSE_BUTTON)
    {
        if(m_windowType == WINDOW_TYPE_VIDEO_POP_UP)
        {
           m_mouseClicked = false;
        }

        m_closeButtonIcon->setVisible(false);
        emit sigWindowImageClicked(WINDOW_CLOSE_BUTTON, m_windowIndex);
    }
}

void LayoutWindow::slotMouseHover(int, bool state)
{
    if (m_windowType == WINDOW_TYPE_VIDEO_POP_UP)
    {
        if(m_windowHeaderText->getText() != "")
        {
            m_windowHeaderToolTip->setVisible(state);
        }
    }
    else if((IS_VALID_OBJ(m_windowHeaderText)) &&
            (m_windowHeaderText->getIsTooltipNeeded()) &&
            ((m_windowType == WINDOW_TYPE_LAYOUT) ||
            (m_windowType == WINDOW_TYPE_DISPLAYSETTINGS) ||
            (m_windowType == WINDOW_TYPE_SEQUENCESETTINGS)))
    {
        if(IS_VALID_OBJ(m_windowToolTip))
        {
            if(state == true)
            {
                m_windowToolTip->textChange(m_windowHeaderText->getText());
                m_windowToolTip->setFontSize(NORMAL_FONT_SIZE);
                m_windowToolTip->resetGeometry((parentWidget()->x() + this->x() + m_windowHeaderText->x() + (m_windowHeaderText->width()/2)),
                                               (parentWidget()->y() +  this->y() + m_windowHeaderText->y() + m_windowHeaderText->height() + 5));
                m_windowToolTip->repaint();
            }
            m_windowToolTip->setVisible(state);
            m_windowToolTip->raise();
        }
    }
}

void LayoutWindow::slotAPToolbarBtnClicked(int index)
{
    emit sigAPCenterBtnClicked(index,m_windowIndex);
}

bool LayoutWindow::getApFeatureStatus()
{
    bool status = false;
    if(IS_VALID_OBJ(m_apNextTile))
    {       
        // To show auto play feature, we have set and reset geometry of m_apNextTile object.
        // So to check whether feature is visible or not, we have checked it's height and width.
        status = (((m_apNextTile->width()) != 0) || ((m_apNextTile->height()) != 0));
    }
    return status;
}

void LayoutWindow::updateAPNextVideoText(quint16 windowIndex)
{
    quint8 timerCount = Layout::autoPlayData.autoPlayFeatureDataType[windowIndex].m_timerCount;
    if(IS_VALID_OBJ(m_apNextVideoText))
    {
        m_apNextVideoText->changeText((QString("Next Video") + QString(" : %1s")).arg(MAX_AP_TIMER_COUNT - timerCount));
        m_apNextVideoText->repaint();
    }
}
