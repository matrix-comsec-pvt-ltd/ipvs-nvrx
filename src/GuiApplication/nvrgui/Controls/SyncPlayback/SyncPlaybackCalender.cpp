#include "SyncPlaybackCalender.h"
#include <QKeyEvent>
#include <QDate>

#define CALENDER_IMG_ICON_SOURCE ":/Images_Nvrx/Calendar/"

#define DAYS_WIDTH              SCALE_WIDTH(36)
#define DAYS_HEIGHT             SCALE_HEIGHT(22)
#define HEADER_ROW_HEIGHT       SCALE_HEIGHT(30)
#define INTER_CONTROL_MARGIN    SCALE_WIDTH(5)
#define WEEKDAY_LEFT_MARGIN     SCALE_WIDTH(24)
#define CALENDER_FONT_SIZE      SCALE_FONT(13)
#define YEAR_MAX_LIMIT          2037
#define YEAR_MIN_LIMIT          2012
#define MAX_MONTH               12

#define DATE_COLOR_HAVING_RECORD    "#b6f935"


static const QString dateArray[MAX_DAYS] = {"", "", "", "", "", "", "",
                                            "", "", "", "", "", "", "",
                                            "", "", "", "", "", "", "",
                                            "", "", "", "", "", "", "",
                                            "", "", "", "", "", "", "",
                                            "", "", "", "", "", "", ""};

static const QString weekdaysArray[MAX_WEEKDAY_COL] = {"Sun",
                                                       "Mon",
                                                       "Tue",
                                                       "Wed",
                                                       "Thu",
                                                       "Fri",
                                                       "Sat"};


static QString monthArray[12];
static const QString imagePath[MAX_DATE_NAVIGATION_IMAGE_TYPE] = {"YearBack/",
                                                                  "MonthBack/",
                                                                  "MonthNext/",
                                                                  "YearNext/"};

