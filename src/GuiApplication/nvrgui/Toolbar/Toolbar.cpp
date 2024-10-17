#include "Toolbar.h"
#include <QPainter>
#include <QShowEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include "ApplicationMode.h"
#include "Layout/Layout.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define DEFAULT_VOL_LEVEL              50

const QString monthsName[] = {"Jan", "Feb",
                              "Mar", "Apr",
                              "May", "Jun",
                              "Jul", "Aug",
                              "Sep", "Oct",
                              "Nov", "Dec"};

const QString toolTipString[MAX_TOOLBAR_BUTTON][MAX_STATE_TYPE] = {
    {"About Us",                 "About Us"},
    {"Login",                    "Logout"},
    {"Live View",                "Live View"},
    {"Display Mode",             "Display Mode"},
    {"Asynchronous Playback",    "Asynchronous Playback"},
    {"Synchronous Playback",     "Synchronous Playback"},
    {"Event Search",             "Event Search"},
    {"Settings",                 "Settings"},
    {"Manage",                   "Manage"},
    {"Device Status",            "Device Status"},
    {"Volume Control",           "Volume Control"},
    {"Auto Page Navigation",     "Auto Page Navigation"},
    {"Quick Backup",             "Quick Backup"},
    {"Video Pop-up : Off",       "Video Pop-up : On"},
    {"Setup Wizard",             "Setup Wizard"},
    {"Style Select",             "Style Select"},
    {"Collapse",                 "Collapse"},
    {"Buzzer",                   "Buzzer"},
    {"Live Event",               "Live Event"},
    {"Unplug USB",               "Unplug USB"},
    {"CPU Usage:",               "CPU Usage:"}
};

static const QString toolBarButtonString[] = {"Exit"};

static const QStringList styleNameList = QStringList() << "Current Style"
                                                       << "Style 1"
                                                       << "Style 2"
                                                       << "Style 3"
                                                       << "Style 4"
                                                       << "Style 5";

Toolbar::Toolbar(QWidget *parent) : KeyBoard(parent)
{
    m_currentElement = MENU_BUTTON;
    m_applController = ApplController::getInstance();
    m_payloadLib = new PayloadLib();
    m_camCount = 0;
    m_isCamNotify = false;
    m_isExpandMode = false;
    m_nextToolbarButtonLoad = MAX_TOOLBAR_BUTTON;

    m_updateTimer = new QTimer();
    connect(m_updateTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotUpdateDateTime()));

    m_cpuLoadState = CPU_LOAD_0;
    m_cpuLoad = 0;
    m_cpuLoadupdateTimer = new QTimer();
    m_cpuLoadupdateTimer->setInterval (10000);
    m_cpuLoadupdateTimer->setSingleShot (true);
    connect(m_cpuLoadupdateTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotUpdateCpuLoad()));

    QDateTime localDateTime = QDateTime::currentDateTime();
    updateDateTime(localDateTime);

    m_currentClickedButton = MAX_TOOLBAR_BUTTON;

    this->setGeometry(ApplController::getXPosOfScreen(),
                      (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - TOOLBAR_BUTTON_HEIGHT),
                      ApplController::getWidthOfScreen(),
                      TOOLBAR_BUTTON_HEIGHT);

    for(quint8 index = 0; index < CONTROL_ICON_BUTTONS; index++)
    {
        m_toolbarButtons[index] = new ToolbarButton((TOOLBAR_BUTTON_TYPE_e)index,
                                                    this,
                                                    (index + MENU_BUTTON));
        m_toolbarButtons[index]->setWhatsThis("ToolbarButton");
        m_elementList[index + MENU_BUTTON] = m_toolbarButtons[index];
        connect(m_toolbarButtons[index],
                SIGNAL(sigButtonClicked(TOOLBAR_BUTTON_TYPE_e)),
                this,
                SLOT(slotButtonClicked(TOOLBAR_BUTTON_TYPE_e)));
        connect(m_toolbarButtons[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_toolbarButtons[index],
                SIGNAL(sigShowHideToolTip(int,bool)),
                this,
                SLOT(slotShowHideTooltip(int,bool)));
    }
    connect(m_toolbarButtons[AUDIO_CONTROL_BUTTON],
            SIGNAL(sigChangeMuteUnmute()),
            this,
            SLOT(slotChangeMuteUnmute()));

    QMap<quint8, QString> styleListMap;
    for(quint8 index = 0; index < styleNameList.length(); index++)
    {
        styleListMap.insert(index, styleNameList.at(index));
    }

    m_styleSelectionPickList = new PickList(((CONTROL_ICON_BUTTONS * TOOLBAR_BUTTON_WIDTH) + SCALE_WIDTH(90)),
                                            ((TOOLBAR_BUTTON_HEIGHT - BGTILE_HEIGHT)/2),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            0,
                                            0,
                                            "",
                                            styleListMap,
                                            0,
                                            "Select Style",
                                            this,
                                            NO_LAYER,
                                            -1,
                                            STYLE_SELECT_PICKLIST);
    m_elementList[STYLE_SELECT_PICKLIST] = m_styleSelectionPickList;
    connect(m_styleSelectionPickList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_styleSelectionPickList,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotStyleChanged(quint8,QString,int)));
    connect (m_styleSelectionPickList,
             SIGNAL(sigPicklistLoad(quint8)),
             this,
             SLOT(slotPicklistLoad(quint8)));
    connect(m_styleSelectionPickList,
            SIGNAL(sigShowHideToolTip(int,bool)),
            this,
            SLOT(slotShowHideTooltip(int,bool)));

    m_previousPageButton = new ControlButton(PREVIOUS_BUTTON_1_INDEX,
                                             (m_styleSelectionPickList->x() + m_styleSelectionPickList->width() + SCALE_WIDTH(10)),
                                             SCALE_HEIGHT(11),
                                             SCALE_WIDTH(40),
                                             SCALE_HEIGHT(29),
                                             this,
                                             NO_LAYER,
                                             0,
                                             "",
                                             true,
                                             PAGE_PREVIOUS);
    m_elementList[PAGE_PREVIOUS] = m_previousPageButton;
    connect(m_previousPageButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_previousPageButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotChangePage(int)));

    m_pageNumber = new ReadOnlyElement(m_previousPageButton->x() + m_previousPageButton->width(),
                                       m_previousPageButton->y(),
                                       SCALE_WIDTH(45), SCALE_HEIGHT(30), SCALE_WIDTH(45), SCALE_HEIGHT(30), "Page" + QString(" "),
                                       this, NO_LAYER);

    m_nextPageButton = new ControlButton(NEXT_BUTTON_1_INDEX,
                                         m_pageNumber->x() + m_pageNumber->width(),
                                         m_pageNumber->y(),
                                         SCALE_WIDTH(40), SCALE_HEIGHT(29),
                                         this, NO_LAYER, 0, "", true,
                                         PAGE_NEXT);
    m_elementList[PAGE_NEXT] = m_nextPageButton;
    connect(m_nextPageButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_nextPageButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotChangePage(int)));

    m_dateText = new TextLabel((this->width() - 2),
                               (TOOLBAR_BUTTON_HEIGHT/2),
                               NORMAL_FONT_SIZE,
                               changeToStandardDateTime(),
                               this,
                               NORMAL_FONT_COLOR,
                               NORMAL_FONT_FAMILY,
                               ALIGN_END_X_CENTRE_Y);

    for(quint8 index = 0; index < STATUS_ICON_BUTTONS; index++)
    {
        m_statusIconButtons[index] = new ToolbarButton((TOOLBAR_BUTTON_TYPE_e)(index + CONTROL_ICON_BUTTONS),
                                                       this,
                                                       (index + STATUS_BUTTON),
                                                       (this->width() - m_dateText->width() - SCALE_WIDTH(250)),
                                                       SCALE_HEIGHT(4),
                                                       (((index + CONTROL_ICON_BUTTONS) == CPU_LOADS_BUTTON) ? true : false),
                                                       (((index + CONTROL_ICON_BUTTONS) == CPU_LOADS_BUTTON) ? false : true));
        m_statusIconButtons[index]->setWhatsThis("StatusIconButton");
        m_elementList[index + STATUS_BUTTON] = m_statusIconButtons[index];
        connect(m_statusIconButtons[index],
                SIGNAL(sigButtonClicked(TOOLBAR_BUTTON_TYPE_e)),
                this,
                SLOT(slotButtonClicked(TOOLBAR_BUTTON_TYPE_e)));
        connect(m_statusIconButtons[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_statusIconButtons[index],
                SIGNAL(sigShowHideToolTip(int,bool)),
                this,
                SLOT(slotShowHideTooltip(int,bool)));
    }

    m_exitButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  (this->width() - SCALE_WIDTH(400)),
                                  SCALE_HEIGHT(25),
                                  toolBarButtonString[0],
                                  this);
    connect(m_exitButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCfgButtonClicked(int)));
    connect(m_exitButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_exitButton->setVisible(false);
    m_exitButton->setIsEnabled(false);
    m_toolTip = NULL;
    m_toolTip = new ToolTip(this->x(),
                            (this->y() - SCALE_HEIGHT(5)),
                            "",
                            parentWidget(),
                            START_X_END_Y);

    m_camCountLabel = new TextLabel((m_statusIconButtons[0]->x () + m_statusIconButtons[0]->width () - SCALE_WIDTH(7)),
                                    (m_statusIconButtons[0]->y () - SCALE_HEIGHT(15)),
                                    SCALE_FONT(22),
                                    "",
                                    this,
                                    "#FBA601");

    this->setEnabled(true);
    this->setMouseTracking(true);
    this->installEventFilter(this);
    this->hide();
}

