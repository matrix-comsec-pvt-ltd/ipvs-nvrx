#include "WindowSequenceSettings.h"
#include "Layout/Layout.h"
#include "MessageBanner.h"
#include <QPainter>
#include <QKeyEvent>
#include "ValidationMessage.h"
#include <DisplaySetting.h>

#define LAYOUT_CREATOR_WIDTH    SCALE_WIDTH(780)
#define LAYOUT_CREATOR_HEIGHT   SCALE_HEIGHT(720)
#define PAGE_LEFT_MARGIN        SCALE_WIDTH(20)
#define PAGE_TOP_MARGIN         SCALE_HEIGHT(50)
#define INTER_CONTROL_MARGIN    10
#define CAMLIST_BGTILE_WIDTH    SCALE_WIDTH(298)
#define BACKGROUND_WIDTH        (LAYOUT_CREATOR_WIDTH + CAMLIST_BGTILE_WIDTH + (2 * PAGE_LEFT_MARGIN) + SCALE_WIDTH(INTER_CONTROL_MARGIN))
#define BACKGROUND_HEIGHT       (LAYOUT_CREATOR_HEIGHT + (2 * PAGE_TOP_MARGIN) + SCALE_HEIGHT(INTER_CONTROL_MARGIN))

WindowSequenceSettings::WindowSequenceSettings(quint16 windowIndex,
                                               DISPLAY_CONFIG_t *displayConfig,
                                               bool &isChangeDone, QWidget *parent)
    : KeyBoard(parent), m_windowIndex(windowIndex)
{
    m_displayConfig = displayConfig;
    m_isChangeDone = &isChangeDone;
    m_selectedChannel = displayConfig->windowInfo[m_windowIndex].currentChannel;
    memcpy(&m_currentDisplayConfig,
           displayConfig,
           sizeof(DISPLAY_CONFIG_t));
    applController = ApplController::getInstance();
    quint16 width = parent->width();
    quint16 height = parent->height();
    quint16 leftMargin = ((width - BACKGROUND_WIDTH) / 2);
    quint16 topMargin = ((((height - SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT))
                           - BACKGROUND_HEIGHT) / 2)
                         + SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT));

    this->setGeometry(0, 0, width, height);

    m_background = new BackGround(leftMargin,
                                  topMargin,
                                  BACKGROUND_WIDTH,
                                  BACKGROUND_HEIGHT,
                                  BACKGROUND_TYPE_4,
                                  MAX_TOOLBAR_BUTTON,
                                  this, true,
                                  QString("Window %1").arg(windowIndex + 1));
    connect(m_background,
            SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));

    m_closeButton = m_background->getCloseButton();
    m_elementList[WINDOWSEQUENCE_STG_CLOSE_BUTTON] = m_closeButton;
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    m_layoutCreator = new LayoutCreator((m_background->x() + PAGE_LEFT_MARGIN),
                                        (m_background->y() + PAGE_TOP_MARGIN),
                                        LAYOUT_CREATOR_WIDTH,
                                        LAYOUT_CREATOR_HEIGHT,
                                        this,
                                        WINDOWSEQUENCE_STG_LAYOUTCREATOR,
                                        WINDOW_TYPE_SEQUENCESETTINGS,
                                        true,
                                        MAX_WINDOWS);
    m_elementList[WINDOWSEQUENCE_STG_LAYOUTCREATOR] = m_layoutCreator;
    connect(m_layoutCreator,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_layoutCreator,
            SIGNAL(sigWindowSelected(quint16)),
            this,
            SLOT(slotChannelSelected(quint16)));
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

    m_cameraListBgTile = new BgTile((m_layoutCreator->x() + m_layoutCreator->width() + SCALE_WIDTH(INTER_CONTROL_MARGIN)),
                                    m_layoutCreator->y(),
                                    CAMLIST_BGTILE_WIDTH,
                                    LAYOUT_CREATOR_HEIGHT,
                                    COMMON_LAYER,
                                    this);
    m_seqCameraList = new CameraList((m_cameraListBgTile->x() + SCALE_WIDTH(INTER_CONTROL_MARGIN)),
                                            (m_cameraListBgTile->y() + SCALE_HEIGHT(INTER_CONTROL_MARGIN)),
                                            this,
                                            WINDOWSEQUENCE_STG_CAMERLIST,
                                             CALLED_BY_WINDOWSEQ_SETTING);
    m_elementList[WINDOWSEQUENCE_STG_CAMERLIST] = m_seqCameraList;
    connect(m_seqCameraList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_seqCameraList,
            SIGNAL(sigCameraButtonClickedWinSeq(quint8,QString,CAMERA_STATE_TYPE_e,CAMERA_STATE_TYPE_e)),
            this,
            SLOT(slotCameraButtonClicked(quint8,QString,CAMERA_STATE_TYPE_e,CAMERA_STATE_TYPE_e)));



    m_sequenceCheckbox = new OptionSelectButton(m_layoutCreator->x(),
                                                (m_layoutCreator->y() + m_layoutCreator->height() + SCALE_HEIGHT(INTER_CONTROL_MARGIN)),
                                                0, BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                "Sequence",
                                                this, NO_LAYER,
                                                0, MX_OPTION_TEXT_TYPE_SUFFIX,
                                                NORMAL_FONT_SIZE,
                                                WINDOWSEQUENCE_STG_CHECKBOX);
    m_elementList[WINDOWSEQUENCE_STG_CHECKBOX] = m_sequenceCheckbox;
    connect(m_sequenceCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));


    m_sequenceIntervalList.clear();
    m_sequenceIntervalList.reserve (251);
    for(quint16 index = 5; index <= 255; index++)
    {
        m_sequenceIntervalList.append (QString("%1").arg (index));
    }

    m_sequenceIntervalSpinbox = new SpinBox((m_sequenceCheckbox->x() + m_sequenceCheckbox->width() + SCALE_WIDTH(INTER_CONTROL_MARGIN)),
                                            m_sequenceCheckbox->y(),
                                            0, BGTILE_HEIGHT,
                                            WINDOWSEQUENCE_STG_SPINBOX,
                                            SPINBOX_SIZE_78,
                                            "",
                                            m_sequenceIntervalList,
                                            this,
                                            "(5-255) sec",
                                            false, 0, NO_LAYER);
    m_elementList[WINDOWSEQUENCE_STG_SPINBOX] = m_sequenceIntervalSpinbox;
    connect(m_sequenceIntervalSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    m_saveButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  (m_sequenceIntervalSpinbox->x() + m_sequenceIntervalSpinbox->width() + SCALE_WIDTH(610)),
                                  (m_sequenceIntervalSpinbox->y() + (m_sequenceIntervalSpinbox->height() / 2)),
                                  "Save",
                                  this,
                                  WINDOWSEQUENCE_STG_SAVEBUTTON);
    m_elementList[WINDOWSEQUENCE_STG_SAVEBUTTON] = m_saveButton;
    connect(m_saveButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_saveButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));

    m_cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                    (m_saveButton->x() + m_saveButton->width() + SCALE_WIDTH(60)),
                                    (m_saveButton->y() + (m_saveButton->height() / 2)),
                                    "Cancel",
                                    this,
                                    WINDOWSEQUENCE_STG_CANCELBUTTON);
    m_elementList[WINDOWSEQUENCE_STG_CANCELBUTTON] = m_cancelButton;
    connect(m_cancelButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect(m_cancelButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));

    m_layoutCreator->setLayoutStyle(FOUR_X_FOUR, m_selectedChannel);
    m_sequenceIntervalSpinbox->setIndexofCurrElement((displayConfig->windowInfo[m_windowIndex].sequenceInterval - 5));
    m_sequenceCheckbox->changeState(displayConfig->windowInfo[m_windowIndex].sequenceStatus
                                    ? ON_STATE : OFF_STATE);
    m_seqCameraList->setCurrentDisplayConfig(m_currentDisplayConfig);

    displayLayoutcreatorForCurrentConfig();

    m_currentElement = WINDOWSEQUENCE_STG_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();

    this->show();
}

