#include "DeviceListButton.h"
#include <QPaintEvent>
#include <QPainter>

#define CAMERA_LIST_IMG_ICON_SOURCE     ":/Images_Nvrx/CameraListIcon/Device/"

const QString devStateStr[MAX_DEV_SEL_STATE] =
{
    "State1/",
    "State2/"
};

const QString deviceImgPath[] = {"DeviceConnected.png",
                                 "DeviceDisconnected.png",
                                 "DeviceConflict.png",
                                 "DeviceDisconnected.png"};

DeviceListButton::DeviceListButton(int index,
                                   QString label,
                                   DEVICE_STATE_TYPE_e connectionState,
                                   QWidget * parent,
                                   int indexInPage,
                                   bool isEnabled,
                                   bool stateChangeImgEnable,
                                   quint16 deviceWidth)
    : MenuButton(index,
                 deviceWidth,
                 CAMERA_LIST_BUTTON_HEIGHT,
                 label,
                 parent,
                 SCALE_WIDTH(60), 0, 0,
                 indexInPage,
                 isEnabled,
                 isEnabled,
                 false,
                 false,
                 true),
      m_currentConnectionState(MAX_DEVICE_STATE)
{
    QString devstateImgpath = CAMERA_LIST_IMG_ICON_SOURCE + devStateStr[DEV_DESELECTED];

    m_stateChangeImgEnable = stateChangeImgEnable;
    m_devStatusChangeImg = new Image((deviceWidth - SCALE_WIDTH(30)),
                                     0,
                                     devstateImgpath,
                                     this, START_X_START_Y,
                                     0, stateChangeImgEnable);
    connect (m_devStatusChangeImg,
             SIGNAL(sigImageClicked(int)),
             this,
             SLOT(slotDevStatusChangeImgClick(int)));

    updateConnectionState(connectionState);
    this->show();
}

DeviceListButton::~DeviceListButton()
{
    m_isDeletionStart = true;
    disconnect (m_devStatusChangeImg,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotDevStatusChangeImgClick(int)));
    delete m_devStatusChangeImg;
}


void DeviceListButton::updateConnectionState(DEVICE_STATE_TYPE_e connectionState)
{
    if((m_index != 0) && (connectionState != CONFLICT ) && (m_stateChangeImgEnable))
    {
        m_devStatusChangeImg->setIsEnabled(true);
    }
    else
    {
        m_devStatusChangeImg->setIsEnabled(false);
    }

    if(m_currentConnectionState != connectionState)
    {
        m_currentConnectionState = connectionState;
        changeImage();
    }
}

DEVICE_STATE_TYPE_e DeviceListButton::getConnectionstate()
{
    return m_currentConnectionState;
}

void DeviceListButton::changeDevSelectionstate(DEV_SELECTION_e state)
{
    m_devStatusChangeImg->updateImageSource(CAMERA_LIST_IMG_ICON_SOURCE + devStateStr[state], true);
}

void DeviceListButton::changeImage()
{
    m_imageSource = QString(CAMERA_LIST_IMG_ICON_SOURCE) + deviceImgPath[m_currentConnectionState];
    m_iconImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_iconImage);
    repaint ();
}

void DeviceListButton::drawImage(QPainter * painter)
{
    painter->drawPixmap((m_mainRect.topLeft().x() + SCALE_WIDTH(10)),
                        m_mainRect.topLeft().y(), m_iconImage);
}

void DeviceListButton::resetGeometry(int xOffset, int yOffset)
{
    this->setGeometry(QRect(xOffset,
                            (m_height * (m_index + yOffset)) ,
                            m_width,
                            m_height));
}
void DeviceListButton::resetGeometryCustIndex(int xOffset, int yOffset)
{

    this->setGeometry(QRect(xOffset,
                            (m_height * yOffset) ,
                            m_width,
                            m_height));
}

void DeviceListButton::paintEvent(QPaintEvent * event)
{
    MenuButton::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    drawRectangles(&painter);
    drawImage(&painter);
    QWidget::paintEvent(event);
}

void DeviceListButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton)
            && (!m_isDeletionStart))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void DeviceListButton::mousePressEvent(QMouseEvent * event)
{
    if((event->button() == m_leftMouseButton)
            && (event->x () < m_devStatusChangeImg->x ())
            && (!m_isDeletionStart))
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void DeviceListButton::navigationKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        switch(event->key())
        {
        case Qt::Key_Right:
            if(this->hasFocus())
            {
                if(m_devStatusChangeImg->getIsEnabled())
                {
                    event->accept();
                    m_devStatusChangeImg->forceActiveFocus();
                }
                else
                {
                    event->accept();
                }
            }
            else
            {
                event->accept();
            }
            break;

        case Qt::Key_Left:
            if(this->hasFocus())
            {
                event->accept();
            }
            else
            {
                event->accept();
                this->setFocus();
            }
            break;

        default:
            event->accept();
            break;
        }
    }
    QWidget::keyPressEvent(event);
}

void DeviceListButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void DeviceListButton::slotDevStatusChangeImgClick(int)
{
    m_devStatusChangeImg->setIsEnabled(false);
    emit sigDevStateChangeImgClick(m_index,
                                   m_currentConnectionState);
}
