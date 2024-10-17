#include "ScheduleBar.h"
#include <QPainter>

static const QString hoursLabel[25]={
    "0", "", "", "", "4", "", "", "",
    "8", "", "", "","12", "", "", "",
    "16", "", "","","20", "", "", "","24"
};

ScheduleBar::ScheduleBar(qreal xParam,
                         qreal yParam,
                         qreal width,
                         qreal height,
                         BGTILE_TYPE_e bgTileType,
                         QString label,
                         qreal leftmargin,
                         QWidget *parent,
                         bool isEnable ,
                         bool isSchBarMarking) :
    BgTile(xParam,yParam,width,height,bgTileType,parent), schBarlabel(label),
    leftMargin(leftmargin), isScheduleBarMarking(isSchBarMarking)
{
    if(isScheduleBarMarking)
    {
        createSchduleBarMarking();
    }
    else
    {
        createDefaultComponets();
    }

    if(!isEnable)
        setIsEnable (isEnable);

    this->show ();
}

void ScheduleBar::createDefaultComponets()
{
    qint8 verticalOffset = 0;
    qreal width = 0;
    qreal labelWidth = 0;
    int maxPossibleWidth=0, barStart = 150;

    if(schBarlabel != "")
    {
        QFont labelFont;

        labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        labelWidth = QFontMetrics(labelFont).width (schBarlabel);

        width += (qreal)((qreal)(10 * SCREEN_WIDTH)/(qreal)1920);
    }

    image = QPixmap(IMAGE_PATH"ScheduleBar/cell.png");
    SCALE_IMAGE(image);

    width += (qreal)image.width () + (qreal)labelWidth ;

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

    maxPossibleWidth = leftMargin + SCALE_WIDTH(barStart) - SCALE_WIDTH(15);
    schBarTextLabel = new TextLabel((qreal)((qreal)leftMargin + (qreal)((qreal)(barStart * SCREEN_WIDTH)/(qreal)1920)),
                                    qreal((this->height ())/2 + verticalOffset),
                                    NORMAL_FONT_SIZE,
                                    schBarlabel,
                                    this,
                                    HIGHLITED_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_END_X_CENTRE_Y, 0, 0, maxPossibleWidth);


    quint16 innerIndex = 0;


    for (quint8 index = 0; index < MAX_HOURS ; index++)
    {
        imgRect[index].setRect (  (qreal)schBarTextLabel->x () +
                                  (qreal)schBarTextLabel->width () +
                                  (qreal)((qreal)(10 * SCREEN_WIDTH)/(qreal)1920) + (qreal)(image.width ()*index),
                                  (qreal)(this->height () - image.height ())/2 +
                                  verticalOffset ,
                                  (qreal)image.width (),
                                  (qreal)image.height ());
        for(quint8 index2 = 0; index2 < MAX_INNER_HOUR_DIV ; index2++)
        {
            innerFrontRect[innerIndex++].setRect (( (qreal)schBarTextLabel->x () +
                                                    (qreal)schBarTextLabel->width () +
                                                    (qreal)((qreal)(10 * SCREEN_WIDTH)/(qreal)1920) + (qreal)(image.width ()*index)+
                                                   ((qreal)((qreal)imgRect[index].width ()/(qreal)12)* (qreal)index2)),
                                                  (imgRect[index].y ()+ SCALE_HEIGHT(5)),
                                                  (qreal)(((qreal)imgRect[index].width ())/(qreal)12) + (qreal)((0.5 * SCREEN_WIDTH)/1920),
                                                  SCALE_HEIGHT(5));

        }
    }

    SCHEDULE_TIMING_t myStruct;

    memset(&myStruct, 0, sizeof(SCHEDULE_TIMING_t));

    setSchedule (myStruct);
}

