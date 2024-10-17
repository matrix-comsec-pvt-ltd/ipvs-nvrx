#include "VolumeControl.h"
#include <QMouseEvent>
#include "ApplicationMode.h"

#define AUDIO_CONTROL_IMAGE_PATH            ":/Images_Nvrx/AudioControl/"
#define SLIDER_WIDTH                        SCALE_WIDTH(56)
#define SLIDER_HEIGHT                       SCALE_HEIGHT(128)
#define BACKGROUND_WIDTH                    SCALE_WIDTH(17)
#define ACTIVE_BAR_WIDTH                    SCALE_WIDTH(5)

#define MAX_VALUE                           100

VolumeControl::VolumeControl(int startX, int startY, int width, int height, QWidget *parent)
    : BackGround(startX, startY, width, height, BACKGROUND_TYPE_3, AUDIO_CONTROL_BUTTON, parent, false)
{
    ApplicationMode::setApplicationMode(PAGE_WITH_TOOLBAR_MODE);
    m_currentElement = VOLUME_SLIDER;
    m_applController = ApplController::getInstance();
    m_mousePressedFlag = false;
    m_chngMuteState = 0;
    m_sliderImageType = 0;
    //borders for background
    m_topBorder = new Rectangle(0, 0,
                                width, 1,
                                BORDER_1_COLOR, this);
    m_rightBorder = new Rectangle((width - SCALE_WIDTH(2)), 0,
                                  1, height,
                                  BORDER_1_COLOR, this);
    m_rightMostBorder = new Rectangle((width - 1), 0,
                                      1, height,
                                      BORDER_2_COLOR, this);

    //headerText says "Volume"
    int fontWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width("Volume");
    m_headingText = new TextLabel((width / 2 - fontWidth / 2),
                                  SCALE_HEIGHT(10),
                                  NORMAL_FONT_SIZE,
                                  "Volume",
                                  this);

    //outer slider area
    m_volumeSliderBackGround = new Rectangle(((width  - BACKGROUND_WIDTH) / 2),
                                             ((height  - SLIDER_HEIGHT) / 2) + SCALE_HEIGHT(5),
                                             BACKGROUND_WIDTH,
                                             SLIDER_HEIGHT + SCALE_HEIGHT(20),
                                             CLICKED_BKG_COLOR,
                                             this,
                                             SCALE_WIDTH(5), 1,
                                             BORDER_2_COLOR);

    //mute checkbox
    m_muteOption = new OptionSelectButton(SCALE_WIDTH(15),
                                          (height - SCALE_HEIGHT(40)),
                                          width,
                                          SCALE_HEIGHT(50),
                                          CHECK_BUTTON_INDEX,
                                          "",
                                          this,
                                          NO_LAYER,
                                          SCALE_WIDTH(10),
                                          MX_OPTION_TEXT_TYPE_SUFFIX,
                                          NORMAL_FONT_SIZE,
                                          MUTE_BUTTON,
                                          true);
    m_elementList[MUTE_BUTTON] = m_muteOption;
    connect(m_muteOption,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotMuteButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_muteOption,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_muteOption->setVisible(false);

    readAudioConfig();

    m_valueMultipler = (float)SLIDER_HEIGHT/(float)MAX_VALUE;

    //slider
    m_volumeSlider = new SliderControl(((width - SLIDER_WIDTH) / 2),
                                       ((height - (SLIDER_HEIGHT + SCALE_HEIGHT(0))) / 2),
                                       SLIDER_WIDTH,
                                       (SLIDER_HEIGHT + SCALE_HEIGHT(36)),
                                       ACTIVE_BAR_WIDTH,
                                       SLIDER_HEIGHT,
                                       QString(AUDIO_CONTROL_IMAGE_PATH),
                                       m_audioLevel,
                                       this,
                                       VERTICAL_SLIDER,
                                       VOLUME_SLIDER,
                                       true,
                                       true,
                                       false,
                                       true);
    m_elementList[VOLUME_SLIDER] = m_volumeSlider;
    connect(m_volumeSlider,
            SIGNAL(sigValueChanged(int,int,bool)),
            this,
            SLOT(slotValueChanged(int,int,bool)));
    connect(m_volumeSlider,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    this->setMouseTracking(true);
    this->setEnabled(true);
    m_elementList[m_currentElement]->forceActiveFocus();
}

VolumeControl::~VolumeControl()
{
    delete m_topBorder;
    delete m_rightBorder;
    delete m_rightMostBorder;
    delete m_headingText;
    delete m_volumeSliderBackGround;

    disconnect(m_muteOption,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_muteOption,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotMuteButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete m_muteOption;

    disconnect(m_volumeSlider,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_volumeSlider,
               SIGNAL(sigValueChanged(int,int,bool)),
               this,
               SLOT(slotValueChanged(int,int,bool)));
    delete m_volumeSlider;
}

void VolumeControl::readAudioConfig()
{
    AUDIO_CONFIG_t audioConfig;
	memset(&audioConfig, 0, sizeof(audioConfig));
    QList<QVariant> paramList;
    paramList.append(READ_AUDIO_ACTIVITY);
    if(m_applController->processActivity(AUDIO_SETTING, paramList, &audioConfig))
    {
        m_valueMultipler = (float)SLIDER_HEIGHT/(float)MAX_VALUE;
        m_audioLevel = audioConfig.level * m_valueMultipler;
        m_currentMuteState = (audioConfig.muteStatus == AUDIO_MUTE ? ON_STATE : OFF_STATE);
        m_muteOption->changeState((OPTION_STATE_TYPE_e)m_currentMuteState);
    }
    paramList.clear();
}

void VolumeControl::writeAudioConfig(int currentState, int audioLevel)
{
    AUDIO_CONFIG_t audioConfig;
    QList<QVariant> paramList;
    paramList.append(WRITE_AUDIO_ACTIVITY);
    audioConfig.level = audioLevel;
    audioConfig.muteStatus = (currentState == OFF_STATE ? AUDIO_UNMUTE : AUDIO_MUTE);
    if(m_applController->processActivity(AUDIO_SETTING, paramList, &audioConfig))
    {
        if(currentState != m_currentMuteState)
        {
            m_currentMuteState = currentState;
            emit sigChangeToolbarButtonState(AUDIO_CONTROL_BUTTON, (STATE_TYPE_e)currentState);
        }
    }
    paramList.clear();
}


void VolumeControl::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_VOLUME_CONTROL_ELEMENT)
                % MAX_VOLUME_CONTROL_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void VolumeControl::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1)
                % MAX_VOLUME_CONTROL_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void VolumeControl::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
        event->accept();
        takeLeftKeyAction();
        break;

    case Qt::Key_Right:
        event->accept();
        takeRightKeyAction();
        break;

//    case Qt::Key_Pause:
//        event->accept();
//        takeMuteKeyAction();
//        break;

    case Qt::Key_PageDown:
        event->accept();
        m_currentElement = VOLUME_SLIDER;
        m_elementList[m_currentElement]->forceActiveFocus();
        m_volumeSlider->takeDownKeyAction();
        break;

    case Qt::Key_PageUp:
        event->accept();
        m_currentElement = VOLUME_SLIDER;
        m_elementList[m_currentElement]->forceActiveFocus();
        m_volumeSlider->takeUpKeyAction();
        break;

//    case Qt::Key_F4:
//    case Qt::Key_Escape:
//        event->accept();
//        emit sigClosePage(AUDIO_CONTROL_BUTTON);
//        break;

    default:
        event->accept();
        break;
    }
}


