#include "InstantPlayback.h"
#include "Layout/Layout.h"
#include <QDateTime>
#include <QKeyEvent>

#define INST_PB_TOOLBAR_SLIDER_IMG_PATH      "Slider/"

#define INST_PB_TOOLBAR_MAX_WIDTH                    SCALE_WIDTH(956)
#define INST_PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET        SCALE_HEIGHT(30)

static const quint8 sliderImageWidth[MAX_PB_TOOLBAR_SIZE] =
{
    28, 25, 22, 14, 12
};

static const quint8 imagesHeight[MAX_PB_TOOLBAR_SIZE] =
{
    60, 40, 30, 24, 16
};

static const quint8 sliderHeight[MAX_PB_TOOLBAR_SIZE] =
{
    14, 10, 8, 6, 5
};


static const type_e closeBtnSize[MAX_PB_TOOLBAR_SIZE] =
{
    CLOSE_BTN_TYPE_1,
    CLOSE_BTN_TYPE_3,
    CLOSE_BTN_TYPE_4,
    CLOSE_BTN_TYPE_5,
    CLOSE_BTN_TYPE_5
};

static const QString pbToolTipStings[] = {
    "Play",
    "Reverse Play",
    "Stop",
    "Enable Audio"
};

static const PBTOOLBAR_BTN_IMAGE_e ipbToolButton[] = {

    PB_TOOLBAR_PLAY,
    PB_TOOLBAR_REVERSE_PLAY,
    PB_TOOLBAR_STOP,
    PB_TOOLBAR_MUTE
};

static const quint8 fontSize[] = {20,15,10,7,6};
static const quint8 closeYOffset[] = {17,12,9,7,8};
static const quint8 closeXOffset[] = {15,8,9,7,7};
static const quint8 dateXOffset[] = {10,10,10,10,5};
static const quint8 dateYOffset[] = {5,3,4,5,5};
static const quint8 fontYOffset[] = {30,24,19,15,12};
static const quint8 speedfontSize[] = {18,10,8,7,5};

InstantPlayback::InstantPlayback(quint16 startX,
                                 quint32 startY,
                                 quint16 windowWidth,
                                 quint16 windowHeight,
                                 quint16 windowId,
                                 quint8 toolbarSize,
                                 quint64 startTime,
                                 quint64 endTime,
                                 quint64 currTime,
                                 QWidget *parent) :
    KeyBoard(parent), m_windowId(windowId), toolBarStartX(WINDOW_BORDER_WIDTH),
    m_recStartTime(startTime), m_recEndTime(endTime), m_recCurrTime(currTime),
    m_totalTimeDiff(m_recEndTime - m_recStartTime), m_toolBarSize((PB_TOOLBAR_SIZE_e)toolbarSize)
{
    toolBarStartY = (windowHeight - SCALE_HEIGHT(WINDOW_BORDER_WIDTH) -
                     SCALE_HEIGHT(imagesHeight[m_toolBarSize]) - SCALE_HEIGHT(sliderHeight[m_toolBarSize]));
    toolBarWidth = windowWidth - (2 * SCALE_WIDTH(WINDOW_BORDER_WIDTH));
    if(windowWidth > INST_PB_TOOLBAR_MAX_WIDTH)
    {
        toolBarStartX = (windowWidth - INST_PB_TOOLBAR_MAX_WIDTH) / 2;
        toolBarWidth = INST_PB_TOOLBAR_MAX_WIDTH;
    }

    this->setGeometry(startX + toolBarStartX,
                      startY + toolBarStartY - INST_PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET,
                      toolBarWidth,
                      SCALE_HEIGHT(imagesHeight[m_toolBarSize]) + SCALE_HEIGHT(sliderHeight[m_toolBarSize]) + INST_PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET);

    for(quint8 index = 0; index < MAX_INST_PB_TOOLBAR_ELE_BTN; index++)
    {
        m_buttonState[index] = true;
    }

    createDefaultComponent();

    updateCurrTime (m_recCurrTime);
    m_currElement = PB_TOOLBAR_ELE_PLAY_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
    this->show();
}

