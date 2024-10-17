#include "SyncPlaybackTimeLine.h"
#include <QKeyEvent>

#define BUTTON_TOP_MARGIN              SCALE_HEIGHT(15)
#define BUTTON_BOTTOM_MARGIN           SCALE_HEIGHT(7)
#define BUTTON_LEFT_MARGIN             SCALE_WIDTH(5)
#define INTER_CONTROL_MARGIN           SCALE_WIDTH(15)
#define BUTTON_HEIGHT                  SCALE_HEIGHT(50)
#define BUTTON_WIDTH                   SCALE_WIDTH(60)

#define GRID_ROW_COLOR                  "#000000"
#define GRID_COL_COLOR                  "#101710"

#define SLIDER_IMAGE_PATH               ":Images_Nvrx/SyncPlayBackToolbar/TimeLineSlider/"



const quint8 offSet[MAX_HOUR_FORMAT_TYPE] = {1, 2, 4, 4};
const quint8 maxSlot[MAX_HOUR_FORMAT_TYPE] = {1, 2, 4, 24};
const quint8 slotValueInHours[MAX_HOUR_FORMAT_TYPE] = {24, 12, 6, 1};

SyncPlaybackTimeLine::SyncPlaybackTimeLine(quint16 startX,
                                           quint16 startY,
                                           quint16 width,
                                           quint16 height,
                                           quint16 indexInPage,
                                           QWidget* parent,
                                           bool isEnable,
                                           bool catchKey)
    : KeyBoard(parent), NavigationControl(indexInPage, isEnable, catchKey)
{
    m_applController = ApplController::getInstance();
    this->setGeometry(startX, startY, width, height);
    m_currentHour = m_currentMinute = m_currentSecond = m_currentSlot = 0;

    updateTimeString();
    m_currentHourFormat = HOUR_24;
    m_currentPlaybackState = SYNC_PLAYBACK_NONE_STATE;
    m_cropStartingPosition = m_cropEndingPosition = 0;
    m_isSliderToolTipVisible = false;

    m_prevSlotButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
										 BUTTON_LEFT_MARGIN,
                                         BUTTON_TOP_MARGIN,
										 SCALE_WIDTH(30), BUTTON_HEIGHT,
                                         this,
                                         NO_LAYER, 0, "",
                                         false,
                                         SYNCPLAYBACK_TIMELINE_PREV_SLOT_BUTTON);
    m_elementList[SYNCPLAYBACK_TIMELINE_PREV_SLOT_BUTTON] = m_prevSlotButton;
    connect(m_prevSlotButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_prevSlotButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotControlButtonClicked(int)));

	m_textLabelBackgroundRect = new Rectangle((m_prevSlotButton->x() + m_prevSlotButton->width() + INTER_CONTROL_MARGIN),
                                              0,
                                              GRID_WIDTH,
                                              BUTTON_TOP_MARGIN,
                                              CLICKED_BKG_COLOR,
                                              this);

    m_nextSlotButton = new ControlButton(NEXT_BUTTON_INDEX,
                                         (m_textLabelBackgroundRect->x() + m_textLabelBackgroundRect->width() + INTER_CONTROL_MARGIN),
                                         BUTTON_TOP_MARGIN,
                                         SCALE_WIDTH(30),
										 BUTTON_HEIGHT,
                                         this,
                                         NO_LAYER, 0, "",
                                         false,
                                         SYNCPLAYBACK_TIMELINE_NEXT_SLOT_BUTTON);
    m_elementList[SYNCPLAYBACK_TIMELINE_NEXT_SLOT_BUTTON] = m_nextSlotButton;
    connect(m_nextSlotButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_nextSlotButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotControlButtonClicked(int)));

    for(quint8 index = 0; index < (MAX_COL + 1); index++)
	{
        m_hourIndicatorLabel[index] = new TextLabel((qreal)((qreal)m_textLabelBackgroundRect->x() + (qreal)((qreal)index * ((qreal)GRID_WIDTH /(qreal)24))),
                                                    (m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height()),
													SCALE_FONT(10),
													QString("%1").arg(index) + ":00",
                                                    this,
                                                    SUFFIX_FONT_COLOR,
                                                    NORMAL_FONT_FAMILY,
													ALIGN_CENTRE_X_END_Y);
        m_gridCols[index] = new Rectangle((qreal)((qreal)m_textLabelBackgroundRect->x() + (qreal)((qreal)index * ((qreal)GRID_WIDTH /(qreal)24))),
                                          (qreal)(m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height()),
                                          (qreal)1,
										  (qreal)BUTTON_HEIGHT,
                                          GRID_COL_COLOR,
										  this);
    }

    m_recordLine = new SyncPlaybackRecordLine(m_textLabelBackgroundRect->x(),
													(m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height() + (BUTTON_HEIGHT / 2)),
                                                     m_currentHourFormat,
                                                     this);
	m_cropAndBackupIndicatorLines = new Rectangle((m_textLabelBackgroundRect->x() - 1),
														 (m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height() + (BUTTON_HEIGHT / 2)),
														 GRID_WIDTH,
														 SCALE_HEIGHT(2),
														 PROCESS_BKG_COLOR,
														 this,
														 0, 0,
														 PROCESS_BKG_COLOR,
														 0.55);
	m_cropAndBackupIndicatorLines->setVisible(false);

    m_slider = new SliderControl((m_textLabelBackgroundRect->x() - SCALE_WIDTH(9)),
                                 (m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height()),
                                 (m_textLabelBackgroundRect->width() + SCALE_WIDTH(18)),
								 BUTTON_HEIGHT,
                                 GRID_WIDTH,
                                 1,
                                 SLIDER_IMAGE_PATH,
                                 0,
                                 this,
                                 HORIZONTAL_SLIDER,
                                 SYNCPLAYBACK_TIMELINE_SLIDER_CONTROL,
								 true,
                                 false,
                                 true,
                                 true,
								 false);
    m_elementList[SYNCPLAYBACK_TIMELINE_SLIDER_CONTROL] = m_slider;
    connect(m_slider,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_slider,
            SIGNAL(sigValueChanged(int,int,bool)),
            this,
            SLOT(slotSliderValueChanged(int,int,bool)));
    connect(m_slider,
            SIGNAL(sigMouseReleaseOnSlider(int,int)),
            this,
            SLOT(slotMouseReleaseOnSlider(int,int)));
    connect(m_slider,
            SIGNAL(sigMousePressedOnSlider(int,int)),
            this,
            SLOT(slotMousePressedOnSlider(int,int)));
	/* Hide Slider control */
	m_slider->setVisible(false);

    m_sliderToolTip = new ToolTip(m_textLabelBackgroundRect->x() + m_slider->getCurrentValue(),
                                  (m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height() + SCALE_HEIGHT(10)),
                                  m_timeString,
                                  this,
                                  CENTER_X_END_Y);
	m_sliderToolTip->showHideTooltip(false);
    m_currentElement = SYNCPLAYBACK_TIMELINE_SLIDER_CONTROL;

}

