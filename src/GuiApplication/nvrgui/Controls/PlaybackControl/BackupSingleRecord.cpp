#include "BackupSingleRecord.h"
#include <QPaintEvent>
#include <QPainter>

#define BKUP_SINGLE_REC_BGRECT_WIDTH        SCALE_WIDTH(526)
#define BKUP_SINGLE_REC_BGRECT_HEIGHT       SCALE_HEIGHT(330)

#define BKUP_LEFT_MARGIN                    SCALE_WIDTH(20)

#define BKUP_STATUS_BAR_RECT_WIDTH        SCALE_WIDTH(480)
#define BKUP_STATUS_BAR_RECT_HEIGHT       SCALE_HEIGHT(48)

#define BKUP_INFO_STRING                   "Records will be stored in .avi format"
#define BKUP_STATUS_STRING                 "Backup Status"

BackupSingleRecord::BackupSingleRecord(PlaybackRecordData *recData, QWidget *parent)
    :KeyBoard(parent)
{
    m_recData = recData;
    this->setGeometry (0,0, parent->width (), parent->height ());

    createDefaultComponent ();
    setPercentage (0);
    fillDateTime ();

    m_currElement = BKUP_SINGLE_REC_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
    this->show ();
}

BackupSingleRecord::~BackupSingleRecord()
{
   delete m_backGroundRect;

    disconnect (m_closeBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    disconnect(m_closeBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    delete m_closeBtn;

    delete m_heading;
    delete m_startDate;

    disconnect (m_startTime,
             SIGNAL(sigTotalCurrentSec(quint16,int)),
             this,
             SLOT(slotClockSpinboxTotalCurrentSec(quint16,int)));
    disconnect(m_startTime,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    delete m_startTime;
    delete m_endDate;

    disconnect (m_endTime,
             SIGNAL(sigTotalCurrentSec(quint16,int)),
             this,
             SLOT(slotClockSpinboxTotalCurrentSec(quint16,int)));
    disconnect(m_endTime,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    delete m_endTime;

    delete m_infoString;

    disconnect (m_startBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    disconnect(m_startBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    delete m_startBtn;

    delete m_BackupLocation;
    delete m_backupStatus;
    delete m_barBackGround;
    delete m_statusRect;
    delete m_percentTextlabel;
}

void BackupSingleRecord::createDefaultComponent()
{
    m_backGroundRect = new Rectangle((this->width () - BKUP_SINGLE_REC_BGRECT_WIDTH)/2,
                                     PB_SEARCH_HEADER_RECT_HEIGHT + (this->height () - BKUP_SINGLE_REC_BGRECT_HEIGHT)/2,
                                     BKUP_SINGLE_REC_BGRECT_WIDTH,
                                     BKUP_SINGLE_REC_BGRECT_HEIGHT,
                                     SCALE_WIDTH(10), BORDER_2_COLOR,
                                     NORMAL_BKG_COLOR, this, 2);

    m_closeBtn = new CloseButtton((m_backGroundRect->x () + m_backGroundRect->width ()),
                                  m_backGroundRect->y (),
                                  SCALE_WIDTH(10),SCALE_HEIGHT(10), this, CLOSE_BTN_TYPE_1,
                                  BKUP_SINGLE_REC_CLOSE_BTN);
    m_elementList[BKUP_SINGLE_REC_CLOSE_BTN] = m_closeBtn;
    connect(m_closeBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_closeBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    m_heading = new TextLabel(m_backGroundRect->x () + m_backGroundRect->width ()/2,
                              m_closeBtn->y () + m_closeBtn->height ()/2,
                              SCALE_FONT(HEADING_FONT_SIZE),
                              "Crop & Backup", this,
                              HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY,
                              ALIGN_CENTRE_X_CENTER_Y);

    m_BackupLocation = new ReadOnlyElement(m_backGroundRect->x () + BKUP_LEFT_MARGIN  ,
                                           (m_backGroundRect->y () +
                                            m_heading->height () + SCALE_HEIGHT(30)),
                                           (BKUP_SINGLE_REC_BGRECT_WIDTH - SCALE_WIDTH(40)),
                                           BGTILE_HEIGHT,
                                           (READONLY_MEDIAM_WIDTH + SCALE_WIDTH(20)),
                                           READONLY_HEIGHT,
                                           "",
                                           this,
                                           COMMON_LAYER,
                                           SCALE_WIDTH(10), SCALE_WIDTH(10),
                                           "Backup Location");

    m_startDate = new CalendarTile(m_backGroundRect->x () + (m_backGroundRect->width () - BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                   m_BackupLocation->y () + m_BackupLocation->height(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT, "",
                                   "Start Time",
                                   this, 0, false,
                                   SCALE_WIDTH(15), COMMON_LAYER, false);

    m_startTime = new ClockSpinbox(m_startDate->x () + SCALE_WIDTH(335),
                                   m_startDate->y (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   BKUP_SINGLE_REC_START_TIME,
                                   CLK_SPINBOX_With_SEC, "", 8, this, "",
                                   false, 0, NO_LAYER, true, 1, true);
    m_elementList[BKUP_SINGLE_REC_START_TIME] = m_startTime;
    connect(m_startTime,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_startTime,
             SIGNAL(sigTotalCurrentSec(quint16,int)),
             this,
             SLOT(slotClockSpinboxTotalCurrentSec(quint16,int)));

    m_endDate = new CalendarTile(m_startDate->x (),
                                 m_startDate->y () + m_startDate->height (),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT, "",
                                 "End Time",
                                 this, 0, false,
                                 SCALE_WIDTH(20), COMMON_LAYER, false);

    m_endTime = new ClockSpinbox(m_endDate->x () + SCALE_WIDTH(335),
                                 m_endDate->y (),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 BKUP_SINGLE_REC_END_TIME,
                                 CLK_SPINBOX_With_SEC, "", 8, this, "",
                                 false, 0, NO_LAYER,  true, 1, true);
    m_elementList[BKUP_SINGLE_REC_END_TIME] = m_endTime;
    connect(m_endTime,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_endTime,
             SIGNAL(sigTotalCurrentSec(quint16,int)),
             this,
             SLOT(slotClockSpinboxTotalCurrentSec(quint16,int)));

    m_backupStatus = new TextLabel(m_backGroundRect->x () + BKUP_LEFT_MARGIN + SCALE_WIDTH(5),
                                   m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(140),
                                   NORMAL_FONT_SIZE,
                                   BKUP_STATUS_STRING,
                                   this, HIGHLITED_FONT_COLOR);

    m_barBackGround = new Rectangle((this->width () - BKUP_STATUS_BAR_RECT_WIDTH)/2,
                                    m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(110),
                                    BKUP_STATUS_BAR_RECT_WIDTH,
                                    BKUP_STATUS_BAR_RECT_HEIGHT,
                                    0, BORDER_2_COLOR, NORMAL_BKG_COLOR,
                                    this, 1);

    m_statusRect = new Rectangle(m_barBackGround->x () + (1),
                                 m_barBackGround->y () + (1),
                                 SCALE_WIDTH(50),
                                 (BKUP_STATUS_BAR_RECT_HEIGHT - (2)),
                                 BLUE_COLOR, this);

    m_percentTextlabel = new TextLabel(m_statusRect->x () + (BKUP_STATUS_BAR_RECT_WIDTH/2),
                                       m_statusRect->y () + m_statusRect->height ()/2,
                                       NORMAL_FONT_SIZE,
                                       "",
                                       this,
                                       BORDER_2_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_CENTER_Y);

    m_infoString = new TextLabel(m_backGroundRect->x () + BKUP_LEFT_MARGIN + SCALE_WIDTH(5),
                                 m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(40),
                                 NORMAL_FONT_SIZE,
                                 BKUP_INFO_STRING,
                                 this, NORMAL_FONT_COLOR,
                                 NORMAL_FONT_FAMILY,
                                 ALIGN_START_X_CENTRE_Y);

    m_startBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (m_backGroundRect->x () + m_backGroundRect->width () - SCALE_WIDTH(70)),
                                (m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(35)),
                                "Export",
                                this,
                                BKUP_SINGLE_REC_START_BTN);

    m_elementList[BKUP_SINGLE_REC_START_BTN] = m_startBtn;
    connect(m_startBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_startBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
}

void BackupSingleRecord::setPercentage(quint8 percent)
{
    m_statusRect->resetGeometry (((BKUP_STATUS_BAR_RECT_WIDTH - SCALE_WIDTH(2)) * percent *100)/10000);
    m_percentTextlabel->changeText (QString("%1 %").arg (percent));
    m_statusRect->update ();
    m_percentTextlabel->update ();
}

void BackupSingleRecord::updateBackUpLocation(QString str)
{
    m_BackupLocation->changeValue (str);
    m_BackupLocation->update ();
}

void BackupSingleRecord::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);
    painter.drawRoundedRect (QRect(0,
                                   0,
                                   PB_SEARCH_HEADER_RECT_WIDTH,
                                   PB_SEARCH_HEADER_RECT_HEIGHT),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);
    painter.drawRoundedRect (QRect(0,
                                   PB_SEARCH_HEADER_RECT_HEIGHT,
                                   PB_SEARCH_MAIN_RECT_WIDTH,
                                   PB_SEARCH_MAIN_RECT_HEIGHT),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void BackupSingleRecord::fillDateTime()
{
    DDMMYY_PARAM_t dateTimeParam;
    quint32 tempToatalSecStart, tempToatalSecEnd;
    QDateTime dateTime = QDateTime::fromString (m_recData->startTime,
                                                "ddMMyyyyHHmmss");
    startDate = dateTime.date ();
    QTime time = dateTime.time ();

    dateTimeParam.date  = startDate.day ();
    dateTimeParam.month = startDate.month ();
    dateTimeParam.year  = startDate.year ();

    m_startDate->setDDMMYY (&dateTimeParam);
    m_startTime->assignValue (time.hour (),
                              time.minute (),
                              time.second ());

    tempToatalSecStart = (time.hour () *3600) + (time.minute () *60) + time.second ();

    dateTime = QDateTime::fromString (m_recData->endTime,
                                      "ddMMyyyyHHmmss");

    endDate = dateTime.date ();
    time = dateTime.time ();

    dateTimeParam.date  = endDate.day ();
    dateTimeParam.month = endDate.month ();
    dateTimeParam.year  = endDate.year ();

    m_endDate->setDDMMYY (&dateTimeParam);
    m_endTime->assignValue (time.hour (),
                            time.minute (),
                            time.second ());

    tempToatalSecEnd = (time.hour () *3600) + (time.minute () *60) + time.second ();

    m_startTime->setMinTotalSeconds (tempToatalSecStart);
    m_startTime->setMaxTotalSeconds (tempToatalSecEnd - 1);

    m_endTime->setMinTotalSeconds (tempToatalSecStart + 1);
    m_endTime->setMaxTotalSeconds (tempToatalSecEnd);
}

void BackupSingleRecord::getDateTime (QString &start, QString &end)
{
    quint32 hour, min, sec;
    m_startTime->currentValue (hour, min, sec);
    QTime time1(hour, min, sec);

    QDateTime datetime1(startDate, time1);
    start = datetime1.toString ("ddMMyyyyHHmmss");

    m_endTime->currentValue (hour, min, sec);
    QTime time2(hour, min, sec);

    QDateTime datetime2(endDate, time2);
    end = datetime2.toString ("ddMMyyyyHHmmss");
}

void BackupSingleRecord::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_BKUP_SINGLE_REC_ELEMETS)
                % MAX_BKUP_SINGLE_REC_ELEMETS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}


void BackupSingleRecord::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1)
                % MAX_BKUP_SINGLE_REC_ELEMETS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void BackupSingleRecord::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BackupSingleRecord::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void BackupSingleRecord::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void BackupSingleRecord::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    emit sigBackUpSingleRecBtnClicked (BKUP_SINGLE_REC_CLOSE_BTN);
}

void BackupSingleRecord::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void BackupSingleRecord::slotClockSpinboxTotalCurrentSec (quint16 currSec, int index)
{
    if(index == BKUP_SINGLE_REC_START_TIME)
    {
        m_endTime->setMinTotalSeconds (currSec + 1);
    }
    else
    {
        m_startTime->setMaxTotalSeconds (currSec - 1);
    }
}

void BackupSingleRecord::slotButtonClick (int index)
{
    if(index == BKUP_SINGLE_REC_START_BTN)
    {
        m_startBtn->setIsEnabled (false);
    }

    emit sigBackUpSingleRecBtnClicked (index);
}
