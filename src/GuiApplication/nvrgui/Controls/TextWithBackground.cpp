#include "TextWithBackground.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

TextWithBackground::TextWithBackground(int startX,
                                       int startY,
                                       int fontSize,
                                       QString text,
                                       QWidget* parent,
                                       QString fontColor,
                                       QString fontFamily,
                                       TEXTLABEL_ALIGNMENT_e align,
                                       int isLight, bool isSetunderLine,
                                       QString backGroundColor,
                                       bool isMouseEffectNeeded,
                                       int indexInPage, quint16 maxWidth,
                                       bool isEllipssisNeeded)
    : KeyBoard(parent), NavigationControl(indexInPage, true, true),
      m_startX(startX), m_startY(startY), m_maxWidth(maxWidth), m_fontSize(fontSize), alignType(align),
      m_backColor(backGroundColor),m_fontFamily(fontFamily),
      m_isSetunderLine(isSetunderLine), m_isMouseEffectNeeded(isMouseEffectNeeded),
      m_isEllipssisNeeded(isEllipssisNeeded)
{
    QFont font;
    m_height = 0;
    m_width = 0;
    m_isBoldNedded = false;
    font = TextLabel::getFont(m_fontFamily, m_fontSize, false, isSetunderLine);
    TextLabel::getWidthHeight(font, text, m_width, m_height);

    m_width += SCALE_WIDTH(10);
    m_height += SCALE_HEIGHT(6);

    if((m_maxWidth != 0) && (m_width > m_maxWidth))
    {
        m_width = m_maxWidth;
    }

    this->setGeometry (startX,startY,m_width,m_height);

    m_startTextX = SCALE_WIDTH(5);
    m_startTextY = SCALE_HEIGHT(3);

    m_textLabel = new TextLabel(m_startTextX,
                                m_startTextY,
                                fontSize,
                                text,
                                this,
                                fontColor,
                                fontFamily,
                                align,
                                isLight,
                                isSetunderLine,
                                ((m_maxWidth == 0) ? (0) : (m_maxWidth - SCALE_WIDTH(10))),
                                0,
                                m_isEllipssisNeeded);

    this->setMouseTracking (true);
    this->installEventFilter(this);
    this->show ();
}

TextWithBackground::~TextWithBackground()
{
    if(NULL != m_textLabel)
    {
        delete m_textLabel;
        m_textLabel = NULL;
    }

}

void TextWithBackground::setFontSize (quint8 fontSize)
{
    m_fontSize = fontSize;
    m_textLabel->setFontSize(fontSize);

    QFont font;

    font = TextLabel:: getFont(m_fontFamily,m_fontSize,false,m_isSetunderLine);
    TextLabel:: getWidthHeight(font,m_textLabel->getText (), m_width,m_height);

    m_width += SCALE_WIDTH(10);
    m_height += SCALE_HEIGHT(6);

    if((m_maxWidth != 0) && (m_width > m_maxWidth))
    {
        m_width = m_maxWidth;
    }

    this->setGeometry (m_startX,m_startY,m_width,m_height);

    update ();
}

void TextWithBackground::setHeight(quint32 height)
{
    m_height = height;
    this->setGeometry (m_startX,m_startY,m_width,m_height);
    update();
}

void TextWithBackground::changeText(QString text)
{
    QFont font;
    m_width = 0;
    m_height = 0;

    font = TextLabel:: getFont(m_fontFamily,m_fontSize,m_isBoldNedded,m_isSetunderLine);
    TextLabel:: getWidthHeight(font,text, m_width,m_height);

    m_width += SCALE_WIDTH(10);
    m_height += SCALE_HEIGHT(6);

    if((m_maxWidth != 0) && (m_width > m_maxWidth))
    {
        m_width = m_maxWidth;
    }

    this->setGeometry (m_startX,m_startY,m_width,m_height);
    setAlignment();
    m_textLabel->changeText (text);

    update ();
}

QString TextWithBackground::getText () const
{
    return m_textLabel->getText ();
}

void TextWithBackground::setOffset (quint16 xOffset, quint16 yOffset)
{
    m_startX = xOffset;
    m_startY = yOffset;
    this->setGeometry (m_startX,m_startY,m_width,m_height);
    setAlignment();
    m_textLabel->setOffset (m_startTextX,m_startTextY);
    update ();
}

void TextWithBackground::setOffset(quint16 xOffset, quint16 yOffset, TEXTLABEL_ALIGNMENT_e alignment)
{
    QFont font;
    m_startX = xOffset;
    m_startY = yOffset;
    alignType = alignment;
    m_width = 0;
    m_height = 0;

    font = TextLabel:: getFont(m_fontFamily, m_fontSize, m_isBoldNedded, m_isSetunderLine);
    TextLabel:: getWidthHeight(font, getText (), m_width, m_height);

    m_width += SCALE_WIDTH(10);
    m_height += SCALE_HEIGHT(6);

    if((m_maxWidth != 0) && (m_width > m_maxWidth))
    {
        m_width = m_maxWidth;
    }

    setAlignment();
    m_textLabel->setOffset (m_startTextX,m_startTextY,alignment);
    update();
}

