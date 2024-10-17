#include "EventActionSchedule.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>
#include "KeyBoard.h"

#define EVT_SCHD_WIDTH          SCALE_WIDTH(1260)
#define EVT_SCHD_HEIGHT         SCALE_HEIGHT(530)
#define EVTBOX_DIFF             SCALE_WIDTH(80)
#define DEV_ALARM_EVENT_INDX    6

typedef enum
{
    EVNT_SCHD_HEADING,
    EVNT_SCHD_ENTIRE_DAY_LABEL,
    EVNT_SCHD_TIME_PERIOD,
    EVNT_SCHD_START_TIME,
    EVNT_SCHD_END_TIME,
    EVNT_SCHD_APY_ACT_SCHD,
    EVNT_SCHD_OK_LABEL,
    EVNT_SCHD_CANCEL_LABEL,
    MAX_EVNT_SCHD_LABEL
}EVNT_SCHD_LABEL_e;

typedef enum
{
    EVT_SCHD_ENTIRE_DAY,
    EVT_SCHD_ACTION_STATUS_ENTIRE_DAY,
    EVT_SCHD_START_TIME1,
    EVT_SCHD_STOP_TIME1,
    EVT_SCHD_ACTION_STATUS_TIME1,
    EVT_SCHD_START_TIME2,
    EVT_SCHD_STOP_TIME2,
    EVT_SCHD_ACTION_STATUS_TIME2,
    EVT_SCHD_START_TIME3,
    EVT_SCHD_STOP_TIME3,
    EVT_SCHD_ACTION_STATUS_TIME3,
    EVT_SCHD_START_TIME4,
    EVT_SCHD_STOP_TIME4,
    EVT_SCHD_ACTION_STATUS_TIME4,
    EVT_SCHD_START_TIME5,
    EVT_SCHD_STOP_TIME5,
    EVT_SCHD_ACTION_STATUS_TIME5,
    EVT_SCHD_START_TIME6,
    EVT_SCHD_STOP_TIME6,
    EVT_SCHD_ACTION_STATUS_TIME6,
    MAX_EVNT_ACT_FLDS_SCH_MNG
}EVNT_ACT_FLDS_SCH_MNG_e;

static const QString eventActionScheduleStrings[MAX_EVNT_SCHD_LABEL] =
{
    "Event And Action schedule",
    "Entire Day",
    "Time Period",
    "Start Time",
    "End Time",
    "Apply Action Schedule to",
    "OK",
    "Cancel"
};

static qint8 elementMargin[MAX_EVNT_SCHD_EVNT] = {0, 5, 2, 2, 0, -5, -5, 0, 0, -5, 0};

static const QString weekdays[MAX_WEEKDAYS] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static const QString weekdaySort[MAX_WEEKDAYS+1] = {"All", "Sun", "Mon" , "Tue" , "Wed" , "Thu" ,"Fri" ,"Sat" };

static const QString events[MAX_EVNT_SCHD_EVNT] = { "Record", "Image", "Email", "TCP", "SMS", "Preset",
                                                    "Device Alarm", "Camera Alarm", "Buzzer",
                                                    "Video Pop-up", "Push Notification"};

