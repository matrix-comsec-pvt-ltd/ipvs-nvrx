///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : SATATYA DEVICES
//   Owner        :  Kaushal Patel
//   File         : MxAutoAddCam.h
//   Description  : This file is used to give pop up of auto add cam on boot.
/////////////////////////////////////////////////////////////////////////////

#ifndef MXAUTOADDCAM_H
#define MXAUTOADDCAM_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/CnfgButton.h"
#include "Controls/TextLabel.h"
#include "Controls/BackGround.h"
#include "NavigationControl.h"
#include "DataStructure.h"
#include "ConfigField.h"
#include "PayloadLib.h"
#include "ApplController.h"
#include "Controls/InvisibleWidgetCntrl.h"
#include "Controls/InfoPage.h"
#include "EnumFile.h"
#include "Controls/Bgtile.h"
#include "Controls/TextBox.h"
#include "Controls/TileWithText.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/ConfigPageControl.h"



#define AUTO_ADD_CONFORMATION_BTN_YES    "Yes"
#define AUTO_ADD_CONFORMATION_BTN_NO     "No"
#define AUTO_ADD_CONFORMATION_BTN_SAVE   "Save"

typedef enum
{
    AUTO_ADD_CLS_BTN,
    AUTO_ADD_YES_BTN,
    AUTO_ADD_NO_BTN,
    AUTO_ADD_CHANGE_CONFIG_LABEL,
    MX_AUTO_ADD_CAM_CAMERA_CRDENTIAL_IMAGE,
    AUTO_ADD_USERNAME,
    AUTO_ADD_PASSWORD,
    AUTO_ADD_SAVE_BTN,

    MAX_AUTO_ADD_ELEMENTS
}AUTO_ADD_ELEMENTS_e;

typedef enum
{
    MX_AUTO_ADD_CAM_PAUSE_PLAYBACK,
    MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD,
    MX_AUTO_ADD_CAM_LOGIN,
    MX_AUTO_ADD_CAM_TIME_OUT,
    MAX_MX_AUTO_ADD_CAM_SEARCH_ACTION
}MX_AUTO_ADD_CAM_SEARCH_ACTION_e;

class MxAutoAddCam : public BackGround
{
    Q_OBJECT

public:

    explicit MxAutoAddCam(QWidget *parent = 0);
    ~MxAutoAddCam();

    void processDeviceResponse (DevCommParam *param, QString);
    void setIsInfoPageLoaded(bool state);
    void setIsCamSearchPageLoaded(bool state);
    QString getPayloadList();
    void showHideElements(bool flag);
    void getConfig();
    void saveConfig();
    void raiseVirtualKeypad();
    bool validationOnSaveBtn();
    void saveConfiguration();
    void unloadAutoAddCam();

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void navigationKeyPressed(QKeyEvent *event);

public slots:
    void slotUpdateCurrentElement (int index);
    void slotCnfgBtnClicked (int index);
    void slotCloseButtonClicked(int);
    void slotSearchDurationTimeOut();
    void slotAqrListTimeOut();
    void slotTextClicked(int);
    void slotTextLableHover(int,bool);
    void slotImageClicked(int);
    void slotTileWithTextClick(int);
    void slotButtonClick(int);
    void slotInfoPageBtnclick(int);
    void loadAutoAddCamAfterLogin();

signals:
    void sigCloseAlert();
    void sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e);


private:
    int                     m_currElement;
    bool                    m_isAutoSearchStarted;
    bool                    m_isInfoPageLoaded;
    bool                    m_isCamSearchPageLoaded;
    bool                    m_isCancelSend;
    bool                    m_isShowCameraCrdntialSettings;
    bool                    m_isShowElement;
    quint8                  m_autoSrchRetryCnt;

    BgTile*                 m_insideTile;
    TileWithText*           m_changeCameraCrdntialTile;
    BgTile*                 m_complTile;
    TextLabel*              m_conformationStr;
    TextLabel*              m_camNoStr;
    TextLabel*              m_devResponseLabel;
    TextLabel*              m_customizeConfigLable;
    CnfgButton*             m_yesButton;
    CnfgButton*             m_noButton;
    CnfgButton*             m_saveButton;
    NavigationControl*      m_elementList[MAX_AUTO_ADD_ELEMENTS];
    PayloadLib*             m_payloadLib;
    ApplController*         m_applController;
    QTimer*                 m_searchDurationTimer;
    QTimer*                 m_getAqrListTimer;
    InvisibleWidgetCntrl*   m_inVisibleWidget;

    QStringList         m_searchList;
    QStringList         m_camStatusList;
    QStringList         m_ipv4AddressList;
    QStringList         m_ipv6AddressList;
    QStringList         m_httpPortList;
    QStringList         m_onvifPortList;
    QStringList         m_brandList;
    QStringList         m_modelList;
    QStringList         m_onvifSupportStatusList;
    QStringList         m_camNumList;
    QStringList         m_camNameList;
    Image*              m_changeCameraCrdntialImage;
    Image*              m_disabledImage;
    AUTO_ADD_ELEMENTS_e m_currentButton;
    bool                m_retainIpAddrInAutoConfig;
    qint32              m_autoConfigIpAddrFamily;

    QString             m_payload;
    QString             m_userName;
    QString             m_password;
    TextBox*            m_userNameTextBox;
    TextboxParam*       m_userNameTextBoxParam;

    PasswordTextbox*    m_passwordTextBox;
    TextboxParam*       m_passwordTextBoxParam;
    InfoPage*           m_infoPage;
    QString             m_currDevName;
    bool                m_isYesbtnclick;
    

    void createDefaultComponent();
    void loadAutoAddCam(bool isShowElement);
    void changeText(quint8 camNo);

    void clearAllList();
    void updateList(DevCommParam *param);

    void sendAutoAddCmd();
    void sendCommand(SET_COMMAND_e cmdType, quint32 totalfeilds = 0);
};

#endif // MXAUTOADDCAM_H
