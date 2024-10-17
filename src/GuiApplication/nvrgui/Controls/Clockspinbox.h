#ifndef CLOCKSPINBOX_H
#define CLOCKSPINBOX_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"
#include "Controls/TextLabel.h"
#include "Bgtile.h"
#include "NavigationControl.h"
#include "Controls/DropDown.h"
#include "Controls/InvisibleWidgetCntrl.h"

#define CLK_SPIN_BG_IMAGE_PATH      IMAGE_PATH "ClockSpinBox/Backgrounds/"
#define CLK_SPIN_CELL_IMAGE_PATH    IMAGE_PATH "ClockSpinBox/Cell/"
#define MAX_HOUR                    23
#define MAX_MIN_SEC                 60

#define BUTTON_HEIGHT           SCALE_HEIGHT(30)

typedef enum
{
    CLK_SPINBOX_With_SEC,
    CLK_SPINBOX_With_NO_SEC,
    MAX_CLK_SPIN_TYPE
}CLK_SPIN_TYPE_e;

typedef enum
{
    HOUR_BOX,
    MINUTE_BOX,
    SEC_BOX,
    MAX_TIME_BOX
}CLK_SPIN_TIME_BOX_e;

typedef enum{
    CLK_SPIN_MAIN,
    CLK_SPIN_HOUR,
    CLK_SPIN_MIN,
    CLK_SPIN_SEC,
}CLK_SPIN_IMAGE_TYPE_e;

const QString clkSpinSubFldr[MAX_CLK_SPIN_TYPE] =
{
    "HH_MM_SS/",
    "HH_MM/"
};


class ClockSpinbox: public BgTile, public NavigationControl
{
    Q_OBJECT
public:
    ClockSpinbox(qint32 xParam,
                 qint32 yParam,
                 qint32 width,
                 qint32 height,
                 quint16 controlIndex,
                 CLK_SPIN_TYPE_e bgType,
                 QString labelStr,
                 qint8 maxElementWithoutScroll = 10,
                 QWidget* parent=0,
                 QString suffixStr = "",
                 bool isBoxStartInCentre = true,
                 quint16 leftMarginOfLabel = 0,
                 BGTILE_TYPE_e bgTileType= COMMON_LAYER,
                 bool isNavigationEnable = true,
                 quint8 cntValue = 1,
                 bool minMaxReq = false,
                 bool isDropUpList = false,
                 quint32 leftMarginfromCenter = 0);
    ~ClockSpinbox();
private:
    CLK_SPIN_TYPE_e         type;
    CLK_SPIN_TIME_BOX_e     currClickedBox;

    QString                 fontColor;
    IMAGE_TYPE_e            currentImageType;
    bool                    enterMode;
    bool                    isCentre;
    bool                    m_isDropUpList;
    quint16                 leftMargin;
    quint32                 m_leftMarginfromCenter;

    qint32                  hrsIndx;
    qint32                  minIndx;
    qint32                  secIndx;
    quint8                  countValue;

    QString                 label;
    QString                 suffix;

    QRect                   bgRect;
    QRect                   hourRect;
    QRect                   minuteRect;
    QRect                   secRect;

    TextLabel               *sepreator1;
    TextLabel               *sepreator2;

    TextLabel               *labelText;
    TextLabel               *suffixText;
    TextLabel               *hrsText;
    TextLabel               *minuteText;
    TextLabel               *secText;
    DropDownList            *m_dropDownList;
    InvisibleWidgetCntrl    *m_dropDownInVisibleWidget;

    QPixmap                 bgImage;
    QPixmap                 hrsImage;
    QPixmap                 minImage;
    QPixmap                 secImage;

    bool                    isMinMaxReq;

    quint16                 currTotalSeconds;
    quint16                 minTotalSeconds;
    quint16                 maxTotalSeconds;

    qint8                   m_maxElementWithoutScroll;

    QMap<quint8, QString>   m_hourList;
    QMap<quint8, QString>   m_minuteList;
    QMap<quint8, QString>   m_secondList;

public:
    void changeImage(CLK_SPIN_IMAGE_TYPE_e index,IMAGE_TYPE_e img_type);
    void assignValue(QString hour, QString minute);
    void assignValue(quint32 hour, quint32 minute);
    void assignValue(QString hour, QString minute, QString sec);
    void assignValue(quint32 hour, quint32 minute, quint32 sec);
    void currentValue(quint32 &currentHour,quint32 &currentMin);
    void currentValue(quint32 &currentHour,quint32 &currentMin,quint32 &currentSec);
    void setMinTotalSeconds(quint16 minSec);
    void setMaxTotalSeconds(quint16 maxSec);
    void loadList(QMap<quint8, QString>  maplist, qint8 maxElementOnList, qint32 listStartX);
    void unloadList();

    void createDefaultComponents();
    void selectControl();
    void deSelectControl();
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeUpKeyAction();
    void takeDownKeyAction ();
    void takeEnterKeyAction();
    void hrsSelect();
    void minSelect();
    void secSelect();
    void setIsEnabled(bool isEnable);
    void forceActiveFocus();
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    void paintEvent(QPaintEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

    void incHrs();
    void decHrs();
    void incMin();
    void decMin();
    void incSec();
    void decSec();

    void updateText();

signals:
    void sigUpdateCurrentElement(int index);
    void sigTotalCurrentSec(quint16 totalSec, int indexInPage);

public slots:
    void slotUnloadDropList();
    void slotDropListDestroyed();
    void slotValueChanged(quint8, QString);
};

#endif // CLOCKSPINBOX_H
