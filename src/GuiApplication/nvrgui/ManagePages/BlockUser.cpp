#include "BlockUser.h"
#include <QKeyEvent>
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define INVALID_FIELD 255

static const QString BlkUserString[MAX_BLOCK_USER_STRING] = { "Username",
                                                              "User Group",
                                                              "Unblock",
                                                              "View Online Users"};
static const QString userGroupName[3] = { "Admin",
                                          "Operator",
                                          "Viewer" };

BlockUser::BlockUser(qint32 startX,
                     qint32 startY,
                     quint32 width,
                     quint32 height,
                     QString devName,
                     QWidget *parent) :
    KeyBoard(parent), m_width(width), m_height(height),
    m_currentDevName(devName)
{

    userList.clear ();
    currentClickIndex = 0;
    m_devListCount = 0;
    m_numberOfBlockUser = 0;

    this->setGeometry (QRect(startX,startY,width,height));

    applController = ApplController::getInstance ();
    payloadLib = new PayloadLib();

    for(quint8 index = 0; index < MAX_BLOCK_USER_CNTRL; index++ )
    {
        m_elementList[index] = NULL;
    }
    createDefaultElements();

    getBlockDevicelist();

    this->show();
}

void BlockUser :: createDefaultElements()
{
    m_bgTop = new BgTile(BLK_USER_LEFT_MARGIN,
                         SCALE_HEIGHT(18),
                         BLK_USER_BGTILE_WIDTH,
                         SCALE_HEIGHT(40),
                         TOP_LAYER,
                         this);

    m_userNameHeader = new TableCell ( m_bgTop->x () + (m_bgTop->width () - BLK_USER_NAME_HEADER_WIDTH
                                                        - BLK_USER_GROUP_HEADER_WIDTH )/2,
                                       m_bgTop->y () + SCALE_HEIGHT(10),
                                       BLK_USER_NAME_HEADER_WIDTH,
                                       BLK_USER_TABLE_CELL_HEIGHT,
                                       this,
                                       true);

    m_userNameLabel = new TextLabel (m_userNameHeader->x () + SCALE_WIDTH(10),
                                     m_userNameHeader->y () + SCALE_HEIGHT(5),
                                     NORMAL_FONT_SIZE,
                                     BlkUserString[USER_NAME_STRING],
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY);

    m_userGroupHeader = new TableCell ( m_userNameHeader->x () + m_userNameHeader->width (),
                                        m_bgTop->y () + SCALE_HEIGHT(10),
                                        BLK_USER_GROUP_HEADER_WIDTH,
                                        BLK_USER_TABLE_CELL_HEIGHT,
                                        this,
                                        true );

    m_userGroupLabel = new TextLabel (m_userGroupHeader->x () + SCALE_WIDTH(10),
                                      m_userNameHeader->y () + SCALE_HEIGHT(5),
                                      NORMAL_FONT_SIZE,
                                      BlkUserString[USER_GROUP_STRING],
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY);


    m_bgmiddle = new BgTile (BLK_USER_LEFT_MARGIN,
                             (m_bgTop->y () + m_bgTop->height ()  ),
                             BLK_USER_BGTILE_WIDTH,
                             MAX_CELL * BLK_USER_TABLE_CELL_HEIGHT,
                             MIDDLE_LAYER,
                             this );


    for (quint8 index = 0 ; index < MAX_CELL ;index++)
    {
        m_userNameCell[index] = new TableCell( m_bgTop->x () + (m_bgTop->width () - BLK_USER_NAME_HEADER_WIDTH
                                                                - BLK_USER_GROUP_HEADER_WIDTH )/2,
                                               m_bgTop->y () + m_bgTop->height ()
                                               + index * BLK_USER_TABLE_CELL_HEIGHT - SCALE_HEIGHT(8) ,
                                               BLK_USER_NAME_HEADER_WIDTH,
                                               BLK_USER_TABLE_CELL_HEIGHT,
                                               this);

        m_optionSelectButton[index] = new OptionSelectButton( (m_userNameCell[index]->x() + SCALE_WIDTH(5)),
                                                              m_bgTop->y () + m_bgTop->height ()
                                                              + index * BLK_USER_TABLE_CELL_HEIGHT -SCALE_HEIGHT(8),
                                                              BLK_USER_NAME_HEADER_WIDTH,
                                                              BLK_USER_TABLE_CELL_HEIGHT,
                                                              CHECK_BUTTON_INDEX,
                                                              this,
                                                              NO_LAYER,
                                                              "",
                                                              "",5,
                                                              index,false);
        m_optionSelectButton[index]->setVisible (false);
        m_optionSelectButton[index]->setIsEnabled(false);

        m_elementList[index] = m_optionSelectButton[index] ;

        connect(m_optionSelectButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        connect (m_optionSelectButton[index],
                 SIGNAL (sigButtonClicked(OPTION_STATE_TYPE_e, int)),
                 this,
                 SLOT(slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e, int )));


        m_userNameCellLabel[index] = new TextLabel (m_userNameCell[index]->x () + SCALE_WIDTH(45),
                                                    m_userNameCell[index]->y () + SCALE_HEIGHT(5),
                                                    SCALE_FONT(15),
                                                    "",
                                                    this);


        m_userGroupCell[index] = new TableCell ( m_userNameHeader->x () + m_userNameHeader->width (),
                                                 m_bgTop->y () + m_bgTop->height ()
                                                 + index * BLK_USER_TABLE_CELL_HEIGHT -SCALE_HEIGHT(8) ,
                                                 BLK_USER_GROUP_HEADER_WIDTH,
                                                 BLK_USER_TABLE_CELL_HEIGHT,
                                                 this);

        m_userGroupCellLabel[index] = new TextLabel (m_userGroupCell[index]->x () + SCALE_WIDTH(10),
                                                     m_userGroupCell[index]->y () + SCALE_HEIGHT(5),
                                                     SCALE_FONT(15),
                                                     "",
                                                     this);
    }


    m_bgBottom = new BgTile (BLK_USER_LEFT_MARGIN,
                             m_bgTop->y () + m_bgTop->height () + m_bgmiddle->height () - SCALE_HEIGHT(8) ,
                             BLK_USER_BGTILE_WIDTH,
                             SCALE_HEIGHT(38),
                             DOWN_LAYER,
                             this );

    m_prevButton = new ControlButton (PREVIOUS_BUTTON_INDEX,
                                      m_bgBottom->x () + SCALE_WIDTH(10),
                                      m_bgBottom->y () , SCALE_WIDTH(40) , SCALE_HEIGHT(40),
                                      this, NO_LAYER, 0, "",false,
                                      PREV_BUTTON);

    m_elementList[PREV_BUTTON] = m_prevButton;

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
                                   BlkUserString[UNBLOCK_STRING],
                                   this,
                                   UNBLOCK_CNFG_BUTTON);

    m_elementList[UNBLOCK_CNFG_BUTTON] = m_blockButton;

    connect(m_blockButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClick(int)));

    connect(m_blockButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_viewBlockButton = new CnfgButton( CNFGBUTTON_EXTRALARGE,
                                        m_bgBottom->x () +
                                        (m_bgBottom->width () /2 ) + SCALE_WIDTH(75),
                                        m_bgBottom->y () +
                                        (m_bgBottom->height () /2) -1  ,
                                        BlkUserString[VIEW_ONLINE_USER_STRING],
                                        this,
                                        VIEW_ONLINE_CNFG_BUTTON);

    m_elementList[VIEW_ONLINE_CNFG_BUTTON] = m_viewBlockButton;

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
                                      (m_bgBottom->y ()), SCALE_WIDTH(40), SCALE_HEIGHT(40),
                                      this,NO_LAYER, 0, "", false,
                                      NEXT_BUTTON);

    m_elementList[NEXT_BUTTON] = m_nextButton;

    connect(m_nextButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect(m_nextButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_currentElement = VIEW_ONLINE_CNFG_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus ();
}

BlockUser::~BlockUser()
{
    userList.clear ();
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

    for (quint8 index = 0 ; index < MAX_CELL ;index++)
    {
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
    delete m_userGroupLabel;
    delete m_userGroupHeader;
    delete m_userNameLabel;
    delete m_userNameHeader;
    delete m_bgTop;
    delete payloadLib;
}


void BlockUser:: getBlockDevicelist()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = VIEW_BLK_USR;

    applController->processActivity(m_currentDevName, DEVICE_COMM, param);
}

void BlockUser::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName == m_currentDevName) &&
            (param->msgType == MSG_SET_CMD) && (param->deviceStatus == CMD_SUCCESS))
    {
        payloadLib->parseDevCmdReply(true, param->payload);

        switch(param->cmdType)
        {
        case VIEW_BLK_USR:
        {
            m_numberOfBlockUser =  ( payloadLib->getTotalCmdFields() /2 );
            qint8 fieldindex = 0;
            for (qint8 index = 0 ; index < m_numberOfBlockUser; index++ )
            {
                usersName[index] = payloadLib->getCnfgArrayAtIndex(fieldindex ).toString ();
                userGroup[index] = (userGroupName[payloadLib->getCnfgArrayAtIndex(fieldindex + 1).toUInt ()]);

                fieldindex += 2 ;
            }

            fillUserList(0);
        }
            break;

        case UNBLK_USR:
        {
            if(!userList.empty ())
            {
                userList.removeFirst ();
                if(userList.empty ())
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(BLOCK_USER_SUCCESS_UNBLOCK));
                    getBlockDevicelist();
                }
                else
                {
                    createCmdForUnblkUser(userList.at (0));
                }
            }
        }
            break;
        default:
            break;
        }
    }
}

