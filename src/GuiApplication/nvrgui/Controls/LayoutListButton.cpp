#include "LayoutListButton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include "ApplController.h"

#define LEFT_MARGIN                     5

LayoutListButton::LayoutListButton(int index, quint8 totalRow, quint8 totalCol, QWidget *parent)
    : KeyBoard(parent), NavigationControl(index, true), m_totalRow(totalRow), m_totalCol(totalCol)
{
    m_index = index;
    m_actualRow = m_index / m_totalCol;
    m_actualCol = m_index % m_totalCol;

    changeButtonImage(IMAGE_TYPE_NORMAL);
    setGeometryForElements();

    m_clickEffectTimer = new QTimer(this);
    connect(m_clickEffectTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotClickEffectTimerTimeout()));
    m_clickEffectTimer->setInterval(75);
    m_clickEffectTimer->setSingleShot(true);

    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();
}

LayoutListButton::~LayoutListButton()
{
    if(m_clickEffectTimer->isActive())
    {
        m_clickEffectTimer->stop();
    }
    disconnect(m_clickEffectTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotClickEffectTimerTimeout()));
    delete m_clickEffectTimer;
}

void LayoutListButton::setGeometryForElements()
{
    m_imageRect.setRect(0,
                        0,
                        m_image.width(),
                        m_image.height());
    this->setGeometry(QRect((LAYOUT_LIST_BUTTON_WIDTH * m_actualCol) + SCALE_WIDTH(LEFT_MARGIN),
                            ((LAYOUT_LIST_BUTTON_HEIGHT * m_actualRow) + SCALE_HEIGHT(LEFT_MARGIN)),
                            LAYOUT_LIST_BUTTON_WIDTH,
                            LAYOUT_LIST_BUTTON_HEIGHT));
}

void LayoutListButton::changeButtonImage(IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    m_imageSource = QString(LAYOUTLIST_BUTTON_IMG_PATH) + layoutListImgPath[m_index] + imgTypePath[type];
    m_image = QPixmap(m_imageSource);
    SCALE_IMAGE(m_image);
}

void LayoutListButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeButtonImage(IMAGE_TYPE_MOUSE_HOVER);
        update();
    }
}

void LayoutListButton::deSelectControl()
{
    changeButtonImage(IMAGE_TYPE_NORMAL);
    update();
}

void LayoutListButton::changeButtonIndex(int index)
{
    m_index = index;
    m_indexInPage = index;
    changeButtonImage(IMAGE_TYPE_NORMAL);
    repaint();
}

void LayoutListButton::takeEnterKeyAction()
{
    if(!m_clickEffectTimer->isActive())
    {
        changeButtonImage(IMAGE_TYPE_CLICKED);
        m_clickEffectTimer->start();
    }
}

void LayoutListButton::forceActiveFocus()
{
    this->setFocus();
}

void LayoutListButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(m_imageRect, m_image);
}

void LayoutListButton::mousePressEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();            
        }
    }
}

void LayoutListButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void LayoutListButton::mouseMoveEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && m_isControlActivated)
    {
        if(this->hasFocus())
        {
            selectControl();
        }
        else
        {
            forceActiveFocus();
        }
    }
}

void LayoutListButton::focusInEvent(QFocusEvent *)
{
    selectControl();
    emit sigUpdateCurrentElement(m_indexInPage);
}

void LayoutListButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void LayoutListButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void LayoutListButton::slotClickEffectTimerTimeout()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClicked(m_indexInPage);
}

void LayoutListButton::setSyncPbButtonGeometry(quint8 iRowValue, quint8 iColValue)
{
	/*	Update LayoutListButton geometry as per the Row and Col Value received
		For SyncPB LayoutList, LayoutListButtons are not as per the layout sequence */
	m_actualCol=iColValue;
	m_actualRow=iRowValue;
	this->setGeometryForElements();
}
