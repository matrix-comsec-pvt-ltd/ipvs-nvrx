#include "EventActionSmsNotify.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>

#define EVNT_SMS_NOTIFY_HEIGHT    SCALE_HEIGHT(310)
#define EVNT_SMS_NOTIFY_WIDTH     SCALE_WIDTH(536)

#define MAX_ALLOWED_SMS_CHAR      255

typedef enum {

    EVNT_SMS_HEADING_LABEL,
    EVNT_SMS_MOB_NUM1_TXTBOX_LABEL,
    EVNT_SMS_MOB_NUM2_TXTBOX_LABEL,
    EVNT_SMS_MSGBOX_LABEL,
    EVNT_SMS_OK_LABEL,
    EVNT_SMS_CANCEL_LABEL,

    MAX_SMS_EVENT_NOTIFY_STRINGS
}EVNT_SMS_NOTIFY_STRINGS_e;

static const QString eventActionSmsNotifyStrings[MAX_SMS_EVENT_NOTIFY_STRINGS] = {
    "SMS Notification",
    "Mobile Number 1",
    "Mobile Number 2",
    "Message",
    "OK",
    "Cancel"
};

EventActionSmsNotify::EventActionSmsNotify(quint8 index,
                                           QString &smsnum1,
                                           QString &smsnum2,
                                           QString &smsmesage,
                                           QWidget *parent) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    m_index = index;

    for(quint8 index = 0; index < MAX_SMS_EVENT_NOTIFY_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    smsNum1 = &smsnum1;
    smsNum2 = &smsnum2;
    smsMsg = &smsmesage;
    infoPage = NULL;
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
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVNT_SMS_NOTIFY_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVNT_SMS_NOTIFY_HEIGHT)/2),
                               EVNT_SMS_NOTIFY_WIDTH,
                               EVNT_SMS_NOTIFY_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    EVNT_SMS_CLS_CTRL);

    m_elementlist[EVNT_SMS_CLS_CTRL] = closeButton;

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
                          eventActionSmsNotifyStrings
                          [EVNT_SMS_HEADING_LABEL],
                          this,
                          HEADING_TYPE_2);

    mobileNumber1Param = new TextboxParam();
    mobileNumber1Param->maxChar = 14;
    mobileNumber1Param->minChar = 10;
    mobileNumber1Param->isTotalBlankStrAllow = true;
    mobileNumber1Param->validation = QRegExp(QString("[0-9]"));
    mobileNumber1Param->labelStr = eventActionSmsNotifyStrings
            [EVNT_SMS_MOB_NUM1_TXTBOX_LABEL];

    mobileNumber1TextBox = new TextBox(backGround->x () + SCALE_WIDTH(25),
                                       backGround->y () + SCALE_HEIGHT(60),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       EVNT_SMS_MOB_NUM1_TXTBOX_CTRL,
                                       TEXTBOX_LARGE,
                                       this,
                                       mobileNumber1Param,
                                       COMMON_LAYER);

    mobileNumber1TextBox->setInputText (smsnum1);

    m_elementlist[EVNT_SMS_MOB_NUM1_TXTBOX_CTRL] = mobileNumber1TextBox;

    connect (mobileNumber1TextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (mobileNumber1TextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));

    mobileNumber2Param = new TextboxParam();
    mobileNumber2Param->maxChar = 14;
    mobileNumber2Param->minChar = 10;
    mobileNumber2Param->isTotalBlankStrAllow = true;
    mobileNumber2Param->validation = QRegExp(QString("[0-9]"));
    mobileNumber2Param->labelStr = eventActionSmsNotifyStrings
            [EVNT_SMS_MOB_NUM2_TXTBOX_LABEL];

    mobileNumber2TextBox = new TextBox(mobileNumber1TextBox->x (),
                                       mobileNumber1TextBox->y () + BGTILE_HEIGHT,
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       EVNT_SMS_MOB_NUM2_TXTBOX_CTRL,
                                       TEXTBOX_LARGE,
                                       this,
                                       mobileNumber2Param,
                                       COMMON_LAYER);

    mobileNumber2TextBox->setInputText (smsnum2);

    m_elementlist[EVNT_SMS_MOB_NUM2_TXTBOX_CTRL] = mobileNumber2TextBox;

    connect (mobileNumber2TextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (mobileNumber2TextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));

    msgBox = new MessageBox(mobileNumber1TextBox->x (),
                            mobileNumber2TextBox->y () + BGTILE_HEIGHT,
                            BGTILE_MEDIUM_SIZE_WIDTH,
                            EVNT_SMS_MSGBOX_CTRL,
                            this,
                            eventActionSmsNotifyStrings
                            [EVNT_SMS_MSGBOX_LABEL],
                            COMMON_LAYER,
                            true,
                            0,
                            MAX_ALLOWED_SMS_CHAR,
                            QRegExp(""),
                            true,true);

    m_elementlist[EVNT_SMS_MSGBOX_CTRL] = msgBox;

    connect (msgBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (msgBox,
             SIGNAL(sigTextValueAppended(QString,int)),
             this,
             SLOT(slotTextBoxValueAppend(QString,int)));

    msgBox->setInputText (smsmesage);

    quint8 charCount = countNumberOfCharInMessage(smsmesage);
    msgBox->setCurrentCharLenght (charCount);

    msgCountLabel = new TextLabel(msgBox->x () + SCALE_WIDTH(420),
                                  msgBox->y () + msgBox->height () + SCALE_HEIGHT(5),
                                  NORMAL_FONT_SIZE,
                                  "",
                                  this);

    msgCountLabel->changeText (QString ("%1").arg (charCount) + "/" +
                               QString ("%1").arg (MAX_ALLOWED_SMS_CHAR));

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(70),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(50),
                              eventActionSmsNotifyStrings[EVNT_SMS_OK_LABEL],
                              this,
                              EVNT_SMS_OK_CTRL);

    m_elementlist[EVNT_SMS_OK_CTRL] = okButton;

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
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(50),
                                  eventActionSmsNotifyStrings
                                  [EVNT_SMS_CANCEL_LABEL],
                                  this,
                                  EVNT_SMS_CANCEL_CTRL);

    m_elementlist[EVNT_SMS_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    currElement = EVNT_SMS_MOB_NUM1_TXTBOX_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

EventActionSmsNotify::~EventActionSmsNotify()
{
    disconnect (mobileNumber1TextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (mobileNumber1TextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));
    delete mobileNumber1TextBox;
    delete mobileNumber1Param;

    disconnect (mobileNumber2TextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (mobileNumber2TextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));
    delete mobileNumber2TextBox;
    delete mobileNumber2Param;

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

    if(infoPage != NULL)
    {
        disconnect (infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageCnfgBtnClick(int)));
        DELETE_OBJ(infoPage);
    }
}

