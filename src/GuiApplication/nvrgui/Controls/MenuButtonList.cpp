#include "MenuButtonList.h"

#include <QKeyEvent>

MenuButtonList::MenuButtonList(int xParam, int yParam,
                               QStringList menuLabelList,
                               QWidget* parent,
                               bool catchKey,
                               quint16 buttonWidth)
    : KeyBoard(parent), m_menuLabelList(menuLabelList), m_catchKey(catchKey),
      m_startX(xParam), m_startY(yParam), m_buttonWidth(buttonWidth)
{
    for(quint8 index = 0; index < m_menuLabelList.length(); index++)
    {
        m_menuButtons[index] = new MenuButton(index,
                                              m_buttonWidth,
                                              BUTTON_HEIGHT,
                                              m_menuLabelList.at(index),
                                              this, SCALE_WIDTH(10), 0, 0,
                                              index,
                                              true,
                                              m_catchKey,
                                              false,
                                              false,
                                              true);
        connect(m_menuButtons[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotMenuButtonClicked(int)));
        connect(m_menuButtons[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        m_elementList[index] = m_menuButtons[index];
    }

    this->setGeometry(m_startX,
                      m_startY,
                      m_buttonWidth,
                      (BUTTON_HEIGHT * m_menuLabelList.length()));

    m_currentElement = 0;
    m_elementList[m_currentElement]->forceActiveFocus();

    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();
}

MenuButtonList::~MenuButtonList()
{
    for(quint8 index = 0; index < m_menuLabelList.length(); index++)
    {
        disconnect(m_menuButtons[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT(slotMenuButtonClicked(int)));

        disconnect(m_menuButtons[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        delete m_menuButtons[index];
    }
    m_menuLabelList.clear();
}

void MenuButtonList::disableButton(quint8 index, bool isDisable)
{
    m_menuButtons[index]->disableButton(isDisable);
}

void MenuButtonList::takeUpKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + m_menuLabelList.length()) % m_menuLabelList.length();
    }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

    m_elementList[m_currentElement]->forceActiveFocus();
}

void MenuButtonList::takeDownKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % m_menuLabelList.length();
    }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

    m_elementList[m_currentElement]->forceActiveFocus();
}

void MenuButtonList::forceActiveFocus()
{
    m_currentElement = 0;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void MenuButtonList::navigationKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        switch(event->key())
        {
        case Qt::Key_Up:
            event->accept();
            takeUpKeyAction();
            break;

        case Qt::Key_Down:
            event->accept();
            takeDownKeyAction();
            break;

        default:
            event->accept();
            break;
        }
    }
}

void MenuButtonList::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    emit sigMenuSelected("", m_menuLabelList.length());
    this->deleteLater();
}

void MenuButtonList::slotMenuButtonClicked(int index)
{
    emit sigMenuSelected(m_menuLabelList.at(index), index);
    this->deleteLater();
}

void MenuButtonList::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
