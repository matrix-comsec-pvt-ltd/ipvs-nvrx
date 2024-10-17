#include "PbToolbarButton.h"
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QEvent>

#include "EnumFile.h"
#include "NavigationControl.h"

const QString buttonTypeFolder[MAX_PB_TOOLBAR_BTN] =
{
    "Play/",
    "Stop/",
    "ReversePlay/",
    "Slow/",
    "Fast/",
    "Previous/",
    "Next/",
    "Mute/",
    "Pause/",
    "UnMute/"
};

PbToolbarButton::PbToolbarButton(quint16 xParam,
                                 quint16 yParam,
                                 PB_TOOLBAR_SIZE_e toolbarSize,
                                 QWidget *parent,
                                 int indexInPage,
                                 bool isEnabled,
                                 bool catchKey,
                                 int buttonTypeIndex)
    : KeyBoard(parent),
      NavigationControl(indexInPage, isEnabled, catchKey)
{
    m_startX = xParam;
    m_startY = yParam;
    m_toolBarSize = toolbarSize;
    m_toolBarBtn = (PBTOOLBAR_BTN_IMAGE_e)buttonTypeIndex;
    if(m_isEnabled)
    {
        m_currentImageType = IMAGE_TYPE_NORMAL;
    }
    else
    {
        m_currentImageType =  IMAGE_TYPE_DISABLE;
    }

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);

    QString imgPath = PBTOOLBAR_IMAGE_PATH + pbButtonSizeFolder[m_toolBarSize] +
            buttonTypeFolder[m_toolBarBtn] + imgTypePath[m_currentImageType];

    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    setGeometryForElements();


    clickeffctTimer = new QTimer(this);
    clickeffctTimer->setInterval (250);
    clickeffctTimer->setSingleShot (true);
    connect (clickeffctTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotclickeffctTimerTimeout()));

    this->installEventFilter (this);
    this->show();
}

PbToolbarButton::~PbToolbarButton()
{
    delete clickeffctTimer;
}

void PbToolbarButton::setGeometryForElements()
{
    m_imageRect.setRect(0,
                        0,
                        m_image.width(),
                        m_image.height());
    if((m_image.width() == 0) || (m_image.height() == 0))
    {
        this->setGeometry(QRect(m_startX, m_startY, 1, 1));
    }
    else
    {
        this->setGeometry(QRect(m_startX, m_startY, m_image.width(), m_image.height()));
    }
}

void PbToolbarButton :: paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap ( m_imageRect, m_image);
}

void PbToolbarButton::changeImage (IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    QString imgPath = PBTOOLBAR_IMAGE_PATH + pbButtonSizeFolder[m_toolBarSize] +
            buttonTypeFolder[m_toolBarBtn] + imgTypePath[m_currentImageType];

    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    update ();
}

void PbToolbarButton::setIsEnabled(bool isEnable)
{
    if(isEnable != m_isEnabled)
    {
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
        if(m_isEnabled == true)
        {
            if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
            {
                changeImage(IMAGE_TYPE_NORMAL);
            }
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
        }
    }
}

void PbToolbarButton::changeToolbarBtn(PBTOOLBAR_BTN_IMAGE_e img)
{
    m_toolBarBtn = img;
    changeImage (m_currentImageType);
//    update ();
}

PBTOOLBAR_BTN_IMAGE_e PbToolbarButton::getToolbarBtn()
{
    return m_toolBarBtn;
}

void PbToolbarButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void PbToolbarButton::deSelectControl()
{
    if(m_currentImageType != IMAGE_TYPE_NORMAL)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void PbToolbarButton::forceActiveFocus()
{
    this->setFocus ();
}

void PbToolbarButton::focusInEvent(QFocusEvent *)
{
    selectControl ();
}

void PbToolbarButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl ();
}


void PbToolbarButton::mouseMoveEvent (QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_isControlActivated))
    {
        if(this->hasFocus())
        {
            selectControl();
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}


void PbToolbarButton::mousePressEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        lastClickPoint = event->pos();
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void PbToolbarButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked == true)
            && (lastClickPoint == event->pos())
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void PbToolbarButton::takeEnterKeyAction()
{
    if(!clickeffctTimer->isActive())
    {
        changeImage(IMAGE_TYPE_CLICKED);
        clickeffctTimer->start();
    }
}

void PbToolbarButton::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeEnterKeyAction();
}

bool PbToolbarButton::eventFilter (QObject *object, QEvent *event)
{
    if((event->type () == QEvent::Leave) && (m_isEnabled))
    {
        emit sigImageMouseHover(m_indexInPage,false);
    }
    else if((event->type () == QEvent::Enter) && (m_isEnabled))
    {
        emit sigImageMouseHover(m_indexInPage,true);
    }
    return QWidget::eventFilter (object, event);
}

void PbToolbarButton::navigationKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void PbToolbarButton::escKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void PbToolbarButton::slotclickeffctTimerTimeout()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClick(m_indexInPage);
}