void TextWithBackground::setAlignment()
{
    switch(alignType)
    {
    case ALIGN_START_X_START_Y:
        m_startTextX = SCALE_WIDTH(5);
        m_startTextY = SCALE_HEIGHT(3);
        this->setGeometry(m_startX,
                          m_startY,
                          m_width,
                          m_height);

        break;

    case ALIGN_START_X_CENTRE_Y:
        m_startTextX = SCALE_WIDTH(5);
        m_startTextY = (m_height / 2);
        this->setGeometry(m_startX,
                          (m_startY - (m_height / 2)),
                          m_width,
                          m_height);
        break;

    case ALIGN_START_X_END_Y:
        m_startTextX = SCALE_WIDTH(5);
        m_startTextY = (m_height - SCALE_HEIGHT(3));

        this->setGeometry(m_startX,
                          (m_startY - m_height),
                          m_width,
                          m_height);
        break;

    case ALIGN_CENTRE_X_START_Y:
        m_startTextX = ((m_width - SCALE_WIDTH(10))/ 2);
        m_startTextY = SCALE_HEIGHT(3);
        this->setGeometry((m_startX - (m_width / 2)),
                          m_startY,
                          m_width,
                          m_height);
        break;

    case ALIGN_CENTRE_X_CENTER_Y:
        m_startTextX = ((m_width - SCALE_WIDTH(10))/ 2);
        m_startTextY = ((m_height - SCALE_HEIGHT(6)) / 2);
        this->setGeometry((m_startX - (m_width / 2)),
                          (m_startY - (m_height / 2)),
                          m_width,
                          m_height);
        break;

    case ALIGN_CENTRE_X_END_Y:
        m_startTextX = ((m_width - SCALE_WIDTH(10))/ 2);
        m_startTextY = (m_height - SCALE_HEIGHT(3));
        this->setGeometry((m_startX - (m_width / 2)),
                          (m_startY - m_height),
                          m_width,
                          m_height);
        break;

    case ALIGN_END_X_START_Y:
        m_startTextX = (m_width - SCALE_WIDTH(5));
        m_startTextY = SCALE_HEIGHT(3);
        this->setGeometry((m_startX - m_width),
                          m_startY,
                          m_width,
                          m_height);
        break;

    case ALIGN_END_X_CENTRE_Y:
        m_startTextX = (m_width - SCALE_WIDTH(5));
        m_startTextY = ((m_height - SCALE_HEIGHT(6)) / 2);
        this->setGeometry((m_startX - m_width),
                          (m_startY - (m_height / 2)),
                          m_width,
                          m_height);
        break;

    case ALIGN_END_X_END_Y:
        m_startTextX = (m_width - SCALE_WIDTH(5));
        m_startTextY = (m_height - SCALE_HEIGHT(3));
        this->setGeometry((m_startX - m_width),
                          (m_startY - m_height),
                          m_width,
                          m_height);
        break;

    default:
        break;
    }
}

void TextWithBackground::setBackGroundColor (QString backColor)
{
    m_backColor = backColor;
}

void TextWithBackground::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent (event);
    if((m_backColor != TRANSPARENTKEY_COLOR) && (m_textLabel->getText () != ""))
    {
        QPainter painter(this);
        painter.setBrush (QBrush(QColor(WINDOW_GRID_COLOR)));
        painter.setPen (Qt::NoPen);

        painter.drawRect (QRect(0,
                                0,
                                m_width,
                                m_height));

        painter.setBrush (QBrush(QColor(m_backColor)));
        painter.setPen (Qt::NoPen);

        painter.drawRect (QRect(SCALE_WIDTH(2),
                                SCALE_HEIGHT(2),
                                (m_width - SCALE_WIDTH(4)),
                                (m_height - SCALE_HEIGHT(4))));
    }
}

void TextWithBackground::mouseMoveEvent (QMouseEvent *event)
{
    QWidget:: mouseMoveEvent (event);
    if(m_isMouseEffectNeeded)
    {
        if(this->hasFocus())
        {
            selectControl();
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void TextWithBackground::mousePressEvent (QMouseEvent *event)
{
    if(m_isMouseEffectNeeded)
    {
        emit sigMousePressClick (getText ());
    }
    QWidget:: mousePressEvent (event);
}

void TextWithBackground::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void TextWithBackground::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void TextWithBackground::forceActiveFocus ()
{
    this->setFocus();
}

void TextWithBackground::selectControl()
{
    if(m_isMouseEffectNeeded)
    {
        m_textLabel->changeColor (HIGHLITED_FONT_COLOR);
        m_textLabel->update ();
    }
}

void TextWithBackground::deSelectControl()
{
    if(m_isMouseEffectNeeded)
    {
        if((m_backColor == TRANSPARENTKEY_COLOR) && (m_textLabel->getText () != ""))
        {
            m_textLabel->changeColor (NORMAL_FONT_COLOR);
            m_textLabel->update ();
        }
    }
}

void TextWithBackground::setBold (bool isBold)
{
    m_isBoldNedded = isBold;
    m_textLabel->SetBold (isBold);
}

void TextWithBackground::changeTextColor (QString fontColor)
{
    m_textLabel->changeColor (fontColor);
}

void TextWithBackground::setMaxWidth(quint16 width)
{
    m_maxWidth = width;
    m_textLabel->setMaxWidth((m_maxWidth - SCALE_WIDTH(10)));
}

bool TextWithBackground::getIsTooltipNeeded()
{
    return m_textLabel->getIsTooltipNeeded();
}

void TextWithBackground::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        if(m_isMouseEffectNeeded)
        {
            emit sigMousePressClick (getText ());
        }
    }
}

bool TextWithBackground::eventFilter(QObject *object, QEvent *event)
{
    if(event->type () == QEvent::Leave)
    {
        emit sigMouseHover(m_indexInPage,false);
    }
    else if(event->type () == QEvent::Enter)
    {
        emit sigMouseHover(m_indexInPage,true);
    }

    return QWidget::eventFilter (object, event);
}