EventActionSchedule::EventActionSchedule(quint32 index,
                                         quint8 dayIndex,
                                         SCHEDULE_TIMING_t *schdRec,
                                         bool *entireDay,
                                         quint32* action,
                                         QWidget *parent,
                                         DEV_TABLE_INFO_t *devTabInfo,
                                         MX_EVNT_ACTION_SCHD_MODE_e eventModeType)
    :  KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    setObjectName("ECT_SCHD");

    initVariable();

    m_index = index;

    for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
    {
        isDaySelectforCopy[index] = false ;
    }

    for(quint8 index = 0; index < MAX_EVNT_SCHD_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    deviceTableInfo = devTabInfo;
    m_currentDayIndex = dayIndex;
    isEntireDaySelected = entireDay;
    camSchdRec = schdRec;
    actionStatus = action;
    m_eventModeType = eventModeType;

    createDefaultElement();

    currElement = EVNT_SCHD_ENTIRE_DAY_CNTRL;
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

EventActionSchedule::~EventActionSchedule()
{
    delete backGround;
    delete m_eleHeadingTile;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    delete heading;

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;

    for(quint8 index = 0; index < MAX_EVNT_SCHD_EVNT ; index++)
    {
        delete m_elementHeadingElide[index];
        delete elementHeading[index];
    }

    disconnect (entireDayCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    disconnect (entireDayCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete entireDayCheckBox;

    for(quint8 index = 0; index < MAX_EVNT_SCHD_EVNT; index++)
    {
        disconnect (entireDayForEventCheckBox[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        disconnect ( entireDayForEventCheckBox[index],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));
        delete entireDayForEventCheckBox[index];
    }

    delete bgTile;

    for(quint8 index = 0;index < MAX_STR_LABEL; index++)
    {
        delete label[index];
    }

    for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        disconnect (start_time[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete start_time[index];
        delete slotLabel[index];

        disconnect ( stop_time[index] ,
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));
        delete stop_time[index];

        disconnect (recordCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete recordCheckBox[index];

        disconnect (imageCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete imageCheckBox[index];

        disconnect (emailCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete emailCheckBox[index];

        disconnect (tcpCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete tcpCheckBox[index];

        disconnect (smsCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete smsCheckBox[index];

        disconnect (ptzCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete ptzCheckBox[index];

        disconnect (alarmCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete alarmCheckBox[index];

        disconnect (camAlarmCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete camAlarmCheckBox[index];

        disconnect (buzzerCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete buzzerCheckBox[index];

        disconnect (m_videoPopUpCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete m_videoPopUpCheckBox[index];

        disconnect (m_pushNotificationCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete m_pushNotificationCheckBox[index];
    }

    delete bgTileBtm;
    delete textLabelBtm;

    for(quint8 index = 0; index < (MAX_WEEKDAYS + 1); index++)
    {
        disconnect ( weekdayCheckBox[index] ,
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));

        disconnect (weekdayCheckBox[index] ,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
        delete weekdayCheckBox[index];
    }

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
}

void EventActionSchedule::initVariable()
{
    INIT_OBJ(m_eleHeadingTile);
    INIT_OBJ(backGround);
    INIT_OBJ(closeButton);
    INIT_OBJ(heading);
    INIT_OBJ(okButton);
    INIT_OBJ(cancelButton);
    INIT_OBJ(entireDayCheckBox);
    INIT_OBJ(infoPage);
    INIT_OBJ(bgTile);
    INIT_OBJ(bgTileBtm);
    INIT_OBJ(textLabelBtm);
    INIT_OBJ(camSchdRec);
    INIT_OBJ(isEntireDaySelected);
    INIT_OBJ(actionStatus);
    INIT_OBJ(deviceTableInfo);
}

void EventActionSchedule::createDefaultElement()
{
    quint8 eventDiff = (m_eventModeType == MAX_MX_EVNT_ACTION_SCHD_MODE) ? EVTBOX_DIFF : (EVTBOX_DIFF - SCALE_WIDTH(5)) ;

    backGround = new Rectangle((SCALE_WIDTH( SETTING_LEFT_PANEL_WIDTH) +
                               SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVT_SCHD_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) +
                               (SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVT_SCHD_HEIGHT)/2),
                               EVT_SCHD_WIDTH,
                               EVT_SCHD_HEIGHT,
                               0,
                               BORDER_2_COLOR,
                               NORMAL_BKG_COLOR,
                               this,1);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    0);

    m_elementlist[EVNT_SCHD_CLS_CTRL] = closeButton;

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(25),
                          eventActionScheduleStrings[EVNT_SCHD_HEADING],
                          this,
                          HEADING_TYPE_2);

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(70),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(40),
                              eventActionScheduleStrings[EVNT_SCHD_OK_LABEL],
                              this,
                              EVNT_SCHD_OK_CTRL);

    m_elementlist[EVNT_SCHD_OK_CTRL] = okButton;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  backGround->x () + backGround->width ()/2 + SCALE_WIDTH(70),
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(40),
                                  eventActionScheduleStrings
                                  [EVNT_SCHD_CANCEL_LABEL],
                                  this,
                                  EVNT_SCHD_CANCEL_CTRL);

    m_elementlist[EVNT_SCHD_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_eleHeadingTile  =  new BgTile(backGround->x () + SCALE_WIDTH(5),
                                    backGround->y () + BGTILE_HEIGHT + SCALE_HEIGHT(10) ,
                                    BGTILE_EXTRALARGE_SIZE_WIDTH,
                                    SCALE_HEIGHT(50),
                                    TOP_LAYER,
                                    this);

    QString fontColor = HIGHLITED_FONT_COLOR;
    QString fontWidth = "" + QString::number(SCALE_WIDTH(13)) +"px";
    QString styl = "ElidedLabel \
    { \
        color: %1; \
        font-size: %2; \
        font-family: %3; \
    }";
    elementHeading[0] = new ElementHeading(backGround->x () + SCALE_WIDTH(415) ,
                                           m_eleHeadingTile->y () + SCALE_HEIGHT(5),
                                           SCALE_WIDTH(60),
                                           BGTILE_HEIGHT,
                                           "",
                                           NO_LAYER,
                                           this,
                                           false,
                                           0,SCALE_WIDTH(12));

    elementHeading[0]->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
    m_elementHeadingElide[0] = new ElidedLabel(Multilang(events[0].toUtf8().constData()), elementHeading[0]);
    m_elementHeadingElide[0]->resize(SCALE_WIDTH(60), SCALE_HEIGHT(40));
    m_elementHeadingElide[0]->raise();
    m_elementHeadingElide[0]->show();

    for(quint8 index = 1; index < MAX_EVNT_SCHD_EVNT; index++)
    {
        quint8 textBoxWidth = 60, xyIndex = (index - 1);
        if (index == EVNT_PUSH_NOTIFICATION)
        {
            textBoxWidth = 80;
            if (m_eventModeType == MAX_MX_EVNT_ACTION_SCHD_MODE)
            {
                xyIndex = (index - 2);
            }
        }

        if ((index == DEV_ALARM_EVENT_INDX) && (deviceTableInfo->alarms == 0) && (deviceTableInfo->sensors == 0))
        {
            fontColor = DISABLE_FONT_COLOR;
        }
        else
        {
            fontColor = HIGHLITED_FONT_COLOR;
        }

        elementHeading[index] = new ElementHeading(elementHeading[xyIndex]->x () + (eventDiff + elementMargin[index]),
                                                   elementHeading[0]->y (),
                                                   SCALE_WIDTH(textBoxWidth),
                                                   BGTILE_HEIGHT,
                                                   "",
                                                   NO_LAYER,
                                                   this,
                                                   false,
                                                   0,SCALE_FONT(12));

        elementHeading[index]->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
        m_elementHeadingElide[index] = new ElidedLabel(Multilang(events[index].toUtf8().constData()), elementHeading[index]);
        m_elementHeadingElide[index]->resize(SCALE_WIDTH(textBoxWidth), SCALE_HEIGHT(40));
        m_elementHeadingElide[index]->raise();
        m_elementHeadingElide[index]->show();
    }

    entireDayCheckBox = new OptionSelectButton(m_eleHeadingTile->x (),
                                               m_eleHeadingTile->y () + BGTILE_HEIGHT,
                                               BGTILE_EXTRALARGE_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               CHECK_BUTTON_INDEX,
                                               this,
                                               MIDDLE_TABLE_LAYER,
                                               eventActionScheduleStrings
                                               [EVNT_SCHD_ENTIRE_DAY_LABEL],
                                               "",
                                               SCALE_WIDTH(30),
                                               EVNT_SCHD_ENTIRE_DAY_CNTRL);

    m_elementlist[EVNT_SCHD_ENTIRE_DAY_CNTRL] = entireDayCheckBox;

    if(isEntireDaySelected[m_currentDayIndex] ==  true)
    {
        entireDayCheckBox->changeState(ON_STATE);
    }

    connect (entireDayCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    connect (entireDayCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    entireDayForEventCheckBox[0] =  new OptionSelectButton(m_eleHeadingTile->x () + SCALE_WIDTH(420),
                                                           m_eleHeadingTile->y () + BGTILE_HEIGHT,
                                                           BGTILE_EXTRALARGE_SIZE_WIDTH,
                                                           BGTILE_HEIGHT,
                                                           CHECK_BUTTON_INDEX,
                                                           this,
                                                           NO_LAYER,"","",0,
                                                           EVNT_SCHD_REC_ENTDAY_CTRL,
                                                           false);

    m_elementlist[EVNT_SCHD_REC_ENTDAY_CTRL] = entireDayForEventCheckBox[0];

    connect (entireDayForEventCheckBox[0],
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    connect ( entireDayForEventCheckBox[0],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 1 ; index < MAX_EVNT_SCHD_EVNT; index++)
    {
        quint8 xyIndex = (index - 1);
        if ((index == EVNT_PUSH_NOTIFICATION) && (m_eventModeType == MAX_MX_EVNT_ACTION_SCHD_MODE))
        {
            xyIndex = (index - 2);
        }

        entireDayForEventCheckBox[index] = new OptionSelectButton(entireDayForEventCheckBox[xyIndex]->x () + eventDiff,
                                                                  entireDayForEventCheckBox[0]->y () ,
                                                                  BGTILE_EXTRALARGE_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  CHECK_BUTTON_INDEX,
                                                                  this,
                                                                  NO_LAYER,"","",0,
                                                                  EVNT_SCHD_REC_ENTDAY_CTRL + index,false);

        m_elementlist[EVNT_SCHD_REC_ENTDAY_CTRL + index] = entireDayForEventCheckBox[index];

        connect (entireDayForEventCheckBox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        connect ( entireDayForEventCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));
    }

    if(isEntireDaySelected[m_currentDayIndex] ==  true)
    {
        for(quint8 index = 0; index < EVNT_VIDEO_POP_UP; index++)
        {
            if(actionStatus[MAX_WEEKDAYS*m_currentDayIndex] & (0x01 << index))
            {
                entireDayForEventCheckBox [EVNT_ALM_REC - index ]->changeState(ON_STATE);
            }
        }

        for(quint8 index = EVNT_VIDEO_POP_UP; index < MAX_EVNT_SCHD_EVNT; index++)
        {
            if(actionStatus[MAX_WEEKDAYS*m_currentDayIndex] & (0x01 << index))
            {
                entireDayForEventCheckBox [index]->changeState(ON_STATE);
            }
        }
    }

    bgTile = new BgTile(m_eleHeadingTile->x (),
                        entireDayForEventCheckBox[0]->y () + BGTILE_HEIGHT,
                        BGTILE_EXTRALARGE_SIZE_WIDTH,
                        BGTILE_HEIGHT,
                        MIDDLE_TABLE_LAYER,
                        this);

    int labelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(eventActionScheduleStrings[EVNT_SCHD_TIME_PERIOD]);
    label[0] = new TextLabel(bgTile->x () + SCALE_WIDTH(30),
                             bgTile->y () + SCALE_HEIGHT(10)  ,
                             NORMAL_FONT_SIZE,
                             eventActionScheduleStrings[EVNT_SCHD_TIME_PERIOD],
                             this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                             0, 0, labelWidth);

    labelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(eventActionScheduleStrings[EVNT_SCHD_START_TIME]);
    label[1] = new TextLabel(bgTile->x () + SCALE_WIDTH(150),
                             bgTile->y () + SCALE_HEIGHT(10)  ,
                             NORMAL_FONT_SIZE,
                             eventActionScheduleStrings[EVNT_SCHD_START_TIME],
                             this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                             0, 0, labelWidth);
    labelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(eventActionScheduleStrings[EVNT_SCHD_END_TIME]);
    label[2] = new TextLabel(bgTile->x () + SCALE_WIDTH(265),
                             bgTile->y () + SCALE_HEIGHT(10)  ,
                             NORMAL_FONT_SIZE,
                             eventActionScheduleStrings[EVNT_SCHD_END_TIME],
                             this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                             0, 0, labelWidth);

    for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        QString hour,min;

        start_time[index] = new ClockSpinbox(bgTile->x () ,
                                             bgTile->y () + (index + 1)*BGTILE_HEIGHT,
                                             BGTILE_EXTRALARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             (EVNT_SCHD_START_TIME_SPINBOX_CTRL + (MAX_CNTRL_ON_ROW * index)),
                                             CLK_SPINBOX_With_NO_SEC,
                                             "", 6, this, "",
                                             false,
                                             SCALE_WIDTH(150),
                                             index < 5 ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                             true,5);

        m_elementlist[EVNT_SCHD_START_TIME_SPINBOX_CTRL + (MAX_CNTRL_ON_ROW * index)] = start_time[index];

        connect ( start_time[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if(isEntireDaySelected[m_currentDayIndex] == false)
        {
            hour = (camSchdRec[m_currentDayIndex].start_hour[index] < 10 ? QString("0") + QString("%1").arg(camSchdRec[m_currentDayIndex].start_hour[index])
                                                                         : QString("%1").arg(camSchdRec[m_currentDayIndex].start_hour[index]));

            min = (camSchdRec[m_currentDayIndex].start_min[index] < 10 ? QString("0") + QString("%1").arg(camSchdRec[m_currentDayIndex].start_min[index])
                                                                       : QString("%1").arg(camSchdRec[m_currentDayIndex].start_min[index]));
        }
        else
        {
            hour = "00" ;
            min = "00";
        }

        start_time[index]->assignValue (hour,min);

        slotLabel[index] = new TextLabel(start_time[index]->x () + SCALE_WIDTH(65),
                                         start_time[index]->y () + SCALE_HEIGHT(10),
                                         NORMAL_FONT_SIZE,
                                         QString("%1").arg (index + 1),
                                         this);

        stop_time[index] = new ClockSpinbox(start_time[index]->x () + SCALE_WIDTH(265),
                                            start_time[index]->y (),
                                            BGTILE_SMALL_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            EVNT_SCHD_END_TIME_SPINBOX_CTRL + (MAX_CNTRL_ON_ROW * index) ,
                                            CLK_SPINBOX_With_NO_SEC,
                                            "", 6, this, "",
                                            false,
                                            0,
                                            NO_LAYER, true, 5);

        m_elementlist[EVNT_SCHD_END_TIME_SPINBOX_CTRL + MAX_CNTRL_ON_ROW*index] = stop_time[index];

        connect ( stop_time[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if(isEntireDaySelected[m_currentDayIndex] == false)
        {
            hour = (camSchdRec[m_currentDayIndex].stop_hour[index] < 10 ? QString("0") + QString("%1").arg(camSchdRec[m_currentDayIndex].stop_hour[index])
                                                                        : QString("%1").arg(camSchdRec[m_currentDayIndex].stop_hour[index]));

            min = (camSchdRec[m_currentDayIndex].stop_min[index] < 10 ? QString("0") + QString("%1").arg(camSchdRec[m_currentDayIndex].stop_min[index])
                                                                      : QString("%1").arg(camSchdRec[m_currentDayIndex].stop_min[index]));
        }
        else
        {
            hour = "00" ;
            min = "00";
        }

        stop_time[index]->assignValue (hour,min);

        recordCheckBox[index] = new OptionSelectButton(stop_time[index]->x () + SCALE_WIDTH(155),
                                                       stop_time[index]->y (),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       CHECK_BUTTON_INDEX,
                                                       this,
                                                       NO_LAYER,"","",0,
                                                       EVNT_SCHD_REC_CTRL + MAX_CNTRL_ON_ROW*index);

        m_elementlist[EVNT_SCHD_REC_CTRL + (MAX_CNTRL_ON_ROW * index)] = recordCheckBox[index];

        connect ( recordCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_ALM_REC)))
        {
            recordCheckBox[index]->changeState(ON_STATE);
        }

        imageCheckBox[index] = new OptionSelectButton(recordCheckBox[index]->x () + eventDiff,
                                                      recordCheckBox[index]->y (),
                                                      BGTILE_SMALL_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      this,
                                                      NO_LAYER,"","",0,
                                                      EVNT_SCHD_IMAGE_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_IMAGE_CTRL + (MAX_CNTRL_ON_ROW * index)] = imageCheckBox[index];

        connect ( imageCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_IMAGE_UPLOAD)))
        {
            imageCheckBox[index]->changeState(ON_STATE);
        }

        emailCheckBox[index] = new OptionSelectButton(imageCheckBox[index]->x () + eventDiff,
                                                      recordCheckBox[index]->y (),
                                                      BGTILE_SMALL_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      this,
                                                      NO_LAYER,"","",0,
                                                      EVNT_SCHD_EMAIL_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_EMAIL_CTRL + (MAX_CNTRL_ON_ROW * index)] = emailCheckBox[index];

        connect ( emailCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_EMAIL)))
        {
            emailCheckBox[index]->changeState(ON_STATE);
        }

        tcpCheckBox[index]  = new OptionSelectButton(emailCheckBox[index]->x () + eventDiff,
                                                     recordCheckBox[index]->y (),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     CHECK_BUTTON_INDEX,
                                                     this,
                                                     NO_LAYER,"","",0,
                                                     EVNT_SCHD_TCP_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_TCP_CTRL + MAX_CNTRL_ON_ROW * index] = tcpCheckBox[index];

        connect ( tcpCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_TCP)))
        {
            tcpCheckBox[index]->changeState(ON_STATE);
        }

        smsCheckBox[index]  = new OptionSelectButton(tcpCheckBox[index]->x () + eventDiff,
                                                     recordCheckBox[index]->y (),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     CHECK_BUTTON_INDEX,
                                                     this,
                                                     NO_LAYER,"","",0,
                                                     EVNT_SCHD_SMS_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_SMS_CTRL + (MAX_CNTRL_ON_ROW * index)] = smsCheckBox[index];

        connect ( smsCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_SMS)))
        {
            smsCheckBox[index]->changeState(ON_STATE);
        }

        ptzCheckBox[index]  = new OptionSelectButton(smsCheckBox[index]->x () + eventDiff,
                                                     recordCheckBox[index]->y (),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     CHECK_BUTTON_INDEX,
                                                     this,
                                                     NO_LAYER,"","",0,
                                                     EVNT_SCHD_PTZ_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_PTZ_CTRL + (MAX_CNTRL_ON_ROW * index)] = ptzCheckBox[index];

        connect ( ptzCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_PTZ)))
        {
            ptzCheckBox[index]->changeState(ON_STATE);
        }

        alarmCheckBox[index] = new OptionSelectButton(ptzCheckBox[index]->x () + eventDiff,
                                                      recordCheckBox[index]->y (),
                                                      BGTILE_SMALL_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      this,
                                                      NO_LAYER,"","",0,
                                                      (EVNT_SCHD_DEVICE_ALARM_CTRL + (MAX_CNTRL_ON_ROW * index)));

        m_elementlist[EVNT_SCHD_DEVICE_ALARM_CTRL + MAX_CNTRL_ON_ROW * index] = alarmCheckBox[index];

        connect ( alarmCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_SYS_DEVICE_ALARM)))
        {
            alarmCheckBox[index]->changeState(ON_STATE);
        }

        camAlarmCheckBox[index] = new OptionSelectButton(alarmCheckBox[index]->x () + eventDiff,
                                                         recordCheckBox[index]->y (),
                                                         BGTILE_SMALL_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         CHECK_BUTTON_INDEX,
                                                         this,
                                                         NO_LAYER,"","",0,
                                                         (EVNT_SCHD_CAMERA_ALARM_CTRL + (MAX_CNTRL_ON_ROW * index)));
        m_elementlist[EVNT_SCHD_CAMERA_ALARM_CTRL + (MAX_CNTRL_ON_ROW * index)] = camAlarmCheckBox[index];
        connect ( camAlarmCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_CAM_ALARM)))
        {
            camAlarmCheckBox[index]->changeState(ON_STATE);
        }

        buzzerCheckBox[index] = new OptionSelectButton(camAlarmCheckBox[index]->x () + eventDiff,
                                                       recordCheckBox[index]->y (),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       CHECK_BUTTON_INDEX,
                                                       this,
                                                       NO_LAYER,"","",0,
                                                       EVNT_SCHD_BUZZER_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_BUZZER_CTRL + (MAX_CNTRL_ON_ROW * index)] = buzzerCheckBox[index];

        connect ( buzzerCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_BUZZER)))
        {
            buzzerCheckBox[index]->changeState(ON_STATE);
        }

        m_videoPopUpCheckBox[index] = new OptionSelectButton(buzzerCheckBox[index]->x () + eventDiff,
                                                             buzzerCheckBox[index]->y (),
                                                             BGTILE_SMALL_SIZE_WIDTH,
                                                             BGTILE_HEIGHT,
                                                             CHECK_BUTTON_INDEX,
                                                             this,
                                                             NO_LAYER,"","",0,
                                                             EVNT_SCHD_VIDEO_POPUP_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_VIDEO_POPUP_CTRL + (MAX_CNTRL_ON_ROW * index)] = m_videoPopUpCheckBox[index];

        connect ( m_videoPopUpCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_VIDEO_POP_UP)))
        {
            m_videoPopUpCheckBox[index]->changeState(ON_STATE);
        }

        m_pushNotificationCheckBox[index] = new OptionSelectButton(((m_eventModeType != MAX_MX_EVNT_ACTION_SCHD_MODE) ?
                                                                   m_videoPopUpCheckBox[index]->x () : buzzerCheckBox[index]->x ()) + eventDiff,
                                                                   (m_eventModeType != MAX_MX_EVNT_ACTION_SCHD_MODE) ?
                                                                   m_videoPopUpCheckBox[index]->y () : buzzerCheckBox[index]->y (),
                                                                   BGTILE_SMALL_SIZE_WIDTH,
                                                                   BGTILE_HEIGHT,
                                                                   CHECK_BUTTON_INDEX,
                                                                   this,
                                                                   NO_LAYER,"","",0,
                                                                   EVNT_SCHD_PUSH_NOTIFICATION_CTRL + (MAX_CNTRL_ON_ROW*index));

        m_elementlist[EVNT_SCHD_PUSH_NOTIFICATION_CTRL + (MAX_CNTRL_ON_ROW * index)] = m_pushNotificationCheckBox[index];

        connect ( m_pushNotificationCheckBox[index] ,
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        if((isEntireDaySelected[m_currentDayIndex] == false) && (actionStatus[index + (MAX_WEEKDAYS*m_currentDayIndex) + 1] & (0x01 << EVNT_PUSH_NOTIFICATION)))
        {
            m_pushNotificationCheckBox[index]->changeState(ON_STATE);
        }
    }

    bgTileBtm = new BgTile(start_time[5]->x (),
                           start_time[5]->y () + BGTILE_HEIGHT + SCALE_HEIGHT(10),
                           BGTILE_EXTRALARGE_SIZE_WIDTH,
                           BGTILE_HEIGHT,
                           COMMON_LAYER,
                           this);

    textLabelBtm = new TextLabel((bgTileBtm->x () + SCALE_WIDTH(150)),
                                 (bgTileBtm->y () + SCALE_HEIGHT(10)),
                                 NORMAL_FONT_SIZE,
                                 eventActionScheduleStrings[EVNT_SCHD_APY_ACT_SCHD],
                                 this);

    weekdayCheckBox[0] = new OptionSelectButton(entireDayForEventCheckBox[EVNT_BUZZER]->x (),
                                                bgTileBtm->y (),
                                                BGTILE_SMALL_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                weekdaySort[0],
                                                this,
                                                NO_LAYER,
                                                0,
                                                MX_OPTION_TEXT_TYPE_SUFFIX,
                                                SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                                EVNT_SCHD_ALL_WEEKDAY_CTRL);

    m_elementlist[EVNT_SCHD_ALL_WEEKDAY_CTRL] = weekdayCheckBox[0];

    connect (weekdayCheckBox[0] ,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (weekdayCheckBox[0] ,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = 1; index < (MAX_WEEKDAYS + 1); index++)
    {
        weekdayCheckBox[index] = new OptionSelectButton(weekdayCheckBox[index-1]->x () + eventDiff,
                                                        bgTileBtm->y (),
                                                        BGTILE_SMALL_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        weekdaySort[index],
                                                        this,
                                                        NO_LAYER,
                                                        0,
                                                        MX_OPTION_TEXT_TYPE_SUFFIX,
                                                        SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                                        (EVNT_SCHD_ALL_WEEKDAY_CTRL  + index),
                                                        ((index-1) != m_currentDayIndex));

        m_elementlist[EVNT_SCHD_ALL_WEEKDAY_CTRL + index] = weekdayCheckBox[index];

        connect (weekdayCheckBox[index] ,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        connect (weekdayCheckBox[index] ,
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        if ((index-1) == m_currentDayIndex)
        {
            weekdayCheckBox[index]->changeState(ON_STATE);
        }
    }

    if(entireDayCheckBox->getCurrentState() == ON_STATE)
    {
        enableControls(false);
    }

    infoPage = new InfoPage (backGround->x (),
                             backGround->y (),
                             backGround->width (),
                             backGround->height (),
                             INFO_PRESET_TOUR_SCHEDULE,
                             this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    #if defined (OEM_JCI)
    elementHeading[EVNT_SMS]->setVisible(false);
    entireDayForEventCheckBox[EVNT_SMS]->setVisible(false);
    entireDayForEventCheckBox[EVNT_SMS]->setIsEnabled(false);
    elementHeading[EVNT_SMS -1]->setVisible(false);
    entireDayForEventCheckBox[EVNT_SMS -1]->setVisible(false);
    entireDayForEventCheckBox[EVNT_SMS -1]->setIsEnabled(false);
    elementMargin[EVNT_SMS + 1] = 1;

    for (quint8 index = EVNT_SMS + 1; index < MAX_EVNT_SCHD_EVNT; index++)
    {

        elementHeading[index]->resetGeometry(elementHeading[index]->x() - 2 * (eventDiff + elementMargin[index]),
                                             elementHeading[index]->y(),
                                             SCALE_WIDTH((index == EVNT_PUSH_NOTIFICATION) ? 80: 60),
                                             BGTILE_HEIGHT);

        entireDayForEventCheckBox[index]->resetGeometry(entireDayForEventCheckBox[index]->x() - (2 * eventDiff),
                                                        entireDayForEventCheckBox[index]->y(),
                                                        BGTILE_EXTRALARGE_SIZE_WIDTH,
                                                        BGTILE_HEIGHT);
    }

    for (quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        smsCheckBox[index]->setVisible(false);
        tcpCheckBox[index]->setVisible(false);
        smsCheckBox[index]->setIsEnabled(false);
        tcpCheckBox[index]->setIsEnabled(false);
        ptzCheckBox[index]->resetGeometry(ptzCheckBox[index]->x() - (2 * eventDiff),
                                          ptzCheckBox[index]->y(),
                                          BGTILE_SMALL_SIZE_WIDTH,
                                          BGTILE_HEIGHT);

        alarmCheckBox[index]->resetGeometry(alarmCheckBox[index]->x() - (2 * eventDiff),
                                            alarmCheckBox[index]->y(),
                                            BGTILE_SMALL_SIZE_WIDTH,
                                            BGTILE_HEIGHT);

        camAlarmCheckBox[index]->resetGeometry(camAlarmCheckBox[index]->x() - (2 * eventDiff),
                                               camAlarmCheckBox[index]->y(),
                                               BGTILE_SMALL_SIZE_WIDTH,
                                               BGTILE_HEIGHT);

        buzzerCheckBox[index]->resetGeometry(buzzerCheckBox[index]->x() - (2 * eventDiff),
                                             buzzerCheckBox[index]->y(),
                                             BGTILE_SMALL_SIZE_WIDTH,
                                             BGTILE_HEIGHT);

        m_videoPopUpCheckBox[index]->resetGeometry(m_videoPopUpCheckBox[index]->x() - (2 * eventDiff),
                                                   m_videoPopUpCheckBox[index]->y(),
                                                   BGTILE_SMALL_SIZE_WIDTH,
                                                   BGTILE_HEIGHT);

        m_pushNotificationCheckBox[index]->resetGeometry(m_pushNotificationCheckBox[index]->x() - (2 * eventDiff),
                                                         m_pushNotificationCheckBox[index]->y(),
                                                         BGTILE_SMALL_SIZE_WIDTH,
                                                         BGTILE_HEIGHT);
    }
    #endif

    updateEventModes();
}

void EventActionSchedule::updateEventModes()
{
    switch (m_eventModeType)
    {
        case MX_VIDEOPOPUP_UPDATE_MODE:
            break;

        case MX_VIDEOPOPUP_DISABLE_MODE:
            entireDayForEventCheckBox [EVNT_VIDEO_POP_UP]->setIsEnabled(false);
            for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
            {
                m_videoPopUpCheckBox[index]->setIsEnabled(false);
            }
            break;

        default:
            elementHeading[EVNT_VIDEO_POP_UP]->setVisible(false);
            entireDayForEventCheckBox [EVNT_VIDEO_POP_UP]->setIsEnabled(false);
            entireDayForEventCheckBox [EVNT_VIDEO_POP_UP]->setVisible(false);
            for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
            {
                m_videoPopUpCheckBox[index]->setIsEnabled(false);
                m_videoPopUpCheckBox[index]->setVisible(false);
            }
            break;
    }

    if( (deviceTableInfo->alarms == 0) && (deviceTableInfo->sensors == 0))
    {
        entireDayForEventCheckBox [DEV_ALARM_EVENT_INDX]->setIsEnabled(false);
        for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
        {
            alarmCheckBox[index]->setIsEnabled(false);
        }
    }
}

void EventActionSchedule::getCurrentActionStatus()
{
    if(entireDayCheckBox->getCurrentState() == ON_STATE)
    {
        isEntireDaySelected[m_currentDayIndex] = true;
        for(quint8 index = 0; index < EVNT_VIDEO_POP_UP; index++)
        {
            if(entireDayForEventCheckBox[index]->getCurrentState() == ON_STATE)
            {
                actionStatus[MAX_WEEKDAYS*m_currentDayIndex] |= (1 << (EVNT_ALM_REC - index)) ;
            }
            else
            {
                actionStatus[MAX_WEEKDAYS*m_currentDayIndex] &= ~(1 << (EVNT_ALM_REC - index));
            }
        }

        for(quint8 index = EVNT_VIDEO_POP_UP; index < MAX_EVNT_SCHD_EVNT; index++)
        {
            if(entireDayForEventCheckBox[index]->getCurrentState() == ON_STATE)
            {
                actionStatus[MAX_WEEKDAYS*m_currentDayIndex] |= (1 << (index)) ;
            }
            else
            {
                actionStatus[MAX_WEEKDAYS*m_currentDayIndex] &= ~(1 << (index));
            }
        }
    }
    else
    {
        isEntireDaySelected[m_currentDayIndex] = false;
        for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
        {
            if(recordCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_ALM_REC);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_ALM_REC));

            if(imageCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_IMAGE_UPLOAD);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_IMAGE_UPLOAD));

            if(emailCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_EMAIL);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_EMAIL));

            if(tcpCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_TCP);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_TCP));

            if(smsCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_SMS);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_SMS));

            if(ptzCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_PTZ);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_PTZ));

            if(alarmCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_SYS_DEVICE_ALARM);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_SYS_DEVICE_ALARM));

            if(camAlarmCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_CAM_ALARM);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_CAM_ALARM));

            if(buzzerCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_BUZZER);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_BUZZER));

            if(m_videoPopUpCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_VIDEO_POP_UP);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_VIDEO_POP_UP));

            if(m_pushNotificationCheckBox[index]->getCurrentState() == ON_STATE)
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] |= 1 << EVNT_PUSH_NOTIFICATION);
            else
                (actionStatus[index + MAX_WEEKDAYS*m_currentDayIndex + 1] &= ~(1 << EVNT_PUSH_NOTIFICATION));
        }
    }

    if(weekdayCheckBox[0]->getCurrentState() == ON_STATE)
    {
        for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
        {
            isDaySelectforCopy[index] = true ;
        }
    }
    else
    {
        for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
        {
            isDaySelectforCopy[index] = weekdayCheckBox[index + 1]->getCurrentState() == ON_STATE ? true : false ;
        }
    }

    for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
    {
        if(index == m_currentDayIndex)
        {
            continue;
        }

        if(isDaySelectforCopy[index] == false)
        {
            continue;
        }

        if(isEntireDaySelected[m_currentDayIndex])
        {
            isEntireDaySelected[index] = true;
        }
        else
        {
            memcpy(&camSchdRec[index], &camSchdRec[m_currentDayIndex], sizeof(SCHEDULE_TIMING_t));
            isEntireDaySelected[index] = false;
        }

        for(quint8 index1 = 0; index1 < MAX_WEEKDAYS; index1++)
        {
            actionStatus[MAX_WEEKDAYS*index + index1] = actionStatus[index1 + MAX_WEEKDAYS*m_currentDayIndex];
        }
    }
}

