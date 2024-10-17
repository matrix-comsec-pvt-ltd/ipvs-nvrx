#include "ConfigPageControl.h"
#include <QKeyEvent>

static const quint8 maxNoOfBtn[4] = {3, 2 ,1, 0};
static const QString cnfgBtnPairStr[3] =
{
    "Default",
    "Refresh",
    "Save"
};

static const int cnfgBtnPairYcentre[3][3] =
{
    {
        ((PAGE_RIGHT_PANEL_WIDTH / 2) - 145),
        (PAGE_RIGHT_PANEL_WIDTH / 2),
        ((PAGE_RIGHT_PANEL_WIDTH / 2) + 145)
    },
    {
        ((PAGE_RIGHT_PANEL_WIDTH / 2) - 70),
        ((PAGE_RIGHT_PANEL_WIDTH / 2) + 70),
    },
    {
        (PAGE_RIGHT_PANEL_WIDTH / 2)
    }
};

ConfigPageControl::ConfigPageControl(QString devName,
                                     QWidget *parent,
                                     int maxControlInPage,
                                     DEV_TABLE_INFO_t *tableInfo,
                                     CNFG_BTN_PAIR_TYPE_e pairType) :
    KeyBoard(parent), NavigationControl(0, true)
{
    devTableInfo = tableInfo;
    cnfgPairType = pairType;
    m_maxControlInPage = maxControlInPage + maxNoOfBtn[cnfgPairType];
    for(quint16 index = 0; index < m_maxControlInPage; index++)
    {
        m_elementList[index] = NULL;
    }
    currDevName = devName;   

    this->setGeometry (SCALE_WIDTH(PAGE_RIGHT_PANEL_STARTX),
                       SCALE_HEIGHT(PAGE_RIGHT_PANEL_STARTY),
                       SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH),
                       SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) );
    payloadLib = new PayloadLib();

    applController = ApplController::getInstance ();
    processBar = new ProcessBar(this->x(),
                                this->y(),
                                this->width(),
                                this->height(),
                                0,
                                parentWidget());
    processBar->unloadProcessBar ();

    infoPage = new InfoPage (0, 0,
                             (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH)),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget());
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    for(quint8 index = 0; index < maxNoOfBtn[cnfgPairType]; index++)
    {
        cnfgBtnPair[index] = new CnfgButton(CNFGBUTTON_MEDIAM,
                                            SCALE_WIDTH(cnfgBtnPairYcentre[cnfgPairType][index]),
                                            (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - SCALE_HEIGHT(75)),
                                            cnfgBtnPairStr[cnfgPairType + index],
                                            this,
                                            maxControlInPage);

        m_elementList[maxControlInPage++] = cnfgBtnPair[index];

        connect (cnfgBtnPair[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotCnfgBtnClicked(int)));
        connect (cnfgBtnPair[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }
    m_currentElement = 0;
    this->show();
}

ConfigPageControl::~ConfigPageControl()
{
    payloadLib->clearPayloadLib();
    delete payloadLib;

    for(quint8 index = 0; index < maxNoOfBtn[cnfgPairType]; index++)
    {
        disconnect (cnfgBtnPair[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        disconnect (cnfgBtnPair[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotCnfgBtnClicked(int)));

        delete cnfgBtnPair[index];
    }
    delete processBar;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
}

void ConfigPageControl::resetGeometryOfCnfgbuttonRow(quint8 yOffset)
{
    for(quint8 index = 0; index < maxNoOfBtn[cnfgPairType]; index++)
    {
        cnfgBtnPair[index]->resetGeometry(SCALE_WIDTH(cnfgBtnPairYcentre[cnfgPairType][index]),
                                          (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - SCALE_HEIGHT(75)) + yOffset);
    }
}

void ConfigPageControl::showHideCnfgBtn(CNFG_BTN_PAIR_e btnType, bool isVisible)
{
    cnfgBtnPair[btnType]->setVisible(isVisible);
}

void ConfigPageControl::getConfig()
{

}

void ConfigPageControl::defaultConfig()
{

}

void ConfigPageControl::saveConfig()
{

}

void ConfigPageControl::handleInfoPageMessage(int)
{

}

void ConfigPageControl::loadProcessBar ()
{

}

bool ConfigPageControl::isUserChangeConfig()
{
    return false;
}

void ConfigPageControl::loadCamsearchOnAutoConfig()
{

}

void ConfigPageControl::updateList(DevCommParam *param)
{
    Q_UNUSED(param);
}

bool ConfigPageControl::takeLeftKeyAction()
{
    bool status = true;

    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (m_maxControlInPage);
        }

        if(m_currentElement)
        {
            m_currentElement = (m_currentElement - 1);
            if((m_currentElement == 0)
                    && (m_elementList[m_currentElement] != NULL))
            {
                if(!m_elementList[m_currentElement]->getIsEnabled())
                {
                    // pass key to parent
//                    status = false;
                    m_currentElement = indexOfLstEnable();
                }
                break;
            }
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(m_elementList[m_currentElement] != NULL)
    {
        if(m_elementList[m_currentElement]->getIsEnabled())
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
    return status;
}

int ConfigPageControl::indexOfLstEnable()
{
    int index=0;

    for(index= (m_maxControlInPage - 1);index >= 0 ; index--)
    {
        if(m_elementList[index]->getIsEnabled())
        {
            break;
        }
    }

    if(index >= 0)
    {
        return index;
    }
    else
    {
        return 0;
    }
}


bool ConfigPageControl::takeRightKeyAction()
{
    bool status = true;

    if (m_maxControlInPage == 0)
    {
        return false;
    }

    do
    {   
        if (m_currentElement == (m_maxControlInPage - 1))
        {
            m_currentElement = -1;
        }
        if (m_currentElement != (m_maxControlInPage - 1))
        {
            m_currentElement = (m_currentElement + 1);
            if ((m_currentElement == (m_maxControlInPage - 1))
                    && (m_elementList[m_currentElement] != NULL))
            {
                if (!m_elementList[m_currentElement]->getIsEnabled())
                {
                    // pass key to parent
                    // status = false;
                    m_currentElement = indexOfFstEnable();
                }
                break;
            }
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if (m_elementList[m_currentElement]->getIsEnabled())
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    return status;
}

int ConfigPageControl::indexOfFstEnable()
{
    int i=0;
    for(i=0;i< m_maxControlInPage;i++)
    {
        if(m_elementList[i]->getIsEnabled())
        {
            break;
        }
    }
    if(i < m_maxControlInPage)
        return i;
    else
        return 0;
}

void ConfigPageControl::forceFocusToPage(bool isFirstElement)
{
    if(isFirstElement)
    {
        m_currentElement = -1;
        takeRightKeyAction();
    }
    else
    {
        m_currentElement = m_maxControlInPage;
        takeLeftKeyAction();
    }
}

void ConfigPageControl::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction ();
}

void ConfigPageControl::ctrl_D_KeyPressed(QKeyEvent *event)
{
    event->accept();
    if((processBar->isVisible() == false) && (cnfgPairType == CNFG_TYPE_DFLT_REF_SAV_BTN))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CONFI_CONTROL_DEFAULT_INFO_MSG), true);
    }
}

void ConfigPageControl::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
    if(processBar->isVisible() == false)
    {
        saveConfig();
    }
}

void ConfigPageControl::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void ConfigPageControl::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void ConfigPageControl::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_F5:
        event->accept();
        if((processBar->isVisible() == false) && (cnfgPairType != CNFG_TYPE_SAV_BTN))
        {
            getConfig();
        }
        break;

    default:
        event->accept();
        break;
    }
}

