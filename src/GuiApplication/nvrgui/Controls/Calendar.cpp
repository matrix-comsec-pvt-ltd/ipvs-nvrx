#include "Calendar.h"
#include "EnumFile.h"
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>

#define CAL_DATECELL_WIDTH      SCALE_WIDTH(36) //34
#define CAL_DATECELL_HEIGHT     SCALE_HEIGHT(22) //20


#define CAL_CLOSEBTN_PATH       IMAGE_PATH"ControlButtons/CloseButtons/ButtonType_6/"
#define CAL_MONTHBACK_PATH      IMAGE_PATH"Calendar/MonthBack/"
#define CAL_MONTHNEXT_PATH      IMAGE_PATH"Calendar/MonthNext/"
#define CAL_YEARBACK_PATH       IMAGE_PATH"Calendar/YearBack/"
#define CAL_YEARNEXT_PATH       IMAGE_PATH"Calendar/YearNext/"

static const QString monthArray[] =
{
    "",
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

static const QString weekdaysArray[CAL_MAX_COL] =
{"Sun", "Mon", "Tue", "Wed","Thu", "Fri", "Sat"};

//static const QString dateArray[CAL_MAX_DAY_INDEX] =
//{   " ", " ", " ", " ", " ", " ", "1",
//    "2", "3", "4", "5", "6", "7", "8",
//    "9", "10", "11", "12", "13", "14", "15",
//    "16", "17", "18", "19", "20", "21", "22",
//    "23", "24", "25", "26", "27", "28", "29",
//    "30", "31", " ", " ", " ", " ", " "
//};
static const QString dateArray[CAL_MAX_DAY_INDEX] =
{   " ", " ", " ", " ", " ", " ", " ",
    "", "", "", " ", " ", " ", " ",
    "", "", "", " ", " ", " ", " ",
    "", "", "", " ", " ", " ", " ",
    "", "", "", " ", " ", " ", " ",
    "", "", " ", " ", " ", " ", " "
};

//*****************************************************************************
// Calendar
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
Calendar::Calendar(quint16 startX,
                   quint16 startY,
                   quint16 width,
                   quint16 height,
                   QWidget *parent) :
    BgTile(startX,
           startY,
           width,
           height, COMMON_LAYER,
           parent),
    bgWidth(width), bgHeight(height)

{
    isCloseBtnIn = true;

    this->setMouseTracking(true);
    this->setEnabled(true);

//    m_dateParam.date = 1;
//    m_dateParam.month = 1;
//    m_dateParam.year = 2000;

    createDefaultComponent ();
    m_currElement = CALENDAR_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
//    intializeCal ();

    this->show ();
}
//*****************************************************************************
// ~Calendar()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
Calendar::~Calendar()
{
    if(isCloseBtnIn == true)
    {
        disconnect (closeBtn,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrElement(int)));
        disconnect (closeBtn,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotImagesClicked(int)));
        delete closeBtn;
    }

    for(quint8 index = 0; index < 4; index++)
    {
        disconnect (m_images[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrElement(int)));
        disconnect (m_images[index],
                    SIGNAL(sigImageClicked(int)),
                    this,
                    SLOT(slotImagesClicked(int)));

        delete m_images[index];
    }

    delete monthYearTextLabel;
    delete midline;

    for(quint8 index = 0; index < CAL_MAX_COL; index++)
    {
        delete daysStr[index];
    }

    for(quint8 row = 0; row < CAL_MAX_ROW; row++)
    {
        for(quint8 col = 0; col <CAL_MAX_COL ; col++)
        {
            disconnect (dateOfMonth[col + (row * CAL_MAX_COL)],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrElement(int)));

            disconnect (dateOfMonth[col + (row * CAL_MAX_COL)],
                     SIGNAL(sigMouseClicked(quint32)),
                     this,
                     SLOT(slotDateBoxClicked(quint32)));

            delete dateOfMonth[col + (row * CAL_MAX_COL)];
        }
    }
}
//*****************************************************************************
// TextBox
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void Calendar::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent (event);
}