SyncPlaybackTimeLine::~SyncPlaybackTimeLine()
{
    delete m_sliderToolTip;

    disconnect(m_slider,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_slider,
               SIGNAL(sigValueChanged(int,int,bool)),
               this,
               SLOT(slotSliderValueChanged(int,int,bool)));
    disconnect(m_slider,
               SIGNAL(sigMouseReleaseOnSlider(int,int)),
               this,
               SLOT(slotMouseReleaseOnSlider(int,int)));
    disconnect(m_slider,
               SIGNAL(sigMousePressedOnSlider(int,int)),
               this,
               SLOT(slotMousePressedOnSlider(int,int)));
    delete m_slider;

    delete m_recordLine;
	delete m_cropAndBackupIndicatorLines;

    for(quint8 index = 0; index < (MAX_COL + 1); index++)
    {
        delete m_hourIndicatorLabel[index];
        delete m_gridCols[index];
    }

    disconnect(m_nextSlotButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_nextSlotButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotControlButtonClicked(int)));
    delete m_nextSlotButton;

    delete m_textLabelBackgroundRect;

    disconnect(m_prevSlotButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_prevSlotButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotControlButtonClicked(int)));
    delete m_prevSlotButton;

}

void SyncPlaybackTimeLine::changePointToTime(int point, bool forceSec)
{
    qreal secondValueOfOnePixel = (qreal)(slotValueInHours[m_currentHourFormat] * 3600) / (qreal)GRID_WIDTH;
    quint32 totalSeconds;

    /* Here it is made to decrease by -1 else tooltip would show for next day */
    if(forceSec)
    {
        totalSeconds = point * secondValueOfOnePixel;
        /* Removing the last second of the total time as it would be considered in next day or next hour depending on the hours format*/
        if(point == GRID_WIDTH)
        {
            totalSeconds = totalSeconds - 1;
        }
        m_currentSecond = (totalSeconds % 60);
    }
    else
    {
        totalSeconds = (m_currentHour * 3600) + (m_currentMinute * 60) + m_currentSecond;
        qreal pixelPosition = (qreal)totalSeconds / (qreal)secondValueOfOnePixel;
        /* PARASOFT : Rule OWASP2021-A5-c - No need to check errno */
        qreal position_point = (qreal) fmod(pixelPosition , (qreal) GRID_WIDTH);
        totalSeconds = position_point * secondValueOfOnePixel;
    }

    m_currentMinute = totalSeconds / 60;
    m_currentHour = (m_currentMinute / 60) + (m_currentSlot * slotValueInHours[m_currentHourFormat]);
    m_currentMinute = m_currentMinute % 60;

    updateTimeString();
}

