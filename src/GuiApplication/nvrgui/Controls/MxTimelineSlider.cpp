#include "MxTimelineSlider.h"
#include <QPainter>
#include <QDateTime>

#define MX_TIMELINE_WIDTH       SCALE_WIDTH(960)
#define MX_TIMELINE_HEIGHT      SCALE_HEIGHT(185)
#define MINUTES_PER_SLOT        5
#define MINIMUM_VALUE           (MX_TIMELINE_WIDTH/MAX_SLOT_NUMBER)
#define MAXIMUM_VALUE           SCALE_WIDTH(480)
#define STARTPOINT              0

#define STARTFWDBRKT            SCALE_WIDTH(480)
#define STARTBWDBRKT            SCALE_WIDTH(720)

#define FORWARDTIMESPANBRKT      ":/Images_Nvrx/TimeBarBrkt/ForwardTimeSpanBrkt/"

#define BACKWARDTIMESPANBRKT     ":/Images_Nvrx/TimeBarBrkt/BackwardTimeSpanBrkt/"

#define MILLISECOND              60*60*1000   // 1 hr
#define ONEMINIT                 60           //sec


const quint16 sliderDurationHr[MAX_HOUR_FORMAT] = {1,6,12,24,48,72};

const quint16 minSliderValueMin[MAX_HOUR_FORMAT] = {5,30,60,120,240,360};

const quint16 defaultSliderMin[MAX_HOUR_FORMAT] = {15,90,180,360,720,1080};

const quint16 offSet[MAX_HOUR_FORMAT] = {1, 1, 1, 2, 4, 6};

const quint16 hroffset[MAX_HOUR_FORMAT] = {60,60,60,120,240,360};

