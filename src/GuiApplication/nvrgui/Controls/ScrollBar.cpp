#include "ScrollBar.h"
#include <QPainter>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include<qmath.h>

#define SCROLLBAR_IMAGE_PATH        ":/Images_Nvrx/Scrollbar/"
#define SLIDER_BAR_WIDTH            SCALE_WIDTH(15)
#define MARGIN_BETWEEN_IMAGE        SCALE_WIDTH(6)
#define SLIDER_BAR_MARGIN           SCALE_WIDTH(5)
#define SCROLLBAR_COLOR             "#596678"

const QString buttonImagePath [] = {"ScrollUp/", "ScrollDown/"};

ScrollBar::ScrollBar(qint32 startX,
                     qint32 startY,
                     qint32 width,
                     qint32 totalSteps,
                     qint32 unitStepSize,
                     qint32 totalElements,
                     qint32 sliderPosition,
                     QWidget *parent,
                     SCROLLBAR_TYPE_e scrollbarType,
                     qint32 indexInPage,
                     bool isEnabled,
                     bool catchKey,
                     bool cameraList)
    : KeyBoard(parent),
      NavigationControl(indexInPage, isEnabled, catchKey), m_scrollbarType(scrollbarType),
      m_startX(startX), m_startY(startY), m_width(width),
      m_height(((totalElements < totalSteps) ? (totalElements * unitStepSize) : (totalSteps * unitStepSize))),
      m_sliderPosition(sliderPosition), m_totalSteps(totalSteps),
      m_totalElements(totalElements), m_sliderRect(NULL),
      m_currentElement(UP_BUTTON), m_isMousePressed(false)
{
    m_cameralist =cameraList;
    errorAccumulator = 0;
    m_upMovement = 0;
    m_downMovemnet = 0;
    m_prevYparm = 0;
    m_stepOffset = 0;
    m_prevYparm = 0;
    m_stepOffset = 0;

    createElements();
    setGeometryForElements();

    this->show();
    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->installEventFilter (this);
}