Toolbar::~Toolbar()
{
    disconnect(m_toolbarButtons[AUDIO_CONTROL_BUTTON],
            SIGNAL(sigChangeMuteUnmute()),
            this,
            SLOT(slotChangeMuteUnmute()));

    //deleting toolbarpage button
    for(quint8 index = 0; index < CONTROL_ICON_BUTTONS; index++)
    {
        disconnect(m_toolbarButtons[index],
                   SIGNAL(sigButtonClicked(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotButtonClicked(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(m_toolbarButtons[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_toolbarButtons[index],
                   SIGNAL(sigShowHideToolTip(int,bool)),
                   this,
                   SLOT(slotShowHideTooltip(int,bool)));
        delete m_toolbarButtons[index];
    }

    //deletint statusicon button
    for(quint8 index = 0; index < STATUS_ICON_BUTTONS; index++)
    {
        disconnect(m_statusIconButtons[index],
                   SIGNAL(sigButtonClicked(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotButtonClicked(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(m_statusIconButtons[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_statusIconButtons[index],
                   SIGNAL(sigShowHideToolTip(int,bool)),
                   this,
                   SLOT(slotShowHideTooltip(int,bool)));
        delete m_statusIconButtons[index];
    }

    //deleting styleselectpicklist
    disconnect (m_styleSelectionPickList,
                SIGNAL(sigPicklistLoad(quint8)),
                this,
                SLOT(slotPicklistLoad(quint8)));
    disconnect(m_styleSelectionPickList,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_styleSelectionPickList,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotStyleChanged(quint8,QString,int)));
    disconnect(m_styleSelectionPickList,
            SIGNAL(sigShowHideToolTip(int,bool)),
            this,
            SLOT(slotShowHideTooltip(int,bool)));
    delete m_styleSelectionPickList;

    //deleting previouspage button
    disconnect(m_previousPageButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_previousPageButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotChangePage(int)));
    delete m_previousPageButton;

    //deleting readonlypage number
    delete m_pageNumber;

    //deleting nextpage number
    disconnect(m_nextPageButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_nextPageButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotChangePage(int)));
    delete m_nextPageButton;

    disconnect(m_exitButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCfgButtonClicked(int)));
    disconnect(m_exitButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    delete m_exitButton;

    //deleting datetime number
    delete m_dateText;
    delete m_payloadLib;
    delete m_updateTimer;
    delete m_camCountLabel;

    m_cpuLoadupdateTimer->stop ();
    delete m_cpuLoadupdateTimer;
    DELETE_OBJ(m_toolTip);
}

QString Toolbar::changeToStandardDateTime()
{
    QString dateTime = (m_currentDate >= 10 ? QString("%1").arg(m_currentDate) : "0" + QString("%1").arg(m_currentDate))
            + "-"
            + monthsName[m_currentMonth - 1]
            + "-"
            + QString("%1").arg(m_currentYear)
            + "\r\n"
            + (m_currentHour >= 10 ? QString("%1").arg(m_currentHour) : "0" + QString("%1").arg(m_currentHour))
            + ":"
            + (m_currentMinute >= 10 ? QString("%1").arg(m_currentMinute) : "0" + QString("%1").arg(m_currentMinute))
            + ":"
            + (m_currentSecond >= 10 ? QString("%1").arg(m_currentSecond) : "0" + QString("%1").arg(m_currentSecond));

    return dateTime;
}

void Toolbar::updateDateTime(QString dateTimeString)
{
    m_currentDate = dateTimeString.mid(0, 2).toInt();
    m_currentMonth = dateTimeString.mid(2, 2).toInt();
    m_currentYear = dateTimeString.mid(4, 4).toInt();

    m_currentHour = dateTimeString.mid(8, 2).toInt() ;
    m_currentMinute = dateTimeString.mid(10, 2).toInt();
    m_currentSecond = dateTimeString.mid(12, 2).toInt();
}

void Toolbar::updateDateTime(QDateTime dateTime)
{
    m_currentDate = dateTime.date().day();
    m_currentMonth = dateTime.date().month();
    m_currentYear = dateTime.date().year();
    m_currentHour = dateTime.time().hour();
    m_currentMinute = dateTime.time().minute();
    m_currentSecond = dateTime.time().second();
}

void Toolbar::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != LOCAL_DEVICE_NAME)
    {
        return;
    }

    switch(param->cmdType)
    {
        case GET_DATE_TIME:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                m_payloadLib->parseDevCmdReply(true, param->payload);
                updateDateTime(m_payloadLib->getCnfgArrayAtIndex(0).toString());
            }

            m_dateText->changeText(changeToStandardDateTime());
            if(!m_cpuLoadupdateTimer->isActive())
            {
                getCpuLoadUpdate ();
            }

            if(!m_updateTimer->isActive())
            {
                m_updateTimer->start(999);
            }
        }
        break;

        case CPU_LOAD:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                m_payloadLib->parseDevCmdReply(true, param->payload);

                CPU_LOAD_STATE_e    cpuState;

                quint8 cpuLoad = m_payloadLib->getCnfgArrayAtIndex(0).toUInt ();

                if(cpuLoad == 0)
                {
                    cpuState = CPU_LOAD_0;
                }
                else if(cpuLoad < 20)
                {
                    cpuState = CPU_LOAD_20;
                }
                else if(cpuLoad < 40)
                {
                    cpuState = CPU_LOAD_40;
                }
                else if(cpuLoad < 60)
                {
                    cpuState = CPU_LOAD_60;
                }
                else if(cpuLoad < 80)
                {
                    cpuState = CPU_LOAD_80;
                }
                else
                {
                    cpuState = CPU_LOAD_100;
                }

                if(m_cpuLoadState != cpuState)
                {
                    m_cpuLoadState = cpuState;

                    if(this->isVisible())
                    {
                        m_statusIconButtons[CPU_LOADS_BUTTON % CONTROL_ICON_BUTTONS]->changeButtonImage ((IMAGE_TYPE_e)cpuState);
                    }
                }

                if(m_cpuLoad != cpuLoad)
                {
                    m_cpuLoad = cpuLoad;

                    if((this->isVisible()) && (m_currentElement == (MAX_TOOLBAR_ELEMENT - 1)))
                    {
                        slotShowHideTooltip (CPU_LOADS_BUTTON, true);
                    }
                }
            }

            if(!m_cpuLoadupdateTimer->isActive())
            {
                m_cpuLoadupdateTimer->start();
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

void Toolbar::getDateAndTime(QString deviceName)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;
    m_applController->processActivity(deviceName, DEVICE_COMM, param);
}

void Toolbar::openToolbarPage(TOOLBAR_BUTTON_TYPE_e index)
{
    bool isOpenPage = true;
    USRS_GROUP_e currentUserType = VIEWER;
    m_nextToolbarButtonLoad = MAX_TOOLBAR_BUTTON;

    m_applController->GetUserGroupType (LOCAL_DEVICE_NAME,currentUserType);

    if((index == SETTINGS_BUTTON) || (index == STYLE_SELECT_BUTTON) || (index == QUICK_BACKUP) ||
            (index == SYNC_PLAYBACK_BUTTON) || (index == WIZARD_BUTTON) || (index == ASYN_PLAYBACK_BUTTON))
    {
        if(currentUserType == VIEWER)
        {
            if(getCurrentToolBarButtonState (LOG_BUTTON) == STATE_1)
            {
                m_nextToolbarButtonLoad = index;
                index = LOG_BUTTON;
            }
            else if ((index != SYNC_PLAYBACK_BUTTON) && (index != QUICK_BACKUP) && (index != ASYN_PLAYBACK_BUTTON))
            {
                isOpenPage = false;
                MessageBanner::addMessageInBanner (ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
            }
        }
        else if(currentUserType == OPERATOR)
        {
            if(index == WIZARD_BUTTON)
            {
                isOpenPage = false;
                MessageBanner::addMessageInBanner (ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
            }
        }
    }

    if(isOpenPage)
    {
        ApplicationMode::setApplicationMode(PAGE_MODE);

        if(index <= WIZARD_BUTTON)
        {
            if(m_toolbarButtons[index]->m_giveClickEffect == true)
            {
                m_toolbarButtons[index]->changeButtonImage(IMAGE_TYPE_CLICKED);
            }
            else
            {
                m_toolbarButtons[index]->changeButtonImage(IMAGE_TYPE_NORMAL);
                this->setVisible(false);
            }
        }
        else
        {
            if(m_statusIconButtons[(index % CONTROL_ICON_BUTTONS)]->m_giveClickEffect == true)
            {
                m_statusIconButtons[(index % CONTROL_ICON_BUTTONS)]->changeButtonImage(IMAGE_TYPE_CLICKED);
            }
            else
            {
                m_statusIconButtons[(index % CONTROL_ICON_BUTTONS)]->changeButtonImage(IMAGE_TYPE_NORMAL);
                this->setVisible(false);
            }
        }

        m_toolTip->setVisible(false);

        if(index != COLLAPSE_BUTTON)
        {
            m_currentClickedButton = index;
        }

        emit sigOpenToolbarPage(index);
    }
}

void Toolbar::closeToolbarPage(TOOLBAR_BUTTON_TYPE_e index)
{
    if(index <= WIZARD_BUTTON)
    {
        if(m_toolbarButtons[index]->m_giveClickEffect == true)
        {
            ApplicationMode::setApplicationMode(TOOLBAR_MODE);
        }
        else
        {
            ApplicationMode::setApplicationMode(IDLE_MODE);
        }

        if(this->isVisible())
        {
            m_toolbarButtons[index]->changeButtonImage(IMAGE_TYPE_MOUSE_HOVER);
        }
    }
    else
    {
        if(m_statusIconButtons[index % CONTROL_ICON_BUTTONS]->m_giveClickEffect == true)
        {
            ApplicationMode::setApplicationMode(TOOLBAR_MODE);
        }
        else
        {
            ApplicationMode::setApplicationMode(IDLE_MODE);
        }

        if(this->isVisible())
        {
            m_statusIconButtons[index % CONTROL_ICON_BUTTONS]->changeButtonImage(IMAGE_TYPE_MOUSE_HOVER);
        }
    }

    if(ApplicationMode::getApplicationMode() == TOOLBAR_MODE)
    {
        if(m_currentClickedButton != MAX_TOOLBAR_BUTTON)
        {
            m_currentElement = ((m_currentClickedButton <= WIZARD_BUTTON)
                                ? m_currentClickedButton : ((m_currentClickedButton % CONTROL_ICON_BUTTONS) + STATUS_BUTTON));
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
    else
    {
        this->setVisible(false);
    }

    USRS_GROUP_e currentUserType = VIEWER;
    m_applController->GetUserGroupType (LOCAL_DEVICE_NAME,currentUserType);

    if((m_nextToolbarButtonLoad == MAX_TOOLBAR_BUTTON) && (m_currentClickedButton == LOG_BUTTON)
            && (getCurrentToolBarButtonState (LOG_BUTTON) == STATE_2) && (currentUserType == ADMIN) && (index == LOG_BUTTON))
    {        
        emit sigOpenWizard();
    }

    if(m_currentClickedButton == index)
    {
        m_currentClickedButton = MAX_TOOLBAR_BUTTON;
    }

    emit sigCloseToolbarPage(index);    
    if(((m_nextToolbarButtonLoad != MAX_TOOLBAR_BUTTON) &&
            ((m_nextToolbarButtonLoad != LOG_BUTTON) && (m_nextToolbarButtonLoad != QUICK_BACKUP) && (m_nextToolbarButtonLoad != WIZARD_BUTTON)
             && (m_nextToolbarButtonLoad != ASYN_PLAYBACK_BUTTON) && (m_nextToolbarButtonLoad != SYNC_PLAYBACK_BUTTON))))
    {
        if (getCurrentToolBarButtonState(LOG_BUTTON) == STATE_2)
        {
            openToolbarPage (m_nextToolbarButtonLoad);
        }
    }
}

/* Added to apply multi-language after login when get language command's response received. If we don't do this
 * then dynamically generated multi-language text will be displayed in english only after applying multi-language */
void Toolbar::openNextToolbarPage(void)
{
    if ((m_nextToolbarButtonLoad == QUICK_BACKUP) || (m_nextToolbarButtonLoad == WIZARD_BUTTON)
            || (m_nextToolbarButtonLoad == ASYN_PLAYBACK_BUTTON) || (m_nextToolbarButtonLoad == SYNC_PLAYBACK_BUTTON))
    {
        if (getCurrentToolBarButtonState(LOG_BUTTON) == STATE_2)
        {
            openToolbarPage (m_nextToolbarButtonLoad);
        }
    }
}

STATE_TYPE_e Toolbar::getCurrentToolBarButtonState(TOOLBAR_BUTTON_TYPE_e toolBarButtonIndex)
{
    STATE_TYPE_e state = STATE_1;

    if(toolBarButtonIndex < CONTROL_ICON_BUTTONS)
    {
        if(IS_VALID_OBJ(m_toolbarButtons[toolBarButtonIndex]))
        {
            state = (m_toolbarButtons[toolBarButtonIndex]->getCurrentButtonState());
        }
    }
    else
    {
        if(IS_VALID_OBJ(m_statusIconButtons[toolBarButtonIndex - CONTROL_ICON_BUTTONS]))
        {
            state = (m_statusIconButtons[toolBarButtonIndex - CONTROL_ICON_BUTTONS]->getCurrentButtonState());
        }
    }

    return state;
}

TOOLBAR_BUTTON_TYPE_e Toolbar::getCurrentToolBarButton()
{
    return m_currentClickedButton;
}

TOOLBAR_BUTTON_TYPE_e Toolbar::getNextToolBarButton()
{
    return m_nextToolbarButtonLoad;
}

void Toolbar::loadToolbarPage(TOOLBAR_BUTTON_TYPE_e index)
{
    if((index == AUDIO_CONTROL_BUTTON) || (index == DISPLAY_MODE_BUTTON))
    {
        if(this->isHidden ())
        {
            this->show ();
            getDateAndTime(LOCAL_DEVICE_NAME);
        }
    }
    m_toolbarButtons[index]->takeEnterKeyAction();
}

void Toolbar::unloadToolbarPage()
{
    if(m_currentClickedButton != MAX_TOOLBAR_BUTTON)
    {
        if(m_currentClickedButton == CONTROL_ICON_BUTTONS)
        {
            m_statusIconButtons[0]->takeEnterKeyAction();
        }
        else
        {
            m_toolbarButtons[m_currentClickedButton]->takeEnterKeyAction();
        }
    }
    this->setVisible(false);
}

void Toolbar::deletePicklist ()
{
    m_styleSelectionPickList->deletePicklistLoader();
}

void Toolbar::getCpuLoadUpdate()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = CPU_LOAD;

    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void Toolbar::changePageOnLayoutChange()
{
    m_pageNumber->changeValue( QString("%1").arg(Layout::currentDisplayConfig[MAIN_DISPLAY].currPage + 1));
}

void Toolbar::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_TOOLBAR_ELEMENT) % MAX_TOOLBAR_ELEMENT;
    }
    while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void Toolbar::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_TOOLBAR_ELEMENT;
    }
    while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

bool Toolbar::eventFilter(QObject * obj, QEvent * event)
{
    bool hideToolbar = false;

    if(event->type() == QEvent::Leave)
    {
        if(m_currentClickedButton != MAX_TOOLBAR_BUTTON)
        {
            if((m_currentClickedButton <= WIZARD_BUTTON)
                    && (!m_toolbarButtons[m_currentClickedButton]->m_giveClickEffect))
            {
                hideToolbar = true;
            }
            else if((m_currentClickedButton >= CONTROL_ICON_BUTTONS)
                    && (!m_statusIconButtons[(m_currentClickedButton % CONTROL_ICON_BUTTONS)]->m_giveClickEffect))
            {
                hideToolbar = true;
            }
        }
        else
        {
            hideToolbar = true;
        }

        if(hideToolbar == true)
        {
            m_updateTimer->stop();
            m_cpuLoadupdateTimer->stop ();
            this->setVisible(false);
        }
    }
    return QObject::eventFilter(obj,event);
}

void Toolbar::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(NORMAL_BKG_COLOR),Qt::SolidPattern));
    painter.drawRect(QRect(0, 0, this->width(), this->height()));
}

void Toolbar::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);

    if( (QApplication::overrideCursor() != NULL)
                && (QApplication::overrideCursor()->shape() == Qt::OpenHandCursor) )
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }

    ApplicationMode::setApplicationMode(TOOLBAR_MODE);

    if(m_currentClickedButton == MAX_TOOLBAR_BUTTON)
    {
        m_currentElement = MENU_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    else
    {
        if(m_currentClickedButton != STYLE_SELECT_BUTTON)
        {
            m_toolbarButtons[m_currentClickedButton]->changeButtonImage (IMAGE_TYPE_CLICKED);
        }
    }

    if((m_isCamNotify == true) && (m_isExpandMode == false))
    {
        m_camCountLabel->setVisible (true);
        m_statusIconButtons[0]->setVisible (true);
        m_camCountLabel->changeText (QString("%1").arg(m_camCount));
        m_camCountLabel->update ();
    }
    else
    {
        m_camCountLabel->setVisible (false);
        m_statusIconButtons[0]->setVisible (false);
    }
}