MxTimelineSlider::MxTimelineSlider(int xParam,
                                   int yParam,
                                   quint16 indexInPage,
                                   QWidget *parent,
                                   bool isEnable,
                                   bool catchKey)
    :QWidget (parent), NavigationControl(indexInPage, isEnable, catchKey)
{
    m_TimeLineTile = NULL;
    m_TimeLineTile1 = NULL;
    m_TimeLineTile2 = NULL;
    m_TimeLineTile3 = NULL;
    m_TimeLineTile4 = NULL;
    m_currElement = 0;
    m_currentHourUpdate = 0;
    m_tileToolTipHours = 0;
    m_tileToolTipMinutes = 0;
    m_tileToolTipSeconds = 0;
    m_timeDiff = 0;
    m_xBeforeSlide = 0;
    m_timedifference = 0;
    m_totalPixelMove = 0;

    for(quint16 index = 0; index < MAX_SLOT_NUMBER ; index++)
    {
        m_division[index] = NULL;
    }
    m_TimeLineTile5 = NULL;
    m_forwardTimeSpanBrkt = NULL;
    m_backwardTimeSpanBrkt = NULL;
    m_diffBetwnBrkt = NULL;

    m_applController = ApplController::getInstance();
    m_payloadLib = new PayloadLib();

    m_fwdBrktClicked = false;
    m_bwdBrktClicked = false;
    m_clickedInSquare = false;
    m_tileToolTipHide = false;

    m_divisionValue = (qreal)MX_TIMELINE_WIDTH/ (qreal)MAX_SLOT_NUMBER;
    m_oneMinuteValue = (qreal)m_divisionValue/ (qreal)(minSliderValueMin[0]);
    m_pixelPerMinit = m_oneMinuteValue*(qreal)(minSliderValueMin[0]);
    m_currentHourFormat = QUICK_BK_HOUR_1;

    QDateTime localDateTime = QDateTime::currentDateTime();
    updateDateTime(localDateTime);

    this->setGeometry(xParam,
                      yParam,
                      MX_TIMELINE_WIDTH,
                      MX_TIMELINE_HEIGHT);

    m_TimeLineTile = new BgTile(0,
                                SCALE_HEIGHT(25),
                                MX_TIMELINE_WIDTH, //+SCALE_WIDTH(1) ,
                                SCALE_HEIGHT(44),
                                COMMON_LAYER,
                                this);

    m_TimeLineTile1 = new BgTile(m_TimeLineTile->x(),
                                 m_TimeLineTile->y() + m_TimeLineTile->height(),//SCALE_HEIGHT(44),
                                 MX_TIMELINE_WIDTH,// +SCALE_WIDTH(1),
                                 SCALE_HEIGHT(14),
                                 COMMON_LAYER,
                                 this);

    m_TimeLineTile2 = new BgTile(m_TimeLineTile1->x(),
                                 m_TimeLineTile1->y() +  m_TimeLineTile1->height(),//SCALE_HEIGHT(14),
                                 MX_TIMELINE_WIDTH,// +SCALE_WIDTH(1),
                                 SCALE_HEIGHT(6),
                                 BLACK_COLOR_LAYER,
                                 this);

    m_TimeLineTile3 = new BgTile(m_TimeLineTile2->x(),
                                 m_TimeLineTile2->y() +  m_TimeLineTile2->height(),//SCALE_HEIGHT(6),
                                 MX_TIMELINE_WIDTH,// +SCALE_WIDTH(1),
                                 SCALE_HEIGHT(3),
                                 TIMEBAR_LAYER,
                                 this);

    m_TimeLineTile4 = new BgTile(m_TimeLineTile3->x(),
                                 m_TimeLineTile3->y() +  m_TimeLineTile3->height(),//SCALE_HEIGHT(3),
                                 MX_TIMELINE_WIDTH, //+SCALE_WIDTH(1),
                                 SCALE_HEIGHT(6),
                                 BLACK_COLOR_LAYER,
                                 this);

    qreal m_divisionslotvalue = 0;
    for(quint16 index = 0; index < MAX_SLOT_NUMBER ; index++)
    {
        m_division[index] = new Rectangle(m_divisionslotvalue,
                                          m_TimeLineTile1->y() + SCALE_HEIGHT(14),
                                          1,
                                          SCALE_HEIGHT(16),
                                          0,
                                          TIME_BAR_SLOT,
                                          TIME_BAR_SLOT,
                                          this);

        m_divisionslotvalue = m_divisionslotvalue + m_divisionValue;
    }

    m_TimeLineTile5 = new BgTile(m_TimeLineTile4->x(),
                                 m_TimeLineTile4->y() +  m_TimeLineTile4->height(),//SCALE_HEIGHT(6),
                                 MX_TIMELINE_WIDTH,// +SCALE_WIDTH(1),
                                 SCALE_HEIGHT(83),
                                 COMMON_BOTTOM_LAYER,
                                 this);

    INIT_OBJ(m_previousButton);
    m_previousButton = new ControlButton(PREVIOUS_BUTTON_1_INDEX,
                                         SCALE_WIDTH(10),
                                         m_TimeLineTile5->y() + SCALE_HEIGHT(15),
                                         SCALE_WIDTH(40),
                                         SCALE_HEIGHT(29),
                                         this,
                                         NO_LAYER,
                                         0,
                                         "",
                                         true,
                                         QUICK_BK_PAGE_PREVIOUS);

    if(IS_VALID_OBJ(m_previousButton))
    {
        m_elementList[QUICK_BK_PAGE_PREVIOUS] = m_previousButton;
        connect(m_previousButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_previousButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotChangePage(int)));
    }

    INIT_OBJ(m_nextButton);
    m_nextButton = new ControlButton(NEXT_BUTTON_1_INDEX,
                                     MX_TIMELINE_WIDTH - SCALE_WIDTH (40),
                                     m_TimeLineTile5->y() + SCALE_HEIGHT(15),
                                     SCALE_WIDTH(40),
                                     SCALE_HEIGHT(29),
                                     this,
                                     NO_LAYER,
                                     0,
                                     "",
                                     true,
                                     QUICK_BK_PAGE_NEXT);

    if(IS_VALID_OBJ(m_previousButton))
    {
        m_elementList[QUICK_BK_PAGE_NEXT] = m_nextButton;

        connect(m_nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotChangePage(int)));
    }

    INIT_OBJ(m_prevDateTimeLable);
    m_prevDateTimeLable = new TextLabel(m_previousButton->x(),
                                    (m_previousButton->y() + m_previousButton->height()
                                    + SCALE_HEIGHT(10)),
                                    SCALE_FONT(16),
                                    "",
                                    this);

    INIT_OBJ(m_nextDateTimeLable);
    m_nextDateTimeLable = new TextLabel(MX_TIMELINE_WIDTH - SCALE_WIDTH(180),
                                    (m_nextButton->y() + m_nextButton->height()
                                    + SCALE_HEIGHT(10)),
                                    SCALE_FONT(16),
                                    "",
                                    this);

    m_forwardTimeSpanBrkt = new Image(STARTFWDBRKT,
                                      m_TimeLineTile->y() + m_TimeLineTile->height(),
                                      FORWARDTIMESPANBRKT,
                                      this,
                                      START_X_START_Y,
                                      FORWARD_CLOSE_SQUARE,
                                      true,
                                      false,
                                      true,
                                      false);

    m_fwdBrktX = STARTFWDBRKT;

    m_backwardTimeSpanBrkt = new Image(STARTBWDBRKT,
                                       m_TimeLineTile->y() + m_TimeLineTile->height(),
                                       BACKWARDTIMESPANBRKT,
                                       this,
                                       END_X_START_Y,
                                       BACKWARD_CLOSE_SQUARE,
                                       true,
                                       false,
                                       true,
                                       false);

    m_bwdBrktX = STARTBWDBRKT;

    m_diffBetwnBrkt = new Rectangle(STARTFWDBRKT + SCALE_WIDTH(5),
                                    m_TimeLineTile2->y() + SCALE_HEIGHT(5),
                                    SCALE_WIDTH(240) - SCALE_WIDTH(10),
                                    SCALE_HEIGHT(5),
                                    0,
                                    DIFF_BTWN_BRKT_COLOR,
                                    DIFF_BTWN_BRKT_COLOR,
                                    this);

    createDefaultTimeText();

    getDateAndTime(LOCAL_DEVICE_NAME);

    m_timeMoverect.setRect(0,0,MX_TIMELINE_WIDTH,MX_TIMELINE_HEIGHT); //set for tooltip hover effect

    this->setEnabled(true);
    this->setMouseTracking(true);
    this->installEventFilter (this);
    this->show();
}


