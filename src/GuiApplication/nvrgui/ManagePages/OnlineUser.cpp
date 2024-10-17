#include "OnlineUser.h"
#include <QKeyEvent>
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define INVALID_FIELD 255

static const QString onlineUserLable[MAX_ONLINE_USER_STRINGS] = { "Username",
                                                                  "User Group",
                                                                  "IP Address",
                                                                  "Block",
                                                                  "View Blocked Users"};

static const QString userGroupName[3]={ "Admin",
                                        "Operator",
                                        "Viewer"};

OnlineUser::OnlineUser(qint32 startX,
                       qint32 startY,
                       quint32 width,
                       quint32 height,
                       QString devName,
                       QWidget *parent) :
    KeyBoard(parent), m_width(width), m_height(height),
    currentClickIndex(INVALID_FIELD)
{
    this->setGeometry (QRect(startX,startY,width,height));

    m_devListCount = 0;
    m_numberOfOnlineUser = 0;

    applController = ApplController::getInstance ();
    m_currentDevName = devName ;
    userList.clear ();

    payloadLib = new PayloadLib();

    for(quint8 index = 0;index < MAX_USER_CONTROL_ELEMENT; index++)
    {
        m_elementList[index] = NULL;
    }
    createElements ();

    getOnlineUserFrmDev();

    if(m_optionSelectButton[0]->isVisible ())
    {
        m_currentElement = USER_OPTION_SELECTION_BUTTON;
    }
    else
    {
        m_currentElement = USER_VIEW_BLOCK_CNFG_BUTTON;
    }

    m_elementList[m_currentElement]->forceActiveFocus ();
    this->show ();
}

