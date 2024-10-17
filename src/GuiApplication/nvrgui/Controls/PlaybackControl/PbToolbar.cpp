#include "PbToolbar.h"
#include "Layout/Layout.h"
#include <QDateTime>
#include <QKeyEvent>

#define PB_TOOLBAR_SLIDER_IMG_PATH      "Slider/"

#define PB_TOOLBAR_MAX_WIDTH                    SCALE_WIDTH(956)
#define PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET        SCALE_HEIGHT(30)

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
    "Stop",
    "Reverse Play",
    "Slow",
    "Fast",
    "Previous Frame",
    "Next Frame",
    "Enable Audio"
};

static const QString pbSpeedStr[2][9] =
{
    {"[X/16]","[X/8]", "[X/4]",  "[X/2]", "",  "[2X]",  "[4X]",  "[8X]", "[16X]"},
    {"[-X/16]","[-X/8]","[-X/4]", "[-X/2]", "", "[-2X]", "[-4X]", "[-8X]", "[-16X]"}
};

static const quint8 fontSize[] = {20,15,10,7,6};
static const quint8 closeYOffset[] = {17,12,9,7,8};
static const quint8 closeXOffset[] = {15,8,9,7,7};
static const quint8 dateXOffset[] = {10,10,10,10,5};
static const quint8 dateYOffset[] = {5,3,2,2,2};
static const quint8 fontYOffset[] = {30,24,19,17,12};
static const quint8 speedfontSize[] = {18,10,8,7,5};
static const quint8 speedXOffset[] = {3,3,3,3,15};

PbToolbar::PbToolbar(int startX, int startY,
                     int windowWidth, int windowHeight,
                     quint16 windowId, quint8 toolbarSize, quint64 startTime,
                     quint64 endTime, quint64 currTime,
                     QWidget *parent)
    :KeyBoard(parent), m_windowId(windowId),
     m_recStartTime(startTime), m_recEndTime(endTime), m_recCurrTime(currTime),
     m_toolBarSize((PB_TOOLBAR_SIZE_e)toolbarSize)
{
    m_totalTimeDiff = m_recEndTime - m_recStartTime;

    toolBarStartX = SCALE_WIDTH(WINDOW_BORDER_WIDTH);
    toolBarStartY = (windowHeight - SCALE_WIDTH(WINDOW_BORDER_WIDTH) -
                     SCALE_HEIGHT(imagesHeight[m_toolBarSize]) - SCALE_HEIGHT(sliderHeight[m_toolBarSize]));
    toolBarWidth = windowWidth - (2 * SCALE_WIDTH(WINDOW_BORDER_WIDTH));
    if(windowWidth > PB_TOOLBAR_MAX_WIDTH)
    {
        toolBarStartX = (windowWidth - PB_TOOLBAR_MAX_WIDTH) / 2;
        toolBarWidth = PB_TOOLBAR_MAX_WIDTH;
    }

    this->setGeometry(startX + toolBarStartX,
                      startY + toolBarStartY - PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET,
                      toolBarWidth,
                      SCALE_HEIGHT(imagesHeight[m_toolBarSize]) + SCALE_HEIGHT(sliderHeight[m_toolBarSize]) + PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET);

    for(quint8 index = 0; index < MAX_PB_TOOLBAR_ELE_BTN; index++)
    {
        m_buttonState[index] = true;
    }

    for(quint8 index = 0; index < MAX_PB_TOOLBAR_ELEMENTS; index++)
    {
        m_elementList[index] = NULL;
    }

    createDefaultComponent();

    updateCurrTime (m_recCurrTime);
    m_currElement = PB_TOOLBAR_ELE_PLAY_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
    this->show();
}