//F4 for registered
void VolumeControl::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    emit sigClosePage(AUDIO_CONTROL_BUTTON);
}

void VolumeControl::takeMuteKeyAction()
{
    m_currentElement = MUTE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
    m_muteOption->takeEnterKeyAction();
}

void VolumeControl::slotMuteButtonClicked(OPTION_STATE_TYPE_e currentState,
                                          int)
{
    if(currentState != m_currentMuteState)
    {
        writeAudioConfig(currentState, m_audioLevel);
    }
}

void VolumeControl::slotValueChanged(int changedValue, int, bool sliderMove)
{
    Q_UNUSED(sliderMove);
    quint32 tempChangedValue = (quint32)(qCeil((float)changedValue / m_valueMultipler));
    m_audioLevel = tempChangedValue;
    if((m_audioLevel == 0) && (m_currentMuteState == OFF_STATE))
    {
        m_chngMuteState = ON_STATE;
    }
    else if((m_audioLevel > 0) &&  (m_currentMuteState == ON_STATE))
    {
        m_chngMuteState = OFF_STATE;
    }
    else
    {
        m_chngMuteState = m_currentMuteState;
    }
    writeAudioConfig(m_chngMuteState, m_audioLevel);
    m_muteOption->changeState((OPTION_STATE_TYPE_e)m_chngMuteState);
}


void VolumeControl::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
