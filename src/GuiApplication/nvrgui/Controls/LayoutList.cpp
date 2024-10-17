#include "LayoutList.h"
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include "ApplController.h"

#define LEFT_MARGIN             5
#define UNIT_WIDTH              1
#define UNIT_HEIGHT             1
#define SCROLL_BAR_WIDTH        SCALE_WIDTH(13)

LayoutList::LayoutList(quint32 startX,
                       quint32 startY,
                       LAYOUT_LIST_TYPE_e listType,
                       QWidget *parent,
                       quint8 maxLayout,
                       int indexInPage)
    : KeyBoard(parent), NavigationControl(indexInPage, true)
{
    INIT_OBJ(m_scrollbar);

    for(quint8 index = 0; index < MAX_LAYOUTS; index++)
    {
        INIT_OBJ(m_layoutListButtons[index]);
        m_elementList[index] = NULL;
    }

    m_startx = startX;
    m_starty = startY;
    m_maxLayout = maxLayout;
    m_listType = listType;
    m_offset = 0;

    switch (m_listType)
    {
    case LAYOUT_LIST_WITH_SCROLL:
        m_totalRow = m_maxLayout - 2; // set to display 14 layout option in Live view
        m_totalCol = 1;
        break;

    case LAYOUT_LIST_4X4_TYPE:
        m_totalRow = 4;
        m_totalCol = 4;
        break;

    case LAYOUT_LIST_4X1_TYPE:
        m_totalRow = 4;
        m_totalCol = 1;
        break;

    default:
        break;
    }

	/* Create buttons with required index for syncPlaybackLayoutList */
	if(LAYOUT_LIST_4X1_TYPE == m_listType)
	{
		m_layoutListButtons[0] = new LayoutListButton(ONE_X_ONE, m_totalRow, m_totalCol, this);
		m_layoutListButtons[1] = new LayoutListButton(TWO_X_TWO, m_totalRow, m_totalCol, this);
		m_layoutListButtons[2] = new LayoutListButton(THREE_X_THREE, m_totalRow, m_totalCol, this);
		m_layoutListButtons[3] = new LayoutListButton(FOUR_X_FOUR, m_totalRow, m_totalCol, this);
	}

	for(quint8 rowIndex = 0; rowIndex < m_totalRow; rowIndex++)
	{
		for(quint8 colIndex = 0; colIndex < m_totalCol; colIndex++)
		{
			int index = ((rowIndex * m_totalCol) + colIndex);

            if(m_listType != LAYOUT_LIST_4X1_TYPE)
			{
                #if defined(RK3568_NVRL)
                /* RK3568 does not support EIGTH_X_EIGTH layout option, Hence in 4x4 mode disable EIGTH_X_EIGTH option for Display Mode */
                if((m_listType == LAYOUT_LIST_4X4_TYPE) && (index == EIGTH_X_EIGTH))
                {
                    continue;
                }
                #endif
				m_layoutListButtons[index] = new LayoutListButton(index, m_totalRow, m_totalCol, this);
			}
			else
			{
				/* Re-arrange buttons Geometry for syncPlaybackLayoutList as per the Row and Col Value */
				m_layoutListButtons[index]->setSyncPbButtonGeometry(rowIndex,colIndex);
			}
			m_elementList[index] = m_layoutListButtons[index];
			connect(m_layoutListButtons[index],
					SIGNAL(sigButtonClicked(int)),
					this,
					SLOT(slotButtonClicked(int)));
			connect(m_layoutListButtons[index],
					SIGNAL(sigUpdateCurrentElement(int)),
					this,
					SLOT(slotUpdateCurrentElement(int)));
		}
	}

    m_currentElement = 0;

    setGeometryForElements();

    if(m_listType == LAYOUT_LIST_WITH_SCROLL)
    {
#if defined(RK3568_NVRL)
        qint32 totalElements =  MAX_LAYOUTS - 1;   /* RK3568 does not support EIGTH_X_EIGTH layout option, hence in LAYOUT_LIST_WITH_SCROLL mode disable EIGTH_X_EIGTH option for Live View  by setting total elements's value 15 */
#else
        qint32 totalElements =  MAX_LAYOUTS;
#endif
        m_scrollbar = new ScrollBar((LAYOUT_LIST_BUTTON_WIDTH + SCALE_WIDTH(5)),
                                    SCALE_HEIGHT(LEFT_MARGIN),
                                    SCROLL_BAR_WIDTH,
                                    (MAX_LAYOUTS - 2),
                                    LAYOUT_LIST_BUTTON_HEIGHT,
                                    totalElements,
                                    0,
                                    this,
                                    VERTICAL_SCROLLBAR,
                                    255);
        if(IS_VALID_OBJ(m_scrollbar))
        {
            connect(m_scrollbar,
                    SIGNAL(sigScroll(int)),
                    this,
                    SLOT(slotScroll(int)));
        }
    }

    this->setEnabled(true);
    this->show();
}

