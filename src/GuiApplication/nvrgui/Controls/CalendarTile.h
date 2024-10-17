#ifndef CALENDARTILE_H
#define CALENDARTILE_H

#include "EnumFile.h"
#include "Controls/Bgtile.h"
#include "NavigationControl.h"

#include "Controls/TextLabel.h"
#include "Controls/Calendar.h"
#include "Controls/InvisibleWidgetCntrl.h"

class CalendarTile:public BgTile, public NavigationControl
{
    Q_OBJECT
public:
    CalendarTile(quint32 startX,
                 quint32 startY,
                 quint32 width,
                 quint32 height,
                 QString dateStr,
                 QString label,
                 QWidget* parent = 0,
                 quint16 controlIndex = 0,
                 bool isBoxStartInCentre = true,
                 quint16 leftMarginOfLabel = 0,
                 BGTILE_TYPE_e bgType = COMMON_LAYER,
                 bool isNavigationEnable = true);
    ~CalendarTile();

    void createDefaultComponent();
    void changeImage(IMAGE_TYPE_e type);
    void takeEnterKeyAction();
    void selectControl();
    void deSelectControl();

    void setIsEnabled(bool isEnable);

    //virtual functions inherited from QWidget
    void forceActiveFocus();
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent *event);

    DDMMYY_PARAM_t * getDDMMYY();
    void constructDayMonthStr();
    void setDDMMYY(DDMMYY_PARAM_t *param);

private:
    QString m_dateStr;
    QString m_label;
    quint16 m_leftMargin;
    QString m_textColor;
    // This is req to display calender
    quint16 calImgOffset;

    DDMMYY_PARAM_t m_dmyParam;

    bool m_isInCentre;

    quint8 m_currentImageType;
    TextLabel *m_labeltext;
    TextLabel *m_datetext;
    InvisibleWidgetCntrl *m_invisibleWidget;
    Calendar *m_calendar;

    QRect m_textBoxRect;
    QRect m_calRect;
    QPixmap m_textBoxImg;
    QPixmap m_calImage;

    bool m_isMouseClick;
    QPoint m_lastClickPoint;

signals:
    void sigUpdateCurrentElement(int index);
    void sigDateChanged();

public slots:
    void slotUpdateDate(DDMMYY_PARAM_t * param);
    void slotDestroyCalendar();
    void slotDestroyCalendarOnOuterClick ();
};

#endif // CALENDARTILE_H
