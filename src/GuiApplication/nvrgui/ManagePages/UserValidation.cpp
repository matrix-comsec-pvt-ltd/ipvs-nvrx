#include "UserValidation.h"
#include <QKeyEvent>
#include <QPainter>
#include "ValidationMessage.h"
#define USER_VALIDATION_BOX_HEIGHT  SCALE_HEIGHT(200)
#define USER_VALIDATION_BOX_WIDTH   SCALE_WIDTH(486)

typedef enum{
    USR_VLD_VALIDATION_STRING,
    USR_VLD_USERNAME_STRING,
    USR_VLD_USERNAME_VALIDATION_STRING,
    USR_VLD_USERNAME_START_STRING,
    USR_VLD_USERNAME_END_STRING,
    USR_VLD_PASSWORD_STRING,
    USR_VLD_PASSWORD_VALIDATION_STRING,
    USR_VLD_OK_BUTTON_STRING,
    MAX_USER_VALIDATION_STRING
}USER_VALIDATION_STRING_e;

static const QString usersValidationStrings[MAX_USER_VALIDATION_STRING] = {
    "Verification",
    "Username",
    "[a-zA-Z0-9_.]",
    "[a-zA-Z]",
    "[a-zA-Z0-9]",
    "Password",
    "[a-zA-Z0-9]",
    "OK"
};

UsersValidation::UsersValidation(QWidget *parent, USER_VALD_PAGE_ID_e pageIndex) :
    KeyBoard(parent), m_pageIndex(pageIndex)
{
     quint16 topMargin;
     quint16 leftMargin;
     quint16 infoPageWidth;
     quint16 infoPageHeight;


     this->setGeometry(0, 0, parent->width(), parent->height());
     if(m_pageIndex == PAGE_ID_MANAGE_PAGE)
     {
         topMargin = (MANAGE_LEFT_PANEL_HEIGHT - MANAGE_RIGHT_PANEL_HEIGHT) + (MANAGE_RIGHT_PANEL_HEIGHT - USER_VALIDATION_BOX_HEIGHT) / 2;
         leftMargin = (MANAGE_LEFT_PANEL_WIDTH + (MANAGE_RIGHT_PANEL_WIDTH - USER_VALIDATION_BOX_WIDTH) / 2);
         infoPageWidth = (MANAGE_LEFT_PANEL_WIDTH + MANAGE_RIGHT_PANEL_WIDTH);
         infoPageHeight = MANAGE_LEFT_PANEL_HEIGHT;
     }
     else
     {
         leftMargin = (parent->width() / 2 - USER_VALIDATION_BOX_WIDTH / 2);
         topMargin = (parent->height() / 2 - USER_VALIDATION_BOX_HEIGHT / 2);
         infoPageWidth = parent->width();
         infoPageHeight = parent->height();
     }

     for(quint8 index = 0; index < MAX_USERVALIDATION_CONTROL; index++)
     {
         m_elementList[index] = NULL;
     }
     m_rectangle = new Rectangle(leftMargin,
                                topMargin,
                                USER_VALIDATION_BOX_WIDTH,
                                USER_VALIDATION_BOX_HEIGHT,
                                0,
                                NORMAL_BKG_COLOR,
                                NORMAL_BKG_COLOR,
                                this);

    m_closeButtton = new CloseButtton(m_rectangle->x () + m_rectangle->width () - SCALE_WIDTH(30),
                                      m_rectangle->y () + SCALE_HEIGHT(23),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      USR_VLD_CLOSE_BTN);
    m_elementList[USR_VLD_CLOSE_BTN] = m_closeButtton;
    connect(m_closeButtton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotClosePage(int)),
            Qt::QueuedConnection);
    connect (m_closeButtton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_heading = new Heading((m_rectangle->x () + USER_VALIDATION_BOX_WIDTH / 2),
                            (m_rectangle->y () + SCALE_HEIGHT(25)),
                            usersValidationStrings[USR_VLD_VALIDATION_STRING],
                            this,
                            HEADING_TYPE_2);

    userParam = new TextboxParam();
    userParam->labelStr = usersValidationStrings[USR_VLD_USERNAME_STRING];
    userParam->isCentre = true;
    userParam->maxChar = 24;
    userParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);
    userParam->isTotalBlankStrAllow = false;

    userTextBox = new TextBox(m_rectangle->x () + SCALE_WIDTH(40),
                              m_rectangle->y () + SCALE_HEIGHT(50),
                              SCALE_WIDTH(410),
                              SCALE_HEIGHT(40),
                              USR_VLD_USER_TEXTBOX,
                              TEXTBOX_LARGE,
                              this,
                              userParam);
    m_elementList[USR_VLD_USER_TEXTBOX] = userTextBox;
    connect (userTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect(userTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));


    passwordParam = new TextboxParam();
    passwordParam->labelStr =  usersValidationStrings[USR_VLD_PASSWORD_STRING];
    passwordParam->isCentre =  true;
    passwordParam->maxChar  = 16;
    passwordParam->minChar = 4;
    passwordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);
    passwordParam->isTotalBlankStrAllow = false;

    passwordTextBox = new PasswordTextbox(m_rectangle->x () + SCALE_WIDTH(40),
                                          userTextBox->y () + SCALE_HEIGHT(40),
                                          SCALE_WIDTH(410),
                                          SCALE_HEIGHT(40),
                                          USR_VLD_PASSWORD_TEXTBOX,
                                          TEXTBOX_LARGE,
                                          this,
                                          passwordParam);
    m_elementList[USR_VLD_PASSWORD_TEXTBOX] = passwordTextBox;
    connect (passwordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect(passwordTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              (m_rectangle->x () + m_rectangle->width ()/2) ,
                              m_rectangle->y () + m_rectangle->height () - SCALE_HEIGHT(35),
                              usersValidationStrings
                              [USR_VLD_OK_BUTTON_STRING],
                              this,
                              3);
    m_elementList[USR_VLD_OK_BTN] = okButton;
    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotOkButtonClick(int)),
             Qt::QueuedConnection);
    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    infoPage = new InfoPage (0, 0,
                             infoPageWidth,
                             infoPageHeight,
                             INFO_MANAGE,
                             this,
                             false,
                             m_pageIndex == PAGE_ID_LIVE_VIEW ? false : true);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    m_currentElement = USR_VLD_USER_TEXTBOX;
    m_elementList[m_currentElement]->forceActiveFocus();

    this->show();
}