MxTimelineSlider:: ~MxTimelineSlider()
{
    for(quint8 index = 0; index < (MAX_SLOT_NUMBER + 1); index++)
    {
        if(IS_VALID_OBJ(m_timeText[index]))
        {
            DELETE_OBJ(m_timeText[index]);
        }
    }

    if(IS_VALID_OBJ(m_backwardTimeSpanBrkt))
    {
        DELETE_OBJ(m_backwardTimeSpanBrkt);
    }

    if(IS_VALID_OBJ(m_forwardTimeSpanBrkt))
    {
        DELETE_OBJ(m_forwardTimeSpanBrkt);
    }

    if(IS_VALID_OBJ(m_TimeLineTile5))
    {
        DELETE_OBJ(m_TimeLineTile5);
    }

    for(quint16 index = 0; index < MAX_SLOT_NUMBER ; index++)
    {
        if(IS_VALID_OBJ(m_division[index]))
        {
            DELETE_OBJ(m_division[index]);
        }
    }

    if(IS_VALID_OBJ(m_TimeLineTile4))
    {
        DELETE_OBJ(m_TimeLineTile4);
    }

    if(IS_VALID_OBJ(m_TimeLineTile3))
    {
        DELETE_OBJ(m_TimeLineTile3);
    }

    if(IS_VALID_OBJ(m_TimeLineTile2))
    {
        DELETE_OBJ(m_TimeLineTile2);
    }

    if(IS_VALID_OBJ(m_TimeLineTile1))
    {
        DELETE_OBJ(m_TimeLineTile1);
    }

    if(IS_VALID_OBJ(m_TimeLineTile))
    {
        DELETE_OBJ(m_TimeLineTile);
    }

    if(IS_VALID_OBJ(m_payloadLib))
    {
        DELETE_OBJ(m_payloadLib);
    }

    if(IS_VALID_OBJ(m_diffBetwnBrkt))
    {
        DELETE_OBJ(m_diffBetwnBrkt);
    }

    if(IS_VALID_OBJ(m_nextButton))
    {
        disconnect(m_nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotChangePage(int)));

        DELETE_OBJ(m_nextButton);
    }

    if(IS_VALID_OBJ(m_previousButton))
    {
        disconnect(m_previousButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_previousButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotChangePage(int)));

        DELETE_OBJ(m_previousButton);
    }

    if(IS_VALID_OBJ(m_prevDateTimeLable))
    {
        DELETE_OBJ(m_prevDateTimeLable);
    }

    if(IS_VALID_OBJ(m_nextDateTimeLable))
    {
        DELETE_OBJ(m_nextDateTimeLable);
    }
}

void MxTimelineSlider::createDefaultTimeText()
{
    updateTimeOnTimeLine();

    for(quint8 index = 0; index < (MAX_SLOT_NUMBER + 1); index++)
    {
        if(index == 0)
        {
            m_timeText[index] = new TextLabel((m_TimeLineTile->x() + ((qreal)index * (qreal)m_divisionValue)),
                                              (m_TimeLineTile->y() + SCALE_HEIGHT(25)),
                                              SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                              changeToStandardTime(),
                                              this,
                                              TIME_BAR_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y);

        }
        else if(index == MAX_SLOT_NUMBER)
        {
            m_timeText[index] = new TextLabel((m_TimeLineTile->x() + ((qreal)index * (qreal)m_divisionValue)),
                                              (m_TimeLineTile->y() + SCALE_HEIGHT(25)),
                                              SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                              changeToStandardTime(),
                                              this,
                                              TIME_BAR_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_END_X_CENTRE_Y);
        }
        else
        {
            m_timeText[index] = new TextLabel((m_TimeLineTile->x() + ((qreal)index * (qreal)m_divisionValue)),
                                              (m_TimeLineTile->y() + SCALE_HEIGHT(25)),
                                              SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                              changeToStandardTime(),
                                              this,
                                              TIME_BAR_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_CENTRE_X_CENTER_Y);
        }

        updateHourMinutes();
    }
}