InstantPlayback::~InstantPlayback ()
{

    disconnect (slider,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrElement(int)));
    disconnect (slider,
             SIGNAL(sigMouseReleaseOnSlider(int,int)),
             this,
             SLOT(slotPbSliderValueChanged(int,int)));
    disconnect (slider,
             SIGNAL(sigHoverOnSlider(int,int)),
             this,
             SLOT(slotHoverOnSlider(int,int)));
    disconnect (slider,
             SIGNAL(sigHoverInOutOnSlider(bool,int)),
             this,
             SLOT(slotHoverInOutOnSlider(bool,int)));
    delete slider;
    delete sliderRect;

    for(quint8 index = 0; index < MAX_INST_PB_TOOLBAR_ELE_BTN; index++)
    {
        disconnect (toolBarBtn[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpadateCurrElement(int)));

        disconnect (toolBarBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        disconnect (toolBarBtn[index],
                 SIGNAL(sigImageMouseHover(quint8,bool)),
                 this,
                 SLOT(slotImageMouseHover(quint8,bool)));
        delete toolBarBtn[index];
    }

    delete closeBtnBg;

    disconnect (closeBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrElement(int)));
    disconnect (closeBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    delete closeBtn;

    delete m_pbDateTime;

    delete timeTooltip;

}

void InstantPlayback::createDefaultComponent ()
{
    DEV_TABLE_INFO_t devTableInfo;

    applController = ApplController::getInstance ();
    applController->GetDeviceInfo (Layout::streamInfoArray[MAIN_DISPLAY][m_windowId].m_deviceName,
                                   devTableInfo);

    sliderRect = new Rectangle(0,
                               INST_PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET,
                               toolBarWidth,
                               SCALE_HEIGHT(sliderHeight[m_toolBarSize]),
                               0,
                               BORDER_1_COLOR, CLICKED_BKG_COLOR, this, 1);

    m_sliderActiveWidth = (sliderRect->width () - SCALE_WIDTH((sliderImageWidth[m_toolBarSize])/2));

    slider = new SliderControl(sliderRect->x (),
                               sliderRect->y () - SCALE_HEIGHT(10),
                               sliderRect->width (),
                               sliderRect->height () + SCALE_HEIGHT(20),
                               m_sliderActiveWidth,
                               1,
                               QString(PBTOOLBAR_IMAGE_PATH + pbButtonSizeFolder[m_toolBarSize] +
                                       INST_PB_TOOLBAR_SLIDER_IMG_PATH),
                               0,
                               this,
                               HORIZONTAL_SLIDER,
                               0, true, false, true, true, true);
    m_elementList[INST_PB_TOOLBAR_ELE_SLIDER] = slider;
    connect (slider,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrElement(int)));
    connect (slider,
             SIGNAL(sigMouseReleaseOnSlider(int,int)),
             this,
             SLOT(slotPbSliderValueChanged(int,int)));

    connect (slider,
             SIGNAL(sigHoverOnSlider(int,int)),
             this,
             SLOT(slotHoverOnSlider(int,int)));

    connect (slider,
             SIGNAL(sigHoverInOutOnSlider(bool,int)),
             this,
             SLOT(slotHoverInOutOnSlider(bool,int)));

    for(quint8 index = 0; index < MAX_INST_PB_TOOLBAR_ELE_BTN; index++)
    {
        toolBarBtn[index] = new PbToolbarButton(((index == 0) ? sliderRect->x () :
                                                                toolBarBtn[index - 1]->x () + toolBarBtn[index - 1]->width ()),
                                                sliderRect->y () + sliderRect->height (),
                                                m_toolBarSize,
                                                this,
                                                index,
                                                true,
                                                true,
                                                ipbToolButton[index]);

        m_elementList[index] = toolBarBtn[index];

        connect (toolBarBtn[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpadateCurrElement(int)));

        connect (toolBarBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (toolBarBtn[index],
                 SIGNAL(sigImageMouseHover(quint8,bool)),
                 this,
                 SLOT(slotImageMouseHover(quint8,bool)));
    }

    closeBtnBg = new BgTile((toolBarBtn[INST_PB_TOOLBAR_ELE_MUTE_BTN]->x () +
                             toolBarBtn[INST_PB_TOOLBAR_ELE_MUTE_BTN]->width ()),
                            toolBarBtn[INST_PB_TOOLBAR_ELE_MUTE_BTN]->y (),
                            (toolBarWidth -
                             (toolBarBtn[INST_PB_TOOLBAR_ELE_MUTE_BTN]->width () * MAX_INST_PB_TOOLBAR_ELE_BTN)),
                            SCALE_HEIGHT(imagesHeight[m_toolBarSize]), COMMON_LAYER, this);

    closeBtn = new CloseButtton(closeBtnBg->x () + closeBtnBg->width () - SCALE_WIDTH(closeXOffset[m_toolBarSize]),
                                closeBtnBg->y () + SCALE_HEIGHT(closeYOffset[m_toolBarSize]),
                                this, closeBtnSize[m_toolBarSize],
                                INST_PB_TOOLBAR_ELE_CLOSE);

    m_elementList[INST_PB_TOOLBAR_ELE_CLOSE] = closeBtn;
    connect (closeBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrElement(int)));
    connect (closeBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    m_pbDateTime = new TextLabel ( closeBtn->x () - SCALE_WIDTH(dateXOffset[m_toolBarSize]),
                                   closeBtn->y () + SCALE_HEIGHT(dateYOffset[m_toolBarSize]),
                                   SCALE_FONT(fontSize[m_toolBarSize]),
                                   "",
                                   this,
                                   NORMAL_FONT_COLOR,
                                   NORMAL_FONT_FAMILY,
                                   ALIGN_END_X_START_Y);

    timeTooltip = new ToolTip(0, 0, "", this);
    if(m_toolBarSize == PB_TOOLBAR_6x6_8x8)
    {
        timeTooltip->setFontSize(EXTRA_SMALL_SUFFIX_FONT_SIZE);
    }
    timeTooltip->setVisible (false);
}