void Toolbar::hideEvent(QHideEvent *)
{
    m_toolTip->showHideTooltip(false);
    if(ApplicationMode::getApplicationMode() == TOOLBAR_MODE)
    {
        ApplicationMode::setApplicationMode(IDLE_MODE);
    }
}

void Toolbar::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
            event->accept();
            takeLeftKeyAction();
            break;

        case Qt::Key_Right:
            event->accept();
            takeRightKeyAction();
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void Toolbar::changeNextToolBarBtnLoad(TOOLBAR_BUTTON_TYPE_e index)
{
    m_nextToolbarButtonLoad = index;
}

void Toolbar::changeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state)
{
    m_toolbarButtons[buttonIndex]->changeButtonState(state);
}

void Toolbar::updateGeometry()
{
    if(m_updateTimer->isActive())
    {
        m_updateTimer->stop();
    }

    if(m_cpuLoadupdateTimer->isActive())
    {
        m_cpuLoadupdateTimer->stop();
    }

    this->setGeometry(ApplController::getXPosOfScreen(),
                      (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - TOOLBAR_BUTTON_HEIGHT),
                      ApplController::getWidthOfScreen(),
                      TOOLBAR_BUTTON_HEIGHT);
    for(quint8 index = 0; index < CONTROL_ICON_BUTTONS; index++)
    {
         m_toolbarButtons[index]->updateGeometry();
    }

    bool isStyleSelectionPickListVisible = m_styleSelectionPickList->isEnabled();
    quint8 keyValue = m_styleSelectionPickList->getCurrentValue();

    disconnect(m_styleSelectionPickList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_styleSelectionPickList,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotStyleChanged(quint8,QString,int)));
    disconnect (m_styleSelectionPickList,
             SIGNAL(sigPicklistLoad(quint8)),
             this,
             SLOT(slotPicklistLoad(quint8)));
    DELETE_OBJ(m_styleSelectionPickList)

    QMap<quint8, QString> styleListMap;
    for(quint8 index = 0; index < styleNameList.length(); index++)
    {
        styleListMap.insert(index, styleNameList.at(index));
    }

    m_styleSelectionPickList = new PickList(((CONTROL_ICON_BUTTONS * TOOLBAR_BUTTON_WIDTH) + SCALE_WIDTH(90)),
                                            ((TOOLBAR_BUTTON_HEIGHT - BGTILE_HEIGHT)/2),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            0,
                                            0,
                                            "",
                                            styleListMap,
                                            0,
                                            "Select Style",
                                            this,
                                            NO_LAYER,
                                            -1,
                                            STYLE_SELECT_PICKLIST);
    m_elementList[STYLE_SELECT_PICKLIST] = m_styleSelectionPickList;
    connect(m_styleSelectionPickList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_styleSelectionPickList,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotStyleChanged(quint8,QString,int)));
    connect (m_styleSelectionPickList,
             SIGNAL(sigPicklistLoad(quint8)),
             this,
             SLOT(slotPicklistLoad(quint8)));
    connect(m_styleSelectionPickList,
            SIGNAL(sigShowHideToolTip(int,bool)),
            this,
            SLOT(slotShowHideTooltip(int,bool)));

    m_styleSelectionPickList->changeValue((int)keyValue);
    m_styleSelectionPickList->setIsEnabled(isStyleSelectionPickListVisible);
    m_styleSelectionPickList->setVisible(isStyleSelectionPickListVisible);

    bool isPreviousPageButtonVisible = m_previousPageButton->isEnabled();
    disconnect(m_previousPageButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_previousPageButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotChangePage(int)));
    DELETE_OBJ(m_previousPageButton);

    m_previousPageButton = new ControlButton(PREVIOUS_BUTTON_1_INDEX,
                                             (m_styleSelectionPickList->x() + m_styleSelectionPickList->width() + SCALE_WIDTH(10)),
                                             SCALE_HEIGHT(11),
                                             SCALE_WIDTH(40),
                                             SCALE_HEIGHT(29),
                                             this,
                                             NO_LAYER,
                                             0,
                                             "",
                                             true,
                                             PAGE_PREVIOUS);
    m_elementList[PAGE_PREVIOUS] = m_previousPageButton;
    connect(m_previousPageButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_previousPageButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotChangePage(int)));
    m_previousPageButton->setVisible(isPreviousPageButtonVisible);
    m_previousPageButton->setIsEnabled(isPreviousPageButtonVisible);

    QString pageNo = m_pageNumber->getCurrentValue();
    DELETE_OBJ(m_pageNumber);

    m_pageNumber = new ReadOnlyElement(m_previousPageButton->x() + m_previousPageButton->width(),
                                       m_previousPageButton->y(),
                                       SCALE_WIDTH(45), SCALE_HEIGHT(30), SCALE_WIDTH(45), SCALE_HEIGHT(30), "Page" + QString(" "),
                                       this, NO_LAYER);
    m_pageNumber->changeValue(pageNo);
    m_pageNumber->setVisible(isPreviousPageButtonVisible);

    bool isNextPageButtonVisible = m_nextPageButton->isEnabled();
    disconnect(m_nextPageButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_nextPageButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotChangePage(int)));
    DELETE_OBJ(m_nextPageButton);

    m_nextPageButton = new ControlButton(NEXT_BUTTON_1_INDEX,
                                         m_pageNumber->x() + m_pageNumber->width(),
                                         m_pageNumber->y(),
                                         SCALE_WIDTH(40), SCALE_HEIGHT(29),
                                         this, NO_LAYER, 0, "", true,
                                         PAGE_NEXT);
    m_elementList[PAGE_NEXT] = m_nextPageButton;
    connect(m_nextPageButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_nextPageButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotChangePage(int)));
    m_nextPageButton->setIsEnabled(isNextPageButtonVisible);
    m_nextPageButton->setVisible(isNextPageButtonVisible);

    DELETE_OBJ(m_dateText);
    m_dateText = new TextLabel((this->width() - 2),
                               (TOOLBAR_BUTTON_HEIGHT/2),
                               NORMAL_FONT_SIZE,
                               changeToStandardDateTime(),
                               this,
                               NORMAL_FONT_COLOR,
                               NORMAL_FONT_FAMILY,
                               ALIGN_END_X_CENTRE_Y);

    for(quint8 index = 0; index < STATUS_ICON_BUTTONS; index++)
    {
        m_statusIconButtons[index]->setOffset((this->width() - m_dateText->width() - SCALE_WIDTH(250)), SCALE_HEIGHT(4));
        m_statusIconButtons[index]->updateGeometry();
    }

    bool isExitVisible = m_exitButton->isEnabled();
    disconnect(m_exitButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCfgButtonClicked(int)));
    disconnect(m_exitButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_exitButton);

    m_exitButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  (this->width() - SCALE_WIDTH(400)),
                                  SCALE_HEIGHT(25),
                                  toolBarButtonString[0],
                                  this);
    connect(m_exitButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCfgButtonClicked(int)));
    connect(m_exitButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_exitButton->setVisible(isExitVisible);
    m_exitButton->setIsEnabled(isExitVisible);

    DELETE_OBJ(m_toolTip);
    m_toolTip = new ToolTip(this->x(),
                            (this->y() - SCALE_HEIGHT(5)),
                            "",
                            parentWidget(),
                            START_X_END_Y);

    QString text = m_camCountLabel->getText();
    DELETE_OBJ(m_camCountLabel);
    m_camCountLabel = new TextLabel((m_statusIconButtons[0]->x () + m_statusIconButtons[0]->width () - SCALE_WIDTH(7)),
                                    (m_statusIconButtons[0]->y () - SCALE_HEIGHT(15)),
                                    SCALE_FONT(22),
                                    text,
                                    this,
                                    "#FBA601");
    if(!m_updateTimer->isActive())
    {
        m_updateTimer->start(999);
    }

    if(!m_cpuLoadupdateTimer->isActive())
    {
        m_cpuLoadupdateTimer->start();
    }
}

