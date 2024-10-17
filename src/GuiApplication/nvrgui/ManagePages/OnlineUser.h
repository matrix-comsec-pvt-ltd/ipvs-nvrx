#ifndef ONLINEUSER_H
#define ONLINEUSER_H

#include "ManageMenuOptions.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/TextLabel.h"
#include "Controls/TableCell.h"
#include "KeyBoard.h"

#define BGTILE_WIDTH                    SCALE_WIDTH(671)     //(513 + 120 + 38)
#define MAX_CELL_ON_PAGE                7
#define MAX_USERS                       9
#define USER_LEFT_MARGIN                SCALE_WIDTH(20)
#define USER_NAME_HEADER_WIDTH          SCALE_WIDTH(150)
#define USER_GROUP_HEADER_WIDTH         SCALE_WIDTH(110)
#define IP_HEADER_WIDTH                 SCALE_WIDTH(390)
#define TABLE_CELL_HEIGHT               SCALE_HEIGHT(30)
#define LABEL_FONT_SIZE                 SCALE_FONT(15)


typedef enum {
    USER_OPTION_SELECTION_BUTTON,
    USER_PREV_BUTTON = MAX_CELL_ON_PAGE,
    USER_BLOCK_CNFG_BUTTON,
    USER_VIEW_BLOCK_CNFG_BUTTON,
    USER_NEXT_BUTTON,
    MAX_USER_CONTROL_ELEMENT
} USER_CONTROL_ELEMENT_e;

typedef enum {
    USERNAME_STRING,
    USERGROUP_STRING,
    IPHEADER_STRING,
    BLOCK_STRING,
    VIEW_BLOCK_STRING,
    MAX_ONLINE_USER_STRINGS
} ONLINE_USER_STRINGS_e;

class OnlineUser :  public KeyBoard
{
    Q_OBJECT

    ApplController *applController;
    PayloadLib *payloadLib;

    qint8 m_width ;
    qint8 m_height;

    BgTile* m_bgBottom;
    BgTile* m_bgTop;
    BgTile* m_bgmiddle;

    quint8 currentClickIndex;

    ControlButton* m_prevButton;
    ControlButton* m_nextButton;

    CnfgButton* m_blockButton;
    CnfgButton* m_viewBlockButton;

    TableCell* m_userNameHeader;
    TextLabel* m_userNameLabel;

    TableCell* m_userGroupHeader;
    TextLabel* m_userGroupLabel;

    TableCell* m_ipHeader;
    TextLabel* m_ipLabel;

    TableCell* m_userNameCell[MAX_USERS];
    TextLabel* m_userNameCellLabel[MAX_USERS];

    TableCell* m_userGroupCell[MAX_USERS];
    TextLabel* m_userGroupCellLabel[MAX_USERS];

    TableCell* m_ipCell[MAX_USERS];
    TextLabel* m_ipCellLabel[MAX_USERS];

    OptionSelectButton* m_optionSelectButton[MAX_CELL_ON_PAGE];

    QStringList m_devList;
    quint8 m_devListCount;
    QString m_currentDevName;

    qint8 m_numberOfOnlineUser;

    QString usersName[MAX_USERS];
    QString userGroup[MAX_USERS];
    QString ipAddress[MAX_USERS];

    NavigationControl* m_elementList[MAX_USER_CONTROL_ELEMENT];
    quint32 m_currentElement;

    QStringList userList;

public:
    OnlineUser(qint32 startX,
               qint32 startY,
               quint32 width,
               quint32 height,
               QString deviceName,
               QWidget *parent = 0);

    ~OnlineUser();

    void createElements();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getOnlineUserFrmDev();
    void fillUserList(quint8 pageIndex);
    void changeUserDispList();
    void blockUser();
    void createCmdForBlkUser(QString blockUserName);

    void handleInfoPageMessage(int buttonIndex);
    void forceFocusToPage(bool isFirstElement);
    //    void forceActiveFocus ();
    bool takeLeftKeyAction ();
    bool takeRightKeyAction ();
    void showEvent (QShowEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);

signals:
    void sigDispProfileChange();
    void sigInfoMsgDisplay(QString);

public slots:
    void slotButtonClick(int);
    void slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e state, int index );
    void slotCnfgButtonClick(int index);
    void slotUpdateCurrentElement(int index);
};

#endif // ONLINEUSER_H
