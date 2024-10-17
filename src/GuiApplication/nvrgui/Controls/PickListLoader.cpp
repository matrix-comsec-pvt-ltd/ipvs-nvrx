#include "PickListLoader.h"
#include <QKeyEvent>

#define MAX_ITEM_ON_PAGE        15
#define BUTTON_WIDTH            SCALE_WIDTH(260)
#define BUTTON_HEIGHT           SCALE_HEIGHT(30)
#define TOP_MARGIN              SCALE_HEIGHT(42)
#define LEFT_MARGIN             SCALE_WIDTH(30)
#define SLIDER_MARGIN           SCALE_WIDTH(10)
#define SLIDER_WIDTH            SCALE_WIDTH(13)

PickListLoader::PickListLoader(QMap<quint8, QString> optionList,
                               quint8 currentKey,
                               QString heading,
                               QWidget *parent,
                               bool catchKey,
                               bool isPickListInUse)
    : BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(200)) / 2)),
                 (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - SCALE_HEIGHT(200)) / 2)),
                 SCALE_WIDTH(200),
                 SCALE_HEIGHT(200),
                 BACKGROUND_TYPE_3,
                 MAX_TOOLBAR_BUTTON,
                 parent,
                 true,
                 heading), m_optionList(optionList), m_pickListHeading(heading),
      m_currentKey(currentKey), m_effectiveSize(0),  m_catchKey(catchKey),
      m_isPickListInUse(isPickListInUse)
{
    m_currentIndex = findMenubuttonIndexForSelectedKey();

    for(quint8 index = 0; index < (MAX_ITEM + 2); index++)
    {
        m_elementList[index] = NULL;
        if(index < MAX_ITEM)
        {
            m_options[index] = NULL;
        }
    }

    if(m_currentIndex >= MAX_ITEM_ON_PAGE)
    {
        m_firstIndex = m_currentIndex - MAX_ITEM_ON_PAGE + 1;
        m_lastIndex = m_currentIndex;
    }
    else
    {
        m_firstIndex = 0;
        m_lastIndex = MAX_ITEM_ON_PAGE - 1;
    }

    createElements();

    m_currentElement = 0;
    m_elementList[m_currentElement]->forceActiveFocus();

    if((m_isPickListInUse) && (m_options[m_currentIndex] != NULL))
    {
        m_options[m_currentIndex]->setShowClickedImage(true);
    }

    this->show();
}

PickListLoader::~PickListLoader()
{
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    disconnect(m_scrollbar,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_scrollbar,
               SIGNAL(sigScroll(int)),
               this,
               SLOT(slotScroll(int)));
    delete m_scrollbar;


    for(int index = 0; index < m_effectiveSize; index++)
    {
        disconnect(m_options[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_options[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT(slotMenuButtonClicked(int)));
        delete m_options[index];
    }
    m_optionList.clear();
}

void PickListLoader::createElements()
{
    //changes in close button
    m_mainCloseButton->setIndexInPage(0);
    m_mainCloseButton->setCatchKey(m_catchKey);
    m_elementList[0] = m_mainCloseButton;
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    //creating menubuttons
    QMap<quint8, QString>::const_iterator iterator;
    m_effectiveSize = 0;

    for(iterator = m_optionList.constBegin(); iterator != m_optionList.constEnd(); ++iterator)
    {
        if(iterator.value() != "")
        {
            bool isVisible = ((m_effectiveSize >= m_firstIndex) && (m_effectiveSize <= m_lastIndex));
            m_options[m_effectiveSize] = new MenuButton(m_effectiveSize,
                                                        BUTTON_WIDTH,
                                                        BUTTON_HEIGHT,
                                                        iterator.value(),
                                                        this,
                                                        SCALE_WIDTH(20),
                                                        LEFT_MARGIN,
                                                        TOP_MARGIN,
                                                        (m_effectiveSize + 1),
                                                        isVisible,
                                                        m_catchKey);
            connect(m_options[m_effectiveSize],
                    SIGNAL(sigButtonClicked(int)),
                    this,
                    SLOT(slotMenuButtonClicked(int)));
            connect(m_options[m_effectiveSize],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

            m_elementList[(m_effectiveSize + 1)] = m_options[m_effectiveSize];
            m_effectiveSize++;
        }
    }
    resetGeometryForMenuButtons();

    //creating scrollbar
    m_isScrollbarActive = (m_effectiveSize >= MAX_ITEM_ON_PAGE);
    m_scrollbar = new ScrollBar((LEFT_MARGIN + BUTTON_WIDTH + SLIDER_MARGIN),
                                TOP_MARGIN,
                                SLIDER_WIDTH,
                                MAX_ITEM_ON_PAGE,
                                BUTTON_HEIGHT,
                                m_effectiveSize,
                                m_currentIndex,
                                this,
                                VERTICAL_SCROLLBAR,
                                (m_effectiveSize + 1),
                                m_isScrollbarActive,
                                m_catchKey);

    connect(m_scrollbar,
            SIGNAL(sigScroll(int)),
            this,
            SLOT(slotScroll(int)));
    connect(m_scrollbar,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
//    m_elementList[m_effectiveSize + 1] = m_scrollbar;

    //reseting geometry of background
    m_backgroundWidth = BUTTON_WIDTH + (2 * LEFT_MARGIN) + SLIDER_MARGIN + SLIDER_WIDTH;
    if(m_effectiveSize <= MAX_ITEM_ON_PAGE)
    {
        m_backgroundHeight = (m_effectiveSize * BUTTON_HEIGHT) + TOP_MARGIN + LEFT_MARGIN;
    }
    else
    {
        m_backgroundHeight = (MAX_ITEM_ON_PAGE * BUTTON_HEIGHT) + TOP_MARGIN + LEFT_MARGIN;
    }

    resetGeometry((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - m_backgroundWidth) / 2)),
                  (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen()  - m_backgroundHeight) / 2)),
                  m_backgroundWidth,
                  m_backgroundHeight);
}