WindowSequenceSettings::~WindowSequenceSettings()
{
    disconnect(m_closeButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));

    disconnect(m_background,
               SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
               this,
               SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
    delete m_background;

    disconnect(m_layoutCreator,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_layoutCreator,
               SIGNAL(sigWindowSelected(quint16)),
               this,
               SLOT(slotChannelSelected(quint16)));
    disconnect(m_layoutCreator,
               SIGNAL(sigSwapWindows(quint16,quint16)),
               this,
               SLOT(slotSwapWindows(quint16,quint16)));
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

    delete m_cameraListBgTile;

    disconnect(m_seqCameraList,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_seqCameraList,
               SIGNAL(sigCameraButtonClickedWinSeq(quint8,QString,CAMERA_STATE_TYPE_e,CAMERA_STATE_TYPE_e)),
               this,
               SLOT(slotCameraButtonClicked(quint8,QString,CAMERA_STATE_TYPE_e,CAMERA_STATE_TYPE_e)));
    delete m_seqCameraList;

    disconnect(m_sequenceCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_sequenceCheckbox;

    disconnect(m_sequenceIntervalSpinbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_sequenceIntervalSpinbox;

    disconnect(m_saveButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_saveButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    delete m_saveButton;

    disconnect(m_cancelButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect(m_cancelButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    delete m_cancelButton;
}

void WindowSequenceSettings::saveConfig()
{
    quint8 channelIndex;

    if(firstAvailableChannel(channelIndex) == true)
    {
        m_currentDisplayConfig.windowInfo[m_windowIndex].currentChannel = channelIndex;
    }

    m_currentDisplayConfig.windowInfo[m_windowIndex].sequenceInterval = (m_sequenceIntervalSpinbox->getIndexofCurrElement() + 5);
    m_currentDisplayConfig.windowInfo[m_windowIndex].sequenceStatus = (m_sequenceCheckbox->getCurrentState() == ON_STATE);
    DisplaySetting::pageSequenceStatus = m_currentDisplayConfig.windowInfo[m_windowIndex].sequenceStatus;

    memcpy(m_displayConfig,
           &m_currentDisplayConfig,
           sizeof(DISPLAY_CONFIG_t));

    MessageBanner::addMessageInBanner (Multilang("Window wise sequencing configured successfully for Window") + QString(" ")
                                       + QString ("%1").arg(m_windowIndex + 1) + ".");
}

void WindowSequenceSettings::displayLayoutcreatorForCurrentConfig()
{
    QString stringToDisplay,tempDeviceName;
    quint8 tempChannel;

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        tempDeviceName = QString(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName);
        stringToDisplay = "";

        if(tempDeviceName != "")
        {
            tempChannel = m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel;
            stringToDisplay = applController->GetDispDeviceName(tempDeviceName);
            stringToDisplay.append("\n");
            stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
            stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
        }

        m_layoutCreator->setWinHeaderForDispSetting(channelIndex, stringToDisplay);
    }
}

bool WindowSequenceSettings::findWindowIndexOfDisplayInfo(quint8 cameraIndex,
                                                          QString deviceName,
                                                          quint16 &windowIndex,
                                                          quint8 &channelIndex)
{
    bool status = false;

    for(windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if((strcmp(deviceName.toUtf8().constData(), m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName) == 0)
                    && (cameraIndex == m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel))
            {
                return true;
            }
        }
    }

    return status;
}

bool WindowSequenceSettings::findFreeChannelIndex(quint8 &channelIndex)
{
    bool status = false;

    for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        if((strcmp(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName, "") == 0)
                && (m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel == INVALID_CAMERA_INDEX))
        {
            return true;
        }
    }

    return status;
}

bool WindowSequenceSettings::firstAvailableChannel(quint8 &channelIndex)
{
    bool status = false;

    for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        if((strcmp(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName, "") != 0)
                && (m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
        {
            return true;
        }
    }

    return status;
}

bool WindowSequenceSettings::isMultipleChannelAssigned(quint16 windowIndex)
{
    bool status = false;
    quint8 assignCount = 0;

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        if((strcmp(m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName, "") != 0)
                && (m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
        {
            assignCount++;
        }

        if(assignCount >= 2)
        {
            status = true;
            break;
        }
    }

    return status;
}

void WindowSequenceSettings::windowCloseButtonClicked(quint8 channelIndex)
{
    if(channelIndex < MAX_WIN_SEQ_CAM)
    {
        *m_isChangeDone = true;
        m_selectedChannel = channelIndex;
        m_layoutCreator->changeSelectedWindow(channelIndex, true);
        m_layoutCreator->setWinHeaderForDispSetting(channelIndex, "");

        snprintf(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s","");
        m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;

        if(isMultipleChannelAssigned(m_windowIndex) == false)
        {
            m_currentDisplayConfig.windowInfo[m_windowIndex].sequenceStatus = false;
            m_sequenceCheckbox->changeState(OFF_STATE);
        }

        m_seqCameraList->setCurrentDisplayConfig(m_currentDisplayConfig);
    }
}

void WindowSequenceSettings::updateDeviceState(QString deviceName, DEVICE_STATE_TYPE_e devState)
{
    m_seqCameraList->updateDeviceCurrentState (deviceName,devState);

    if((devState == DELETED) || (devState == CONFLICT))
    {
        for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
        {
            for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
            {
                if(strcmp(deviceName.toUtf8().constData(), m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName) == 0)
                {
                    snprintf(m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s", "");
                    m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;

                    if(windowIndex == m_windowIndex)
                    {
                        m_layoutCreator->setWinHeaderForDispSetting(channelIndex, "");
                        if(isMultipleChannelAssigned(m_windowIndex) == false)
                        {
                            m_currentDisplayConfig.windowInfo[m_windowIndex].sequenceStatus = false;
                            m_sequenceCheckbox->changeState(OFF_STATE);
                        }
                    }
                }
            }
        }
    }
    else if ((devState == CONNECTED) && (deviceName == LOCAL_DEVICE_NAME))
    {
        quint8  tempChannelId;
        QString stringToDisplay;
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if(strcmp(deviceName.toUtf8().constData(), m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName) != 0)
            {
                continue;
            }

            stringToDisplay = applController->GetDispDeviceName(deviceName);
            tempChannelId = m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel;
            stringToDisplay.append("\n");
            stringToDisplay.append(INT_TO_QSTRING(tempChannelId) + ":" );
            stringToDisplay.append(Layout::getCameraNameOfDevice(deviceName, (tempChannelId - 1)));
            m_layoutCreator->setWinHeaderForDispSetting(channelIndex, stringToDisplay);
        }
    }
    m_seqCameraList->setCurrentDisplayConfig(m_currentDisplayConfig);
}

void WindowSequenceSettings::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_WINDOWSEQUENCE_STG_ELEMENTS)
                % MAX_WINDOWSEQUENCE_STG_ELEMENTS;
    }while((m_elementList[m_currentElement] == NULL) ||
           (!m_elementList[m_currentElement]->getIsEnabled()));

    switch(m_currentElement)
    {
    case WINDOWSEQUENCE_STG_LAYOUTCREATOR:
    case WINDOWSEQUENCE_STG_CAMERLIST:
        m_elementList[m_currentElement]->forceFocusToPage(true);
        break;

    default:
        m_elementList[m_currentElement]->forceActiveFocus();
        break;
    }
}

void WindowSequenceSettings::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1)
                % MAX_WINDOWSEQUENCE_STG_ELEMENTS;
    }while((m_elementList[m_currentElement] == NULL) ||
                (!m_elementList[m_currentElement]->getIsEnabled()));

    switch(m_currentElement)
    {
    case WINDOWSEQUENCE_STG_LAYOUTCREATOR:
    case WINDOWSEQUENCE_STG_CAMERLIST:
        m_elementList[m_currentElement]->forceFocusToPage(true);
        break;

    default:
        m_elementList[m_currentElement]->forceActiveFocus();
        break;
    }
}