void OnlineUser::createElements ()
{
    m_bgTop = new BgTile (USER_LEFT_MARGIN,
                          SCALE_HEIGHT(18),
                          BGTILE_WIDTH,
                          SCALE_HEIGHT(40),
                          TOP_LAYER,
                          this);

    m_userNameHeader = new TableCell ( m_bgTop->x () + SCALE_WIDTH(10),
                                       m_bgTop->y () + SCALE_HEIGHT(10),
                                       USER_NAME_HEADER_WIDTH,
                                       TABLE_CELL_HEIGHT,
                                       this,
                                       true);

    m_userNameLabel = new TextLabel (m_userNameHeader->x () + SCALE_WIDTH(10),
                                     m_userNameHeader->y () + SCALE_HEIGHT(5),
                                     LABEL_FONT_SIZE,
                                     onlineUserLable[USERNAME_STRING],
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                     0, 0, (USER_NAME_HEADER_WIDTH - SCALE_WIDTH(10)));

    m_userGroupHeader = new TableCell ( m_userNameHeader->x () +
                                        m_userNameHeader->width (),
                                        m_bgTop->y () + SCALE_HEIGHT(10),
                                        USER_GROUP_HEADER_WIDTH,
                                        TABLE_CELL_HEIGHT,
                                        this,
                                        true );

    m_userGroupLabel = new TextLabel (m_userGroupHeader->x () + SCALE_WIDTH(10),
                                      m_userNameHeader->y () + SCALE_HEIGHT(5),
                                      LABEL_FONT_SIZE,
                                      onlineUserLable[USERGROUP_STRING],
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, (USER_GROUP_HEADER_WIDTH - SCALE_WIDTH(10)));

    m_ipHeader = new  TableCell ( m_userGroupHeader->x () +
                                  m_userGroupHeader->width (),
                                  m_bgTop->y () + SCALE_HEIGHT(10),
                                  IP_HEADER_WIDTH,
                                  TABLE_CELL_HEIGHT,
                                  this,
                                  true );

    m_ipLabel = new TextLabel (m_ipHeader->x () + SCALE_WIDTH(25),
                               m_ipHeader->y () + SCALE_HEIGHT(5),
                               LABEL_FONT_SIZE,
                               onlineUserLable[IPHEADER_STRING],
                               this,
                               HIGHLITED_FONT_COLOR,
                               NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                               0, 0, (IP_HEADER_WIDTH - SCALE_WIDTH(25)));

    m_bgmiddle = new BgTile (USER_LEFT_MARGIN,
                             (m_bgTop->y () + m_bgTop->height ()  ),
                             BGTILE_WIDTH,
                             MAX_CELL_ON_PAGE * TABLE_CELL_HEIGHT,
                             MIDDLE_TABLE_LAYER,
                             this );


    for (quint8 index = 0 ; index < MAX_CELL_ON_PAGE ;index++)
    {
        m_userNameCell[index] = new TableCell( m_bgTop->x () + SCALE_WIDTH(10),
                                               m_bgTop->y () + m_bgTop->height ()
                                               + index * TABLE_CELL_HEIGHT  -SCALE_HEIGHT(8) ,
                                               USER_NAME_HEADER_WIDTH,
                                               TABLE_CELL_HEIGHT,
                                               this);

        m_optionSelectButton[index] = new OptionSelectButton( m_bgTop->x () + SCALE_WIDTH(20),
                                                              m_bgTop->y () + m_bgTop->height ()
                                                              + index * TABLE_CELL_HEIGHT - SCALE_HEIGHT(8),
                                                              USER_NAME_HEADER_WIDTH,
                                                              TABLE_CELL_HEIGHT,
                                                              CHECK_BUTTON_INDEX,
                                                              this,
                                                              NO_LAYER,
                                                              "",
                                                              "",0,
                                                              USER_OPTION_SELECTION_BUTTON + index,false,
                                                              15);

        m_elementList[index + USER_OPTION_SELECTION_BUTTON]= m_optionSelectButton[index];
        m_optionSelectButton[index]->hide ();
        m_optionSelectButton[index]->setIsEnabled(false);

        connect (m_optionSelectButton[index],
                 SIGNAL (sigButtonClicked(OPTION_STATE_TYPE_e, int )),
                 this,
                 SLOT(slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e, int )));

        connect(m_optionSelectButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));


        m_userNameCellLabel[index] = new TextLabel (m_userNameCell[index]->x () + SCALE_WIDTH(45) ,
                                                    m_userNameCell[index]->y () + SCALE_HEIGHT(5),
                                                    SCALE_FONT(15),
                                                    "",
                                                    this);




        m_userGroupCell[index] = new TableCell ( m_userNameHeader->x () + m_userNameHeader->width (),
                                                 m_bgTop->y () + m_bgTop->height ()
                                                 + index * TABLE_CELL_HEIGHT -SCALE_HEIGHT(8) ,
                                                 USER_GROUP_HEADER_WIDTH,
                                                 TABLE_CELL_HEIGHT,
                                                 this);

        m_userGroupCellLabel[index] = new TextLabel (m_userGroupCell[index]->x () + SCALE_WIDTH(10),
                                                     m_userGroupCell[index]->y () + SCALE_HEIGHT(5),
                                                     SCALE_FONT(15),
                                                     "",
                                                     this);

        m_ipCell[index] = new TableCell ( m_userGroupHeader->x () + m_userGroupHeader->width (),
                                          m_bgTop->y () + m_bgTop->height ()  +
                                          index * TABLE_CELL_HEIGHT-SCALE_HEIGHT(8) ,
                                          IP_HEADER_WIDTH,
                                          TABLE_CELL_HEIGHT,
                                          this);

        m_ipCellLabel[index] =   new TextLabel (m_ipCell[index]->x () + SCALE_WIDTH(5),
                                                m_ipCell[index]->y () + SCALE_HEIGHT(5),
                                                SCALE_FONT(15),
                                                "",
                                                this,
                                                HIGHLITED_FONT_COLOR,
                                                NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                                0, 0, IP_HEADER_WIDTH);
    }


    m_bgBottom = new BgTile ( USER_LEFT_MARGIN,
                              m_bgTop->y () + m_bgTop->height () + m_bgmiddle->height () - SCALE_HEIGHT(8) ,
                              BGTILE_WIDTH,
                              SCALE_HEIGHT(38),
                              DOWN_LAYER,
                              this );

    m_prevButton = new ControlButton (PREVIOUS_BUTTON_INDEX,
                                      m_bgBottom->x () + SCALE_WIDTH(10),
                                      m_bgBottom->y () ,
                                      SCALE_WIDTH(40), SCALE_HEIGHT(40), this, NO_LAYER,0, "",false,USER_PREV_BUTTON);

    m_elementList[ USER_PREV_BUTTON] = m_prevButton;

    connect(m_prevButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_prevButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_blockButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                   m_bgBottom->x () +
                                   (m_bgBottom->width () /2 )- SCALE_WIDTH(120),
                                   m_bgBottom->y () + (m_bgBottom->height () /2) -1  ,
                                   onlineUserLable[BLOCK_STRING],
                                   this,
                                   USER_BLOCK_CNFG_BUTTON);

    m_elementList[ USER_BLOCK_CNFG_BUTTON] = m_blockButton;

    connect(m_blockButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClick(int)));

    connect(m_blockButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_viewBlockButton = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                       (m_bgBottom->x () +(m_bgBottom->width () /2 ) + SCALE_WIDTH(75)),
                                       (m_bgBottom->y () +(m_bgBottom->height () /2) - 1) ,
                                       onlineUserLable[VIEW_BLOCK_STRING],
                                       this,
                                       USER_VIEW_BLOCK_CNFG_BUTTON);

    m_elementList[ USER_VIEW_BLOCK_CNFG_BUTTON] = m_viewBlockButton;

    connect(m_viewBlockButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClick(int)));
    connect(m_viewBlockButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));


    m_nextButton = new ControlButton (NEXT_BUTTON_INDEX,
                                      (m_bgBottom->x () + m_bgBottom->width () - SCALE_WIDTH(40)),
                                      m_bgBottom->y () , SCALE_WIDTH(40), SCALE_HEIGHT(40),
                                      this,NO_LAYER, 0, "",
                                      false,USER_NEXT_BUTTON);

    m_elementList[ USER_NEXT_BUTTON] = m_nextButton;

    connect(m_nextButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_nextButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
}