LayoutList::~LayoutList()
{
    if(IS_VALID_OBJ(m_scrollbar))
    {
        disconnect(m_scrollbar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScroll(int)));
        DELETE_OBJ(m_scrollbar);
    }

    for(int index = 0; index < MAX_LAYOUTS; index++)
    {
        if(IS_VALID_OBJ(m_layoutListButtons[index]))
        {
            disconnect(m_layoutListButtons[index],
                       SIGNAL(sigButtonClicked(int)),
                       this,
                       SLOT(slotButtonClicked(int)));

            disconnect(m_layoutListButtons[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));

            DELETE_OBJ(m_layoutListButtons[index]);
            m_elementList[index] = NULL;
        }
    }    
}

void LayoutList::setGeometryForElements()
{
    this->setGeometry(QRect(m_startx,
                            m_starty,
                            ((LAYOUT_LIST_BUTTON_WIDTH * m_totalCol) + (2 * (SCALE_WIDTH(LEFT_MARGIN))) +
                                ((m_listType == LAYOUT_LIST_WITH_SCROLL) ? (SCROLL_BAR_WIDTH) : (0))),
                            ((LAYOUT_LIST_BUTTON_HEIGHT * m_totalRow) + (2 * SCALE_HEIGHT(LEFT_MARGIN)))));

    m_mainRect.setRect(0,
                       0,
                       this->width(),
                       this->height());

    m_topBorder.setRect(0,
                        0,
                        this->width(),
                        UNIT_HEIGHT);

    m_leftBorder.setRect(0,
                         0,
                         UNIT_WIDTH,
                         this->height());

    m_rightBorder.setRect((this->width() - UNIT_WIDTH),
                          0,
                          UNIT_WIDTH,
                          this->height());

    m_bottomBorder.setRect (0,
                            (this->height () - UNIT_WIDTH),
                            this->width (),
                            UNIT_HEIGHT);
}

void LayoutList::changeMaxLayout(quint8 maxLayout)
{
    m_maxLayout = maxLayout;
    for(quint8 index = 0; index < MAX_LAYOUTS; index++)
    {
        m_layoutListButtons[index]->setVisible (false);
        m_layoutListButtons[index]->setIsEnabled (false);
    }

    for(quint8 index = 0; index < m_maxLayout; index++)
    {
        m_layoutListButtons[index]->setVisible (true);
        m_layoutListButtons[index]->setIsEnabled (true);
    }
    setGeometryForElements();
}