UsersValidation::~UsersValidation()
{
    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;

    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotOkButtonClick(int)));
    delete okButton;

    disconnect (passwordTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    disconnect (passwordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete passwordTextBox;
    delete passwordParam;

    disconnect(userTextBox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    disconnect (userTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete userTextBox;
    delete userParam;

    disconnect (m_closeButtton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_closeButtton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotClosePage(int)));
    delete m_closeButtton;

    delete m_heading;    
    delete m_rectangle;
}

void UsersValidation::paintEvent (QPaintEvent *event)
{
    if(m_pageIndex == PAGE_ID_MANAGE_PAGE)
    {
        QPainter painter(this);
        QColor bg(PROCESS_BKG_COLOR);
        bg.setAlpha(150);

        QRect rect;
        rect.setRect((this->width() - MANAGE_RIGHT_PANEL_WIDTH),
                     (this->height() - MANAGE_RIGHT_PANEL_HEIGHT),
                     MANAGE_RIGHT_PANEL_WIDTH,
                     MANAGE_RIGHT_PANEL_HEIGHT);
        painter.setBrush(QBrush(bg));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect,SCALE_WIDTH(15),SCALE_HEIGHT(15));
    }

    QWidget:: paintEvent(event);
}


void UsersValidation::takeLeftKeyAction ()
{
    bool status = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (MAX_USERVALIDATION_CONTROL);
        }
        if(m_currentElement)
        {
            m_currentElement = (m_currentElement - 1);
        }
        else
        {
              status = false;
              break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           ||(!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void UsersValidation::takeRightKeyAction ()
{
    bool status = true;
    do
    {
        if(m_currentElement == (MAX_USERVALIDATION_CONTROL - 1))
        {
            m_currentElement = -1;
        }
        if(m_currentElement != (MAX_USERVALIDATION_CONTROL - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
              status = false;
              break;
        }
    }while((m_currentElement < MAX_USERVALIDATION_CONTROL) && ((m_elementList[m_currentElement] == NULL)
           ||(!m_elementList[m_currentElement]->getIsEnabled())));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void UsersValidation::showEvent(QShowEvent *event)
{
    QWidget::showEvent (event);
    if(!infoPage->isVisible())
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void UsersValidation::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void UsersValidation::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void UsersValidation::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void UsersValidation::slotClosePage(int)
{
    emit sigOkButtonClicked("", "");
}

void UsersValidation::slotOkButtonClick(int)
{
    userTextBox->getInputText(m_username);
    passwordTextBox->getInputText(m_password);
    if(m_username == "")
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_USER_NM));
    }
    else if(m_password == "")
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PASSWORD_RANGE_ERROR));
    }
    else
    {
        emit sigOkButtonClicked(m_username, m_password);
    }
}

void UsersValidation:: slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void UsersValidation::slotTextBoxLoadInfopage(int index,
                                              INFO_MSG_TYPE_e msgType)
{
    QString tempStr = "";

    if( msgType == INFO_MSG_ERROR)
    {
        if (index == USR_VLD_USER_TEXTBOX)
            tempStr =  ValidationMessage::getValidationMessage(ENT_USER_NM);
        else
            tempStr =  ValidationMessage::getValidationMessage(PASSWORD_RANGE_ERROR);
    }
    infoPage->loadInfoPage(tempStr);
}

void UsersValidation::slotInfoPageBtnclick(int)
{
    m_elementList[m_currentElement]->forceActiveFocus();
}