OnlineUser:: ~OnlineUser()
{
    disconnect(m_nextButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    disconnect(m_nextButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClick(int)));
    delete m_nextButton;

    disconnect(m_viewBlockButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));


    disconnect(m_viewBlockButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCnfgButtonClick(int)));
    delete m_viewBlockButton;


    disconnect(m_blockButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_blockButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCnfgButtonClick(int)));
    delete m_blockButton;

    disconnect(m_prevButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_prevButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotButtonClick(int)));
    delete m_prevButton;

    delete m_bgBottom;

    for (quint8 index = 0 ; index < MAX_CELL_ON_PAGE ;index++)
    {
        delete m_ipCellLabel[index];
        delete m_ipCell[index];
        delete m_userGroupCellLabel[index];
        delete m_userGroupCell[index];
        delete m_userNameCellLabel[index] ;
        disconnect(m_optionSelectButton[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_optionSelectButton[index],
                    SIGNAL (sigButtonClicked(OPTION_STATE_TYPE_e, int )),
                    this,
                    SLOT(slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e, int )));
        delete m_optionSelectButton[index];
        delete m_userNameCell[index];
    }

    delete m_bgmiddle;
    delete m_ipLabel;
    delete m_ipHeader;
    delete m_userGroupLabel;
    delete m_userGroupHeader;
    delete m_userNameLabel;
    delete m_userNameHeader;
    delete m_bgTop;
    delete payloadLib;
}

void OnlineUser::forceFocusToPage(bool isFirstElement)
{
    if(isFirstElement)
    {
        m_currentElement = -1;
        takeRightKeyAction();
    }
    else
    {
        m_currentElement = (MAX_USER_CONTROL_ELEMENT);
        takeLeftKeyAction();
    }
    m_elementList[m_currentElement]->forceActiveFocus ();
}


bool OnlineUser::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (MAX_USER_CONTROL_ELEMENT);
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

    return status;
}

bool OnlineUser::takeRightKeyAction ()
{
    bool status = true;
    do
    {
        if(m_currentElement == (MAX_USER_CONTROL_ELEMENT - 1))
        {
            m_currentElement = -1;
        }
        if(m_currentElement != (MAX_USER_CONTROL_ELEMENT - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
            status = false;
            break;
        }
    }while((m_currentElement < MAX_USER_CONTROL_ELEMENT) && ((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled())));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }

    return status;
}

void OnlineUser::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void OnlineUser::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void OnlineUser::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void OnlineUser::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void OnlineUser::getOnlineUserFrmDev()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = ONLINE_USR;

    applController->processActivity(m_currentDevName, DEVICE_COMM, param);
}

void OnlineUser::blockUser()
{
    if(userList.isEmpty ())
    {
        emit sigInfoMsgDisplay("Please select a User");
    }
    else
    {
        createCmdForBlkUser (userList.at (0));
    }
}

void OnlineUser::createCmdForBlkUser (QString blockUserName)
{
    payloadLib->setCnfgArrayAtIndex (0,blockUserName);

    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = BLK_USR;
    param->payload = payloadString;
    userList.removeFirst ();
    applController->processActivity(m_currentDevName, DEVICE_COMM, param);
}

void OnlineUser::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName == m_currentDevName) &&
            (param->msgType == MSG_SET_CMD))
    {
        payloadLib->parseDevCmdReply(true, param->payload);

        switch(param->cmdType)
        {
        case ONLINE_USR:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                m_numberOfOnlineUser =  ( payloadLib->getTotalCmdFields() / 3 );
                quint8 fieldindex = 0;
                for (quint8 index = 0 ; index < m_numberOfOnlineUser ;index ++ )
                {
                    usersName[index] = payloadLib->getCnfgArrayAtIndex(fieldindex ).toString ();
                    userGroup[index] = userGroupName[payloadLib->getCnfgArrayAtIndex(fieldindex + 1).toUInt ()];
                    ipAddress[index] = payloadLib->getCnfgArrayAtIndex(fieldindex + 2).toString ();

                    fieldindex += 3 ;
                }
                fillUserList(0);
                update ();
            }
        }
            break;
        case BLK_USR:
        {
            if(!userList.empty ())
            {
                createCmdForBlkUser (userList.at (0));
            }
            else
            {
                 if(param->deviceStatus == CMD_SUCCESS)
                 {
                     MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(ONLINE_USER_SUCCESS_BLOCK));
                 }
                 getOnlineUserFrmDev();
            }
        }
            break;
        default:
            break;
        }
    }
}