int SyncPlaybackTimeLine::changeTimeToPoint()
{
	qreal secondValueOfOnePixel = (qreal)(slotValueInHours[m_currentHourFormat] * 3600) / (qreal)GRID_WIDTH;
    quint32 totalSeconds = (m_currentHour * 3600) + (m_currentMinute * 60) + m_currentSecond;
    /* Adding a second as the time to be shown as for 1hr and 6hrs format it would not point as expected reducing the recording to 2-14 seconds */
    quint32 pixelPosition = (totalSeconds + 1) / secondValueOfOnePixel;
	return (pixelPosition % GRID_WIDTH);
}

void SyncPlaybackTimeLine::updateTimeString()
{
    m_timeString = (m_currentHour < 10 ? "0" + QString("%1").arg(m_currentHour) : QString("%1").arg(m_currentHour))
            + ":" + (m_currentMinute < 10 ? "0" + QString("%1").arg(m_currentMinute) : QString("%1").arg(m_currentMinute))
            + ":" + (m_currentSecond < 10 ? "0" + QString("%1").arg(m_currentSecond) : QString("%1").arg(m_currentSecond));
}

void SyncPlaybackTimeLine::changeHourFormat(HOURS_FORMAT_TYPE_e hourFormat)
{
    m_currentHourFormat = hourFormat;
    if(m_currentHourFormat != HOUR_24)
    {
        m_prevSlotButton->setIsEnabled(true);
        m_nextSlotButton->setIsEnabled(true);
    }
    else
    {
        m_prevSlotButton->setIsEnabled(false);
        m_nextSlotButton->setIsEnabled(false);
    }

    changeCurrentSlot((m_currentHour / slotValueInHours[m_currentHourFormat]), true);
}

void SyncPlaybackTimeLine::changeCurrentSlot(quint8 currentSlot, bool changeHourIndicatorFlag)
{
    if((m_currentPlaybackState == SYNC_PLAYBACK_CROPANDBACKUP_STATE)
            && (m_currentSlot != currentSlot))
    {
        resetCropAndBackupDataLines();
        m_cropStartingPosition = m_cropEndingPosition = changeTimeToPoint();
    }

    if((m_currentSlot != currentSlot) || (changeHourIndicatorFlag == true))
    {
        for(quint8 index = 0; index < (MAX_COL + 1); index++)
        {
            m_hourIndicatorLabel[index]->changeText("");
            if(!(index % offSet[m_currentHourFormat]))
            {
                quint32 minute = ((index * 60) / maxSlot[m_currentHourFormat]) + (currentSlot * slotValueInHours[m_currentHourFormat] * 60);
                m_hourIndicatorLabel[index]->changeText(changeMinuteToHour(minute));
            }
        }
    }

    m_recordLine->changeHourFormatAndSlot(m_currentHourFormat, currentSlot);
    m_currentSlot = currentSlot;
    m_slider->changeValue(changeTimeToPoint());
}