void Toolbar::slotClosePage(TOOLBAR_BUTTON_TYPE_e index)
{
    closeToolbarPage(index);
}

void Toolbar::slotButtonClicked(TOOLBAR_BUTTON_TYPE_e index)
{
    if(index == CPU_LOADS_BUTTON)
    {
        return;
    }

    bool isToolbarWithPageMode = ((index == AUDIO_CONTROL_BUTTON)
                                  || (index == DISPLAY_MODE_BUTTON)
                                  || (index == USB_CONTROL_BUTTON));

    bool isToolbarForExpandWindowMode = (((index == DISPLAY_MODE_BUTTON)
                                          || (index == LIVE_VIEW_BUTTON)
                                          || (index == ASYN_PLAYBACK_BUTTON)
                                          || (index == SYNC_PLAYBACK_BUTTON)
                                          || (index == EVENT_LOG_BUTTON)
                                          || (index == LOG_BUTTON)
                                          || (index == SEQUENCE_BUTTON)
                                          || (index == QUICK_BACKUP)
                                          || (index == VIDEO_POPUP_BUTTON)
                                          || (index == WIZARD_BUTTON)
                                          || (index == SETTINGS_BUTTON)
                                          || (index == STYLE_SELECT_BUTTON))
                                         && (Layout::currentModeType[MAIN_DISPLAY] == STATE_EXPAND_WINDOW));

    bool isToolbarForVideoPopupMode = (((index == DISPLAY_MODE_BUTTON)
                                        || (index == LIVE_VIEW_BUTTON)
                                        || (index == ASYN_PLAYBACK_BUTTON)
                                        || (index == SYNC_PLAYBACK_BUTTON)
                                        || (index == EVENT_LOG_BUTTON)
                                        || (index == LOG_BUTTON)
                                        || (index == SEQUENCE_BUTTON)
                                        || (index == QUICK_BACKUP)
                                        || (index == SETTINGS_BUTTON)
                                        || (index == STYLE_SELECT_BUTTON))
                                       && (Layout::currentModeType[MAIN_DISPLAY] == STATE_VIDEO_POPUP_FEATURE));

    bool isToolbarForLocalDecording = (((index == DISPLAY_MODE_BUTTON)
                                        || (index == LIVE_VIEW_BUTTON)
                                        || (index == SEQUENCE_BUTTON)
                                        || (index == STYLE_SELECT_BUTTON))
                                       && (Layout::currentModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING));

    if((Layout::cosecPopupEvtData.m_isCosecHandlingStarted == false)
            && (!(isToolbarWithPageMode && (this->isVisible() == false)))
            && (isToolbarForExpandWindowMode == false) && (isToolbarForVideoPopupMode == false)
            && (isToolbarForLocalDecording == false))
    {
        if(m_currentClickedButton == MAX_TOOLBAR_BUTTON)
        {
            openToolbarPage(index);
        }
        else
        {
            if(m_currentClickedButton == index)
            {
                closeToolbarPage(m_currentClickedButton);
                m_currentClickedButton = MAX_TOOLBAR_BUTTON;
            }
            else
            {
                if(m_currentClickedButton != CONTROL_ICON_BUTTONS)
                {
                    m_toolbarButtons[m_currentClickedButton]->changeButtonImage(IMAGE_TYPE_NORMAL);
                }
                closeToolbarPage(m_currentClickedButton);
                openToolbarPage(index);
            }
        }
    }
    else if(isToolbarForLocalDecording == true)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED_CLICK_ACTION));

    }
    else if(isToolbarForExpandWindowMode == true)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TOOLBAR_COLLAPSE_WINDOW_MESSAGE));
    }
    else if(isToolbarForVideoPopupMode == true)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TOOLBAR_VIDEO_POPUP_WINDOW_MESSAGE));
    }
}

