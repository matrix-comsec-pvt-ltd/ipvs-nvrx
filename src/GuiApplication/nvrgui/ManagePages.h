#ifndef MANAGEPAGES_H
#define MANAGEPAGES_H

#include "ManagePages/ManageMenuOptions.h"
#include "ManagePages/AlaramOutput.h"
#include "ManagePages/ModifyPassword.h"
#include "ManagePages/SystemControl.h"
#include "ManagePages/Language.h"
#include "ManagePages/Users.h"
#include "ManagePages/ManagePasswordRecovery.h"
#include "Controls/MenuButton.h"
#include "Controls/DropDown.h"


#define MAX_SUB_SETTING_CONTOL_ELEMENT 2

typedef enum
{
    MANAGE_OPTION_USERS = 0,
    MANAGE_OPTION_STSTEM_CONTROL,
    MANAGE_OPTION_STSTEM_LANGUAGE,
    MANAGE_OPTION_MODIFY_PASSWORD,
    MANAGE_OPTION_ALARAM_OUTPUT,
    MANAGE_OPTION_PASSWORD_RECOVERY,
    MAX_MANAGE_OPTION
}MANAGE_OPTION_e;

typedef enum
{
    MNG_CLOSE_BTN,
    MNG_DROPBOX_CTRL,
    MNG_MANAGE_OPTION,
    MAX_MANAGE_CONTROL_ELEMENT = 8
}MANAGE_CONTROL_ELEMENT_e;


class ManagePages : public BackGround
{
    Q_OBJECT

public:

    explicit ManagePages(STATE_TYPE_e state,
                QWidget *parent = 0);
    ~ManagePages();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void updatePage(QString deviceName, qint8 status, qint8 index, quint8 eventType, quint8 eventSubType);
    void loadSubPage(quint8 optionIndex);
    void createSubPageObject(quint8 optionIndex);
    void deleteSubPageObject();
    void disconnectDeviceNotify(QString deviceName, bool forcePopup);
    void updateDeviceList(void);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeMenuKeyAction();

    void rightPanelClose ();

    void showEvent (QShowEvent *event);
    void hideEvent(QHideEvent *);

    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigButtonClick(int index);
    void sigDevNameChange(QString name);

public slots:
    void slotSubHeadingChange(bool);
    void slotManageOptions(int index);
    void slotSpinBoxValueChange(QString name,quint32 index);
    void slotUpdateCurrentElement(int index);
    void slotInfoPageBtnclick(int);
    void slotFocusToOtherElement(bool isPrevoiusElement);
    void slotCancelbuttonClick(void);

private:
    bool isUserPageCreated;
    bool isLanguagePageCreated;
    bool subElementFocusListEnable;
    bool isRightPanelOpen;
    bool isPasswordRecoveryPageCreated;

    quint8              m_optionIndex;
    quint8              m_currentElement;
    quint8              m_subcurrentElement;
    QString             m_currentDevName;
    QString             m_currLoginUser;

    DEV_TABLE_INFO_t    devTableInfo;
    STATE_TYPE_e        m_logState;

    DropDown*           deviceNameDropDown;
    MenuButton*         m_ManagePageOptions[MAX_MANAGE_OPTION];
    ManageMenuOptions*  m_manageMenuOptions;

    ApplController*     applController;

    NavigationControl*  m_elementList[MAX_MANAGE_CONTROL_ELEMENT];
    NavigationControl*  m_subelementList[MAX_SUB_SETTING_CONTOL_ELEMENT];

    InfoPage*           infoPage;
};

#endif // MANAGEPAGES_H