ScrollBar::~ScrollBar()
{
    delete m_mainRect;

    for(quint8 index = 0; index < MAX_BUTTON_TYPE; index++)
    {
        disconnect(m_buttonImage[index],
                   SIGNAL(sigImageClicked(int)),
                   this,
                   SLOT(slotImageClicked(int)));
        disconnect(m_buttonImage[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_buttonImage[index];
        m_elementList[index] = NULL;
    }

    DELETE_OBJ(m_sliderRect);
}

void ScrollBar::createElements()
{
    //main background rectangle
    m_mainRect = new Rectangle(0,
                               0,
                               m_width,
                               m_height,
                               CLICKED_BKG_COLOR,
                               this,
                               0,
                               1,
                               BORDER_2_COLOR);

    //up and down buttons
    for(quint8 index = 0; index < MAX_BUTTON_TYPE; index++)
    {
        POINT_PARAM_TYPE_e paramType = (index == 0 ? START_X_START_Y : END_X_END_Y);
        quint16 startX = (index == 0 ? 0 : m_width);
        quint16 startY = (index == 0 ? 0 : m_height);

        m_buttonImage[index] = new Image(startX,
                                         startY,
                                         (QString(SCROLLBAR_IMAGE_PATH) + buttonImagePath[index]),
                                         this,
                                         paramType,
                                         index,
                                         m_isEnabled,
                                         false,
                                         m_catchKey);
        connect(m_buttonImage[index],
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotImageClicked(int)));
        connect(m_buttonImage[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        m_elementList[index] = m_buttonImage[index];
    }

    if(m_isEnabled)
    {
        qreal internalHeight = (m_height - (2 * m_buttonImage[UP_BUTTON]->height()));
        m_unitStepSizeForBar = internalHeight /(qreal) m_totalElements;
        qreal extraSpace = m_unitStepSizeForBar * (m_totalElements - m_totalSteps);
        m_sliderLength = internalHeight - extraSpace;
        qreal yParam = 0;
        if(m_cameralist == true)
        {
            if(m_sliderPosition < m_totalElements)
            {
                yParam = m_buttonImage[UP_BUTTON]->height()+(m_sliderPosition * m_unitStepSizeForBar);
            }

            if(yParam + m_sliderLength > (m_height - m_buttonImage[DOWN_BUTTON]->height() ))
            {
                yParam = (m_height - m_buttonImage[DOWN_BUTTON]->height()) -m_sliderLength;
            }
        }
        else
        {
            if(m_sliderPosition < m_totalSteps)
            {
                yParam = m_buttonImage[UP_BUTTON]->height();
            }
            else
            {
                yParam = m_buttonImage[UP_BUTTON]->height() + ((m_sliderPosition - m_totalSteps + 1) * m_unitStepSizeForBar);
            }
        }

        m_sliderRect = new Rectangle(SLIDER_BAR_MARGIN,
                                     yParam,
                                     (m_width - (2*SLIDER_BAR_MARGIN)),
                                     m_sliderLength,
                                     SCROLLBAR_COLOR,
                                     this);
    }
}

void ScrollBar::setGeometryForElements()
{
    this->setGeometry(QRect(m_startX, m_startY, m_width, m_height));
}

void ScrollBar::updateBarGeometry(qreal offSet)
{
    if((m_isEnabled) && (offSet != 0) && (m_sliderRect != NULL))
    {
        qreal newYParam = ((qreal)(m_sliderRect->y() + (qreal)(offSet * m_unitStepSizeForBar)));
        if(((qreal)(newYParam) + m_sliderLength) <= ((qreal)(m_height - m_buttonImage[DOWN_BUTTON]->height())))
        {
            int temp = newYParam;
            if(temp > newYParam)
            {
                errorAccumulator += (temp - newYParam);
            }
            else
            {
                errorAccumulator += (newYParam - temp);
            }

            if(errorAccumulator >= 1.0f)
            {
                newYParam += 1;
                errorAccumulator -= 1;
            }
        }

        if(((newYParam >= m_buttonImage[UP_BUTTON]->height()-1)) && (((qreal)(newYParam + m_sliderLength))
                    <= ((qreal)(m_height - (m_buttonImage[DOWN_BUTTON]->height()-1)))))
        {
            m_sliderRect->resetGeometry(m_sliderRect->x(),
                                        (newYParam),
                                        m_sliderRect->width(),
                                        m_sliderRect->height());
            emit sigScroll(offSet);
        }
    }
}

void ScrollBar::forceActiveFocus()
{
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ScrollBar::setIsEnabled(bool isEnable)
{
    if(m_isEnabled == isEnable)
    {
        return;
    }

    m_isEnabled = isEnable;
    this->setEnabled(m_isEnabled);

    for(quint8 index = 0; index < MAX_BUTTON_TYPE; index++)
    {
        m_elementList[index]->setIsEnabled(m_isEnabled);
    }

    if(m_sliderRect == NULL)
    {
        return;
    }

    m_sliderRect->changeColor(m_isEnabled ? SCROLLBAR_COLOR : DISABLE_FONT_COLOR);
    m_sliderRect->update();
}

void ScrollBar::takeUpKeyAction()
{
    m_currentElement = UP_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ScrollBar::takeDownKeyAction()
{
    m_currentElement = DOWN_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

bool ScrollBar::eventFilter(QObject *object, QEvent *event)
{
    if((event->type () == QEvent::HoverEnter) || (event->type ()== QEvent::Enter))
    {
        if((m_isEnabled) && (m_sliderRect != NULL))
        {
            m_sliderRect->changeColor (HIGHLITED_FONT_COLOR);
            m_sliderRect->update ();
        }
        event->accept();
        return true;
    }
    else if((event->type() == QEvent::HoverLeave) || (event->type() == QEvent::Leave))
    {
        if((m_isEnabled) && (m_sliderRect != NULL))
        {
            m_sliderRect->changeColor (SCROLLBAR_COLOR);
            m_sliderRect->update ();
        }
        m_isMousePressed =  false;
        event->accept();
        return true;
    }
    else
    {
        return QWidget::eventFilter (object, event);
    }
}

void ScrollBar::wheelEvent(QWheelEvent * event)
{
    if(m_isEnabled)
    {
        qreal numOfSteps = event->delta() / 120;
        updateBarGeometry(-numOfSteps);
    }
    QWidget::wheelEvent(event);
}

void ScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if((m_isControlActivated) && (m_isEnabled) && (m_sliderRect != NULL))
    {
        if(this->hasFocus())
        {
            forceActiveFocus();
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }

        if(false == m_isMousePressed)
        {
            return;
        }

        if(m_lastClickPoint == event->pos())
        {
            return;
        }

        qreal posMoved = (m_lastClickPoint.y () - event->y ());
        if(posMoved < 0) // lastSected point to downwords
        {
            m_upMovement = 0;
            m_downMovemnet = m_downMovemnet - posMoved;
            if(m_downMovemnet > m_unitStepSizeForBar )
            {
                updateBarGeometry((int)(m_downMovemnet/m_unitStepSizeForBar ));
                m_downMovemnet = 0;
            }
        }
        else if (posMoved > 0)
        {
            m_downMovemnet = 0;
            m_upMovement = m_upMovement + posMoved;
            if(m_upMovement > m_unitStepSizeForBar)
            {
                updateBarGeometry(-(int)(m_upMovement/m_unitStepSizeForBar));
                m_upMovement = 0;
            }
        }

        m_lastClickPoint = QPoint(event->pos());
    }
}

void ScrollBar::mousePressEvent (QMouseEvent *event)
{
    if(m_isEnabled)
    {
        if(m_mainRect->geometry ().contains (event->pos ()))
        {
            m_isMousePressed = true;
            m_lastClickPoint = QPoint(event->pos());
        }
    }
}

void ScrollBar::mouseReleaseEvent (QMouseEvent *)
{
    m_isMousePressed = false;
    if((m_isEnabled) && (m_sliderRect != NULL))
    {
        m_sliderRect->changeColor (SCROLLBAR_COLOR);
        m_sliderRect->update ();
        m_upMovement =   0;
        m_downMovemnet = 0;
    }
}

void ScrollBar::navigationKeyPressed(QKeyEvent *event)
{
    if (false == m_catchKey)
    {
        return;
    }

    switch(event->key())
    {
        case Qt::Key_Up:
            if(m_currentElement == DOWN_BUTTON)
            {
                event->accept();
                takeUpKeyAction();
            }
            else
            {
                event->accept();
            }
            break;
        case Qt::Key_Down:
            if(m_currentElement == UP_BUTTON)
            {
                event->accept();
                takeDownKeyAction();
            }
            else
            {
                event->accept();
            }
            break;
        default:
            event->accept();
            break;
    }
}

void ScrollBar::slotImageClicked(int indexInPage)
{
    updateBarGeometry((indexInPage == UP_BUTTON) ? -1 : 1);
}

void ScrollBar::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
    if(!this->hasFocus())
    {
        emit sigUpdateCurrentElement(m_indexInPage);
    }
}

void ScrollBar::updateTotalElement(qint32 element)
{
    m_totalElements = element;
    if (m_totalElements <= 0)
    {
        return;
    }

    qreal internalHeight = (m_height - (2 * m_buttonImage[UP_BUTTON]->height()));
    m_unitStepSizeForBar = internalHeight / m_totalElements;
    qreal extraSpace = m_unitStepSizeForBar * (m_totalElements - m_totalSteps);
    m_sliderLength = internalHeight - extraSpace;
    qint32 yParam;
    if(m_sliderPosition < m_totalSteps)
    {
        yParam = m_buttonImage[UP_BUTTON]->height();
    }
    else
    {
        yParam = m_buttonImage[UP_BUTTON]->height() + ((m_sliderPosition - m_totalSteps + 1) * m_unitStepSizeForBar);
    }

    DELETE_OBJ(m_sliderRect);
    m_sliderRect = new Rectangle(SLIDER_BAR_MARGIN,
                                 yParam,
                                 (m_width - (2*SLIDER_BAR_MARGIN)),
                                 m_sliderLength,
                                 SCROLLBAR_COLOR,
                                 this);
}
