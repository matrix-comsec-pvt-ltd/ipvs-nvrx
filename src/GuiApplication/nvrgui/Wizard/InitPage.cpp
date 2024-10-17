#include "InitPage.h"

#define WIZARD_WELCOME_TEXT_STR             "Welcome to NVR Quick Setup Installation."
#define WIZARD_SETUP_TEXT_STR               "This Setup will help in configuring the basic settings necessary for device to function appropriately."
#define WIZARD_SETUP_CHECKBOX_STR           "Always run wizard on device startup?"
#define WIZARD_WELCOME_TEXT_FONT_COLOR      "#c0c0c0"
#define WIZARD_TEXT_FONT_COLOR              "#8c8c8b"

#define WIZARD_WELCOME_TEXT_FONT_SIZE       SCALE_FONT(18)
#define WIZARD_TEXT_FONT_SIZE               SCALE_FONT(16)
#define BGTILE_HEADING_WIDTH                SCALE_WIDTH(1080)
#define BGTILE_HEADING_HEIGHT               SCALE_HEIGHT(550)

InitPage::InitPage(QString devName, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId):WizardCommon(parent, pageId)
{
    INIT_OBJ(m_welcomeTextLabel);
    INIT_OBJ(m_wizardSetupTextLabel);
    INIT_OBJ(m_wizardRunTextLabel);
    INIT_OBJ(m_wizardRunCheckBox);
    INIT_OBJ(m_modifyPassword);
    INIT_OBJ(m_initChangePass);
    INIT_OBJ(m_inVisibleWidget);

    m_currentDevName = devName;
    m_applController = ApplController::getInstance();
    m_payloadLib = new PayloadLib();
    createDefaultElements();
}

void InitPage::createDefaultElements ()
{
    m_welcomeTextLabel = new TextLabel(SCALE_WIDTH(68), SCALE_HEIGHT(30), WIZARD_WELCOME_TEXT_FONT_SIZE,
                                       WIZARD_WELCOME_TEXT_STR, this, WIZARD_WELCOME_TEXT_FONT_COLOR, NORMAL_FONT_FAMILY,
                                       ALIGN_START_X_START_Y, 0, false, SCALE_WIDTH(1000));

    m_wizardSetupTextLabel = new TextLabel(SCALE_WIDTH(68) , m_welcomeTextLabel->y() + SCALE_HEIGHT(65), WIZARD_TEXT_FONT_SIZE,
                                           WIZARD_SETUP_TEXT_STR, this, WIZARD_TEXT_FONT_COLOR, NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_START_Y, 0, false, SCALE_WIDTH(1000));

    m_wizardRunCheckBox = new OptionSelectButton(SCALE_WIDTH(68),
                                                 m_wizardSetupTextLabel->y() + SCALE_HEIGHT(65),
                                                 SCALE_HEIGHT(20),
                                                 BGTILE_HEIGHT,CHECK_BUTTON_INDEX,
                                                 this, NO_LAYER,
                                                 "", "",SCALE_HEIGHT(20),
                                                 0,true,
                                                 NORMAL_FONT_SIZE);

    getOtherUserParam();
    m_wizardRunTextLabel = new TextLabel(SCALE_WIDTH(100), m_wizardSetupTextLabel->y() + SCALE_HEIGHT(75), WIZARD_TEXT_FONT_SIZE,
                                         WIZARD_SETUP_CHECKBOX_STR, this, BLUE_COLOR, NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_START_Y, 0, false, SCALE_WIDTH(950));

    m_initChangePass = new TextLabel(BGTILE_HEADING_WIDTH - SCALE_WIDTH(30),
                                     BGTILE_HEADING_HEIGHT-SCALE_HEIGHT(30),
                                     (WIZARD_TEXT_FONT_SIZE),
                                     "Change Password",
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_END_X_END_Y,
                                     0,
                                     true,
                                     0,
                                     0);
    connect(m_initChangePass,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_initChangePass,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));
    this->show();
}

void InitPage:: rememberStatusToFile()
{
    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, WRITE_WIZARD_PARAM);

    WizOpenStatus.wizardOpen = (m_wizardRunCheckBox->getCurrentState () == ON_STATE) ? true : false;
    m_applController->processActivity(OTHER_LOGIN_ACTIVITY, paramList, &WizOpenStatus);
}

void InitPage::getOtherUserParam()
{
    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, READ_WIZARD_PARAM);

    if(m_applController->processActivity(OTHER_LOGIN_ACTIVITY, paramList, &WizOpenStatus) == true)
    {
        m_wizardRunCheckBox->changeState((WizOpenStatus.wizardOpen == true) ? ON_STATE : OFF_STATE);
    }
}

InitPage::~InitPage()
{
    DELETE_OBJ(m_welcomeTextLabel);
    DELETE_OBJ(m_wizardSetupTextLabel);
    DELETE_OBJ(m_wizardRunCheckBox);
    DELETE_OBJ(m_wizardRunTextLabel);

    if(IS_VALID_OBJ(m_initChangePass))
    {
        disconnect(m_initChangePass,
                SIGNAL(sigTextClick(int)),
                this,
                SLOT(slotTextClicked(int)));
        disconnect(m_initChangePass,
                SIGNAL(sigMouseHover(int,bool)),
                this,
                SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_initChangePass);
    }

    if(IS_VALID_OBJ(m_modifyPassword))
    {
        disconnect(m_modifyPassword,
                   SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotcloseButtonClick(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(m_modifyPassword,
                   SIGNAL(sigExitPage(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotcloseButtonClick(TOOLBAR_BUTTON_TYPE_e)));
        DELETE_OBJ(m_modifyPassword);
        DELETE_OBJ(m_inVisibleWidget);
    }    

    DELETE_OBJ(m_payloadLib);
}

void InitPage :: saveConfig()
{

}

void InitPage :: processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(IS_VALID_OBJ(m_modifyPassword))
    {
        m_modifyPassword->processDeviceResponse(param,deviceName);
    }
}

void InitPage::slotTextClicked(int)
{
    m_inVisibleWidget = new InvisibleWidgetCntrl(m_parent);
    m_modifyPassword = new WizardChangePass(m_currentDevName, m_parent, (STATE_TYPE_e)1);
    connect(m_modifyPassword,
            SIGNAL(sigExitPage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotcloseButtonClick(TOOLBAR_BUTTON_TYPE_e)));
}

void InitPage::slotTextLableHover(int, bool isHoverIn)
{
    if(isHoverIn == true)
    {
        m_initChangePass->changeColor(MOUSE_HOWER_COLOR);
        m_initChangePass->repaint();
    }
    else
    {
        m_initChangePass->changeColor(HIGHLITED_FONT_COLOR);
        m_initChangePass->repaint();
    }
}

void InitPage :: slotcloseButtonClick(TOOLBAR_BUTTON_TYPE_e)
{
    if(IS_VALID_OBJ(m_modifyPassword))
    {
        disconnect(m_modifyPassword,
                SIGNAL(sigExitPage(TOOLBAR_BUTTON_TYPE_e)),
                this,
                SLOT(slotcloseButtonClick(TOOLBAR_BUTTON_TYPE_e)));
        DELETE_OBJ(m_modifyPassword);
    }

    DELETE_OBJ(m_inVisibleWidget);
}