void BlockUser:: fillUserList(quint8 pageIndex)
{
    quint8 dispNumOfUser;
    switch(pageIndex)
    {
    case 0:
        if(m_numberOfBlockUser > MAX_CELL )
        {
            dispNumOfUser = MAX_CELL;
            m_nextButton->setIsEnabled (true);
        }
        else
            dispNumOfUser = m_numberOfBlockUser;

        for (quint8 index = 0 ; index < dispNumOfUser ;index ++ )
        {
            m_userNameCellLabel[index]->changeText (usersName[index]);
            m_userGroupCellLabel[index]->changeText (userGroup[index]);
            m_optionSelectButton[index]->setVisible (true);
            m_optionSelectButton[index]->setIsEnabled(true);
            m_optionSelectButton[index]->changeState (OFF_STATE);
        }

        for (quint8 index = dispNumOfUser ; index < MAX_CELL ;index ++ )
        {
            m_userNameCellLabel[index]->changeText ("");
            m_userGroupCellLabel[index]->changeText ("");
            m_optionSelectButton[index]->setVisible (false);
            m_optionSelectButton[index]->setIsEnabled(false);
            m_optionSelectButton[index]->changeState (OFF_STATE);
        }

        break;

    case 1 :
        dispNumOfUser = m_numberOfBlockUser - MAX_CELL ;

        quint8 cellIndex = 0;
        for (qint8 index = MAX_CELL ; index < m_numberOfBlockUser ;index ++ )
        {
            m_userNameCellLabel[cellIndex]->changeText (usersName[index]);
            m_userGroupCellLabel[cellIndex]->changeText (userGroup[index]);
            m_optionSelectButton[cellIndex]->setVisible (true);
            m_optionSelectButton[cellIndex]->setIsEnabled(true);
            m_optionSelectButton[cellIndex]->changeState (OFF_STATE);
            cellIndex++;
        }

        for (quint8 index = cellIndex ; index < MAX_CELL ;index ++ )
        {
            m_userNameCellLabel[index]->changeText ("");
            m_userGroupCellLabel[index]->changeText ("");
            m_optionSelectButton[index]->setVisible (false);
            m_optionSelectButton[index]->setIsEnabled(false);
            m_optionSelectButton[index]->changeState (OFF_STATE);
        }

        break;
    }
    update ();
}

