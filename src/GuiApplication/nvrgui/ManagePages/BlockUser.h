#ifndef BLOCKUSER_H
#define BLOCKUSER_H

#include "ManageMenuOptions.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/TextLabel.h"
#include "Controls/TableCell.h"
#include "KeyBoard.h"

#define MAX_CELL                    7
#define MAX_BLK_USERS               9
#define BLK_USER_LEFT_MARGIN        SCALE_WIDTH(20)
#define BLK_USER_BGTILE_WIDTH       SCALE_WIDTH(671)     //(513 + 120 + 38)
#define BLK_USER_NAME_HEADER_WIDTH  SCALE_WIDTH(460)     //(370 + 90)
#define BLK_USER_GROUP_HEADER_WIDTH SCALE_WIDTH(192)     //(123 + 67)
#define BLK_USER_TABLE_CELL_HEIGHT  SCALE_HEIGHT(30)

typedef enum {
    OPTION_SELECTION_BUTTON,
    PREV_BUTTON = MAX_CELL,
    UNBLOCK_CNFG_BUTTON,
    VIEW_ONLINE_CNFG_BUTTON,
    NEXT_BUTTON,
    MAX_BLOCK_USER_CNTRL
} BLOCK_USER_CNTRL_e;

typedef enum {
    USER_NAME_STRING,
    USER_GROUP_STRING,
    UNBLOCK_STRING,
    VIEW_ONLINE_USER_STRING,
    MAX_BLOCK_USER_STRING
} BLOCK_USER_STRING_e;

class BlockUser : public KeyBoard
{
    Q_OBJECT

    ApplController *applController;
    PayloadLib *payloadLib;

    quint32 m_width ;
    quint32 m_height;
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

    TableCell* m_userNameCell[MAX_BLK_USERS];
    TextLabel* m_userNameCellLabel[MAX_BLK_USERS];

    TableCell* m_userGroupCell[MAX_BLK_USERS];
    TextLabel* m_userGroupCellLabel[MAX_BLK_USERS];

    OptionSelectButton* m_optionSelectButton[MAX_CELL];

    QStringList m_devList;
    quint8 m_devListCount;
    QString m_currentDevName;

    qint8 m_numberOfBlockUser;

    QString usersName[MAX_BLK_USERS];
    QString userGroup[MAX_BLK_USERS];

    NavigationControl* m_elementList[MAX_BLOCK_USER_CNTRL];
    quint32 m_currentElement;

    QStringList userList;


public:
    BlockUser(qint32 startX,
              qint32 startY,
              quint32 width,
              quint32 height,
              QString deviceName,
              QWidget *parent = 0);
    ~BlockUser();

    void createDefaultElements();
    void getBlockDevicelist();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void fillUserList(quint8 pageIndex);
    void unBlockUser();
    void createCmdForUnblkUser(QString unblockUserName);

    void handleInfoPageMessage(int buttonIndex);
    void forceFocusToPage(bool isFirstElement);
    bool takeLeftKeyAction();
    bool takeRightKeyAction();
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void showEvent (QShowEvent *event);

signals:
    void sigDispProfileChange();
    void sigInfoMsgDisplay(QString);

public slots:
    void slotButtonClick(int);
    void slotOptionSelectButtonClicked(OPTION_STATE_TYPE_e state, int index );
    void slotCnfgButtonClick(int index);
    void slotUpdateCurrentElement(int index);

};

#endif // BLOCKUSER_H