void WindowSequenceSettings::paintEvent(QPaintEvent *)
{
    QColor color;
    color.setAlpha(150);

    QPainter painter(this);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(0, 0,
                            SCALE_WIDTH(DISP_SETTING_PAGE_HEADING_WIDTH),
                            SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                            SCALE_WIDTH(RECT_RADIUS),
                            SCALE_HEIGHT(RECT_RADIUS));
    painter.drawRoundedRect(0,
                            SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                            SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH),
                            (SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT) - SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT)),
                            SCALE_WIDTH(RECT_RADIUS),
                            SCALE_HEIGHT(RECT_RADIUS));
}

void WindowSequenceSettings::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void WindowSequenceSettings::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = WINDOWSEQUENCE_STG_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void WindowSequenceSettings::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void WindowSequenceSettings::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void WindowSequenceSettings::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_elementList[m_currentElement]->forceActiveFocus();
}

void WindowSequenceSettings::slotUpadateCurrentElement(int index)
{
    m_currentElement = index;
}

void WindowSequenceSettings::slotConfigButtonClicked(int index)
{
    if(index == WINDOWSEQUENCE_STG_SAVEBUTTON)
    {
        saveConfig();
    }

    emit sigObjectDelete();
}

void WindowSequenceSettings::slotClosePage(TOOLBAR_BUTTON_TYPE_e)
{
    emit sigObjectDelete();
}