void Calendar::setDDMMYY(DDMMYY_PARAM_t *param)
{
    m_dateParam.date = param->date;
    m_dateParam.month = param->month;
    m_dateParam.year = param->year;
    intializeCal();
    //    update ();
}

//*****************************************************************************
// TextBox
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void Calendar::createDefaultComponent()
{
    if(isCloseBtnIn == true)
    {
        closeBtn = new CloseButtton(this->width (),
                                    0,
                                    0, 0, this,
                                    CLOSE_BTN_TYPE_6,
                                    CALENDAR_CLOSE_BTN);

        m_elementList[CALENDAR_CLOSE_BTN] = closeBtn;

        connect (closeBtn,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotImagesClicked(int)));
        connect (closeBtn,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrElement(int)));
    }

    m_images[0] = new Image(SCALE_WIDTH(25), SCALE_HEIGHT(15),
                            CAL_YEARBACK_PATH, this,
                            START_X_START_Y, CALENDAR_YEARBACK, true);

    m_images[1] = new Image((m_images[0]->x () + m_images[0]->width () + SCALE_WIDTH(5)),
                            SCALE_HEIGHT(15),
                            CAL_MONTHBACK_PATH, this,
                            START_X_START_Y, CALENDAR_MONTHBACK, true);

    m_images[3] = new Image((this->width () - SCALE_WIDTH(25)),
                            SCALE_HEIGHT(15) + m_images[0]->height (),
                            CAL_YEARNEXT_PATH, this,
                            END_X_END_Y, CALENDAR_YEARNEXT, true);

    m_images[2] = new Image(m_images[3]->x () - SCALE_WIDTH(5),
                            SCALE_HEIGHT(15) + m_images[1]->height (),
                            CAL_MONTHNEXT_PATH, this,
                            END_X_END_Y, CALENDAR_MONTHNEXT, true);

    for(quint8 index = 0; index < 4; index++)
    {
        m_elementList[CALENDAR_YEARBACK + index] = m_images[index];
        connect (m_images[index],
                 SIGNAL(sigImageClicked(int)),
                 this,
                 SLOT(slotImagesClicked(int)));

        connect (m_images[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrElement(int)));
    }


    QString tempImagePath = monthArray[m_dateParam.month] + ", " + QString("%1").arg (m_dateParam.year);

    monthYearTextLabel = new TextLabel((this->width ())/2,
                                       SCALE_HEIGHT(15), SCALE_WIDTH(14), tempImagePath, this,
                                       HIGHLITED_FONT_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_START_Y);

    midline = new Rectangle(SCALE_WIDTH(10),
                            monthYearTextLabel->y () + monthYearTextLabel->height () + SCALE_HEIGHT(2),
                            (this->width () - SCALE_WIDTH(20)),
                            1, 0, BORDER_1_COLOR,
                            BORDER_1_COLOR, this);

    for(quint8 index = 0; index < CAL_MAX_COL; index++)
    {

        daysStr[index] = new RectWithText((SCALE_WIDTH(12) + (index *CAL_DATECELL_WIDTH)),
                                          monthYearTextLabel->y () + monthYearTextLabel->height () + SCALE_HEIGHT(5),
                                          CAL_DATECELL_WIDTH, CAL_DATECELL_HEIGHT, weekdaysArray[index],
                                          this, CALENDAR_FONT_COLOR, SCALE_WIDTH(12), -1, false);
    }

    for(quint8 row = 0; row < CAL_MAX_ROW; row++)
    {
        for(quint8 col = 0; col <CAL_MAX_COL ; col++)
        {
            dateOfMonth[col + (row * CAL_MAX_COL)] = new RectWithText((SCALE_WIDTH(12) + (col * CAL_DATECELL_WIDTH)),
                                                                      (monthYearTextLabel->y () + monthYearTextLabel->height () + SCALE_HEIGHT(30) + (row * CAL_DATECELL_HEIGHT)),
                                                                      CAL_DATECELL_WIDTH,
                                                                      CAL_DATECELL_HEIGHT,
                                                                      dateArray[col + (row * 7)],
                                                                      this, CALENDAR_FONT_COLOR, SCALE_WIDTH(12),
                                                                      (CALENDAR_FIRST_DATE + col + (row * CAL_MAX_COL)));

            m_elementList[CALENDAR_FIRST_DATE + col + (row * CAL_MAX_COL)] =
                    dateOfMonth[col + (row * CAL_MAX_COL)];

            connect (dateOfMonth[col + (row * CAL_MAX_COL)],
                     SIGNAL(sigMouseClicked(quint32)),
                     this,
                     SLOT(slotDateBoxClicked(quint32)));

            connect (dateOfMonth[col + (row * CAL_MAX_COL)],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrElement(int)));
        }
    }
}
//*****************************************************************************
// TextBox
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void Calendar::intializeCal()
{
    if(m_dateParam.year > 2037)
    {
        m_dateParam.year = 2000;
    }

    QString tempImagePath = monthArray[m_dateParam.month] + ", " + QString("%1").arg (m_dateParam.year);

    monthYearTextLabel->changeText (tempImagePath);

    dateObj.setDate (m_dateParam.year, m_dateParam.month, 1);
    // gives total days in month
    int totalDayInMonth = dateObj.daysInMonth ();
    // gives which day month start
    int dayNameIndex = dateObj.dayOfWeek ();

    int tempFirstDay = (dayNameIndex == CAL_MAX_COL)?0:dayNameIndex;

    for(quint8 index = 0; index < tempFirstDay; index++)
    {
        dateOfMonth[index] -> changeText ("");
        dateOfMonth[index]->setColor (NORMAL_BKG_COLOR);
        dateOfMonth[index]->changeFontColor (CALENDAR_FONT_COLOR);
    }

    int tempDay = 0;
    do
    {
        dateOfMonth[tempFirstDay + tempDay]->changeText (QString("%1").arg (tempDay + 1));
        dateOfMonth[tempFirstDay + tempDay]->setColor (NORMAL_BKG_COLOR);
        dateOfMonth[tempFirstDay + tempDay]->changeFontColor (CALENDAR_FONT_COLOR);
        tempDay++;
    }while(tempDay < (totalDayInMonth));

    for(quint8 index = (tempDay + tempFirstDay); index < CAL_MAX_DAY_INDEX; index++)
    {
        dateOfMonth[index] -> changeText ("");
        dateOfMonth[index]->setColor (NORMAL_BKG_COLOR);
        dateOfMonth[index]->changeFontColor (CALENDAR_FONT_COLOR);
    }

    if(dateOfMonth[m_dateParam.date + tempFirstDay -1]->getText () != "")
    {
        dateOfMonth[m_dateParam.date + tempFirstDay -1]->setColor (CLICKED_BKG_COLOR);
        dateOfMonth[m_dateParam.date + tempFirstDay-1]->changeFontColor (HIGHLITED_FONT_COLOR);
    }
    update ();
}

