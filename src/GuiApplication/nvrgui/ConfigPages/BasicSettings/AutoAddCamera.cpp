#include "AutoAddCamera.h"
#include "ValidationMessage.h"
#include <QPainter>
#include <QKeyEvent>

#define AUTO_ADD_CAM_WIDTH          SCALE_WIDTH(500)
#define AUTO_ADD_CAM_HEIGHT         SCALE_HEIGHT(250)

#define AUTO_ADD_CAM_STRING         "Auto Add Camera Settings"

static const QString autoAddCamSettingElementStrings[MAX_AUTO_ADD_CAM_ELEMETS] = {
    "",
    "TCP Port",
    "Poll Duration",
    "Poll Interval",
    "",
    ""
};

static const QString autoAddCamSettingElementSuffixStrings[MAX_AUTO_ADD_CAM_ELEMETS]={
    "",
    " (1-65535)",
    " (5-60)",
    " (0-60)",
    "",
    ""
};

AutoAddCamera::AutoAddCamera(QStringList *elementList, QWidget *parent):KeyBoard(parent)
{
    this->setGeometry (0, 0, parent->width(), parent->height());

    autoAddCamList = elementList;
    infoPage = NULL;

    createDefaultElements();
    fillRecords();

    m_currElement = AUTO_ADD_CAM_TCP_PORT;
    elementlist[m_currElement]->forceActiveFocus();

    this->show ();
}

