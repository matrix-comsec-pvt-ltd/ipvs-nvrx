///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : DVR ( Digital Video Recorder)
//   Owner        : Shruti Sahni
//   File         : mainwindow.cpp
//   Description  : This is menubutton file for multiDvrClient.
/////////////////////////////////////////////////////////////////////////////
#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <QWidget>
#include "TextLabel.h"
#include "KeyBoard.h"
#include "../NavigationControl.h"
#include <QTimer>

#define UNIT_BORDER_THICKNESS       1
#define TWICE_BORDER_THICKNESS      SCALE_WIDTH(2)
#define THRICE_BORDER_THICKNESS     SCALE_WIDTH(3)


class MenuButton : public KeyBoard, public NavigationControl
{
    Q_OBJECT
protected:
    int m_index;
    int m_width;
    int m_height;
    int m_horizontalOffset;
    int  m_startX, m_startY;
    bool m_showClickedImage;
    bool m_isMouseHover;
    bool m_changeTextColorOnClick;
    bool m_isClickOnClickNeeded;
    bool m_isClickEffectNeeded;
    bool m_isDeletionStart;
    bool m_isBoarderNeed;
    int  m_deviceIndex;

    QRect m_mainRect;
    QRect m_topRect;
    QRect m_bottomRect;
    QRect m_leftRect;
    QRect m_rightRect;
    QRect m_bottomRect_1;
    QRect m_bottomRect_2;
    QRect m_bottomRect_3;
    QRect m_leftRect_1;
    QRect m_leftRect_2;
    QRect m_leftRect_3;
    QString m_label;
    TextLabel* m_textLabel;
    QTimer* m_clickEffectTimer;
    QString m_fontColor;
    QString m_backgroundColor;

public:
    MenuButton(int index,
               int width,
               int height,
               QString label,
               QWidget *parent,
               int horizontalOffset = 0,
               int startX = 0,
               int startY = 0,
               int indexInPage = 0,
               bool isEnabled = true,
               bool catchKey = true,
               bool isClickOnClickNeeded = false,
               bool changeTextColorOnClick = true,
               bool isClickEffectNeeded = false,
               bool isBoarderNeed = true,
               QString fontColor = NORMAL_FONT_COLOR,
               int deviceIndex =-1);
    ~MenuButton();

    void setRectanglesGeometry();
    void drawRectangles(QPainter * painter);
    virtual void resetGeometry(int xOffset, int yOffset);
    void setShowClickedImage(bool flag);
    void selectControl();
    void deSelectControl();

    void forceActiveFocus();
    void disableButton(bool isDisable);
    virtual void setIsEnabled(bool isEnable);

    virtual void takeEnterKeyAction();

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals:
    void sigButtonClicked(int index);
    void sigUpdateCurrentElement(int index);
    void sigShowHideToolTip(quint16 startX,
                            quint16 startY,
                            int index,
                            bool toShowTooltip);
   void sigShowHideDeviceToolTip(quint16 startX,
                                 quint16 startY,
                                 int deviceIndex,
                                 int index,
                                bool toShowTooltip);

protected slots:
    virtual void slotClickEffectTimerout();
};

#endif // MENUBUTTON_H
