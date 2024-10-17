#include "DropDownList.h"
#include <QKeyEvent>

#define BUTTON_WIDTH            SCALE_WIDTH(260)
#define BUTTON_HEIGHT           SCALE_HEIGHT(30)
#define TOP_MARGIN              SCALE_HEIGHT(42)
#define LEFT_MARGIN             SCALE_WIDTH(10)
#define SLIDER_MARGIN           SCALE_HEIGHT(10)
#define SLIDER_WIDTH            SCALE_WIDTH(13)

DropDownList::DropDownList(quint16 startx,
                           quint16 starty,
                           quint16 width,
                           QMap<quint8, QString> optionList,
                           quint8 currentKey, QWidget *parent,
                           quint8 maxElemetOnList)
    : BackGround(startx,
                 starty,
                 width,
                 SCALE_HEIGHT(200),
                 BACKGROUND_TYPE_3,
                 MAX_TOOLBAR_BUTTON,
                 parent,
                 false,
                 "",
                 CLICKED_BKG_COLOR,
                 1.0,
                 WINDOW_GRID_COLOR), m_currentKey(currentKey), m_effectiveSize(0),
      m_tempstartX(startx), m_tempstartY(starty), m_firstIndex(0), m_currentElement(0),
      m_tempwidth(width), m_optionList(optionList), m_scrollbar(NULL), m_maxElemetOnList(maxElemetOnList)
{
    m_lastIndex = (m_maxElemetOnList - 1);
    m_catchKey = false;
    m_currentIndex = 0;
    m_isPickListInUse = false;
    m_isUsedForTextList = false;

    createElements();

    if (m_scrollbar != NULL)
    {
        quint8 maxUpStep;
        if (m_currentKey >= m_effectiveSize)
        {
            maxUpStep = 0;
        }
        else if (m_currentKey > (m_effectiveSize - m_maxElemetOnList - 1))
        {
            maxUpStep = m_effectiveSize - m_maxElemetOnList;
        }
        else
        {
            maxUpStep = m_currentKey;
        }
        m_scrollbar->updateBarGeometry(maxUpStep);
    }
    else if(m_effectiveSize > 0)
    {       
        m_elementList[m_currentElement]->forceActiveFocus();        
    }

    this->setFocus ();
    this->show();
}