void PickListLoader::resetGeometryForMenuButtons()
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
}

quint8 PickListLoader::findMenubuttonIndexForSelectedKey()
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

quint8 PickListLoader::findKeyForMenubuttonIndex()
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

void PickListLoader::closePickListLoaderAction(bool isCancelClick)
{
    emit sigValueChanged(findKeyForMenubuttonIndex(),
                         m_optionList.value(findKeyForMenubuttonIndex()),
                         isCancelClick);
    this->deleteLater();
}

void PickListLoader::takeUpKeyAction()
{
    bool status = false;
    do
    {
        if(m_currentElement == 0)
        {
            status = false;
            break;
        }
        else if((m_isScrollbarActive) &&
                (m_firstIndex > 0) &&
                (m_currentElement == (m_firstIndex + 1)))
        {
            status = true;
            m_currentElement = (m_currentElement - 1);
            break;
        }
        else
        {
            status = true;
            m_currentElement = (m_currentElement - 1);
        }

    }while((m_elementList[m_currentElement] == NULL) ||
           (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        if((IS_VALID_OBJ(m_scrollbar)) &&
                (m_isScrollbarActive) &&
                (m_currentElement == m_firstIndex))
        {
            m_scrollbar->updateBarGeometry(-1);
            if(IS_VALID_OBJ(m_elementList[m_currentElement]))
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
        else
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void PickListLoader::takeDownKeyAction()
{
    bool status = false;
    do
    {
        if(m_currentElement == m_effectiveSize)
        {
            status = false;
            break;
        }
        else if((m_isScrollbarActive) && (m_currentElement == (MAX_ITEM_ON_PAGE + m_firstIndex)))
        {
            status = true;
            m_currentElement = (m_currentElement + 1);
            break;
        }
        else
        {
            status = true;
            m_currentElement = (m_currentElement + 1);
        }

    }while((m_elementList[m_currentElement] == NULL) ||
           (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        if((IS_VALID_OBJ(m_scrollbar)) &&
                (m_isScrollbarActive) &&
                (m_currentElement == (MAX_ITEM_ON_PAGE + 1 + m_firstIndex)))
        {
            m_scrollbar->updateBarGeometry(1);
            if(IS_VALID_OBJ(m_elementList[m_currentElement]))
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
        else
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void PickListLoader::navigationKeyPressed(QKeyEvent * event)
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

void PickListLoader::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    closePickListLoaderAction(true);
}

void PickListLoader::wheelEvent(QWheelEvent *event)
{
    if((event->y() >= m_scrollbar->y())
            && (event->y() <= (m_scrollbar->y() + m_scrollbar->height())))
    {
        m_scrollbar->wheelEvent(event);
    }
}

void PickListLoader::slotCloseButtonClicked(int)
{
    closePickListLoaderAction(true);
}

void PickListLoader::slotMenuButtonClicked(int index)
{
    m_currentIndex = index;
    closePickListLoaderAction(false);
}

void PickListLoader::slotScroll(int numberOfSteps)
{
    if ((m_lastIndex + numberOfSteps) > (m_effectiveSize - 1))
    {
        numberOfSteps = (m_effectiveSize - 1) - m_lastIndex;
    }

    if ((m_firstIndex + numberOfSteps) < 0)
    {
        numberOfSteps = -m_firstIndex;
    }

    m_firstIndex = m_firstIndex + numberOfSteps;
    m_lastIndex = m_lastIndex + numberOfSteps;
    resetGeometryForMenuButtons();
}

void PickListLoader::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void PickListLoader::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void PickListLoader::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
}
