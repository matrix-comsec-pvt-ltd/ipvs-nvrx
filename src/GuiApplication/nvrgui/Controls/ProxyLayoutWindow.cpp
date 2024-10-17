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
#include "ProxyLayoutWindow.h"
#include <QPainter>
#include "EnumFile.h"
#include "ApplController.h"

#define BORDER_WIDTH        2

ProxyLayoutWindow::ProxyLayoutWindow(QWidget *parent) :
    QWidget(parent)
{
    //border for window
    m_topBorder = new Rectangle(0,
                                0,
                                this->width(),
                                SCALE_HEIGHT(BORDER_WIDTH),
                                WINDOW_GRID_COLOR,
                                this);
    m_bottomBorder = new Rectangle(0,
                                   (this->height() - SCALE_HEIGHT(BORDER_WIDTH)),
                                   this->width(),
                                   SCALE_HEIGHT(BORDER_WIDTH),
                                   WINDOW_GRID_COLOR,
                                   this);
    m_leftBorder = new Rectangle(0,
                                 0,
                                 SCALE_WIDTH(BORDER_WIDTH),
                                 this->height(),
                                 WINDOW_GRID_COLOR,
                                 this);
    m_rightBorder = new Rectangle((this->width() - SCALE_WIDTH(BORDER_WIDTH)),
                                  0,
                                  SCALE_WIDTH(BORDER_WIDTH),
                                  this->height(),
                                  WINDOW_GRID_COLOR,
                                  this);


    this->show ();
    this->setEnabled(true);
    this->setMouseTracking(true);
}

ProxyLayoutWindow::~ProxyLayoutWindow ()
{
    delete m_bottomBorder;
    delete m_topBorder;
    delete m_leftBorder;
    delete m_rightBorder;
}

void ProxyLayoutWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    QColor color;
    color = QColor(SHADOW_FONT_COLOR);

    painter.fillRect(SCALE_WIDTH(BORDER_WIDTH),
                     SCALE_HEIGHT(BORDER_WIDTH),
                     (this->width() - (2 * SCALE_WIDTH(BORDER_WIDTH))),
                     (this->height() - (2 * SCALE_HEIGHT(BORDER_WIDTH))),
                     color);
}

void ProxyLayoutWindow::resizeEvent(QResizeEvent*)
{
    m_topBorder->resetGeometry(0,
                               0,
                               this->width(),
                               SCALE_HEIGHT(BORDER_WIDTH));
    m_bottomBorder->resetGeometry(0,
                                  (this->height() - SCALE_HEIGHT(BORDER_WIDTH)),
                                  this->width(),
                                  SCALE_HEIGHT(BORDER_WIDTH));
    m_leftBorder->resetGeometry(0,
                                0,
                                SCALE_WIDTH(BORDER_WIDTH),
                                this->height());
    m_rightBorder->resetGeometry((this->width() - SCALE_WIDTH(BORDER_WIDTH)),
                                 0,
                                 SCALE_WIDTH(BORDER_WIDTH),
                                 this->height());
}