void LayoutList::takeUpKeyAction()
{
    switch (m_listType)
    {
    case LAYOUT_LIST_WITH_SCROLL:
    {
        bool status = true;
        do
        {            
            if((m_offset > 0) && (m_currentElement == 0))
            {
                status = true;
                break;
            }
            else if((m_currentElement > 0) && (m_currentElement <= (m_maxLayout - 1)))
            {
                m_currentElement = (m_currentElement - 1);
            }
            else
            {
                status = false;
                break;
            }

        }while((m_elementList[m_currentElement] == NULL)
               || (!m_elementList[m_currentElement]->getIsEnabled()));

        if(status == true)
        {
            if((m_offset > 0) && (m_currentElement == 0) &&
                    (m_layoutListButtons[m_currentElement]->hasFocus()))
            {               
                m_scrollbar->updateBarGeometry(-1);
                m_currentElement = 0;
                m_layoutListButtons[m_currentElement]->selectControl();
                if(m_elementList[m_currentElement] != NULL)
                {
                    m_elementList[m_currentElement]->forceActiveFocus();
                }
            }
            else
            {
                if(m_elementList[m_currentElement] != NULL)
                {
                    m_elementList[m_currentElement]->forceActiveFocus();
                }
            }
        }
    }
        break;

    case LAYOUT_LIST_4X4_TYPE:
    {
        quint8 rowNo = m_currentElement / m_totalRow;

        if(rowNo > 0)
        {
            m_currentElement = m_currentElement - m_totalCol;
            if((IS_VALID_OBJ(m_elementList[m_currentElement])) &&
                    (m_elementList[m_currentElement]->getIsEnabled()))
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
    }
        break;

    default:
        break;
    }
}

void LayoutList::takeDownKeyAction()
{
    switch (m_listType)
    {
    case LAYOUT_LIST_WITH_SCROLL:
    {
        bool status = true;
        do
        {
            if((m_currentElement > (m_totalRow - 1)) && (m_currentElement <= (m_maxLayout - 1)))
            {
                status = true;
                break;
            }
            else if((m_currentElement >= 0) && (m_currentElement < (m_maxLayout - 1)))
            {
                m_currentElement = (m_currentElement + 1);
            }
            else
            {
                status = false;
                break;
            }

        }while((m_elementList[m_currentElement] == NULL)
               || (!m_elementList[m_currentElement]->getIsEnabled()));

        if(status == true)
        {
            if(m_currentElement > (m_totalRow - 1))
            {                
                m_scrollbar->updateBarGeometry (1);
                m_currentElement = m_totalRow - 1;
                m_layoutListButtons[m_currentElement]->selectControl();
                m_elementList[m_currentElement]->forceActiveFocus();
            }
            else
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
    }
        break;

    case LAYOUT_LIST_4X4_TYPE:
    {
        quint8 rowNo = m_currentElement / m_totalRow;

        if(rowNo < (m_totalRow-1))
        {
            m_currentElement = m_currentElement + m_totalCol;
            if((IS_VALID_OBJ(m_elementList[m_currentElement])) &&
                    (m_elementList[m_currentElement]->getIsEnabled()))
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
    }
        break;

    default:
        break;
    }
}

void LayoutList::takeLeftKeyAction()
{
    quint8 colNo = m_currentElement % m_totalCol;

    if(colNo > 0)
    {
        m_currentElement = m_currentElement - 1;
        if((IS_VALID_OBJ(m_elementList[m_currentElement])) &&
                (m_elementList[m_currentElement]->getIsEnabled()))
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void LayoutList::takeRightKeyAction()
{
    quint8 colNo = m_currentElement % m_totalCol;

    if(colNo < (m_totalCol-1))
    {
        m_currentElement = m_currentElement + 1;
        if((IS_VALID_OBJ(m_elementList[m_currentElement])) &&
                (m_elementList[m_currentElement]->getIsEnabled()))
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void LayoutList::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    painter.setBrush (QBrush(QColor(NORMAL_BKG_COLOR), Qt::SolidPattern));
    painter.drawRect(m_mainRect);

    painter.setBrush (QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
    painter.drawRect(m_topBorder);
    painter.drawRect(m_leftBorder);
    painter.drawRect(m_rightBorder);
    painter.drawRect (m_bottomBorder);
}

void LayoutList::wheelEvent(QWheelEvent *event)
{
    if ((IS_VALID_OBJ(m_scrollbar))
            && (event->x() >= (m_scrollbar->x() - LAYOUT_LIST_BUTTON_WIDTH)) && (event->x() <= (m_scrollbar->x() + m_scrollbar->width()))
            && (event->y() >= m_scrollbar->y()) && (event->y() <= (m_scrollbar->y() + m_scrollbar->height())))
    {
        m_scrollbar->wheelEvent(event);
    }
    else
    {
        QWidget::wheelEvent(event);
    }
}

void LayoutList::navigationKeyPressed(QKeyEvent *event)
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

    case Qt::Key_Left:
        event->accept();
        switch (m_listType)
        {
        case LAYOUT_LIST_WITH_SCROLL:
//            takeUpKeyAction();
            break;

        case LAYOUT_LIST_4X4_TYPE:
            takeLeftKeyAction();
            break;

        default:
            break;
        };
        break;

    case Qt::Key_Right:
        event->accept();
        switch (m_listType)
        {
        case LAYOUT_LIST_WITH_SCROLL:
//            takeDownKeyAction();
            break;

        case LAYOUT_LIST_4X4_TYPE:
            takeRightKeyAction();
            break;

        default:
            break;
        };
        break;

    default:
        event->accept();
        break;
    }
}

void LayoutList::forceFocusToPage(bool isFirstElement)
{
    if(isFirstElement == true)
    {
        m_currentElement = 0;
        if(m_elementList[m_currentElement]!= NULL)
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
    else
    {
        m_currentElement = m_maxLayout;
        takeUpKeyAction();
    }
}

void LayoutList::slotButtonClicked(int index)
{
    emit sigChangeLayout((LAYOUT_TYPE_e)index);
}

void LayoutList::slotUpdateCurrentElement(int index)
{
    m_currentElement = index - m_offset;
    emit sigUpdateCurrentElement(m_indexInPage);
}

void LayoutList::slotScroll(int offset)
{
    if((m_offset + offset) >= 0)
    {
        m_offset += offset;

        for(quint8 index = 0; index < m_totalRow; index++)
        {
            if(IS_VALID_OBJ(m_layoutListButtons[index]))
            {
                m_layoutListButtons[index]->changeButtonIndex(index + m_offset);
            }
        }
    }
}

void LayoutList::resetGeometry(quint32 iStartX, quint32 iStartY)
{
	/*	Update TextLabel geometry as per the StartX and StartY Value received */
	m_startx = iStartX;
	m_starty = iStartY;
	setGeometryForElements();
}
