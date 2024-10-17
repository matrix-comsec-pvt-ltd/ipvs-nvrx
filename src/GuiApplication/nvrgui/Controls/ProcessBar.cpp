#include "ProcessBar.h"

#include <QPainter>
#include <QPaintEvent>

#define PROCESS_IMAGE_PATH  IMAGE_PATH"AnimatedImage/Processing.png"

ProcessBar::ProcessBar(int xParam,
                       int yParam,
                       int width,
                       int height,
                       int radius,
                       QWidget *parent)
    :QWidget(parent), roundRadius(radius), image(QPixmap(PROCESS_IMAGE_PATH))
{
    SCALE_IMAGE(image);
    this->setGeometry (xParam,
                       yParam,
                       width,
                       height);

    mainRect.setRect (0, 0, width, height);
    imageRect.setRect ((this->width () - image.width ())/2,
                       (this->height ()- image.height())/2,
                       image.width (),
                       image.height ());
    QWidget::setVisible(false);
}

void ProcessBar::loadProcessBar()
{
    setVisible(true);
}

bool ProcessBar::isLoadedProcessBar()
{
    return (isVisible());
}

void ProcessBar::unloadProcessBar()
{
    setVisible(false);
}

void ProcessBar::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    QColor bgColor = QColor(PROCESS_BKG_COLOR);
    bgColor.setAlphaF(0.55);
    painter.setPen(Qt::NoPen);
    painter.setBrush (QBrush(bgColor));
    painter.drawRoundedRect(mainRect, roundRadius, roundRadius);
    painter.drawPixmap(imageRect, image);
}