void BlockUser::unBlockUser()
{
    if(userList.isEmpty ())
    {
        emit sigInfoMsgDisplay("Please select a User");
    }
    else
    {
        createCmdForUnblkUser (userList.at (0));
    }
}


void BlockUser::createCmdForUnblkUser (QString unblockUserName)
{
    payloadLib->setCnfgArrayAtIndex (0, unblockUserName);

    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = UNBLK_USR;
    param->payload = payloadString;

    applController->processActivity(m_currentDevName, DEVICE_COMM, param);
}

void BlockUser::handleInfoPageMessage (int)
{
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void BlockUser::forceFocusToPage(bool isFirstElement)
{
    if(isFirstElement)
    {
        m_currentElement = -1;
        takeRightKeyAction();
    }
    else
    {
        m_currentElement = (MAX_BLOCK_USER_CNTRL);
        takeLeftKeyAction();
    }
    m_elementList[m_currentElement]->forceActiveFocus ();
}

bool BlockUser::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (MAX_BLOCK_USER_CNTRL);
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

bool BlockUser::takeRightKeyAction ()
{
    bool status = true;
    do
    {
        if(m_currentElement == (MAX_BLOCK_USER_CNTRL - 1))
        {
            m_currentElement = -1;
        }
        if(m_currentElement != (MAX_BLOCK_USER_CNTRL - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
              status = false;
              break;
        }
    }while((m_currentElement < MAX_BLOCK_USER_CNTRL) && ((m_elementList[m_currentElement] == NULL)
           ||(!m_elementList[m_currentElement]->getIsEnabled())));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }

    return status;
}

void BlockUser::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void BlockUser::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void BlockUser::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void BlockUser::navigationKeyPressed (QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
        if(!takeLeftKeyAction ())
        {
            QWidget::keyPressEvent (event);
        }
        else
        {
            event->accept();
        }
        break;

    case Qt::Key_Right:
        if(!takeRightKeyAction())
        {
            QWidget::keyPressEvent (event);
        }
        else
        {
            event->accept();
        }
        break;

    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void BlockUser:: slotButtonClick(int index)
{
    switch (index)
    {
    case NEXT_BUTTON:
        fillUserList (1);
        m_prevButton->setIsEnabled (true);
        m_currentElement = PREV_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus ();
        m_nextButton->setIsEnabled (false);
        break;

    case PREV_BUTTON:
        fillUserList (0);
        m_nextButton->setIsEnabled (true);
        m_currentElement = NEXT_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus ();
        m_prevButton->setIsEnabled (false);
        break;

    default:
        break;

    }
}

void BlockUser:: slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e state, int index )
{
    if(state == ON_STATE)
    {
        if(!(userList.contains (usersName[index])))
        {
            userList.append (usersName[index]);
        }
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

void BlockUser:: slotCnfgButtonClick(int index)
{
    switch(index)
    {
    case UNBLOCK_CNFG_BUTTON:
        unBlockUser();
        break;

    case VIEW_ONLINE_CNFG_BUTTON:
        emit sigDispProfileChange ();
        break;

    default:
        break;
    }
}

void BlockUser::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