bool EventActionSchedule::timeValidation ()
{
    quint32 startTimeInMin,endTimeInMin;

    quint32 currentHour,currentMin;
    quint32 stopHour,stopMin;

    for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        start_time[index]->currentValue (currentHour,currentMin);
        stop_time[index]->currentValue (stopHour,stopMin);

        startTimeInMin = currentHour*60 + currentMin;
        endTimeInMin = stopHour*60 + stopMin ;

        // changes to support 23:55 to 00:00 time interval as per change log 44 of v3r4
        if((startTimeInMin >= endTimeInMin) && (((startTimeInMin != 0) && (endTimeInMin != 0))))
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

    for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        start_time[index]->currentValue (currentHour,currentMin);
        stop_time[index]->currentValue (stopHour,stopMin);

        camSchdRec[m_currentDayIndex].start_hour[index] = currentHour;
        camSchdRec[m_currentDayIndex].start_min[index] = currentMin;
        camSchdRec[m_currentDayIndex].stop_hour[index] = stopHour;
        camSchdRec[m_currentDayIndex].stop_min[index] = stopMin;
    }

    return true;
}

void EventActionSchedule::paintEvent (QPaintEvent *)
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

void EventActionSchedule::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_EVNT_SCHD_CTRL) % MAX_EVNT_SCHD_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionSchedule::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_EVNT_SCHD_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionSchedule::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionSchedule::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventActionSchedule::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void EventActionSchedule::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventActionSchedule::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVNT_SCHD_CLS_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();
}