AutoAddCamera::~AutoAddCamera()
{
    if(IS_VALID_OBJ(backGround))
        DELETE_OBJ(backGround);

    if(IS_VALID_OBJ(pageHeading))
        DELETE_OBJ(pageHeading);

    if(IS_VALID_OBJ(closeButtton))
    {
        disconnect (closeButtton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        DELETE_OBJ(closeButtton);
    }

    if(IS_VALID_OBJ(tcpPortTextBox))
    {
        disconnect (tcpPortTextBox,
                    SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                    this,
                    SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        disconnect (tcpPortTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(tcpPortTextBox);
    }

    if(IS_VALID_OBJ(tcpPortTextBoxParam))
        DELETE_OBJ(tcpPortTextBoxParam);

    if(IS_VALID_OBJ(pollDurationTextBox))
    {
        disconnect (pollDurationTextBox,
                    SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                    this,
                    SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        disconnect (pollDurationTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(pollDurationTextBox);
    }

    if(IS_VALID_OBJ(pollDurationTextBoxParam))
        DELETE_OBJ(pollDurationTextBoxParam);

    if(IS_VALID_OBJ(pollIntervalTextBox))
    {
        disconnect (pollIntervalTextBox,
                    SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                    this,
                    SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        disconnect (pollIntervalTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(pollIntervalTextBox);
    }

    if(IS_VALID_OBJ(pollIntervalTextBoxParam))
        DELETE_OBJ(pollIntervalTextBoxParam);

    if(IS_VALID_OBJ(okBtn))
    {
        disconnect (okBtn,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        disconnect (okBtn,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(okBtn);
    }

    if(IS_VALID_OBJ(cancleBtn))
    {
        disconnect (cancleBtn,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        disconnect (cancleBtn,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(cancleBtn);
    }

    if(IS_VALID_OBJ(infoPage))
    {
        disconnect (infoPage,
                    SIGNAL(sigInfoPageCnfgBtnClick(int)),
                    this,
                    SLOT(slotInfoPageBtnclick(int)));

        DELETE_OBJ(infoPage);
    }
}

void AutoAddCamera::createDefaultElements()
{
    backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(10) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - AUTO_ADD_CAM_WIDTH) / 2)),
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- AUTO_ADD_CAM_HEIGHT) / 2)),
                               AUTO_ADD_CAM_WIDTH,
                               AUTO_ADD_CAM_HEIGHT,
                               SCALE_WIDTH(RECT_RADIUS),
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,this);

    pageHeading = new Heading(backGround->x() + (AUTO_ADD_CAM_WIDTH/2),
                              backGround->y() + SCALE_HEIGHT(25), AUTO_ADD_CAM_STRING,
                              this,
                              HEADING_TYPE_1,
                              SCALE_FONT(20));

    closeButtton = new CloseButtton(backGround->x() + (AUTO_ADD_CAM_WIDTH - SCALE_WIDTH(20)),
                                    backGround->y() + SCALE_HEIGHT(20),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    AUTO_ADD_CLOSE_BUTTON);

    elementlist[AUTO_ADD_CLOSE_BUTTON] = closeButtton;

    connect (closeButtton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    // tcp port text port property
    tcpPortTextBoxParam = new TextboxParam();
    tcpPortTextBoxParam->labelStr = autoAddCamSettingElementStrings[AUTO_ADD_CAM_TCP_PORT];
    tcpPortTextBoxParam->suffixStr = autoAddCamSettingElementSuffixStrings[AUTO_ADD_CAM_TCP_PORT];
    tcpPortTextBoxParam->isNumEntry = true;
    tcpPortTextBoxParam->minNumValue = 1;
    tcpPortTextBoxParam->maxNumValue = 65535;
    tcpPortTextBoxParam->maxChar = 5;
    tcpPortTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // poll Duration text box property
    pollDurationTextBoxParam = new TextboxParam();
    pollDurationTextBoxParam->labelStr = autoAddCamSettingElementStrings[AUTO_ADD_CAM_POLL_DURATION];
    pollDurationTextBoxParam->suffixStr = autoAddCamSettingElementSuffixStrings[AUTO_ADD_CAM_POLL_DURATION];
    pollDurationTextBoxParam->isNumEntry = true;
    pollDurationTextBoxParam->minNumValue = 5;
    pollDurationTextBoxParam->maxNumValue = 60;
    pollDurationTextBoxParam->maxChar = 2;
    pollDurationTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // poll Interval text box property
    pollIntervalTextBoxParam = new TextboxParam();
    pollIntervalTextBoxParam->labelStr = autoAddCamSettingElementStrings[AUTO_ADD_CAM_POLL_INTERVAL];
    pollIntervalTextBoxParam->suffixStr = autoAddCamSettingElementSuffixStrings[AUTO_ADD_CAM_POLL_INTERVAL];
    pollIntervalTextBoxParam->isNumEntry = true;
    pollIntervalTextBoxParam->minNumValue = 0;
    pollIntervalTextBoxParam->maxNumValue = 60;
    pollIntervalTextBoxParam->maxChar = 2;
    pollIntervalTextBoxParam->validation = QRegExp(QString("[0-9]"));

    //**********Auto Add Camera Setting

    tcpPortTextBox = new TextBox(backGround->x() + SCALE_WIDTH(40),
                                 backGround->y() + SCALE_HEIGHT(60),
                                 BGTILE_SMALL_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 AUTO_ADD_CAM_TCP_PORT,
                                 TEXTBOX_SMALL,
                                 this,
                                 tcpPortTextBoxParam);

    elementlist[AUTO_ADD_CAM_TCP_PORT] = tcpPortTextBox;

    connect (tcpPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect (tcpPortTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    pollDurationTextBox = new TextBox(tcpPortTextBox->x (),
                                 tcpPortTextBox->y () + tcpPortTextBox->height (),
                                 BGTILE_SMALL_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 AUTO_ADD_CAM_POLL_DURATION,
                                 TEXTBOX_SMALL,
                                 this,
                                 pollDurationTextBoxParam);

    elementlist[AUTO_ADD_CAM_POLL_DURATION] = pollDurationTextBox;

    connect (pollDurationTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect (pollDurationTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    pollIntervalTextBox = new TextBox(pollDurationTextBox->x (),
                                 pollDurationTextBox->y () + pollDurationTextBox->height (),
                                 BGTILE_SMALL_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 AUTO_ADD_CAM_POLL_INTERVAL,
                                 TEXTBOX_SMALL,
                                 this,
                                 pollIntervalTextBoxParam);

    elementlist[AUTO_ADD_CAM_POLL_INTERVAL] = pollIntervalTextBox;

    connect (pollIntervalTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect (pollIntervalTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    okBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                           pollIntervalTextBox->x () + SCALE_WIDTH(140),
                           pollIntervalTextBox->y () + pollIntervalTextBox->height () + SCALE_HEIGHT(35),
                           "OK",
                           this,
                           AUTO_ADD_CAM_OK_BUTTON);

    elementlist[AUTO_ADD_CAM_OK_BUTTON] = okBtn;

    connect (okBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancleBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               okBtn->x () + okBtn->width () + SCALE_WIDTH(70),
                               pollIntervalTextBox->y () + pollIntervalTextBox->height () + SCALE_HEIGHT(35),
                               "Cancel",
                               this,
                               AUTO_ADD_CAM_CANCEL_BUTTON);

  elementlist[AUTO_ADD_CAM_CANCEL_BUTTON] = cancleBtn;

  connect (cancleBtn,
           SIGNAL(sigButtonClick(int)),
           this,
           SLOT(slotButtonClick(int)));

  connect (cancleBtn,
           SIGNAL(sigUpdateCurrentElement(int)),
           this,
           SLOT(slotUpdateCurrentElement(int)));

  infoPage = new InfoPage (0, 0,
                           SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                           SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                           INFO_CONFIG_PAGE,
                           parentWidget());
  connect (infoPage,
           SIGNAL(sigInfoPageCnfgBtnClick(int)),
           this,
           SLOT(slotInfoPageBtnclick(int)));

}

void AutoAddCamera::slotButtonClick(int index)
{
    bool ret=true;

    if ( (IS_VALID_OBJ(pollIntervalTextBox))
         && (IS_VALID_OBJ(pollDurationTextBox))
         && (IS_VALID_OBJ(tcpPortTextBox)))
    {
        if((!pollIntervalTextBox->doneKeyValidation())
                || (!pollDurationTextBox->doneKeyValidation())
                || (!tcpPortTextBox->doneKeyValidation()))
        {
            ret = false;
        }
    }

    if(ret)
    {
        switch(index)
        {
        case AUTO_ADD_CAM_OK_BUTTON:
            setRecords();
            emit sigObjectDelete ();
            break;

        case AUTO_ADD_CLOSE_BUTTON:
        case AUTO_ADD_CAM_CANCEL_BUTTON:
            emit sigObjectDelete ();
            break;

        default:
            break;
        }
    }
}

void AutoAddCamera::fillRecords()
{
    tcpPortTextBox->setInputText (autoAddCamList->at(FIELD_AUTO_ADD_TCP_PORT));
    pollDurationTextBox->setInputText (autoAddCamList->at(FIELD_AUTO_ADD_POLL_DURATION));
    pollIntervalTextBox->setInputText(autoAddCamList->at (FIELD_AUTO_ADD_POLL_INTERVAL));
}

void AutoAddCamera::setRecords()
{
    autoAddCamList->replace (FIELD_AUTO_ADD_TCP_PORT, tcpPortTextBox->getInputText());
    autoAddCamList->replace (FIELD_AUTO_ADD_POLL_DURATION, pollDurationTextBox->getInputText());
    autoAddCamList->replace (FIELD_AUTO_ADD_POLL_INTERVAL, pollIntervalTextBox->getInputText());
}

void AutoAddCamera::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    QString tempStr = "";
    switch(msgType)
    {
    case INFO_MSG_ERROR:
        switch(index)
        {
        case AUTO_ADD_CAM_TCP_PORT:
            tempStr = ValidationMessage::getValidationMessage(GEN_SETT_AUTO_ADD_CAM_TCP_PORT);
            break;

        case AUTO_ADD_CAM_POLL_DURATION:
            tempStr = ValidationMessage::getValidationMessage(GEN_SETT_AUTO_ADD_CAM_POLL_DURATION);
            break;

        case AUTO_ADD_CAM_POLL_INTERVAL:
            tempStr = ValidationMessage::getValidationMessage(GEN_SETT_AUTO_ADD_CAM_POLL_INTERVAL);
            break;

        default:
            break;
        }

        break;

    default:
        break;
    }

    if((tempStr != "") && (infoPage != NULL))
    {
        infoPage->loadInfoPage(tempStr);
    }
}

void AutoAddCamera::paintEvent (QPaintEvent *)
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
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                            SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void AutoAddCamera::slotUpdateCurrentElement(int index)
{
    m_currElement = index;
}

void AutoAddCamera::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void AutoAddCamera::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void AutoAddCamera::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void AutoAddCamera::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = AUTO_ADD_CLOSE_BUTTON;
    elementlist[m_currElement]->forceActiveFocus ();
}

void AutoAddCamera::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currElement == 0)
        {
            m_currElement = (MAX_AUTO_ADD_CAM_ELEMETS);
        }
        if(m_currElement)
        {
            m_currElement = (m_currElement - 1);
        }
        else
        {
              status = false;
              break;
        }
    }while((elementlist[m_currElement] == NULL)
           ||(!elementlist[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        elementlist[m_currElement]->forceActiveFocus();
    }
}

void AutoAddCamera::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(m_currElement == (MAX_AUTO_ADD_CAM_ELEMETS - 1))
        {
            m_currElement = -1;
        }
        if(m_currElement != (MAX_AUTO_ADD_CAM_ELEMETS - 1))
        {
            m_currElement = (m_currElement + 1);
        }
        else
        {
              status = false;
              break;
        }
    }while((elementlist[m_currElement] == NULL)
           ||(!elementlist[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        elementlist[m_currElement]->forceActiveFocus();
    }
}

void AutoAddCamera::slotInfoPageBtnclick(int)
{
    elementlist[m_currElement]->forceActiveFocus ();
}

void AutoAddCamera::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
