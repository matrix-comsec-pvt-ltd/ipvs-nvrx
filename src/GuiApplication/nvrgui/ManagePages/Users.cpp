#include "Users.h"


Users::Users(QString devName,
             QWidget *parent) :
    ManageMenuOptions(devName, parent, 1)
{
    m_blockUser = NULL;
    /* PARASOFT: Memory Dealloacated in destructor */
    m_onlineUser = new OnlineUser(0,
                                  0,
                                  MANAGE_PAGE_RIGHT_PANEL_WIDTH,
                                  MANAGE_PAGE_RIGHT_PANEL_HEIGHT,
                                  devName,
                                  this);
    connect(m_onlineUser,
            SIGNAL(sigDispProfileChange()),
            this,
            SLOT(slotDispProfileChange()));

    connect (m_onlineUser,
             SIGNAL(sigInfoMsgDisplay(QString)),
             this,
             SLOT(slotInfoMsgDisplay(QString)));

    isOnlineUserVisible = true;
}

Users::~Users()
{
    if(m_onlineUser != NULL)
    {
        disconnect (m_onlineUser,
                    SIGNAL(sigInfoMsgDisplay(QString)),
                    this,
                    SLOT(slotInfoMsgDisplay(QString)));
        disconnect (m_onlineUser,
                    SIGNAL(sigDispProfileChange()),
                    this,
                    SLOT(slotDispProfileChange()));
        m_onlineUser->deleteLater ();
        m_onlineUser = NULL;
    }

    if (m_blockUser != NULL)
    {
        disconnect (m_blockUser,
                    SIGNAL(sigInfoMsgDisplay(QString)),
                    this,
                    SLOT(slotInfoMsgDisplay(QString)));
        disconnect (m_blockUser,
                    SIGNAL(sigDispProfileChange()),
                    this,
                    SLOT(slotDispProfileChange()));
        m_blockUser->deleteLater ();
        m_blockUser = NULL;
    }
}

void Users::forceFocusToPage(bool isFirstElement)
{
    if(m_onlineUser != NULL)
    {
        m_onlineUser->forceFocusToPage(isFirstElement);
    }
    if(m_blockUser != NULL)
    {
        m_blockUser->forceFocusToPage(isFirstElement);
    }
}

void Users:: handleInfoPageMessage(int buttonIndex)
{
    if(m_onlineUser != NULL)
    {
        m_onlineUser->handleInfoPageMessage(buttonIndex);
    }
    if(m_blockUser != NULL)
    {
        m_blockUser->handleInfoPageMessage(buttonIndex);
    }
}


void Users :: processDeviceResponse(DevCommParam *param,
                                    QString deviceName)
{
    if(param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
    }

    switch(param->cmdType)
    {
    case ONLINE_USR:
    case BLK_USR:
        if (m_onlineUser != NULL)
        {
            m_onlineUser->processDeviceResponse(param,deviceName);
        }
        break;

    case UNBLK_USR:
    case VIEW_BLK_USR:
        if (m_blockUser != NULL)
        {
            m_blockUser->processDeviceResponse(param,deviceName);
        }
        break;

    default:
        break;
    }
}

void Users::slotDispProfileChange()
{
    if( m_onlineUser != NULL)
    {
        disconnect (m_onlineUser,
                    SIGNAL(sigInfoMsgDisplay(QString)),
                    this,
                    SLOT(slotInfoMsgDisplay(QString)));
        disconnect (m_onlineUser,
                    SIGNAL(sigDispProfileChange()),
                    this,
                    SLOT(slotDispProfileChange()));
        m_onlineUser->deleteLater();
        m_onlineUser = NULL;

        emit sigSubHeadingChange(false);
        /* PARASOFT: Memory Dealloacated in destructor */
        m_blockUser = new BlockUser (0,
                                     0,
                                     MANAGE_PAGE_RIGHT_PANEL_WIDTH,
                                     MANAGE_PAGE_RIGHT_PANEL_HEIGHT,
                                     m_currentDeviceName,
                                     this);
        connect (m_blockUser,
                 SIGNAL(sigDispProfileChange()),
                 this,
                 SLOT(slotDispProfileChange()));
        connect (m_blockUser,
                 SIGNAL(sigInfoMsgDisplay(QString)),
                 this,
                 SLOT(slotInfoMsgDisplay(QString)));

    }
    else
    {
        disconnect (m_blockUser,
                    SIGNAL(sigInfoMsgDisplay(QString)),
                    this,
                    SLOT(slotInfoMsgDisplay(QString)));
        disconnect (m_blockUser,
                    SIGNAL(sigDispProfileChange()),
                    this,
                    SLOT(slotDispProfileChange()));
        m_blockUser->deleteLater();
        m_blockUser = NULL;

        emit sigSubHeadingChange(true);
        /* PARASOFT: Memory Dealloacated in destructor */
        m_onlineUser = new OnlineUser(0,
                                      0,
                                      MANAGE_PAGE_RIGHT_PANEL_WIDTH,
                                      MANAGE_PAGE_RIGHT_PANEL_HEIGHT,
                                      m_currentDeviceName,
                                      this);
        connect (m_onlineUser,
                 SIGNAL(sigDispProfileChange()),
                 this,
                 SLOT(slotDispProfileChange()));

        connect (m_onlineUser,
                 SIGNAL(sigInfoMsgDisplay(QString)),
                 this,
                 SLOT(slotInfoMsgDisplay(QString)));
    }
}

void Users::slotInfoMsgDisplay (QString str)
{
    infoPage->loadInfoPage(str);
}