void MxTimelineSlider::updateTimeOnTimeLine()
{
    m_currentUpdateMin = 0;

    for(quint8 m_index = 0; m_index < MAX_SLOT_NUMBER ; m_index++)
    {
        if(m_currentMinute == m_currentUpdateMin)
        {
            m_currentUpdateMin = m_currentMinute;
            break;
        }
        else if((m_currentMinute > m_currentUpdateMin) && (m_currentMinute < (m_currentUpdateMin + 5)))
        {
            break;
        }
        else
        {
            m_currentUpdateMin = m_currentUpdateMin + 5;
            if(m_currentUpdateMin >= 60)
            {
                m_currentUpdateMin = 0;
            }
        }
    }

    m_currentUpdateDisMin = m_currentUpdateMin - 45;
    m_currentUpdateDisHour = m_currentHour;

    if(m_currentUpdateDisMin < 0)
    {
        if(m_currentUpdateDisHour == 0)
        {
            m_currentUpdateDisHour = 24;
        }
        m_currentUpdateDisHour = m_currentUpdateDisHour - 1;
        m_currentUpdateDisMin = 60 + m_currentUpdateDisMin;
    }
}
void MxTimelineSlider::updateDateTime(QString dateTimeString)
{
    m_currDateTime = QDateTime::fromString(dateTimeString,"ddMMyyyyHHmmss");

    m_currentDate = dateTimeString.mid(0, 2).toInt();
    m_currentMonth = dateTimeString.mid(2, 2).toInt();
    m_currentYear = dateTimeString.mid(4, 4).toInt();

    m_currentHour = dateTimeString.mid(8, 2).toInt() ;
    m_currentMinute = dateTimeString.mid(10, 2).toInt();
    m_currentSecond = dateTimeString.mid(12, 2).toInt();

}

void MxTimelineSlider::updateDateTime(QDateTime dateTime)
{
    m_currentDate = dateTime.date().day();
    m_currentMonth = dateTime.date().month();
    m_currentYear = dateTime.date().year();
    m_currentHour = dateTime.time().hour();
    m_currentMinute = dateTime.time().minute();
    m_currentSecond = dateTime.time().second();
}

void MxTimelineSlider::mouseMoveEvent(QMouseEvent *event)
{
    if(((event->x() >= m_TimeLineTile->x()) && (event->x() <= MX_TIMELINE_WIDTH))
        && ((event->y() >= m_TimeLineTile->y()) && (event->y() <= m_TimeLineTile5->y() + SCALE_HEIGHT(5))))
    {
        if(m_fwdBrktClicked)
        {
            calcualtePixelMin(m_bwdBrktX,event->x());
            if(m_timedifference >= m_pixelPerMinit)
            {
                m_forwardTimeSpanBrkt->resetGeometry(event->x(),m_forwardTimeSpanBrkt->y());
                m_forwardTimeSpanBrkt->update();
                m_fwdBrktX = event->x();

                m_diffBetwnBrkt->resetGeometry(m_fwdBrktX + SCALE_WIDTH(5)
                                               ,((m_bwdBrktX- m_fwdBrktX) - SCALE_WIDTH(10.5)) );
                m_tileToolTipHide = false;
                showtiletooltip(event->x(),event->y());    // when press and drag slider then updated timing
                dateTimeLableToolTip();
            }
            else
            {
                m_fwdBrktX = (m_bwdBrktX - m_pixelPerMinit);
                m_forwardTimeSpanBrkt->resetGeometry(m_fwdBrktX,m_forwardTimeSpanBrkt->y());
                m_forwardTimeSpanBrkt->update();

                m_diffBetwnBrkt->resetGeometry(m_fwdBrktX + SCALE_WIDTH(5)
                                               ,((m_bwdBrktX- m_fwdBrktX) - SCALE_WIDTH(10.5)) );
            }
        }
        else if(m_bwdBrktClicked)
        {
            calcualtePixelMin(event->x(),(qreal)m_fwdBrktX);
            if(m_timedifference >= m_pixelPerMinit)
            {
                m_backwardTimeSpanBrkt->resetGeometry(event->x(),m_backwardTimeSpanBrkt->y());
                m_backwardTimeSpanBrkt->update();
                m_bwdBrktX = event->x();
                m_diffBetwnBrkt->resetGeometry(m_fwdBrktX + SCALE_WIDTH(5)
                                               ,((m_bwdBrktX- m_fwdBrktX) - SCALE_WIDTH(10.5)) );

                m_tileToolTipHide = false;
                showtiletooltip(event->x(),event->y());    // when press and drag slider then updated timing
                dateTimeLableToolTip();
            }
            else
            {
                m_bwdBrktX = (m_fwdBrktX + m_pixelPerMinit);
                m_backwardTimeSpanBrkt->resetGeometry(m_bwdBrktX,m_backwardTimeSpanBrkt->y());
                m_backwardTimeSpanBrkt->update();
                m_diffBetwnBrkt->resetGeometry(m_fwdBrktX + SCALE_WIDTH(5)
                                               ,((m_bwdBrktX- m_fwdBrktX) - SCALE_WIDTH(10.5)) );
            }
        }
        else if(m_clickedInSquare)
        {
            updateDateTimeOnMouseMove(event->x());      // change time on timebar when drag cursor
            dateTimeLableToolTip();

        }
        else if(m_forwardTimeSpanBrkt->frameGeometry().contains(event->pos()))
        {
            m_tileToolTipHide = false;
            showtiletooltip(event->x(),event->y());
            dateTimeLableToolTip();
            showSliderToolTip(event->x(),event->y());
        }
        else if(m_backwardTimeSpanBrkt->frameGeometry().contains(event->pos()))
        {
            m_tileToolTipHide = false;
            showtiletooltip(event->x(),event->y());
            dateTimeLableToolTip();
            showSliderToolTip(event->x(),event->y());
        }
        else if(m_timeMoverect.contains(event->pos()))
        {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
            m_tileToolTipHide = true;
            showtiletooltip(event->x(),event->y());
            emit sigSliderToolTipHide(false);
        }
    }
    else
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        m_tileToolTipHide = true;

        emit sigTileToolTipHide(false);
        emit sigSliderToolTipHide(false);
    }
}

