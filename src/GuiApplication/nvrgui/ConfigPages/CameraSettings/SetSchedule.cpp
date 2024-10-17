#include "SetSchedule.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>

#define SET_SCHEDULE_WIDTH SCALE_WIDTH(536)
#define SET_SCHEDULE_HEIGHT SCALE_HEIGHT(560)

#define ELEMENT_MARGIN            SCALE_WIDTH(15)
#define ADAPTIVE_CHECKBOXES_WIDTH SCALE_WIDTH(30)
#define TIME_BOX_WIDTH            SCALE_WIDTH(60)

#define SHIFT_BIT_ONCE      1

static QString copyToDaysString[]={
    "All" , "Sun" , "Mon" , "Tue" , "Wed" , "Thu" , "Fri" , "Sat"
};

static const quint16 textLabelOffsets[]={
    99,143,187,230,275,320,360,405
};

typedef enum{

    SET_SCHD_ENTR_DAY,
    SET_SCHD_ADPT,
    SET_SCHD_ADPT_CHBOX,
    SET_SCHD_TIME_PER,
    SET_SCHD_START_TIME,
    SET_SCHD_END_TIME,
    SET_SCHD_CPY_TO_CAM,
    SET_SCHD_OK,
    SET_SCHD_CANCEL,
    MAX_SET_SCHD_STRING
}SET_SCHD_STRING_e;

static const QString setScheduleString [MAX_SET_SCHD_STRING] =
{
    "Entire Day",
    "Adaptive",
    "Adaptive",
    "Time Period",
    "Start Time",
    "End Time",
    "Copy To",
    "OK",
    "Cancel"
};

static const QString setRecTypeString [MAX_SET_REC_TYPE] =
{
    "Schedule Recording",
    "Schedule Snapshot",
};

static const QString weekdays[7]=
{
    "Sunday", "Monday", "Tuesday", "Wednesday","Thursday", "Friday", "Saturday"
};

