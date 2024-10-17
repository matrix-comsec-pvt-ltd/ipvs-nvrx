#include "SetupWizard.h"
#include "WizardCommon.h"

#define QUICK_SETUP_STRING      "Quick Setup"
#define CNFG_QUIT_BTN_STRING    "Quit"
#define CNFG_NEXT_BTN_STRING    "Next"
#define CNFG_BACK_BTN_STRING    "Back"
#define CNFG_DONE_BTN_STRING    "Done"
#define PAGE_HEADING_HEIGHT     SCALE_HEIGHT(40)
#define QUIT_BTN_WIDTH          SCALE_HEIGHT(90)
#define BACK_BTN_WIDTH          SCALE_HEIGHT(930)
#define SAVE_BTN_WIDTH          SCALE_HEIGHT(1055)
#define BTN_HEIGHT              SCALE_HEIGHT(700)
#define AUTO_CNFG_FIELD         20
#define CNFG_TO_INDEX           1
#define INITIAL_TEXT_COLOR      "#8c8c8b"
#define CONFIGURED_TEXT_COLOR   "#d9d9d9"

typedef enum
{
    ELE_QUIT_BTN,
    ELE_NEXT_BTN,
    ELE_BACK_BTN,
    ELE_MAX_BTN
}ELEMENT_INDEX_e;

static const QString quickSetupTabs[MAX_WIZ_PG] =
{
    "",
    "Time and Language",
    "Network",
    "DHCP Server",
    "Storage",
    "HDD Group",
    "Configure Camera",
    "Search Camera",
    "Status",
};