void MxTimelineSlider::mousePressEvent(QMouseEvent *event)
{
    if(((event->x() >= m_TimeLineTile->x()) && (event->x() <= MX_TIMELINE_WIDTH))
            && ((event->y() >= m_TimeLineTile->y()) && (event->y() <= m_TimeLineTile5->y() + SCALE_HEIGHT(5))))
    {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        if((m_forwardTimeSpanBrkt->frameGeometry().contains(event->pos()))
                && (event->button() == m_leftMouseButton))
        {
            m_fwdBrktClicked = true;
        }
        else if((m_backwardTimeSpanBrkt->frameGeometry().contains(event->pos()))
                && (event->button() == m_leftMouseButton))
        {
            m_bwdBrktClicked = true;
        }
        else
        {
            m_clickedInSquare = true;
            m_lastClickPoint = QPoint(event->pos());
            m_xBeforeSlide = m_lastClickPoint.x();
            m_totalPixelMove = 0;
        }
        emit sigTileToolTipHide(false);        //For Tile toopTip show-hide
        emit sigSliderToolTipHide(false);
    }
}

void MxTimelineSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if((m_fwdBrktClicked == true) || (m_bwdBrktClicked == true) || (m_clickedInSquare == true))
    {
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
        m_fwdBrktClicked = false;
        m_bwdBrktClicked = false;
        m_clickedInSquare = false;
        emit sigTileToolTipHide(true);

    }

    emit sigTileToolTipHide(false);
    Q_UNUSED(event);
}

bool MxTimelineSlider::eventFilter(QObject *obj, QEvent *event)
{
    if((event->type() == QEvent::Leave))
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        emit sigTileToolTipHide(false);
        emit sigSliderToolTipHide(false);
    }

    return QObject::eventFilter(obj, event);
}

void MxTimelineSlider::updateHourMinutes()   // update 5-5 min every time
{
    m_currentUpdateDisMin = m_currentUpdateDisMin + 5;

    if(m_currentUpdateDisMin >= 60)
    {
        m_currentHour = m_currentHour + 1;
        m_currentUpdateDisMin = m_currentUpdateDisMin - 60;
        if(m_currentHour >= 24)
        {
            m_currentHour = 0;
        }
    }
}
QString MxTimelineSlider::changeToStandardTime()
{
    QString dateTime = INT_TO_QSTRING(m_currentUpdateDisHour).rightJustified(2,'0');
    + ":"
    + INT_TO_QSTRING(m_currentUpdateDisMin).rightJustified(2,'0');

    return dateTime;
}

void MxTimelineSlider::processDeviceResponseforSliderTime(DevCommParam *param, QString deviceName)
{
    if((deviceName == LOCAL_DEVICE_NAME) && (param->cmdType == GET_DATE_TIME))
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            m_payloadLib->parseDevCmdReply(true, param->payload);
            updateDateTime(m_payloadLib->getCnfgArrayAtIndex(0).toString());
            break;

        default:
            break;
        }

//        dateTime = QDateTime(QDate(m_currentYear,m_currentMonth,m_currentDate),
//                             QTime(m_currentHour,m_currentMinute,m_currentSecond));

        defaultSliderDraw(QUICK_BK_HOUR_1);

    }
}

void MxTimelineSlider::getDateAndTime(QString deviceName)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;

    m_applController->processActivity(deviceName, DEVICE_COMM, param);
}