void OnlineUser:: fillUserList(quint8 pageIndex)
{
    quint8 dispNumOfUser;
    switch(pageIndex)
    {
    case 0:
    {
        if(m_numberOfOnlineUser > MAX_CELL_ON_PAGE )
        {
            dispNumOfUser = MAX_CELL_ON_PAGE ;
            m_nextButton->setIsEnabled (true);
        }
        else
            dispNumOfUser = m_numberOfOnlineUser;

        for (quint8 index = 0 ; index < dispNumOfUser ;index ++ )
        {
            m_userNameCellLabel[index]->changeText (usersName[index]);
            m_optionSelectButton[index]->setVisible (true);
            m_optionSelectButton[index]->setIsEnabled(true);
            m_optionSelectButton[index]->changeState (OFF_STATE);
            m_userGroupCellLabel[index]->changeText (userGroup[index]);
            m_ipCellLabel[index]->changeText (ipAddress[index]);
        }

        for (quint8 index = dispNumOfUser ; index < MAX_CELL_ON_PAGE ;index ++ )
        {
            m_userNameCellLabel[index]->changeText ("");
            m_optionSelectButton[index]->setVisible (false);
            m_optionSelectButton[index]->setIsEnabled(false);
            m_optionSelectButton[index]->changeState (OFF_STATE);
            m_userGroupCellLabel[index]->changeText ("");
            m_ipCellLabel[index]->changeText ("");
        }
    }
        break;

    case 1 :
    {
        dispNumOfUser = (m_numberOfOnlineUser - MAX_CELL_ON_PAGE);

        quint8 cellIndex = 0;
        for (quint8 index = MAX_CELL_ON_PAGE  ; index < m_numberOfOnlineUser ;index ++ )
        {
            m_userNameCellLabel[cellIndex]->changeText ( usersName[index]);
            m_userGroupCellLabel[cellIndex]->changeText (userGroup[index]);
            m_ipCellLabel[cellIndex]->changeText (ipAddress[index]);
            m_optionSelectButton[cellIndex]->setVisible (true);
            m_optionSelectButton[cellIndex]->setIsEnabled(true);
            m_optionSelectButton[cellIndex]->changeState (OFF_STATE);
            cellIndex++;
        }

        for (quint8 index = cellIndex ; index < MAX_CELL_ON_PAGE ;index ++ )
        {
            m_userNameCellLabel[index] ->changeText ("");
            m_userGroupCellLabel[index] ->changeText ("");
            m_ipCellLabel[index] ->changeText ("");
            m_optionSelectButton[index]->setVisible (false);
            m_optionSelectButton[index]->setIsEnabled(false);
            m_optionSelectButton[index]->changeState (OFF_STATE);
        }
    }
        break;

    }
    update ();
}

void OnlineUser::handleInfoPageMessage(int)
{
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void OnlineUser:: slotButtonClick(int index)
{
    switch (index)
    {
    case USER_NEXT_BUTTON:
        fillUserList (1);
        m_prevButton->setIsEnabled (true);
        m_currentElement = USER_PREV_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus ();
        m_nextButton->setIsEnabled (false);
        break;

    case USER_PREV_BUTTON:
        fillUserList (0);
        m_nextButton->setIsEnabled (true);
        m_currentElement = USER_NEXT_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus ();
        m_prevButton->setIsEnabled (false);
        break;
    }
}

void OnlineUser:: slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e state, int index )
{
    if(state == ON_STATE)
    {
        userList.append (usersName[index]);
    }
    else
    {
        if((userList.contains (usersName[index])))
        {
            quint8 tempIndex = userList.indexOf (usersName[index]);
            userList.removeAt (tempIndex);
        }
    }
}

void OnlineUser:: slotCnfgButtonClick(int index)
{
    switch(index)
    {
    case USER_BLOCK_CNFG_BUTTON:
        blockUser();
        break;

    case USER_VIEW_BLOCK_CNFG_BUTTON:
        emit sigDispProfileChange ();
        break;

    default:
        break;
    }
}

void OnlineUser::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
