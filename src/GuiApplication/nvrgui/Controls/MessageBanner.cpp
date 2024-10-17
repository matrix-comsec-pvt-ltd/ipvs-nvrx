#include "MessageBanner.h"
#include <QPaintEvent>
#include "ApplicationMode.h"

#include <iostream>

#define LEFT_MARGIN     SCALE_WIDTH(17)
#define TOP_MARGIN      SCALE_HEIGHT(10)

MessageBanner* MessageBanner::messageBanner = NULL;

MessageBanner* MessageBanner::getInstance(QWidget* parent)
{
    if(messageBanner == NULL)
    {
        messageBanner = new MessageBanner(parent);
    }
    return messageBanner;
}

void MessageBanner::addMessageInBanner(QString message)
{
    if(messageBanner != NULL)
    {
        messageBanner->loadMessageBanner(message);
    }
}

void MessageBanner::flushQueueOfMsgBanner()
{
    if(messageBanner != NULL)
    {
        messageBanner->flushMessageQueue ();
    }
}

MessageBanner::MessageBanner(QWidget* parent) : QWidget(parent)
{
    m_showTimer = new QTimer(this);
    m_showTimer->setInterval(2000);
    connect(m_showTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotShowTimerTimeout()));

    quint16 labelWidth = 0;
    quint16 labelHeight = 0;
    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE),
                              "",
                              labelWidth,
                              labelHeight);

    this->setGeometry((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - (labelWidth + (2 * LEFT_MARGIN))) / 2)),
                      ApplController::getYPosOfScreen(),
                      (labelWidth + (2 * LEFT_MARGIN)),
                      (labelHeight + (2 * TOP_MARGIN)));

    m_backgroundRectangle = new Rectangle(0,
                                          0,
                                          (labelWidth + (2 * LEFT_MARGIN)),
                                          (labelHeight + (2 * TOP_MARGIN)),
                                          CLICKED_BKG_COLOR,
                                          this,
                                          SCALE_WIDTH(2),
                                          SCALE_WIDTH(2),
                                          WINDOW_GRID_COLOR);
    m_textLabel = new TextLabel((this->width() / 2),
                                (this->height() / 2),
                                NORMAL_FONT_SIZE,
                                "",
                                this,
                                NORMAL_FONT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_CENTRE_X_CENTER_Y);
    QWidget::setVisible(false);
}

MessageBanner::~MessageBanner()
{
    disconnect(m_showTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotShowTimerTimeout()));
    delete m_showTimer;

    delete m_backgroundRectangle;
    delete m_textLabel;
    m_messageList.clear();
}

void MessageBanner::loadMessageBanner(QString message)
{
    if(message != "")
    {
        if(m_messageList.isEmpty ())
        {
            m_messageList.append(message);
        }
        else if(m_messageList.last () != message)
        {
            m_messageList.append(message);
        }

        if(!m_showTimer->isActive())
        {
            changeMessage(m_messageList.first());
            m_showTimer->start(2000);
            this->setVisible(true);
        }
    }
}

void MessageBanner::changeMessage(QString text)
{
    quint16 labelWidth = 0;
    quint16 labelHeight = 0;
    text = QApplication::translate(QT_TRANSLATE_STR, text.toUtf8().constData());
    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE),
                              text,
                              labelWidth,
                              labelHeight);
    m_backgroundRectangle->resetGeometry(0,
                                         0,
                                         (labelWidth + (2 * LEFT_MARGIN)),
                                         (labelHeight + (2 * TOP_MARGIN)));
    this->setGeometry((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - (labelWidth + (2 * LEFT_MARGIN))) / 2)),
                      ApplController::getYPosOfScreen(),
                      (labelWidth + (2 * LEFT_MARGIN)),
                      (labelHeight + (2 * TOP_MARGIN)));
    m_textLabel->setOffset((this->width() / 2),
                           (this->height() / 2));
    m_textLabel->changeText(text);
    m_textLabel->update();
}

void  MessageBanner::flushMessageQueue()
{
    if(m_showTimer->isActive())
    {
        m_showTimer->stop();
        m_textLabel->changeText("");
        this->setVisible(false);
        m_messageList.clear ();
    }
}

void MessageBanner::updateGeometry()
{
    if(IS_VALID_OBJ(m_textLabel))
    {
        m_textLabel->setFontSize(NORMAL_FONT_SIZE);
        changeMessage(m_textLabel->getText());
    }
}

void MessageBanner::slotShowTimerTimeout()
{
    m_messageList.removeFirst();
    if(m_messageList.isEmpty())
    {
        m_showTimer->stop();
        m_textLabel->changeText("");
        this->setVisible(false);
    }
    else
    {
        changeMessage(m_messageList.first());
    }
}
