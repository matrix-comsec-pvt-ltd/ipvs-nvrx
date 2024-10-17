#include "SyncPlaybackToolbar.h"
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>

const QString speedString[MAX_PB_SPEED] = {"x/16",
                                           "x/8",
                                           "x/4",
                                           "x/2",
                                           "1x",
                                           "2x",
                                           "4x",
                                           "8x",
                                           "16x"};
const QString directionString[MAX_PLAY_DIRECTION] = {"","-"};

static const bool syncPlaybackToolbarButtonState[SYNC_PLAYBACK_MAX_STATE][MAX_SYNCPB_BUTTON] =
{//SYNC_PLAYBACK_NONE_STATE
 {false, false, false, false, false, false, false, false, false, false, false, false, false},
 //SYNC_PLAYBACK_PLAY_STATE
 {true, true, true, true, true, true, true, true, true, true, true, true, true},
 //SYNC_PLAYBACK_PAUSE_STATE
 {true, true, true, false, false, true, true, true, true, true, true, true, true},
 //SYNC_PLAYBACK_STOP_STATE
 {false, false, false, false, false, false, false, false, false, false, true, false, false},
 //SYNC_PLAYBACK_CROPANDBACKUP_STATE
 {true, true, true, false, false, true, true, true, false, true, false, true, false},
 //SYNC_PLAYBACK_REVERSEPLAY_STATE
 {true, true, true, true, true, true, true, false, true, true, true, true, true},
 //SYNC_PLAYBACK_STEP_STATE
 {true, true, true, false, false, true, true, false, true, true, true, true, true}
};

const QString toolTipString[MAX_SYNCPB_BUTTON][MAX_STATE_TYPE] = {{"Play", "Pause"},
                                                                  {"Stop", "Stop"},
                                                                  {"Reverse Play", "Pause"},
                                                                  {"Slow", "Slow"},
                                                                  {"Fast", "Fast"},
                                                                  {"Previous Frame", "Previous Frame"},
                                                                  {"Next Frame", "Next Frame"},
                                                                  {"Enable Audio", "Disable Audio"},
                                                                  {"Zoom In", "Zoom Out"},
                                                                  {"Clip Start", "Clip Stop"},
                                                                  {"Clip List", "Clip List"},
                                                                  {"Display Mode"},
                                                                  {"Full Screen", "Normal View"}};

SyncPlaybackToolbar::SyncPlaybackToolbar(quint16 startX,
                                         quint16 startY,
                                         quint16 width,
                                         quint16 height,
                                         int indexInPage,
                                         QWidget *parent)
    : KeyBoard(parent),
      NavigationControl(indexInPage, true), m_startX(startX), m_startY(startY),
      m_width(width), m_height(height), m_currentMode(NORMAL_MODE),
      m_currentLayout(ONE_X_ONE_PLAYBACK), m_currentSpeed(PB_SPEED_NORMAL),
      m_currentDirection(FORWARD_PLAY)
{
    this->setGeometry(m_startX, m_startY, m_width, m_height);

    for(quint8 index = 0; index < MAX_SYNCPB_BUTTON; index++)
    {
        m_toolbarButton[index] = new SyncPlaybackToolbarButton((SYNCPB_TOOLBAR_BUTTON_TYPE_e)index,
                                                               this,
                                                               index);
        m_elementList[index] = m_toolbarButton[index];
        connect(m_toolbarButton[index],
                SIGNAL(sigButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)),
                this,
                SLOT(slotButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)));
        connect(m_toolbarButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_toolbarButton[index],
                SIGNAL(sigShowHideToolTip(int,bool)),
                this,
                SLOT(slotShowHideTooltip(int,bool)));

    }
    m_speedTextLabel = new TextLabel((m_toolbarButton[CHANGE_MODE_BUTTON]->x() + m_toolbarButton[CHANGE_MODE_BUTTON]->width() + SCALE_WIDTH(10)),
                                     (this->height() / 2),
                                     NORMAL_FONT_SIZE,
                                     "",
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_START_X_CENTRE_Y);
	m_dateTimeTextLabel = new TextLabel((m_toolbarButton[CHANGE_MODE_BUTTON]->x() + m_toolbarButton[CHANGE_MODE_BUTTON]->width() + SCALE_WIDTH(80)),
                                        (this->height() / 2),
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        HIGHLITED_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y);

    m_toolTip = new ToolTip(this->x(),
                            (this->y() - SCALE_HEIGHT(5)),
                            "",
                            parentWidget(),
                            START_X_END_Y);
    m_toolTip->setVisible(false);
	/* Create SyncPB Layout List */
	m_layoutList = new LayoutList(m_toolbarButton[LAYOUT_BUTTON]->x(),
                                    (this->y()-(MAX_SYNC_PB_LAYOUT*LAYOUT_LIST_BUTTON_HEIGHT)-SCALE_WIDTH(10)),
									LAYOUT_LIST_4X1_TYPE,
									parentWidget(),
									MAX_SYNC_PB_LAYOUT,
									indexInPage);
	/* Hide LayoutList By default */
	setLayoutListVisiblity(false);
	connect(m_layoutList,
			 SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
			 this,
			 SLOT(slotChangeLayout(LAYOUT_TYPE_e)));
    changeState(SYNC_PLAYBACK_NONE_STATE);
    this->show();
}

