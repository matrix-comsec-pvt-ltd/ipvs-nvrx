#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H

#include <QWidget>
#include "NavigationControl.h"
#include "ApplController.h"
#include "EnumFile.h"
#include <QTimer>
#include "KeyBoard.h"

typedef enum
{
    VERTICAL_SLIDER,
    HORIZONTAL_SLIDER
}SLIDER_TYPE_e;

class SliderControl : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    SLIDER_TYPE_e m_sliderType;
    int m_startX, m_startY, m_width, m_height;
    int m_activeWidth, m_activeHeight;
    int m_horizontalMargin, m_verticalMargin;
    QString m_imagePath;
    int m_currentImageType;
    int m_currentValue;
    bool m_isActiveBarNeeded;
    bool m_isControlOnEnter;
    bool m_isHoverConfinedToImageOnly;
    bool m_isValueChangeOnClickAllowed;

    bool m_activeAreaClicked;
    bool m_mouseClickedOnImage;
    bool m_remoteSlidingMode;

    QRect m_activeBar;
    QRect m_imageRect;
    QPixmap m_image;
    QPoint m_lastClickPoint;

    QRect m_rect;
    QRect m_activeRect;

    QTimer *clickEffectTimer;

public:
   SliderControl(int startX,
                 int startY,
                 int width,
                 int height,
                 int activeWidth,
                 int activeHeight,
                 QString imagePath,
                 int currentValue = 0,
                 QWidget *parent = 0,
                 SLIDER_TYPE_e sliderType = VERTICAL_SLIDER,
                 int indexInPage = 0,
                 bool isEnabled = true,
                 bool isActiveBarNeeded = true,
                 bool isControlOnEnter = false,
                 bool isValueChangeOnClickAllowed = false,
                 bool isHoverConfinedToImageOnly = false);

   ~SliderControl();

   void setGeometryForElements();
   void changeImage(IMAGE_TYPE_e imageType);
   void resetGeometry(int Offset, bool sliderMove = false, bool toEmitSignal = true);
   void changeValue(int value);
   int getCurrentValue();
   void setCurrentValue(int value);
   void resetFlags();
   void selectControl();
   void deSelectControl();

   void forceActiveFocus();
   void setIsEnabled(bool isEnable);

   void takeUpKeyAction();
   void takeDownKeyAction();
   void takeEnterKeyAction();

   void paintEvent(QPaintEvent *);
   void mousePressEvent(QMouseEvent * event);
   void mouseReleaseEvent(QMouseEvent * event);
   void mouseMoveEvent(QMouseEvent * event);
   void focusInEvent(QFocusEvent *);
   void focusOutEvent(QFocusEvent *);
   bool eventFilter (QObject * obj, QEvent *evt);
   void wheelEvent(QWheelEvent* event);
   virtual void enterKeyPressed(QKeyEvent *event);
   virtual void navigationKeyPressed(QKeyEvent *event);
   virtual void tabKeyPressed(QKeyEvent *event);
   virtual void backTab_KeyPressed(QKeyEvent *event);

signals:
   void sigHoverInOutOnSlider(bool isHoverIn, int indexInPage);
   void sigHoverOnSlider(int value, int indexInPage);
   void sigMouseReleaseOnSlider(int value, int indexInPage);
   void sigMousePressedOnSlider(int value, int indexInPage);
   void sigValueChanged(int changedValue, int indexInpage, bool sliderMove);
   void sigUpdateCurrentElement(int index);

public slots:
   void slotClickEffectTimerTimeout();

};

#endif // SLIDERCONTROL_H