void WindowSequenceSettings::slotCameraButtonClicked(quint8 cameraIndex,
                                                     QString deviceName,
                                                     CAMERA_STATE_TYPE_e connectionState,
                                                     CAMERA_STATE_TYPE_e clearConnection)
{
    bool status = false;
    quint16 windowIndex;
    quint8 channelIndex;
    bool isCamAssigned = (connectionState == CAM_STATE_NONE);
    QString stringToDisplay = "";

    *m_isChangeDone = true;
    switch(connectionState)
    {
    case CAM_STATE_NONE:
        status = findFreeChannelIndex(channelIndex);
        if(status == true)
        {
            windowIndex = m_windowIndex;
            if(channelIndex < MAX_WIN_SEQ_CAM)
            {
                snprintf(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s", deviceName.toUtf8().constData());
                m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel = cameraIndex;
            }
            stringToDisplay = applController->GetDispDeviceName(deviceName);
            stringToDisplay.append("\n");
            stringToDisplay.append(INT_TO_QSTRING(cameraIndex) + ":" );
            stringToDisplay.append(Layout::getCameraNameOfDevice(deviceName, (cameraIndex - 1)));
        }
        else
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(NO_FREE_CHNL_AVAILABLE));
        }
        break;

    case CAM_STATE_ASSIGNED:
        status = findWindowIndexOfDisplayInfo(cameraIndex, deviceName, windowIndex, channelIndex);
        if(channelIndex >= MAX_WIN_SEQ_CAM)
        {
            status = false;
        }

        if(status == true)
        {
            quint8 temCamIndex;

            status = findFreeChannelIndex(temCamIndex);
            if((status == true) || (windowIndex == m_windowIndex))
            {
                snprintf(m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s", "");
                m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;

                if(windowIndex != m_windowIndex)
                {
                    stringToDisplay = applController->GetDispDeviceName(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName);
                    if(stringToDisplay != "")
                    {
                        quint8 tempChannelId = m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel;
                        stringToDisplay.append("\n");                      
                        stringToDisplay.append(INT_TO_QSTRING(tempChannelId) + ":" );
                        stringToDisplay.append(Layout::getCameraNameOfDevice(deviceName, (tempChannelId - 1)));
                    }
                }

                if(isMultipleChannelAssigned(windowIndex) == false)
                {
                    m_currentDisplayConfig.windowInfo[windowIndex].sequenceStatus = false;
                    m_currentDisplayConfig.windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;

                    if(windowIndex == m_windowIndex)
                    {
                        m_sequenceCheckbox->changeState(OFF_STATE);
                    }
                }

                isCamAssigned = false;
                status = true;
            }
            else
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(NO_FREE_CHNL_AVAILABLE));
                isCamAssigned = false;
            }
        }

        if(((windowIndex != m_windowIndex) && ((clearConnection == MAX_CAMERA_STATE) || (clearConnection == CAM_STATE_NONE))))
        {
            status = findFreeChannelIndex(channelIndex);
            if(status == true)
            {
                windowIndex = m_windowIndex;
                snprintf(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s", deviceName.toUtf8().constData());
                m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[channelIndex].defChannel = cameraIndex;

                stringToDisplay = applController->GetDispDeviceName(deviceName);
                stringToDisplay.append("\n");               
                stringToDisplay.append(INT_TO_QSTRING(cameraIndex) + ":" );
                stringToDisplay.append(Layout::getCameraNameOfDevice(deviceName, (cameraIndex - 1)));
                isCamAssigned = true;
            }
            else
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(NO_FREE_CHNL_AVAILABLE));
                isCamAssigned = false;
            }
        }
        break;

    default:
        break;
    }

    if(status == true)
    {
        m_seqCameraList->setCurrentDisplayConfig(m_currentDisplayConfig);

        if(isCamAssigned == true)
        {
            if(m_selectedChannel != channelIndex)
            {
                m_selectedChannel = channelIndex;
                m_layoutCreator->changeSelectedWindow(m_selectedChannel);
            }

            m_layoutCreator->setWinHeaderForDispSetting(channelIndex, stringToDisplay);
        }
        else if(windowIndex == m_windowIndex)
        {
            m_layoutCreator->setWinHeaderForDispSetting(channelIndex, stringToDisplay);
        }
    }
}