void MxTimelineSlider::updateDateTimeOnMouseMove(int curX)
{
    int temp;
    m_totalPixelMove += curX - m_xBeforeSlide;
    m_xBeforeSlide = curX;

    temp = (m_totalPixelMove < 0) ? (-m_totalPixelMove) : m_totalPixelMove;
    if(temp > m_divisionValue)
    {
        m_currentUpdateDisMin += (m_totalPixelMove > 0) ? (-minSliderValueMin[m_currentHourFormat]) : (minSliderValueMin[m_currentHourFormat]);
        m_startDateTime = (m_totalPixelMove > 0) ? (m_startDateTime.addSecs(-minSliderValueMin[m_currentHourFormat]*60)) : (m_startDateTime.addSecs(minSliderValueMin[m_currentHourFormat]*60));
        m_totalPixelMove += (m_totalPixelMove > 0) ? (-m_divisionValue) : (m_divisionValue);

        if(m_currentUpdateDisMin >= ONEMINIT)
        {
            m_currentUpdateDisMin = m_currentUpdateDisMin % ONEMINIT;
            m_currentUpdateDisHour = m_currentUpdateDisHour + offSet[m_currentHourFormat]; // hr time slot
            if(m_currentUpdateDisHour >= 24)
            {
                m_currentUpdateDisHour = m_currentUpdateDisHour -24;
            }
        }
        if(m_currentUpdateDisMin < 0)
        {
            m_currentUpdateDisMin = hroffset[m_currentHourFormat] + m_currentUpdateDisMin;
            m_currentUpdateDisHour = m_currentUpdateDisHour - offSet[m_currentHourFormat];
            if(m_currentUpdateDisHour < 0)
            {
                m_currentUpdateDisHour = 24 + m_currentUpdateDisHour;
            }
        }
        m_startDateTime.setTime(QTime(m_startDateTime.time().hour(),m_currentUpdateDisMin,m_startDateTime.time().second()));

        changeSliderTime();
    }
}

// for display tileToolTip
void MxTimelineSlider::showtiletooltip(int curX,int curY)
{
    m_oneMinuteValue = ((qreal)m_divisionValue/(qreal)(minSliderValueMin[m_currentHourFormat]));
    m_tileToolTipMinutes =(curX/(qreal)m_oneMinuteValue) ;

    qint64 startTimeLocal = m_startDateTime.toMSecsSinceEpoch();
    QDateTime toolTipTime =(QDateTime::fromMSecsSinceEpoch(startTimeLocal + ((qint64)m_tileToolTipMinutes*60*1000)));

    /* PARASOFT : Rule OWASP2021-A5-c - No need to check errno */
    m_tileToolTipSeconds = (qreal)((fmod((qreal)curX, (qreal)m_oneMinuteValue)) * 60) / (qreal)(m_oneMinuteValue);   // couting display seconds

    m_timeString = INT_TO_QSTRING(toolTipTime.time().hour()).rightJustified(2,'0')
            + ":"
            + INT_TO_QSTRING(toolTipTime.time().minute()).rightJustified(2,'0')
            + ":"
            + INT_TO_QSTRING(m_tileToolTipSeconds).rightJustified(2,'0');

    if(m_tileToolTipHide == true)
    {
        emit sigTileToolTipUpdate(m_timeString,curX,curY);
    }
}

void MxTimelineSlider::showSliderToolTip(int curX,int curY)
{
    QString m_SliderDateTime = Multilang("Start Time") + QString(" : ")
            + m_fwdSliderToolTipDate
            + " "
            + m_fwdTimeString
            + "\n"
            + Multilang("End Time") + QString(" : ")
            + m_bwdSliderToolTipDate
            + " "
            + m_bwdTimeString
            + "\n"
            + Multilang("Duration") + QString(" : ")
            + duration ;

    emit sigTileToolTipHide(false);

    emit sigSliderToolTipUpdate(m_SliderDateTime,curX,curY);
}

void MxTimelineSlider::getDateTime(QDateTime* startDtTime,QDateTime* endDtTime)
{
   *startDtTime = fwdDateTime;
   *endDtTime = bwdDateTime;
}

void MxTimelineSlider::slotUpdateCurrentElement(int index)
{
    m_currElement = index;
}

void MxTimelineSlider::slotChangePage(int index)
{
    qint64 startTimeLocal = m_startDateTime.toMSecsSinceEpoch(); // current date is eual to backwd bar
    if(index == QUICK_BK_PAGE_PREVIOUS)
    {
        m_startDateTime =(QDateTime::fromMSecsSinceEpoch(startTimeLocal -
                                                         ((qint64)sliderDurationHr[m_currentHourFormat]*MILLISECOND)));
    }
    else
    {
        m_startDateTime =(QDateTime::fromMSecsSinceEpoch(startTimeLocal +
                                                       ((qint64)sliderDurationHr[m_currentHourFormat]*MILLISECOND)));
    }

    changeSliderTime();
    dateTimeLableToolTip();
}