void EventActionSchedule::enableControls(bool status)
{
    for(quint8 index = 0; index < MAX_EVNT_SCHD_EVNT; index++)
    {
        if (index == EVNT_VIDEO_POP_UP)
        {
            if(m_eventModeType == MX_VIDEOPOPUP_UPDATE_MODE)
            {
                entireDayForEventCheckBox[index]->setIsEnabled(!status);
            }
        }
        else if(index == DEV_ALARM_EVENT_INDX)
        {
            if(deviceTableInfo->alarms != 0)
            {
                entireDayForEventCheckBox[index]->setIsEnabled(!status);
            }
        }
        else
        {
            entireDayForEventCheckBox[index]->setIsEnabled(!status);
        }
    }

    for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        recordCheckBox[index]->setIsEnabled(status);
        imageCheckBox[index]->setIsEnabled(status);
        emailCheckBox[index]->setIsEnabled(status);
        tcpCheckBox[index]->setIsEnabled(status);
        smsCheckBox[index]->setIsEnabled(status);
        ptzCheckBox[index]->setIsEnabled(status);
        if(deviceTableInfo->alarms != 0)
        {
            alarmCheckBox[index]->setIsEnabled(status);
        }
        camAlarmCheckBox[index]->setIsEnabled(status);
        buzzerCheckBox[index]->setIsEnabled(status);
        if(m_eventModeType == MX_VIDEOPOPUP_UPDATE_MODE)
        {
            m_videoPopUpCheckBox[index]->setIsEnabled(status);
        }
        m_pushNotificationCheckBox[index]->setIsEnabled(status);
        start_time[index]->setIsEnabled(status);
        stop_time[index]->setIsEnabled(status);
    }
}