SetSchedule::SetSchedule(SCHEDULE_TIMING_t *setSchdTiming,
                         quint8 *entireDayAndAdaptiveSelect,
                         quint8 *copyToWeekdaysFields,
                         QWidget *parent,
                         quint8 dayIndex,
                         SET_RECORD_TYPE_e recordType) : KeyBoard(parent)
{
    for(quint8 index = 0; index < MAX_SET_SCHD_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    this->setGeometry (0,0,parent->width (),parent->height ());

    setScheduleTiming = setSchdTiming;

    /* isEntireDayAndAdaptiveSelect variable stores the entire flag value on bit 0, entireDayAdaptiveCheckbox at bit 1 and remaining 6 bits  to store adaptive checkboxes */
    isEntireDayAndAdaptiveSelect = entireDayAndAdaptiveSelect;

    currentDayIndex = dayIndex;
    isCopyToWeekdaysSelect = copyToWeekdaysFields;
    m_recordType = recordType;

    infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget ());
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));


    backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) +
                               (SCALE_WIDTH( SETTING_RIGHT_PANEL_WIDTH) - SET_SCHEDULE_WIDTH)/2) ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)) +
                               (SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- SET_SCHEDULE_HEIGHT)/2,
                               SET_SCHEDULE_WIDTH,
                               SET_SCHEDULE_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(20),this,CLOSE_BTN_TYPE_1,
                                    0);

    m_elementlist[SET_SCHD_CLOSE_BTN] = closeButton;

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCloseButtonClick(int)));

    connect (closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(20),
                          setRecTypeString[recordType],
                          this,
                          HEADING_TYPE_2);

    entireDay = new OptionSelectButton(backGround->x () +
                                       ((backGround->width() - BGTILE_MEDIUM_SIZE_WIDTH) / 2),
                                       (backGround->y () + SCALE_HEIGHT(50)),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       CHECK_BUTTON_INDEX,
                                       setScheduleString[SET_SCHD_ENTR_DAY],
                                       this,
                                       COMMON_LAYER,
                                       SCALE_WIDTH(10),
                                       MX_OPTION_TEXT_TYPE_SUFFIX,
                                       NORMAL_FONT_SIZE,
                                       SET_SCHD_ENTIRE_DAY_CTRL);

    m_elementlist[SET_SCHD_ENTIRE_DAY_CTRL] = entireDay;

    if(true == (*isEntireDayAndAdaptiveSelect & IS_SET_SCHD_ENTIREDAY_CHECKED))
    {
        entireDay->changeState (ON_STATE);
    }
    m_currentElement = SET_SCHD_ENTIRE_DAY_CTRL;

    connect (entireDay,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    connect (entireDay,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    entireDayAdaptiveCheckbox = new OptionSelectButton(entireDay->x() + (entireDay->width()/2),
                                       entireDay->y(),
                                       BGTILE_MEDIUM_SIZE_WIDTH / 2,
                                       BGTILE_HEIGHT,
                                       CHECK_BUTTON_INDEX,
                                       setScheduleString[SET_SCHD_ADPT],
                                       this,
                                       NO_LAYER,
                                       SCALE_WIDTH(10),
                                       MX_OPTION_TEXT_TYPE_SUFFIX,
                                       NORMAL_FONT_SIZE,
                                       SET_SCHD_ADAPTIVE_CTRL);

    m_elementlist[SET_SCHD_ADAPTIVE_CTRL] = entireDayAdaptiveCheckbox;

	connect (entireDayAdaptiveCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
			 
    connect (entireDayAdaptiveCheckbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(false == (*isEntireDayAndAdaptiveSelect & IS_SET_SCHD_ENTIREDAY_CHECKED))
    {
        entireDayAdaptiveCheckbox->setIsEnabled(false);
    }

    if(*isEntireDayAndAdaptiveSelect & IS_SET_SCHD_ENTIREDAY_ADAPTIVE_CHECKED)
    {
        entireDayAdaptiveCheckbox->changeState(ON_STATE);
    }

    timePeriod = new ElementHeading(entireDay->x (),
                                    entireDay->y () + entireDay->height () + SCALE_HEIGHT(5),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    setScheduleString[SET_SCHD_TIME_PER],
                                    UP_LAYER,
                                    this,false,ELEMENT_MARGIN, NORMAL_FONT_SIZE, false);

    adaptiveCheckboxHeading = new ElementHeading(timePeriod->x () + timePeriod-> width()/4 + SCALE_WIDTH(4),
                                    timePeriod->y(),
                                    BGTILE_MEDIUM_SIZE_WIDTH / 4,
                                    BGTILE_HEIGHT,
                                    setScheduleString[SET_SCHD_ADPT_CHBOX],
                                    NO_LAYER,
                                    this,false,ELEMENT_MARGIN, NORMAL_FONT_SIZE, false);

    startTimeHeading = new ElementHeading(timePeriod->x () + timePeriod-> width()/2,
                                          timePeriod->y (),
                                          BGTILE_MEDIUM_SIZE_WIDTH / 4,
                                          BGTILE_HEIGHT,
                                          setScheduleString[SET_SCHD_START_TIME],
                                          NO_LAYER,
                                          this,false,0, NORMAL_FONT_SIZE, false);

    stopTimeHeading = new ElementHeading (timePeriod->x () + timePeriod-> width() - timePeriod-> width()/4 - SCALE_WIDTH(4),
                                          timePeriod->y () ,
                                          BGTILE_MEDIUM_SIZE_WIDTH /4,
                                          BGTILE_HEIGHT,
                                          setScheduleString[SET_SCHD_END_TIME],
                                          NO_LAYER,
                                          this,
                                          false,0, NORMAL_FONT_SIZE, false);

    /* Hide Adaptive Checkboxes for schedule snaphot */
    if(m_recordType == SET_SCHD_SNAP_SHOT)
    {
        entireDayAdaptiveCheckbox->setVisible(false);
        adaptiveCheckboxHeading->setVisible(false);
    }

    quint8 index ;
    QString hour,min;
    for( index = 0 ; index < MAX_SPINBOX ; index++)
    {
        /* Left margin of 75 given for aligning the numbers below the time period heading */
        slotCountLabel[index] = new ElementHeading (timePeriod-> x(),
                                                    timePeriod->y () +
                                                    (index + 1) * BGTILE_HEIGHT,
                                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                                    BGTILE_HEIGHT,
                                                    QString("%1").arg(index + 1),
                                                    index < 5 ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                                    this,false,60, NORMAL_FONT_SIZE, false,NORMAL_FONT_COLOR);

        /* adaptive checkbox placed below adaptive heading  */
        adaptiveCheckboxes[index] = new OptionSelectButton(adaptiveCheckboxHeading->x () + adaptiveCheckboxHeading->width()/2 - SCALE_WIDTH(20),
                                                           timePeriod->y () +
                                                           (index + 1)*BGTILE_HEIGHT,
                                                           ADAPTIVE_CHECKBOXES_WIDTH,
                                                           BGTILE_HEIGHT,
                                                           CHECK_BUTTON_INDEX,
                                                           "",
                                                           this,
                                                           NO_LAYER,
                                                           SCALE_WIDTH(10),
                                                           MX_OPTION_TEXT_TYPE_SUFFIX,
                                                           NORMAL_FONT_SIZE,
                                                           SET_SCHD_ADAPTIVE_CHECKBOX + index);

        m_elementlist[SET_SCHD_ADAPTIVE_CHECKBOX + index] = adaptiveCheckboxes[index];

		connect (adaptiveCheckboxes[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
				 
        connect (adaptiveCheckboxes[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        if(*isEntireDayAndAdaptiveSelect & (1 << (index + IS_SET_SCHD_ENTIREDAY_ADAPTIVE_CHECKED)))
        {
            adaptiveCheckboxes[index]->changeState(ON_STATE);
        }
        /* Hide Adaptive Checkboxes for schedule snaphot */
        if(m_recordType == SET_SCHD_SNAP_SHOT)
        {
            adaptiveCheckboxes[index]->setVisible(false);
        }

        start_time[index] = new ClockSpinbox(startTimeHeading->x() - SCALE_WIDTH(2),
                                             timePeriod->y () +
                                             (index + 1)*BGTILE_HEIGHT,
                                             TIME_BOX_WIDTH,
                                             BGTILE_HEIGHT,
                                             index + SET_SCHD_START_TIME_CLK_SPINBOX,
                                             CLK_SPINBOX_With_NO_SEC,
                                             "", 6,
                                             this,
                                             "",
                                             false,
                                             0,
                                             NO_LAYER,
                                             true,
                                             5);

        hour = (setScheduleTiming->start_hour[index] < 10 ?
                    QString("0") + QString("%1").arg(setScheduleTiming->start_hour[index])
                  : QString("%1").arg(setScheduleTiming->start_hour[index]));

        min = (setScheduleTiming->start_min[index] < 10 ?
                   QString("0") + QString("%1").arg(setScheduleTiming->start_min[index])
                 : QString("%1").arg(setScheduleTiming->start_min[index]));

        start_time[index]->assignValue (hour,min);

        m_elementlist[ (index*2) + SET_SCHD_START_TIME_CLK_SPINBOX] = start_time[index];

        connect (start_time[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        stop_time[index] = new ClockSpinbox(stopTimeHeading->x() - SCALE_WIDTH(2),
                                            timePeriod->y ()  +
                                            (index + 1 ) * BGTILE_HEIGHT,
                                            TIME_BOX_WIDTH,
                                            BGTILE_HEIGHT,
                                            index + SET_SCHD_END_TIME_CLK_SPINBOX,
                                            CLK_SPINBOX_With_NO_SEC,
                                            "", 6,
                                            this,
                                            "",
                                            false,
                                            0,
                                            NO_LAYER,
                                            true,
                                            5);

        hour = (setScheduleTiming->stop_hour[index] < 10 ?
                    QString("0") + QString("%1").arg(setScheduleTiming->stop_hour[index])
                  : QString("%1").arg(setScheduleTiming->stop_hour[index]));

        min = (setScheduleTiming->stop_min[index] < 10 ?
                   QString("0") + QString("%1").arg(setScheduleTiming->stop_min[index])
                 : QString("%1").arg(setScheduleTiming->stop_min[index]));

        stop_time[index]->assignValue (hour,min);

        m_elementlist[(index*2) + SET_SCHD_END_TIME_CLK_SPINBOX] = stop_time[index] ;

        connect (stop_time [index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));



        if(*isEntireDayAndAdaptiveSelect & IS_SET_SCHD_ENTIREDAY_CHECKED)
        {            
            start_time[index]->setIsEnabled (false);            
            stop_time[index]->setIsEnabled (false);            
            adaptiveCheckboxes[index]->setIsEnabled (false);
        }

    }

    copyToDays = new ElementHeading (entireDay->x (),
                                     start_time[5]->y ()+
                                     BGTILE_HEIGHT + SCALE_HEIGHT(15),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     SCALE_HEIGHT(55),
                                     setScheduleString[SET_SCHD_CPY_TO_CAM],
                                     TOP_LAYER,
                                     this,false,SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    weekDays[0] = new OptionSelectButton (entireDay->x (),
                                          (copyToDays->y () + copyToDays->height ()),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          BOTTOM_TABLE_LAYER,
                                          "", "", SCALE_WIDTH(97),
                                          SET_SCHD_CPY_TO_CHECK_BOX);

    m_elementlist[SET_SCHD_CPY_TO_CHECK_BOX] = weekDays[0] ;

    connect (weekDays[0],
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect (weekDays[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    weekDays[1] = new OptionSelectButton (entireDay->x () + SCALE_WIDTH(150),
                                          copyToDays->y () + copyToDays->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "", "", 0,
                                          (SET_SCHD_CPY_TO_CHECK_BOX + 1));

    m_elementlist[1 + SET_SCHD_CPY_TO_CHECK_BOX] = weekDays[1] ;

    connect (weekDays[1],
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    connect (weekDays[1],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    for(index = 2 ; index < MAX_WEEKDAY_WITH_ALL ; index++)
    {
        weekDays[index] = new OptionSelectButton (weekDays[index -1]->x () + weekDays[index-1]->width () + SCALE_WIDTH(20),
                                                  copyToDays->y () + copyToDays->height (),
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  NO_LAYER,
                                                  "", "", 0,
                                                  (index + SET_SCHD_CPY_TO_CHECK_BOX));

        m_elementlist[index + SET_SCHD_CPY_TO_CHECK_BOX] = weekDays[index] ;

        connect (weekDays[index] ,
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

        connect (weekDays[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

    }

    /* Do not disable the current day checkbox in case of schedule snapshot */
    if(m_recordType == SET_SCHD_REC_FOR)
    {
        weekDays[ (currentDayIndex + 1) ]->changeState (ON_STATE);
        weekDays[ (currentDayIndex + 1) ]->setIsEnabled (false);
    }
    else
    {
        /* Configure copy to checkboxes as per the isCopyToWeekdaysSelect variable in case of schedule snapshot */
        quint8 totalDays = 0;
        for(quint8 index = 0; index <  MAX_WEEKDAYS; index++)
        {
            if(*isCopyToWeekdaysSelect & (1 << index))
            {
                totalDays++;
                weekDays[ (index + 1) ]->changeState (ON_STATE);
            }
            else
            {
                weekDays[ (index + 1) ]->changeState (OFF_STATE);
            }
        }

        if(totalDays == MAX_WEEKDAYS)
        {
            weekDays[0]->changeState (ON_STATE);
        }
    }

    for(quint8 index = 0 ; index < MAX_WEEKDAY_WITH_ALL ;index ++)
    {
        int maxWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(copyToDaysString[index]);
        weekDaysLabel[index] = new TextLabel(entireDay->x () +
                                             SCALE_WIDTH(textLabelOffsets[index]) + SCALE_WIDTH(20),
                                             copyToDays->y () +
                                             copyToDays->height () - SCALE_HEIGHT(15),
                                             SCALE_FONT(14),
                                             copyToDaysString[index],
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_CENTRE_X_CENTER_Y, 0, 0, maxWidth);
    }

    okButton = new CnfgButton (CNFGBUTTON_MEDIAM,
                               backGround->x () + SET_SCHEDULE_WIDTH/2 - SCALE_WIDTH(70),
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)) +
                               (SET_SCHEDULE_HEIGHT - (weekDays[0]->y () + weekDays[0]->height ()  ))/2 +
                               (weekDays[0]->y () + BGTILE_HEIGHT) + SCALE_HEIGHT(40) ,
                               setScheduleString[SET_SCHD_OK],
                               this,
                               SET_SCHD_OK_CTRL,
                               true);

    m_elementlist[SET_SCHD_OK_CTRL] = okButton ;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotOkButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancelButton = new CnfgButton (CNFGBUTTON_MEDIAM,
                                   backGround->x () + SET_SCHEDULE_WIDTH/2 + SCALE_WIDTH(70),
                                   (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)) +
                                   (SET_SCHEDULE_HEIGHT - (weekDays[0]->y () + weekDays[0]->height ()  ))/2 +
                                   (weekDays[0]->y () + BGTILE_HEIGHT) + SCALE_HEIGHT(40) ,
                                   setScheduleString[SET_SCHD_CANCEL],
                                   this,
                                   SET_SCHD_CANCEL_CTRL,
                                   true);

    m_elementlist[SET_SCHD_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCloseButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_elementlist[m_currentElement]->forceActiveFocus ();

    this->show ();
}

SetSchedule::~SetSchedule ()
{
    for(quint8 index = 0 ; index < MAX_WEEKDAY_WITH_ALL ; index++)
    {
        disconnect (weekDays[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

        disconnect (weekDays[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete weekDays[index];
        delete weekDaysLabel[index];
    }
    delete copyToDays;

    for(quint8 index = 0 ; index < MAX_SPINBOX ; index++)
    {

        delete slotCountLabel[index] ;

		disconnect (adaptiveCheckboxes[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
					
        disconnect (adaptiveCheckboxes[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        delete adaptiveCheckboxes[index];

        disconnect (stop_time [index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete stop_time[index];

        disconnect (start_time[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete start_time[index];

    }
    delete stopTimeHeading;
    delete startTimeHeading;
    delete adaptiveCheckboxHeading;
    delete timePeriod;

    disconnect (entireDayAdaptiveCheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    disconnect (entireDayAdaptiveCheckbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete entireDayAdaptiveCheckbox;

    disconnect (entireDay,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    disconnect (entireDay,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete entireDay;
    delete heading;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCloseButtonClick(int)));

    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    delete backGround;
    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;

    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCloseButtonClick(int)));
    delete cancelButton;

    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotOkButtonClick(int)));
    delete okButton;
}

void SetSchedule::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void SetSchedule::takeLeftKeyAction ()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + (MAX_SET_SCHD_CTRL ))
                % (MAX_SET_SCHD_CTRL );
    }while(!m_elementlist[m_currentElement]->getIsEnabled());

    m_elementlist[m_currentElement]->forceActiveFocus();
}

void SetSchedule::takeRightKeyAction ()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % (MAX_SET_SCHD_CTRL );

    }while( !(m_elementlist[m_currentElement]->getIsEnabled()));

    m_elementlist[m_currentElement]->forceActiveFocus();
}

void SetSchedule::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[m_currentElement]  != NULL)
    {
        m_elementlist[m_currentElement]->forceActiveFocus();
    }
}

void SetSchedule::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void SetSchedule::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = SET_SCHD_CLOSE_BTN;
    m_elementlist[m_currentElement]->forceActiveFocus ();
}

bool SetSchedule::timeValidation ()
{
    quint32 startTimeInMin,endTimeInMin;
    quint32 strtTimeInMin,edTimeInMin;

    quint32 currentHour,currentMin;
    quint32 stopHour,stopMin;

    for(quint8 index = 0 ; index < MAX_SPINBOX ; index++)
    {
        setScheduleTiming->start_hour[index] = 0;
        setScheduleTiming->start_min[index] = 0;
        setScheduleTiming->stop_hour[index] = 0;
        setScheduleTiming->stop_min[index] = 0;
    }

    for(quint8 index = 0 ; index < MAX_SPINBOX ; index++)
    {
        start_time[index]->currentValue (currentHour,currentMin);
        stop_time[index]->currentValue (stopHour,stopMin);

        startTimeInMin = currentHour*60 + currentMin ;

        endTimeInMin = stopHour*60 + stopMin;

        // changes to support 23:55 to 00:00 time interval as per change log 44 of v3r4
        if((startTimeInMin >= endTimeInMin)
                && (/*((startTimeInMin != 0) && (endTimeInMin == 0))
                    ||*/ ((startTimeInMin != 0) && (endTimeInMin != 0))))
        {
            if(startTimeInMin == endTimeInMin)
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(FIVE_MIN_DIFF));
                return false;
            }
            else
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GREATER_START_TM));
                return false;
            }
        }
    }

    for(quint8 index = 0 ; index < MAX_SPINBOX ; index++)
    {
        start_time[index]->currentValue (currentHour,currentMin);
        stop_time[index]->currentValue (stopHour,stopMin);

        startTimeInMin = currentHour*60 + currentMin ;

        endTimeInMin = stopHour*60 + stopMin ;

        if(startTimeInMin != endTimeInMin)
        {
            for(quint8 index1 = 0 ; index1 < MAX_SPINBOX ; index1++)
            {
                start_time[index1]->currentValue (currentHour,currentMin);
                stop_time[index1]->currentValue (stopHour,stopMin);

                strtTimeInMin = currentHour*60 + currentMin ;
                edTimeInMin = stopHour*60 + stopMin ;

                if( strtTimeInMin != edTimeInMin )
                {
                    if(index != index1)
                    {
                        if(( (startTimeInMin > strtTimeInMin) && (startTimeInMin < edTimeInMin) ) ||
                                ( (strtTimeInMin > startTimeInMin) && (strtTimeInMin < endTimeInMin) ) ||
                                ( (strtTimeInMin > startTimeInMin) && (endTimeInMin == 0) ) ||
                                (startTimeInMin == strtTimeInMin) )
                        {
                            infoPage->loadInfoPage ( ValidationMessage::getValidationMessage(SCHD_OVRLAP_NT_ALW));
                            return false;
                        }
                    }
                }
            }
        }
        else
        {
            if((startTimeInMin != 0) && (endTimeInMin != 0))
            {
                infoPage->loadInfoPage ( ValidationMessage::getValidationMessage(FIVE_MIN_DIFF));
                return false;
            }
        }
    }

    for(quint8 index = 0 ; index < MAX_SPINBOX ; index++)
    {
        start_time[index]->currentValue (currentHour,currentMin);
        stop_time[index]->currentValue (stopHour,stopMin);

        setScheduleTiming->start_hour[index] = currentHour;
        setScheduleTiming->start_min[index] = currentMin;
        setScheduleTiming->stop_hour[index] = stopHour;
        setScheduleTiming->stop_min[index] = stopMin;
    }

    return true;
}

void SetSchedule::slotCloseButtonClick(int)
{
    emit sigDeleteObject ();
}

void SetSchedule::slotOptionButtonClicked (OPTION_STATE_TYPE_e status, int index)
{
    if (index == SET_SCHD_ADAPTIVE_CTRL)
    {
        /* if flag is enabled */
        if (entireDayAdaptiveCheckbox->getCurrentState () == ON_STATE)
        {
            /* check if any of the adaptive flag is enabled */
            for (quint8 count = 0; count < MAX_ADAPTIVE_CHECKBOX; count++)
            {
                if (adaptiveCheckboxes[count]->getCurrentState () == ON_STATE)
                {
                    /* if any flag is enabled no need to show validation */
                    return;
                }                
            }
            /* show validation if flag is enabled first time */
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ADAPTREC_EN_ADAPT_REC));
        }
        return;
    }

    if ((index >= SET_SCHD_ADAPTIVE_CHECKBOX) && (index < SET_SCHD_START_TIME_CLK_SPINBOX))
    {
        /* if current flag is enabled */
        if (adaptiveCheckboxes[index - SET_SCHD_ADAPTIVE_CHECKBOX]->getCurrentState () == ON_STATE)
        {
            if (entireDayAdaptiveCheckbox->getCurrentState () == ON_STATE)
            {
                /* if entire day adaptive flag is enabled no need to show validation */
                return;
            }

            /* check if any of the adaptive flag is enabled */
            for (quint8 count = 0; count < MAX_ADAPTIVE_CHECKBOX; count++)
            {
                if (count == (index - SET_SCHD_ADAPTIVE_CHECKBOX))
                {
                    /* skip current flag */
                    continue;
                }

                if (adaptiveCheckboxes[count]->getCurrentState () == ON_STATE)
                {
                    /* if any flag is enabled no need to show validation */
                    return;
                }
            }
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ADAPTREC_EN_ADAPT_REC));
        }
        return;
    }

    if(index == SET_SCHD_ENTIRE_DAY_CTRL)
    {
        /* When entire day flag is checked, it disable the time as well as adaptive checkboxes, enables them when entire day is unchecked*/
        for(quint8 index = 0 ;index < MAX_SPINBOX ; index++)
        {
            start_time[index]->setIsEnabled (status == ON_STATE ? false : true);
            stop_time[index]->setIsEnabled  (status == ON_STATE ? false : true);
            adaptiveCheckboxes[index]->setIsEnabled(status == ON_STATE ? false : true);
        }
        /* When entire day flag is not checked, it disable the entireDayAdaptiveCheckbox */
        entireDayAdaptiveCheckbox->setIsEnabled(status == OFF_STATE ? false : true);

        /* if entire day flag changed to OFF, at that time if adaptive was ON and change it to OFF */
        if (status == OFF_STATE)
        {
            entireDayAdaptiveCheckbox->changeState(OFF_STATE);
        }
    }

    /* Checking Unchecking weekday flags according to weekday's "all" checkbox selection */
    if(index == SET_SCHD_CPY_TO_CHECK_BOX)
    {
        for(quint8 index = 1 ; index < MAX_WEEKDAY_WITH_ALL ; index++)
        {
            /* For schedule snapshot, Currentday value always be 0 and hence updated checkbox unconditionally */
            if(m_recordType == SET_SCHD_REC_FOR)
            {
                if(index != (currentDayIndex + 1))
                    weekDays[index]->changeState (status);
            }
            else
            {
                weekDays[index]->changeState (status);
            }
        }
    }

    quint8 tempCount = 0;

    for(quint8 index = 1; index < MAX_WEEKDAY_WITH_ALL; index++)
    {
        if(weekDays[index]->getCurrentState () == ON_STATE)
        {
            tempCount++;
        }
    }

    weekDays[0]->changeState ((tempCount == 7) ? ON_STATE : OFF_STATE);
}

void SetSchedule::slotOkButtonClick (int)
{
    quint8 t_isEnitreDay = 0;
    *isCopyToWeekdaysSelect = 0;
    /* fills the isEntire day value to 0th bit (LSB)*/
    t_isEnitreDay |= ((entireDay->getCurrentState () == ON_STATE) ? 1: 0);
    /* fills the entireDayAdaptiveCheckbox value to 1st bit */
    t_isEnitreDay |= (((entireDayAdaptiveCheckbox->getCurrentState () == ON_STATE) ? 1: 0) << SHIFT_BIT_ONCE);
    /* fills the adaptivecheckbox value from bit 2 to 7(MSB) */
    for(quint8 index = 0 ; index < MAX_ADAPTIVE_CHECKBOX; index++ )
    {
       t_isEnitreDay  |= (((adaptiveCheckboxes[index]->getCurrentState () == ON_STATE) ? 1 : 0) << (index + IS_SET_SCHD_ENTIREDAY_ADAPTIVE_CHECKED));
    }

    /* Do not turn off the page if entire day is not checked on and start time enterd is greater than or equal to stop time */
    if(false == timeValidation())
    {
        return;
    }

    /* store back the value of Entireday and adaptive flags as well as copy to days parameter  */
    if(currentDayIndex !=  MAX_WEEKDAYS)
    {
        *isEntireDayAndAdaptiveSelect = t_isEnitreDay;
    }

    for(quint8 index = 1 ; index < MAX_WEEKDAY_WITH_ALL; index++ )
    {
       *isCopyToWeekdaysSelect |= (((weekDays[index]->getCurrentState () == ON_STATE) ? 1 : 0) << (index-1));
    }
    emit sigDeleteObject();
}

void SetSchedule::slotInfoPageCnfgBtnClick (int)
{
    m_elementlist[m_currentElement]->forceActiveFocus();
}

void SetSchedule::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void SetSchedule::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void SetSchedule::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void SetSchedule::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