void InstantPlayback::navigationKeyPressed(QKeyEvent *event)
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

    case Qt::Key_Up:
        event->accept();
        keyHandling(INST_PB_TOOLBAR_ELE_STOP_BTN);
        break;

    case Qt::Key_Down:
        event->accept();
        takeDownKeyAction ();
        break;
    }
}

void InstantPlayback::asciiCharKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        // emit sig to close toolbar
        event->accept();
        emit sigInstantPbToolbarBtnClick(INST_PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
        break;
    }
}

void InstantPlayback::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
        // emit sig to close toolbar
        event->accept();
        emit sigInstantPbToolbarBtnClick(INST_PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
        break;

    }
}

void InstantPlayback::escKeyPressed(QKeyEvent *event)
{
    // emit sig to close toolbar
    event->accept();
    emit sigInstantPbToolbarBtnClick(INST_PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
}

void InstantPlayback::tabKeyPressed(QKeyEvent *event)
{
    // emit sig to close toolbar
    event->accept();
    emit sigInstantPbToolbarBtnClick(INST_PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
}

void InstantPlayback::insertKeyPressed(QKeyEvent *event)
{
    // emit sig to close toolbar
    event->accept();
    emit sigInstantPbToolbarBtnClick(INST_PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
}

void InstantPlayback::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_INST_PB_TOOLBAR_ELEMENTS)
                % MAX_INST_PB_TOOLBAR_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void InstantPlayback::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1)
                % MAX_INST_PB_TOOLBAR_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void InstantPlayback::takeDownKeyAction ()
{
    if(m_buttonState[INST_PB_TOOLBAR_ELE_PLAY_BTN] == false)
    {
        keyHandling (INST_PB_TOOLBAR_ELE_PLAY_BTN);
    }
    else if(m_buttonState[INST_PB_TOOLBAR_ELE_REV_PLAY_BTN] == false)
    {
        keyHandling (INST_PB_TOOLBAR_ELE_REV_PLAY_BTN);
    }
    else
    {
        if(toolBarBtn[INST_PB_TOOLBAR_ELE_REV_PLAY_BTN]->hasFocus ())
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_REV_PLAY_BTN]->takeEnterKeyAction ();
        }
        else
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_PLAY_BTN]->takeEnterKeyAction ();
        }
    }
}