PbToolbar::~PbToolbar()
{
    delete sliderRect;

    disconnect (slider,
                SIGNAL(sigHoverInOutOnSlider(bool,int)),
                this,
                SLOT(slotHoverInOutOnSlider(bool,int)));
    disconnect (slider,
                SIGNAL(sigHoverOnSlider(int,int)),
                this,
                SLOT(slotHoverOnSlider(int,int)));
    disconnect (slider,
                SIGNAL(sigMouseReleaseOnSlider(int,int)),
                this,
                SLOT(slotPbSliderValueChanged(int,int)));
    disconnect (slider,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete slider;

    for(quint8 index = 0; index < MAX_PB_TOOLBAR_ELE_BTN; index++)
    {
        disconnect (toolBarBtn[index],
                    SIGNAL(sigImageMouseHover(quint8,bool)),
                    this,
                    SLOT(slotImageMouseHover(quint8,bool)));
        disconnect (toolBarBtn[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotPbBtnButtonClick(int)));
        disconnect (toolBarBtn[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpadateCurrentElement(int)));
        delete toolBarBtn[index];
    }

    delete closeBtnBg;

    disconnect (closeBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPbBtnButtonClick(int)));
    disconnect (closeBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete closeBtn;
    delete m_pbSpeed;
    delete m_pbDateTime;

    delete timeTooltip;
}

void PbToolbar::createDefaultComponent()
{
    DEV_TABLE_INFO_t devTableInfo;

    applController = ApplController::getInstance ();
    applController->GetDeviceInfo (Layout::streamInfoArray[MAIN_DISPLAY][m_windowId].m_deviceName,
                                   devTableInfo);

    sliderRect = new Rectangle(0,
                               PB_TOOLBAR_TOOLTIP_HEIGHT_OFFSET,
                               toolBarWidth,
                               SCALE_HEIGHT(sliderHeight[m_toolBarSize]),
                               0,
                               BORDER_1_COLOR, CLICKED_BKG_COLOR, this, 1);

    m_sliderActiveWidth = (sliderRect->width () - (SCALE_WIDTH(sliderImageWidth[m_toolBarSize])/2));

    slider = new SliderControl(sliderRect->x (),
                               sliderRect->y () - SCALE_WIDTH(10),
                               sliderRect->width (),
                               sliderRect->height () + SCALE_HEIGHT(20),
                               m_sliderActiveWidth,
                               1,
                               QString(PBTOOLBAR_IMAGE_PATH + pbButtonSizeFolder[m_toolBarSize] + PB_TOOLBAR_SLIDER_IMG_PATH),
                               0,
                               this,
                               HORIZONTAL_SLIDER,
                               0, true, false, true, true, true);
    m_elementList[PB_TOOLBAR_ELE_SLIDER] = slider;
    connect (slider,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
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

    for(quint8 index = 0; index < MAX_PB_TOOLBAR_ELE_BTN; index++)
    {
        toolBarBtn[index] = new PbToolbarButton(((index == 0)?sliderRect->x () :
                                                              toolBarBtn[index - 1]->x () + toolBarBtn[index - 1]->width ()),
                                                sliderRect->y () + sliderRect->height (),
                                                m_toolBarSize,
                                                this, index,
                                                true,
                                                true,
                                                index);

        m_elementList[index] = toolBarBtn[index];
        connect (toolBarBtn[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpadateCurrentElement(int)));

        connect (toolBarBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotPbBtnButtonClick(int)));
        connect (toolBarBtn[index],
                 SIGNAL(sigImageMouseHover(quint8,bool)),
                 this,
                 SLOT(slotImageMouseHover(quint8,bool)));
    }

    closeBtnBg = new BgTile(toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->x () + toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->width (),
                            toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->y (),
                            (toolBarWidth - (toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->width () * MAX_PB_TOOLBAR_ELE_BTN)),
                            SCALE_HEIGHT(imagesHeight[m_toolBarSize]), COMMON_LAYER, this);

    m_pbDateTime = new TextLabel (toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->x () + toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->width () + SCALE_WIDTH(dateXOffset[m_toolBarSize]),
                                  toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->y () + SCALE_HEIGHT(dateYOffset[m_toolBarSize]),
                                  SCALE_FONT(fontSize[m_toolBarSize]),
                                  "",
                                  this,
                                  NORMAL_FONT_COLOR);


    closeBtn = new CloseButtton(closeBtnBg->x () + closeBtnBg->width () - SCALE_WIDTH(closeXOffset[m_toolBarSize]),
                                closeBtnBg->y () + SCALE_HEIGHT(closeYOffset[m_toolBarSize]),
                                this, closeBtnSize[m_toolBarSize],
                                PB_TOOLBAR_ELE_CLOSE);

    m_elementList[PB_TOOLBAR_ELE_CLOSE] = closeBtn;
    connect (closeBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    connect (closeBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPbBtnButtonClick(int)));

    m_pbSpeed = new TextLabel (this->width () - SCALE_WIDTH(speedXOffset[m_toolBarSize]),
                               (closeBtn->y () + SCALE_HEIGHT(fontYOffset[m_toolBarSize])),
                               SCALE_FONT(speedfontSize[m_toolBarSize]),
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

void PbToolbar::navigationKeyPressed(QKeyEvent *event)
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

    default:
        event->accept();
        break;
    }
}

void PbToolbar::asciiCharKeyPressed(QKeyEvent *event)
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
        emit sigPbToolbarBtnClick(PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
        break;
    }
}

