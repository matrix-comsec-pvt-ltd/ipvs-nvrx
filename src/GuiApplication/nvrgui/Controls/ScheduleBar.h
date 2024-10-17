#ifndef SCHEDULEBAR_H
#define SCHEDULEBAR_H

#include "EnumFile.h"
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"

#define SCHEDULEBAR_IMAGEPATH    IMAGE_PATH"ScheduleBar/cell.png"

#define MAX_HOURS       24
#define MAX_INNER_HOUR_DIV  12       //12
#define MAX_TIME_SLOT    6

typedef struct {
    quint32 start_hour[MAX_TIME_SLOT];
    quint32 start_min[MAX_TIME_SLOT];
    quint32 stop_hour[MAX_TIME_SLOT];
    quint32 stop_min[MAX_TIME_SLOT];

}SCHEDULE_TIMING_t;


class ScheduleBar : public BgTile
{
    QPixmap image;
    QString schBarlabel;
    QRectF imgRect[MAX_HOURS];
    QRectF innerFrontRect[MAX_HOURS*MAX_INNER_HOUR_DIV];
    quint32 m_startPoint[MAX_TIME_SLOT];
    quint32 m_stopPoint[MAX_TIME_SLOT];

    TextLabel* schBarTextLabel;
    TextLabel* schBarMarkingLabel[MAX_HOURS + 1];
    qreal leftMargin;

    bool isScheduleBarMarking;

public:
    ScheduleBar(qreal xParam,
                qreal yParam,
                qreal width,
                qreal height,
                BGTILE_TYPE_e bgTileType,
                QString label = "",
                qreal leftmargin = (qreal)((qreal)(10 * SCREEN_WIDTH)/(qreal)1920),
                QWidget *parent = 0,
                bool isEnable = true,
                bool isSchBarMarking =false);

    ~ScheduleBar();

    void createDefaultComponets();
    void createSchduleBarMarking();
    void setScheduleForEntireDay(bool entireDay);
    void setSchedule(SCHEDULE_TIMING_t schdTiming);
    void paintEvent (QPaintEvent *event);
    void setIsEnable(bool isEnable);
};

#endif // SCHEDULEBAR_H