void EventActionSchedule::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventActionSchedule::slotButtonClick (int index)
{
    if(index == EVNT_SCHD_OK_CTRL)
    {
        if(timeValidation())
        {
            getCurrentActionStatus();
            emit sigDeleteObject(m_index);
        }
    }
    else if((index == EVNT_SCHD_CLS_CTRL) || (index == EVNT_SCHD_CANCEL_CTRL))
    {
        emit sigDeleteObject(m_index);
    }
}

void EventActionSchedule::slotCheckBoxClicked(OPTION_STATE_TYPE_e state, int index)
{
    if(index == EVNT_SCHD_ENTIRE_DAY_CNTRL)
    {
        enableControls(!state);
        if(state == OFF_STATE)
        {
            for(quint8 index = 0; index < MAX_EVNT_SCHD_EVNT ; index++)
            {
                if (index == EVNT_VIDEO_POP_UP)
                {
                    if(m_eventModeType == MX_VIDEOPOPUP_UPDATE_MODE)
                    {
                        entireDayForEventCheckBox[index]->changeState(OFF_STATE);
                    }
                }
                else if (index == DEV_ALARM_EVENT_INDX)
                {
                    if(deviceTableInfo->alarms != 0)
                    {
                        entireDayForEventCheckBox[index]->changeState(OFF_STATE);
                    }
                }
                else
                {
                    entireDayForEventCheckBox[index]->changeState(OFF_STATE);
                }
            }
        }
        else
        {
            for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
            {
                recordCheckBox[index]->changeState(OFF_STATE);
                imageCheckBox[index]->changeState(OFF_STATE);
                emailCheckBox[index]->changeState(OFF_STATE);
                tcpCheckBox[index]->changeState(OFF_STATE);
                smsCheckBox[index]->changeState(OFF_STATE);
                ptzCheckBox[index]->changeState(OFF_STATE);
                if(deviceTableInfo->alarms != 0)
                {
                    alarmCheckBox[index]->changeState(OFF_STATE);
                }
                camAlarmCheckBox[index]->changeState(OFF_STATE);
                buzzerCheckBox[index]->changeState(OFF_STATE);
                if(m_eventModeType == MX_VIDEOPOPUP_UPDATE_MODE)
                {
                    m_videoPopUpCheckBox[index]->changeState(OFF_STATE);
                }
                m_pushNotificationCheckBox[index]->changeState(OFF_STATE);
            }
        }

        #if defined (OEM_JCI)
        entireDayForEventCheckBox[EVNT_SMS]->setIsEnabled(false);
        entireDayForEventCheckBox[EVNT_SMS -1]->setIsEnabled(false);
        for (quint8 index = 0; index < MAX_TIME_SLOT; index++)
        {
            smsCheckBox[index]->setIsEnabled(false);
            tcpCheckBox[index]->setIsEnabled(false);
        }
        #endif
    }

    if(index == EVNT_SCHD_ALL_WEEKDAY_CTRL)
    {
        for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
        {
            if (index != m_currentDayIndex)
            {
                weekdayCheckBox[index+1]->changeState(state);
            }
        }
    }

    if((EVNT_SCHD_ALL_WEEKDAY_CTRL < index) && (index < EVNT_SCHD_OK_CTRL))
    {
        quint8 totalDays = 0;
        for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
        {
            if(weekdayCheckBox[index+1]->getCurrentState() == ON_STATE)
            {
                totalDays++;
            }
        }
        weekdayCheckBox[0]->changeState((totalDays == MAX_WEEKDAYS) ? ON_STATE : OFF_STATE);
    }
}

void EventActionSchedule:: slotInfoPageBtnclick (int)
{
    m_elementlist[currElement]->forceActiveFocus ();
}

void EventActionSchedule::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
