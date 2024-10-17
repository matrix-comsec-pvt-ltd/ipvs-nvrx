#ifndef SYNCPLAYBACKCALENDER_H
#define SYNCPLAYBACKCALENDER_H

#include "NavigationControl.h"
#include "Controls/ImageControls/Image.h"
#include "Controls/RectWithText.h"
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "SyncPlaybackToolbar.h"
#include "Controls/LayoutWindowRectangle.h"
#include "EnumFile.h"

#define MAX_WEEKDAY_COL     7
#define MAX_WEEKDAY_ROW     6
#define MAX_DAYS            (MAX_WEEKDAY_COL * MAX_WEEKDAY_ROW)

typedef enum
{
    SYNCPLAYBACK_CALENDAR_PREV_YEAR,
    SYNCPLAYBACK_CALENDAR_PREV_MONTH,
    SYNCPLAYBACK_CALENDAR_NEXT_MONTH,
    SYNCPLAYBACK_CALENDAR_NEXT_YEAR,
    SYNCPLAYBACK_CALENDAR_DATE,
    SYNCPLAYBACK_MAX_CALENDAR_ELEMENT = 46
}SYNCPLAYBACK_CALENDAR_ELEMENT_LIST_e;

typedef enum
{
    IMAGE_PREV_YEAR,
    IMAGE_PREV_MONTH,
    IMAGE_NEXT_MONTH,
    IMAGE_NEXT_YEAR,
    MAX_DATE_NAVIGATION_IMAGE_TYPE
}DATE_NAVIGATION_IMAGE_TYPE_e;

class SyncPlaybackCalender :  public LayoutWindowRectangle, public NavigationControl
{
    Q_OBJECT
private:
    Image* m_naviagtionImage[MAX_DATE_NAVIGATION_IMAGE_TYPE];
    RectWithText* m_daysRectangle[MAX_DAYS];
    TextLabel* m_monthYearLabel;
    TextLabel* m_weekDaysLabel[MAX_WEEKDAY_COL];

    QString m_monthYearString, m_dateMonthYearString;
    quint16 m_currentDate, m_currentMonth, m_currentYear, m_daysInMonth, m_currentDateIndex;
    quint8 m_monthsFirstDay;
    quint32 m_dateCollection;

    int m_currentElement;
    NavigationControl *m_elementList[SYNCPLAYBACK_MAX_CALENDAR_ELEMENT];

public:
    SyncPlaybackCalender(quint16 startX,
                         quint16 startY,
                         quint16 width,
                         quint16 height,
                         quint16 indexInPage,
                         QWidget* parent = 0,
                         bool isEnable = true,
                         bool catchKey = true);
    ~SyncPlaybackCalender();

    void checkBoundaryConditions();
    void changeDateRectText();
    void initializeCalender(QString dateString);
    QString getMonthYearString();
    QString getDateMonthYearString();
    void changeDateCollection(quint64 dateCollection);
    quint8 changeDateToArrayIndex(quint8 date);
    quint8 changeArrayIndexToDate(quint8 arrayIndex);
    void selectNewDate(quint8 newDate);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeUpKeyAction();
    void takeDownKeyAction();

    void forceFocusToPage(bool isFirstElement);
    void setIsEnabled(bool isEnable);
    virtual void navigationKeyPressed(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void sigUpdateCurrentElement(int index);
    void sigFocusToOtherElement(bool isPrevoiusElement);
    void sigFetchNewSelectedDate();
    void sigFetchRecordForNewDate();

public slots:
    void slotUpdateCurrentElement(int index);
    void slotChangeCalenderDate(int index);
    void slotDateSelected(quint32 index);
};

#endif // SYNCPLAYBACKCALENDER_H