void SyncPlaybackTimeLine::updateTooltipPosition()
{
    m_sliderToolTip->textChange(m_timeString);
    m_sliderToolTip->resetGeometry(m_textLabelBackgroundRect->x() + m_slider->getCurrentValue(),
                                   (m_textLabelBackgroundRect->y() + m_textLabelBackgroundRect->height() + SCALE_HEIGHT(10)));
}

QString SyncPlaybackTimeLine::changeMinuteToHour(quint32 minute)
{
    quint8 hour, min;
    QString hourString;
    if(minute == 0)
    {
		hourString = QString("%1").arg(minute) + ":00";
    }
    else if(minute < 60)
    {
        hour = ((qreal) minute / 100);
		hourString = "00:" + QString("%1").arg(minute);
    }
    else
    {
        hour = minute / 60;
        min = minute % 60;
		hourString = QString("%1").arg(hour) + (min != 0 ? ":" + QString("%1").arg(min) : ":00");
    }
    return hourString;
}

void SyncPlaybackTimeLine::showRecordsOnTimeline(quint8 iCamIndex)
{
	QList<quint16> tCamRecordData;
	resetRecordsOnTimeline();
	if(iCamIndex < INVALID_CAMERA_INDEX)
	{
		tCamRecordData.clear();
		if(true == m_applController->GetCamRecData(iCamIndex -1, tCamRecordData))
		{
			tCamRecordData.removeFirst();
			m_recordLine->changeRecordList(tCamRecordData);
		}
		/* Visible Slider control */
		m_slider->setVisible(true);
	}
}

void SyncPlaybackTimeLine::resetRecordsOnTimeline()
{
	/* Hide Slider control */
	m_slider->setVisible(false);
	m_recordLine->resetRecordList();
}

void SyncPlaybackTimeLine::showCropAndBackupDataLines()
{
	m_cropStartingPosition = m_cropEndingPosition = changeTimeToPoint();
	m_cropAndBackupIndicatorLines->setVisible(true);
	m_cropAndBackupIndicatorLines->resetGeometry((m_cropStartingPosition + m_textLabelBackgroundRect->x()), 1);
}

void SyncPlaybackTimeLine::resetCropAndBackupDataLines()
{
	m_cropStartingPosition = m_cropEndingPosition = 0;
	if(m_currentPlaybackState != SYNC_PLAYBACK_CROPANDBACKUP_STATE)
	{
		m_cropAndBackupIndicatorLines->setVisible(false);
	}
	else
	{
		m_cropAndBackupIndicatorLines->setVisible(true);
	}
	m_cropAndBackupIndicatorLines->resetGeometry((m_textLabelBackgroundRect->x() - 1), 1);
}

QString SyncPlaybackTimeLine::getTimeString()
{
    return QTime(m_currentHour, m_currentMinute, m_currentSecond).toString("hhmmss");
}

void SyncPlaybackTimeLine::changeTime(QTime time)
{
    if(((qint32)m_currentHour != time.hour())
            || ((qint32)m_currentMinute != time.minute())
            || ((qint32)m_currentSecond != time.second()))
    {
        m_currentHour = time.hour();
        m_currentMinute = time.minute();
        m_currentSecond = time.second();
        changeCurrentSlot((m_currentHour / slotValueInHours[m_currentHourFormat]));
    }
}

void SyncPlaybackTimeLine::changeState(SYNC_PLAYBACK_STATE_e state)
{
	m_currentPlaybackState = state;
}

void SyncPlaybackTimeLine::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (SYNCPLAYBACK_MAX_TIMELINE_ELEMENT);
        }

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
            if(m_currentElement < SYNCPLAYBACK_TIMELINE_PREV_SLOT_BUTTON)
            {
                emit sigFocusToOtherElement(true);
            }
            else
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
    }
}