void MxTimelineSlider::convertTimeTOEpochTime()
{
    m_oneMinuteValue = (qreal)m_divisionValue/ (qreal)(minSliderValueMin[m_currentHourFormat]);

    qint64 startTimeLocal = m_currDateTime.toMSecsSinceEpoch(); // current date is eual to backwd bar ]

    m_midTime =(QDateTime::fromMSecsSinceEpoch(startTimeLocal -
                                               ((qint64)defaultSliderMin[m_currentHourFormat]*ONEMINIT*1000))); // according to hr format mid time

    m_startDateTime = (QDateTime::fromMSecsSinceEpoch(m_midTime.toMSecsSinceEpoch() -
                                                      ((qint64)(((qreal)sliderDurationHr[m_currentHourFormat]/(qreal)2)*MILLISECOND)))); // according to mid time start slider and end slider slow

    m_currentMinute = m_startDateTime.time().minute();

}

void MxTimelineSlider::updateSliderMin()
{
    m_currentUpdateMin = 0;

    for(quint8 m_index = 0; m_index < MAX_SLOT_NUMBER ; m_index++)
    {
        if(m_currentMinute == m_currentUpdateMin)
        {
            m_currentUpdateMin = m_currentMinute;
            break;
        }
        else if((m_currentMinute > m_currentUpdateMin) &&
                (m_currentMinute < (m_currentUpdateMin + minSliderValueMin[m_currentHourFormat])))
        {
            break;

        }
        else
        {
            m_currentUpdateMin = m_currentUpdateMin + minSliderValueMin[m_currentHourFormat] ;

            if(m_currentUpdateMin >= ONEMINIT) // 60
            {
                m_currentUpdateMin = 0;
            }
        }
    }
    m_startDateTime.setTime(QTime(m_startDateTime.time().hour(),m_currentUpdateMin,m_startDateTime.time().second()));
    m_currentUpdateDisMin = m_startDateTime.time().minute();
    m_currentUpdateDisHour = m_startDateTime.time().hour();
}

void MxTimelineSlider::changeSliderTime()
{
    int tempMin,tempHour;
    tempMin = m_startDateTime.time().minute();
    tempHour = m_startDateTime.time().hour();

    for(quint8 index = 0; index < (MAX_SLOT_NUMBER + 1); index++)
    {
        m_timeText[index]->changeText(INT_TO_QSTRING(tempHour).rightJustified(2,'0')
                                      + ":"
                                      + INT_TO_QSTRING(tempMin).rightJustified(2,'0'));

        if((tempHour == 0) && (tempMin == 0))
        {
            m_timeText[index]->setFontSize(NORMAL_FONT_SIZE);
            m_timeText[index]->SetBold(true);
        }
        else
        {
            m_timeText[index]->setFontSize(SCALE_FONT(SMALL_SUFFIX_FONT_SIZE));
            m_timeText[index]->SetBold(false);
        }

        m_timeText[index]->update();
        tempMin += minSliderValueMin[m_currentHourFormat];  // minimum time slot
        if(tempMin >= ONEMINIT)
        {
            tempMin = tempMin % ONEMINIT;
            tempHour = tempHour + offSet[m_currentHourFormat]; // hr time slot
            if(tempHour >= 24)
            {
                tempHour = tempHour -24;
            }
        }
    }
}

void MxTimelineSlider::drawBkBarAndFwBarOnSlider()
{
    qreal startBkwd = convertTimeToSecMinit(m_currDateTime, m_startDateTime);

    m_backwardTimeSpanBrkt->resetGeometry(startBkwd,m_backwardTimeSpanBrkt->y());
    m_backwardTimeSpanBrkt->update();
    m_bwdBrktX = startBkwd;        // bKBar] position.

    qreal startFrwd = convertTimeToSecMinit(m_midTime, m_startDateTime);
    m_forwardTimeSpanBrkt->resetGeometry(startFrwd,m_forwardTimeSpanBrkt->y());
    m_forwardTimeSpanBrkt->update();
    m_fwdBrktX = startFrwd;         // fWBar [ position

    m_diffBetwnBrkt->resetGeometry(m_fwdBrktX + SCALE_WIDTH(5)
                                   ,((m_bwdBrktX- m_fwdBrktX) - SCALE_WIDTH(10.5)) );

}

// draw BkBar and FwBar according to currentTime and MidTime
qreal MxTimelineSlider::convertTimeToSecMinit(QDateTime startTime,QDateTime startSliderTime)
{
    uint tempCurrentTime = startTime.toTime_t();
    uint tempSliderStart = startSliderTime.toTime_t();
    uint tempDiffSec = (tempCurrentTime - tempSliderStart);
    qreal tempMinut = (qreal)tempDiffSec / (qreal)ONEMINIT;
    qreal pixelPerMinit = (qreal)(MX_TIMELINE_WIDTH/(qreal)(sliderDurationHr[m_currentHourFormat]*ONEMINIT));
    qreal tempDiffMimimum = (qreal)tempMinut*pixelPerMinit;
    return tempDiffMimimum;  // calculate each pixel value so according to FwBar and BkBar draw.
}

