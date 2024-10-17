#include "EventActionTCPNotify.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>

#define EVNT_TCP_NOTIFY_WIDTH   SCALE_WIDTH(536)
#define EVNT_TCP_NOTIFY_HEIGHT  SCALE_HEIGHT(240)
#define MAX_ALLOWED_TCP_CHAR    300

typedef enum{

    EVNT_TCP_HEADING,
    EVNT_TCP_MSGBOX_LABEL,
    EVNT_TCP_OK_LABEL,
    EVNT_TCP_CANCEL_LABEL,
    MAX_EVNT_TCP_NOTIFY_STRINGS
}EVNT_TCP_NOTIFY_STRINGS_e;

static const QString eventActionTCPNotifyStrings[MAX_EVNT_TCP_NOTIFY_STRINGS] = {
    "TCP Notification",
    "Message",
    "OK",
    "Cancel",
};


EventActionTCPNotify::EventActionTCPNotify(quint8 index,QString &tcpmessage,QWidget *parent):
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    m_index = index;

    for(quint8 index = 0; index < MAX_EVNT_TCP_NOTIFY_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    message =  &tcpmessage;

    infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget ());
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVNT_TCP_NOTIFY_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVNT_TCP_NOTIFY_HEIGHT)/2),
                               EVNT_TCP_NOTIFY_WIDTH,
                               EVNT_TCP_NOTIFY_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    EVNT_TCP_CLS_CTRL);

    m_elementlist[EVNT_TCP_CLS_CTRL] = closeButton;

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(25),
                          eventActionTCPNotifyStrings[EVNT_TCP_HEADING],
                          this,
                          HEADING_TYPE_2);

    msgBox =  new MessageBox(backGround->x () + SCALE_WIDTH(25),
                             backGround->y () + SCALE_HEIGHT(60),
                             BGTILE_MEDIUM_SIZE_WIDTH,
                             EVNT_TCP_MSGBOX_CTRL,
                             this,
                             eventActionTCPNotifyStrings
                             [EVNT_TCP_MSGBOX_LABEL],
                             COMMON_LAYER,
                             true,
                             0,
                             300,
                             QRegExp(QString("[^{}\\[\\]()+&=]")));

    m_elementlist[EVNT_TCP_MSGBOX_CTRL] = msgBox;

    connect (msgBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (msgBox,
             SIGNAL(sigTextValueAppended(QString,int)),
             this,
             SLOT(slotTextBoxValueAppend(QString,int)));

    msgBox->setInputText (tcpmessage);

    quint32 charCount = countNumberOfCharInMessage(tcpmessage);
    msgBox->setCurrentCharLenght (charCount);

    msgCountLabel = new TextLabel(msgBox->x () + SCALE_WIDTH(420),
                                  msgBox->y () + msgBox->height () + SCALE_HEIGHT(5),
                                  NORMAL_FONT_SIZE,
                                  "",
                                  this);

    msgCountLabel->changeText (QString ("%1").arg (charCount) + "/" +
                               QString ("%1").arg (MAX_ALLOWED_TCP_CHAR));

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(60),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(50),
                              eventActionTCPNotifyStrings[EVNT_TCP_OK_LABEL],
                              this,
                              EVNT_TCP_OK_CTRL);

    m_elementlist[EVNT_TCP_OK_CTRL] = okButton;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  backGround->x () + backGround->width ()/2 + SCALE_WIDTH(60),
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(50),
                                  eventActionTCPNotifyStrings
                                  [EVNT_TCP_CANCEL_LABEL],
                                  this,
                                  EVNT_TCP_CANCEL_CTRL);

    m_elementlist[EVNT_TCP_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    currElement = EVNT_TCP_MSGBOX_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

EventActionTCPNotify::~EventActionTCPNotify()
{
    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    disconnect (msgBox,
             SIGNAL(sigTextValueAppended(QString,int)),
             this,
             SLOT(slotTextBoxValueAppend(QString,int)));
    disconnect (msgBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete msgBox;
    delete msgCountLabel;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;
    delete heading;
    delete backGround;
}

void EventActionTCPNotify::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void EventActionTCPNotify::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_EVNT_TCP_NOTIFY_CTRL) % MAX_EVNT_TCP_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}


void EventActionTCPNotify::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_EVNT_TCP_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionTCPNotify::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionTCPNotify::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventActionTCPNotify::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}


void EventActionTCPNotify::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventActionTCPNotify::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVNT_TCP_CLS_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();
}

quint32 EventActionTCPNotify::countNumberOfCharInMessage(QString str)
{
    quint32 totalChar = str.length ();

    totalChar += (str.count ("%E") * (MAX_EVENT_STRING_LENGTH - 2));
    totalChar += (str.count ("%D") * (MAX_DATE_STRING_LENGTH - 2));
    totalChar += (str.count ("%T") * (MAX_TIME_STRING_LENGTH - 2));

    return totalChar;
}

void EventActionTCPNotify::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventActionTCPNotify::slotButtonClick (int index)
{
    if(index == EVNT_TCP_OK_CTRL)
    {
        if(msgBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_MESS));
            return;
        }
        else if(countNumberOfCharInMessage(msgBox->getInputText ()) > MAX_ALLOWED_TCP_CHAR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MSG_SIZE_MAX_LIMIT));
            return;
        }
        *message = msgBox->getInputText ();
    }

    emit sigDeleteObject(m_index);
}

void EventActionTCPNotify::slotInfoPageCnfgBtnClick (int)
{
    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionTCPNotify::slotTextBoxValueAppend(QString str, int)
{
    quint32 charCount = countNumberOfCharInMessage(str);
    msgCountLabel->changeText (QString ("%1").arg (charCount) + "/" +
                               QString ("%1").arg (MAX_ALLOWED_TCP_CHAR));
    msgCountLabel->update ();
    msgBox->setCurrentCharLenght (charCount);
}

void EventActionTCPNotify::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