SyncPlaybackToolbar::~SyncPlaybackToolbar()
{
    for(int index = 0; index < MAX_SYNCPB_BUTTON; index++)
    {
        disconnect(m_toolbarButton[index],
                   SIGNAL(sigButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)),
                   this,
                   SLOT(slotButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)));
        disconnect(m_toolbarButton[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_toolbarButton[index],
                   SIGNAL(sigShowHideToolTip(int,bool)),
                   this,
                   SLOT(slotShowHideTooltip(int,bool)));
        delete m_toolbarButton[index];
    }
	disconnect(m_layoutList,
				SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
				this,
				SLOT(slotChangeLayout(LAYOUT_TYPE_e)));
	delete m_layoutList;

    delete m_dateTimeTextLabel;
    delete m_speedTextLabel;
    delete m_toolTip;
}

void SyncPlaybackToolbar::changeMode(SYNCPB_TOOLBAR_MODE_TYPE_e mode)
{
    m_toolTip->setVisible(false);
	/* Hide LayoutList on ViewMode Change */
	setLayoutListVisiblity(false);
    m_currentMode = mode;
    for(int index = 0; index < MAX_SYNCPB_BUTTON; index++)
    {
        m_toolbarButton[index]->changeMode(m_currentMode);
    }

    STATE_TYPE_e state = (m_currentMode == FULL_MODE ? STATE_2 : STATE_1);
    changeButtonState(CHANGE_MODE_BUTTON, state);

    m_speedTextLabel->setOffset((m_toolbarButton[CHANGE_MODE_BUTTON]->x() + m_toolbarButton[CHANGE_MODE_BUTTON]->width() + SCALE_WIDTH(10)),
                                (this->height() / 2));
	m_dateTimeTextLabel->setOffset((m_toolbarButton[CHANGE_MODE_BUTTON]->x() + m_toolbarButton[CHANGE_MODE_BUTTON]->width() + SCALE_WIDTH(80)),
                                   (this->height() / 2));
	/* Update LayoutList Geometry on ViewMode Change */
    m_layoutList->resetGeometry(m_toolbarButton[LAYOUT_BUTTON]->x(),this->y()-(4*LAYOUT_LIST_BUTTON_HEIGHT)-SCALE_WIDTH(10));
}

void SyncPlaybackToolbar::changeState(SYNC_PLAYBACK_STATE_e state)
{
    bool changeFocusToOtherElement = false;
    bool buttonEnableState;
    for(quint8 index = 0; index < MAX_SYNCPB_BUTTON; index++)
    {
        buttonEnableState = syncPlaybackToolbarButtonState[state][index];

        if((m_toolbarButton[index]->hasFocus())
                && (!buttonEnableState))
        {
            changeFocusToOtherElement = true;
        }
        m_toolbarButton[index]->setIsEnabled(buttonEnableState);
    }
    if(changeFocusToOtherElement)
    {
        m_currentElement = (PLAY_BUTTON - 1);
        takeRightKeyAction();
    }
}

