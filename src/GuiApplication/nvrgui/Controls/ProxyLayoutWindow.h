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
//   File         : ProxyLayoutWindow.cpp
//   Description  : This is ProxyLayoutWindow file for multiDvrClient.
/////////////////////////////////////////////////////////////////////////////
#ifndef PROXYLAYOUTWINDOW_H
#define PROXYLAYOUTWINDOW_H

#include "Controls/Rectangle.h"
#include <QWidget>

class ProxyLayoutWindow : public QWidget
{
    Rectangle* m_leftBorder;
    Rectangle* m_rightBorder;
    Rectangle* m_topBorder;
    Rectangle* m_bottomBorder;

public:
    explicit ProxyLayoutWindow(QWidget *parent = 0);
    ~ProxyLayoutWindow();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif // PROXYLAYOUTWINDOW_H
