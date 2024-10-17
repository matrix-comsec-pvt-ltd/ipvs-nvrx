#include "ManageMenuOptions.h"
#include <QKeyEvent>

ManageMenuOptions::ManageMenuOptions(QString deviceName,
                                     QWidget* parent,
                                     quint16 maxElements)
    : KeyBoard(parent), NavigationControl(1, true)
{
    m_applController = ApplController::getInstance();
    m_payloadLib = new PayloadLib();
    m_currentDeviceName  = deviceName;
    m_maxElements = maxElements;

    for(quint16 index = 0; index < m_maxElements; index++)
    {
        m_elementList[index] = NULL;
    }

    this->setGeometry(MANAGE_PAGE_RIGHT_PANEL_STARTX,
                      MANAGE_PAGE_RIGHT_PANEL_STARTY,
                      MANAGE_PAGE_RIGHT_PANEL_WIDTH,
                      MANAGE_PAGE_RIGHT_PANEL_HEIGHT);

    processBar = new ProcessBar(this->x(),
                                this->y(),
                                this->width(),
                                this->height(),
                                0,
                                parentWidget());

    infoPage = new InfoPage (0, 0,
                             (MANAGE_LEFT_PANEL_WIDTH + MANAGE_RIGHT_PANEL_WIDTH),
                             MANAGE_LEFT_PANEL_HEIGHT,
                             INFO_MANAGE,
                             parentWidget ());

    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    m_currentElement = 0;
    this->show();
}

ManageMenuOptions::~ManageMenuOptions ()
{
    delete processBar;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;

    delete m_payloadLib;
}

void ManageMenuOptions::updateStatus (QString , qint8 , qint8,quint8,quint8)
{

}

void ManageMenuOptions::handleInfoPageMessage(int)
{

}

void ManageMenuOptions::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (m_maxElements);
        }
        if(m_currentElement)
        {
            m_currentElement = (m_currentElement - 1);
        }
        else
        {
              status = false;
              break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           ||(!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void ManageMenuOptions::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == (m_maxElements - 1))
        {
            m_currentElement = -1;
        }
        if(m_currentElement != (m_maxElements - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void ManageMenuOptions::forceFocusToPage(bool isFirstElement)
{
    if ( isFirstElement)
    {
        m_currentElement = -1;
        takeRightKeyAction();
    }
    else
    {
        m_currentElement = m_maxElements;
        takeLeftKeyAction();
    }
}

void ManageMenuOptions::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void ManageMenuOptions::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void ManageMenuOptions::navigationKeyPressed(QKeyEvent * event)
{
    event->accept();
}

void ManageMenuOptions::showEvent(QShowEvent *event)
{
    QWidget::showEvent (event);
    if ( !infoPage->isVisible())
    {
        if ( m_elementList[m_currentElement] != NULL)
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void ManageMenuOptions::slotInfoPageBtnclick(int index)
{
    if ( m_elementList[m_currentElement] != NULL)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    handleInfoPageMessage(index);
}

void ManageMenuOptions::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