void WindowSequenceSettings::slotChannelSelected(quint16 index)
{
    m_selectedChannel = (quint8)index;
    m_layoutCreator->changeSelectedWindow(index, true);
}

void WindowSequenceSettings::slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType,
                                                    quint16 windowIndex)
{
    *m_isChangeDone = true;
    if(imageType == WINDOW_CLOSE_BUTTON)
    {
        windowCloseButtonClicked((quint8)windowIndex);
    }
}

void WindowSequenceSettings::slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType,
                                                  quint16 windowIndex,
                                                  bool isHover)
{
    if(imageType == WINDOW_CLOSE_BUTTON)
    {
        m_layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
    }
}

void WindowSequenceSettings::slotSwapWindows(quint16 firstWindow,
                                             quint16 secondWindow)
{
    *m_isChangeDone = true;
    m_selectedChannel =  (quint8)secondWindow;
    m_layoutCreator->changeSelectedWindow(secondWindow, true);

    char tempDeviceName[MAX_DEVICE_NAME_SIZE];
    quint8 tempChannel = 0;
    QString stringToDisplay;

    snprintf(tempDeviceName,MAX_DEVICE_NAME_SIZE,"%s", m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[firstWindow].deviceName);
    tempChannel = m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[firstWindow].defChannel;

    snprintf(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[firstWindow].deviceName,MAX_DEVICE_NAME_SIZE,"%s",
           m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[secondWindow].deviceName);
    m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[firstWindow].defChannel =
            m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[secondWindow].defChannel;

    snprintf(m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[secondWindow].deviceName,MAX_DEVICE_NAME_SIZE,"%s", tempDeviceName);
    m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[secondWindow].defChannel = tempChannel;

    snprintf(tempDeviceName,MAX_DEVICE_NAME_SIZE,"%s", m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[firstWindow].deviceName);
    if(strcmp(tempDeviceName, "") != 0)
    {
        tempChannel = m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[firstWindow].defChannel;
        stringToDisplay = applController->GetDispDeviceName(tempDeviceName);
        stringToDisplay.append("\n");
        stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
        stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
    }
    else
    {
        stringToDisplay = "";
    }
    m_layoutCreator->setWinHeaderForDispSetting(firstWindow, stringToDisplay);

    snprintf(tempDeviceName,MAX_DEVICE_NAME_SIZE,"%s", m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[secondWindow].deviceName);
    if(strcmp(tempDeviceName, "") != 0)
    {
        tempChannel = m_currentDisplayConfig.windowInfo[m_windowIndex].camInfo[secondWindow].defChannel;
        stringToDisplay = applController->GetDispDeviceName(tempDeviceName);
        stringToDisplay.append("\n");
        stringToDisplay.append(INT_TO_QSTRING(tempChannel) + ":" );
        stringToDisplay.append(Layout::getCameraNameOfDevice(tempDeviceName, (tempChannel - 1)));
    }
    else
    {
        stringToDisplay = "";
    }
    m_layoutCreator->setWinHeaderForDispSetting(secondWindow, stringToDisplay);

    if((QApplication::overrideCursor() != NULL) && (QApplication::overrideCursor()->shape() == Qt::OpenHandCursor))
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }

    m_seqCameraList->setCurrentDisplayConfig(m_currentDisplayConfig);
}

void WindowSequenceSettings::slotDragStartStopEvent(bool isStart)
{
    if(isStart)
    {
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
    }
    else
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
}
