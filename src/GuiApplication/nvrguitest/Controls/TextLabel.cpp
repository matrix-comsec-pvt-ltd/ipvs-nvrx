#include "TextLabel.h"
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>

TextLabel::TextLabel(int startX,
                     int startY,
                     int fontSize,
                     QString text,
                     QWidget* parent,
                     QString fontColor,
                     QString fontFamily,
                     TEXTLABEL_ALIGNMENT_e align,
                     int isLight, bool isSetunderLine, quint16 maxWidth, int indexInPage) : QWidget(parent)
{
	setObjectName("TXT_LBL");

    m_startX = startX;
    m_startY = startY;
    m_fontSize = fontSize;
    m_fontFamily = fontFamily;
    m_fontColor = fontColor;
    m_text = text;
    isBold = false;
    lighterVal = isLight;
    alignType = align;
    m_isSetUnderLine = isSetunderLine;
    m_maxWidth = maxWidth;
    m_indexInPage = indexInPage;
	
    setGeometryForText();

    this->installEventFilter(this);
    this->setMouseTracking (true);
    this->show ();
}

QFont TextLabel::getFont(QString fontFamily, quint8 fontSize, bool isBold, bool isSetUnderLine)
{
    float letterSpacing, wordSpacing;
    QFont font = QFont();

    font.setFamily(NORMAL_FONT_FAMILY);
    if(isBold == true)
        font.setBold (true);
    if(isSetUnderLine)
        font.setUnderline (true);

    if(fontFamily == NORMAL_FONT_FAMILY)
    {
        letterSpacing = 0.75;
        wordSpacing = (2);
        if(isBold == true)
            font.setWeight(QFont::Bold);
    }
    else
    {
        letterSpacing = 1;
        wordSpacing = (2);
        font.setWeight(QFont::Bold);
    }

    font.setLetterSpacing(QFont::AbsoluteSpacing, letterSpacing);
    font.setWordSpacing(wordSpacing);
    font.setPixelSize(fontSize);
    return font;
}

void TextLabel::getWidthHeight(QFont font, QString text, quint16 &width, quint16 &height)
{
    if(text.contains("\n") == true)
    {
        QStringList list;
        quint16 maxWidth = 0;

        list.clear ();
        list = text.split("\n");

        for(quint8 index = 0; index < list.size(); index++)
        {
            width = QFontMetrics(font).width(list.at(index));

            if(maxWidth < width)
            {
                maxWidth = width;
            }
        }

        height = QFontMetrics(font).height() * list.size();
        width = maxWidth;
    }
    else
    {
        height = QFontMetrics(font).height();
        width = QFontMetrics(font).width(text);
    }
}

void TextLabel::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setFont(m_font);
    if(lighterVal == 0)
    {
        painter.setPen(QColor(m_fontColor));
    }
    else
    {
        painter.setPen(QColor(m_fontColor).lighter (lighterVal));
    }
    painter.drawText(textRect, Qt::AlignVCenter, m_text);
}

void TextLabel :: SetBold(bool state)
{
    isBold = state ;
    setGeometryForText();
}

void TextLabel::changeText(QString text)
{
    m_text = text;
    setGeometryForText(false);
}

QString TextLabel::getText()
{
    return m_text;
}

void TextLabel::changeColor(QString fontColor, int lightVal)
{
    m_fontColor = fontColor;
    lighterVal = lightVal;
}

void TextLabel::setOffset(quint16 xOffset, quint16 yOffset)
{
    m_startX = xOffset;
    m_startY = yOffset;
    setGeometryForText(false);
}

void TextLabel::setOffset(quint16 xOffset, quint16 yOffset, TEXTLABEL_ALIGNMENT_e alignment)
{
    alignType = alignment;
    setOffset(xOffset, yOffset);
}

void TextLabel::setFontSize(quint8 fontSize)
{
    m_fontSize = fontSize;
    setGeometryForText();
}

void TextLabel::setUnderline(bool state)
{
    m_font.setUnderline(state);
}

void TextLabel::resetGeometry(int startX)
{
    m_startX = startX;
    setGeometryForText();
}

void TextLabel::setGeometryForText(bool fontResetNeeded)
{
    quint16 width = 0, height = 0;

    if(fontResetNeeded == true)
    {
        m_font = getFont(m_fontFamily, m_fontSize, isBold,m_isSetUnderLine);
    }
    getWidthHeight(m_font, m_text, width, height);

    width = (m_maxWidth != 0) ? m_maxWidth : width;

    switch(alignType)
    {
    case ALIGN_START_X_START_Y:
        this->setGeometry(m_startX,
                          m_startY,
                          width,
                          height);
        break;

    case ALIGN_START_X_CENTRE_Y:
        this->setGeometry(m_startX,
                          (m_startY - (height / 2)),
                          width,
                          height);
        break;

    case ALIGN_START_X_END_Y:
        this->setGeometry(m_startX,
                          (m_startY - height),
                          width,
                          height);
        break;

    case ALIGN_CENTRE_X_START_Y:
        this->setGeometry((m_startX - (width / 2)),
                          m_startY,
                          width,
                          height);
        break;

    case ALIGN_CENTRE_X_CENTER_Y:
        this->setGeometry((m_startX - (width / 2)),
                          (m_startY - (height / 2)),
                          width,
                          height);
        break;

    case ALIGN_CENTRE_X_END_Y:
        this->setGeometry((m_startX - (width / 2)),
                          (m_startY - height),
                          width,
                          height);
        break;

    case ALIGN_END_X_START_Y:
        this->setGeometry((m_startX - width),
                          m_startY,
                          width,
                          height);
        break;

    case ALIGN_END_X_CENTRE_Y:
        this->setGeometry((m_startX - width),
                          (m_startY - (height / 2)),
                          width,
                          height);
        break;

    case ALIGN_END_X_END_Y:
        this->setGeometry((m_startX - width),
                          (m_startY - height),
                          width,
                          height);
        break;

    default:
        break;
    }
    textRect.setRect(0, 0, width, height);
}

void TextLabel::forceActiveFocus()
{
    this->setFocus();
}

void TextLabel::mousePressEvent(QMouseEvent * event)
{
    if((textRect.contains(event->pos()))
            && (event->button() == Qt::LeftButton))
    {
        if(!this->hasFocus())
        {
            forceActiveFocus();
//            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }

    QWidget:: mousePressEvent (event);
}

void TextLabel::mouseReleaseEvent(QMouseEvent * event)
{
    if((textRect.contains(event->pos()))
            && (event->button() == Qt::LeftButton))
    {
        emit sigTextClick(m_indexInPage);
    }
    QWidget::mouseReleaseEvent (event);
}

void TextLabel::mouseMoveEvent (QMouseEvent *event)
{
    QWidget:: mouseMoveEvent (event);
}

void TextLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if((textRect.contains(event->pos()))
            && (event->button() == Qt::LeftButton))
    {
        emit sigTextDoubleClicked(m_indexInPage);
    }
	QWidget::mouseDoubleClickEvent (event);
}

bool TextLabel::eventFilter(QObject *object, QEvent *event)
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