void Toolbar::slotUpdateDateTime()
{
    QDateTime newDateTime = QDateTime(QDate(m_currentYear, m_currentMonth, m_currentDate),
                                      QTime(m_currentHour, m_currentMinute, m_currentSecond),
                                      Qt::UTC);
    updateDateTime(newDateTime.addSecs(1));
    m_dateText->changeText(changeToStandardDateTime());
    m_dateText->update();
}

void Toolbar::slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index, bool state)
{
    m_statusIconButtons[(index % CONTROL_ICON_BUTTONS)]->setIsEnabled(state);
}

void Toolbar::slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e index,
                                           STATE_TYPE_e state)
{
    m_toolbarButtons[index]->changeButtonState(state);
}

void Toolbar::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void Toolbar::slotChangePageFromLayout()
{
    changePageOnLayoutChange();
}

void Toolbar::slotChangePage(int index)
{
    if(m_currentClickedButton == DISPLAY_MODE_BUTTON)
    {
        closeToolbarPage(DISPLAY_MODE_BUTTON);
    }

    if(m_currentClickedButton == AUDIO_CONTROL_BUTTON)
    {
        closeToolbarPage(AUDIO_CONTROL_BUTTON);
    }

    emit sigChangePage((NAVIGATION_TYPE_e)(index - PAGE_PREVIOUS));

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void Toolbar::slotShowHideTooltip(int index, bool toShowTooltip)
{
    if(toShowTooltip)
    {
        if(m_toolTip->isVisible() == false)
        {
            m_toolTip->setVisible (true);
        }

        m_toolTip->showHideTooltip(true);
        if(index != CPU_LOADS_BUTTON)
        {
            m_toolTip->textChange(toolTipString[index][getCurrentToolBarButtonState((TOOLBAR_BUTTON_TYPE_e)index)]);
        }
        else
        {
            QString str = toolTipString[index][getCurrentToolBarButtonState((TOOLBAR_BUTTON_TYPE_e)index)] + " " + QString("%1").arg (m_cpuLoad) +  "%";
            m_toolTip->textChange(str);
        }

        if(index <= WIZARD_BUTTON)
        {
            m_toolTip->resetGeometry((ApplController::getXPosOfScreen() + m_toolbarButtons[index]->x()), this->y() - SCALE_HEIGHT(5));
        }
        else if (index== STYLE_SELECT_BUTTON)
        {
            if(this->isVisible())
            {
                m_toolTip->resetGeometry((ApplController::getXPosOfScreen() + m_styleSelectionPickList->x()), this->y() - SCALE_HEIGHT(5));
            }
            else
            {
                m_toolTip->showHideTooltip(false);
            }
        }
        else
        {
            m_toolTip->resetGeometry((ApplController::getXPosOfScreen() + m_statusIconButtons[(index % CONTROL_ICON_BUTTONS)]->x()), this->y() - SCALE_HEIGHT(5));
        }
    }
    else
    {
         m_toolTip->showHideTooltip(false);
    }
}

void Toolbar::slotStyleChanged(quint8 index, QString, int)
{
    QList<QVariant> paramList;
    DISPLAY_CONFIG_t displayConfig;
    STYLE_TYPE_e styleNo;

    styleNo = ((index == 0) ? MAX_STYLE_TYPE : (STYLE_TYPE_e)(index - 1));

    if(styleNo != MAX_STYLE_TYPE)
    {
        paramList.append(READ_DISP_ACTIVITY);
        paramList.append(MAIN_DISPLAY);
        paramList.append(styleNo);
        m_applController->processActivity(DISPLAY_SETTING, paramList, &displayConfig);
        paramList.clear();

        for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
        {
            for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
            {
                if((strcmp(displayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName, LOCAL_DEVICE_NAME) == 0)
                        && (displayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel > deviceRespInfo.maxCameras))
                {
                    displayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                    displayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                }
            }

            if(Layout::isMultipleChannelAssigned(MAIN_DISPLAY, windowIndex) == false)
            {
                displayConfig.windowInfo[windowIndex].sequenceStatus = false;
            }
        }

        emit sigToolbarApplyStyle(MAIN_DISPLAY, displayConfig, styleNo);
    }
    else
    {
        memcpy((void*)&displayConfig,
               (void*)&Layout::currentDisplayConfig[MAIN_DISPLAY],
               sizeof(DISPLAY_CONFIG_t));

        emit sigToolbarApplyStyle(MAIN_DISPLAY, displayConfig, styleNo);
    }
}

void Toolbar::slotStyleChnagedNotify(STYLE_TYPE_e styleNo)
{
    m_styleSelectionPickList->changeValue((styleNo != MAX_STYLE_TYPE) ? (styleNo + 1) : 0);
}

void Toolbar::slotPicklistLoad(quint8)
{
    if(Layout::currentModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED_CLICK_ACTION));
        m_styleSelectionPickList->deletePicklistLoader ();
    }

    if(m_currentClickedButton == DISPLAY_MODE_BUTTON)
    {
        closeToolbarPage(DISPLAY_MODE_BUTTON);
    }

    if(m_currentClickedButton == AUDIO_CONTROL_BUTTON)
    {
        closeToolbarPage(AUDIO_CONTROL_BUTTON);
    }
}