DropDownList::~DropDownList()
{
    for(quint8 index = 0; index < m_effectiveSize; index++)
    {
        disconnect(m_options[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_options[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT(slotOptionClicked(int)));
        m_options[index].clear();
    }
    m_optionList.clear();

    if(m_scrollbar != NULL)
    {
        disconnect(m_scrollbar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScroll(int)));
        disconnect(m_scrollbar,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        m_scrollbar.clear();
    }
}

void DropDownList::createElements ()
{
    //creating menubuttons
    QMap<quint8, QString>::const_iterator iterator;
    quint8 index = 0;
    m_effectiveSize = 0;

    //creating scrollbar
    m_isScrollbarActive = (m_optionList.size () > m_maxElemetOnList);

    //creating optionList
    for(iterator = m_optionList.constBegin(); iterator != m_optionList.constEnd(); ++iterator)
    {
        if(iterator.value() != "")
        {
            bool isVisible = ((m_effectiveSize >= m_firstIndex) && (m_effectiveSize <= m_lastIndex));
            m_options[m_effectiveSize] = new MenuButton(m_effectiveSize,
                                                        ((m_isScrollbarActive) ?  (m_tempwidth - SLIDER_WIDTH)
                                                                                : m_tempwidth),
                                                        BUTTON_HEIGHT,
                                                        iterator.value(),
                                                        this,
                                                        SCALE_WIDTH(5),
                                                        SCALE_WIDTH(5),
                                                        1,
                                                        (m_effectiveSize + 1),
                                                        isVisible,
                                                        true,
                                                        true,
                                                        true,
                                                        false,
                                                        false,
                                                        ((index == m_currentKey) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR));
            connect(m_options[m_effectiveSize],
                    SIGNAL(sigButtonClicked(int)),
                    this,
                    SLOT(slotOptionClicked(int)));
            connect(m_options[m_effectiveSize],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

            m_elementList[(m_effectiveSize)] = m_options[m_effectiveSize];
            m_effectiveSize++;
        }
        index ++;
    }

    if(m_isScrollbarActive)
    {
        m_scrollbar = new ScrollBar(((m_tempwidth - SLIDER_WIDTH) + SCALE_WIDTH(5)),
                                    1,
                                    SLIDER_WIDTH,
                                    m_maxElemetOnList,
                                    BUTTON_HEIGHT,
                                    m_effectiveSize,
                                    0,
                                    this,
                                    VERTICAL_SCROLLBAR,
                                    (m_effectiveSize),
                                    m_isScrollbarActive,
                                    true);

        m_elementList[m_effectiveSize] = m_scrollbar;

        connect(m_scrollbar,
                SIGNAL(sigScroll(int)),
                this,
                SLOT(slotScroll(int)),
                Qt::DirectConnection);
        connect(m_scrollbar,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }
    //reseting geometry of background
    m_backgroundWidth = m_tempwidth + SLIDER_MARGIN ;

    if(m_effectiveSize <= m_maxElemetOnList)
    {
        m_backgroundHeight = (m_effectiveSize * BUTTON_HEIGHT) + SCALE_HEIGHT(3);
    }
    else
    {
        m_backgroundHeight = (m_maxElemetOnList * BUTTON_HEIGHT) + SCALE_HEIGHT(3);
    }

    resetGeometry(m_tempstartX,m_tempstartY,m_backgroundWidth,m_backgroundHeight);
}

void DropDownList::resetGeometryForMenuButtons()
{
    for(quint8 index = 0; index < m_effectiveSize; index++)
    {
        m_options[index]->resetGeometry(0, 0);
        if((index < m_firstIndex)
                || (index > m_lastIndex))
        {
            m_options[index]->setIsEnabled(false);
        }
        else
        {
            m_options[index]->setIsEnabled(true);
            m_options[index]->resetGeometry(0, -m_firstIndex);
        }
    }

    if ((m_currentElement <= m_firstIndex) && (m_currentElement >= m_lastIndex))
    {
        m_currentElement = m_firstIndex;
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

quint8 DropDownList::findMenubuttonIndexForSelectedKey()
{
    QList<quint8> keys = m_optionList.keys();
    qint8 menubuttonIndex = -1;
    for(quint8 index = 0; index < keys.size(); index++)
    {
        quint8 key = keys.at(index);
        if(m_optionList.value(key) != "")
        {
            menubuttonIndex++;
        }
        if(key == m_currentKey)
        {
            break;
        }
    }
    return menubuttonIndex;
}

quint8 DropDownList::findKeyForMenubuttonIndex()
{
    QList<quint8> keys = m_optionList.keys();
    quint8 key = m_currentKey;
    qint16 menubuttonIndex = -1;
    for(quint8 index = 0; (index < keys.size() && menubuttonIndex < m_currentIndex); index++)
    {
        key = keys.at(index);
        if(m_optionList.value(key) != "")
        {
            menubuttonIndex++;
        }
    }
    return key;
}

void DropDownList::closeDropListAction()
{
    emit sigValueChanged(findKeyForMenubuttonIndex(),
                         m_optionList.value(findKeyForMenubuttonIndex()));
    this->deleteLater();
}

void DropDownList::takeUpKeyAction()
{
    if((m_currentElement == (m_firstIndex))
            && (m_firstIndex != 0)
            && (m_scrollbar != NULL))
    {
        m_scrollbar->updateBarGeometry (-1);
    }
    else
    {
        if((m_currentElement == (m_firstIndex) )
                && (m_firstIndex == 0)
                && (m_scrollbar != NULL))
        {
            m_scrollbar->updateBarGeometry ((m_effectiveSize -  m_lastIndex - 1));
        }
    }

    if(m_effectiveSize > 0)
    {
        do
        {
            m_currentElement = (m_currentElement - 1 + (m_effectiveSize))
                    % (m_effectiveSize);
        }while(!m_elementList[m_currentElement]->getIsEnabled());

        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void DropDownList::takeDownKeyAction()
{
    if((m_currentElement == (m_effectiveSize - 1))
            && (m_lastIndex != (m_maxElemetOnList - 1))
            && (m_scrollbar != NULL))
    {
        m_scrollbar->updateBarGeometry (-m_firstIndex);
    }
    else
    {
        if((m_currentElement == (m_lastIndex))
                && (m_scrollbar != NULL))
        {
            m_scrollbar->updateBarGeometry (1);
        }
    }

    if(m_effectiveSize > 0)
    {
        do
        {
            m_currentElement = (m_currentElement + 1)
                    % (m_effectiveSize);
        }while(!m_elementList[m_currentElement]->getIsEnabled());

        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void DropDownList::navigationKeyPressed(QKeyEvent *event)
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

void DropDownList::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    closeDropListAction();
}

void DropDownList::enterKeyPressed(QKeyEvent *event)
{
    event->accept ();
    m_currentIndex = m_currentElement;
    closeDropListAction();
}

void DropDownList::wheelEvent (QWheelEvent *event)
{
    if((m_scrollbar != NULL) &&
            (event->y () >= m_scrollbar->y ()) &&
            event->y () <= (m_scrollbar->y () + m_scrollbar->height ()))
    {
        m_scrollbar->wheelEvent (event);
    }
}

void DropDownList::setFocusToPage()
{
    m_currentElement = 0;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void DropDownList::slotOptionClicked (int index)
{
    m_currentIndex = index;
    closeDropListAction();
}

void DropDownList::slotScroll(int numberOfSteps)
{
    if ((m_lastIndex + numberOfSteps) > (m_effectiveSize - 1))
    {
        numberOfSteps = (m_effectiveSize - 1) - m_lastIndex;
    }

    if ((m_firstIndex + numberOfSteps) < 0)
    {
        numberOfSteps = -m_firstIndex;
    }

    m_firstIndex += numberOfSteps;
    m_lastIndex += numberOfSteps;
    resetGeometryForMenuButtons();
}

void DropDownList::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void DropDownList::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void DropDownList::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void DropDownList::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void DropDownList::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void DropDownList::ctrl_D_KeyPressed(QKeyEvent *event)
{
    event->accept();
}
