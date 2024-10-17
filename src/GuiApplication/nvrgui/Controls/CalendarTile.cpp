#include "CalendarTile.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>

#define CALENDAR_TEXTBOX_PATH       IMAGE_PATH"Textbox/Medium/"
#define CALENDAR_ICON_PATH          IMAGE_PATH"Calendar/Icon/"

static const QString monthShortName[] =
{
    "",    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
//*****************************************************************************
// CalendarTile
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
CalendarTile::CalendarTile(quint32 startX,
                           quint32 startY,
                           quint32 width,
                           quint32 height,
                           QString dateStr,
                           QString label,
                           QWidget* parent,
                           quint16 controlIndex,
                           bool isBoxStartInCentre,
                           quint16 leftMarginOfLabel,
                           BGTILE_TYPE_e bgType,
                           bool isNavigationEnable):
    BgTile(startX,
           startY,
           width,
           height,
           bgType, parent), NavigationControl(controlIndex, isNavigationEnable),
    m_dateStr(dateStr), m_label(label), m_leftMargin(leftMarginOfLabel),
    m_textColor(NORMAL_FONT_COLOR), calImgOffset(0), m_isMouseClick(0)
{
    m_invisibleWidget = NULL;
    m_isInCentre = isBoxStartInCentre;
    m_calendar = NULL;
    m_dmyParam.date = 1;
    m_dmyParam.month = 1;
    m_dmyParam.year = 2000;

    if(m_isEnabled)
    {
        m_currentImageType =  IMAGE_TYPE_NORMAL;
    }
    else
    {
        m_currentImageType =  IMAGE_TYPE_DISABLE;
    }

    QString imgPath = CALENDAR_TEXTBOX_PATH + imgTypePath[m_currentImageType];
    m_textBoxImg = QPixmap(imgPath);
    SCALE_IMAGE(m_textBoxImg);

    imgPath = CALENDAR_ICON_PATH + imgTypePath[m_currentImageType];
    m_calImage = QPixmap(imgPath);
    SCALE_IMAGE(m_calImage);

    createDefaultComponent ();
    this->setEnabled (m_isEnabled);
    this->setMouseTracking (true);

    this->show ();
}
//*****************************************************************************
// ~CalendarTile()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
CalendarTile::~CalendarTile()
{
    delete m_labeltext;
    delete m_datetext;
    if(m_calendar != NULL)
    {
        disconnect (m_calendar,
                    SIGNAL(sigUpdateDate(DDMMYY_PARAM_t*)),
                    this,
                    SLOT(slotUpdateDate(DDMMYY_PARAM_t*)));
        disconnect(m_calendar,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotDestroyCalendar()));
        delete m_calendar;

        disconnect (m_invisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotDestroyCalendarOnOuterClick()));
        delete m_invisibleWidget;
    }
}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::changeImage(IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    QString imgPath = CALENDAR_TEXTBOX_PATH + imgTypePath[m_currentImageType];
    m_textBoxImg = QPixmap(imgPath);
    SCALE_IMAGE(m_textBoxImg);

    imgPath = CALENDAR_ICON_PATH + imgTypePath[m_currentImageType];
    m_calImage = QPixmap(imgPath);
    SCALE_IMAGE(m_calImage);
    update ();
}
//*****************************************************************************
// createDefaultComponent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::createDefaultComponent()
{
    quint16 labelWidth = 0, labelHeight = 0, translatedlabelWidth = 0;
    QFont labelFont;
    qint8 verticalOffset = 0;
    quint16 width = 0;

    labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, SCALE_WIDTH(15));
    labelWidth = QFontMetrics(labelFont).width (m_label);
    translatedlabelWidth = QFontMetrics(labelFont).width (QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
    labelHeight = QFontMetrics(labelFont).height ();

    width = labelWidth + SCALE_WIDTH(20) + m_textBoxImg.width () + SCALE_WIDTH(10) + m_calImage.width ();
    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        break;

    case TOP_TABLE_LAYER:
        verticalOffset = (TOP_MARGIN / 2);
        break;

    case BOTTOM_TABLE_LAYER:
        verticalOffset = -(TOP_MARGIN / 2);
        break;

    default:
        break;
    }

    if(m_isInCentre == true)
    {
        labelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20))) ? (getWidth()/2 - SCALE_WIDTH(20)) : translatedlabelWidth;
        m_labeltext = new TextLabel(((this->width ()/2) - labelWidth - SCALE_WIDTH(10)),
                                    (this->height () - labelHeight)/2 + verticalOffset,
                                    NORMAL_FONT_SIZE, m_label,
                                    this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                    0, 0, labelWidth, 0, 0, Qt::AlignRight);

        m_textBoxRect.setRect (this->width ()/2,
                               (this->height () - m_textBoxImg.height ())/2 + verticalOffset,
                               m_textBoxImg.width (),
                               m_textBoxImg.height ());
    }
    else
    {
        translatedlabelWidth = (translatedlabelWidth > ((m_leftMargin + labelWidth) - SCALE_WIDTH(17))) ? ((m_leftMargin + labelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
        m_labeltext = new TextLabel(abs((abs(translatedlabelWidth - (m_leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                    (this->height () - labelHeight)/2 + verticalOffset,
                                    NORMAL_FONT_SIZE, m_label,
                                    this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                    0, 0, translatedlabelWidth, 0, 0, Qt::AlignRight);

        m_textBoxRect.setRect (m_leftMargin + labelWidth,
                               (this->height () - m_textBoxImg.height ())/2 + verticalOffset,
                               m_textBoxImg.width (),
                               m_textBoxImg.height ());
    }

    m_calRect.setRect (m_textBoxRect.x () + m_textBoxRect.width ()+ SCALE_WIDTH(10),
                       (this->height () - m_calImage.height ())/2 + verticalOffset,
                       m_calImage.width (),
                       m_calImage.height ());

    m_datetext = new TextLabel(m_textBoxRect.x () + SCALE_WIDTH(10),
                               m_textBoxRect.y () + (m_textBoxRect.height ()/2),
                               NORMAL_FONT_SIZE,
                               m_dateStr,
                               this,
                               (m_isEnabled == true)?NORMAL_FONT_COLOR:DISABLE_FONT_COLOR,
                               NORMAL_FONT_FAMILY,
                               ALIGN_START_X_CENTRE_Y,
                               (m_isEnabled == true)?200:0);
}
//*****************************************************************************
// paintEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent (event);
    QPainter painter(this);
    painter.drawPixmap (m_textBoxRect, m_textBoxImg);
    painter.drawPixmap (m_calRect, m_calImage);
}
//*****************************************************************************
// mouseMoveEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::mouseMoveEvent (QMouseEvent *event)
{
    if((m_textBoxRect.contains (event->pos ())) ||
            (m_calRect.contains (event->pos ())))
    {
        if(m_calendar == NULL)
        {
            if(this->hasFocus ())
            {
                selectControl ();
            }
            else
            {
                forceActiveFocus ();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }
        else
        {
            selectControl ();
        }
    }
}
//*****************************************************************************
// forceActiveFocus
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::forceActiveFocus()
{
    if(m_isEnabled)
    {
        this->setFocus ();
    }
}
void CalendarTile::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage (IMAGE_TYPE_MOUSE_HOVER);
    }
}

void CalendarTile::deSelectControl()
{
    changeImage (IMAGE_TYPE_NORMAL);
}

void CalendarTile::focusInEvent(QFocusEvent *)
{
    selectControl ();
}

void CalendarTile::focusOutEvent(QFocusEvent *)
{
    if(m_calendar == NULL)
    {
        deSelectControl ();
    }
}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled (m_isEnabled);

        if(isEnable == true)
        {
            changeImage(IMAGE_TYPE_NORMAL);
            m_datetext->changeColor (NORMAL_FONT_COLOR);
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
            m_datetext->changeColor (DISABLE_FONT_COLOR);
        }
        update();
    }
}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::mousePressEvent(QMouseEvent * event)
{
    if((m_textBoxRect.contains (event->pos ())) ||
            (m_calRect.contains (event->pos ())))
    {
        m_isMouseClick = true;
        m_lastClickPoint = event->pos ();
    }
}

void CalendarTile::mouseReleaseEvent (QMouseEvent * event)
{
    if((m_isMouseClick == true) && (m_lastClickPoint == event->pos ()))
    {
        takeEnterKeyAction ();
    }
    m_isMouseClick = false;
}
//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CalendarTile::takeEnterKeyAction()
{
    // add calendar control here
    // which will be deleted on any date click, close click internally,
    if(m_calendar == NULL)
    {
        m_invisibleWidget = new InvisibleWidgetCntrl(parentWidget ());
        m_invisibleWidget->setGeometry (0,0,
                                        parentWidget ()->width (),
                                        parentWidget ()->height ());
        m_invisibleWidget->show ();

        connect (m_invisibleWidget,
                 SIGNAL(sigMouseClick()),
                 this,
                 SLOT(slotDestroyCalendarOnOuterClick()));
        //        m_calendar = new Calendar(this->x () + this->width () - 100,
        //                                  this->y () + this->height () - 5,
        //                                  CAL_BG_WIDTH, CAL_BG_HEIGHT,
        //                                  parentWidget ());
        m_calendar = new Calendar(this->x () + m_calRect.x () - SCALE_WIDTH(5),
                                  (this->y () + this->height () - SCALE_HEIGHT(5)),
                                  CAL_BG_WIDTH, CAL_BG_HEIGHT,
                                  parentWidget ());
        m_calendar->setDDMMYY (&m_dmyParam);

        connect(m_calendar,
                SIGNAL(destroyed()),
                this,
                SLOT(slotDestroyCalendar()));
        connect (m_calendar,
                 SIGNAL(sigUpdateDate(DDMMYY_PARAM_t*)),
                 this,
                 SLOT(slotUpdateDate(DDMMYY_PARAM_t*)));
        m_calendar->setFocusToPage ();
    }
    selectControl ();
}

void CalendarTile::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeEnterKeyAction();
}