void InstantPlayback::keyHandling(INST_PB_TOOLBAR_ELEMENTS_e index)
{
    m_currElement = index;
    m_elementList[m_currElement]->forceActiveFocus ();
    toolBarBtn[index]->takeEnterKeyAction ();
}

quint16 InstantPlayback::getPbToolbarWinId() const
{
    return m_windowId;
}

bool InstantPlayback::getStateOfButton(INST_PB_TOOLBAR_ELEMENTS_e index) const
{
    return  m_buttonState[index];
}

void InstantPlayback::changeStateOfButton(INST_PB_TOOLBAR_ELEMENTS_e index, bool state, bool tooltipUpdation)
{
    bool tooltipChanged = false;
    m_buttonState[index] = state;
    switch(index)
    {
    case INST_PB_TOOLBAR_ELE_PLAY_BTN:
        if(state == true)
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_PLAY);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == "Pause") && tooltipUpdation)
            {
                timeTooltip->textChange (pbToolTipStings[index]);
                tooltipChanged = true;
            }
        }
        else
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_PAUSE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == pbToolTipStings[index]))
            {
                timeTooltip->textChange ("Pause");
                tooltipChanged = true;
            }
        }
        break;

    case INST_PB_TOOLBAR_ELE_REV_PLAY_BTN:
        if(state == true)
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_REV_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_REVERSE_PLAY);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == "Pause") && tooltipUpdation)
            {
                timeTooltip->textChange (pbToolTipStings[index]);
                tooltipChanged = true;
            }
        }
        else
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_REV_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_PAUSE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == pbToolTipStings[index]))
            {
                timeTooltip->textChange ("Pause");
                tooltipChanged = true;
            }
        }
        break;

    case INST_PB_TOOLBAR_ELE_MUTE_BTN:
        if(state == true)
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_MUTE_BTN]->changeToolbarBtn (PB_TOOLBAR_MUTE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == "Disable Audio"))
            {
                timeTooltip->textChange (pbToolTipStings[index]);
                tooltipChanged = true;
            }
        }
        else
        {
            toolBarBtn[INST_PB_TOOLBAR_ELE_MUTE_BTN]->changeToolbarBtn (PB_TOOLBAR_UNMUTE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == pbToolTipStings[index]))
            {
                timeTooltip->textChange ("Disable Audio");
                tooltipChanged = true;
            }
        }
        break;

    default:
        break;
    }

    if(tooltipChanged == true)
    {
        timeTooltip->resetGeometry (toolBarBtn[index]->x () +
                                    toolBarBtn[index]->width ()/2 + SCALE_WIDTH(10),
                                    SCALE_HEIGHT(15));
    }
}

void InstantPlayback::updateCurrTime(quint64 currTime)
{
    m_recCurrTime = currTime;
    if(m_totalTimeDiff != 0)
    {
        quint64 percent = (((m_recCurrTime - m_recStartTime) *10000) /m_totalTimeDiff);
        slider->changeValue ((m_sliderActiveWidth * percent) / 10000);
    }
    else
    {
        m_pbDateTime->changeText("");
        m_pbDateTime->update();
    }
}