void ConfigPageControl::showEvent (QShowEvent * event)
{
    QWidget::showEvent (event);
    if(!infoPage->isVisible ())
    {
        if(m_elementList[m_currentElement] != NULL)
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void ConfigPageControl::slotCnfgBtnClicked(int index)
{
    switch(cnfgPairType)
    {
    case CNFG_TYPE_DFLT_REF_SAV_BTN:
        if(index == cnfgBtnPair[0]->getIndexInPage())
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CONFI_CONTROL_DEFAULT_INFO_MSG), true);
        }
        else if(index == cnfgBtnPair[1]->getIndexInPage())
        {
            getConfig();
        }
        else if(index == cnfgBtnPair[2]->getIndexInPage())
        {
            saveConfig();
        }
        break;

    case CNFG_TYPE_REF_SAV_BTN:
        if(index == cnfgBtnPair[0]->getIndexInPage())
        {
            getConfig();
        }
        else if(index == cnfgBtnPair[1]->getIndexInPage())
        {
            saveConfig();
        }
        break;

    case CNFG_TYPE_SAV_BTN:
        saveConfig();
        break;

    default:
        break;
    }
}

void ConfigPageControl::slotInfoPageBtnclick(int index)
{
    if((index == INFO_OK_BTN)
            && ((infoPage->getText() == ValidationMessage::getValidationMessage(CONFI_CONTROL_DEFAULT_INFO_MSG))
                || (infoPage->getText() == ValidationMessage::getValidationMessage(CONFI_CONTROL_DEFAULT_INFO_MSG))))
    {
        defaultConfig();
    }
    else
    {
        handleInfoPageMessage(index);
    }
    // to be decided whether page will give control to currentelement or not if not then
    //following statement will be placed under above ifelse condition of defined infoe msgs in control
    if((m_elementList[m_currentElement] != NULL ) && (m_elementList[m_currentElement]->getIsEnabled ()))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void ConfigPageControl::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
