#include "EventActionEmailNotify.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>

#define EVNT_EMAIL_NOTIFY_HEIGHT    SCALE_HEIGHT(310)
#define EVNT_EMAIL_NOTIFY_WIDTH     SCALE_WIDTH(536)

#define MAX_ALLOWED_EMAIL_CHAR     50
#define LEFT_MARGIN_FROM_CENTER    SCALE_WIDTH(90)

typedef enum {

    EVNT_NOFY_HEADING_LABEL,
    EVNT_NOFY_EMAIL_TXTBOX_LABEL,
    EVNT_NOFY_SUB_TXTBOX_LABEL,
    EVNT_NOFY_MSGBOX_LABEL,
    EVNT_NOFY_OK_LABEL,
    EVNT_NOFY_CANCEL_LABEL,

    MAX_EMAIL_EVENT_NOTIFY_STRINGS
}EVNT_EMAIL_NOTIFY_STRINGS_e;

static const QString eventActionEmailNotifyStrings[] = {
    "Email Notification",
    "Mail Address",
    "Subject",
    "Message",
    "OK",
    "Cancel"
};


EventActionEmailNotify::EventActionEmailNotify(quint8 index,QString &address,
                                               QString &subject,
                                               QString &message,
                                               QWidget *parent) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    m_index = index;
    emailNoteLabel = NULL;

    for(quint8 index = 0; index < MAX_EMAIL_EVENT_NOTIFY_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    m_address = &address;
    m_subject = &subject;
    m_message = &message;

    backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVNT_EMAIL_NOTIFY_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVNT_EMAIL_NOTIFY_HEIGHT)/2),
                               EVNT_EMAIL_NOTIFY_WIDTH,
                               EVNT_EMAIL_NOTIFY_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    EVNT_NOFY_CLS_CTRL);

    m_elementlist[EVNT_NOFY_CLS_CTRL] = closeButton;

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
                          eventActionEmailNotifyStrings
                          [EVNT_NOFY_HEADING_LABEL],
                          this,
                          HEADING_TYPE_2);

    emailTextBoxParam = new TextboxParam();
    emailTextBoxParam->maxChar = 100;
    emailTextBoxParam->labelStr = eventActionEmailNotifyStrings[EVNT_NOFY_EMAIL_TXTBOX_LABEL];
    emailTextBoxParam->isEmailAddrType = true;

    emailTextBox = new TextBox(backGround->x () + SCALE_WIDTH(25),
                               backGround->y () + SCALE_HEIGHT(60),
                               BGTILE_MEDIUM_SIZE_WIDTH,
                               BGTILE_HEIGHT,
                               EVNT_NOFY_EMAIL_TXTBOX_CTRL,
                               TEXTBOX_ULTRALARGE,
                               this,
                               emailTextBoxParam,
                               COMMON_LAYER, true,
                               false, false,
                               LEFT_MARGIN_FROM_CENTER);

    emailTextBox->setInputText (address);

    m_elementlist[EVNT_NOFY_EMAIL_TXTBOX_CTRL] = emailTextBox;

    connect (emailTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (emailTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));

    subjectTextBoxParam = new TextboxParam();
    subjectTextBoxParam->maxChar = 50;
    subjectTextBoxParam->labelStr = eventActionEmailNotifyStrings[EVNT_NOFY_SUB_TXTBOX_LABEL];

    subjectTextBox = new TextBox(emailTextBox->x (),
                                 emailTextBox->y () + BGTILE_HEIGHT,
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 EVNT_NOFY_SUB_TXTBOX_CTRL,
                                 TEXTBOX_ULTRALARGE,
                                 this,
                                 subjectTextBoxParam,
                                 COMMON_LAYER,
                                 true,
                                 true,
                                 true,
                                 LEFT_MARGIN_FROM_CENTER);

    subjectTextBox->setInputText (subject);

    m_elementlist[EVNT_NOFY_SUB_TXTBOX_CTRL] = subjectTextBox;

    connect (subjectTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (subjectTextBox,
             SIGNAL(sigTextValueAppended(QString,int)),
             this,
             SLOT(slotTextBoxValueAppend(QString,int)));

    quint8 charCount = countNumberOfCharInMessage(subject);
    subjectTextBox->setCurrentCharLenght (charCount);

    msgCountLabel = new TextLabel(subjectTextBox->x () + SCALE_WIDTH(430),
                                  subjectTextBox->y () + SCALE_HEIGHT(15),
                                  NORMAL_FONT_SIZE,
                                  "",
                                  this);

    msgCountLabel->changeText (QString ("%1").arg (charCount) + "/" +
                               QString ("%1").arg (MAX_ALLOWED_EMAIL_CHAR));

    msgBox =  new MessageBox(emailTextBox->x (),
                             subjectTextBox->y () + BGTILE_HEIGHT,
                             BGTILE_MEDIUM_SIZE_WIDTH,
                             EVNT_NOFY_MSGBOX_CTRL,
                             this,
                             eventActionEmailNotifyStrings
                             [EVNT_NOFY_MSGBOX_LABEL],
                             COMMON_LAYER,
                             true,
                             0,
                             300,
                             QRegExp(""),
                             true,
                             false,
                             true,
                             LEFT_MARGIN_FROM_CENTER);

    m_elementlist[EVNT_NOFY_MSGBOX_CTRL] = msgBox;

    connect (msgBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    msgBox->setInputText (message);

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(70),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(30),
                              eventActionEmailNotifyStrings[EVNT_NOFY_OK_LABEL],
                              this,
                              EVNT_NOFY_OK_CTRL);

    m_elementlist[EVNT_NOFY_OK_CTRL] = okButton;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  backGround->x () + backGround->width ()/2 + SCALE_WIDTH(70),
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(30),
                                  eventActionEmailNotifyStrings
                                  [EVNT_NOFY_CANCEL_LABEL],
                                  this,
                                  EVNT_NOFY_CANCEL_CTRL);

    m_elementlist[EVNT_NOFY_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget ());

    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    currElement = EVNT_NOFY_EMAIL_TXTBOX_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();

    QString emailNote = QString("Note : Multiple Email IDs can be added using (;) or (,)");

    emailNoteLabel = new TextLabel(msgBox->x (),
                                   msgBox->y () + msgBox->height () + SCALE_HEIGHT(5),
                                   NORMAL_FONT_SIZE,
                                   emailNote,
                                   this);

    emailNoteLabel->setVisible(true);

    this->show ();
}