void CalendarTile::constructDayMonthStr()
{
    QString tempStr;
    if(m_dmyParam.date < 10)
    {
        tempStr = QString("%1%2").arg (0).arg (m_dmyParam.date);
    }
    else
    {
        tempStr = QString("%1").arg (m_dmyParam.date);
    }
    tempStr += "-";
    tempStr+= monthShortName[m_dmyParam.month] + QString("%1%2").arg("-").arg (m_dmyParam.year);

    m_datetext->changeText (tempStr);

    if(m_calendar != NULL)
    {
        m_calendar->setDDMMYY (&m_dmyParam);
    }
    update ();
}

DDMMYY_PARAM_t * CalendarTile::getDDMMYY()
{
    // Use this data as read only
    // Do not modify
    return &m_dmyParam;
}

void CalendarTile::setDDMMYY(DDMMYY_PARAM_t *param)
{
    m_dmyParam.date = param->date;
    m_dmyParam.month = param->month;
    m_dmyParam.year = param->year;
    constructDayMonthStr ();
    emit sigDateChanged();
}

void CalendarTile::slotUpdateDate(DDMMYY_PARAM_t * param)
{
    setDDMMYY(param);
    disconnect (m_calendar,
                SIGNAL(sigUpdateDate(DDMMYY_PARAM_t*)),
                this,
                SLOT(slotUpdateDate(DDMMYY_PARAM_t*)));
}

void CalendarTile::slotDestroyCalendar()
{
    disconnect(m_calendar,
               SIGNAL(destroyed()),
               this,
               SLOT(slotDestroyCalendar()));
    m_calendar = NULL;

    disconnect (m_invisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotDestroyCalendarOnOuterClick()));
    delete m_invisibleWidget;

    if(!this->hasFocus ())
    {
        forceActiveFocus ();
    }
}

void CalendarTile::slotDestroyCalendarOnOuterClick()
{
    if(m_calendar != NULL)
    {
        disconnect (m_calendar,
                    SIGNAL(sigUpdateDate(DDMMYY_PARAM_t*)),
                    this,
                    SLOT(slotUpdateDate(DDMMYY_PARAM_t*)));
        disconnect(m_calendar,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotDestroyCalendar()));
        delete m_calendar;
        m_calendar = NULL;

        disconnect (m_invisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotDestroyCalendarOnOuterClick()));
        delete m_invisibleWidget;
    }
}