SetupWizard::SetupWizard(QString devName, QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - WIZARD_MAIN_RECT_WIDTH) / 2)),
               (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - WIZARD_MAIN_RECT_HEIGHT) / 2)),
               WIZARD_MAIN_RECT_WIDTH, (WIZARD_MAIN_RECT_HEIGHT), BACKGROUND_TYPE_1, MAX_TOOLBAR_BUTTON,
               parent, 0, 0, "", "", false), NavigationControl(0, true)
{
    int initialOffset, spaceBtwnLabel, tXpos, pgIdxOffset;
    m_isLanguagePageCreated = false;
    nextPIndex = MAX_WIZ_PG;
    INIT_OBJ(m_pageHeading);
    INIT_OBJ(m_quitBtn);
    INIT_OBJ(m_backBtn);
    INIT_OBJ(m_nextBtn);
    INIT_OBJ(m_heading_1);
    INIT_OBJ(m_datenTime);
    INIT_OBJ(m_network);
    INIT_OBJ(m_dhcpServer);
    INIT_OBJ(m_storage);
    INIT_OBJ(m_hddGroup);
    INIT_OBJ(m_configureCamera);
    INIT_OBJ(m_searchCamera);
    INIT_OBJ(m_status);
    INIT_OBJ(m_setupPage);
    INIT_OBJ(m_infoPage);

    m_currentDevName = devName;
    applController = ApplController::getInstance();
    applController->GetDeviceInfo(m_currentDevName, devTable);

    /* Display HDD Group if current variant is NVR9608XP2, NVR3202XP2, NVR3204XP2, NVR6404XP2 and NVR6408XP2 */
    #if defined(RK3588_NVRH)
    isHddGrpCreate = true;
    initialOffset = 40;
    spaceBtwnLabel = 15;
    pgIdxOffset = 0;
    #else
    isHddGrpCreate = false;
    initialOffset = 60;
    spaceBtwnLabel = 28;
    pgIdxOffset = 1;
    #endif

    for(quint8 index = 0; index < MAX_WIZ_PG; index++)
    {
        m_visitPageStatus[index] = false;
    }

    m_pageHeading = new Heading(SCALE_WIDTH(1145/2), SCALE_HEIGHT(30), QUICK_SETUP_STRING, this,
                              HEADING_TYPE_1,
                              SCALE_FONT(20));

    m_quitBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               QUIT_BTN_WIDTH,
                               BTN_HEIGHT,
                               CNFG_QUIT_BTN_STRING,
                               this, ELE_QUIT_BTN,true);
    connect (m_quitBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonOrLinkClick(int)));

    m_infoPage = new InfoPage((ApplController::getXPosOfScreen() + ApplController::getWidthOfScreen())/2 - SCALE_WIDTH(500)/2,
                               ((ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen())/2) - SCALE_WIDTH(300)/2,
                               SCALE_WIDTH(500),
                               SCALE_HEIGHT(300),
                               MAX_INFO_PAGE_TYPE,
                               parentWidget());
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    m_backBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               BACK_BTN_WIDTH,
                               BTN_HEIGHT,
                               CNFG_BACK_BTN_STRING,
                               this, ELE_BACK_BTN,true);
    connect (m_backBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonOrLinkClick(int)));
    m_backBtn->setVisible(false);

    m_nextBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               SAVE_BTN_WIDTH,
                               BTN_HEIGHT,
                               CNFG_NEXT_BTN_STRING,
                               this, ELE_NEXT_BTN,true);
    connect (m_nextBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonOrLinkClick(int)));

    m_heading_1 = new ElementHeading(SCALE_WIDTH(30),
                                     SCALE_HEIGHT(60),
                                     SCALE_WIDTH(1080),
                                     SCALE_HEIGHT(40),
                                     "",
                                     COMMON_LAYER,
                                     this,
                                     false,
                                     SCALE_WIDTH(10),
                                     SCALE_WIDTH(15));

    QString labelStr = QString("%1.%2").arg(WIZ_PG_DTTIME).arg(Multilang(quickSetupTabs[WIZ_PG_DTTIME].toUtf8().constData()));
    quint16 tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_datenTime = new TextLabel(SCALE_WIDTH(initialOffset),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_DTTIME);
    connect(m_datenTime,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_datenTime,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    labelStr = QString("%1.%2").arg(WIZ_PG_NTWCONF).arg(Multilang(quickSetupTabs[WIZ_PG_NTWCONF].toUtf8().constData()));
    tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_network = new TextLabel(m_datenTime->x() + m_datenTime->width() + SCALE_WIDTH(spaceBtwnLabel),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_NTWCONF);
    connect(m_network,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_network,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    labelStr = QString("%1.%2").arg(WIZ_PG_DHCPSERVER).arg(Multilang(quickSetupTabs[WIZ_PG_DHCPSERVER].toUtf8().constData()));
    tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_dhcpServer = new TextLabel(m_network->x() + m_network->width() + SCALE_WIDTH(spaceBtwnLabel),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_DHCPSERVER);
    connect(m_dhcpServer,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_dhcpServer,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    labelStr = QString("%1.%2").arg(WIZ_PG_STRGCONF).arg(Multilang(quickSetupTabs[WIZ_PG_STRGCONF].toUtf8().constData()));
    tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_storage = new TextLabel((m_dhcpServer->x() + m_dhcpServer->width() + SCALE_WIDTH(spaceBtwnLabel)),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_STRGCONF);
    connect(m_storage,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_storage,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    if (isHddGrpCreate == true)
    {
        labelStr = QString("%1.%2").arg(WIZ_PG_HDDGROUP).arg(Multilang(quickSetupTabs[WIZ_PG_HDDGROUP].toUtf8().constData()));
        tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
        m_hddGroup = new TextLabel((m_storage->x() + m_storage->width() + SCALE_WIDTH(spaceBtwnLabel)),
                                    (m_heading_1->y() + (m_heading_1->height() / 2)),
                                    NORMAL_FONT_SIZE,
                                    labelStr,
                                    this,
                                    INITIAL_TEXT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y,
                                    0,
                                    false,
                                    tempWidth,
                                    WIZ_PG_HDDGROUP);
        connect(m_hddGroup,
                SIGNAL(sigTextClick(int)),
                this,
                SLOT(slotTextClicked(int)));
        connect(m_hddGroup,
                SIGNAL(sigMouseHover(int,bool)),
                this,
                SLOT(slotTextLableHover(int,bool)));

        tXpos = m_hddGroup->x() + m_hddGroup->width() + SCALE_WIDTH(spaceBtwnLabel);
    }
    else
    {
        tXpos = m_storage->x() + m_storage->width() + SCALE_WIDTH(spaceBtwnLabel);
    }

    labelStr = QString("%1.%2").arg(WIZ_PG_AUTOCONF - pgIdxOffset).arg(Multilang(quickSetupTabs[WIZ_PG_AUTOCONF].toUtf8().constData()));
    tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_configureCamera = new TextLabel(tXpos, (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_AUTOCONF);
    connect(m_configureCamera,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_configureCamera,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    labelStr = QString("%1.%2").arg(WIZ_PG_CAMSRCH - pgIdxOffset).arg(Multilang(quickSetupTabs[WIZ_PG_CAMSRCH].toUtf8().constData()));
    tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_searchCamera = new TextLabel((m_configureCamera->x() + m_configureCamera->width() + SCALE_WIDTH(spaceBtwnLabel)),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_CAMSRCH);
    connect(m_searchCamera,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_searchCamera,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    labelStr = QString("%1.%2").arg(WIZ_PG_STATUS - pgIdxOffset).arg(Multilang(quickSetupTabs[WIZ_PG_STATUS].toUtf8().constData()));
    tempWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelStr);
    m_status = new TextLabel((m_searchCamera->x() + m_searchCamera->width() + SCALE_WIDTH(spaceBtwnLabel)),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                NORMAL_FONT_SIZE,
                                labelStr,
                                this,
                                INITIAL_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y,
                                0,
                                false,
                                tempWidth,
                                WIZ_PG_STATUS);
    connect(m_status,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_status,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    hideStepper();
    m_setupPage = new InitPage(devName, this, WIZ_PG_INIT);
}

SetupWizard::~SetupWizard()
{
    if(m_isLanguagePageCreated)
    {
        if(IS_VALID_OBJ(m_setupPage))
        {
            disconnect(m_setupPage,
                       SIGNAL(sigLanguageCfgChanged(QString)),
                       applController,
                       SLOT(slotLanguageCfgChanged(QString)));
        }
        m_isLanguagePageCreated = false;
    }

    DELETE_OBJ(m_pageHeading);

    if(IS_VALID_OBJ(m_quitBtn))
    {
        disconnect(m_quitBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonOrLinkClick(int)));
        DELETE_OBJ(m_quitBtn);
    }

    if(IS_VALID_OBJ(m_backBtn))
    {
        disconnect(m_backBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonOrLinkClick(int)));
        DELETE_OBJ(m_backBtn);
    }

    if(IS_VALID_OBJ(m_nextBtn))
    {
        disconnect(m_nextBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonOrLinkClick(int)));
        DELETE_OBJ(m_nextBtn);
    }

    DELETE_OBJ(m_heading_1);

    if(IS_VALID_OBJ(m_datenTime))
    {
        disconnect(m_datenTime,
                   SIGNAL(sigTextClick(int)),
                   this,
                   SLOT(slotTextClicked(int)));
        disconnect(m_datenTime,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_datenTime);
    }

    if(IS_VALID_OBJ(m_network))
    {
        disconnect(m_network,
                 SIGNAL(sigTextClick(int)),
                 this,
                 SLOT(slotTextClicked(int)));
        disconnect(m_network,
                 SIGNAL(sigMouseHover(int,bool)),
                 this,
                 SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_network);
    }

    if(IS_VALID_OBJ(m_dhcpServer))
    {
        disconnect(m_dhcpServer,
                 SIGNAL(sigTextClick(int)),
                 this,
                 SLOT(slotTextClicked(int)));
        disconnect(m_dhcpServer,
                 SIGNAL(sigMouseHover(int,bool)),
                 this,
                 SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_dhcpServer);
    }

    if(IS_VALID_OBJ(m_storage))
    {
        disconnect(m_storage,
                   SIGNAL(sigTextClick(int)),
                   this,
                   SLOT(slotTextClicked(int)));
        disconnect(m_storage,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_storage);
    }

    if(IS_VALID_OBJ(m_hddGroup))
    {
        disconnect(m_hddGroup,
                SIGNAL(sigTextClick(int)),
                this,
                SLOT(slotTextClicked(int)));
        disconnect(m_hddGroup,
                SIGNAL(sigMouseHover(int,bool)),
                this,
                SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_hddGroup);
    }

    if(IS_VALID_OBJ(m_configureCamera))
    {
        disconnect(m_configureCamera,
                 SIGNAL(sigTextClick(int)),
                 this,
                 SLOT(slotTextClicked(int)));
        disconnect(m_configureCamera,
                 SIGNAL(sigMouseHover(int,bool)),
                 this,
                 SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_configureCamera);
    }

    if(IS_VALID_OBJ(m_searchCamera))
    {
        disconnect(m_searchCamera,
                  SIGNAL(sigTextClick(int)),
                  this,
                  SLOT(slotTextClicked(int)));
        disconnect(m_searchCamera,
                  SIGNAL(sigMouseHover(int,bool)),
                  this,
                  SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_searchCamera);
    }

    if(IS_VALID_OBJ(m_status))
    {
        disconnect(m_status,
                  SIGNAL(sigTextClick(int)),
                  this,
                  SLOT(slotTextClicked(int)));
        disconnect(m_status,
                  SIGNAL(sigMouseHover(int,bool)),
                  this,
                  SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_status);
    }

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect (m_infoPage,
                    SIGNAL(sigInfoPageCnfgBtnClick(int)),
                    this,
                    SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(m_infoPage);
    }

    DELETE_OBJ(m_setupPage);
}

void SetupWizard::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if((deviceName == m_currentDevName) && (IS_VALID_OBJ(m_setupPage)))
    {
        m_setupPage->processDeviceResponse(param, deviceName);
        if((m_setupPage->isObjDeleteReq()))
        {
            updatePage(m_setupPage->getCurrPageIndex(), false, CONFIGURED_TEXT_COLOR);
            DELETE_OBJ(m_setupPage);
            openNextPrevPage(nextPIndex);
        }
    }
}

void SetupWizard::slotButtonOrLinkClick(int index)
{
    switch (index)
    {
        case ELE_QUIT_BTN:
        {
            quitButton();
        }
        break;

        case ELE_NEXT_BTN:
        {
            if((IS_VALID_OBJ(m_setupPage)) && ((m_setupPage->getCurrPageIndex()) <= WIZ_PG_STATUS))
            {
                nextPIndex = m_setupPage->getNextPrevPageIndex(NEXT_PG);
                if ((isHddGrpCreate == false) && (nextPIndex == WIZ_PG_HDDGROUP))
                {
                    nextPIndex = WIZARD_PAGE_INDEXES_e(nextPIndex + 1);
                }

                if(m_setupPage->getCurrPageIndex() == WIZ_PG_STATUS)
                {
                    DELETE_OBJ(m_setupPage);
                    emit sigQuitSetupWiz();
                }
                else if(m_setupPage->getCurrPageIndex() == WIZ_PG_INIT)
                {
                    ((InitPage*)m_setupPage)->rememberStatusToFile();
                    updatePage(m_setupPage->getCurrPageIndex(), false, CONFIGURED_TEXT_COLOR);
                    DELETE_OBJ(m_setupPage);
                    openNextPrevPage(nextPIndex);
                }
                else
                {
                    if(m_setupPage->getCurrPageIndex() == WIZ_PG_CAMSRCH)
                    {
                        m_nextBtn->changeText(CNFG_DONE_BTN_STRING);
                        m_quitBtn->setVisible(false);
                    }
                    if(m_setupPage->getCurrPageIndex() == WIZ_PG_NTWCONF)
                    {
                        ((NetworkSetting*)m_setupPage)->setFlag(true);
                    }
                    m_setupPage->saveConfig();
                }
            }
        }
        break;

        case ELE_BACK_BTN:
        {
            if((m_setupPage != NULL) && ((m_setupPage->getCurrPageIndex()) > WIZ_PG_INIT))
            {
                nextPIndex = m_setupPage->getNextPrevPageIndex(PREV_PG);
                if ((isHddGrpCreate == false) && (nextPIndex == WIZ_PG_HDDGROUP))
                {
                    nextPIndex = WIZARD_PAGE_INDEXES_e(nextPIndex - 1);
                }

                m_nextBtn->changeText(CNFG_NEXT_BTN_STRING);
                m_quitBtn->setVisible(true);

                if(m_setupPage->getCurrPageIndex() == WIZ_PG_STATUS)
                {
                    updatePage(m_setupPage->getCurrPageIndex(), false, CONFIGURED_TEXT_COLOR);
                    DELETE_OBJ(m_setupPage);
                    openNextPrevPage(nextPIndex);
                }
                else
                {
                    if(m_setupPage->getCurrPageIndex() == WIZ_PG_NTWCONF)
                    {
                        ((NetworkSetting*)m_setupPage)->setFlag(true);
                    }
                    m_setupPage->saveConfig();
                }
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void SetupWizard::quitButton()
{
    WizardCommon::InfoPageImage();
    m_infoPage->raise();
    m_infoPage->loadInfoPage("Do you really want to quit the setup?", true, true, "", "Yes", "No");
}

void SetupWizard::slotInfoPageBtnclick(int index)
{
    if(index == 0)
    {
        emit sigQuitSetupWiz();
    }
    else
    {
        WizardCommon::UnloadInfoPageImage();
    }
}

void SetupWizard::openNextPrevPage(WIZARD_PAGE_INDEXES_e pageId)
{
    if (m_setupPage != NULL)
    {
        return;
    }

    switch (pageId)
    {
        case WIZ_PG_INIT:
            hideStepper();
            m_setupPage = new InitPage(m_currentDevName, this, pageId);
            break;

        case WIZ_PG_DTTIME:
            showStepper();
            m_setupPage = new DateAndTime(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            m_isLanguagePageCreated = true;
            connect(m_setupPage ,
                    SIGNAL(sigLanguageCfgChanged(QString)),
                    applController,
                    SLOT(slotLanguageCfgChanged(QString)));
            break;

        case WIZ_PG_NTWCONF:
            showStepper();
            m_setupPage = new NetworkSetting(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        case WIZ_PG_DHCPSERVER:
            showStepper();
            m_setupPage = new DhcpServer(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        case WIZ_PG_STRGCONF:
            showStepper();
            m_setupPage = new Storage(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        case WIZ_PG_HDDGROUP:
            showStepper();
            m_setupPage = new HDDGroup(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        case WIZ_PG_AUTOCONF:
            showStepper();
            m_setupPage = new ConfigureCamera(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        case WIZ_PG_CAMSRCH:
            showStepper();
            m_setupPage = new SearchCamera(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        case WIZ_PG_STATUS:
            showStepper();
            m_setupPage = new WizardStatus(m_currentDevName, quickSetupTabs[pageId], this, pageId);
            break;

        default:
            break;
    }

    if(m_setupPage != NULL)
    {
        m_visitPageStatus[pageId] = true;
        updatePage(pageId, true, BLUE_COLOR);
    }
}

void SetupWizard::slotTextLableHover(int pageId, bool status)
{
    QString color;

    if(m_visitPageStatus[pageId] == true)
    {
        if(status == true)
        {
            color = BLUE_COLOR;
        }
        else
        {
            status = false;
            color = CONFIGURED_TEXT_COLOR;
        }
    }
    else
    {
        status = false;
        color = INITIAL_TEXT_COLOR;
    }

    if((m_setupPage != NULL) && (m_setupPage->getCurrPageIndex() != pageId))
    {
        updatePage((WIZARD_PAGE_INDEXES_e)pageId, status, color);
    }
}

void SetupWizard::slotTextClicked(int pgId)
{
    if(m_visitPageStatus[pgId] == false)
    {
        return;
    }

    if((m_setupPage != NULL) && (pgId < MAX_WIZ_PG) && (m_setupPage->getCurrPageIndex() != pgId))
    {
        nextPIndex = (WIZARD_PAGE_INDEXES_e)pgId;
        if(pgId != WIZ_PG_STATUS)
        {
            m_nextBtn->changeText(CNFG_NEXT_BTN_STRING);
            m_quitBtn->setVisible(true);
        }

        if(pgId == WIZ_PG_STATUS)
        {
            m_nextBtn->changeText(CNFG_DONE_BTN_STRING);
            m_quitBtn->setVisible(false);
        }

        if(m_setupPage->getCurrPageIndex() == WIZ_PG_STATUS)
        {
            nextPIndex = (WIZARD_PAGE_INDEXES_e) pgId;
            updatePage(m_setupPage->getCurrPageIndex(), false, CONFIGURED_TEXT_COLOR);
            DELETE_OBJ(m_setupPage);
            openNextPrevPage(nextPIndex);
        }
        else
        {
            if(m_setupPage->getCurrPageIndex() == WIZ_PG_NTWCONF)
            {
                ((NetworkSetting*)m_setupPage)->setFlag(true);
            }
            m_setupPage->saveConfig();
        }
    }
}

void SetupWizard::updatePage(WIZARD_PAGE_INDEXES_e pageId, bool status, QString color)
{
    switch(pageId)
    {
        case WIZ_PG_INIT:
            break;

        case WIZ_PG_DTTIME:
            m_datenTime->setUnderline(status);
            m_datenTime->changeColor(color);
            m_datenTime->repaint();
            break;

        case WIZ_PG_NTWCONF:
            m_network->setUnderline(status);
            m_network->changeColor(color);
            m_network->repaint();
            break;

        case WIZ_PG_DHCPSERVER:
            m_dhcpServer->setUnderline(status);
            m_dhcpServer->changeColor(color);
            m_dhcpServer->repaint();
            break;

        case WIZ_PG_STRGCONF:
            m_storage->setUnderline(status);
            m_storage->changeColor(color);
            m_storage->repaint();
            break;

        case WIZ_PG_HDDGROUP:
            m_hddGroup->setUnderline(status);
            m_hddGroup->changeColor(color);
            m_hddGroup->repaint();
            break;

        case WIZ_PG_AUTOCONF:
            m_configureCamera->setUnderline(status);
            m_configureCamera->changeColor(color);
            m_configureCamera->repaint();
            break;

        case WIZ_PG_CAMSRCH:
            m_searchCamera->setUnderline(status);
            m_searchCamera->changeColor(color);
            m_searchCamera->repaint();
            break;

        case WIZ_PG_STATUS:
            m_status->setUnderline(status);
            m_status->changeColor(color);
            m_status->repaint();
            break;

        default:
            break;
    }
}

void SetupWizard:: hideStepper()
{
    m_backBtn->setVisible(false);
    m_heading_1->hide();
    m_datenTime->hide();
    m_network->hide();
    m_dhcpServer->hide();
    m_storage->hide();
    if (isHddGrpCreate == true)
    {
        m_hddGroup->hide();
    }

    m_configureCamera->hide();
    m_searchCamera->hide();
    m_status->hide();
}

void SetupWizard:: showStepper()
{
    m_backBtn->setVisible(true);
    m_heading_1->show();
    m_datenTime->show();
    m_network->show();
    m_dhcpServer->show();
    m_storage->show();
    if (isHddGrpCreate == true)
    {
        m_hddGroup->show();
    }

    m_configureCamera->show();
    m_searchCamera->show();
    m_status->show();
}
