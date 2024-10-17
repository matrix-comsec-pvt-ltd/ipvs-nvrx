#include "ReadOnlyElement.h"
#include <QPainter>
#include <QPaintEvent>

ReadOnlyElement:: ReadOnlyElement(int startX,
                                  int startY,
                                  int tileWidth,
                                  int tileHeight,
                                  int width,
                                  int height,
                                  QString value,
                                  QWidget *parent,
                                  BGTILE_TYPE_e tileType,
                                  int pixelAlign,
                                  int valueMargin,
                                  QString label,
                                  QString suffix,
                                  QString suffixFontColor,
                                  int suffixTextSize,
                                  bool isOuterBorderNeeded)
    : BgTile(startX, startY, tileWidth, tileHeight, tileType, parent)
{
    m_textLabel = NULL;
    m_textSuffix = NULL;
    m_textValue = NULL;
    m_value = value;
    m_rectWidth = width;
    m_rectHeight = height;
    m_valueMargin = valueMargin;
    m_label = label;
    m_suffix = suffix;
    m_pixelAlign = pixelAlign;
    m_textLabelSize = NORMAL_FONT_SIZE;
    m_textValueSize = NORMAL_FONT_SIZE;
    m_textSuffixSize = suffixTextSize;  // prev default value is 10
    m_suffixFontColor = suffixFontColor;
    m_isOuterBorderNeeded = isOuterBorderNeeded;
    setGeometryForElements();
    this->show ();
}

ReadOnlyElement::~ReadOnlyElement()
{
    if(m_textLabel != NULL)
    {
        delete m_textLabel;
    }
    if(m_textSuffix != NULL)
    {
        delete m_textSuffix;
    }
    delete m_textValue;
}

void ReadOnlyElement::setGeometryForElements()
{
    int width = 0, verticalOffSet = 0;
    int textLabelWidth = 0;
    int textLabelHeight = 0;
    if(m_label != "")
    {
        textLabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textLabelSize)).width(m_label);
        textLabelHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textLabelSize)).height();
        width += (10);
    }
    int suffixWidth = 0;
    int suffixHeight = 0;
    if(m_suffix != "")
    {
        suffixWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textSuffixSize)).width(m_suffix);
        suffixHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textSuffixSize)).height();
        width += (10);
    }
    width += (m_rectWidth + textLabelWidth + suffixWidth);

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        m_pixelAlign = 0;
        break;

    case TOP_TABLE_LAYER:
        if(m_pixelAlign != -1)
        {
            m_pixelAlign += LEFT_MARGIN;
        }
        //fall through
    case TOP_LAYER:
        verticalOffSet = (TOP_MARGIN / 2);
        break;

    case BOTTOM_TABLE_LAYER:
        if(m_pixelAlign != -1)
        {
            m_pixelAlign += LEFT_MARGIN;
        }
        //fall through
    case BOTTOM_LAYER:
        verticalOffSet = -(TOP_MARGIN / 2);
        break;

    case MIDDLE_TABLE_LAYER:
        if(m_pixelAlign != -1)
        {
            m_pixelAlign += LEFT_MARGIN;
        }
        break;

    default:
        break;
    }

    if(m_pixelAlign == -1) // align image into middle of tile
    {
        if(m_width != m_rectWidth)
        {
            setRectanglesGeometry((m_width / 2),
                                  ((m_height - m_rectHeight) / 2) + verticalOffSet);
        }
        else
        {
            setRectanglesGeometry(0,
                                  ((m_height - m_rectHeight) / 2) + verticalOffSet);
        }

        if(m_label != "")
        {
            m_textLabel = new TextLabel((m_mainInnerRect.x() - (10) - textLabelWidth),
                                        (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                        m_textLabelSize,
                                        m_label,
                                        this);
        }

        if(m_suffix != "")
        {
            m_textSuffix = new TextLabel((m_mainInnerRect.x() + m_mainInnerRect.width() + (10)),
                                         (((m_height - suffixHeight) / 2) + verticalOffSet),
                                         m_textSuffixSize,
                                         m_suffix,
                                         this,
                                         m_suffixFontColor);
        }
    }
    else
    {
        if(m_label != "")
        {
            m_textLabel = new TextLabel(m_pixelAlign,
                                        (((m_height -textLabelHeight) / 2) + verticalOffSet),
                                        m_textLabelSize,
                                        m_label,
                                        this);
            textLabelWidth += (10);
        }

        setRectanglesGeometry((m_pixelAlign + textLabelWidth),
                              (((m_height - m_rectHeight) / 2) + verticalOffSet));


        if(m_suffix != "")
        {
            m_textSuffix = new TextLabel((m_pixelAlign + textLabelWidth + m_mainInnerRect.width() + (10)),
                                         (((m_height - suffixHeight) / 2) + verticalOffSet),
                                         m_textSuffixSize,
                                         m_suffix,
                                         this,
                                         m_suffixFontColor);
        }
    }

    if(m_valueMargin == -1) // align center
    {
        m_textValue = new TextLabel((m_mainInnerRect.x() + (m_rectWidth / 2)),
                                    ((m_height / 2) + verticalOffSet),
                                    m_textValueSize,
                                    m_value,
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_CENTRE_X_CENTER_Y);
    }
    else
    {
        m_textValue = new TextLabel((m_valueMargin + m_mainInnerRect.x()),
                                    ((m_height / 2) + verticalOffSet),
                                    m_textValueSize,
                                    m_value,
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y);
    }
}

void ReadOnlyElement::setRectanglesGeometry(int xAlignment, int yAlingnment)
{
    m_mainInnerRect.setRect(xAlignment,
                       yAlingnment,
                       m_rectWidth,
                       m_rectHeight);
    m_topRect.setRect(xAlignment,
                      yAlingnment,
                      m_rectWidth,
                      UNIT_WIDTH);
    m_bottomRect.setRect(xAlignment,
                         (m_rectHeight - UNIT_WIDTH + yAlingnment),
                         m_rectWidth,
                         UNIT_WIDTH);
    m_leftRect.setRect(xAlignment,
                       yAlingnment,
                       UNIT_WIDTH,
                       m_rectHeight);
    m_rightRect.setRect((m_rectWidth - UNIT_WIDTH + xAlignment),
                        yAlingnment,
                        UNIT_WIDTH,
                        m_rectHeight);
}

void ReadOnlyElement::drawRectangles(QPainter * painter)
{
    //main rectangle
    painter->setBrush(QBrush(QColor(NORMAL_BKG_COLOR),Qt::SolidPattern));
    painter->drawRect(m_mainInnerRect);

    //right border
    painter->setBrush(QBrush(QColor(BORDER_2_COLOR),Qt::SolidPattern));
    painter->drawRect(m_rightRect);

    //bottom border
    painter->setBrush(QBrush(QColor(BORDER_1_COLOR),Qt::SolidPattern));
    painter->drawRect(m_bottomRect);

    //top border
    painter->setBrush(QBrush(QColor(BORDER_2_COLOR), Qt::SolidPattern));
    painter->drawRect(m_topRect);

    //left border
    painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
    painter->drawRect(m_leftRect);
}

void ReadOnlyElement::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    if(m_isOuterBorderNeeded)
    {
        drawRectangles(&painter);
    }
}

void ReadOnlyElement::changeValue (QString text)
{
    m_value = text;
    m_textValue->changeText(text);
    update();
}

QString ReadOnlyElement::getCurrentValue () const
{
    return m_value;
}
