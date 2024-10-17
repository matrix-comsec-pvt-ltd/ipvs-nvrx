#ifndef CALENDAR_H
#define CALENDAR_H

#include <QWidget>
#include <QDate>

#include "Controls/Bgtile.h"
#include "Controls/Closebuttton.h"
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"

#include "Controls/ImageControls/Image.h"
#include "Controls/RectWithText.h"
#include "NavigationControl.h"

#define CAL_BG_WIDTH            SCALE_WIDTH(276)//300//250
#define CAL_BG_HEIGHT           SCALE_HEIGHT(200)

#define CAL_MAX_COL             7
#define CAL_MAX_ROW             6
#define CAL_MAX_DAY_INDEX       (CAL_MAX_ROW * CAL_MAX_COL)

typedef enum
{
    CALENDAR_CLOSE_BTN,
    CALENDAR_YEARBACK,
    CALENDAR_MONTHBACK,
    CALENDAR_MONTHNEXT,
    CALENDAR_YEARNEXT,

    CALENDAR_FIRST_DATE,                // 5
    CALENDAR_LAST_DATE = 47,            // 5 +42

    MAX_CALENDAR_ELEMENT
}CALENDAR_ELEMENT_LIST_e;

typedef enum
{
    CAL_KEY_LEFT,
    CAL_KEY_RIGHT,
    CAL_KEY_UP,
    CAL_KEY_DOWN
}CAL_KEY_e;

typedef struct
{
    quint8 date;
    quint8 month;
    quint16 year;
}DDMMYY_PARAM_t;


class Calendar : public BgTile
{
    Q_OBJECT
public:
    Calendar(quint16 startX,
             quint16 startY,
             quint16 width,
             quint16 height,
            QWidget *parent = 0);

    ~Calendar();

    void paintEvent (QPaintEvent *event);
    void createDefaultComponent(void);
    void intializeCal();

    void setDDMMYY(DDMMYY_PARAM_t *param);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);
    virtual void ctrl_D_KeyPressed(QKeyEvent *event);
    void showEvent (QShowEvent *event);
    void checkNavigationValid(CAL_KEY_e key);
    void setFocusToPage();

private:
    quint16 bgWidth;
    quint16 bgHeight;

    QDate dateObj;
    DDMMYY_PARAM_t m_dateParam;

    bool isCloseBtnIn;
    CloseButtton *closeBtn;
    Image *m_images[4];

    TextLabel *monthYearTextLabel;
    Rectangle *midline;

    RectWithText *daysStr[CAL_MAX_COL];
    RectWithText *dateOfMonth[CAL_MAX_DAY_INDEX];

    quint32 m_currElement;
    NavigationControl *m_elementList[MAX_CALENDAR_ELEMENT];

signals:
    void sigUpdateDate(DDMMYY_PARAM_t *param);

public slots:
    void slotImagesClicked(int index);
    void slotDateBoxClicked(quint32 index);
    void slotUpdateCurrElement(int index);
};

#endif // CALENDAR_H
