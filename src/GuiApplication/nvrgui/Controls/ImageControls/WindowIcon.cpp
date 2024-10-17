#include "WindowIcon.h"
#include <QPainter>
#include <QPaintEvent>

#define BORDER_WIDTH                        2
#define MARGIN(m_windowIconType)            ((m_windowIconType == ICON_TYPE_8X8) ? SCALE_WIDTH(4) : SCALE_WIDTH(5))
#define LEFT_MARGIN(m_windowIconType)       (SCALE_WIDTH(BORDER_WIDTH) + MARGIN(m_windowIconType) + ((m_windowIconType == ICON_TYPE_8X8) ? (0) : SCALE_WIDTH(15)))
#define TOP_MARGIN(m_windowIconType)        (SCALE_HEIGHT(BORDER_WIDTH) + MARGIN(m_windowIconType))

WindowIcon::WindowIcon(OSD_POSITION_e alignmentType, int alignmentOffset, QString imgSource,
                       WINDOW_ICON_TYPE_e windowIconType, QWidget *parent) : QWidget(parent),
                        m_alignmentType(alignmentType), m_imgSource(imgSource), m_windowIconType(windowIconType)
{
    m_alignmentOffset = alignmentOffset;
    updateImageSource(m_imgSource);
    setGeometryForElements(parent->width(),
                           parent->height());
    QWidget::setVisible(true);
}

void WindowIcon::resetGeometry(int width, int height)
{
    setGeometryForElements(width, height);
}



void WindowIcon::updateImageSource(QString imgSource)
{
    if(m_imgSource != imgSource)
    {
        m_imgSource = imgSource;
        m_image = QPixmap(m_imgSource);
        SCALE_IMAGE(m_image);
        setGeometryForElements(parentWidget()->width(),
                               parentWidget()->height());
        update();
    }
}

void WindowIcon::setGeometryForElements(int width, int height)
{
    m_imageRect.setRect(0, 0, m_image.width(), m_image.height());
    switch(m_alignmentType)
    {
    case OSD_NONE:
        m_startX = 0;
        m_startY = 0;
        break;

    case TOP_RIGHT:
        m_startX = width - LEFT_MARGIN(m_windowIconType) - ((m_image.width() + MARGIN(m_windowIconType)) * (m_alignmentOffset + 1)) -
                ((m_windowIconType == ICON_TYPE_8X8) ? (SCALE_WIDTH(14)) : SCALE_WIDTH(42/(ICON_TYPE_5X5 - m_windowIconType + 1)));
        m_startY = TOP_MARGIN(m_windowIconType);      
        break;

    case TOP_LEFT:
        m_startX = LEFT_MARGIN(m_windowIconType) + SCALE_WIDTH(10) + ((m_image.width() + MARGIN(m_windowIconType)) * m_alignmentOffset);
        m_startY = TOP_MARGIN(m_windowIconType);
        break;

    case BOTTOM_LEFT:
        m_startX = LEFT_MARGIN(m_windowIconType) - SCALE_WIDTH(15) + ((m_image.width() + MARGIN(m_windowIconType)) * m_alignmentOffset);
        m_startY = height - m_image.height() - TOP_MARGIN(m_windowIconType);
        break;

    case BOTTOM_RIGHT:
        m_startX = width - LEFT_MARGIN(m_windowIconType) - ((m_image.width() + MARGIN(m_windowIconType)) * (m_alignmentOffset + 1));
        m_startY = height - m_image.height() - TOP_MARGIN(m_windowIconType);
        break;

    default:
        break;
    }

    if((m_image.width() == 0) || (m_image.height() == 0) || (m_startX < 0) || (m_startY < 0))
    {
        m_startX = 0;
        m_startY = 0;
        this->setGeometry(m_startX, m_startY, 1, 1);
    }
    else
    {
        this->setGeometry(m_startX, m_startY, m_image.width(), m_image.height());
    }
}

void WindowIcon::changeAlignmentOffset(int alignmentOffset)
{
    m_alignmentOffset = alignmentOffset;
}

void WindowIcon::setWindowIconType(WINDOW_ICON_TYPE_e windowIconType)
{
    m_windowIconType = windowIconType;
}

void WindowIcon::changeAlignmentType(OSD_POSITION_e alignmentType)
{
    m_alignmentType = alignmentType;
    if(m_alignmentType == OSD_NONE)
    {
        setVisible(false);
    }
    else
    {
        setVisible(true);
    }
    setGeometryForElements(parentWidget()->width(),
                           parentWidget()->height());
}

void WindowIcon::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawPixmap(m_imageRect, m_image);
}