void PbToolbar::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
        // emit sig to close toolbar
        event->accept();
        emit sigPbToolbarBtnClick(PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
        break;

    case Qt::Key_F12:
        event->accept();
        keyHandling (PB_TOOLBAR_ELE_NEXT_BTN);
        break;

    }
}

void PbToolbar::escKeyPressed(QKeyEvent *event)
{
    // emit sig to close toolbar
    event->accept();
    emit sigPbToolbarBtnClick(PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
}

void PbToolbar::insertKeyPressed(QKeyEvent *event)
{
    // emit sig to close toolbar
    event->accept();
    emit sigPbToolbarBtnClick(PB_TOOLBAR_ELE_CLOSE, m_windowId, false);
}

void PbToolbar::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_PB_TOOLBAR_ELEMENTS)
                % MAX_PB_TOOLBAR_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void PbToolbar::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1)
                % MAX_PB_TOOLBAR_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void PbToolbar::takeDownKeyAction ()
{
    if(m_buttonState[PB_TOOLBAR_ELE_PLAY_BTN] == false)
    {
        keyHandling (PB_TOOLBAR_ELE_PLAY_BTN);
    }
    else if(m_buttonState[PB_TOOLBAR_ELE_REV_PLAY_BTN] == false)
    {
        keyHandling (PB_TOOLBAR_ELE_REV_PLAY_BTN);
    }
    else
    {
        if(toolBarBtn[PB_TOOLBAR_ELE_REV_PLAY_BTN]->hasFocus ())
        {
            toolBarBtn[PB_TOOLBAR_ELE_REV_PLAY_BTN]->takeEnterKeyAction ();
        }
        else
        {
            toolBarBtn[PB_TOOLBAR_ELE_PLAY_BTN]->takeEnterKeyAction ();
        }
    }
}

void PbToolbar::keyHandling(PB_TOOLBAR_ELEMENTS_e index)
{
    m_currElement = index;
    m_elementList[m_currElement]->forceActiveFocus ();
    toolBarBtn[index]->takeEnterKeyAction ();
}

quint16 PbToolbar::getPbToolbarWinId()
{
    return m_windowId;
}

bool PbToolbar::getStateOfButton(PB_TOOLBAR_ELEMENTS_e index)
{
    return  m_buttonState[index];
}

