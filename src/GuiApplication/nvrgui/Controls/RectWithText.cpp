#include "RectWithText.h"
#include "QKeyEvent"
#include <QPaintEvent>

RectWithText::RectWithText(quint16 xParam,
                           quint16 yParam,
                           quint16 rectWidth,
                           quint16 rectHeight,
                           QString text,
                           QWidget *parent,
                           QString fontcolor,
                           int fontSize,
                           int indexInPage,
                           bool isEnabled)
    :Rectangle(xParam,
               yParam,
               rectWidth,
               rectHeight,
               0,
               NORMAL_BKG_COLOR,
               NORMAL_BKG_COLOR,
               parent,
               1), NavigationControl(indexInPage, isEnabled),
      m_text(text), m_fontColor(fontcolor), m_fontSize(fontSize)
{

    m_height = 0;
    m_width = 0;
    m_startX = 0;
    m_startY = 0;
    this->setEnabled(m_isEnabled);
    this->setMouseTracking (true);

    m_textLabel = new TextLabel(this->width ()/2,
                                this->height ()/2,
                                m_fontSize, m_text,
                                this, m_fontColor,
                                NORMAL_FONT_FAMILY,
                                ALIGN_CENTRE_X_CENTER_Y);
}

RectWithText::~RectWithText()
{
    delete m_textLabel;
}

void RectWithText::changeText(QString text)
{
    m_text = text;
    m_textLabel->changeText(m_text);
}

void RectWithText::changeFontColor(QString color)
{
    if(color != m_fontColor)
    {
        m_fontColor = color;
        m_textLabel->changeColor(m_fontColor);
    }
}

QString RectWithText::getText(void)
{
    return m_text;
}

QString RectWithText::getFontColor()
{
    return m_fontColor;
}

void RectWithText::selectRect()
{
    if(this->hasFocus())
    {
        changeColor(CLICKED_BKG_COLOR);
    }
    else
    {
        setColor(CLICKED_BKG_COLOR);
    }
}

void RectWithText::deselectRect()
{
    if(this->hasFocus())
    {
        changeColor(NORMAL_BKG_COLOR);
    }
    else
    {
        setColor(NORMAL_BKG_COLOR);
    }
}

void RectWithText::selectControl()
{
    if(m_text != "")
    {
        changeBorderColor(HIGHLITED_FONT_COLOR);
        update();
    }
}

void RectWithText::deSelectControl()
{
    changeBorderColor(NORMAL_BKG_COLOR);
    update();
}

void RectWithText::forceActiveFocus()
{
    this->setFocus();
}

void RectWithText::takeEnterKeyAction()
{
    emit sigMouseClicked(m_indexInPage);
}

void RectWithText::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    Rectangle::paintEvent (event);
}

void RectWithText::mouseMoveEvent(QMouseEvent *)
{
    if((m_text != "") && (m_isEnabled))
    {
        if(this->hasFocus ())
        {
            selectControl ();
        }
        else
        {
            forceActiveFocus ();
            emit sigUpdateCurrentElement (m_indexInPage);
        }
    }
}

void RectWithText::mousePressEvent(QMouseEvent * event)
{
    if((m_text != "")
            && (m_isEnabled)
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
    }
}

void RectWithText::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_text != "")
            && (m_isEnabled)
            && (event->button() == m_leftMouseButton)
            && m_mouseClicked)
    {
        takeEnterKeyAction();
        if(!this->hasFocus())
        {
            forceActiveFocus ();
            emit sigUpdateCurrentElement (m_indexInPage);
        }
    }
    m_mouseClicked = false;
}

void RectWithText::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void RectWithText::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void RectWithText::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        switch(event->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            event->accept();
            takeEnterKeyAction();
            break;

        default:
            event->accept();
            break;
        }
    }
}