EventActionEmailNotify::~EventActionEmailNotify()
{
    disconnect (emailTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (emailTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));
    delete emailTextBox;
    delete emailTextBoxParam;

    disconnect (subjectTextBox,
                SIGNAL(sigTextValueAppended(QString,int)),
                this,
                SLOT(slotTextBoxValueAppend(QString,int)));
    disconnect (subjectTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete subjectTextBox;
    delete subjectTextBoxParam;

    disconnect (msgBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete msgBox;
    delete msgCountLabel;

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;
    delete heading;
    delete backGround;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;

    if(IS_VALID_OBJ(emailNoteLabel))
    {
        DELETE_OBJ(emailNoteLabel);
    }

}

void EventActionEmailNotify::paintEvent (QPaintEvent *)
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

void EventActionEmailNotify::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_EMAIL_EVENT_NOTIFY_CTRL) % MAX_EMAIL_EVENT_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}


void EventActionEmailNotify::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_EMAIL_EVENT_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionEmailNotify::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionEmailNotify::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventActionEmailNotify::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void EventActionEmailNotify::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventActionEmailNotify::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVNT_NOFY_CLS_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();
}

quint8 EventActionEmailNotify::countNumberOfCharInMessage(QString str)
{
    quint8 totalChar = str.length ();

    totalChar += (str.count ("%E") * (MAX_EVENT_STRING_LENGTH - 2));
    totalChar += (str.count ("%D") * (MAX_DATE_STRING_LENGTH - 2));
    totalChar += (str.count ("%T") * (MAX_TIME_STRING_LENGTH - 2));

    return totalChar;
}

void EventActionEmailNotify::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventActionEmailNotify::slotButtonClick (int index)
{
    if(index == EVNT_NOFY_OK_CTRL)
    {
        if(!emailTextBox->doneKeyValidation())
        {
            return;
        }

        if(emailTextBoxParam->textStr == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_EMAIL_ADD));
            return;
        }
        else if (subjectTextBoxParam->textStr == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_SUBJECT));
            return;
        }
        else if (msgBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_MESS));
            return;
        }
        else if(countNumberOfCharInMessage(subjectTextBox->getInputText ()) >
                MAX_ALLOWED_EMAIL_CHAR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MSG_SIZE_MAX_LIMIT));
            return;
        }


        *m_address = emailTextBox->getInputText ();
        *m_subject = subjectTextBox->getInputText ();
        *m_message = msgBox->getInputText ();
    }
    emit sigDeleteObject(m_index);
}

void EventActionEmailNotify::slotInfoPageCnfgBtnClick (int)
{
    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionEmailNotify::slotTextBoxInfoPage(int,INFO_MSG_TYPE_e errorType)
{
    if(errorType == INFO_MSG_STRAT_CHAR)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_FIRST_ALPH));
    }
    else
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VAILD_EMAIL_ADD));
    }
}

void EventActionEmailNotify::slotTextBoxValueAppend(QString str, int)
{
    quint8 charCount = countNumberOfCharInMessage(str);
    msgCountLabel->changeText (QString ("%1").arg (charCount) + "/" +
                               QString ("%1").arg (MAX_ALLOWED_EMAIL_CHAR));
    msgCountLabel->update ();
    msgBox->setCurrentCharLenght (charCount);
}

void EventActionEmailNotify::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