void ScheduleBar:: createSchduleBarMarking()
{
    for (quint8 index = 0 ; index <= MAX_HOURS; index++)
    {
        schBarMarkingLabel[index] = new TextLabel (leftMargin + SCALE_WIDTH(165) + index* (SCALE_WIDTH(30)),
                                                   this->height () - SCALE_HEIGHT(10),
                                                   SCALE_FONT(SUFFIX_FONT_SIZE),
                                                   hoursLabel[index],
                                                   this,
                                                   DISABLE_FONT_COLOR,
                                                   NORMAL_FONT_FAMILY,
                                                   ALIGN_END_X_CENTRE_Y);
    }
}

ScheduleBar::~ScheduleBar()
{
    if(isScheduleBarMarking)
    {
        for (quint8 index = 0 ; index<= MAX_HOURS; index++)
            delete schBarMarkingLabel[index] ;
    }
    else
    {
        delete schBarTextLabel;
    }
}

void ScheduleBar::paintEvent (QPaintEvent *event)
{
    BgTile::paintEvent (event);

    if(!(isScheduleBarMarking))
    {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        QRectF source(0.0, 0.0, 0.0, 0.0);

        for (quint8 index = 0; index < MAX_HOURS ; index++)
        {
            painter.drawPixmap (imgRect[index], image, source);
        }
        for(quint16 index = m_startPoint[0]; index < m_stopPoint[0] ; index++)
        {
            painter.fillRect (innerFrontRect[index],QColor(SCHEDULE_BAR_COLOR));
        }
        for(quint16 index = m_startPoint[1]; index < m_stopPoint[1] ; index++)
        {
            painter.fillRect (innerFrontRect[index],QColor(SCHEDULE_BAR_COLOR));
        }
        for(quint16 index = m_startPoint[2]; index < m_stopPoint[2] ; index++)
        {
            painter.fillRect (innerFrontRect[index],QColor(SCHEDULE_BAR_COLOR));
        }
        for(quint16 index = m_startPoint[3]; index < m_stopPoint[3] ; index++)
        {
            painter.fillRect (innerFrontRect[index],QColor(SCHEDULE_BAR_COLOR));
        }
        for(quint16 index = m_startPoint[4]; index < m_stopPoint[4] ; index++)
        {
            painter.fillRect (innerFrontRect[index],QColor(SCHEDULE_BAR_COLOR));
        }
        for(quint16 index = m_startPoint[5]; index < m_stopPoint[5] ; index++)
        {
            painter.fillRect (innerFrontRect[index],QColor(SCHEDULE_BAR_COLOR));
        }
    }
}

void ScheduleBar::setScheduleForEntireDay (bool entireDay)
{
    if(entireDay)
    {
        m_startPoint[0] = 0  ;
        m_stopPoint[0] = 24*12 ;
    }
    else
    {
        m_startPoint[0] = 0  ;
        m_stopPoint[0] = 0 ;

    }
}

void ScheduleBar:: setSchedule (SCHEDULE_TIMING_t schdTiming)
{
    for(quint8 index = 0; index < MAX_TIME_SLOT; index++)
    {
        if(((schdTiming.stop_hour[index] == 0) &&
                (schdTiming.stop_min[index] == 0)) &&
                ((schdTiming.start_hour[index] > 0) ||
                (schdTiming.start_min[index] > 0)))
        {
            schdTiming.stop_hour[index] = 24;
            schdTiming.stop_min[index] = 00;
        }

        m_startPoint[index] = 0;
        m_startPoint[index] =(((schdTiming.start_hour[index])*12) +
                              schdTiming.start_min[index]/5);
        m_stopPoint[index] = 0;
        m_stopPoint[index] =(((schdTiming.stop_hour[index] )*12) +
                             schdTiming.stop_min[index]/5);
    }
}

void ScheduleBar::setIsEnable (bool isEnable)
{
    if(isEnable)
    {
        schBarTextLabel->changeColor (HIGHLITED_FONT_COLOR);
    }
    else
    {
        schBarTextLabel->changeColor (DISABLE_FONT_COLOR);
    }
    update ();
}
