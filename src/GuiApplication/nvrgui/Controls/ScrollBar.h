#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QWidget>
#include "NavigationControl.h"
#include "Rectangle.h"
#include "ImageControls/Image.h"
#include "KeyBoard.h"

typedef enum
{
    VERTICAL_SCROLLBAR,
    HORIZONTAL_SCROLLBAR
}SCROLLBAR_TYPE_e;

typedef enum
{
    UP_BUTTON,
    DOWN_BUTTON,
    MAX_BUTTON_TYPE
}SCROLL_BAR_BUTTON_TYPE_e;

class ScrollBar : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    SCROLLBAR_TYPE_e m_scrollbarType;
    qint32 m_startX, m_startY, m_width, m_height, m_sliderPosition;
    qint32 m_totalSteps, m_totalElements, m_stepOffset;
    qreal m_unitStepSizeForBar;
    QString m_buttonImagepath[MAX_BUTTON_TYPE];
    qreal m_sliderLength;
    Rectangle* m_sliderRect;
    Rectangle* m_mainRect;
    Image* m_buttonImage[MAX_BUTTON_TYPE];

    qint32 m_currentElement;
    NavigationControl* m_elementList[MAX_BUTTON_TYPE];

    QPoint m_lastClickPoint;
    bool m_isMousePressed;
    bool m_cameralist;
    qreal m_upMovement;
    qreal m_downMovemnet;

    float errorAccumulator;
    qreal m_prevYparm;

public:
    ScrollBar(qint32 startX, qint32 startY, qint32 width,
              qint32 totalSteps, qint32 unitStepSize,
              qint32 totalElements, qint32 sliderPosition,
              QWidget *parent = 0,
              SCROLLBAR_TYPE_e scrollbarType = VERTICAL_SCROLLBAR,
              qint32 indexInPage = 0,
              bool isEnabled = true,
              bool catchKey = true, bool cameraList =false);
    ~ScrollBar();

    void createElements();
    void setGeometryForElements();
    void updateBarGeometry(qreal offSet);

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeUpKeyAction();
    void takeDownKeyAction();

    bool eventFilter (QObject *, QEvent *);
    void wheelEvent (QWheelEvent *);
    void mouseMoveEvent (QMouseEvent *);
    void mousePressEvent (QMouseEvent *);
    void mouseReleaseEvent (QMouseEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);
    void updateTotalElement(qint32 element);

signals:
    void sigScroll(int numberOfSteps);
    void sigUpdateCurrentElement(int index);

public slots:
    void slotImageClicked(int indexInPage);
    void slotUpdateCurrentElement(int index);
};

#endif // SCROLLBAR_H