void Toolbar::slotShowHideStyleAndPageControl(bool toShow,quint8 type)
{
    if(type == STATE_VIDEO_POPUP_FEATURE)
    {
        m_isExpandMode = false;

        m_styleSelectionPickList->setVisible(toShow);
        m_styleSelectionPickList->setIsEnabled(toShow);

        m_exitButton->setVisible(!toShow);
        m_exitButton->setIsEnabled(!toShow);

        m_statusIconButtons[1]->setIsEnabled (false);

        m_nextPageButton->setVisible(toShow);
        m_nextPageButton->setIsEnabled(toShow);

        m_previousPageButton->setVisible(toShow);
        m_previousPageButton->setIsEnabled(toShow);

        m_pageNumber->setVisible(toShow);

        if(m_isCamNotify == true)
        {
            m_statusIconButtons[0]->setIsEnabled (toShow);
            m_statusIconButtons[0]->setVisible(toShow);
        }
        else
        {
            m_statusIconButtons[0]->setIsEnabled (false);
            m_statusIconButtons[0]->setVisible(false);
        }
    }
    else
    {
        m_isExpandMode = (!toShow);

        m_styleSelectionPickList->setVisible(toShow);
        m_styleSelectionPickList->setIsEnabled(toShow);

        m_nextPageButton->setVisible(toShow);
        m_nextPageButton->setIsEnabled(toShow);

        m_previousPageButton->setVisible(toShow);
        m_previousPageButton->setIsEnabled(toShow);

        m_pageNumber->setVisible(toShow);

        m_statusIconButtons[1]->setIsEnabled (!toShow);

        m_exitButton->setVisible(false);
        m_exitButton->setIsEnabled(false);

        if(m_isCamNotify == true)
        {
            m_statusIconButtons[0]->setIsEnabled (toShow);
            m_statusIconButtons[0]->setVisible(toShow);
        }
        else
        {
            m_statusIconButtons[0]->setIsEnabled (false);
            m_statusIconButtons[0]->setVisible(false);
        }
    }
}

