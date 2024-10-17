#include "IpChangeProcessRequest.h"
#include <QPainter>

IpChangeProcessRequest::IpChangeProcessRequest(QWidget *parent) :
    QWidget(parent)
{
    this->setGeometry (0, 0, parent->width(), parent->height());
    this->show ();
}

void IpChangeProcessRequest::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor color;
    QRect mainRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT));
    color.setAlpha(150);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(mainRect, SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    QFont font = QFont();
    font.setFamily(NORMAL_FONT_FAMILY);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0.75);
    font.setWordSpacing(2);
    font.setPixelSize(SCALE_FONT(HEADING_FONT_SIZE));

    painter.setFont(font);
    painter.setPen(QColor(HIGHLITED_FONT_COLOR));

    quint8 height = QFontMetrics(font).height();
    quint8 width = QFontMetrics(font).width("Processing Request");

    QRect textRect;

    textRect.setRect (SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + (mainRect.width ()- width)/2,
                      (mainRect.height ()- height)/2,
                      width,
                      height);

    painter.drawText(textRect, Qt::AlignVCenter, "Processing Request");
}
