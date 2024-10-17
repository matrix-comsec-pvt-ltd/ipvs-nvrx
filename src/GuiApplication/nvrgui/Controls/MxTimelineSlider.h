#ifndef MXTIMELINESLIDER_H
#define MXTIMELINESLIDER_H

#include <QWidget>
#include <QShowEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDateTime>
#include "Bgtile.h"
#include "TextLabel.h"
#include "Rectangle.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "ApplicationMode.h"
#include "ToolTip.h"
#include "ImageControls/Image.h"
#include "Controls/ControlButton.h"

#define MAX_SLOT_NUMBER         12

typedef enum
{
    FORWARD_CLOSE_SQUARE,
    BACKWARD_CLOSE_SQUARE,
    QUICK_BK_PAGE_PREVIOUS,
    QUICK_BK_PAGE_NEXT,

    MAX_CLOSE_SQUARE
}TIME_BAR_ELEMNET_e;

typedef enum
{
    QUICK_BK_HOUR_1,
    QUICK_BK_HOUR_6,
    QUICK_BK_HOUR_12,
    QUICK_BK_HOUR_24,
    QUICK_BK_HOUR_48,
    QUICK_BK_HOUR_72,

    MAX_HOUR_FORMAT
}QUICK_BK_HOURS_FORMAT_e;

typedef enum
{
    QUICK_BACKUP_NEXT_BUTTON,
    QUICK_BACKUP_PREV_BUTTON,

    MAX_ELE_REF

}QUICK_BACKUP_ELE_REF;

class MxTimelineSlider : public QWidget, public NavigationControl
{
    Q_OBJECT
private:
    int             m_xBeforeSlide;
    int             m_timeDiff;
    qreal           m_totalPixelMove;
    qreal           m_timedifference;
    qreal           m_fwdBrktX;
    qreal           m_bwdBrktX;

    bool            m_fwdBrktClicked;
    bool            m_bwdBrktClicked;
    bool            m_clickedInSquare;
    bool            m_tileToolTipHide;

    BgTile*         m_TimeLineTile;
    BgTile*         m_TimeLineTile1;
    BgTile*         m_TimeLineTile2;
    BgTile*         m_TimeLineTile3;
    BgTile*         m_TimeLineTile4;
    BgTile*         m_TimeLineTile5;
    Rectangle*      m_division[MAX_SLOT_NUMBER];
    Image*          m_forwardTimeSpanBrkt;
    Image*          m_backwardTimeSpanBrkt;
    Rectangle*      m_diffBetwnBrkt;

    TextLabel*      m_timeText[MAX_SLOT_NUMBER + 1];

    ApplController* m_applController;
    PayloadLib* m_payloadLib;
    NavigationControl *m_elementList[MAX_CLOSE_SQUARE];

    int m_currentDate;
    int m_currentMonth;
    int m_currentYear;
    int m_currentHour;
    int m_currentMinute;
    int m_currentSecond;
    int m_currentUpdateMin;
    int m_currentUpdateDisMin;
    int m_currentHourUpdate;
    int m_currentUpdateDisHour;

    int m_tileToolTipMinutes;
    int m_tileToolTipSeconds;
    int m_tileToolTipHours;

    qreal m_divisionValue;
    qreal m_oneMinuteValue;
    QPoint m_lastClickPoint;
    QRect m_timeMoverect;
    QDateTime fwdDateTime;
    QDateTime bwdDateTime;
    QString m_timeString;
    QString m_fwdSliderToolTipDate;
    QString m_bwdSliderToolTipDate;
    QString duration;
    QString m_fwdTimeString;
    QString m_bwdTimeString;

    ControlButton* m_previousButton;
    ControlButton* m_nextButton;
    int            m_currElement;
    TextLabel*     m_prevDateTimeLable;
    TextLabel*     m_nextDateTimeLable;
    QDateTime      m_dateTime;
    QUICK_BK_HOURS_FORMAT_e m_currentHourFormat;
    QDateTime      m_currDateTime;
    QDateTime      m_startDateTime;
    QDateTime      m_endDateTime;

    QDateTime      m_sliderDateTime;
    QDateTime      m_startTime;
    QDateTime      m_midTime;
    qreal          m_pixelPerMinit;

public:
    MxTimelineSlider(int xParam,
                     int yParam,
                     quint16 indexInPage,
                     QWidget* parent = 0,
                     bool isEnable = true,
                     bool catchKey = true);
    ~MxTimelineSlider();

    void PrivateVarAccess(NavigationControl **temp,QUICK_BACKUP_ELE_REF e ) const;
    void createDefaultTimeText();
    void updateTimeOnTimeLine();
    void getDateAndTime(QString deviceName);
    void processDeviceResponseforSliderTime(DevCommParam* param, QString deviceName);
    QString changeToStandardTime();
    void updateDateTime(QString dateTimeString);
    void updateDateTime(QDateTime dateTime);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter (QObject *, QEvent *event);
    void updateDateTimeOnMouseMove(int curX);
    void showtiletooltip(int curX,int curY);
    void showSliderToolTip(int curX,int curY);
    void updateHourMinutes();
    void getDateTime(QDateTime* startDtTime,QDateTime* endDtTime);
    void defaultSliderDraw(QUICK_BK_HOURS_FORMAT_e hourFormat);
    void changeSliderTime();
    void convertTimeTOEpochTime();
    qreal convertTimeToSecMinit(QDateTime startTime, QDateTime startSliderTime);
    void updateSliderMin();
    void dateTimeLableToolTip();
    void drawBkBarAndFwBarOnSlider();
    void calcualtePixelMin(qreal bkwdX, qreal fwrdX);

signals:
    void sigTileToolTipUpdate(QString m_timeString, int curX, int curY);
    void sigTileToolTipHide(bool toolTip);
    void sigSliderToolTipUpdate(QString m_SliderDateTime,int curX,int curY);
    void sigSliderToolTipHide(bool sliderToolTip);

public slots:

    void slotUpdateCurrentElement(int index);
    void slotChangePage(int index = 0);

};

#endif //MXTIMELINESLIDER_H