void SyncPlaybackToolbar::changeButtonState(SYNCPB_TOOLBAR_BUTTON_TYPE_e button,
                                            STATE_TYPE_e state)
{
    m_toolbarButton[button]->changeButtonState(state);
    if((m_toolTip->isVisible())
            && (m_toolbarButton[button]->hasFocus()))
    {
        m_toolTip->textChange(toolTipString[button][m_toolbarButton[button]->getButtonState()]);
        m_toolTip->resetGeometry(m_toolbarButton[button]->x(), (this->y() - SCALE_HEIGHT(5)));
    }
}

void SyncPlaybackToolbar::changeButtonEnableState(SYNCPB_TOOLBAR_BUTTON_TYPE_e button,
                                                  bool isEnable)
{
    m_toolbarButton[button]->setIsEnabled(isEnable);
}

STATE_TYPE_e SyncPlaybackToolbar::getButtonState(SYNCPB_TOOLBAR_BUTTON_TYPE_e button)
{
    return m_toolbarButton[button]->getButtonState();
}

bool SyncPlaybackToolbar::isButtonEnable(SYNCPB_TOOLBAR_BUTTON_TYPE_e button)
{
    return m_toolbarButton[button]->isEnabled();
}

void SyncPlaybackToolbar::changeDateTime(QDateTime dateTime)
{
    QRect textLableGeometry = m_dateTimeTextLabel->geometry();
    m_dateTimeTextLabel->changeText(dateTime.toString("dd-MM-yyyy hh:mm:ss"));

    if(textLableGeometry == m_dateTimeTextLabel->geometry())
    {
        m_dateTimeTextLabel->update();
    }
}

void SyncPlaybackToolbar::changePlaybackSpeedDirection(PB_SPEED_e speed,
                                                       PB_DIRECTION_e direction)
{
    m_currentSpeed = speed;
    m_currentDirection = direction;
    m_speedTextLabel->changeText(directionString[m_currentDirection] + speedString[m_currentSpeed]);
    if(m_currentSpeed == PB_SPEED_NORMAL)
    {
        m_speedTextLabel->changeColor (HIGHLITED_FONT_COLOR);
    }
    else
    {
        m_speedTextLabel->changeColor (YELOW_COLOR);
    }
    m_speedTextLabel->update();
}

void SyncPlaybackToolbar::resetToolbar()
{
    for(quint8 buttonIndex = PLAY_BUTTON; buttonIndex <= ZOOM_BUTTON; buttonIndex++)
    {
        m_toolbarButton[buttonIndex]->changeButtonState(STATE_1);
    }
    m_currentSpeed = PB_SPEED_NORMAL;
    m_currentDirection = FORWARD_PLAY;

    if(m_speedTextLabel->getText() != "")
    {
        m_speedTextLabel->changeText("");
        m_speedTextLabel->update();
    }

    if(m_dateTimeTextLabel->getText() != "")
    {
        m_dateTimeTextLabel->changeText("");
        m_dateTimeTextLabel->update();
    }
}

void SyncPlaybackToolbar::resetGeometry(quint16 startX,
                                        quint16 startY,
                                        quint16 width,
                                        quint16 height)
{
    m_startX = startX;
    m_startY = startY;
    m_width = width;
    m_height = height;
    this->setGeometry(m_startX, m_startY, m_width, m_height);
}

void SyncPlaybackToolbar::takeButtonEnterKeyAction(SYNCPB_TOOLBAR_BUTTON_TYPE_e button)
{
    if(m_toolbarButton[button]->getIsEnabled() == true)
    {
        emit sigToolbarButtonClicked(button, m_toolbarButton[button]->getButtonState());
    }
}