void Calendar::checkNavigationValid(CAL_KEY_e key)
{
    quint32 tempElement = 0;
    if((m_currElement >= CALENDAR_FIRST_DATE) &&
            (m_currElement <= CALENDAR_LAST_DATE))
    {
        switch(key)
        {
        case CAL_KEY_LEFT:
            m_currElement--;
            if(dateOfMonth[m_currElement - CALENDAR_FIRST_DATE]->getText () == "")
            {
                m_currElement = CALENDAR_YEARNEXT;
            }
            m_elementList[m_currElement]->forceActiveFocus ();
            break;

        case CAL_KEY_RIGHT:
            m_currElement++;
            if(dateOfMonth[m_currElement - CALENDAR_FIRST_DATE]->getText () == "")
            {
                m_currElement = CALENDAR_CLOSE_BTN;
            }
            m_elementList[m_currElement]->forceActiveFocus ();
            break;

        case CAL_KEY_UP:
            if(m_currElement >= CALENDAR_FIRST_DATE + CAL_MAX_COL)
            {
                tempElement = (m_currElement-CAL_MAX_COL);
                if(dateOfMonth[tempElement - CALENDAR_FIRST_DATE]->getText () != "")
                {
                    m_currElement = tempElement;
                    m_elementList[m_currElement]->forceActiveFocus ();
                }
            }
            break;

        case CAL_KEY_DOWN:
            if(m_currElement <= CALENDAR_LAST_DATE - CAL_MAX_COL)
            {
                tempElement = (m_currElement+CAL_MAX_COL);
                if(dateOfMonth[tempElement - CALENDAR_FIRST_DATE]->getText () != "")
                {
                    m_currElement = tempElement;
                    m_elementList[m_currElement]->forceActiveFocus ();
                }
            }
            break;

        default:
            break;
        }
    }
    else
    {
        switch(key)
        {
        case CAL_KEY_LEFT:
            if(m_currElement == CALENDAR_CLOSE_BTN)
            {
                m_currElement = CALENDAR_LAST_DATE;
                do
                {
                    m_currElement--;
                }while(dateOfMonth[m_currElement - CALENDAR_FIRST_DATE]->getText () == "");
            }
            else
            {
                m_currElement--;
            }
            m_elementList[m_currElement]->forceActiveFocus ();
            break;

        case CAL_KEY_RIGHT:
            if(m_currElement == CALENDAR_YEARNEXT)
            {
                do
                {
                    m_currElement++;
                }while(dateOfMonth[m_currElement - CALENDAR_FIRST_DATE]->getText () == "");
            }
            else
            {
                m_currElement++;
            }
            if(m_currElement < MAX_CALENDAR_ELEMENT)
            {
                m_elementList[m_currElement]->forceActiveFocus ();
            }
            break;

        default:
            break;
        }
    }
}