void SyncPlaybackTimeLine::takeRightKeyAction()
{
	bool status = true;
	do
	{
		if(m_currentElement == (SYNCPLAYBACK_MAX_TIMELINE_ELEMENT - 1))
		{
			m_currentElement = -1;
		}
		if(m_currentElement != (SYNCPLAYBACK_MAX_TIMELINE_ELEMENT - 1))
		{
			m_currentElement = (m_currentElement + 1);
		}
		else
		{
			status = false;
			break;
		}
	}while((m_elementList[m_currentElement] == NULL) ||
			(!m_elementList[m_currentElement]->getIsEnabled()));

	if(status == true)
	{
		if(m_currentElement >= SYNCPLAYBACK_MAX_TIMELINE_ELEMENT)
		{
			emit sigFocusToOtherElement(false);
		}
		else
		{
			m_elementList[m_currentElement]->forceActiveFocus();
		}
	}
}

void SyncPlaybackTimeLine::forceFocusToPage(bool isFirstElement)
{
	if(isFirstElement)
	{
		m_currentElement = (SYNCPLAYBACK_TIMELINE_PREV_SLOT_BUTTON - 1);
		takeRightKeyAction();
	}
	else
	{
		m_currentElement = SYNCPLAYBACK_MAX_TIMELINE_ELEMENT;
		takeLeftKeyAction();
	}
}

void SyncPlaybackTimeLine::navigationKeyPressed(QKeyEvent *event)
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

        break;

    case Qt::Key_Down:
        event->accept();

        break;

    default:
        event->accept();
        break;
    }
}

void SyncPlaybackTimeLine::mouseMoveEvent(QMouseEvent *)
{
    emit sigUpdateCurrentElement(m_indexInPage);
}

void SyncPlaybackTimeLine::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
    emit sigUpdateCurrentElement(m_indexInPage);
}

void SyncPlaybackTimeLine::slotSliderValueChanged(int changedValue, int, bool sliderMove)
{
	changePointToTime(changedValue, sliderMove);
	if(m_isSliderToolTipVisible == true)
	{
		updateTooltipPosition();
	}
	if(m_currentPlaybackState == SYNC_PLAYBACK_CROPANDBACKUP_STATE)
	{
		if((quint32)changeTimeToPoint() < m_cropStartingPosition)
		{
			m_cropAndBackupIndicatorLines->resetGeometry((changeTimeToPoint() + m_textLabelBackgroundRect->x()),
																(m_cropStartingPosition - changeTimeToPoint()));
		}
		else
		{
			m_cropAndBackupIndicatorLines->resetGeometry((m_cropStartingPosition + m_textLabelBackgroundRect->x()),
																(changeTimeToPoint() - m_cropStartingPosition));
		}
	}
}

void SyncPlaybackTimeLine::slotControlButtonClicked(int indexInPage)
{
	switch(indexInPage)
	{
		case SYNCPLAYBACK_TIMELINE_PREV_SLOT_BUTTON:
			m_currentMinute = m_currentSecond = 0;
			m_currentHour = (m_currentSlot * slotValueInHours[m_currentHourFormat]);
			changeCurrentSlot(((m_currentSlot - 1 + maxSlot[m_currentHourFormat]) % maxSlot[m_currentHourFormat]));
			emit sigSliderPositionChangedStart();
			emit sigSliderPositionChanged();
			break;

		case SYNCPLAYBACK_TIMELINE_NEXT_SLOT_BUTTON:
			m_currentMinute = m_currentSecond = 0;
			m_currentHour = (m_currentSlot * slotValueInHours[m_currentHourFormat]);
			changeCurrentSlot((m_currentSlot + 1) % maxSlot[m_currentHourFormat]);
			emit sigSliderPositionChangedStart();
			emit sigSliderPositionChanged();
			break;

		default:
			break;
	}
}

void SyncPlaybackTimeLine::slotMouseReleaseOnSlider(int, int)
{
	m_isSliderToolTipVisible = false;
	m_sliderToolTip->showHideTooltip(false);
	emit sigSliderPositionChanged();
}

void SyncPlaybackTimeLine::slotMousePressedOnSlider(int, int)
{
	m_isSliderToolTipVisible = true;
	updateTooltipPosition();
	emit sigSliderPositionChangedStart();
}

