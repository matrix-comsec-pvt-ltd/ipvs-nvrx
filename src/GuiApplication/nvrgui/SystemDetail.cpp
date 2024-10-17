#include "SystemDetail.h"
#include <QPainter>
#include <QDesktopWidget>

SystemDetail::SystemDetail(QWidget *parent) :
    QWidget(parent)
{
    this->setGeometry(ApplController::getXPosOfScreen(),
                      ApplController::getYPosOfScreen(),
                      ApplController::getWidthOfScreen(),
                      ApplController::getHeightOfScreen());

    m_healthStatus = new HealthStatus(this);
    connect (m_healthStatus,
             SIGNAL(sigNextPage(QString,MENU_TAB_e)),
             this,
             SLOT(slotShowAdvanceDetail(QString,MENU_TAB_e)));

    connect (m_healthStatus,
             SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
             this,
             SLOT(slotCloseButtonClicked(TOOLBAR_BUTTON_TYPE_e)));

    m_healthStatus->setVisible (true);

    m_advanceDetails = new AdvanceDetails(this);
    connect (m_advanceDetails,
             SIGNAL(sigPrevPage(QString,bool)),
             this,
             SLOT(slotShowHealthStatus(QString,bool)));
    connect (m_advanceDetails,
             SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
             this,
             SLOT(slotCloseButtonClicked(TOOLBAR_BUTTON_TYPE_e)));

    m_advanceDetails->setVisible (false);
    m_healthStatus->setDefaultFocus ();
    this->show ();
}

SystemDetail::~SystemDetail ()
{
    disconnect (m_healthStatus,
                SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                this,
                SLOT(slotCloseButtonClicked(TOOLBAR_BUTTON_TYPE_e)));
    disconnect (m_healthStatus,
                SIGNAL(sigNextPage(QString,MENU_TAB_e)),
                this,
                SLOT(slotShowAdvanceDetail(QString,MENU_TAB_e)));
    DELETE_OBJ (m_healthStatus);

    disconnect (m_advanceDetails,
                SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                this,
                SLOT(slotCloseButtonClicked(TOOLBAR_BUTTON_TYPE_e)));

    disconnect (m_advanceDetails,
                SIGNAL(sigPrevPage(QString,bool)),
                this,
                SLOT(slotShowHealthStatus(QString,bool)));
    DELETE_OBJ (m_advanceDetails);
}

void SystemDetail::slotShowHealthStatus(QString devName, bool isCamFieldToShow)
{
    if(m_healthStatus->isVisible () == false)
    {
        m_advanceDetails->setVisible (false);
        m_healthStatus->setVisible (true);
        m_healthStatus->showHlthStatus (devName, ((isCamFieldToShow) ? (CAMERA_TAB) : (SYSTEM_TAB)));
    }
}

void SystemDetail::slotShowAdvanceDetail(QString devName, MENU_TAB_e tabToShow)
{
    if(m_advanceDetails->isVisible () == false)
    {
        m_healthStatus->setVisible (false);
        m_advanceDetails->setVisible (true);
        m_advanceDetails->showAdvDetail (devName, ((tabToShow == SYSTEM_TAB) ? false : true));
    }
}

void SystemDetail::slotCloseButtonClicked (TOOLBAR_BUTTON_TYPE_e)
{
    emit sigClosePage (SYSTEM_STATUS_BUTTON);
}

void SystemDetail::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(param->cmdType == HEALTH_STS)
    {
        if(m_healthStatus->isVisible () == true)
        {
            m_healthStatus->processDeviceResponse (param, deviceName);
        }
    }
    else
    {
        if(m_advanceDetails->isVisible () == true)
        {
            m_advanceDetails->processDeviceResponse (param, deviceName);
        }
    }
}

void SystemDetail::updateDeviceList(void)
{
    if (m_healthStatus != NULL)
    {
        m_healthStatus->updateDeviceList();
    }

    if (m_advanceDetails != NULL)
    {
        m_advanceDetails->updateDeviceList();
    }
}
