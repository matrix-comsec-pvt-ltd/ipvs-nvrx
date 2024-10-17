#ifndef USERS_H
#define USERS_H

#include <QWidget>

#include "ManageMenuOptions.h"
#include "ManagePages/OnlineUser.h"
#include "ManagePages/BlockUser.h"

class Users : public ManageMenuOptions
{
    Q_OBJECT

    bool isOnlineUserVisible;
    OnlineUser* m_onlineUser;
    BlockUser* m_blockUser;

public:
    explicit Users(QString deviceName,
          QWidget *parent = 0);

    ~Users();

    void processDeviceResponse(DevCommParam *param, QString devName);
    void forceFocusToPage(bool isFirstElement);
    void handleInfoPageMessage(int buttonIndex);

public slots:
    void slotDispProfileChange();    
    void slotInfoMsgDisplay(QString);
};

#endif // USERS_H
