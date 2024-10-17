#include "CameraSearchProcess.h"
#include <QPainter>

CameraSearchProcess::CameraSearchProcess(quint8 startx,
                                         quint8 starty,
                                         quint16 width,
                                         quint16 height,
                                         QWidget *parent) :
    QWidget(parent)
{
    this->setGeometry (startx,starty,width,height);
    this->show ();
}

void CameraSearchProcess::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRect (QRect(0,
                            0,
                            this->width (),
                            this->height ()));

    QFont font = QFont();
    font.setFamily(NORMAL_FONT_FAMILY);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0.75);
    font.setWordSpacing(2);
    font.setPixelSize(SCALE_FONT(HEADING_FONT_SIZE));

    painter.setFont(font);
    painter.setPen(QColor(HIGHLITED_FONT_COLOR));

    quint8 height = QFontMetrics(font).height();
    quint8 width = QFontMetrics(font).width("Searching...");

    QRect textRect;

    textRect.setRect ((this->width ()- width)/2,
                      (this->height ()- height)/2,
                      width,
                      height);

    painter.drawText(textRect, Qt::AlignVCenter, QApplication::translate(QT_TRANSLATE_STR, "Searching..."));
}
