////////////////////////////////////////////////+-///////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : DVR ( Digital Video Recorder)
//   Owner        : Shruti Sahni
//   File         : MenuButton.cpp
//   Description  : This is menubutton file for multiDvrClient.
/////////////////////////////////////////////////////////////////////////////
#include "MenuButton.h"
#include "EnumFile.h"
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include "ApplController.h"

#define FOCUS_BACKGROUND_COLOR      "#2E4052"

MenuButton::MenuButton(int index,
                       int width,
                       int height,
                       QString label,
                       QWidget *parent,
                       int horizontalOffset,
                       int startX,
                       int startY,
                       int indexInPage,
                       bool isEnabled,
                       bool catchKey,
                       bool isClickOnClickNeeded,
                       bool changeTextColorOnClick,
                       bool isClickEffectNeeded,
                       bool isBorderNeeded,
                       QString fontColor,
                       int deviceIndex)
    : KeyBoard(parent), NavigationControl(indexInPage, isEnabled, catchKey), m_label(label),
      m_fontColor(fontColor)
{
    m_isDeletionStart = false;
    m_changeTextColorOnClick = changeTextColorOnClick;
    m_isClickOnClickNeeded = isClickOnClickNeeded;
    m_isClickEffectNeeded = isClickEffectNeeded;
    m_showClickedImage = false;
    m_index = index;
    m_width = width;
    m_height = height;
    m_startX = startX;
    m_startY = startY;
    m_isMouseHover = false;
    m_horizontalOffset = horizontalOffset;
    m_isBoarderNeed = isBorderNeeded;
    m_backgroundColor = (m_isBoarderNeed)? NORMAL_BKG_COLOR : CLICKED_BKG_COLOR;
    m_deviceIndex = deviceIndex;

    int translatedlabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY,
                                                    NORMAL_FONT_SIZE)).width(QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
    int textHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY,
                                                     NORMAL_FONT_SIZE)).height();
    if(m_horizontalOffset != 0)
    {
        m_textLabel = new TextLabel(m_horizontalOffset,
                                    (height / 2 - textHeight / 2) ,
                                    NORMAL_FONT_SIZE,
                                    m_label,
                                    this,
                                    m_fontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y, 0, 0, (width - m_horizontalOffset), 0, 0);
    }
    else
    {
        translatedlabelWidth = (translatedlabelWidth > (width - SCALE_WIDTH(14))) ? (width - SCALE_WIDTH(14)) : translatedlabelWidth;
        m_textLabel = new TextLabel((width / 2 - translatedlabelWidth / 2),
                                    (height / 2 - textHeight / 2) ,
                                    NORMAL_FONT_SIZE,
                                    m_label,
                                    this,
                                    m_fontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y, 0, 0, translatedlabelWidth, 0, 0);
    }

    m_clickEffectTimer = new QTimer(this);
    connect(m_clickEffectTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotClickEffectTimerout()));
    m_clickEffectTimer->setInterval(75);
    m_clickEffectTimer->setSingleShot(true);
    this->setGeometry(QRect(m_startX,
                            ((m_height * index) + m_startY) ,
                            m_width,
                            m_height));
    setRectanglesGeometry();

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    QWidget::setVisible(m_isEnabled);
}

MenuButton::~MenuButton()
{
    m_isDeletionStart = true;
    DELETE_OBJ(m_textLabel);

    if(m_clickEffectTimer->isActive())
    {
        m_clickEffectTimer->stop();
    }
    disconnect(m_clickEffectTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotClickEffectTimerout()));
    DELETE_OBJ(m_clickEffectTimer);
}

void MenuButton::setRectanglesGeometry()
{
    m_mainRect.setRect(0, 0,
                       m_width,
                       m_height);
    if(m_isBoarderNeed)
    {
        m_topRect.setRect(0, 0,
                          m_width,
                          UNIT_BORDER_THICKNESS);
        m_bottomRect.setRect(0,
                             m_height - UNIT_BORDER_THICKNESS,
                             m_width,
                             UNIT_BORDER_THICKNESS);
        m_leftRect.setRect(0, 0,
                           UNIT_BORDER_THICKNESS,
                           m_height);
        m_rightRect.setRect(m_width - UNIT_BORDER_THICKNESS,
                            0,
                            UNIT_BORDER_THICKNESS,
                            m_height);
        m_bottomRect_1.setRect(m_width - THRICE_BORDER_THICKNESS,
                               m_height - THRICE_BORDER_THICKNESS,
                               THRICE_BORDER_THICKNESS,
                               UNIT_BORDER_THICKNESS);
        m_bottomRect_2 .setRect(m_width - TWICE_BORDER_THICKNESS,
                                m_height - TWICE_BORDER_THICKNESS,
                                TWICE_BORDER_THICKNESS,
                                UNIT_BORDER_THICKNESS);
        m_bottomRect_3.setRect(m_width - UNIT_BORDER_THICKNESS,
                               m_height - UNIT_BORDER_THICKNESS,
                               UNIT_BORDER_THICKNESS,
                               UNIT_BORDER_THICKNESS);
        m_leftRect_1.setRect(0, 0,
                             UNIT_BORDER_THICKNESS,
                             UNIT_BORDER_THICKNESS);
        m_leftRect_2.setRect(1, 0,
                             UNIT_BORDER_THICKNESS,
                             TWICE_BORDER_THICKNESS);
        m_leftRect_3.setRect(2 , 0,
                             UNIT_BORDER_THICKNESS,
                             THRICE_BORDER_THICKNESS);
    }
}

