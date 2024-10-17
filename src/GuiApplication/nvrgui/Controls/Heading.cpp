#include "Heading.h"

#define SHADOW_LEFT_MARGIN      SCALE_WIDTH(3)

#define SUB_SHADOW_LEFT_MARGIN  SCALE_WIDTH(2)

Heading::Heading(int centerX,
                 int centerY,
                 QString headingText,
                 QWidget *parent,
                 HEADING_TYPE_e type,
                 quint8 fontSize) :
    QWidget(parent), m_headingText(headingText)
{
    m_centerX = centerX ;
    m_centerY = centerY ;
    if(type == HEADING_TYPE_1)
    {
        if(fontSize == MAX_FONT_VALUE)
        {
            m_fontSize = SCALE_FONT(HEADING_FONT_SIZE);
        }
        else
        {
            m_fontSize = fontSize;
        }
        m_shadowMargin = SHADOW_LEFT_MARGIN;
    }
    else
    {
        if(fontSize == MAX_FONT_VALUE)
        {
            m_fontSize = SCALE_FONT(SUB_HEADING_FONT_SIZE);
        }
        else
        {
            m_fontSize = fontSize;
        }
        m_shadowMargin = SUB_SHADOW_LEFT_MARGIN;
    }
    m_headingText = headingText;
    labelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_fontSize)).width(headingText);
    m_shadow = new TextLabel(m_shadowMargin,
                             m_shadowMargin,
                             m_fontSize,
                             m_headingText,
                             this,
                             SHADOW_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                             0, 0, labelWidth);

    m_mainHeading = new TextLabel(0,
                                  0,
                                  m_fontSize,
                                  m_headingText,
                                  this,
                                  HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                  0, 0, labelWidth);

    setGeometryForElements();
    this->show();
}

Heading ::~Heading()
{
    delete m_mainHeading;
    delete m_shadow;
}

void Heading::setGeometryForElements()
{
    QFont font = TextLabel::getFont (NORMAL_FONT_FAMILY, m_fontSize);
    quint32 width = QFontMetrics(font).width(m_headingText) + m_shadowMargin;
    quint32 height = QFontMetrics(font).height() + m_shadowMargin;

    this->setGeometry (QRect((m_centerX - (width / 2)),
                             (m_centerY - (height / 2)),
                             width,
                             height));
}

void Heading::changeHeadingText(QString text)
{
    if(text != m_headingText)
    {
        quint16 startX = this->x();
        quint16 startY = this->y();
        quint16 width = this->width();
        quint16 height = this->height();
        m_headingText = text;
        QFont font = TextLabel::getFont (NORMAL_FONT_FAMILY, m_fontSize);

        m_mainHeading->changeText(text, QFontMetrics(font).width(m_headingText) + m_shadowMargin);
        m_shadow->changeText(text);
        setGeometryForElements();
        if((startX == this->x())
                && (startY == this->y())
                && (width == this->width())
                && (height == this->height()))
        {
            update();
        }
    }
}

void Heading::resetGeometry(int centerX, int centerY)
{
    m_centerX = centerX;
    m_centerY = centerY;
    setGeometryForElements();
}