void EventActionSmsNotify::paintEvent (QPaintEvent *)
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

void EventActionSmsNotify::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_SMS_EVENT_NOTIFY_CTRL) % MAX_SMS_EVENT_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}


void EventActionSmsNotify::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_SMS_EVENT_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionSmsNotify::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionSmsNotify::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventActionSmsNotify::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void EventActionSmsNotify::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventActionSmsNotify::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVNT_SMS_CLS_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();
}

quint16 EventActionSmsNotify::countNumberOfCharInMessage (QString str)
{
    quint16 totalChar = str.length ();

    totalChar += (str.count ("%E") * (MAX_EVENT_STRING_LENGTH - 2));
    totalChar += (str.count ("%D") * (MAX_DATE_STRING_LENGTH - 2));
    totalChar += (str.count ("%T") * (MAX_TIME_STRING_LENGTH - 2));

    return totalChar;
}

void EventActionSmsNotify::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventActionSmsNotify::slotButtonClick (int index)
{
    if(index == EVNT_SMS_OK_CTRL)
    {
        if (!mobileNumber1TextBox->doneKeyValidation())
        {
            return;
        }

        if (!mobileNumber2TextBox->doneKeyValidation())
        {
            return;
        }

        if (msgBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_MESS));
            return;
        }
        else if(countNumberOfCharInMessage(msgBox->getInputText ()) >
                MAX_ALLOWED_SMS_CHAR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MSG_SIZE_MAX_LIMIT));
            return;
        }

        *smsNum1 = mobileNumber1Param->textStr ;
        *smsNum2 = mobileNumber2Param->textStr;
        *smsMsg = msgBox->getInputText ();
    }
    emit sigDeleteObject(m_index);
}

void EventActionSmsNotify::slotInfoPageCnfgBtnClick (int)
{
    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionSmsNotify::slotTextBoxInfoPage(int index,INFO_MSG_TYPE_e msgtype)
{
    if(msgtype == INFO_MSG_ERROR)
{
        if(index == EVNT_SMS_MOB_NUM1_TXTBOX_CTRL)
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VALID_MOB_NM1));
        }
        else if(index == EVNT_SMS_MOB_NUM2_TXTBOX_CTRL)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VALID_MOB_NM2));
        }
    }
}

void EventActionSmsNotify::slotTextBoxValueAppend (QString str, int)
{
    quint8 charCount = countNumberOfCharInMessage(str);
    msgCountLabel->changeText (QString ("%1").arg (charCount) + "/" +
                               QString ("%1").arg (MAX_ALLOWED_SMS_CHAR));
    msgCountLabel->update ();
    msgBox->setCurrentCharLenght (charCount);
}

void EventActionSmsNotify::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