SyncPlaybackCalender::SyncPlaybackCalender(quint16 startX,
                                           quint16 startY,
                                           quint16 width,
                                           quint16 height,
                                           quint16 indexInPage,
                                           QWidget* parent,
                                           bool isEnable,
                                           bool catchKey)
    : LayoutWindowRectangle(startX, startY, width, height,BORDER_1_COLOR, parent),
      NavigationControl(indexInPage, isEnable, catchKey),m_dateCollection(0),
      m_currentElement(0)
{
    QString monthsArray[] ={Multilang("January"),
                                        Multilang("February"),
                                        Multilang("March"),
                                        Multilang("April"),
                                        Multilang("May"),
                                        Multilang("June"),
                                        Multilang("July"),
                                        Multilang("August"),
                                        Multilang("September"),
                                        Multilang("October"),
                                        Multilang("November"),
                                        Multilang("December")};
    for(int i=0;i < 12;i++)
    {
        monthArray[i] = monthsArray[i];
    }

    m_monthYearLabel = new TextLabel((this->width() / 2),
                                     (HEADER_ROW_HEIGHT / 2),
                                     NORMAL_FONT_SIZE,
                                     "Month Year",
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_CENTRE_X_CENTER_Y, 0, false, SCALE_WIDTH(150));

    for(quint8 index = 0; index < MAX_DATE_NAVIGATION_IMAGE_TYPE; index++)
    {
        quint16 offset;
        POINT_PARAM_TYPE_e pointParam;
        switch(index)
        {
        case 0:
            offset = SCALE_WIDTH(10);
            pointParam = START_X_CENTER_Y;
            break;
        case 1:
            offset = (m_naviagtionImage[index - 1]->x() + m_naviagtionImage[index - 1]->width() + SCALE_WIDTH(10));
            pointParam = START_X_CENTER_Y;
            break;
        case 2:
            offset = (this->width() - SCALE_WIDTH(10) - m_naviagtionImage[0]->width() - SCALE_WIDTH(10));
            pointParam = END_X_CENTER_Y;
            break;
        default:
            offset = (this->width() - SCALE_WIDTH(10));
            pointParam = END_X_CENTER_Y;
            break;
        }

        m_naviagtionImage[index] = new Image(offset,
                                             (HEADER_ROW_HEIGHT / 2),
                                             QString(CALENDER_IMG_ICON_SOURCE) + imagePath[index],
                                             this,
                                             pointParam,
                                             (SYNCPLAYBACK_CALENDAR_PREV_YEAR + index),
                                             true);
        m_elementList[SYNCPLAYBACK_CALENDAR_PREV_YEAR + index] = m_naviagtionImage[index];
        connect(m_naviagtionImage[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_naviagtionImage[index],
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotChangeCalenderDate(int)));
    }

    for(quint8 index = 0; index < MAX_WEEKDAY_COL; index++)
    {
        m_weekDaysLabel[index] = new TextLabel((WEEKDAY_LEFT_MARGIN + (index * DAYS_WIDTH) + (DAYS_WIDTH / 2)),
                                               (HEADER_ROW_HEIGHT + INTER_CONTROL_MARGIN),
                                               CALENDER_FONT_SIZE,
                                               weekdaysArray[index],
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_CENTRE_X_START_Y);
    }

    quint8 arrayIndex = 0;
    for(quint8 rowIndex = 0; rowIndex < MAX_WEEKDAY_ROW; rowIndex++)
    {
        for(quint8 colIndex = 0; colIndex < MAX_WEEKDAY_COL; colIndex++)
        {
            arrayIndex = (rowIndex * MAX_WEEKDAY_COL) + colIndex;
            m_daysRectangle[arrayIndex] = new RectWithText((WEEKDAY_LEFT_MARGIN + (colIndex * DAYS_WIDTH)),
                                                           (m_weekDaysLabel[colIndex]->y() + DAYS_HEIGHT + (rowIndex * DAYS_HEIGHT)),
                                                           DAYS_WIDTH,
                                                           DAYS_HEIGHT,
                                                           dateArray[arrayIndex],
                                                           this,
                                                           NORMAL_FONT_COLOR,
                                                           CALENDER_FONT_SIZE,
                                                           (SYNCPLAYBACK_CALENDAR_DATE + arrayIndex),
                                                           true);
            m_elementList[SYNCPLAYBACK_CALENDAR_DATE + arrayIndex] = m_daysRectangle[arrayIndex];
            connect(m_daysRectangle[arrayIndex],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
            connect(m_daysRectangle[arrayIndex],
                    SIGNAL(sigMouseClicked(quint32)),
                    this,
                    SLOT(slotDateSelected(quint32)));
        }
    }

    QDate currentDate = QDate::currentDate();
    m_currentDate = currentDate.day();
    m_currentMonth = currentDate.month();
    m_currentYear = currentDate.year();
    m_monthsFirstDay = QDate(m_currentYear, m_currentMonth, 1).dayOfWeek() % MAX_WEEKDAY_COL;
    m_currentDateIndex = changeDateToArrayIndex(m_currentDate);
    changeDateRectText();
    this->setEnabled(isEnable);
    this->setMouseTracking(true);
}

SyncPlaybackCalender::~SyncPlaybackCalender()
{
    for(quint8 index = 0; index < MAX_DAYS; index++)
    {
        disconnect(m_daysRectangle[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_daysRectangle[index],
                   SIGNAL(sigMouseClicked(quint32)),
                   this,
                   SLOT(slotDateSelected(quint32)));
        delete m_daysRectangle[index];
    }

    for(quint8 index = 0; index < MAX_WEEKDAY_COL; index++)
    {
        delete m_weekDaysLabel[index];
    }


    for(quint8 index = 0; index < MAX_DATE_NAVIGATION_IMAGE_TYPE; index++)
    {
        disconnect(m_naviagtionImage[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_naviagtionImage[index],
                   SIGNAL(sigImageClicked(int)),
                   this,
                   SLOT(slotChangeCalenderDate(int)));
        delete m_naviagtionImage[index];
    }

    delete m_monthYearLabel;
}

void SyncPlaybackCalender::checkBoundaryConditions()
{
    m_naviagtionImage[SYNCPLAYBACK_CALENDAR_NEXT_YEAR]->setIsEnabled(true);
    m_naviagtionImage[SYNCPLAYBACK_CALENDAR_NEXT_MONTH]->setIsEnabled(true);
    m_naviagtionImage[SYNCPLAYBACK_CALENDAR_PREV_YEAR]->setIsEnabled(true);
    m_naviagtionImage[SYNCPLAYBACK_CALENDAR_PREV_MONTH]->setIsEnabled(true);
    if(m_currentYear == YEAR_MAX_LIMIT)
    {
        m_naviagtionImage[SYNCPLAYBACK_CALENDAR_NEXT_YEAR]->setIsEnabled(false);
        if(m_currentMonth == MAX_MONTH)
        {
            m_naviagtionImage[SYNCPLAYBACK_CALENDAR_NEXT_MONTH]->setIsEnabled(false);
        }
    }
    if(m_currentYear == YEAR_MIN_LIMIT)
    {
        m_naviagtionImage[SYNCPLAYBACK_CALENDAR_PREV_YEAR]->setIsEnabled(false);
        if(m_currentMonth == 1)
        {
            m_naviagtionImage[SYNCPLAYBACK_CALENDAR_PREV_MONTH]->setIsEnabled(false);
        }
    }
}

void SyncPlaybackCalender::changeDateRectText()
{
    m_daysRectangle[m_currentDateIndex]->deselectRect();
    if(m_daysRectangle[m_currentDateIndex]->getFontColor() != DATE_COLOR_HAVING_RECORD)
    {
        m_daysRectangle[m_currentDateIndex]->changeFontColor(NORMAL_FONT_COLOR);
    }

    QDate tempCurrentDate = QDate(m_currentYear, m_currentMonth, 1);
    if(tempCurrentDate.daysInMonth() <= m_currentDate)
    {
        m_currentDate = tempCurrentDate.daysInMonth();
    }
    QDate currentDate = QDate(m_currentYear, m_currentMonth, m_currentDate);
    m_daysInMonth = currentDate.daysInMonth();
    m_monthYearLabel->changeText(monthArray[m_currentMonth - 1] + " " + QString("%1").arg(m_currentYear), SCALE_WIDTH(150));
    m_monthYearLabel->update();
    m_monthsFirstDay = QDate(m_currentYear, m_currentMonth, 1).dayOfWeek() % MAX_WEEKDAY_COL;
    m_currentDateIndex = changeDateToArrayIndex(m_currentDate);
    for(quint8 index = 0; index < MAX_DAYS; index++)
    {
        if((index < m_monthsFirstDay) || ((index - m_monthsFirstDay) >= m_daysInMonth))
        {
            m_daysRectangle[index]->changeText("");
            m_daysRectangle[index]->setIsEnabled(false);
        }
        else
        {
            m_daysRectangle[index]->changeText(QString("%1").arg(index + 1 - m_monthsFirstDay));
            m_daysRectangle[index]->setIsEnabled(true);
        }
    }

    m_daysRectangle[m_currentDateIndex]->selectRect();
    if(m_daysRectangle[m_currentDateIndex]->getFontColor() != DATE_COLOR_HAVING_RECORD)
        m_daysRectangle[m_currentDateIndex]->changeFontColor(HIGHLITED_FONT_COLOR);
}

void SyncPlaybackCalender::initializeCalender(QString dateString)
{
    m_currentDate = dateString.mid(0, 2).toInt();
    m_currentMonth = dateString.mid(2, 2).toInt();
    m_currentYear = dateString.mid(4, 4).toInt();
    QDate date(m_currentYear, m_currentMonth, m_currentDate);
    m_monthYearString = date.toString("MMyyyy");
    m_dateMonthYearString = date.toString("ddMMyyyy");
    changeDateRectText();
}

QString SyncPlaybackCalender::getMonthYearString()
{
    return m_monthYearString;
}

QString SyncPlaybackCalender::getDateMonthYearString()
{
    return m_dateMonthYearString;
}

void SyncPlaybackCalender::changeDateCollection(quint64 dateCollection)
{
    if(dateCollection != m_dateCollection)
    {
        m_dateCollection = dateCollection;        
        for(quint8 index = 0; index < MAX_DAYS; index++)
        {
            if(changeDateToArrayIndex(m_currentDate) == index)
            {
                m_daysRectangle[index]->changeFontColor(HIGHLITED_FONT_COLOR);
            }
            else
            {
                m_daysRectangle[index]->changeFontColor(NORMAL_FONT_COLOR);
            }
            if(dateCollection == 0)
            {
                m_daysRectangle[index]->update();
            }
        }

        if(dateCollection != 0)
        {
            quint8 tempArray[32];
            for(quint8 index = 0; index < 32; index++)
            {
                tempArray[index] = (dateCollection) & (0x00000001);
                dateCollection = dateCollection >> 1;
            }

            for(quint8 index = 0; index < m_daysInMonth; index++)
            {
                quint8 arrayIndex = index + m_monthsFirstDay;
                if(tempArray[index])
                {
                    m_daysRectangle[arrayIndex]->changeFontColor(DATE_COLOR_HAVING_RECORD);
                }
            }
        }
    }
}

quint8 SyncPlaybackCalender::changeDateToArrayIndex(quint8 date)
{
    return (date + m_monthsFirstDay - 1);
}

quint8 SyncPlaybackCalender::changeArrayIndexToDate(quint8 arrayIndex)
{
    return (arrayIndex - m_monthsFirstDay + 1);
}

void SyncPlaybackCalender::selectNewDate(quint8 newDate)
{
    m_daysRectangle[changeDateToArrayIndex(newDate)]->takeEnterKeyAction();
}

void SyncPlaybackCalender::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (SYNCPLAYBACK_MAX_CALENDAR_ELEMENT);
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


    if((m_currentElement < SYNCPLAYBACK_CALENDAR_DATE)
            || ((m_currentElement >= SYNCPLAYBACK_CALENDAR_DATE)
                && (m_daysRectangle[m_currentElement - SYNCPLAYBACK_CALENDAR_DATE]->getText() != "")))
    {
        if(status == true)
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void SyncPlaybackCalender::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == (SYNCPLAYBACK_MAX_CALENDAR_ELEMENT - 1))
        {
            m_currentElement = -1;
        }

        if(m_currentElement != (SYNCPLAYBACK_MAX_CALENDAR_ELEMENT - 1))
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

    if((m_currentElement < SYNCPLAYBACK_CALENDAR_DATE)
            || ((m_currentElement >= SYNCPLAYBACK_CALENDAR_DATE)
                && (m_daysRectangle[m_currentElement - SYNCPLAYBACK_CALENDAR_DATE]->getText() != "")))
    {
        if(status == true)
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void SyncPlaybackCalender::takeUpKeyAction()
{
    quint32 tempElement = 0;
    if(m_currentElement >= SYNCPLAYBACK_CALENDAR_DATE + MAX_WEEKDAY_COL)
    {
        tempElement = (m_currentElement - MAX_WEEKDAY_COL);
        if(m_daysRectangle[tempElement - SYNCPLAYBACK_CALENDAR_DATE]->getText () != "")
        {
            m_currentElement = tempElement;
            m_elementList[m_currentElement]->forceActiveFocus ();
        }
    }
}

void SyncPlaybackCalender::takeDownKeyAction()
{
    quint32 tempElement = 0;
    if(m_currentElement <= SYNCPLAYBACK_MAX_CALENDAR_ELEMENT - MAX_WEEKDAY_COL)
    {
        tempElement = (m_currentElement + MAX_WEEKDAY_COL);
        if(m_daysRectangle[tempElement - SYNCPLAYBACK_CALENDAR_DATE]->getText () != "")
        {
            m_currentElement = tempElement;
            if((m_currentElement >= SYNCPLAYBACK_CALENDAR_PREV_YEAR) && (m_currentElement < SYNCPLAYBACK_MAX_CALENDAR_ELEMENT))
            {
                m_elementList[m_currentElement]->forceActiveFocus ();
            }
        }
    }
}
void SyncPlaybackCalender::forceFocusToPage(bool isFirstElement)
{
    if(isFirstElement)
    {
        m_currentElement = (SYNCPLAYBACK_CALENDAR_PREV_YEAR - 1);
        takeRightKeyAction();
    }
    else
    {
        m_currentElement = SYNCPLAYBACK_MAX_CALENDAR_ELEMENT;
        takeLeftKeyAction();
    }
}

void SyncPlaybackCalender::setIsEnabled(bool isEnable)
{
    this->setEnabled(isEnable);
    NavigationControl::setIsEnabled(isEnable);
    for(quint8 index = 0; index < SYNCPLAYBACK_MAX_CALENDAR_ELEMENT; index++)
    {
        m_elementList[index]->setIsEnabled(isEnable);
    }

    for(quint8 index = 0; index < MAX_DAYS; index++)
    {
        if(m_daysRectangle[index]->getText() == "")
        {
            m_daysRectangle[index]->setIsEnabled(false);
        }
        else
        {
            m_daysRectangle[index]->setIsEnabled(isEnable);
        }
    }
}

void SyncPlaybackCalender::navigationKeyPressed(QKeyEvent *event)
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
        takeUpKeyAction();
        break;

    case Qt::Key_Down:
        event->accept();
        takeDownKeyAction();
        break;
    }
}

void SyncPlaybackCalender::mouseMoveEvent(QMouseEvent *)
{
    emit sigUpdateCurrentElement(m_indexInPage);
}

void SyncPlaybackCalender::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void SyncPlaybackCalender::slotChangeCalenderDate(int index)
{
    switch(index)
    {
    case SYNCPLAYBACK_CALENDAR_PREV_YEAR:
        m_currentYear--;
        break;
    case SYNCPLAYBACK_CALENDAR_PREV_MONTH:
        m_currentMonth--;
        if(m_currentMonth < 1)
        {
            m_currentMonth = MAX_MONTH;
            m_currentYear--;
        }
        break;
    case SYNCPLAYBACK_CALENDAR_NEXT_MONTH:
        m_currentMonth++;
        if(m_currentMonth > MAX_MONTH)
        {
            m_currentMonth = 1;
            m_currentYear++;
        }
        break;
    case SYNCPLAYBACK_CALENDAR_NEXT_YEAR:
        m_currentYear++;
        break;
    }
//    checkBoundaryConditions();
    changeDateRectText();
    this->update();
    m_monthYearString = QDate(m_currentYear, m_currentMonth, m_currentDate).toString("MMyyyy");
    m_dateMonthYearString = QDate(m_currentYear, m_currentMonth, m_currentDate).toString("ddMMyyyy");
    emit sigFetchNewSelectedDate();
}

void SyncPlaybackCalender::slotDateSelected(quint32 index)
{
    m_daysRectangle[m_currentDateIndex]->deselectRect();
    if(m_daysRectangle[m_currentDateIndex]->getFontColor() != DATE_COLOR_HAVING_RECORD)
    {
        m_daysRectangle[m_currentDateIndex]->changeFontColor(NORMAL_FONT_COLOR);
    }
    m_daysRectangle[m_currentDateIndex]->update();

    m_currentDateIndex = (index - SYNCPLAYBACK_CALENDAR_DATE);
    m_daysRectangle[m_currentDateIndex]->selectRect();
    if(m_daysRectangle[m_currentDateIndex]->getFontColor() != DATE_COLOR_HAVING_RECORD)
        m_daysRectangle[m_currentDateIndex]->changeFontColor(HIGHLITED_FONT_COLOR);

    m_currentDate = changeArrayIndexToDate(m_currentDateIndex);
    m_monthYearString = QDate(m_currentYear, m_currentMonth, m_currentDate).toString("MMyyyy");
    m_dateMonthYearString = QDate(m_currentYear, m_currentMonth, m_currentDate).toString("ddMMyyyy");
    emit sigFetchRecordForNewDate();
}