// according to FwBar and BkBar Position making toolTip with duration
void MxTimelineSlider::dateTimeLableToolTip()
{
    qint64 startTimeLocal = m_startDateTime.toMSecsSinceEpoch(); // according to slider start time draw fwbar and Bkbar

    qreal bkwdTimeLocalMin = (m_bwdBrktX/(qreal)m_oneMinuteValue);
    QDateTime bkToolTipTime =(QDateTime::fromMSecsSinceEpoch(startTimeLocal +
                                                             ((qint64)(bkwdTimeLocalMin*60*1000))));

    qreal frwdTimeLocalMin = (m_fwdBrktX/(qreal)m_oneMinuteValue) ;   // from position and division value found minit.
    QDateTime fwToolTipTime =(QDateTime::fromMSecsSinceEpoch(startTimeLocal +
                                                             ((qint64)(frwdTimeLocalMin*60*1000))));

    m_currDateTime = bkToolTipTime;  // for hr change then start with bkbar(currentTime) position insted current time
    m_midTime = fwToolTipTime;

    fwdDateTime = fwToolTipTime;
    bwdDateTime = bkToolTipTime;
    QDate fwddate = fwToolTipTime.date();

    m_fwdSliderToolTipDate = INT_TO_QSTRING(fwToolTipTime.date().day()).rightJustified(2,'0')
            + "-"
            + fwddate.shortMonthName(fwddate.month())
            + "-"
            + INT_TO_QSTRING(fwToolTipTime.date().year()).rightJustified(2,'0');

    m_fwdTimeString = INT_TO_QSTRING(fwToolTipTime.time().hour()).rightJustified(2,'0')
            + ":"
            + INT_TO_QSTRING(fwToolTipTime.time().minute()).rightJustified(2,'0')
            + ":"
            + INT_TO_QSTRING(fwToolTipTime.time().second()).rightJustified(2,'0');


    QDate bwddate = bkToolTipTime.date();   // temp for display month in character

    m_bwdSliderToolTipDate = INT_TO_QSTRING(bkToolTipTime.date().day()).rightJustified(2,'0')
            + "-"
            + bwddate.shortMonthName(bwddate.month())
            + "-"
            + INT_TO_QSTRING(bkToolTipTime.date().year()).rightJustified(2,'0');

    m_bwdTimeString = INT_TO_QSTRING(bkToolTipTime.time().hour()).rightJustified(2,'0')
            + ":"
            + INT_TO_QSTRING(bkToolTipTime.time().minute()).rightJustified(2,'0')
            + ":"
            + INT_TO_QSTRING(bkToolTipTime.time().second()).rightJustified(2,'0');

    uint tempDiffSec = (bkToolTipTime.toTime_t() - fwToolTipTime.toTime_t());

    uint tempMinut = tempDiffSec / (uint)ONEMINIT;  // convert to min

    uint tempHour = tempMinut/(uint)ONEMINIT;

    uint temSec = tempDiffSec % ONEMINIT;

    if(tempMinut >= ONEMINIT)
    {
        tempMinut = tempMinut % ONEMINIT;

    }

    duration = INT_TO_QSTRING(tempHour).rightJustified(2,'0')
            +":"
            +INT_TO_QSTRING(tempMinut).rightJustified(2,'0')
            +":"
            +INT_TO_QSTRING(temSec).rightJustified(2,'0');

    m_prevDateTimeLable->changeText(m_fwdSliderToolTipDate + " " +  m_fwdTimeString );

    m_nextDateTimeLable->changeText(m_bwdSliderToolTipDate + " " +  m_bwdTimeString );

    m_prevDateTimeLable->update();
    m_nextDateTimeLable->update();
}

void MxTimelineSlider::calcualtePixelMin(qreal bkwdX,qreal fwrdX)
{
    m_timedifference = ((qreal)bkwdX - (qreal)fwrdX);  // gives diffrence between BkBar and FwBar
}

void MxTimelineSlider::defaultSliderDraw(QUICK_BK_HOURS_FORMAT_e hourFormat)
{
    m_currentHourFormat = hourFormat; // current hr format;

    qreal oneMinuteValue = ((qreal)m_divisionValue/
                        (qreal)(minSliderValueMin[m_currentHourFormat]));

    m_pixelPerMinit = oneMinuteValue*(qreal)(minSliderValueMin[m_currentHourFormat]); // according to hr format

    convertTimeTOEpochTime();  // convert current date time to epoch time

    updateSliderMin();         // update SliderStartMin making round

    changeSliderTime();    // according to startDateTime draw hole slider.

    drawBkBarAndFwBarOnSlider(); // according to currentDateTime ,MidTime draw fwBar and bKbar.

    dateTimeLableToolTip();   // show toolTipLable,prev-next also.

}

void MxTimelineSlider:: PrivateVarAccess(NavigationControl **temp,QUICK_BACKUP_ELE_REF index ) const
{
    switch( index )
    {
        case QUICK_BACKUP_NEXT_BUTTON:
            *temp=m_nextButton;
            break;

        case QUICK_BACKUP_PREV_BUTTON:
            *temp=m_previousButton;
            break;

        default :
            break;
    }
}

