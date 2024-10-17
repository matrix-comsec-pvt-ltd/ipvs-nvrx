#include "TextLabel.h"
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QTranslator>
#include <QToolTip>

TextLabel::TextLabel(qreal startX,
                     qreal startY,
                     int fontSize,
                     QString text,
                     QWidget* parent,
                     QString fontColor,
                     QString fontFamily,
                     TEXTLABEL_ALIGNMENT_e align,
                     int isLight, bool isSetunderLine, qreal maxWidth,
                     int indexInPage, bool isEllipssisNeeded, Qt::AlignmentFlag alignF, qreal leftMargin) : KeyBoard(parent),
    NavigationControl(indexInPage, true)
{
    setObjectName("TXT_LBL");

    m_startX = startX;
    m_startY = startY;
    m_fontSize = fontSize;
    m_fontFamily = fontFamily;
    m_fontColor = fontColor;

    if (text == "")
    {
        m_text = "";
    }
    else
    {
        m_text = QApplication::translate(QT_TRANSLATE_STR, text.toUtf8().constData());
        if(m_text == "")
        {
            m_text = text;
        }
    }

    isBold = false;
    lighterVal = isLight;
    alignType = align;
    m_isSetUnderLine = isSetunderLine;
    m_maxWidth = maxWidth;
    m_isEllipssisNeeded = isEllipssisNeeded;
    m_isTooltipNeeded = false;
    m_ellipssisText2 = text;
    m_isTruncted = false;
    alignFlag  = alignF;
    m_letterSpacing = 0;
    m_wordSpacing = 0;
    m_leftMargin = leftMargin;
    setGeometryForText();

    this->installEventFilter(this);
    this->setMouseTracking (true);
    this->show ();
}