void SyncPlaybackToolbar::takeLeftKeyAction()
{
    bool status = true;
    if(m_currentElement == 0)
    {
        m_currentElement = (MAX_SYNCPB_BUTTON);
    }
    do
    {
        if(m_currentElement)
        {
            m_currentElement = (m_currentElement - 1);
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }

    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {

        if(m_elementList[m_currentElement]->getIsEnabled())
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void SyncPlaybackToolbar::takeRightKeyAction()
{
    bool status = true;
    if(m_currentElement == (MAX_SYNCPB_BUTTON - 1))
    {
        m_currentElement = -1;
    }
    do
    {
        if(m_currentElement != (MAX_SYNCPB_BUTTON - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
            status = false;
            break;
        }

    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void SyncPlaybackToolbar::forceFocusToPage(bool isFirstElement)
{
    if(isFirstElement)
    {
        m_currentElement = (PLAY_BUTTON - 1);
        takeRightKeyAction();
    }
    else
    {
        m_currentElement = MAX_SYNCPB_BUTTON;
        takeLeftKeyAction();
    }
}

void SyncPlaybackToolbar::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(NORMAL_BKG_COLOR),Qt::SolidPattern));
    painter.drawRect(QRect(0, 0, this->width(), this->height()));
}

void SyncPlaybackToolbar::navigationKeyPressed(QKeyEvent *event)
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

void SyncPlaybackToolbar::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_F4:
        event->accept();
        break;

    default:
        event->accept();
        break;
    }
}

void SyncPlaybackToolbar::showEvent(QShowEvent * event)
{
    if(m_currentMode == FULL_MODE)
    {
        forceFocusToPage(true);
    }
    QWidget::showEvent(event);
}

void SyncPlaybackToolbar::hideEvent(QHideEvent * event)
{
    if(m_toolTip->isVisible())
    {
        m_toolTip->setVisible(false);
    }
    QWidget::hideEvent(event);
}

void SyncPlaybackToolbar::slotButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state)
{
	/* Toggle LayoutList Visibility on Layout Button Click */
	if(index==LAYOUT_BUTTON)
	{
		if (false == m_layoutList->isVisible())
		{
			setLayoutListVisiblity(true);
		}
		else
		{
			setLayoutListVisiblity(false);
		}

	}
	else
	{
		/* Hide LayoutList on other toolbar Button clicked */
		setLayoutListVisiblity(false);
		if(this->isVisible() == true)
		{
			emit sigToolbarButtonClicked(index, state);
		}
	}
}

void SyncPlaybackToolbar::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
    emit sigUpdateCurrentElement(m_indexInPage);
}

void SyncPlaybackToolbar::slotShowHideTooltip(int index, bool toShowTooltip)
{
    if(toShowTooltip)
    {
        if(this->isVisible() == true)
        {
            m_toolTip->textChange(toolTipString[index][m_toolbarButton[index]->getButtonState()]);
            m_toolTip->resetGeometry(m_toolbarButton[index]->x(), (this->y() - SCALE_HEIGHT(5)));
            m_toolTip->setVisible(true);
        }
    }
    else
    {
        m_toolTip->setVisible(false);
    }
}

bool SyncPlaybackToolbar::isLayoutListVisible(void)
{
	/* Return LayoutList Visibility */
	return (m_layoutList->isVisible());
}

void SyncPlaybackToolbar::setLayoutListVisiblity(bool iVisibleF)
{
	/* Update LayoutList Visibility */
	m_layoutList->setVisible(iVisibleF);
}

void SyncPlaybackToolbar::slotChangeLayout(LAYOUT_TYPE_e iLayoutType)
{
	LAYOUT_TYPE_e tSyncPbLayoutType = iLayoutType;

	/* Hide LayoutList on Layout Selection */
	setLayoutListVisiblity(false);

	/* Map Layout Type to SyncPB LayoutType for NORMAL Mode */
	if(NORMAL_MODE == m_currentMode)
	{
		switch (tSyncPbLayoutType)
		{
			case ONE_X_ONE:
			{
				tSyncPbLayoutType = ONE_X_ONE_PLAYBACK;
			}
			break;

			case TWO_X_TWO:
			{
				tSyncPbLayoutType = TWO_X_TWO_PLAYBACK;
			}
			break;

			case THREE_X_THREE:
			{
				tSyncPbLayoutType = THREE_X_THREE_PLAYBACK;
			}
			break;

			case FOUR_X_FOUR:
			{
				tSyncPbLayoutType = FOUR_X_FOUR_PLAYBACK;
			}
			break;

			default:
			{
				return;
			}
		}
	}
	emit sigChangeLayout(tSyncPbLayoutType);
}