//*****************************************************************************
// TextBox
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void Calendar::slotDateBoxClicked(quint32 index)
{
    QString tempDate;
    tempDate = dateOfMonth[index - CALENDAR_FIRST_DATE]->getText ();

    m_dateParam.date = tempDate.toUInt ();
    this->deleteLater ();
    emit sigUpdateDate (&m_dateParam);
}

void Calendar::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currElement]->forceActiveFocus();
}

void Calendar::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
        event->accept();
        checkNavigationValid(CAL_KEY_LEFT);
        break;

    case Qt::Key_Right:
        event->accept();
        checkNavigationValid(CAL_KEY_RIGHT);
        break;

    case Qt::Key_Up:
        event->accept();
        checkNavigationValid(CAL_KEY_UP);
        break;

    case Qt::Key_Down:
        event->accept();
        checkNavigationValid(CAL_KEY_DOWN);
        break;

    default:
        event->accept();
        break;
    }
}

void Calendar::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    emit sigUpdateDate (&m_dateParam);
    deleteLater ();
}

void Calendar::slotUpdateCurrElement(int index)
{
    m_currElement = index;
}

void Calendar::slotImagesClicked(int index)
{
    switch(index)
    {
    case CALENDAR_CLOSE_BTN:
        emit sigUpdateDate (&m_dateParam);
        deleteLater ();
        break;

    case CALENDAR_YEARBACK:
        m_dateParam.year--;
        intializeCal ();
        break;

    case CALENDAR_MONTHBACK:
        m_dateParam.month--;
        if(m_dateParam.month < 1)
        {
            m_dateParam.month = 12;
            m_dateParam.year--;
        }
        intializeCal ();
        break;

    case CALENDAR_MONTHNEXT:
        m_dateParam.month++;
        if(m_dateParam.month > 12)
        {
            m_dateParam.month = 1;
            m_dateParam.year++;
        }
        intializeCal ();
        break;

    case CALENDAR_YEARNEXT:
        m_dateParam.year++;
        intializeCal ();
        break;

    default:
        break;
    }
}
void Calendar::setFocusToPage()
{
    m_currElement = CALENDAR_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
}

void Calendar::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void Calendar::tabKeyPressed(QKeyEvent *event)
{
    event->accept();       // simply pass the event.
}

void Calendar::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void Calendar::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void Calendar::ctrl_D_KeyPressed(QKeyEvent *event)
{
    event->accept();
}