void TextLabel::slotShowHideToolTip(bool isHoverIn)
{
    if (false == m_isTruncted)
    {
        return;
    }

    if (false == isHoverIn)
    {
        this->setToolTip("");
        return;
    }

    QString padding = "" + QString::number(SCALE_WIDTH(2)) + "px";
    QString border = "" + QString::number(SCALE_WIDTH(2)) + "px";
    QString borderRadius = "" + QString::number(SCALE_WIDTH(5)) +"px";
    QString fontWidth = "" + QString::number(NORMAL_FONT_SIZE) +"px";

    QString style = "QToolTip \
    { \
        padding: %1; \
        font-size: %4; \
        color: #528dc9; \
        background-color: #161616; \
        border: %2 solid #606060; \
        border-radius: %3; \
        font-family: %5; \
    }";

    this->setStyleSheet(style.arg(padding).arg(border).arg(borderRadius).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
    this->setToolTip(QApplication::translate(QT_TRANSLATE_STR, m_ellipssisText2.toUtf8().constData()));
}

QFont TextLabel::getFont(QString fontFamily, quint8 fontSize, bool tIsBold, bool isSetUnderLine)
{
    float letterSpacing, wordSpacing;
    QFont font = QFont();

    font.setFamily(NORMAL_FONT_FAMILY);
    if(tIsBold == true)
    {
        font.setBold (true);
    }

    if(isSetUnderLine)
    {
        font.setUnderline (true);
    }

    if(fontFamily == NORMAL_FONT_FAMILY)
    {
        letterSpacing = 0.75;
        wordSpacing = SCALE_WIDTH(2);
        if(tIsBold == true)
        {
            font.setWeight(QFont::Bold);
        }
    }
    else
    {
        letterSpacing = 1;
        wordSpacing = SCALE_WIDTH(2);
        font.setWeight(QFont::Bold);
    }

    font.setLetterSpacing(QFont::AbsoluteSpacing, letterSpacing);
    font.setWordSpacing(wordSpacing);
    font.setPixelSize(fontSize);
    return font;
}

void TextLabel::getWidthHeight(QFont font, QString text, qreal &width, qreal &height)
{
    if (false == text.contains("\n"))
    {
        height = QFontMetrics(font).height();
        width = QFontMetrics(font).width(text);
        return;
    }

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

void TextLabel::getWidthHeight(QFont font, QString text, quint16 &width, quint16 &height)
{
    if(false == text.contains("\n"))
    {
        height = QFontMetrics(font).height();
        width = QFontMetrics(font).width(text);
        return;
    }

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

    painter.drawText(textRect, alignFlag, QApplication::translate(QT_TRANSLATE_STR, m_ellipssisText.toUtf8().constData()));
}

void TextLabel :: SetBold(bool state)
{
    isBold = state ;
    setGeometryForText();
}

void TextLabel::setAlignment(Qt::AlignmentFlag align)
{
    alignFlag =align;
}

void TextLabel::changeText(QString text, qreal maxWidth, bool forceWidthSetF)
{
    m_ellipssisText2 = text;

    if ((true == forceWidthSetF) || (maxWidth))
    {
        m_maxWidth = maxWidth;
    }

    if (text == "")
    {
        m_text = "";
    }
    else
    {
        m_text = QApplication::translate(QT_TRANSLATE_STR, text.toUtf8().constData());
        if (m_text == "")
        {
            m_text = text;
        }
    }

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

void TextLabel::setOffset(qreal xOffset, qreal yOffset)
{
    m_startX = xOffset;
    m_startY = yOffset;
    setGeometryForText(false);
}

void TextLabel::setOffset(qreal xOffset, qreal yOffset, TEXTLABEL_ALIGNMENT_e alignment)
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

void TextLabel::resetGeometry(qreal startX)
{
    m_startX = startX;
    setGeometryForText();
}

void TextLabel::resetGeometry(qreal iStartX,qreal iStartY)
{
    /* Update TextLabel geometry as per the StartX and StartY Value received */
	m_startX = iStartX;
	m_startY = iStartY;
	setGeometryForText();
}

void TextLabel::generateEllipssisText()
{
    quint16 width;
    QStringList list;
    quint16 maxWidth = m_maxWidth - (QFontMetrics(m_font).width("..")) - m_leftMargin;
    m_ellipssisText = QString("");

    list.clear ();
    list = m_text.split("\n");

    for(quint8 index = 0; index < list.size(); index++)
    {
        if(index > 0)
        {
            m_ellipssisText += QString("\n");
        }

        QString tempStr = list.at(index);
        width = QFontMetrics(m_font).width(tempStr);
        if(width > (m_maxWidth - m_leftMargin))
        {
            while(width > maxWidth)
            {
                tempStr = tempStr.left(tempStr.length() - 1);
                width = QFontMetrics(m_font).width(tempStr);
            }
            tempStr += QString("..");
            m_isTooltipNeeded = true;
            m_isTruncted = true;
        }
        m_ellipssisText += tempStr;
    }
}

void TextLabel::setMaxWidth(qreal width)
{
    m_maxWidth = width;
}

bool TextLabel::getIsTooltipNeeded()
{
    return m_isTooltipNeeded;
}

void TextLabel::setGeometryForText(bool fontResetNeeded)
{
    qreal width = 0, height = 0, width1 = 0;

    if(fontResetNeeded == true)
    {
        m_font = getFont(m_fontFamily, m_fontSize, isBold,m_isSetUnderLine);
    }

    getWidthHeight(m_font, m_text, width, height);
    m_ellipssisText = m_text;
    m_isTooltipNeeded = false;

    if(m_maxWidth != 0)
    {
        if(width > (m_maxWidth - m_leftMargin))
        {
            m_isTooltipNeeded = true;
            width = m_maxWidth;
            generateEllipssisText();
        }
    }
    else
    {
       getWidthHeight(m_font, m_ellipssisText2, width1, height);
       if(width > width1)
       {
           m_isTooltipNeeded = true;
           m_maxWidth = width1;
           width = m_maxWidth;
           generateEllipssisText();
       }
    }

    switch(alignType)
    {
        case ALIGN_START_X_START_Y:
            this->setGeometry(m_startX, m_startY, width, height);
            break;

        case ALIGN_START_X_CENTRE_Y:
            this->setGeometry(m_startX, (m_startY - (height / 2)), width, height);
            break;

        case ALIGN_START_X_END_Y:
            this->setGeometry(m_startX, (m_startY - height), width, height);
            break;

        case ALIGN_CENTRE_X_START_Y:
            this->setGeometry((m_startX - (width / 2)), m_startY, width, height);
            break;

        case ALIGN_CENTRE_X_CENTER_Y:
            this->setGeometry((m_startX - (width / 2)), (m_startY - (height / 2)), width, height);
            break;

        case ALIGN_CENTRE_X_END_Y:
            this->setGeometry((m_startX - (width / 2)), (m_startY - height), width, height);
            break;

        case ALIGN_END_X_START_Y:
            this->setGeometry((m_startX - width), m_startY, width, height);
            break;

        case ALIGN_END_X_CENTRE_Y:
            this->setGeometry((m_startX - width), (m_startY - (height / 2)), width, height);
            break;

        case ALIGN_END_X_END_Y:
            this->setGeometry((m_startX - width), (m_startY - height), width, height);
            break;

        default:
            break;
    }
    textRect.setRect(0, 0, width, height);
}

void TextLabel::focusInEvent(QFocusEvent *)
{
    emit sigMouseHover(m_indexInPage,true);
    slotShowHideToolTip(true);
}

void TextLabel::focusOutEvent(QFocusEvent *)
{
    emit sigMouseHover(m_indexInPage,false);
    slotShowHideToolTip(false);
}

void TextLabel::forceActiveFocus()
{
    this->setFocus();
}

void TextLabel::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    emit sigTextClick(m_indexInPage);
}

void TextLabel::mousePressEvent(QMouseEvent * event)
{
    if((textRect.contains(event->pos())) && (event->button() == Qt::LeftButton))
    {
        if(!this->hasFocus())
        {
            forceActiveFocus();
        }
    }
    QWidget::mousePressEvent (event);
}

void TextLabel::mouseReleaseEvent(QMouseEvent * event)
{
    if((textRect.contains(event->pos())) && (event->button() == Qt::LeftButton))
    {
        emit sigTextClick(m_indexInPage);
    }
    QWidget::mouseReleaseEvent (event);
}

void TextLabel::mouseMoveEvent (QMouseEvent *event)
{
    QWidget::mouseMoveEvent (event);
}

void TextLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if((textRect.contains(event->pos())) && (event->button() == Qt::LeftButton))
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
        slotShowHideToolTip(false);
    }
    else if(event->type () == QEvent::Enter)
    {
        emit sigMouseHover(m_indexInPage,true);
        slotShowHideToolTip(true);
    }

    return QWidget::eventFilter (object, event);
}