void MenuButton::resetGeometry(int xOffset, int yOffset)
{
    this->setGeometry(QRect((m_startX + xOffset),
                            ((m_height * (m_index + yOffset) + m_startY)),
                            m_width,
                            m_height));
}

void MenuButton::drawRectangles(QPainter * painter)
{
    if(m_showClickedImage == false)
    {
        if(m_isBoarderNeed)
        {
            if(m_isMouseHover == true)
            {
                m_bottomRect.setTop(m_height - THRICE_BORDER_THICKNESS);
                m_rightRect.setLeft(m_width - THRICE_BORDER_THICKNESS);
                m_rightRect.setWidth(THRICE_BORDER_THICKNESS);
                m_topRect.setHeight(THRICE_BORDER_THICKNESS);
                m_bottomRect.setHeight(THRICE_BORDER_THICKNESS);
                m_leftRect.setWidth(THRICE_BORDER_THICKNESS);
            }
            else
            {
                m_bottomRect.setTop(m_height - UNIT_BORDER_THICKNESS);
                m_rightRect.setLeft(m_width - UNIT_BORDER_THICKNESS);
                m_rightRect.setWidth(UNIT_BORDER_THICKNESS);
                m_topRect.setHeight(UNIT_BORDER_THICKNESS);
                m_bottomRect.setHeight(UNIT_BORDER_THICKNESS);
                m_leftRect.setWidth(UNIT_BORDER_THICKNESS);
            }
        }

        //main rectangle
        painter->setBrush(QBrush(QColor(m_backgroundColor),Qt::SolidPattern));
        painter->drawRect(m_mainRect);

        if(m_isBoarderNeed)
        {
            //right border
            painter->setBrush(QBrush(QColor(BORDER_1_COLOR),Qt::SolidPattern));
            painter->drawRect(m_rightRect);

            //bottom border
            painter->setBrush(QBrush(QColor(BORDER_2_COLOR),Qt::SolidPattern));
            painter->drawRect(m_bottomRect);

            if(m_isMouseHover == true )
            {
                painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
                painter->drawRect(m_bottomRect_1);
                painter->drawRect(m_bottomRect_2);
                painter->drawRect(m_bottomRect_3);
            }

            //top border
            painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
            painter->drawRect(m_topRect);

            //left border
            painter->setBrush(QBrush(QColor(BORDER_2_COLOR), Qt::SolidPattern));
            painter->drawRect(m_leftRect);

            if(m_isMouseHover == true )
            {
                painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
                painter->drawRect(m_leftRect_1);
                painter->drawRect(m_leftRect_2);
                painter->drawRect(m_leftRect_3);
            }
        }

        if(m_changeTextColorOnClick)
        {
            m_textLabel->changeColor(m_fontColor);
        }
    }
    else
    {
        if(m_isBoarderNeed)
        {
            if(m_isMouseHover == true)
            {
                m_bottomRect.setTop(m_height - THRICE_BORDER_THICKNESS);
                m_rightRect.setLeft(m_width - THRICE_BORDER_THICKNESS);
                m_rightRect.setWidth(THRICE_BORDER_THICKNESS);
                m_topRect.setHeight(THRICE_BORDER_THICKNESS);
                m_bottomRect.setHeight(THRICE_BORDER_THICKNESS);
                m_leftRect.setWidth(THRICE_BORDER_THICKNESS);
            }
            else
            {
                m_bottomRect.setTop(m_height - UNIT_BORDER_THICKNESS);
                m_rightRect.setLeft(m_width - UNIT_BORDER_THICKNESS);
                m_rightRect.setWidth(UNIT_BORDER_THICKNESS);
                m_topRect.setHeight(UNIT_BORDER_THICKNESS);
                m_bottomRect.setHeight(UNIT_BORDER_THICKNESS);
                m_leftRect.setWidth(UNIT_BORDER_THICKNESS);
            }
        }

        //main rectangle
        painter->setBrush(QBrush(QColor(CLICKED_BKG_COLOR),Qt::SolidPattern));
        painter->drawRect(m_mainRect);

        if(m_isBoarderNeed)
        {
            //right border
            painter->setBrush(QBrush(QColor(BORDER_2_COLOR),Qt::SolidPattern));
            painter->drawRect(m_rightRect);

            //bottom border
            painter->setBrush(QBrush(QColor(BORDER_1_COLOR),Qt::SolidPattern));
            painter->drawRect(m_bottomRect);

            if(m_isMouseHover == true )
            {
                painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
                painter->drawRect(m_bottomRect_1);
                painter->drawRect(m_bottomRect_2);
                painter->drawRect(m_bottomRect_3);
            }

            //top border
            painter->setBrush(QBrush(QColor(BORDER_2_COLOR), Qt::SolidPattern));
            painter->drawRect(m_topRect);

            //left border
            painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
            painter->drawRect(m_leftRect);

            if(m_isMouseHover == true )
            {
                painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
                painter->drawRect(m_leftRect_1);
                painter->drawRect(m_leftRect_2);
                painter->drawRect(m_leftRect_3);
            }
        }
        if(m_changeTextColorOnClick)
        {
            m_textLabel->changeColor(HIGHLITED_FONT_COLOR);
        }
    }

    if(!m_isBoarderNeed)
    {
        if(m_isMouseHover)
        {
            if(m_fontColor == HIGHLITED_FONT_COLOR)
            {
                m_textLabel->changeColor(NORMAL_FONT_COLOR);
            }
            else
            {
                m_textLabel->changeColor(m_fontColor);
            }
        }
        else
        {
            m_textLabel->changeColor(m_fontColor);
        }
    }
}