void PbToolbar::changeStateOfButton(PB_TOOLBAR_ELEMENTS_e index, bool state, bool tooltipUpdation)
{
    bool tooltipChanged = false;
    m_buttonState[index] = state;
    switch(index)
    {
    case PB_TOOLBAR_ELE_PLAY_BTN:
        if(state == true)
        {
            toolBarBtn[PB_TOOLBAR_ELE_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_PLAY);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == "Pause") && tooltipUpdation)
            {
                timeTooltip->textChange (pbToolTipStings[index]);
                tooltipChanged = true;
            }
        }
        else
        {
            toolBarBtn[PB_TOOLBAR_ELE_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_PAUSE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == pbToolTipStings[index]))
            {
                timeTooltip->textChange ("Pause");
                tooltipChanged = true;
            }
        }
        break;

    case PB_TOOLBAR_ELE_REV_PLAY_BTN:
        if(state == true)
        {
            toolBarBtn[PB_TOOLBAR_ELE_REV_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_REVERSE_PLAY);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == "Pause") && tooltipUpdation)
            {
                timeTooltip->textChange (pbToolTipStings[index]);
                tooltipChanged = true;
            }
        }
        else
        {
            toolBarBtn[PB_TOOLBAR_ELE_REV_PLAY_BTN]->changeToolbarBtn (PB_TOOLBAR_PAUSE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == pbToolTipStings[index]))
            {
                timeTooltip->textChange ("Pause");
                tooltipChanged = true;
            }
        }
        break;

    case PB_TOOLBAR_ELE_MUTE_BTN:
        if(state == true)
        {
            toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->changeToolbarBtn (PB_TOOLBAR_MUTE);
            if((timeTooltip->isVisible ()) && (timeTooltip->getTooltipText () == "Disable Audio"))
            {
                timeTooltip->textChange (pbToolTipStings[index]);
                tooltipChanged = true;
            }
        }
        else
        {
            toolBarBtn[PB_TOOLBAR_ELE_MUTE_BTN]->changeToolbarBtn (PB_TOOLBAR_UNMUTE);
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

void PbToolbar::changeButtonEnableState(PB_TOOLBAR_ELEMENTS_e button,bool isEnable)
{
    toolBarBtn[button]->setIsEnabled(isEnable);
}

void PbToolbar::updateCurrTime(quint64 currTime)
{
    m_recCurrTime = currTime;
    if(m_totalTimeDiff != 0)
    {
        quint64 percent = (((m_recCurrTime - m_recStartTime) *10000) /m_totalTimeDiff);
        slider->changeValue ((m_sliderActiveWidth * percent) / 10000);
    }
    else
    {
        m_pbSpeed->changeText("");
        m_pbDateTime->changeText("");
        m_pbSpeed->update();
        m_pbDateTime->update();
    }
}

void PbToolbar::slotPbBtnButtonClick(int index)
{
    emit sigPbToolbarBtnClick(index, m_windowId, m_buttonState[index]);
}

void PbToolbar::slotPbSliderValueChanged(int currVal ,int)
{
    quint64 percent = ((currVal * 10000)/m_sliderActiveWidth);
    m_recCurrTime = (m_recStartTime + (((m_totalTimeDiff) * percent) / 10000));
    slider->resetFlags ();
    emit sigSliderValueChanged (m_recCurrTime, m_windowId);
}

void PbToolbar::slotHoverOnSlider(int currVal, int)
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

void PbToolbar::slotHoverInOutOnSlider (bool hoverState, int)
{
    if(!hoverState)
    {
        timeTooltip->setVisible (hoverState);
    }
}

void PbToolbar::slotUpadateCurrentElement(int index)
{
    m_currElement = index;
}

void PbToolbar::slotImageMouseHover(quint8 indexInPage, bool isHover)
{
    switch(indexInPage)
    {
    case PB_TOOLBAR_ELE_PLAY_BTN:
        if(m_buttonState[indexInPage] == true)
        {
            timeTooltip->textChange (pbToolTipStings[indexInPage]);
        }
        else
        {
            timeTooltip->textChange ("Pause");
        }
        break;

    case PB_TOOLBAR_ELE_REV_PLAY_BTN:
        if(m_buttonState[indexInPage] == true)
        {
            timeTooltip->textChange (pbToolTipStings[indexInPage]);
        }
        else
        {
            timeTooltip->textChange ("Pause");
        }
        break;

    case PB_TOOLBAR_ELE_MUTE_BTN:
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

void PbToolbar::showPbSpeedDateTime(quint16 arrayWindowIndex)
{
    QString pbdateTimeStr = Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_pbCurrTimeStr;
    pbdateTimeStr.insert (2, "/");
    pbdateTimeStr.insert (5, "/");
    pbdateTimeStr.insert (10, "\n");
    pbdateTimeStr.insert (13, ":");
    pbdateTimeStr.insert (16, ":");

    m_pbSpeed->changeText(pbSpeedStr
                          [Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_pbDirection]
                          [Layout::streamInfoArray[MAIN_DISPLAY][arrayWindowIndex].m_pbSpeed]);
    m_pbDateTime->changeText(pbdateTimeStr);
    m_pbSpeed->update();
    m_pbDateTime->update();
}