void Toolbar::slotUpdateCpuLoad()
{
    getCpuLoadUpdate ();
}

void Toolbar::slotCfgButtonClicked(int index)
{
    if(Layout::currentModeType[MAIN_DISPLAY] == STATE_VIDEO_POPUP_FEATURE)
    {
        emit sigExitVideoPopupState();
    }
    Q_UNUSED(index);
}

void Toolbar::slotChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e index,bool backupState)
{
    m_toolbarButtons[index]->changeButtonState((backupState == true) ? STATE_2 : STATE_1);
}

void Toolbar:: slotChangeMuteUnmute()
{
    if(m_currentClickedButton == AUDIO_CONTROL_BUTTON)
    {
        closeToolbarPage(AUDIO_CONTROL_BUTTON);
    }
    m_applController->readAudioConfig();

    if(m_applController->m_currentMuteState == ON_STATE)
    {
        if(m_applController->m_audioLevel == 0)
        {
            m_applController->m_audioLevel = DEFAULT_VOL_LEVEL;
        }
        m_applController->writeAudioConfig(FALSE,m_applController->m_audioLevel);
        slotChangeToolbarButtonState(AUDIO_CONTROL_BUTTON, STATE_1); //Audio Off
    }
    else
    {
        m_applController->writeAudioConfig(TRUE,m_applController->m_audioLevel);
        slotChangeToolbarButtonState(AUDIO_CONTROL_BUTTON, STATE_2); //Audio On
    }
}