void InstantPlayback::showInstantPbDateTime()
{
    QString pbdateTimeStr = Layout::streamInfoArray[MAIN_DISPLAY][m_windowId].m_pbCurrTimeStr;
    pbdateTimeStr.insert (2, "/");
    pbdateTimeStr.insert (5, "/");
    pbdateTimeStr.insert (10, "\n");
    pbdateTimeStr.insert (13, ":");
    pbdateTimeStr.insert (16, ":");

    m_pbDateTime->changeText(pbdateTimeStr);
    m_pbDateTime->update();
}

void InstantPlayback::slotButtonClick(int index)
{
    emit sigInstantPbToolbarBtnClick(index, m_windowId, m_buttonState[index]);
}

void InstantPlayback::slotPbSliderValueChanged(int currVal ,int)
{
    quint64 percent = ((currVal * 10000)/m_sliderActiveWidth);
    m_recCurrTime = (m_recStartTime + (((m_totalTimeDiff) * percent) / 10000));
    slider->resetFlags ();
    emit sigSliderValueChanged (m_recCurrTime, m_windowId);
}

void InstantPlayback::slotHoverOnSlider(int currVal, int)
{
    QString timeStr;
    QDateTime dateTime;

    quint64 percent = ((currVal * 10000)/m_sliderActiveWidth);
    m_recCurrTime = (m_recStartTime + (((m_totalTimeDiff) * percent) / 10000));

    dateTime = QDateTime::fromMSecsSinceEpoch (m_recCurrTime);
    timeStr = dateTime.toString ("HHmmss");
    timeStr.insert (2, ":");
    timeStr.insert (5, ":");

    timeTooltip->setVisible (true);
    timeTooltip->textChange (timeStr);

    if(percent >= 9200)
    {
        timeTooltip->resetGeometry ((toolBarWidth) - timeTooltip->width () + SCALE_WIDTH(20), SCALE_HEIGHT(15));
    }
    else if(percent <= 500)
    {
        timeTooltip->resetGeometry (SCALE_WIDTH(40),SCALE_HEIGHT(15));
    }
    else
    {
        timeTooltip->resetGeometry (currVal + SCALE_WIDTH(10), SCALE_HEIGHT(15));
    }
}

void InstantPlayback::slotHoverInOutOnSlider (bool hoverState, int)
{
    if(!hoverState)
    {
        timeTooltip->setVisible (hoverState);
    }
}

void InstantPlayback::slotUpadateCurrElement(int index)
{
    m_currElement = index;
}

void InstantPlayback::slotImageMouseHover(quint8 indexInPage, bool isHover)
{
    switch(indexInPage)
    {
    case INST_PB_TOOLBAR_ELE_PLAY_BTN:
        if(m_buttonState[indexInPage] == true)
        {
            timeTooltip->textChange (pbToolTipStings[indexInPage]);
        }
        else
        {
            timeTooltip->textChange ("Pause");
        }
        break;

    case INST_PB_TOOLBAR_ELE_REV_PLAY_BTN:
        if(m_buttonState[indexInPage] == true)
        {
            timeTooltip->textChange (pbToolTipStings[indexInPage]);
        }
        else
        {
            timeTooltip->textChange ("Pause");
        }
        break;

    case INST_PB_TOOLBAR_ELE_MUTE_BTN:
        if(m_buttonState[indexInPage] == true)
        {
            timeTooltip->textChange (pbToolTipStings[indexInPage]);
        }
        else
        {
            timeTooltip->textChange ("Disable Audio");
        }
        break;

    default:
        timeTooltip->textChange (pbToolTipStings[indexInPage]);
        break;
    }

    if(isHover)
    {
        timeTooltip->setVisible (true);
        timeTooltip->resetGeometry (toolBarBtn[indexInPage]->x () +
                                    toolBarBtn[indexInPage]->width ()/2 + SCALE_WIDTH(10),
                                    SCALE_HEIGHT(15));
    }
    else
        timeTooltip->setVisible (false);
}
