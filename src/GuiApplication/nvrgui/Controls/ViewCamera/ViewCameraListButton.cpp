#include "ViewCameraListButton.h"
#include "EnumFile.h"
#include <QPaintEvent>
#include <QPainter>

#define CAMERA_LIST_BUTTON_WIDTH        SCALE_WIDTH(320)
#define CAMERA_LIST_BUTTON_HEIGHT       SCALE_HEIGHT(30)

#define CAMERA_LIST_IMG_ICON_SOURCE ":/Images_Nvrx/CameraListIcon/Camera/"

const QString cameraImgPath[] = {"Normal.png",
                                 "Connecting_Assigned.png",
                                 "Connected_Assigned.png",
                                 "Retry_Assigned.png",
                                 "Assigned.png"};

ViewCameraListButton::ViewCameraListButton(int index,
                                           QString label,
                                           CAMERA_STATE_TYPE_e connectionState,
                                           QWidget *parent,
                                           int indexInPage,
                                           bool isEnabled)
    : MenuButton(index,
                 CAMERA_LIST_BUTTON_WIDTH,
                 CAMERA_LIST_BUTTON_HEIGHT,
                 label,
                 parent,
                 SCALE_WIDTH(60), 0, 0,
                 indexInPage,
                 isEnabled,
                 isEnabled,
                 false,
                 false,
                 true), m_currentConnectionState(MAX_CAMERA_STATE)
{
    updateConnectionState(connectionState);
}

void ViewCameraListButton::updateConnectionState(CAMERA_STATE_TYPE_e connectionState)
{
    if(m_currentConnectionState != connectionState)
    {
        m_currentConnectionState = connectionState;
        changeImage();
    }
}

void ViewCameraListButton::changeImage()
{
    m_imageSource = QString(CAMERA_LIST_IMG_ICON_SOURCE) + cameraImgPath[m_currentConnectionState];
    m_iconImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_iconImage);
    update ();
}

void ViewCameraListButton::drawImage(QPainter * painter)
{
    painter->drawPixmap((m_mainRect.topLeft().x() + SCALE_WIDTH(15)),
                        m_mainRect.topLeft().y() + SCALE_HEIGHT(3), m_iconImage);
}

void ViewCameraListButton::resetGeometry(int xOffset, int yOffset)
{
    this->setGeometry(QRect(xOffset,
                            (m_height * (m_index + yOffset)) ,
                            m_width,
                            m_height));
}

void ViewCameraListButton::paintEvent(QPaintEvent * event)
{
    MenuButton::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    drawRectangles(&painter);
    drawImage(&painter);
    QWidget::paintEvent(event);
}

void ViewCameraListButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton)
            && (!m_isDeletionStart))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void ViewCameraListButton::mousePressEvent(QMouseEvent * event)
{
    if((event->button() == m_leftMouseButton)
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

void ViewCameraListButton::slotClickEffectTimerout()
{
    setShowClickedImage(false);
    emit sigButtonClicked(m_index, m_currentConnectionState);
}