void MenuButton::setShowClickedImage(bool flag)
{
    m_showClickedImage = flag;
    update();
}

void MenuButton::selectControl()
{
    if(!m_isMouseHover)
    {
        m_isMouseHover = true;
        m_backgroundColor = (m_isBoarderNeed)? NORMAL_BKG_COLOR : FOCUS_BACKGROUND_COLOR;
        update();
    }
}

void MenuButton::deSelectControl()
{
    m_isMouseHover = false;
    m_backgroundColor = (m_isBoarderNeed)? NORMAL_BKG_COLOR : CLICKED_BKG_COLOR;
    update();
}

void MenuButton::forceActiveFocus()
{
    this->setFocus();
}

void MenuButton::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
        this->setVisible(m_isEnabled);
    }
}

void MenuButton::takeEnterKeyAction()
{
    setShowClickedImage(true);

    if(m_isClickEffectNeeded)
    {
        if(!m_clickEffectTimer->isActive())
        {
            m_clickEffectTimer->start();
        }
    }
    else
    {
        emit sigButtonClicked(m_index);
    }
}

void MenuButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    drawRectangles(&painter);
    QWidget::paintEvent(event);
}

void MenuButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton)
            && (!m_isDeletionStart))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void MenuButton::mousePressEvent(QMouseEvent * event)
{
    if(!m_isDeletionStart)
    {
        if((event->button() == m_leftMouseButton)
                && (m_isClickOnClickNeeded))
        {
            m_mouseClicked = true;
            if(!this->hasFocus())
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }
        else if((event->button() == m_leftMouseButton)
                && (!m_showClickedImage))
        {
            m_mouseClicked = true;
            if(!this->hasFocus())
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }
    }
}

void MenuButton::mouseMoveEvent(QMouseEvent *)
{
    if((m_isControlActivated)
            && (!m_isDeletionStart))
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

void MenuButton::focusInEvent(QFocusEvent *)
{
    selectControl();
    if(m_deviceIndex != -1)
    {
        sigShowHideDeviceToolTip((this->x() + this->width()), this->y(), m_deviceIndex,m_index, true);

    }
    else
    {
        sigShowHideToolTip((this->x() + this->width()), this->y(), m_index, true);
    }
}

void MenuButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();

    if(m_deviceIndex != -1)
    {
        sigShowHideDeviceToolTip((this->x() + this->width()), this->y(), m_deviceIndex,m_index, false);

    }
    else
    {
        sigShowHideToolTip((this->x() + this->width()), this->y(), m_index, false);
    }
}

void MenuButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void MenuButton::slotClickEffectTimerout()
{
    setShowClickedImage(false);
    emit sigButtonClicked(m_index);
}

void MenuButton::disableButton(bool isDisable)
{

    m_isEnabled = isDisable;
    this->setEnabled(!m_isEnabled);

    if(isDisable == true)
    {
        m_fontColor = DISABLE_FONT_COLOR;
        m_textLabel->changeColor(m_fontColor);
    }
    else
    {
        m_fontColor = NORMAL_FONT_COLOR;
        m_textLabel->changeColor(m_fontColor);
    }
    this->update();
}
